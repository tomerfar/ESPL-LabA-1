

all: assembly

start.o: start.s
	nasm -f elf32 start.s -o start.o

Util.o: Util.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector Util.c -o Util.o

main.o: main.c
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o

assembly: start.o main.o Util.o
	ld -m elf_i386 start.o main.o Util.o -o assembly

.PHONY: clean

clean:
	rm -f start.o Util.o main.o assembly