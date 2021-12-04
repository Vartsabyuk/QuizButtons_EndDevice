#include <EERTOSHAL.h>

//RTOS Запуск системного таймера
inline void RunRTOS (void)
{
	Disable_Interrupt
	TCCR0A = 1<<WGM01; 				        // Freq = CK/64 - Установить режим и предделитель
	TCCR0B = 3<<CS00;						// Автосброс после достижения регистра сравнения
	TCNT0  = 0;								// Установить начальное значение счётчиков
	OCR0A  = LO(TimerDivider); 				// Установить значение в регистр сравнения
	TIMSK0  = 0<<TOIE0|1<<OCIE0A;				// Разрешаем прерывание RTOS - запуск ОС
	Enable_Interrupt
}
