#ifdef __cplusplus
	extern "C" {
#endif

#include <HAL.h>
#include <EERTOS.h>
#include <usart.h>
#include <nRF24.h>

#ifdef __cplusplus
	}
#endif

#define buttonID 0b00000011
// Глобальные переменные =====================================================

u08 TX_data[2] = {buttonID, 0x01};
u08 pushButtonStatus = 0;

// Прототипы задач ===========================================================
void CheckButtonPush(void);
void CheckButtonPull(void);
void ButtonPush(void);
void ButtonPull(void);
void Sleep(void);
void parsingUART(void);
void parsing_nRF24(void);
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
		USART_SendStr("SENDING BYTE");
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

void parsing_nRF24(void)
{
	u08 temp;
	if ((temp = nRF_get_byte()))
	{
		USART_SendStr("RECEIVING BYTE: ");
		USART_SendNum(temp);
		if (temp == 0x55)
		{
			SetTask(ButtonPush); //запускаем обработчик нажатия кнопки
		}
		else
		{
			SetTask(Sleep); //а иначе уходим спать
		}
		TX_data[1]++;
	}
}
//==============================================================================
int main(void)
{
	InitAll();			// Инициализируем периферию
	USART_Init();		// Инициализация USART
	nRF_init();			// Инициализация nRF24L01
	InitRTOS();			// Инициализируем ядро
	RunRTOS();			// Старт ядра. 
	// Запуск фоновых задач.
	//SetTask(CheckButtonPush);
	SetTask(Sleep);
	while(1) 		// Главный цикл диспетчера
	{
		wdt_reset();	// Сброс собачьего таймера
		TaskManager();	// Вызов диспетчера
	}

	return 0;
}
