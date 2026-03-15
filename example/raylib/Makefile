all: raylib.mila mila_raylib.h
	python3 ../../mila_header_gen.py mila_raylib.h
	gcc -o raylib.so mila_raylib.mila-wrap.c -shared -fPIC -lraylib -lm -I../..
	../../mila raylib.mila

3d: raylib_3d.mila mila_raylib.h
	python3 ../../mila_header_gen.py mila_raylib.h
	gcc -o raylib.so mila_raylib.mila-wrap.c -shared -fPIC -lraylib -lm -I../..
	../../mila raylib_3d.mila

clean:
	rm raylib.so mila_raylib.mila-wrap.c