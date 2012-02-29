#include "puerto.h"
#include "trama.h"
#include <string.h>
#include <time.h>

#define tamBufCir 20000

#define SABM 0
#define DISC 1
#define UA 2
#define RR 0
#define RNR 2
#define REJ 1
#define SREJ 3

typedef enum {
	CORRECTA,
	FALLIDA,
	NO_ENCONTRADA
}ESTADO_TRAMA;


int puerto;
unsigned char buffRec[tamBufCir];
int pr = 0, pw = 0;
int bytesLibresBufCir = tamBufCir;

void guardarBufferCircular(unsigned char *bufAux, int nBytes) {
	int i;
	for(i = 0; i < nBytes; i++) {
		buffRec[pw++] = bufAux[i];
		pw = pw%tamBufCir;
		bytesLibresBufCir--;
	}
}

void imprimirBufferCircular() {
	int i, j;
	printf("Contenido del buffer circular:\n");
	for(i = pr, j = 0;  j < tamBufCir-bytesLibresBufCir; j++, i++) {
		i = i%tamBufCir;
		printf("%c", buffRec[i]);
	}
	printf("\n");
}

void conectaLAPB() {
	Trama tramaPeticion;
	int no_conectado = 1;
	long int tiempo = 0;
	int estado;
	int bytesLeidosTmp;
	unsigned char buffTmp[tamBufCir];
	unsigned char *buffSalida = 0;
	
	puerto = abrirPuerto();
	guardarPuerto(puerto);
	configurarPuerto(puerto);
	
	while(no_conectado) {
		if(time(NULL)-tiempo > 3) {
			tramaPeticion = generaTramaLAPB(" ", 1, NO_NUMERADA, SABM, 0, SABM);
			bytesLeidosTmp = tramaToBuffer(&tramaPeticion, &buffSalida);
			enviarDatos(puerto, buffSalida, bytesLeidosTmp);
			tiempo = time(NULL);	
		}
		
		bytesLeidosTmp = recibirDatos(puerto, buffTmp, bytesLibresBufCir);
		guardarBufferCircular(buffTmp, bytesLeidosTmp);
		
		estado = delimitaLAPB(&tramaPeticion);
		
		if(estado == CORRECTA) {
			if( tramaPeticion.tipoTrama == NO_NUMERADA ) {
				if((tramaPeticion.ctrlFlujo[0]&0x3F) == SABM) {
					tramaPeticion = generaTramaLAPB(" ", 1, NO_NUMERADA, UA, 0, UA);
					bytesLeidosTmp = tramaToBuffer(&tramaPeticion, &buffSalida);
					enviarDatos(puerto, buffSalida, bytesLeidosTmp);
					no_conectado = 0;
				}
				if((tramaPeticion.ctrlFlujo[0]&0x3F) == UA) {
					no_conectado = 0;
				}
			}
		}
	}
	
	
	pr = pw = 0;
	bytesLibresBufCir = tamBufCir;
}

void desconectaLAPB() {
	Trama tramaPeticion;
	int conectado = 1;
	long int tiempo = 0;
	int estado;
	int bytesLeidosTmp;
	unsigned char buffTmp[tamBufCir];
	unsigned char *buffSalida = 0;
	
	pr = pw = 0;
	bytesLibresBufCir = tamBufCir;
	
	while(conectado) {
		if(time(NULL)-tiempo > 3) {
			tramaPeticion = generaTramaLAPB(" ", 1, NO_NUMERADA, DISC, 0, DISC);
			bytesLeidosTmp = tramaToBuffer(&tramaPeticion, &buffSalida);
			enviarDatos(puerto, buffSalida, bytesLeidosTmp);
			tiempo = time(NULL);
		}
		
		bytesLeidosTmp = recibirDatos(puerto, buffTmp, bytesLibresBufCir);
		guardarBufferCircular(buffTmp, bytesLeidosTmp);
		
		estado = delimitaLAPB(&tramaPeticion);
		
		if(estado == CORRECTA) {
			if( tramaPeticion.tipoTrama == NO_NUMERADA ) {
				if((tramaPeticion.ctrlFlujo[0]&0x3F) == DISC) {
					tramaPeticion = generaTramaLAPB(" ", 1, NO_NUMERADA, UA, 0, UA);
					bytesLeidosTmp = tramaToBuffer(&tramaPeticion, &buffSalida);
					enviarDatos(puerto, buffSalida, bytesLeidosTmp);
					conectado = 0;
				}
				if((tramaPeticion.ctrlFlujo[0]&0x3F) == UA) {
					conectado = 0;
				}
			}
		}
	}
	
    restaurarPuerto(puerto);
}

