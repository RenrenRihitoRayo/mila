all: raylib.mila mila_raylib.h
	python3 ../../mila_header_gen.py mila_raylib.h
	gcc -o raylib.so mila_raylib.mila-wrap.c -g -O0 -fsanitize=address -shared -fPIC -lraylib -lm -I../.. -I../../headers
	LD_PRELOAD="/usr/lib/gcc/x86_64-pc-linux-gnu/16.1.1/../../../../lib/libasan.so" ../../mila raylib.mila

clean:
	rm raylib.so mila_raylib.mila-wrap.c