CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lcurl -lzip -ljson-c

mlpkg: mlpkg.c
	$(CC) $(CFLAGS) -o mlpkg mlpkg.c $(LIBS)

install: mlpkg
	cp mlpkg /usr/local/bin/

clean:
	rm -f mlpkg

.PHONY: install clean