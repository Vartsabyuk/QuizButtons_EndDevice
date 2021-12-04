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
#define delayButtonPush 5000 //задержка после нажатия кнопки в мс


// Глобальные переменные =====================================================

u08 TX_data[2] = {buttonID, 0xF0};
u08 pushButtonStatus = 0;


// Прототипы задач ===========================================================
void CheckButtonPush(void);
void ButtonPush(void);
void ButtonPull(void);
void parsingUART(void);
void parsing_nRF24(void);
//============================================================================
//Верктора прерываний 
//============================================================================
#ifdef __cplusplus
	extern "C" {
#endif
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

void CheckButtonPush (void)
{
	SetTimerTask(CheckButtonPush,10); //каждые 10 мс проверяем нажата ли кнопка
	pushButtonStatus <<= 1; //сдвиг влево на 1
	if (BitIsSet(Button_PIN,ButtonIn))	//если кнопка нажата
	{
		pushButtonStatus |= 1; //то записываем в конец 1
	} 
	if ((pushButtonStatus|0b11111000) == 0xFF) //если последние 3 бита равны 1 (XXXXX111) значит кнопка действительно нажата
	{
		SetTask(ButtonPush); //запускаем обработчик нажатия кнопки
		SetTimerTask(CheckButtonPush, delayButtonPush); // в следующий раз будем проверять кнопку через 3с
	}
}

void ButtonPush(void)
{
	sbi(Button_PORT,ButtonOut); //отправляем на выход 1 (как бы нажимаем кнопку)
	SetTimerTask(ButtonPull,500);
	USART_SendStr("SENDING BYTE");
	nRF_send_data(TX_data, 2);
}

void ButtonPull(void)
{
	cbi(Button_PORT,ButtonOut); //отправляем на выход 0 (как бы отпускаем кнопку)
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
			USART_SendStr("SENDING BYTE");	
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
		SetTimerTask(CheckButtonPush, delayButtonPush); // в следующий раз будем проверять кнопку через 3с
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
	SetTask(CheckButtonPush);

	while(1) 		// Главный цикл диспетчера
	{
		wdt_reset();	// Сброс собачьего таймера
		TaskManager();	// Вызов диспетчера
	}

	return 0;
}
