/* --------------------------------
program:sp_bd2406_2018.4.13
revise date:2018.8.21
function: 
.IO & tim4 interrupt
.key press
.adc
 ----------------------------------*/
 
#include "sp_bd2406.h"
#include "stm8l151_x.h"
volatile Bytebits Bbituse;//tag relate to adc
#define bituse Bbituse.Byte
#define adc_command 			Bbituse.Bits.bit0
#define vb_check_command 	Bbituse.Bits.bit1
#define vt_check_command 	Bbituse.Bits.bit2
#define vo_command 				Bbituse.Bits.bit3

#define vin_command				Bbituse.Bits.bit4
#define dcin_command			Bbituse.Bits.bit5

#define vb_check_tag			Bbituse.Bits.bit7
//#define vt_check_tag			Bbituse.Bits.bit6

volatile Bytebits Bbituse1;//tag relate to key press & operation
#define bituse1 Bbituse1.Byte
//key press tag
#define key_press_tag	Bbituse1.Bits.bit0//key press tag
#define kps_tag				Bbituse1.Bits.bit1//key press steady tag
#define krs_tag				Bbituse1.Bits.bit2//key relax steady tag
//long press key tag 
#define klp_tag				Bbituse1.Bits.bit3//key long press tag
#define klp_count_tag Bbituse1.Bits.bit4//key long press count tag

#define vb_init_tag			Bbituse1.Bits.bit5
#define vt_init_tag			Bbituse1.Bits.bit6
#define start_up_delay 	Bbituse1.Bits.bit7


volatile Bytebits Bbituse2;
#define bituse2 Bbituse2.Byte
#define c_1			Bbituse2.Bits.bit0//led flicker 1hz	
#define c_2			Bbituse2.Bits.bit1//led flicker 0.5hz	
#define c_3			Bbituse2.Bits.bit2//led operating disp

#define sleep_tag 		Bbituse2.Bits.bit4
#define led_disp_tag 	Bbituse2.Bits.bit5
#define charge_tag 		Bbituse2.Bits.bit6
#define chrg_full_tag Bbituse2.Bits.bit7

volatile Bytebits Bbituse3;
#define bituse3 Bbituse3.Byte
#define vin_tag 	Bbituse3.Bits.bit0
#define vdc_tag 	Bbituse3.Bits.bit1
#define pg_tag 		Bbituse3.Bits.bit2
#define pg_ack		Bbituse3.Bits.bit3

//next 3-bit effect to load op can't be changed
#define ot55_tag	Bbituse3.Bits.bit5//temperature over 55C
#define ot65_tag	Bbituse3.Bits.bit6//temperature over 65C
#define ut_tag 		Bbituse3.Bits.bit7//under temperature or ntc not connect

volatile Bytebits Bbituse4;//tag relate to bat. volt
#define bituse4 Bbituse4.Byte
#define lev1c_tag 		Bbituse4.Bits.bit0
#define lev2c_tag 		Bbituse4.Bits.bit1
#define lev3c_tag 		Bbituse4.Bits.bit2
#define levc_full_tag Bbituse4.Bits.bit3

#define lev1d_tag 		Bbituse4.Bits.bit4
#define lev2d_tag 		Bbituse4.Bits.bit5
#define lev3d_tag 		Bbituse4.Bits.bit6

volatile Bytebits Bbituse5;
#define bituse5 Bbituse5.Byte
#define uv_tag 		Bbituse5.Bits.bit0//under volt.
#define uvd_end 	Bbituse5.Bits.bit1//under volt. delay time end
#define uvpd_tag 	Bbituse5.Bits.bit2//under volt. power down tag

unsigned int intCount = 0;

//adc interval timer count
unsigned int intCount_adc = 0;

//key press variable
unsigned char t_kps = 0;//key press steady time
unsigned char t_krs = 0;//key relax steady time
//long press key variable
unsigned char t_klp = 0;//key long press last time

unsigned char s_led = 0;//led status
unsigned char t_sd = 0;//sleep delay time
unsigned char t_sud = 0;//start up delay time
unsigned char t_uvd = 0;//under volt. delay time

//adc
unsigned char t_adc_int = 0;//adc interval time

unsigned char vb_adc_count = 0;
unsigned int vb = 0;
unsigned int vb_sum = 0;

unsigned char vt_adc_count = 0;
unsigned int vt = 0;
unsigned int vt_sum = 0;

