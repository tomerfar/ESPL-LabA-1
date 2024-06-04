all: bubble

bubble: Bubblesort.o
	gcc -g -m32 -Wall -o bubble Bubblesort.o

Bubblesort.o: Bubblesort.c
	gcc -g -Wall -m32 -c -o Bubblesort.o Bubblesort.c

.PHONY: clean

clean:
	rm -f *.o Bubblesort
