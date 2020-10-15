# SIGSENSE
Produto para monitoramento de tempo de máquina ligada pela sua vibração.

# TESTES EM ANDAMENTO

Em app_state_machine->enter e exit (check_config e configuration), tem uma rotina de 
mudança de clock

***
# BUGS CONHECIDOS
1. Em rtc-board.c, na função 
static uint64_t RtcGetCalendarValue( RTC_DateTypeDef* date, RTC_TimeTypeDef* time )
foi necessário colocar um timeout dentro do "do{}while(), pois existiam situações que isso estava agarrando o programa.


# DETALHES DO FIRMWARE

## PINOUT LORA
### SPI -> SPI_1  (10MHz)
Descrição        | Pino
---------------- | ---------
MOSI             | PA7
MISO             | PA6
SCK              | PB3
NSS              | PA15

### DIO -> DIO0 .. DIO5
Descrição        | Pino
---------------- | ---------
DIO0             | PB4
DIO1             | PB1
DIO2             | PB0
DIO3             | PC13
NA               | PA5  [DIO4] --> desabilitada no fw
NA               | PA4  [DIO5] --> desabilitada no fw

### Radio (output)
Descrição                 | Pino
------------------------- | ---------
RADIO_TCXO_POWER          | PA12
RADIO_ANT_SWITCH_RX       | PA1
RADIO_ANT_SWITCH_TX_BOOST | PC1
RADIO_ANT_SWITCH_TX_RFO   | PC2
RADIO_RESET               | PC0
 
![DIO Mapping](/images/Semtech_DIO_Mapping.png)
 
## PINOUT USART1 -> BLE

Descrição        | Pino
---------------- | ---------
RX               | PA10
TX               | PA9


## LED

Descrição        | Pino
---------------- | ---------
LED              | PB5

## ACELEROMETRO E SENSOR TEMPERATURA/UMIDADE SI70 -> I2C1
Descrição        | Pino
---------------- | ---------
INTERRUPÇÃO XL   | PA0
INTERRRUPÇÃO MAG | PB2
SCL              | PB8
SDA              | PB9

## HEADER P1 -> FUNCÕES ALTERNATIVAS (USADOS PARA PROGRAMAR O uC)
Descrição        | Pino
---------------- | ---------
LPUART1_TX (HEADER PINO 2) | PA14
LPUART1_RX (HEADER PINO 4) | PA13

## HEADER P2 -> FUNCÕES ALTERNATIVAS (USADOS PARA PROGRAMAR O SPBTLE-1s)

## HEADER P3 -> FUNCÕES ALTERNATIVAS
Posição Header   				| Pino  |   Obs
---------------- 				| -----	| ----------------
VCC (HEADER PINO 1) 			| 		|  
--- (HEADER PINO 3)          	| PA5   | [influencia no dio irq4, mas DIO4 Lora está desabilitado]
--- (HEADER PINO 5)          	| PA8  	|
GND (HEADER PINO 7) 			| 		|  
PULSOS2_IN (HEADER PINO 2)     	| PB15 	| --SPIMOSI
PULSOS1_IN (HEADER PINO 4)     	| PB14 	| --SPIMISO
OneWire2 (HEADER PINO 6)        | PB13 	| --SPI_SCK
OneWire1 (HEADER PINO 8)        | PB12 	| --SPI_NSS

Obs: Os pinos PB13 e PB14 são de RADIO_DEGUG, são definidos em board-config.h, mas não estão sendo utilizados.

## HEADER P4 -> FUNCÕES ALTERNATIVAS
Descrição        | Pino
---------------- | ---------
USART_TX (ALTERNATE FUNCTION) | PB6 (HEADER PINO 2)
USART_RX (ALTERNATE FUNCTION) | PB7 (HEADER PINO 4)
----                      	  | PA4 (HEADER PINO 6)
----                          | PA3 (HEADER PINO 8)		
VCC                           | PINO 1
LED                           | PB5 (PINO 3)
VBAT                          | PINO 5             
GND                           | PINO 7
  
## BLE -> USART1, SPI2 E IO
Descrição        | Pino
---------------- | ---------
USART1_TX        | PA10 
USART1_RX        | PA9
SPI_MOSI         | PB15 usado para --> pulse2 
SPI_MISO         | PB14 usado para --> pulse1
SPI_SCK          | PB13
SPI_NSS          | PB12
BLE_RST          | PA11
BLE_WKP          | PA8
BATERIA INTERNA  | PA2
		
			
## INTERRUPÇÕES USADAS
DIO2             | PB0
// Desabilitada -> INTERRUPÇÃO XL   | PA0
DIO1             | PB1
INTERRRUPÇÃO MAG | PB2
INTERRUPÇÃO XL2  | PA3
DIO0             | PB4
DIO3             | PC13
PULSOS1_IN       | PB14
PULSOS2_IN       | PB15
			
