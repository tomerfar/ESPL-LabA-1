all: virus

virus: AntiVirus.o
	gcc -g -m32 -Wall -o virus AntiVirus.o

AntiVirus.o: AntiVirus.c
	gcc -g -Wall -m32 -c -o AntiVirus.o AntiVirus.c

.PHONY: clean

clean:
	rm -f *.o  virus