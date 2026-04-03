cc ?= gcc
std ?= c11
files = mila.c ml_builtins.c ml_dict.c mila.h ml_ll.c
cflags = -O3 -lm -lc -std=$(std) -ffast-math

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

vmm: $(files)
	gcc -E mila.c -o mila_vmm.c

static: $(files)
	gcc -o mila -Os mila.c -lm -static
	strip mila

smallest: $(files)
	gcc -o mila -Os mila.c -lm
	strip mila

bare: $(files)
	$(cc) -std=$(std) -O3 -lm -o mila mila.c -D MILA_USE_SHARED

	# Build mila shared library
	$(cc) -std=$(std) -lm -fPIC -shared -o libmila_runtime.so mila.c -D MILA_USE_SHARED -D ML_LIB
	
	# Build mila builtins shared library, linking against mila.so
	$(cc) -std=$(std) -lm -fPIC -shared -o mila_builtins.so ml_builtins.c -L. -lmila_runtime
	
	@echo "Copying builtins canonical to /lib"
	sudo cp mila_builtins.so /lib
	@echo "Copying runtime to /lib"
	sudo cp libmila_runtime.so /lib

release: $(files)
	$(cc) $(cflags) -o mila mila.c -march=native
	strip mila

release-custom: $(files)
	$(cc) $(cflags) -o mila mila.c -march=native -D MILA_CUSTOM

clean:
	rm mila mila_vmm.c *.so