Esta versão está funcional e não apresentou travamento do rádio durante as transmissões
O consumo medido está em 0,350mA de média. 
Problemas detectados: 
1. Consumo ainda elevado para os requisitos exigidos (0,250mAh)

2. Necessário aumentar o controle da sensibilidade


  
***
TODO:  ajustar toda explicação da parte da comunicação de dados. 
       os dados abaixo foram copiados do produto Sigpower. 
       
****       
       
# COMUNICACAO DE DADOS

## HEADER UPLINK
* Bits 0 a 3 : Tipo do frame.
* Bits 4 a 5: Contador de frames. Incrementa sempre que uma nova mensagem é gerada pelo dispositivo. Utilizado para diferenciar mensagens consecutivas geradas pelo dispositivo.
* Bit 6: Flag de estado da bateria.
	* 0 = Nível baixo de bateria;
	* 1 = Nível bom de bateria.
* Bit 7: Flag de sensor de porta.
	* 0 = Porta fechada;
	* 1 = Porta aberta.

## HEADER_2 UPLINK

Byte   | Bits   | bit len |   Nome  | Descrição
-------| -----  | ------- | --------| ----------------
0      | 0 - 2  |    3    |  Frame  | Código do frame 
0      | 1      |    1    |  Flag1  | Flag de estado da bateria. 
0      | 4      |    1    |  Ciclo1 | Ciclo do contador 1 
0      | 5      |    1    |  Ciclo2 | Ciclo do contador 2 
0      | 6      |    1    |  Ciclo3 | Ciclo do machine on e time
0      | 7      |    1    |  Ciclo4 | Ciclo do timer on



## FRAME UPLINK
Byte 0: Header Uplink. Frame tipo 0.
Byte 1 e 2: Quantidade de aberturas de porta.
Byte 3 e 4: Quantidade de eventos de tensão.
Byte 5 e 6: Valor medido de temperatura (formato TEMP2).
Byte 7: Estado atual da detecção de tensão.
	* 0 = Normal;
	* 1 = Sobretensão;
	* 2 = Subtensão;
	* 3 = Sem tensão. (Valores menores que 20V)
* Byte 8: Valor medido da bateria [dV].
* Bytes 9 a 11: Valor analógico de tensão.


## UPLINK: KEEP ALIVE (2)

Byte  | Nome   | Descrição
----- | -------| ----------------
0     | Header | Bitfield header uplink
1     | Cod    | Código do report
2 - 11| Valor  | Valor do report   


## UPLINK: REPORT FRAME (4)

Byte  | Nome   | Descrição
----- | -------| ----------------
0     | Header | Bitfield header uplink
1     | Cod    | Código do report
2 - 11| Valor  | Valor do report    


## UPLINK: INFO FRAME - HORIMETRO (5)


Byte  | Nome   | Descrição
----- | -------| ----------------
0     | Header | Bitfield header uplink
1     | Temp   | Valor da temperatura -127°C a +128°C
2     | Bat    | Valor da bateria (x100mV). Valores de 0 a 25.5V   
3 - 5 | Tstamp | Timestamp minutos desde 00:00:00 01/01/2017
6 - 8 | Tempo  | Tempo acumulado de máquina ligada  
9 - 11| Counter| Quantidade de vezes que a máquina foi ligada 

## UPLINK: INFO FRAME - HORIMETRO RAW DATA (6)


Byte  | Nome   | Descrição
----- | -------| ----------------
0     | Header | Bitfield header uplink
1     | MSB_x  | MSB da medida bruta do sensor acelerômetro no eixo X
2     | LSB_x  | LSB da medida bruta do sensor acelerômetro no eixo X   
3     | MSB_y  | MSB da medida bruta do sensor acelerômetro no eixo Y
4     | LSB_y  | LSB da medida bruta do sensor acelerômetro no eixo Y   
5     | MSB_z  | MSB da medida bruta do sensor acelerômetro no eixo Z
6     | LSB_z  | LSB da medida bruta do sensor acelerômetro no eixo Z
7 - 8 | Counter| Quantidade de vezes que a máquina foi ligada
9 - 11| Tempo  | Tempo acumulado de máquina ligada

## UPLINK: INFO FRAME - HORIMETRO & PULSIMETRO (7)

