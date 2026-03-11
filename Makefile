all: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -o mila mila.c -fsanitize=address -g

bare: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -o mila mila.c -D MILA_USE_SHARED
	gcc -std=c99 -lm -o mila_builtins.so ml_builtins.c -D MILA_USE_MILA_C -fPIC -shared
	echo "Copying builtins cannonical to /lib"
	sudo cp mila_builtins.so /lib

termux:
	gcc -fsanitize=address -lm -Wl,--dynamic-linker=~/../usr/bin/llvm-symbolizer mila.c -o mila

release: ml.c ml_builtins.c ml_dict.c ml.h
	gcc -std=c99 -lm -O3 -ffast-math -o mila mila.c
	strip mila

clean:
	rm mila
	for file in *.so; do \
		[ -e "$$f" ] || continue; \
		rm $${file}; \
	done