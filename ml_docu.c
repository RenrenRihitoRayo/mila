#include "./ml_commons.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
    char* file = read_file(argv[1]);
    puts(file);
    free(file);
    return 0;
}
