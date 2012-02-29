#ifndef _TRAMA_H_
#define _TRAMA_H_

#include <stdlib.h>
#include <stdio.h>

#define SIZE_DATOS_TRAMA_MAX 1000

typedef enum {
	INFORMACION,
	SUPERVISION,
	NO_NUMERADA
} TipoTrama;

typedef struct {
	int nBytesTrama;
	int nBytesDatos;
	TipoTrama tipoTrama;
	unsigned char direccion;
	unsigned char ctrlFlujo[2];
	unsigned char datos[SIZE_DATOS_TRAMA_MAX]; //Datos hasta sizeB
	unsigned short crc; //Control de errores 2B
} Trama;

void conectaLAPB();
void desconectaLAPB();
void enlaceLAPB(unsigned char *enviar, int longenv, unsigned char *recibidos, int *longrec);
int delimitaLAPB(Trama *trama);

Trama generaTramaLAPB(void *datos, size_t tamDatos, TipoTrama tipo, int NS_S_N, int PF, int NR_N);
//int extraerDatosTrama(Trama trama, unsigned char *datos);
int tramaToBuffer(Trama *t, unsigned char **bufSalida);
Trama bufferToTrama(unsigned char *buf, size_t tamTrama);
unsigned short CRCCCITT(unsigned char *data, size_t length);


#endif
