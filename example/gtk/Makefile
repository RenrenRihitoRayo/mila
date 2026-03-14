all: mila_gtk.mila
	python3 ../../mila_header_gen.py mila_gtk.h
	gcc -o gtk.so mila_gtk.mila-wrap.c -shared -fPIC `pkgconf --cflags gtk+-3.0` `pkgconf --libs gtk+-3.0` -lm -I../..
	../../mila mila_gtk.mila

clean:
	rm gtk.so
	rm mila_gtk.mila-wrap.c