unsigned int lev_value = 0;

unsigned char pgh_count = 0;
unsigned char pgl_count = 0;

void cap_level(void);
void vt_check(void);

void gpio_init(void)
{
	//pa
	pa_odr=0;
	//pa_idr=0;
	pa_ddr=0xfc;//1=out,0=in	
	pa_cr1=0x3c;
	pa_cr2=0;
	
	//pb
	pb_odr =0;
	//pb_idr =0;
	pb_ddr =0xbe;	
	pb_cr1=0xc2;
	pb_cr2=0x00;
		
	//port b interrpt
	//EXTI_CR1 |= 0x28;//faling edge trigger
	//EXTI_CR3 = 0x02;//port b trigger by faling edge 
	//EXTI_CONF = 0x03;//PB[7:0] are used for EXTIB interrupt generation
	
	//pc
	pc_odr =0;
	//pc_idr =0;
	pc_ddr =0xf7;
	pc_cr1 =0x1c;
	pc_cr2 =0x08;	
	//port c interrpt
	//bit3 interrupt sensitivity= rising edge(=01)
	EXTI_CR1 |= (01 << 6);	
	
	//pd
	pd_odr = 0;
	//pd_idr = 0;
	pd_ddr = 0xb6;
	pd_cr1 = 0xc7;
	pd_cr2 = 0x41;
	//port d interrpt
	//bit0 interrupt sensitivity= falling edge(=10)
	EXTI_CR1 |= (10 << 0);
	//bit6 interrupt sensitivity= rising edge(=01)
	EXTI_CR2 |= (01 << 4);
}

void clock_init(void)
{
		//CLK_SWR = 0x01;//set HSI as the main clock souce
		CLK_ICKCR |= 0x01; //enable internal RC=16mhz
		while(!(CLK_ICKCR & 0x02)); //Wait for clock stabilization
		CLK_CKDIVR = 0x02;//set fmaster=16M/4, fcpu=fmaster	
	
		//set system clock to prepheral
		//1 = enable ,0=disable
		CLK_PCKENR1 = 0;
		CLK_PCKENR2 = 0;		
}

void wwdg_init(void)
{
		WWDG_WR = WINDOW_VALUE;
		WWDG_CR = COUNTER_INIT_VALUE;
		WWDG_CR |= 0x80;//ÆôÓÃWWDG
		//clrwdt();
}

void clrwdt(void)
{
		unsigned char counter_value = 0;
		counter_value = WWDG_CR & 0x7f;
		if(counter_value < WINDOW_VALUE)
				{
						WWDG_CR = COUNTER_INIT_VALUE;
				}
}

void tim4_init(void)
{
		CLK_PCKENR1 |= TIM4;//set system clock to peripheral tim4	
		TIM4_PSCR=0x07;//perscale=2^7 
		TIM4_ARR =0xe5;//counter overflow threshold
		TIM4_CNTR=0x00;//counter initialize
		TIM4_IER =0x01;//tim4 interupt enable
		TIM4_SR1 &= ~(1 << 0);//clear updata interrupt flag	
		TIM4_CR1 &= ~(1 << 0);//time4 count disable(TIM4_CR4.CEN)
}
void adc_init(void)
{
		CLK_PCKENR2 |= ADC1;//set system clock to peripheral adc1	
		ADC1_CR1 = 0;//12-bit resolution & single convertion mode
		ADC1_CR2 = 0x87;//PRESC=1;SMTP1=7;
		ADC1_CR3 = 0x7f;//SMTP2=7;
		ADC1_CR1 |= (1 << 0);//adc power on
}
void var_init(void)
{
		bituse = 0;
		bituse1 = 0;
		bituse2 = 0;
		bituse3 = 0;
		bituse4 = 0;
		bituse5 = 0;
		
		led1 = LED_OFF;
		led2 = LED_OFF;
		led3 = LED_OFF;
		led4 = LED_OFF;
		led0 = LED_OFF;
		
		sw_vin = 	IN_OFF;//vin off
		sw_dcin = IN_OFF;//dcin off
		sw_vo = VO_OFF;//output off
		sw_chrg = CHRG_OFF;//charge ic disable
		sw_vt = VT_ON;//ntc switch on
		sw_vb = VB_ON;//ntc switch on
		
		lev_value = LEV1D;
} 
void init(void)
{
		clock_init();
		wwdg_init();
		gpio_init();
		tim4_init();
		adc_init();
		var_init();
}
/*--------------delay_t
t=1//10us
t=10//40us
t=20//80us
t=30//120us
t=250//1ms
t=500//1.8ms
t>=5000//18ms
-------------------*/
void delay_t(unsigned int t)
{
		if(t > 5000)
				t = 5000;
		while(t--)
				clrwdt();//clear wdt.			
}

