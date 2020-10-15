# SIGSENSE
Produto para monitoramento de tempo de máquina ligada pela sua vibração.

***

# CADERNO DE TESTES DO FIRMWARE e HARDWARE
Testes a serem realizados no firmware e no hardware: 

1. Consumo: 
1.1 Medir o consumo médio do dispositivo durante o work time (210uA)
1.2 Medir o consumo médio do dispositivo fora do work time 
1.3 Com a entrada de energia em 3.1V medir o consumo de energia durante o work time
1.4 Com a entrada de energia em 3.1V medir o consumo de energia fora do work time 

2. Downlink:
2.1 Efetuar testes de downlink reconfigurando o dispositivo entre modo periódio e modo sense.
2.2 Efetuar testes de downlink alterando o intervalo de tempo de transmissão quando no modo periódico.
2.3 Efetuar testes de downlink alterando a sensibilidade do dispositivo.
2.4.1 Efetuar testes de downlink através do uso de imã. O imã fica habilitado por 2 minutos após o detectar que a máquina parou de vibrar.
2.4.2 Efetuar testes de downlink no processo diário de check config.

3. Uplink:
3.1 Efetuar testes de uplink de acordo com o modo de funcionamento do dispositivo
3.2 Verificar as ocorrências de retransmissão, anotar quando ocorrem
3.3 Verificar se os pacotes enviados estão de acordo com as especificações de protocolo definidas

4. Teste de Robustez:
4.1 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 23°C +/- 3°C
4.2 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 50°C +/- 3°C
4.3 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 7°C +/- 3°C

5. Teste de armazenamento e recuperação de dados da memória:
5.1 Configurar um dispositivo para transmissões periodicas de 5 em 5 minutos. Conectar em algum aparelho ligado (com vibração). Após 10 transmissões com ele ligado, deve-se desligar e religar o dispositivo. O teste consiste em verificar se a primeira transmissão após ligado mantém ou incrementa em apx. 4 minutos ou mais o tempo da última transmissão feita antes de desligar.

# ELEMENTOS PARA TESTE
# Downlink Payload BASE64
MDA2NTU1MUEwMDAzMGMzMA==

# Código para teste do RTC
 int count = 0;
 while(1){
  	RtcDelayMs(1000);
   	if (count == 10){
   	  printf("dez\r\n");
   	  count = 0;
   	}
   	count++;
}

# Código para teste do sense por
uint8_t act_timeout = 0x19;
uint8_t act_threshold = 0x03;
	
lsm303agr_act_timeout_set(&dev_ctx_xl, act_timeout);
lsm303agr_act_threshold_set(&dev_ctx_xl, act_threshold);
	