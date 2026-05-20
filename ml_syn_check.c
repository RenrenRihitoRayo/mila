#define ML_LIB
#include "mila.c"

// buggy as hell
int main() {
    char* str = read_file("example/func.mila");
    printf("%s\n", str);
    Src* src = src_new(str);
    syn_check(src);
    return 0;
}