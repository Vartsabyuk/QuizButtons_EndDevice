#ifndef HAL_H
#define HAL_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avrlibtypes.h>
#include "avrlibdefs.h"
#include "avrlibtypes.h"
#include <avr/pgmspace.h>
#include <avr/wdt.h>

#define STATUS_REG 			SREG
#define Interrupt_Flag		SREG_I
#define Disable_Interrupt	cli();
#define Enable_Interrupt	sei();
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)

//--макросы для удобства--
#define BitIsClear(reg, bit)    ((reg & (1<<(bit))) == 0)
//пример: if (BitIsClear(PORTB,1)) {...} //если бит очищен
#define BitIsSet(reg, bit)       ((reg & (1<<(bit))) != 0)
//пример: if(BitIsSet(PORTB,2)) {...} //если бит установлен

//Clock Config
#define F_CPU 8000000L

//System Timer Config
#define Prescaler	  		64
#define	TimerDivider  		(F_CPU/Prescaler/1000)		// 1 mS


//USART Config
#define baudrate 9600L
#define bauddivider ((F_CPU+baudrate*8)/(baudrate*16)-1)


//PORT Defines
//#define LED1 		4
//#define LED2		5
//#define LED3		6

//#define LED_PORT 	PORTD
//#define LED_DDR	DDRD

#define ButtonIn 	0
#define ButtonOut 	1

#define Button_PORT PORTB
#define Button_DDR  DDRB
#define Button_PIN 	PINB


extern void InitAll(void);



#endif
