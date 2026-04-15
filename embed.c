#define ML_LIB
#define ML_NO_MATH
#include "mila.c"

int main() {
    Env* globals = mila_init();
    Value* v = eval_str("println(\"Hello, world!\")", globals);
    val_release(v);
    mila_deinit(globals);
    return 0;
}
