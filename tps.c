#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "funcionesTerminal.h"

int main(int argc, char **argv) {
    int modo; //1: enviar, 0: recibir
    char *comando;
	
	if(argc < 2) {
		printf("Argumentos incorrectos. Uso: ./tps -[s|c]\n");
		exit(-2);
	}
	
	modo = (argv[1][1] == 's' ? 1 : 0);
	
	
	//Servidor
	if(modo == 1) {
		int fdPantalla;
		
		printf("Conectando...\n");
		conectaLAPB();
		printf("Servidor Conectado\n");
		
		do {
			comando = obtenerComando(1);
			if(strcmp(comando, "rver") == 0) {
				enviarSalidaComando("ls");
				printf("Lista de archivos enviada\n");
			}
			else if(strstr(comando, "rmover ")-comando == 0) {
				chdir(&comando[7]);
				printf("Nos movemos a la carpeta: '%s'\n", &comando[7]);
			} else if(strstr(comando, "llevar ")-comando == 0)
				recibirArchivo(&comando[7]);
			else if(strstr(comando, "traer ")-comando == 0)
				enviarArchivo(&comando[6]);
				
		} while(strcmp(comando, "salir") != 0);
		
		printf("\nDesconectando...\n");
		desconectaLAPB(1);
		printf("Desconetado.\n");
		
	} //Cliente
	else {
		printf("Conectando...\n");
		conectaLAPB();
		printf("Cliente Conectado\n");
		
		do {
			comando = obtenerComando(0);
			
			if(strcmp(comando, "lver") == 0)
				system("ls");
			else if(strcmp(comando, "rver") == 0) {
				enviarComando(comando);
				obtenerSalidaComando();
			}
			else if(strstr(comando, "lmover ")-comando == 0)
				chdir(&comando[7]);
			else if(strstr(comando, "llevar ")-comando == 0) {
				enviarComando(comando);
				enviarArchivo(&comando[7]);
			}
			else if(strstr(comando, "traer ")-comando == 0) {
				enviarComando(comando);
				recibirArchivo(&comando[6]);
			}
			else //Enviamos el resto de los comandos (los remotos)
				enviarComando(comando);
			
		} while(strcmp(comando, "salir") != 0);
		
		
		printf("\nDesconectando...\n");
		desconectaLAPB(0);
		printf("Desconetado.\n");
	}
	
	return 0;
}



