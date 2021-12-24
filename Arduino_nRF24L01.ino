#ifdef __cplusplus
	extern "C" {
#endif

//#define USART_debugging

#include <HAL.h>
#include <EERTOS.h>
#include <nRF24.h>
#include <avr/eeprom.h>

#ifdef USART_debugging
	#include <usart.h>
#endif

#ifdef __cplusplus
	}
#endif

//Команды для управления кнопками
#define activityButton 		0b01010101 // 0x55
#define notActivityButton 	0b10101010 // 0xAA
#define changeNumberButton	0b10011001 // 0x99


// Адреса EEPROM

u08 EEMEM buttonID_addr;

// Глобальные переменные =====================================================
u08 buttonID;
u08 TX_data[2] = {0x00, 0x01};
u08 pushButtonStatus = 0;

// Прототипы задач ===========================================================
void CheckButtonPush(void);
void CheckButtonPull(void);
void ButtonPush(void);
void ButtonPull(void);
void Sleep(void);
void parsing_nRF24(void);

#ifdef USART_debugging
	void parsingUART(void);
#endif

//============================================================================
//Верктора прерываний 
//============================================================================
#ifdef __cplusplus
	extern "C" {
#endif
//Прерывание изменения уровня пинов 0-й группы (порт B) PCINT[7:0]
	ISR (PCINT0_vect)
	{
		SetTask(CheckButtonPush);
		cbi(PCMSK0, ButtonIn); //выключить прерывание пина ButtonIn
	}
//Прерывание изменения уровня пинов 1-й группы (порт С) PCINT[14:8]
	ISR (PCINT1_vect)
	{
		nRF_IRQ_handler();
		SetTask(parsing_nRF24);
	}
//RTOS Interrupt
	ISR (RTOS_ISR)
	{
		TimerService();
	}
#ifdef USART_debugging
	//Прерывание по опустошению буффера USART
		ISR (USART_UDRE_vect)
		{
			USART_UDRE_Handler();
		}
		ISR (USART_RX_vect)
		{
			USART_RXC_Handler();
			SetTask(parsingUART);
		}
#endif
#ifdef __cplusplus
	}
#endif
//============================================================================
//Область задач
//============================================================================

void CheckButtonPush(void)
{
	pushButtonStatus <<= 1; //сдвиг влево на 1
	if (BitIsSet(Button_PIN,ButtonIn))	//если кнопка нажата
	{
		pushButtonStatus |= 1; //то записываем в конец 1
	} 
	if ((pushButtonStatus|0b11111000) == 0xFF) //если последние 3 бита равны 1 (XXXXX111) значит кнопка действительно нажата
	{
		#ifdef USART_debugging
			USART_SendStr("SENDING BYTE");
		#endif
		nRF_send_data(TX_data, 2);
		SetTask(CheckButtonPull);
	}
	else
	{
		SetTimerTask(CheckButtonPush,10);
	}
}

void CheckButtonPull(void)
{
	pushButtonStatus <<= 1; //сдвиг влево на 1
	if (BitIsSet(Button_PIN,ButtonIn))	//если кнопка нажата
	{
		pushButtonStatus |= 1; //то записываем в конец 1
	} 
	if ((pushButtonStatus&0b00000111) == 0x00) //если последние 3 бита равны 0 (XXXXX000) значит кнопка действительно отжата
	{
		sbi(PCMSK0, ButtonIn); //включить прерывание пина ButtonIn
	}
	else
	{
		SetTimerTask(CheckButtonPull,10);
	}
}

void ButtonPush(void)
{
	sbi(Button_PORT,ButtonOut); //отправляем на выход 1 (как бы нажимаем кнопку)
	SetTimerTask(ButtonPull,500);
}

void ButtonPull(void)
{
	cbi(Button_PORT,ButtonOut); //отправляем на выход 0 (как бы отпускаем кнопку)
	SetTask(Sleep);
}
void Sleep(void)
{
	if (HaveTasks())				//если есть еще невыполненные задачи
	{
		SetTimerTask(Sleep,100); 	//попробуем уснуть чуть позже
	}
	else							//иначе засыпаем
	{
		set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  		sleep_mode();
	}
}

#ifdef USART_debugging
	void parsingUART(void)
	{
		u08 temp;
		if ((temp = USART_GetChar()))
		{
			//USART_SendByte(temp);
			if (temp == 's')
			{
				USART_SendStr("STATUS REG: ");
				USART_PutChar(ReadReg(STATUS));
			}
			else if (temp == 'c')
			{
				USART_SendStr("CONFIG REG: ");
				USART_PutChar(ReadReg(CONFIG));
			}
			else if (temp == 'b')
			{
				USART_SendStr("SENDING ");
	 			nRF_send_data(TX_data, 2);
			}
		}
	}
#endif

void parsing_nRF24(void)
{
	u08 RX_data[2];
	nRF_get_data(RX_data);
	if (RX_data[0])
	{
		#ifdef USART_debugging
			USART_SendStr("RECEIVING BYTE: ");
			USART_SendNum(RX_data[0]);
		#endif
		if (RX_data[0] == activityButton) //если можно нажимать кнопку
		{
			SetTask(ButtonPush); //запускаем обработчик нажатия кнопки
		}
		else if (RX_data[0] == changeNumberButton) //если это команда смены номера кнопки
		{
			buttonID = RX_data[1]; 		//меняем номер
			TX_data[0] = buttonID;
			eeprom_update_byte(&buttonID_addr, buttonID); //перезаписываем это значение в EEPROM
			eeprom_busy_wait();
			SetTask(Sleep); // и уходим дальше спать
		}
		else
		{
			SetTask(Sleep); //а иначе уходим спать
		}
		TX_data[1]++;
	}
	else
	{
		SetTask(Sleep); //если ничего не словили из космоса, ну чтож тоже спать
	}
}
//==============================================================================
int main(void)
{
	//Инициалищация переменных из EEPROM
	buttonID = eeprom_read_byte(&buttonID_addr);
	TX_data[0] = buttonID;
	//---------------------------------------------
	InitAll();			// Инициализируем периферию

	#ifdef USART_debugging
		USART_Init();		// Инициализация USART
	#endif

	nRF_init();			// Инициализация nRF24L01
	InitRTOS();			// Инициализируем ядро
	RunRTOS();			// Старт ядра. 
	// Запуск фоновых задач.
	SetTask(Sleep);
	while(1) 		// Главный цикл диспетчера
	{
		wdt_reset();	// Сброс собачьего таймера
		TaskManager();	// Вызов диспетчера
	}

	return 0;
}
