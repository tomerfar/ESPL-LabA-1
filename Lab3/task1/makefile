all: start

start: start.o util.o
	gcc -m32 -nostartfiles start.o util.o -o start

start.o: start.s
	nasm -f elf32 start.s

util.o: util.c
	gcc -m32 -c util.c

clean:
	rm -f start.o util.o start