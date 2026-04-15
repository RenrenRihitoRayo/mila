cc ?= gcc
files = mila.c ml_builtins.c ml_dict.c mila.h ml_ll.c ml_string.c ml_threading.c ml_primitives.c
files_web = $(files) ./addon/ml_web.c
targets_web = ./build/web/mila.wasm ./build/web/mila.js

libraries ?= -lm
eflags ?= 
cflags = -O3 $(libraries) -Wextra -Wall -Wno-nonnull -Wno-unused-parameter -std=c11 $(eflags)

.PHONY: web

all: $(files)
	$(cc) $(libraries) $(eflags) -O0 -o mila mila.c -fsanitize=address -g\

# dont include asan, compile with debug logging
debug: $(files)
	$(cc)  $(libraries) -O0 -o mila mila.c -g -D MILA_DEBUG

debug-asan: $(files)
	$(cc)  $(libraries) -O0 -o mila mila.c -g -D MILA_DEBUG -fsanitize=address

test:
	$(cc) -o test.o0.mila -O0 mila.c $(libraries)
	$(cc) -o test.o3.mila -O3 mila.c $(libraries)
	$(cc) -o test.m.o3.mila -O3 -march=native mila.c $(libraries)
	$(cc) -o test.os.mila -Os mila.c $(libraries)

	$(cc) -o test.s.o0.mila -O0 mila.c $(libraries) 
	strip test.s.o0.mila
	$(cc) -o test.s.o3.mila -O3 mila.c $(libraries) 
	strip test.s.o0.mila
	$(cc) -o test.s.os.mila -Os mila.c $(libraries) 
	strip test.s.os.mila

	@echo -e "\nMiLa -O3 -march=native build"
	@ls -lh test.m.o3.mila
	@./test.m.o3.mila ./example/speed.mila
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
	$(cc) -o mila -Os mila.c $(libraries) -static 
	strip mila

smallest: $(files)
	$(cc) -o mila -Os mila.c $(libraries) 
	strip mila

release: $(files)
	$(cc) $(cflags) -o mila mila.c -march=native
	strip mila

web $(targets_web): $(files_web)
	mkdir -p build/web;:
	emcc -O3 -s WASM=1 -s EXPORTED_FUNCTIONS='["_main"]' -s EXPORTED_RUNTIME_METHODS='["FS","callMain"]' mila.c addon/ml_web.c -o ./build/web/mila.js -D EXT_WEB

clean:
	rm mila *.so test.*