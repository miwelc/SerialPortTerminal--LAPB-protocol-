#include "funcionesTerminal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trama.h"

char *obtenerComando(int modo) {
	char *com = malloc(512*sizeof(char));
	
	if(modo == 0) { //Cliente
		printf("\n>");
		gets(com);
	}
	else { //Servidor
		enlaceLAPB(NULL, 0, com, NULL);
	}

	return com;
}

void enviarComando(char *com) {
	enlaceLAPB(com, strlen(com)+1, NULL, NULL);
}

void enviarSalidaComando(char *comando) {
	int numbytes;
	const int tamSalida = 4096;
	char salida[tamSalida];
	
	//Redirigimos stdout
	int fdPantalla = redirectStdOut();
	
	system(comando);
	numbytes = read(fdPantalla, salida, tamSalida-1);
	salida[numbytes] = '\0';
	
	restoreStdOut();

	
	numbytes = strlen(salida);
	
	enlaceLAPB(salida, numbytes+1, NULL, NULL);
}

void obtenerSalidaComando() {
	const int tamSalida = 4096;
	char salida[tamSalida];
	
	enlaceLAPB(NULL, 0, salida, NULL);
	printf("%s", salida);
}

void enviarArchivo(char * nombre) {
	FILE *fd;
	int numbytes;
	const int tamBuf = 1500000;
	unsigned char datos[tamBuf];
	
	if((fd = fopen(nombre, "rb")) == 0) {
    	printf("No se ha podido abrir el archivo \'%s\'\n", nombre);
    	exit(-3);
	}
    while((numbytes = fread(datos, 1, tamBuf, fd)) > 0) { //Leemos de fichero
		enlaceLAPB(datos, numbytes, NULL, NULL);
	}
	
	printf("Arhivo enviado: '%s'\n", nombre);
}

void recibirArchivo(char *nombre) {
	FILE *fd;
	int numbytes;
	const int tamBuf = 1500000;
	unsigned char datos[tamBuf];
	
	if((fd = fopen(nombre, "wb")) == 0) {
    	printf("No se ha podido crear el archivo \'%s\'\n", nombre);
    	exit(-3);
	}
	enlaceLAPB(NULL, 0, datos, &numbytes);
	fwrite(datos, 1, numbytes, fd); //Lo escribimos en el fichero
	fclose(fd);
	
	printf("Arhivo recibido: '%s'\n", nombre);
}

int pipePair[2];
int oldstdout;

int redirectStdOut() {
  oldstdout = dup(STDOUT_FILENO);

  if ( pipe(pipePair) != 0){
    return -1;
  }

  dup2(pipePair[1], STDOUT_FILENO);
  close (pipePair[1]);
  return pipePair[0];
}
void restoreStdOut() {
  dup2(oldstdout, STDOUT_FILENO);

  close(pipePair[0]);
  pipePair[0] = -1;
}
