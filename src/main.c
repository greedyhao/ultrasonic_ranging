/**
 * @file main.c
 * @author greedyhao (hao_ke@163.com)
 * @brief 
 * @version 0.1
 * @date 2019-11-07
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#include <STC12C5A60S2.H>
#include <math.h>

/* macro */
#define sound_velocity 34300  	/* sound velocity in cm per second */

#define period_in_us pow(10,-6)
#define Clock_period 1.085*period_in_us		/* period for clock cycle of 8051*/

#define SEG_PORT	P1

sbit SEG_BIT0	= P2^3;
sbit SEG_BIT1	= P2^2;
sbit SEG_BIT2	= P2^1;
sbit SEG_BIT3	= P2^0;

sbit BEEP_PIN	= P2^4;
sbit TRIG_PIN	= P2^5;
sbit ECHO_PIN	= P2^6;

/* user define */
code unsigned char seg_table[]={0xc0,0xf9,0xa4,0xb0,0x99,0x92,0x82,0xf8,0x80,0x90,0x88,0x83,0xc6,0xa1,0x86,0x8e};
unsigned int num = 0;			/* the number display on 7-segment */
unsigned char time_out = 0;		/* ECHO port */
unsigned char beep_frq = 0;
unsigned char beep_flag = 0;	/* status of beep 0: no work 1: short beep 2: long beep */

/* static func */


/* public func */
void init_timer(void);
void delay_10us(void);
void delay_ms(unsigned char n);
void seg_dis(unsigned int num);
void uw_sen_start(void);

/* ------------- main ---------------- */
void main(void){
	unsigned long tmp = 0;
	float distance_measurement, value;
	P2 = 0x0f;
	P2M1 = 0x40; //0100 0000
	P2M0 = 0x30; //0011 0000
	init_timer();

	while (1)
	{
		uw_sen_start();
		time_out = 0;
		while (ECHO_PIN == 0 && time_out < 2);
		time_out = 0;
		TR0 = 1;
		ET1 = 0;
		while (ECHO_PIN == 1);
		TR0 = 0;

		/* calculate distance using timer */
		value = Clock_period * sound_velocity; 
		distance_measurement = (TL0|(TH0<<8));	/* read timer register for time count */
		distance_measurement = (distance_measurement*value)/2.0;  /* find distance(in cm) */
		
		num = (int)(distance_measurement*10)%10000;

		// if	 (num < 800) beep_frq = (num - 300)/25; /* (x - 300)/500*20  (level 20) */
		// else BEEP_PIN = 0;
		// if (num <= 300) beep_frq = 30;

		if (num < 800 && num > 300) beep_flag = 1;
		else if (num <= 300) beep_flag = 2;
		else beep_flag = 0;

		ET1 = 1;

		delay_ms(50);
	}
}

/* ------------ delay --------------- */
static void _delay_1ms(void)   //误差 -0.651041666667us
{
    unsigned char a,b;
    for(b=4;b>0;b--)
        for(a=113;a>0;a--);
}

static void _delay_10ms(void)		//@11.0592MHz
{
	unsigned char a,b;
    for(b=151;b>0;b--)
        for(a=29;a>0;a--);
}

void delay_ms(unsigned char n)   //误差 -0.000000000002us
{
	do
	{
		_delay_1ms();
	} while (--n);
}

/* ------------ ultrasonic --------------- */
void init_timer(void)
{
	TMOD	= 0x11;
	TF0		= 0;
	TR0 	= 0;

	TH1 = 0xB8;
    TL1 = 0x00;
    EA = 1;
    ET1 = 1;
    TR1 = 1;
}

void delay_10us(void)
{
	TL0=0xF7;
	TH0=0xFF;
	TR0=1;
	while (TF0!=1);
	TR0=0;
	TF0=0;
}

/**
 * @brief ultrasonic sensor start working
 * 
 */
void uw_sen_start(void)
{
	TRIG_PIN = 1;
	delay_10us();
	TRIG_PIN = 0;
}

/* ------------ segment --------------- */
void seg_dis(unsigned int num)
{
	unsigned char _bit0, _bit1, _bit2, _bit3;
	_bit0 = _bit1 = _bit2 = _bit3 = 0;

	_bit0 = num % 10;
	_bit1 = num / 10 % 100;
	_bit2 = num / 100 % 1000;
	_bit3 = num / 1000;

	P2 = P2 | 0x0f;
	SEG_BIT0 = 0;
	SEG_PORT = seg_table[_bit0%10];
	delay_ms(1);
	SEG_BIT0 = 1;
	// SEG_PORT = 0xff;

	SEG_BIT1 = 0;
	SEG_PORT = seg_table[_bit1%10]&0x7F;
	delay_ms(1);
	SEG_BIT1 = 1;
	// SEG_PORT = 0xff;

	SEG_BIT2 = 0;
	SEG_PORT = seg_table[_bit2%10];
	delay_ms(1);
	SEG_BIT2 = 1;
	// SEG_PORT = 0xff;

	SEG_BIT3 = 0;
	SEG_PORT = seg_table[_bit3%10];
	delay_ms(1);
	SEG_BIT3 = 1;
	// SEG_PORT = 0xff;
}

void Timer1Interrupt(void) interrupt 3
{
    TH1 = 0xB8;
    TL1 = 0x00;
    //add your code here!
	// --beep_frq;
	seg_dis(num);
	// if (beep_frq == 0) BEEP_PIN = !BEEP_PIN;
	// if (beep_frq >= 20)
	// {
	// 	beep_frq++;
	// 	BEEP_PIN = 1;
	// }

	switch (beep_flag)
	{
	case 0:
		BEEP_PIN = 0;
		break;
	case 1:
		BEEP_PIN = !BEEP_PIN;
		break;
	case 2:
		BEEP_PIN = 1;
		break;
	default:
		break;
	}

	time_out++;
}
