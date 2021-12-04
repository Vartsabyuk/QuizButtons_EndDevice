#include <HAL.h>

inline void InitAll(void)
{

	//InitPort
	/*DDRB = 0b00000000; //все выводы на вход
	DDRC = 0b00000000; 
	DDRD = 0b00000000; 

	PORTB = 0b11111111; //на всех выводах подтяжка
	PORTC = 0b11111111;
	PORTD = 0b11111111;*/

	//LED_DDR |= 1<<LED1|1<<LED2|1<<LED3;
	Button_DDR |= 1<<ButtonOut; //выход с кнопки на выход
	cbi(Button_DDR,ButtonIn);	//вход с кнопки на вход
	sbi(Button_DDR,ButtonOut);	//выход с кнопки на выход
	cbi(Button_PORT,ButtonIn);	// подтяжка на входе выключена, будет внешнее прижатие на землю
	cbi(Button_PORT,ButtonOut);	// выход на 0, по умолчанию прижат
	//InitUSART
	//UBRR0L = LO(bauddivider);
	//UBRR0H = HI(bauddivider);
	//UCSR0A = 0;
	//UCSR0B = 1<<RXEN0|1<<TXEN0|0<<RXCIE0|0<<TXCIE0;
	//UCSR0C = 1<<UCSZ00|1<<UCSZ01;

	


}