//set adc channel
void adc_channel(unsigned char adc_ch)
{
		ADC1_SQR1 = 0;
		ADC1_SQR2 = 0;
		ADC1_SQR3 = 0;
		ADC1_SQR4 = 0;
		if(adc_ch < 8)
				ADC1_SQR4 |= (1 << adc_ch);	
				else if(adc_ch < 16)
						ADC1_SQR3 |= (1 << (adc_ch-8));		
				else if(adc_ch < 24)
						ADC1_SQR2 |= (1 << (adc_ch-16));	
				else 
						ADC1_SQR1 |= (1 << (adc_ch-24));
}
unsigned int adc_data(void)
{
		unsigned char adc_hb = 0;
		unsigned char adc_lb = 0;
		unsigned int adc_value = 0;
		//delay
		{
				unsigned char i = 0; 				
				for(i = 0;i < 20;i++)
						_asm("nop");
		}
		//adc
		ADC1_CR1 |= (1 << 1);//start adc
		while(!(ADC1_SR & 0x01))
				clrwdt();
		adc_hb = ADC1_DRH;
		adc_lb = ADC1_DRL;
		adc_value = (unsigned int)(adc_hb << 8) + adc_lb;
		return adc_value;
}

void vb_adc(void)
{				
		//vb check
		if(vb_check_command)
				{
						vb_check_command = 0;
						vb_adc_count++;
						adc_channel(VB);//set channel
						vb_sum += adc_data();
						if(vb_adc_count >= 8)
								{
										vb = vb_sum >> 3;
										//reset
										vb_adc_count = 0;
										vb_sum = 0;
										vb_check_tag = 1;
										//switch to next channel
										adc_channel(VT);
								}
				}	

}

//temperature adc
void vt_adc(void)
{
		if(vt_check_command)
				{
						vt_check_command = 0;
						vt_adc_count++;
						adc_channel(VT);//set channel as VT
						vt_sum += adc_data();
						if(vt_adc_count >= 8)
								{
										vt = vt_sum >> 3;										
										//reset
										vt_adc_count = 0;
										vt_sum = 0;	
										
										adc_command = 0;
										vb_check_tag = 0;
										
										//switch to next channel
										adc_channel(VB);
								}
				}
	
}
//adc data init.
void adc_data_init(void)
{
		if(!vt_init_tag)
		{
				//vt fast check
				adc_channel(VT);
				delay_t(1500);
				vt = adc_data();				
				vt_init_tag = 1;
		}		
		if((!vb_init_tag) && (start_up_delay))
		{
				//vb fast check
				adc_channel(VB);
				delay_t(1500);
				vb = adc_data();
				vb_init_tag = 1;
		}
}
void key_check(void)
{		
		//key press check
		if(KEY_RELEASE == key)
				{
						//kps_tag reset
						kps_tag = 0;
						t_kps = 0;
						//key release ack.
						if(krs_tag)
								key_press_tag = 0;		
				}
				else 
						{
								//krs_tag reset
								krs_tag = 0;
								t_krs = 0;
								//key press ack.
								if(kps_tag)
									key_press_tag = 1;				
						}																	
				
		//key long press count
		if(klp_tag && (!klp_count_tag))
				{
						klp_count_tag = 1;
						vo_command = !vo_command;
				}
		//tag reset
		if(!key_press_tag)
				{
						//key long press tag reset
						klp_tag = 0;
						t_klp = 0;
						klp_count_tag = 0;
				}		
				
}

void charge_check(void)
{	
		//vin_tag
		if(vin)
				{
						delay_t(10);
								if(vin)
										vin_tag = 1;
				}else
						{
								delay_t(10);
										if(!vin)
												vin_tag = 0;
						}
		//vdc_tag
		if(vdc)
				{
						delay_t(10);
								if(vdc)
										vdc_tag = 1;
				}else
						{
								delay_t(10);
										if(!vdc)
												vdc_tag = 0;
						}
		//pg_tag				
		if(pg)
				{
						delay_t(10);
								if(pg)
										pg_tag = 1;
				}else
						{
								delay_t(10);
										if(!pg)
												pg_tag = 0;
						}						
				
		//charge check
		if(vin_tag || vdc_tag)
				charge_tag = 1;
				else charge_tag = 0;
		if(charge_tag && pg_ack)
				chrg_full_tag = 1;
				else chrg_full_tag = 0;		
}

