cc ?= gcc
files = *.[ch] headers/*
files_web = $(files) ./addon/ml_web.c ./addon/ml_web.h ./addon/ml_socket.c ./addon/http/http.c ./addon/http/http.h
targets_web = ./build/web/mila.wasm ./build/web/mila.js

libraries ?= -lm
eflags ?=
cflags_debug = $(libraries) -Wextra -Wall -Wno-nonnull\
         -Wno-unused-parameter -Wno-enum-compare -Wno-enum-conversion -std=c11\
         $(eflags) -Iheaders
cflags_generic = $(libraries) -Wextra -Wall -Wno-nonnull\
         -Wno-unused-parameter -Wno-enum-compare -Wno-enum-conversion -std=c11\
         $(eflags) -Iheaders -Wno-overflow -flto -ffunction-sections -fdata-sections -Wl,-s\
         -Wl,--gc-sections -fno-stack-protector
cflags = $(cflags_generic) -march=native
cflags_lib = $(libraries) -Wextra -Wall -Wno-nonnull\
         -Wno-unused-parameter -Wno-enum-compare -Wno-enum-conversion -std=c11\
         $(eflags) -Iheaders -Wno-overflow -DML_NO_MAIN

.PHONY: web

all: $(files)
	$(cc) $(libraries) $(eflags) $(cflags_debug) -O0 -o mila mila.c -fsanitize=address -g -Iheaders

# dont include asan, compile with debug logging
debug: $(files)
	$(cc) $(libraries) $(cflags_debug) -O0 -o mila mila.c -g -D MILA_DEBUG

debug-asan: $(files)
	$(cc)  $(libraries) $(cflags_debug) -O0 -o mila mila.c -g -D MILA_DEBUG -fsanitize=address

lib: $(files)
	$(cc) $(libraries) $(cflags_lib) -O3 -shared -fPIC mila.c -o libmila.so

libstatic: $(files)
	$(cc) $(libarries) $(cflags_lib) -O3 -c mila.c -o mila.so -DML_NO_DL
	ar rcs libmila.a mila.so

libstatic-debug: $(files)
	$(cc) $(libarries) $(cflags_lib) -O0 -g -c mila.c -o mila.so -DML_NO_DL
	ar rcs libmila.a mila.so

test:
	$(cc) -o test.o0.mila -O0 mila.c $(cflags)
	$(cc) -o test.o3.mila -O3 mila.c $(cflags)
	$(cc) -o test.os.mila -Os mila.c $(cflags)

	$(cc) -o test.s.o0.mila -O0 mila.c $(cflags)
	strip test.s.o0.mila
	$(cc) -o test.s.o3.mila -O3 mila.c $(cflags)
	strip test.s.o0.mila
	$(cc) -o test.s.os.mila -Os mila.c $(cflags)
	strip test.s.os.mila

	@echo -e "\nMiLa -O0 build"
	@ls -lh test.o0.mila
	@time ./test.o0.mila -r "println(\"Hello, world!\");"
	@echo -e "\nMiLa -O3 build"
	@ls -lh test.o3.mila
	@time ./test.o3.mila -r "println(\"Hello, world!\");"
	@echo -e "\nMiLa -Os build"
	@ls -lh test.os.mila
	@time ./test.os.mila -r "println(\"Hello, world!\");"

	@echo -e "\nMiLa -O0 build stripped"
	@ls -lh test.s.o0.mila
	@time ./test.s.o0.mila -r "println(\"Hello, world!\");"
	@echo -e "\nMiLa -O3 build stripped"
	@ls -lh test.s.o3.mila
	@time ./test.s.o3.mila -r "println(\"Hello, world!\");"
	@echo -e "\nMiLa -Os build stripped"
	@ls -lh test.s.os.mila
	@time ./test.s.os.mila -r "println(\"Hello, world!\");"

	@rm test.*

static: $(files)
	$(cc) -o mila mila.c -static -DML_NO_DL $(cflags_generic)
	strip mila

smallest: $(files)
	$(cc) $(cflags) -o mila -Os mila.c
	strip mila

release: $(files)
	$(cc) $(cflags) -O3 -o mila mila.c
	strip mila

test-embed: embed.c
	gcc -o embed embed.c -Iheaders
	./embed && ls -lh embed
	rm embed

test-lib: hello.c
	@gcc -o hello.so hello.c -Iheaders -lm -shared -fPIC
	@mila -r 'load("hello.so"); hello(); greet("User");'
	@rm hello.so

install:
	make release
	sudo cp mila /usr/bin
	cp mila mila.release

install-debug:
	make
	sudo cp mila /usr/bin
	cp mila mila.release

clean:
	rm mila *.so test.* *.a mtags

mtags: $(files)
	gcc -o mtags mtags.c -Iheaders -O3 -std=c11\
         $(eflags) -flto -ffunction-sections -fdata-sections -Wl,-s\
         -Wl,--gc-sections -fno-stack-protector


clang-format:
	clang-format -i -style="{BasedOnStyle: LLVM, BreakBeforeBraces: Attach, IndentWidth: 4, UseTab: Never}" *.[ch] **/*.[ch]
