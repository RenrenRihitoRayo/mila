CC ?= $(CC)

all: mila.c ml_builtins.c ml_dict.c mila.h
	$(CC) -std=c23 -lc -lm -O0 -o mila mila.c -fsanitize=address -g\
	 -D MILA_NO_SIGNAL_HANDLER

# dont include asan, compile with debug logging
debug: mila.c ml_builtins.c ml_dict.c mila.h
	$(CC) -std=c23 -lc -lm -O0 -o mila mila.c -g -D MILA_NO_SIGNAL_HANDLER -D MILA_DEBUG

test:
	$(CC) -o test.o0.mila -O0 mila.c -lm
	$(CC) -o test.o3.mila -O3 mila.c -lm
	$(CC) -o test.os.mila -Os mila.c -lm

	$(CC) -o test.s.o0.mila -O0 mila.c -lm
	strip test.s.o0.mila
	$(CC) -o test.s.o3.mila -O3 mila.c -lm
	strip test.s.o0.mila
	$(CC) -o test.s.os.mila -Os mila.c -lm
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

static: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -o mila -Os -ffast-math mila.c -lm -static
	strip mila

smallest: mila.c ml_builtins.c ml_dict.c mila.h
	gcc -o mila -Os -ffast-math mila.c -lm
	strip mila

bare: mila.c ml_builtins.c ml_dict.c mila.h
	$(CC) -std=c23 -O3 -lm -o mila mila.c -D MILA_USE_SHARED

	# Build mila shared library
	$(CC) -std=c23 -lm -fPIC -shared -o libmila_runtime.so mila.c -D MILA_USE_SHARED -D ML_LIB
	
	# Build mila builtins shared library, linking against mila.so
	$(CC) -std=c23 -lm -fPIC -shared -o mila_builtins.so ml_builtins.c -L. -lmila_runtime
	
	@echo "Copying builtins canonical to /lib"
	sudo cp mila_builtins.so /lib
	@echo "Copying runtime to /lib"
	sudo cp libmila_runtime.so /lib

release: mila.c ml_builtins.c ml_dict.c mila.h
	$(CC) -std=c23 -lm -O3 -o mila mila.c
	strip mila

clean:
	rm mila *.so
