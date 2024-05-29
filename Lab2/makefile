all: shell

shell: myshell.o
	gcc -g -Wall -m32 -o shell myshell.o
	
myshell.o: myshell.c LineParser.c LineParser.h
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

.PHONY: clean

clean:
	rm -f *.o shell
	