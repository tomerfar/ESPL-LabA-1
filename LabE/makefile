all: myELF tomer omri myELFaa

myELF: myELF.o
	gcc -g -Wall -m32 -o myELF myELF.o

myELF.o: myELF.c
	gcc -g -Wall -m32 -c -o myELF.o myELF.c

tomer: tomer.o
	gcc -g -Wall -m32 -o tomer tomer.o

tomer.o: tomer.c
	gcc -g -Wall -m32 -c -o tomer.o tomer.c

omri: omri.o
	gcc -g -Wall -m32 -o omri omri.o

omri.o: omri.c
	gcc -g -Wall -m32 -c -o omri.o omri.c

myELFaa: myELFaa.o
	gcc -g -Wall -m32 -o myELFaa myELFaa.o

myELFaa.o: myELFaa.c
	gcc -g -Wall -m32 -c -o myELFaa.o myELFaa.c


.PHONY: clean

clean:
	rm -f myELF myELF.o tomer tomer.o omri omri.o myELFaa myELFaa.o