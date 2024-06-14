all: start

start: start.o
	ld -m elf_i386 -e main start.o -o start

start.o: start.s
	nasm -f elf32 start.s -o start.o

clean:
	rm -f start.o start