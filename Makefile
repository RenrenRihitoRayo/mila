all: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -Wall -o mila mila.c -fsanitize=address -g

better: ml.c ml_builtins.c ml_dict.c ml.h
	gcc -std=c99 -lm -Wall -O3 -ffast-math -o mila mila.c

clean:
	rm mila
