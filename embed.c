// tell MiLa to not be standalone
#include "mila.h"
#define ML_NO_MAIN
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
    Env *globals = mila_global_init();
    // Set a variable
    env_set_local_raw(globals, "name", vstring_dup("User"));
    // Run the following code (yup this easy)
    Value *v = eval_str("println(\"Hello \" + name + \"!\");", globals);
    // Free what ever eval returns (for this example this should be null)
    val_release(v);
    // Clean up
    mila_global_deinit(globals);
    return 0;
}
