#include <emscripten.h>
#include "../mila.h"

Value* native_dom(Env* e, int argc, Value** argv);
Value* native_dom_eval(Env* e, int argc, Value** argv);
Value* native_dom_set_text(Env* e, int argc, Value** argv);
Value* native_dom_get_text(Env* e, int argc, Value** argv);
Value* native_dom_set_html(Env* e, int argc, Value** argv);
Value* native_dom_add_class(Env* e, int argc, Value** argv);
Value* native_console_log(Env* e, int argc, Value** argv);
Value* native_alert(Env* e, int argc, Value** argv);
void env_register_web_ext(Env* e);