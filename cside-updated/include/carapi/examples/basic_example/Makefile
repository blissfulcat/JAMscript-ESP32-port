all: basic_example.o
	gcc -Wall basic_example.o -lwiringPi

basic_example.o: basic_example.c
	gcc -c basic_example.c

clean:
	rm basic_example.o a.out
