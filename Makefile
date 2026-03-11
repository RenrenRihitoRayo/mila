test:
	gcc -o test.o0.mila -O0 mila.c -lm
	gcc -o test.o3.mila -O3 mila.c -lm
	gcc -o test.os.mila -Os mila.c -lm

	gcc -o test.s.o0.mila -O0 mila.c -lm
	strip test.s.o0.mila
	gcc -o test.s.o3.mila -O3 mila.c -lm
	strip test.s.o0.mila
	gcc -o test.s.os.mila -Os mila.c -lm
	strip test.s.os.mila

	@echo "MiLa -O0 build"
	@file test.o0.mila
	@./test.o0.mila ./example/speed.mila
	@echo -e "\nMiLa -O3 build"
	@file test.o0.mila
	@./test.o3.mila ./example/speed.mila
	@echo -e "\nMiLa -Os build"
	@file test.os.mila
	@./test.os.mila ./example/speed.mila

	@echo -e "\nMiLa -O0 build stripped"
	@file test.s.o0.mila
	@./test.s.o0.mila ./example/speed.mila
	@echo -e "\nMiLa -O3 build stripped"
	@file test.s.o0.mila
	@./test.s.o3.mila ./example/speed.mila
	@echo -e "\nMiLa -Os build stripped"
	@file test.s.os.mila
	@./test.s.os.mila ./example/speed.mila

	@rm test.*

all: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -o mila mila.c -fsanitize=address -g

bare: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -o mila mila.c -D MILA_USE_SHARED
	gcc -std=c99 -lm -o mila_builtins.so ml_builtins.c -D MILA_USE_MILA_C -fPIC -shared
	echo "Copying builtins cannonical to /lib"
	sudo cp mila_builtins.so /lib

termux:
	gcc -fsanitize=address -lm -Wl,--dynamic-linker=~/../usr/bin/llvm-symbolizer mila.c -o mila

release: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -std=c99 -lm -O3 -ffast-math -o mila mila.c
	strip mila

clean:
	rm mila
	for file in *.so; do \
		[ -e "$$f" ] || continue; \
		rm $${file}; \
	done