// tell MiLa to not be standalone
#define ML_LIB
// so we dont need -lm
#define ML_NO_LIBM
// no dl mechanisms
#define ML_NO_DL
// Yes the C file not header
#include "mila.c"

int main() {
    // MiLa init handles
    // * Env creation
    // * Bindings injection (this is manual for best customizability)
    // other init
    Env* globals = mila_init();
    // Run the following code (yup this easy)
    Value* v = eval_str(
            "println(\"Hello, world!\");",
    globals);
    // Free what ever eval returns (for this example this should be null)
    val_release(v);
    // Clean up
    mila_deinit(globals);
    return 0;
}
