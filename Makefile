CC = gcc
CFLAGS  = -I../inc -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -g -Wall
LIBS := -lglib-2.0 -lreadline -lm

obj-m += hanmatek.o

all : hanmatek install

hanmatek: hanmatek.o
	$(CC) $(CFLAGS) -o hanmatek hanmatek.o $(LIBS)
	rm -f *.o

install:
	sudo usermod -a -G dialout $(shell whoami)

clean:
	rm -f hanmatek