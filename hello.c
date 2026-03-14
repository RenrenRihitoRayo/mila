/*
    Minimal MiLa library
*/

#include "mila.c"
#include <stdio.h>

// Simple function
Value* hello(Env* e, int argc, Value** args) {
	printf("Hello from C!\n");
	return vnull();
}

// Bellow is optional
void _mila_lib_init(Env* e) {
	printf("Lib init!\n");
	return;
}

// Enter functions we want to export
const NativeEntry lib_function_entries[] = {
	{"hello", hello},
	{NULL, NULL}
};

// Bellow is also supported (and takes precedence over above)

const char* lib_functions[] = {
	"hello", // simply put the function name here
	NULL
};