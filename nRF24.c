#include <nRF24.h>

//приемный буфер
u08 nRF_RX_buf = 0;

void nRF_init(void)						
{
	/*НАСТРОЙКА ПОРТОВ*/
	spi_DDR &=~(1<<MISO);				// MISO на вход
	spi_DDR |= 1<<SCK|1<<MOSI|1<<SS;	// все остальное на выход
	spi_PORT &=~(1<<SCK|1<<MOSI);		// SCK и MOSI в 0
	spi_PORT |= 1<<MISO|1<<SS;			// SS в 1, а для MISO включена подтяжка
	cbi(DDR_IRQ,IRQ);					// IRQ на вход
	sbi(DDR_CE,CE);						// CE на выход
	sbi(PORT_IRQ,IRQ);					// Подтяжка для IRQ по умолчанию всегда поднят
	cbi(PORT_CE,CE);					// CE на 0, по умолчанию прижат

	/*Настройка прерывания IRQ*/
	PCICR |= 1<<PCIE1; //включаем прерывание по переключению пинов 1 группы (порт С) PCINT[14:8]
	PCMSK1 |= 1<<IRQ;  //включаем проверку пина IRQ 

	/*Настройка SPI*/ 
	SPCR = (0<<SPIE)|(1<<SPE)|(0<<DORD)|(1<<MSTR)|(0<<CPOL)|(0<<CPHA)|(0<<SPR1)|(0<<SPR0);
        
	  /*    SPIE - разрешение прерывания от SPI, 1-ON,0-OFF
	        SPE  - вкл/выкл SPI,                 1-ON,0-OFF
	        DORD - порядок передачи данных.      1-байт передается с младшего бита, 0->7
	                                             0-байт передается со старшего бита.7->0                       
	        MSTR - Выбор режима (Master/Slave)   1-Master, 0-Slave
	        CPOL - полярность тактового сигнала  0-импульсы положительной полярности
	                                             1-импульсы отрицательной полярности
	        CPHA - Фаза тактового сигнала        0-биты считываются по переднему фронту ноги SCK
	                                             1-биты считываются по заднему фронту ноги SCK
	        SPR1 - Скорость передачи
	        SPR0 - Скорость передачи
	        
	        SPI2X  SPR1  SPR0   Частота SCK
	          0     0     0           F_CPU/4
	          0     0     1           F_CPU/16
	          0     1     0           F_CPU/64
	          0     1     1           F_CPU/128
	          1     0     0           F_CPU/2
	          1     0     1           F_CPU/8
	          1     1     0           F_CPU/32
	          1     1     1           F_CPU/64
	        */
                                  
	SPSR = (0<<SPIF)|(0<<WCOL)|(1<<SPI2X);

	  /*    SPIF  - Флаг прерывания от SPI. уст в 1 по окончании передачи байта
	        WCOL  - Флаг конфликта записи. Уст в 1 если байт не передан, а уже попытка записать новый.
	        SPI2X - Удвоение скорости обмена.*/

	//Настроим радиомодуль nRF на прием.
    WriteReg(CONFIG,(1<<PWR_UP)|(1<<EN_CRC)|(0<<PRIM_RX));
    _delay_ms(2);
    WriteReg(RX_PW_P0,1);//размер поля данных 1 байт.
	RXmod();//на прием
	
}

u08 ReadReg(u08 addr)
{
	cbi(spi_PORT,SS); 			//Прижимаем вывод CSN(SS) МК к земле, тем самым сообщаем о начале обмена данных.
	SPDR = addr; 				//Закидываем адрес регистра
	while(BitIsClear(SPSR,SPIF)); 	//ожидаем когда освободится SPI 
	if (addr != STATUS)			//Если проверяем не статус
	{
		SPDR = NOP; 					//Закидываем пустышку чтобы вытащить еще байт
		while(BitIsClear(SPSR,SPIF)); 	//ожидаем когда освободится SPI 
	}
	sbi(spi_PORT,SS); 			//Вывод CSN(SS) МК к питанию, обмен данных завершен.
	return SPDR;
}

void WriteReg(u08 addr,u08 byte)	
{
    cbi(spi_PORT,SS); 			//Прижимаем вывод CSN(SS) МК к земле, тем самым сообщаем о начале обмена данных.
    SPDR = addr | W_REGISTER;	//накладываем маску записи на адрес и засылаем;
    while(BitIsClear(SPSR,SPIF)); 	//ожидаем когда освободится SPI
    SPDR = byte; 				//отправляем непосредственно байт
    while(BitIsClear(SPSR,SPIF));	//ожидаем когда освободится SPI
    addr = SPDR;				//это для сброса флага SPIF
    sbi(spi_PORT,SS); 			//Вывод CSN(SS) МК к питанию, обмен данных завершен.
}

void RXmod(void)//Настроим nRF на прием.
{
    WriteReg(CONFIG,(1<<PWR_UP)|(1<<EN_CRC)|(1<<PRIM_RX));
    sbi(PORT_CE,CE);
    //_delay_us(135);
    //режим приема RX
}
void TXmod(void)//Настроим nRF на передачу и сразу же отправим байт в пространство.
{
    cbi(PORT_CE,CE);
    WriteReg(CONFIG,(1<<PWR_UP)|(1<<EN_CRC)|(0<<PRIM_RX));
    sbi(PORT_CE,CE);
    _delay_us(15);
    cbi(PORT_CE,CE);
    //_delay_us(135);
}
void nRF_send_byte(u08 byte)//отправка байта.
{
    WriteReg(W_TX_PAYLOAD, byte);//запись байта в буфер TX для отправки
    TXmod();//передача байта
    //while (BitIsSet(PIN_IRQ,IRQ));//Ждем пока байт не передан
    //u08 temp = ReadReg(STATUS);//прочитали статус регистр
    //WriteReg(STATUS, temp);//сброс флагов прерываний - обязательно
    //RXmod();//на прием
}
void nRF_IRQ_handler(void)
{
	if (BitIsClear(PIN_IRQ,IRQ))  //если ножка IRQ прижата
	{
		u08 status = ReadReg(STATUS);//прочитали статус регистр
		WriteReg(STATUS, status);//сброс флагов прерываний - обязательно
		if (BitIsSet(status,RX_DR)) //если этот бит равен 1 то байт принят.
		{
			nRF_RX_buf = ReadReg(R_RX_PAYLOAD); //чтение 1 байта из буфера приема RX_FIFO в озу buff
		}
		else if (BitIsSet(status,TX_DS)) //если этот бит равен 1 то байт передан.)
		{
			RXmod();//переключаемся обратно на прием
		}
	}
}
u08 nRF_get_byte(void)
{
	u08 temp = nRF_RX_buf;
	nRF_RX_buf = 0;
	return temp;
}