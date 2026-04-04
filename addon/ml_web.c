#include <emscripten.h>
#define MILA_PROTO
#include "../mila.h"

/* ============================================================================
 * BETTER VERSION: Dedicated functions instead of eval
 * ============================================================================
 * More secure and performant - define specific JS functions to call
 */

// Update DOM element
EM_JS(void, js_set_innerHTML, (const char* selector, const char* html), {
    var sel = UTF8ToString(selector);
    var htmlStr = UTF8ToString(html);
    var elem = document.querySelector(sel);
    if (elem) {
        elem.innerHTML = htmlStr;
    }
});

// Set text content
EM_JS(void, js_set_text, (const char* selector, const char* text), {
    var sel = UTF8ToString(selector);
    var textStr = UTF8ToString(text);
    var elem = document.querySelector(sel);
    if (elem) {
        elem.textContent = textStr;
    }
});

// Get element text
EM_JS(const char*, js_get_text, (const char* selector), {
    var sel = UTF8ToString(js_get_text);
    var elem = document.querySelector(sel);
    var text = elem ? elem.textContent : "";
    
    var resultLen = lengthBytesUTF8(text) + 1;
    var buffer = _malloc(resultLen);
    stringToUTF8(text, buffer, resultLen);
    return buffer;
});

// Add CSS class
EM_JS(void, js_add_class, (const char* selector, const char* className), {
    var sel = UTF8ToString(selector);
    var cls = UTF8ToString(className);
    var elem = document.querySelector(sel);
    if (elem) {
        elem.classList.add(cls);
    }
});

// Log to console
EM_JS(void, js_console_log, (const char* text), {
    console.log(UTF8ToString(text));
});

// Alert
EM_JS(void, js_alert_msg, (const char* text), {
    alert(UTF8ToString(text));
});


/* ============================================================================
 * MiLa Builtins wrapping these functions
 * ============================================================================
 */

Value* native_dom_set_text(Env* e, int argc, Value** argv)
{
    if (argc < 2) return vnull();
    js_set_text(GET_STRING(argv[0]), GET_STRING(argv[1]));
    return vnull();
}

Value* native_dom_get_text(Env* e, int argc, Value** argv)
{
    if (argc < 1) return vnull();
    const char* result = js_get_text(GET_STRING(argv[0]));
    Value* v = vstring_dup(result);
    free((void*)result);
    return v;
}

Value* native_dom_set_html(Env* e, int argc, Value** argv)
{
    if (argc < 2) return vnull();
    js_set_innerHTML(GET_STRING(argv[0]), GET_STRING(argv[1]));
    return vnull();
}

Value* native_dom_add_class(Env* e, int argc, Value** argv)
{
    if (argc < 2) return vnull();
    js_add_class(GET_STRING(argv[0]), GET_STRING(argv[1]));
    return vnull();
}

Value* native_console_log(Env* e, int argc, Value** argv)
{
    if (argc < 1) return vnull();
    js_console_log(GET_STRING(argv[0]));
    return vnull();
}

Value* native_alert(Env* e, int argc, Value** argv)
{
    if (argc < 1) return vnull();
    js_alert_msg(GET_STRING(argv[0]));
    return vnull();
}

/* Register all the web functions */
void env_register_web_ext(Env* e) {
    // DOM manipulation
    env_register_native(e, "dom.setText", native_dom_set_text);
    env_register_native(e, "dom.getText", native_dom_get_text);
    env_register_native(e, "dom.setHtml", native_dom_set_html);
    env_register_native(e, "dom.addClass", native_dom_add_class);
    
    // Console and alerts
    env_register_native(e, "console.log", native_console_log);
    env_register_native(e, "alert", native_alert);
}


/* ============================================================================
 * USAGE IN MiLa:
 * ============================================================================
 * 
 * // Set text in an element
 * dom.setText("#output", "Hello from MiLa!");
 * 
 * // Get text from an element
 * var text = dom.getText("#input");
 * println(text);
 * 
 * // Set HTML (dangerous with user input!)
 * dom.setHtml("#container", "<p>Hello</p>");
 * 
 * // Add CSS class
 * dom.addClass("#button", "active");
 * 
 * // Log to console
 * console.log("Debug message");
 * 
 * // Alert
 * alert("This is important!");
 * 
 * // Unsafe eval (use caution!)
 * dom.eval("document.title = 'New Title'");
 * 
 * ============================================================================
 */