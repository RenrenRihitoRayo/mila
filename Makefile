cc ?= gcc
std ?= c11
files = mila.c ml_builtins.c ml_dict.c mila.h ml_ll.c
files_web = $(files) ./addon/ml_web.c
targets_web = ./web/mila.wasm ./web/mila.js
cflags = -O3 -lm -lc -std=$(std) -ffast-math

.PHONY: web

all: $(files)
	$(cc) -std=$(std) -lc -lm -O0 -o mila mila.c -fsanitize=address -g\
	 -D MILA_NO_SIGNAL_HANDLER

# dont include asan, compile with debug logging
debug: $(files)
	$(cc) -std=$(std) -lc -lm -O0 -o mila mila.c -g -D MILA_NO_SIGNAL_HANDLER -D MILA_DEBUG

debug-custom: $(files)
	$(cc) -std=$(std) -lc -lm -O0 -o mila mila.c -g -D MILA_NO_SIGNAL_HANDLER -D MILA_DEBUG -D MILA_CUSTOM

debug-asan: $(files)
	$(cc) -std=$(std) -lc -lm -O0 -o mila mila.c -g -D MILA_NO_SIGNAL_HANDLER -D MILA_DEBUG -fsanitize=address

debug-custom-asan: $(files)
	$(cc) -std=$(std) -lc -lm -O0 -o mila mila.c -g -D MILA_NO_SIGNAL_HANDLER -D MILA_DEBUG -D MILA_CUSTOM -fsanitize=address

test:
	$(cc) -o test.o0.mila -O0 mila.c -lm
	$(cc) -o test.o3.mila -O3 mila.c -lm
	$(cc) -o test.os.mila -Os mila.c -lm

	$(cc) -o test.s.o0.mila -O0 mila.c -lm
	strip test.s.o0.mila
	$(cc) -o test.s.o3.mila -O3 mila.c -lm
	strip test.s.o0.mila
	$(cc) -o test.s.os.mila -Os mila.c -lm
	strip test.s.os.mila

	@echo "MiLa -O0 build"
	@ls -lh test.o0.mila
	@./test.o0.mila ./example/speed.mila
	@echo -e "\nMiLa -O3 build"
	@ls -lh test.o3.mila
	@./test.o3.mila ./example/speed.mila
	@echo -e "\nMiLa -Os build"
	@ls -lh test.os.mila
	@./test.os.mila ./example/speed.mila

	@echo -e "\nMiLa -O0 build stripped"
	@ls -lh test.s.o0.mila
	@./test.s.o0.mila ./example/speed.mila
	@echo -e "\nMiLa -O3 build stripped"
	@ls -lh test.s.o3.mila
	@./test.s.o3.mila ./example/speed.mila
	@echo -e "\nMiLa -Os build stripped"
	@ls -lh test.s.os.mila
	@./test.s.os.mila ./example/speed.mila

	@rm test.*

static: $(files)
	gcc -o mila -Os mila.c -lm -static
	strip mila

smallest: $(files)
	$(cc) -o mila -Os mila.c -lm
	strip mila

release: $(files)
	$(cc) $(cflags) -o mila mila.c -march=native
	strip mila

release-custom: $(files)
	$(cc) $(cflags) -o mila mila.c -march=native -D MILA_CUSTOM

web $(targets_web): $(files_web)
	mkdir -p build/web;:
	emcc -O3 -s WASM=1 -s EXPORTED_FUNCTIONS='["_main"]' -s EXPORTED_RUNTIME_METHODS='["FS","callMain"]' mila.c addon/ml_web.c -o ./build/web/mila.js -D EXT_WEB

clean:
	rm mila *.so test.*