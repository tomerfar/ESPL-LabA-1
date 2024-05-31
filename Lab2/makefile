all: shell looper

shell: myshell.o
	gcc -g -Wall -m32 -o shell myshell.o
    
myshell.o: myshell.c LineParser.c LineParser.h Looper.c
	gcc -g -Wall -m32 -c -o myshell.o myshell.c

looper: Looper.o
	gcc -g -Wall -m32 -o looper Looper.o

Looper.o: Looper.c
	gcc -g -Wall -m32 -c -o Looper.o Looper.c

.PHONY: clean

clean:
	rm -f *.o shell looper