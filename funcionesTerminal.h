#ifndef _FUNCIONES_TERM_H_
#define _FUNCIONES_TERM_H_

char *obtenerComando(int modo);
void enviarComando(char *com);
void enviarSalidaComando(char *comando);
void obtenerSalidaComando();
void enviarArchivo(char *archivo);
void recibirArchivo(char *archivo);

int redirectStdOut();
void restoreStdOut();

#endif