void enlaceLAPB(unsigned char *enviar, int longenv, unsigned char *recibidos, int *longrec) {
	int nTramaEnv = 0, nTramaRec = 0;
	int bytesEnviados = 0, bytesRecibidos = 0;
	int bytesAenviarTrama, bytesLeidosTmp;
	unsigned char buffTmp[tamBufCir];
	Trama tramaEnv, tramaRec, tramaResp;
	unsigned char *buffSalida = 0;
	int estado;
	int i;
	int transmitiendoTramas = 1;
	int enviarResp = 0, seguirEnviando = 1;
	long int tiempo = 0;
	
	if(longrec)
		*longrec = 0;
	
	pr = pw = 0;
	bytesLibresBufCir = tamBufCir;
	
	//printf("Enlace\n");
	
	while(transmitiendoTramas) {
		//Enviar
		if(bytesEnviados < longenv && seguirEnviando) {
			//Construir la trama a enviar
			bytesAenviarTrama = bytesEnviados+SIZE_DATOS_TRAMA_MAX <= longenv ? SIZE_DATOS_TRAMA_MAX : longenv-bytesEnviados;
			tramaEnv = generaTramaLAPB(&enviar[bytesEnviados], bytesAenviarTrama, INFORMACION, nTramaEnv, (bytesEnviados+bytesAenviarTrama)==longenv, nTramaEnv+1);
			//printf("Datos enviados: %s, %i, crc:%i\n", tramaEnv.datos, tramaEnv.nBytesDatos, tramaEnv.crc);
			//printf("\nNo. trama: %i, EsUltima:%i, Enviado: [%i/%i]\n", nTramaEnv, (tramaEnv.ctrlFlujo[1]&0x80) ? 1:0, bytesEnviados+bytesAenviarTrama, longenv);
			
	        //enviarla
			bytesLeidosTmp = tramaToBuffer(&tramaEnv, &buffSalida);
			/*printf("Datos enviados:\n");
			for(i = 0;  i < bytesLeidosTmp; i++) {
				printf("%c", buffSalida[i]);
			}
			printf("\n");
			*/
			enviarDatos(puerto, buffSalida, bytesLeidosTmp); //Lo escribimos en el puerto
			seguirEnviando = 0;
			tiempo = time(NULL);

		}

		if(seguirEnviando == 0 && time(NULL)-tiempo > 3) { //5s sin RR
		    //printf("Timeout expirado volvemos a enviar\n");
		    //printf("Datos enviados: %s, %i, crc:%i\n", tramaEnv.datos, tramaEnv.nBytesDatos, tramaEnv.crc);
			//tramaResp = generaTramaLAPB(" ", 1, SUPERVISION, 0, 0, 0); //Preguntamos por la trama en la que se ha quedado
			//enviarResp = 1;
			seguirEnviando = 1;
			tiempo = time(NULL);
		}
		
		//Recibir
		//Extraemos en el buffer circular los datos leidos del puerto
		bytesLeidosTmp = recibirDatos(puerto, buffTmp, bytesLibresBufCir);
			/*if(bytesLeidosTmp) {
				printf("Datos recibidos:\n");
				for(i = 0;  i < bytesLeidosTmp; i++) {
					printf("%c", buffTmp[i]);
				}
				printf("\n");
			}*/
				
		guardarBufferCircular(buffTmp, bytesLeidosTmp);
		//Extraemos la trama del buffer circular
		estado = delimitaLAPB(&tramaRec);
		
		if(estado == CORRECTA) {
			if(tramaRec.tipoTrama == INFORMACION) { //Si es de tpo informacion, generamos trama de confir, copiamos al buffer de salida y inidicamos que hay que enviar respuesta
				//printf("Trama de informacion recibida: ");
				
				//bytesLeidosTmp = (bytesLeidosTmp+bytesRecibidos <= longrec) ? bytesLeidosTmp : bytesLeidosTmp+bytesRecibidos - longrec;
				
				if(recibidos) {
					memcpy(&recibidos[bytesRecibidos], tramaRec.datos, tramaRec.nBytesDatos);
					bytesRecibidos += tramaRec.nBytesDatos;
					if(longrec)
						*longrec = bytesRecibidos;/*+++*/
					tramaResp = generaTramaLAPB(" ", 1, SUPERVISION, RR, 1, nTramaRec++);
					enviarResp = 1;
					//printf("Enviamos trama de confirmacion de la trama %i\n", nTramaRec-1);
				}
				if((tramaRec.ctrlFlujo[1]&0x80)) {//pf
					//printf("Ultima trama recibida\n");
					transmitiendoTramas = 0;
				}
			}
			if(tramaRec.tipoTrama == SUPERVISION) { //Si es de supervision
				//printf("Trama de supervision recibida: ");
				int s =	tramaRec.ctrlFlujo[0]&0x3F;
				int nr = tramaRec.ctrlFlujo[1]&0x7F;
				
				if((tramaRec.ctrlFlujo[1]&0x80) == 0){ //TIEMPO EXPIRDADO -> decimos trama por la que ibamos
					tramaResp = generaTramaLAPB(" ", 1, SUPERVISION, SREJ, 1, nTramaRec);
					//printf("Enviamos el numero de trama que queremos (timeout)\n");
					enviarResp = 1;
					//Vaciamos el buffer: vamos a volver a recibir la trama que queremos
					pr = pw = 0;
					bytesLibresBufCir = tamBufCir;
				}
				switch(s) {
					case RR:
						//printf("Hemos recibido confirmacion de la trama %i\n", nr);
						seguirEnviando = 1;
						nTramaEnv = nr+1; //Marcamos por cual vamos
						bytesEnviados += bytesAenviarTrama; //Se han enviado correctamente
						if((tramaEnv.ctrlFlujo[1]&0x80)) //Si la última trama que habíamos enviado era la última
							transmitiendoTramas = 0;
						break;
					case RNR:
						break;
					case REJ:
						break;
					case SREJ:
						//printf("Hemos recibido confirmacion negativa -> volver a enviar trama %i\n", nr);
						seguirEnviando = 1;
						nTramaEnv = nr;
						break;
				}
			}

		}
		else if(estado == FALLIDA) { //Si es fallida creamos la trama de supervision e indicamos que hay que enviar respuesta
			//printf("Fallo CRC En trama %i -> Deshechamos la trama\n", nTramaRec);
			//printf("Datos recibidos: %s, %i, crc:%i\n", tramaRec.datos, tramaRec.nBytesDatos, tramaRec.crc);
			//Vaciamos el buffer: desechamos el resto
			//tramaResp = generaTramaLAPB(" ", 1, SUPERVISION, SREJ, 1, nTramaRec);
			//enviarResp = 1;
		}
		
		if(enviarResp) {
			bytesLeidosTmp = tramaToBuffer(&tramaResp, &buffSalida);
			enviarDatos(puerto, buffSalida, bytesLeidosTmp); //Lo escribimos en el puerto
			enviarResp = 0;
		}
	}
}

