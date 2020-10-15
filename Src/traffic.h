#ifndef _TRAFFIC_H
#define _TRAFFIC_H

/*************************************************************************************************/
/*    DEFINES                                                                                    */
/*************************************************************************************************/
#define ESTADO_AGUARDANDO_DETECCAO		0
#define ESTADO_VEICULO_DETECTADO		1
#define ESTADO_INICIAL                  2
#define ESTADO_ERRO_AMPLITUDE           3

#define SENSIBILIDADE_DETECCAO          20
#define SENSIBILIDADE_LIBERACAO         10

//#define IGNORE_SAMPLES                  70

//#define NUM_AMOSTRAS_COUNT              3      //numero de ocorrencias na amostra acima de um limiar para mudança de estado

#define SIZE_BUFFER_MEDIA 400
int16_t bufferMediaX[SIZE_BUFFER_MEDIA];
int16_t bufferMediaY[SIZE_BUFFER_MEDIA];
int16_t bufferMediaZ[SIZE_BUFFER_MEDIA];

int16_t valorRefX;
int16_t valorRefY;
int16_t valorRefZ;

uint8_t estadoX;
uint8_t estadoY;
uint8_t estadoZ;
uint8_t estadoChangedX;
uint8_t estadoChangedY;
uint8_t estadoChangedZ;

typedef struct {
	
	int16_t *p16_bufAddress;
	uint16_t bufferSize;
	uint16_t next_pos;
	int16_t media;
	
} st_traffic_buffer;

st_traffic_buffer stTrafficBufferX;
st_traffic_buffer stTrafficBufferY;
st_traffic_buffer stTrafficBufferZ;
/*************************************************************************************************/
/*    PROTOTYPES                                                                                    */
/*************************************************************************************************/

void verificaProcessaEventoThreshold(unsigned int numCalibracao);
void verificarRegistradores(void);
//uint8_t algoritmoContagemEixo (int16_t magnetometroValor, int16_t valorReferencia, uint8_t *pEstado, uint16_t linhaDeCorte,uint16_t *pignoreSamples, uint8_t *pDetectadoCount);
uint8_t algoritmoContagemEixo (int16_t magnetometroValor, int16_t valorReferencia, uint8_t *pEstado, uint16_t linhaDeCorteSuperior, uint16_t linhaDeCorteInferior ,uint16_t *pignoreSamples, uint8_t *pDetectadoCount);
uint8_t algoritmoContagemEixoXYZ (int16_t magX, int16_t referenciaX, int16_t magY, int16_t referenciaY, int16_t magZ, int16_t referenciaZ, uint8_t *pEstado, uint16_t linhaDeCorte);
int16_t filtroMedia( int16_t valorNovo, int16_t *pBuff, uint16_t numAmostras);
float kalmanFilterAtualizar(float *pant, float *xant, float valorMedido, float Qw, float Qv);
bool filtraDadoMagnetometro(int16_t *valorMedioX, int16_t *valorMedioY, int16_t *valorMedioZ);
void calibracao(int16_t *valorRefX, int16_t *valorRefY, int16_t *valorRefZ, uint16_t);
void atualizaMedias(int16_t *valorRefX, int16_t *valorRefY, int16_t *valorRefZ);
void atualizaMedia(int16_t *valorRef, int16_t rawData, uint8_t eixo);
uint8_t algoritmoContagemEixoXYZ_2(uint8_t contx, uint8_t conty, uint8_t contz, uint8_t estadox, uint8_t estadoy, uint8_t estadoz);
int16_t filtroMediaSt( int16_t valorNovo, st_traffic_buffer *pStBuffer);
void clearBuffer(uint8_t *pbuf, uint8_t len);

#endif
