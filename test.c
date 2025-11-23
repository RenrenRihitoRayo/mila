// for impl otherwise use mila.h
#define ML_LIB
#include "./mila.c"

int main(void) {
    // set up env
    Env* e = env_new(NULL);
    env_register_builtins(e);
    
    // set variables
    env_set(e, "var", vint(42));
    
    // set local
    env_set_local(e, "var2", vfloat(84.0));
    
    // run code
    Value* v = eval_str("println(\"Hello, world!\", var);", e);
    
    // delete a variable
    Value* tmp = env_get(e, "var");
    val_release(tmp);
    env_set(e, "var", NULL);
    
    // Clean up
    val_release(v); 
    env_free(e);
    return 0;
}