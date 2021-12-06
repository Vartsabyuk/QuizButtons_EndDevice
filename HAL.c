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

	/*Настройка прерывания ButtonIn*/
	PCICR |= 1<<PCIE0; //включаем прерывание по переключению пинов 0 группы (порт B) PCINT[7:0]
	PCMSK0 |= 1<<ButtonIn;  //включаем проверку пина ButtonIn 

	cbi(Button_DDR,ButtonIn);	//вход с кнопки на вход
	sbi(Button_DDR,ButtonOut);	//выход с кнопки на выход
	cbi(Button_PORT,ButtonIn);	// подтяжка на входе выключена, будет внешнее прижатие на землю
	cbi(Button_PORT,ButtonOut);	// выход на 0, по умолчанию прижат
	

}



