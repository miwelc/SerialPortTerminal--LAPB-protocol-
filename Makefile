
CC = gcc
CFLAGS = -g -O1

all :		tps

tps :	puerto.o trama.o funcionesTerminal.o tps.o
	$(CC) $(CFLAGS) puerto.o trama.o funcionesTerminal.o tps.o -o tps

puerto.o :	puerto.c
	$(CC) $(CFLAGS) -c puerto.c

trama.o :	trama.c
	$(CC) $(CFLAGS) -c trama.c

funcionesTerminal.o :	funcionesTerminal.c
	$(CC) $(CFLAGS) -c funcionesTerminal.c

tps.o :	tps.c
	$(CC) $(CFLAGS) -c tps.c


clean	:
	rm -f *.o tps