int delimitaLAPB(Trama *trama) {
	int inicioTrama = 0, bytesLeidosBuff = 0;
    unsigned char bufferTrama[SIZE_DATOS_TRAMA_MAX+7 +2];
    unsigned short crc;
	int i, j, k;

	for(i = 0, j = pr; bytesLeidosBuff < tamBufCir-bytesLibresBufCir; j++, bytesLeidosBuff++) {
		j = j%tamBufCir;

		if(buffRec[j] == '~') {
			if(inicioTrama == 0)
				inicioTrama = 1;
			else if(inicioTrama == 1) { //Hemos recibido una trama completa
				if(i == 1) { // Hemos cogido ~~, así que quitamos todo lo anterior al segundo ~
					pr = j;
					bytesLibresBufCir += bytesLeidosBuff;
					return NO_ENCONTRADA;
				}
				
				bufferTrama[i++] = buffRec[j]; //Caracter delimitador final
				
				//Sacamos del buffer la trama
				*trama = bufferToTrama(bufferTrama, i);
				
				//Comprobamos CRC
				crc = CRCCCITT(trama->datos, trama->nBytesDatos);
					
				//Lo quitamos del buffer Circular
				pr = (j+1)%tamBufCir;
				bytesLibresBufCir += bytesLeidosBuff+1;
				
				if(trama->crc == crc)
					return CORRECTA;
				else
					return FALLIDA;
			}
		}
		
		
		if(inicioTrama && buffRec[j] == '#')
			inicioTrama = 2;
		if(inicioTrama == 2 && buffRec[j] != '#')
				inicioTrama = 1;
		
		if(inicioTrama > 0)
			bufferTrama[i++] = buffRec[j];
	}
	
	//Si no hemos encontrado ningun inicio de trama
	//o si el buffer está lleno y no hay ninguna trama completa, lo vaciamos 
	if(inicioTrama == 0 || (inicioTrama == 1 && bytesLibresBufCir == 0)) {
		pr = pw = 0;
		bytesLibresBufCir = tamBufCir;
	}
	
	return NO_ENCONTRADA;
}