Byte   | Bits   | bit len |   Nome   | Descrição
-------| -----  | ------- | -------- | ----------------
0      | 0 - 7  |    8    | Header_2 | Bitfield header 2 uplink
1      | 8 - 10 |    3    | Flags In | Flags indicando o estado atual das entradas (2,1,0) =(fmon,fc2,fc1)
1 - 3  | 11 - 27|   17    | Contador1| 0  a 131072 pulsos
3 - 5  | 28 - 44|   17    | Contador2| 0  a 131072 pulsos
5 - 7  | 45 - 57|   13    | MachineOn| 0 a 16384 vezes ligada
7 - 8  | 58 - 68|   11    | Temp1    | (Temperatura + 56) * 10. Range -56 a 128.0C
8 - 9  | 69 - 79|   11    | Temp2    | (Temperatura +56 ) * 10. Range -56 a 128.0C
10 - 11| 80 - 95|   16    | TimeOn   | 0 a 65536 minutos




## HEADER DOWNLINK
* Bits 0 a 2 : Tipo do frame.
* Bit 3: Flag configuração pendente no servidor. 
	0 = sem configuração pendente
	1 = existem configurações no servidor para downlink
* Bit 4: reservado
* Bit 5: reset do contador 1
	0 = não reinicie o contador
	1 = zere o contador 
* Bit 6: reset do contador 2
	0 = não reinicie o contador
	1 = zere o contador 
* Bit 7: reset do machine_on e do timer_on
	0 = não reinicie o contador
	1 = zere o contador 

## HEADER_2 DOWNLINK

Byte   | Bits   | bit len |   Nome  | Descrição
-------| -----  | ------- | --------| ----------------
0      | 0 - 2  |    3    |  Frame  | Código do frame 
0      | 1      |    1    |  Flag1  | Flag de estado da bateria. 
0      | 4      |    1    |  res    | Reservado 
0      | 5      |    1    | reset_c1| Flag para reset do counter1 
0      | 6      |    1    | reset_c2| Flag para reset do counter2
0      | 7      |    1    |reset_ton| Flag para reset do machine on e do timer on


## FRAME DOWNLINK
## DAYLI UPDATE
Byte   |  Nome    | Descrição
-------| -------- | ----------
0      | Header   | Downlink header
1 - 3  |Timestamp | Timestamp desde 00:00:00 01/01/2017 em minutos 
4 - 7  | Config   | Working hour bitfield

***



# CONFIGURACOES FIRMWARE-HARDWARE

## DEVICE LORAWAN IDS
O objetivo é utilizar o identificador único do dispositivo para gerar os parâmetros: Device EUI e Device Address. 
O STM32 possui um identificador único composto por 12 bytes (3 variáveis de 32 bits). No firmware propões algorítimo para combinar e reduzir este indetificador em um vetor de 8 bytes. O vetor completo será usado como **Device EUI** e os 4 bytes menos significativos deste vetor, usados para formar a variável de 32 bits usada como **Device Address**.

Abaixo a composição do vetor:

UID: | 31 34 35 33 | 31 31 47 0B | 00 23 00 38 |

*	''31'' = 0x31 (bits 95-88)
*	''57'' = 0x34 (bits 87-80) + 0x23 (bits 23-16)
*	''35'' = 0x35 (bits 79-72)
*	''6B'' = 0x33 (bits 71-64) + 0x38 (bits 7-0)
*	''31'' = 0x31 (bits 63-56)
*	''78'' = 0x31 (bits 55-48) + 0x47 (bits 47-40)
*	''0B'' = 0x0B (bits 39-32)
*	''00'' = 0x00 (bits 31-24)

Device EUI: 0x31, 0x57, 0x35, 0x6B, 0x31, 0x78, 0x0B, 0x00

Device Addr: 0x31780B00

## CANAL ANALOGICO
Com objetivo de utilizar o pino PA5 como entrada analógica pra medição de tensão.

O pino PA5 é utilizado pela biblioteca LoRaMac em duas situações:

1. Configurado como LED2 em correspondÃªncia com o hardware de desenvolvimento B-L072-LRWAN1.
2. Configurado como possibilidade de pino para entrada de sinal de interrupÃ§Ã£o vinda do rÃ¡dio SX1276, aparentemente sem conexÃ£o interna.

Aparentemente, no datasheet esse pino é usado como funcionalidade do módulo quando no modo Sigfox para monitorar a IO DIO4.
Conclui-se que este pino não tem influência na biblioteca LoRa exceto quando utilizado como fonte de interrupção externa. 

No firmware atual do SigPower estão comentados:

* As definições pré-existentes de LED2 e LED_RED1 em board-config.h;
* As linhas 105, 115 e 127 na biblioteca sx1276-board.c Funções afetadas: 
	* void SX1276IoInit(void);
	* void SX1276IoIrqInit(DioIrqHandler * irqHandlers);
	* void SX1276IoDeInit(void);

***