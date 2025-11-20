mila: *.c
	gcc -std=c99 -lm -Wall -o mila mila.c

better: *.c
	gcc -std=c99 -lm -Wall -O3 -ffast-math -o mila mila.c

clean:
	rm mila
