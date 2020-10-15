# SIGSENSE
Produto para monitoramento de tempo de m�quina ligada pela sua vibra��o.

***

# CADERNO DE TESTES DO FIRMWARE e HARDWARE
Testes a serem realizados no firmware e no hardware: 

1. Consumo: 
1.1 Medir o consumo m�dio do dispositivo durante o work time (210uA)
1.2 Medir o consumo m�dio do dispositivo fora do work time 
1.3 Com a entrada de energia em 3.1V medir o consumo de energia durante o work time
1.4 Com a entrada de energia em 3.1V medir o consumo de energia fora do work time 

2. Downlink:
2.1 Efetuar testes de downlink reconfigurando o dispositivo entre modo peri�dio e modo sense.
2.2 Efetuar testes de downlink alterando o intervalo de tempo de transmiss�o quando no modo peri�dico.
2.3 Efetuar testes de downlink alterando a sensibilidade do dispositivo.
2.4.1 Efetuar testes de downlink atrav�s do uso de im�. O im� fica habilitado por 2 minutos ap�s o detectar que a m�quina parou de vibrar.
2.4.2 Efetuar testes de downlink no processo di�rio de check config.

3. Uplink:
3.1 Efetuar testes de uplink de acordo com o modo de funcionamento do dispositivo
3.2 Verificar as ocorr�ncias de retransmiss�o, anotar quando ocorrem
3.3 Verificar se os pacotes enviados est�o de acordo com as especifica��es de protocolo definidas

4. Teste de Robustez:
4.1 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 23�C +/- 3�C
4.2 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 50�C +/- 3�C
4.3 Manter o dispositivo funcionando por 7 dias initerruptos em um ambiente controlado, com temperatura de 7�C +/- 3�C

5. Teste de armazenamento e recupera��o de dados da mem�ria:
5.1 Configurar um dispositivo para transmiss�es periodicas de 5 em 5 minutos. Conectar em algum aparelho ligado (com vibra��o). Ap�s 10 transmiss�es com ele ligado, deve-se desligar e religar o dispositivo. O teste consiste em verificar se a primeira transmiss�o ap�s ligado mant�m ou incrementa em apx. 4 minutos ou mais o tempo da �ltima transmiss�o feita antes de desligar.

# ELEMENTOS PARA TESTE
# Downlink Payload BASE64
MDA2NTU1MUEwMDAzMGMzMA==

# C�digo para teste do RTC
 int count = 0;
 while(1){
  	RtcDelayMs(1000);
   	if (count == 10){
   	  printf("dez\r\n");
   	  count = 0;
   	}
   	count++;
}

# C�digo para teste do sense por
uint8_t act_timeout = 0x19;
uint8_t act_threshold = 0x03;
	
lsm303agr_act_timeout_set(&dev_ctx_xl, act_timeout);
lsm303agr_act_threshold_set(&dev_ctx_xl, act_threshold);
	