/*------------------vb check.
cap.level: 
lev3 = 75%
lev2 = 50%
lev1 = 25%
-------------------------*/
void cap_level(void)
{	
		if(charge_tag)
				{
						if(((vb > VBF) || chrg_full_tag) || levc_full_tag)
								{
										s_led = 0x80;
										levc_full_tag = 1;
								}
								else if((vb > LEV3C) || lev3c_tag)//cap.>75%
										{
												s_led = 0x40;
												lev3c_tag = 1;
										}
								else if((vb >= LEV2C) || lev2c_tag)//50%<cap.<75%
										{
												s_led = 0x20;	
												lev2c_tag = 1;
										}
								else if((vb >= LEV1C) || lev1c_tag)//25%<cap.<50%
										{
												s_led = 0x10;
												lev1c_tag = 1;
										}
								else 	s_led = 0;

						//levc_full_tag reset		
						if(vb < VBFR)	
								levc_full_tag = 0;		
						//charge lev_tag reset
						if(vb < LEV3CR)
								lev3c_tag = 0;
						if(vb < LEV2CR)
								lev2c_tag = 0;
						if(vb < LEV1CR)
								lev1c_tag = 0;
										
						//discharge lev_tag reset
						if(vb >= LEV1C)
								lev1d_tag = 0;
						if(vb >= LEV2C)
								lev2d_tag = 0;
						if(vb >= LEV3C)
								lev3d_tag = 0;	

				}	
				else//if(!charge_tag)
						{
								if((vb < VB_LOW) || uv_tag)//under volt.
										{
												s_led = 0;
												uv_tag = 1;
										}
										else if((vb < LEV1D) || lev1d_tag)//cap.<25%
												{
														s_led = 0x10;	
														lev1d_tag = 1;
												}
										else if((vb < LEV2D) || lev2d_tag)//cap.=50-25%
												{
														s_led = 0x20;	
														lev2d_tag = 1;
												}											
										else if((vb < LEV3D) || lev3d_tag)//cap.=75-50%
												{
														s_led = 0x40;	
														lev3d_tag = 1;
												}
										else s_led = 0x80;

								//uv_tag reset		
								if(vb >= VB_LOWR)		
										uv_tag = 0;
								if(vb >= LEV1DR)
									lev1d_tag = 0;
								if(vb >= LEV2DR)
									lev2d_tag = 0;
								if(vb >= LEV3DR)
									lev3d_tag = 0;	
									
								//charge lev_tag reset
								if(vb < LEV3D)
										{
												levc_full_tag = 0;
												lev3c_tag = 0;
										}
								if(vb < LEV2D)
										{
												levc_full_tag = 0;
												lev3c_tag = 0;
												lev2c_tag = 0;	
										}
								if(vb < LEV1D)
										{
												levc_full_tag = 0;
												lev3c_tag = 0;
												lev2c_tag = 0;
												lev1c_tag = 0;
										}


						}
						
}
//temperture check
void vt_check(void)
{
		//over temp.
		if(vt <= VT55 )
				ot55_tag = 1;
		if(vt <= VT65)
				ot65_tag = 1;
		//under temp.
		if(vt >= VTNEG20)
				ut_tag = 1;
		//ot_tag reset
		if(vt > VT45)
				{
						ot55_tag = 0;
						ot65_tag = 0;
				}
		//ut_tag reset
		if(vt < VTNEG5)
				ut_tag = 0;
}

