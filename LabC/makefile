all: myshell mypipeline looper

myshell: myshell.o LineParser.o
	gcc -g -Wall -o myshell myshell.o LineParser.o

myshell.o: myshell.c
	gcc -g -Wall -c -o myshell.o myshell.c

LineParser.o: LineParser.c LineParser.h
	gcc -g -Wall -c -o LineParser.o LineParser.c

mypipeline: mypipeline.o 
	gcc -g -Wall -o mypipeline mypipeline.o

mypipeline.o: mypipeline.c
	gcc -g -Wall -c -o mypipeline.o mypipeline.c

looper: looper.o
	gcc -g -Wall -m32 -o looper looper.o

looper.o: looper.c
	gcc -g -Wall -m32 -c -o looper.o looper.c


.PHONY: clean 

clean:
	rm -f myshell myshell.o LineParser.o  mypipeline mypipeline.o looper looper.o