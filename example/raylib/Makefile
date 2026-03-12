all: raylib.mila
	python3 ../../mila_header_gen.py mila_raylib.h
	gcc -o raylib.so mila_raylib.mila-wrap.c -shared -fPIC -lraylib -lm -I../..
	../../mila raylib.mila

clean:
	rm raylib.so
	rm mila_raylib.mila-wrap.c
