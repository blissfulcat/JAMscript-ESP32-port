all: pin_permute.o
	gcc -Wall pin_permute.o -lwiringPi

pin_permute.o: pin_permute.c
	gcc -c pin_permute.c

clean:
	rm pin_permute.o a.out