//operation
void op(void)
{
		//charge input control
		if((!ot55_tag) && (!ot65_tag) && (!ut_tag))
				{
						sw_chrg = CHRG_ON;
						if(vin_tag && vdc_tag) 
								{
										if(!dcin_command)
												{
														vin_command = 1;
														dcin_command = 0;
												}
								}
								else if(vin_tag)
										{
												vin_command = 1;
												dcin_command = 0;
										}
								else if(vdc_tag)
										{
												dcin_command = 1;
												vin_command = 0;
										}
								else
										{
												vin_command = 0;
												dcin_command = 0;
										}										
										
				}
				else
						{
								vin_command = 0;
								dcin_command = 0;	
								sw_chrg = CHRG_OFF;
						}	
		if(vin_command)
				sw_vin = IN_ON;
				else sw_vin = IN_OFF;
		if(dcin_command)
				sw_dcin = IN_ON;
				else sw_dcin = IN_OFF;				
		//output control
		if(vo_command && (!ot65_tag) && (!ut_tag) && (!uvd_end))
				sw_vo = VO_ON;
				else sw_vo = VO_OFF;
		if(uvd_end && (!uvpd_tag))	
				{
						vo_command = 0;
						uvpd_tag = 1;
				}
}

//loe power dissipation
void low_power_dissipation(void)
{
		//sleep
		if(sleep_tag)
				{			
						sw_chrg = CHRG_OFF;
						sw_vb = VB_OFF;
						sw_vt = VT_OFF;
						led0 = LED_OFF;
						
						TIM4_CR1 &= ~(1 << 0);//time4 count disable
						ADC1_CR1 &= ~(1 << 0);//adc power off	
										
						//close all prepherals
						CLK_PCKENR1 = 0;
						CLK_PCKENR2 = 0;	
		
						//sleep tag reset
						sleep_tag = 0;
						t_sd = 0;
						//sleep
						_asm("halt");
						_asm("nop");

						//connect system clock to prepheral
						CLK_PCKENR1 |= TIM4;//open tim4 clock
						CLK_PCKENR2 |= ADC1;//open adc1	clock
										
						TIM4_CR1 |= (1 << 0);//time4 count enable
						ADC1_CR1 |= (1 << 0);//adc power on
										
						sw_vb = VB_ON;
						sw_vt = VT_ON;
						//sw_chrg = CHRG_ON;
				}
}

