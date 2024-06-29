all: hexeditplus

hexeditplus: hexeditplus.o
	gcc -g -Wall -m32 -o hexeditplus hexeditplus.o

hexeditplus.o: hexeditplus.c
	gcc -g -Wall -m32 -c -o hexeditplus.o hexeditplus.c

.PHONY: clean

clean:
	rm -f *.o hexeditplus