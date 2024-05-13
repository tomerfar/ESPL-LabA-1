all: echo

echo: my_echo.o
	gcc -g -m32 -Wall -o echo my_echo.o

my_echo.o: my_echo.c
	gcc -g -Wall -m32 -c -o my_echo.o my_echo.c

.PHONY: clean

clean:
	rm -f *.o echo