@far @interrupt void tim4_isr(void)
{
		TIM4_SR1 &= ~(1 << 0);//clear updata interrupt flag
		intCount++;	
	
		//timer 1/4s
		if(0==(intCount & ONE_4_SECOND_COUNT_MASK))
		{	
				//last time of key long press 
				if(key_press_tag && (!klp_tag))
						{
								t_klp++;
								if(t_klp >= TIME_KLP)
										klp_tag = 1;
						}	
				
		}//end 1/4s timer		
		
		//timer 1/2s
		if(0==(intCount & ONE_2_SECOND_COUNT_MASK))
		{
				c_2 = !c_2;
		}//end 1/2s timer			
		
		//timer 1s 
		if(0 == (intCount & ONE_SECOND_COUNT_MASK))
		{	
		
				if(!start_up_delay)
						{
								t_sud++;
								if(t_sud >= TIME_START_UP_DELAY)
										start_up_delay = 1;				
						}
				
				//pg_ack
				if(charge_tag && pg_tag)
					{   
							//pg_ack set
							pgl_count = 0;
							if(!pg_ack)
									pgh_count++;
							if(pgh_count >= 2)
									pg_ack = 1;
					}
					else
							{
									//pg_ack reset
									pgh_count = 0;
									if(pg_ack)
											pgl_count++;
									if(pgl_count >= 10)
											pg_ack = 0;
									
							}
				//under volt. cut off delay
				if(uv_tag && (!uvd_end))
						{
								t_uvd++;
								if(t_uvd >= TIME_UVD)
										uvd_end = 1;
						}
				//uv_tag reset
				if((!uv_tag) || charge_tag) 
						{
								t_uvd = 0;
								uvd_end = 0;
						}
				if(uvpd_tag && vo_command)	
						{
								uvpd_tag = 0;
								t_uvd = 0;
								uvd_end = 0;
						}
				//sleep delay timer
				//if(key & (!sleep_tag))//for test
				if((!led_disp_tag) && (!adc_command) && (!sleep_tag))
						{
								t_sd++;
								if(t_sd >= TIME_SD)
										sleep_tag = 1;
						}
						
		}//end 1s timer

		//adc interval timer 
		if(adc_command)
				{
						intCount_adc++;
						if(0 == (intCount_adc & TIME_ADC_INT_MASK))
								{
										t_adc_int++;
										if(t_adc_int >= 2)
												t_adc_int = 0;
								}
				}else 
						{
								t_adc_int = 0;
								intCount_adc = 0;
						}
				
		//key steady timer	
		//key press steady timer
		if((KEY_PRESS_DOWN == key) && (0 == kps_tag))
				{
						t_kps++;
						if(t_kps >= TIME_KS)
								kps_tag = 1;	
				}
		//key relax steady timer
		if((KEY_RELEASE == key) && (0 == krs_tag))
				{
						t_krs++;
						if(t_krs >= TIME_KS)
								krs_tag = 1;	
				}	
				
		//adc command
		if(vb_init_tag && vb_init_tag)
				{
						if(0 == (intCount & TIME_ADC_MASK))
								adc_command = 1;
				}
		if(adc_command)
				{
						switch(t_adc_int)
						{
								case 0://vb check command
								{	
										if((0 == (intCount & TIME_VB_MASK)) && (!vb_check_tag))
												vb_check_command = 1;
								}break;
								case 1://vt check command
								{
										if(0 == (intCount & TIME_VT_MASK))
												vt_check_command = 1;
								}break;
								default:;
								break;
						}
				}
				
		//led flicker timer
		c_1 = LED_OFF;
		if((intCount & LED_FLICKER_COUNT_MASK) < LED_FLICKER_COUNT_END)
				c_1 = LED_ON;
		//led operating disp.
		c_3 = LED_OFF;
		if((intCount & LED_OP_DISP_COUNT_MASK) < LED_OP_DISP_COUNT_END)
				c_3 = LED_ON;				
		//led_disp
		if(led_disp_tag)
		{		
				if(ot55_tag || ot65_tag || ut_tag)
						{
								led4 = c_2;
								led3 = c_2;
								led2 = c_2;
								led1 = c_2;														
						}
				else{
				if((s_led & 0x80) != 0)
						{
								led4 = LED_ON;
								led3 = LED_ON;
								led2 = LED_ON;
								led1 = LED_ON;
						}
				else if((s_led & 0x40) != 0)//rsoc=75
						{
								if(charge_tag)
										led4 = c_1;
										else led4 = LED_OFF;
								led3 = LED_ON;
								led2 = LED_ON;
								led1 = LED_ON;								
						}
				else if((s_led & 0x20) != 0)//rsoc=50
						{
								led4 = LED_OFF;
								if(charge_tag)
										led3 = c_1;
										else led3 = LED_OFF;
								led2 = LED_ON;
								led1 = LED_ON;								
						}						
				else if((s_led & 0x10) != 0)//rsoc=25
						{
								led4 = LED_OFF;
								led3 = LED_OFF;
								if(charge_tag)
										led2 = c_1;
										else led2 = LED_OFF;
								led1 = LED_ON;								
						}	
				else //s_led=0
						{
								led4 = LED_OFF;
								led3 = LED_OFF;
								led2 = LED_OFF;
								//if(charge_tag)
										led1 = c_1;
										//else led1 = LED_ON;	
						}	
				}
		}else//if(!led_disp_tag)
				{
						led4 = LED_OFF;
						led3 = LED_OFF;
						led2 = LED_OFF;
						led1 = LED_OFF;						
				}
		
}//end tim4_isr

//external interrupt
//key int.
@far @interrupt void bit0_isr(void)
{
		EXTI_SR1 |= (1 << 0);
		_asm("nop");
		
		t_sd = 0;
		sleep_tag = 0;
		adc_command = 1;
}
//vin int.
@far @interrupt void bit3_isr(void)
{
		EXTI_SR1 |= (1 << 3);
		_asm("nop");
		
		t_sd = 0;
		sleep_tag = 0;
		adc_command = 1;		
}
//vdc int.
@far @interrupt void bit6_isr(void)
{
		EXTI_SR1 |= (1 << 6);
		_asm("nop");
		
		t_sd = 0;
		sleep_tag = 0;
		adc_command = 1;		
}

main()
{
		init();
		_asm("rim");//interrupt enable
		TIM4_CR1 |= (1 << 0);//enable tim4 count
		_asm("nop");
		delay_t(100);
		while (1)
		{
				clrwdt();	
				_asm("nop");
				//operating indicator
				led0 = c_3;
				adc_data_init();
				vb_adc();
				vt_adc();
				vt_check();
				cap_level();
				charge_check();					
				op();
				key_check();
				//led disp. tag
				//condition:vo_command/charge
				if(vo_command || charge_tag)
						led_disp_tag = 1;
						else led_disp_tag = 0;

				//sleep
				low_power_dissipation();	
			
		}//end while(1)
}