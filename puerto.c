#include "puerto.h"
#include <string.h>

int abrirPuerto(){
    int fd;

    fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );

    if (fd <0 ) { perror(MODEMDEVICE); exit(-1); }

    return fd;
}

void guardarPuerto(int puerto){
   tcgetattr(puerto,&oldtio);  
}

void configurarPuerto(int fd){
   struct termios newtio;

   bzero(&newtio, sizeof(newtio));
  
   newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
   newtio.c_iflag = IGNPAR;
   newtio.c_oflag = 0;

   /* pone el modo entrada (no-canónico, sin eco,...) */
   newtio.c_lflag = 0;
   newtio.c_cc[VTIME] = 0; /* temporizador entre carácter, no usado */
   newtio.c_cc[VMIN] = 0; //=0 No bloquea /*=5 bloquea lectura hasta recibir 5 chars */
   tcflush(fd, TCIFLUSH);
   tcsetattr(fd,TCSANOW,&newtio);

}

void restaurarPuerto(int fd){
   tcsetattr(fd,TCSANOW,&oldtio);
   close(fd);
}

int enviarDatos(int puerto, void* buffer, int size) {
  int bytesT = 0;
  do {
    bytesT += write(puerto, (void*)(buffer+bytesT), size-bytesT);
  } while(bytesT < size);
}

int recibirDatos(int puerto, void* buffer, int size) {
  return read(puerto, buffer, size);
}
