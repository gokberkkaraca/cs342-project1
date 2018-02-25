all: prime
prime: prime.c
	cc -Wall -g -o prime prime.c
