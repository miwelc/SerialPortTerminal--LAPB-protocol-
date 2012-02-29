#ifndef _PUERTO_H_
#define _PUERTO_H_

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS0"

struct termios oldtio;

int abrirPuerto();
void guardarPuerto(int puerto);
void configurarPuerto(int fd);
void restaurarPuerto(int fd);
int enviarDatos(int puerto, void* buffer, int size);
int recibirDatos(int puerto, void* buffer, int size);

#endif
