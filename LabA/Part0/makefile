#format is target name : target dependencies

#ALL Targets
all: exec

exec: main.o numbers.o add.o
	gcc -g -m32 -Wall -o exec main.o numbers.o add.o

main.o: main.c
	gcc -g -Wall -m32 -c -o main.o main.c

numbers.o: numbers.c
	gcc -g -Wall -m32 -c -o numbers.o numbers.c

add.o: add.s
	nasm -g -f elf -w+all -o add.o add.s

#tell make that clean is not a file name!

.PHONY: clean
#Clean the built directory
clean:
	rm -f *.o exec


