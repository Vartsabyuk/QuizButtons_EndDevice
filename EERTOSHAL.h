#ifndef EERTOSHAL_H
#define EERTOSHAL_H
#include <HAL.h>

//RTOS Config
#define RTOS_ISR  			TIMER0_COMPA_vect
#define	TaskQueueSize		32 //обязательно должны быть степенями 2
#define MainTimerQueueSize	16 //обязательно должны быть степенями 2

extern void RunRTOS (void);

#endif