Trama generaTramaLAPB(void *datos, size_t tamDatos, TipoTrama tipo, int NS_S_N, int PF, int NR_N) {
	Trama t;
	unsigned short crc;
	int i, j;
	
	t.nBytesTrama = tamDatos+7;
	t.nBytesDatos = tamDatos;
	t.tipoTrama = tipo;
	t.direccion = 0;
	
	t.ctrlFlujo[0] = t.ctrlFlujo[1] = 0;
	if(tipo == INFORMACION) {
		t.ctrlFlujo[0] &= 0x7F; //0
		t.ctrlFlujo[0] |= (NS_S_N&0xFF); //Hasta 127 Es NS
		if(PF) t.ctrlFlujo[1] |= 0x80; else t.ctrlFlujo[1] &= 0x7F; //PF
		t.ctrlFlujo[1] |= (NR_N&0xFF); //Hasta 127
	}
	else if(tipo == SUPERVISION) {
		t.ctrlFlujo[0] = ((t.ctrlFlujo[0]&0xBF)|0x80); //10
		t.ctrlFlujo[0] |= (NS_S_N&0xFF); //Es S
		if(PF) t.ctrlFlujo[1] |= 0x80; else t.ctrlFlujo[1] &= 0x7F; //PF
		t.ctrlFlujo[1] |= (NR_N&0xFF); //Hasta 127
	}
	else if(tipo == NO_NUMERADA) {
		t.ctrlFlujo[0] |= 0xC0; //11
		t.ctrlFlujo[0] |= (NS_S_N&0xFF); //Es N
		if(PF) t.ctrlFlujo[1] |= 0x80; else t.ctrlFlujo[1] &= 0x7F; //PF
		t.ctrlFlujo[1] |= (NR_N&0xFF); //Es N
	}
	 
	memcpy(&t.datos, datos, t.nBytesDatos);
	
	crc = CRCCCITT(t.datos, t.nBytesDatos);
	memcpy(&t.crc, &crc, sizeof(unsigned short));
	
	return t;
}

/*
int extraerDatosTrama(Trama t, unsigned char *datos) {
	unsigned char *buffTmp = malloc(t.nBytesDatos);
	int i, j;

	if(t.nBytesDatos == 0)
		return 0;

	memcpy(buffTmp, t.datos, t.nBytesDatos);
	
	datos[0] = buffTmp[0]; //Copiamos el primero
	//Comprobamos ocurrencias del caracter delimitador
	for(i = 1, j = 1; i < t.nBytesDatos; i++) { //Recorremos hasta el penúltimo
		if(buffTmp[i-1] == '#' && buffTmp[i] == '~') //Si # y despues ~ entonces ~
			datos[j-1] = '~';
		else //Si no hay una '#' especial, ni es ~
		 	datos[j++] = buffTmp[i];
	}
	
	return j;
}
*/

