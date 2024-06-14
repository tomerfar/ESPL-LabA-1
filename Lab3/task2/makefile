all:
	gcc -m32 -c main.c -o main.o
	gcc -m32 -c Util.c -o Util.o
	nasm -f elf32 start.s -o start.o
	gcc -m32 main.o Util.o start.o -o main

clean:
	rm -f main.o Util.o start.o main