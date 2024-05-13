all: debugMode

debugMode: debug.o
	gcc -g -m32 -Wall -o debugMode debug.o

debug.o: debug.c
	gcc -g -Wall -m32 -c -o debug.o debug.c

.PHONY: clean

clean:
	rm -f *.o debug