int tramaToBuffer(Trama *t, unsigned char **bufSalida) {
	int i, j;
	unsigned char *bufTrama = (unsigned char *)malloc(t->nBytesTrama);
	
	if(*bufSalida)
		free(*bufSalida);
	*bufSalida = (unsigned char *)malloc(t->nBytesTrama*2);
	
	bufTrama[0] = '~';
	memcpy(&bufTrama[1], &t->direccion, 1);
	memcpy(&bufTrama[2], t->ctrlFlujo, 2);
	memcpy(&bufTrama[4], t->datos, t->nBytesDatos);
	memcpy(&bufTrama[4+t->nBytesDatos], &t->crc, 2);
	bufTrama[4+t->nBytesDatos+2] = '~';	
	
	
	//Comprobamos ocurrencias del caracter delimitador Entre los dos delimitadores
	for(i = 1, j = 1; i < t->nBytesTrama-1; i++) {
		if(bufTrama[i] == '~')
			(*bufSalida)[j++] = '#';
		(*bufSalida)[j++] = bufTrama[i];
	}
	//Ponemos los delimitadores
	(*bufSalida)[0] = bufTrama[0];
	(*bufSalida)[j++] = bufTrama[i];
	
	free(bufTrama);
	
	return j;
}

Trama bufferToTrama(unsigned char *bufEntrada, size_t tamBufEntrada) {
	int i, j;
	Trama t;
	unsigned char *bufTrama = (unsigned char *)malloc(tamBufEntrada);
	int bitsTipoTrama;
	
	bufTrama[0] = bufEntrada[0]; //Copiamos el primero
	//Comprobamos ocurrencias del caracter delimitador
	for(i = 1, j = 1; i < tamBufEntrada; i++) {
		if(bufEntrada[i-1] == '#' && bufEntrada[i] == '~') //Si # y despues ~ entonces ~
			bufTrama[j-1] = '~';
		else //Si no había una '#' especial antes, ni es ~
		 	bufTrama[j++] = bufEntrada[i];
	}
	//printf("%i\n", j);
	t.nBytesTrama = j;
	if(t.nBytesTrama < 8) {//Trama incompleta (8 porque el campo de datos min. 1B)
		printf("Datos recibidos:\n");
		for(i = 0; i < tamBufEntrada; i++)
			printf("%c", bufEntrada[i]);
		printf("\n");
		t.nBytesDatos = 0;
		t.crc = 0;
		return t;
	}
	t.nBytesDatos = j - 7;
	memcpy(&t.direccion, &bufTrama[1], 1);
	memcpy(t.ctrlFlujo, &bufTrama[2], 2);
	memcpy(t.datos, &bufTrama[4], t.nBytesDatos);
	memcpy(&t.crc, &bufTrama[4+t.nBytesDatos], 2);
	bitsTipoTrama = (t.ctrlFlujo[0] >> 6);
	if(bitsTipoTrama <= 1)
		t.tipoTrama = INFORMACION;
	else if(bitsTipoTrama == 2)
		t.tipoTrama = SUPERVISION;
	else if(bitsTipoTrama == 3)
		t.tipoTrama = NO_NUMERADA;
	
	free(bufTrama);
	
	return t;
}

static unsigned short crc_table[256] = {
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5,
	0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
	0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210,
	0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c,
	0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
	0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b,
	0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6,
	0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
	0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5,
	0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969,
	0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
	0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc,
	0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03,
	0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
	0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6,
	0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a,
	0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
	0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1,
	0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c,
	0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
	0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb,
	0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447,
	0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
	0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2,
	0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9,
	0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
	0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c,
	0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0,
	0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
	0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07,
	0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba,
	0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
	0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

unsigned short CRCCCITT(unsigned char *data, size_t length) {
     size_t count;
     unsigned int crc = 0xffff;
     unsigned int temp;
     unsigned short final = 0;
     
     for(count = 0; count < length; count++) {
          temp = (*data++^(crc>>8)) & 0xff;
          crc = crc_table[temp] ^ (crc<<8);
     }
     return (unsigned short)(crc^final);
}
