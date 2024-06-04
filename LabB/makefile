all: bubble hexaPrint

bubble: Bubblesort.o
	gcc -g -m32 -Wall -o bubble Bubblesort.o

Bubblesort.o: Bubblesort.c
	gcc -g -Wall -m32 -c -o Bubblesort.o Bubblesort.c

hexaPrint: hexaPrint.o
	gcc -g -m32 -Wall -o hexaPrint hexaPrint.o

hexaPrint.o: hexaPrint.c
	gcc -g -Wall -m32 -c -o hexaPrint.o hexaPrint.c

.PHONY: clean

clean:
	rm -f *.o Bubblesort
