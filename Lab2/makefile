all: shell

shell: myshell.o LineParser.o
	gcc -g -Wall -m32 -o shell myshell.o LineParser.o
	
myshell.o: myshell.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -m32 -c -o LineParser.o LineParser.c





.PHONY: clean

clean:
	rm -f *.o shell
	