/*
    Minimal MiLa library
*/

// ML_LIB tells mila to define functions but not cli mechanisms
// Allows to compile with its own implementation
// Without needing to link with mila
#define ML_LIB
#include "mila.c"
#include <stdio.h>

// Simple function
Value *hello(Env *e, int argc, Value **argv) {
  printf("Hello from C!\n");
  return vnull();
}

Value *greet(Env *e, int argc, Value **argv) {
  if (argc != 1 || MILA_GET_TYPE(argv[0]) != T_STRING) {
    return vtagged_error(E_TYPE_ERROR,
                         "Expected first argument to be a string but got %s!",
                         MILA_GET_TYPENAME(argv[0]));
  }
  mila_printf("Hello %?!\n", argv[0]);
  return vnull();
}

// This fuction is optional
void _mila_lib_init(Env *e) {
  printf("Lib init!\n");
  return;
}

// Enter functions we want to export
// Passing function pointers is great for C++ libraries
// that cannot be sure of the function symbols due to
// name mangling of the functions
const NativeEntry lib_function_entries[] = {
    {"hello", hello}, // function name in string then function pointer
    {"greet", greet},
    {NULL, NULL}};

// Below can be used for C code bases
// where the function name is sure to be the same

const char *lib_functions[] = {"hello", // simply put the function name here
                               "greet", NULL};