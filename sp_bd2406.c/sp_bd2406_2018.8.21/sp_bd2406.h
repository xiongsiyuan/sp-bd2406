/*------------------------------
program name:  sp_bd2406.h
perfect date：2018.7.11
mcu:stm8L151k4
schematic:sp-bd2406_r0331
-------------------------------*/ 
#ifndef _SP_BD2406_H_
#define _SP_BD2406_H_

		typedef union BYTEBITSDEF{
			unsigned char Byte;
			struct{
				unsigned char bit0:1;
				unsigned char bit1:1;
				unsigned char bit2:1;
				unsigned char bit3:1;
				unsigned char bit4:1;
				unsigned char bit5:1;
				unsigned char bit6:1;
				unsigned char bit7:1;
			} Bits;
		
} Bytebits;

#define led4 pa_odr_5
#define led3 pa_odr_4
#define led2 pa_odr_3
#define led1 pa_odr_2
#define led0 pc_odr_4//operating indicator

#define key 		pd_idr_0
#define sw_vo 	pd_odr_1//bat. output switch
#define sw_vt 	pd_odr_2//ntc switch
#define sw_vb 	pb_odr_1//vb switch
#define sw_chrg pb_odr_7//charge ic enable
#define pg 			pb_idr_6//charge status,1=full 

#define sw_dcin pd_odr_7//dcin switch
#define vdc 		pd_idr_6//vdc check
#define sw_vin 	pc_odr_2//vin switch
#define vin 		pc_idr_3//vin check

//adc channel
#define AN0 0
#define AN1 1
#define AN2 2
#define AN3 3
#define AN4 4
#define AN5 5
#define AN6 6
#define AN7 7

#define AN8 8
#define AN9 9
#define AN10 10
#define AN11 11
#define AN12 12
#define AN13 13
#define AN14 14
#define AN15 15

#define AN16 16
#define AN17 17
#define AN18 18
#define AN19 19
#define AN20 20
#define AN21 21
#define AN22 22
//#define AN23 23

//#define AN24 24
//#define AN25 25
//#define AN26 26
//#define AN27 27
#define AN28 28//Vrefint
#define AN29 29//Vts

#define VB AN18//PB0 
#define VT AN19//PD3

/*---------------wwdg value
WWDG计数器的计数周期=12288/4M = 3ms
counter_value-WINDOW_VALU = 7f-60 = 1f = 31
counter_value刷新周期 = 31*3ms = 93ms
---------------------------*/
#define WINDOW_VALUE 0x50
#define COUNTER_INIT_VALUE 0x7f

//user-defined status
#define KEY_PRESS_DOWN 0
#define KEY_RELEASE	1

#define LED_ON 	1
#define LED_OFF 0

#define VO_ON 	1
#define VO_OFF 	0

#define IN_ON 	1
#define IN_OFF 	0

#define CHRG_ON 	0
#define CHRG_OFF 	1 

#define VB_ON 	1
#define VB_OFF 	0

#define VT_ON 	0
#define VT_OFF 	1

//system clock to peripherals
//PCKEN1
#define TIM4 (1 << 2)
//PCKEN2
#define ADC1 (1 << 0)

//timer constant
#define ONE_SECOND_COUNT 128//1s
#define ONE_SECOND_COUNT_MASK ONE_SECOND_COUNT-1

#define ONE_2_SECOND_COUNT (128 >> 1)	//1s/(2^1) = 1/2 s
#define ONE_2_SECOND_COUNT_MASK ONE_2_SECOND_COUNT-1

#define ONE_4_SECOND_COUNT (128 >> 2)	//1s/(2^2) = 1/4 s
#define ONE_4_SECOND_COUNT_MASK ONE_4_SECOND_COUNT-1

#define ONE_8_SECOND_COUNT (128 >> 3)	//1s/(2^3) = 1/8 s
#define ONE_8_SECOND_COUNT_MASK ONE_8_SECOND_COUNT-1

#define ONE_16_SECOND_COUNT (128 >> 4)	//1s/(2^4) = 1/16 s
#define ONE_16_SECOND_COUNT_MASK ONE_16_SECOND_COUNT-1

//led filcker
#define LED_FLICKER_COUNT	128
#define LED_FLICKER_COUNT_MASK 	LED_FLICKER_COUNT-1
#define LED_FLICKER_COUNT_END	42
//led operating disp
#define LED_OP_DISP_COUNT (128 << 3)
#define LED_OP_DISP_COUNT_MASK 	LED_OP_DISP_COUNT-1
#define LED_OP_DISP_COUNT_END	3

//adc command
//adc interval time
#define TIME_ADC_INT (128 << 5)	//1s*(2^5) = 32 s
#define TIME_ADC_INT_MASK TIME_ADC_INT-1
//adc time
#define TIME_ADC (128 << 7)	//1s*(2^6) = 128 s
#define TIME_ADC_MASK TIME_ADC-1
//vt check time
#define TIME_VT (128 << 1)	//1s*(2^1) = 2 s
#define TIME_VT_MASK TIME_VT-1
//vb check time
#define TIME_VB (128 << 0)	//1s*(2^0) = 1 s
#define TIME_VB_MASK TIME_VB-1


//1= 8ms
#define TIME_KS 6//key steady time=6*8ms


//1= 1/4s
#define TIME_KLP 8//key long press last time

//1= 1s
#define TIME_SD 10//sleep delay time
#define TIME_START_UP_DELAY 5//start up delay
#define TIME_UVD 180// under volt. delay 

//temperature
#define VT45	0x0545//45C
#define VT55 	0x04b2//50C //0x042f//55C 
#define VT65 	0x034d//65C
#define VTNEG20 0x0e0a//-20C
#define VTNEG5 	0x0c7d//-5C

//bat. type select
#define LI

//bat. volt
#ifdef LI
//under volt.
#define VB_LOW 	0x07ee//3000mv
#define VB_LOWR 0x0875//3200mv
//full volt.
#define VBF 		0x0b03//4166mv
#define VBFR		0x0ac0//4066mv

//vb level
//under condition of discharge
//great power bat. data
#define LEV3D		0x0a55//3909mv
#define LEV2D		0x0987//3605mv
#define LEV1D		0x095c//3541mv
//release volt.
#define VRD 0x21//50mv
#define LEV3DR LEV3D-VRD
#define LEV2DR LEV2D-VRD
#define LEV1DR LEV1D-VRD

//under condition of charge
#define LEV3C		0x0a91//3996mv
#define LEV2C		0x09f8//3771mv
#define LEV1C		0x099f//3639mv
//release volt.
#define VRC 0x21//50mv
#define LEV3CR LEV3C-VRC
#define LEV2CR LEV2C-VRC
#define LEV1CR LEV1C-VRC
#endif


void clrwdt(void);
#endif