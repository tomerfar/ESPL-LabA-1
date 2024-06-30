all: hexeditplus count_digit

hexeditplus: hexeditplus.o
	gcc -g -Wall -m32 -o hexeditplus hexeditplus.o

hexeditplus.o: hexeditplus.c
	gcc -g -Wall -m32 -c -o hexeditplus.o hexeditplus.c

count_digit: count_digit.o
	gcc -g -Wall -m32 -fno-pie -fno-stack-protector -o count_digit count_digit.o

count_digit.o: count_digit.c
	gcc -g -Wall -m32 -fno-pie -fno-stack-protector -c -o count_digit.o count_digit.c

.PHONY: clean

clean:
	rm -f *.o hexeditplus count_digit