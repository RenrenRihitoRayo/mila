#!/usr/bin/env python3

from clang import cindex
import sys
import os

# If libclang cannot be found automatically, uncomment and set path
cindex.Config.set_library_file("/usr/lib/libclang.so.21.1.8")

typedefs = set()
enums = set()
structs = set()

type_maps = {}

enums_value = {}

mila_to_c = {
    "char *": "{value}->v.s",
    "const char *": "{value}->v.s",
    "int": "((int){value}->v.i)",
    "long": "{value}->v.i",
    "float": "{value}->v.f",
    "double": "{value}->v.f",
    "void *": "{value}->v.opaque",
    "const void*": "{value}->v.opaque",
    "unsigned int": "{value}->v.i",
    "_Bool": "{value}->v.b",
    "bool": "{value}->v.b",
    "unsigned char": "(unsigned char)({value}->v.i)",
    "Value *": "{value}"
}

c_to_mila = {
    "char *": "vstring_dup({value})",
    "const char *": "vstring_take({value})",
    "int": "vint((long){value})",
    "long": "vint({value})",
    "float": "vfloat((double){value})",
    "double": "vfloat({value})",
    "void *": "vopaque({value})",
    "const void *": "vopaque({value})",
    "unsigned int": "vint({value})",
    "_Bool": "vbool({value})",
    "bool": "vbool({value})",
    "unsigned char": "vint({value})",
    "Value *": "(Value*){value}"
}

# -------------------
# STRUCT CONSTRUCTOR
def make_struct_constructor(cursor):
    if cursor.kind != cindex.CursorKind.STRUCT_DECL:
        raise ValueError("Cursor must be a struct declaration")

    struct_name = cursor.spelling

    if " " in repr(struct_name):
        return ""

    fields = []
    for child in cursor.get_children():
        if child.kind == cindex.CursorKind.FIELD_DECL:
            fields.append((child.type, child.spelling))

    lines = [f"Value* mila_{struct_name}_new(Env* e, int argc, Value** argv) {{",
             f"    (void)e;",
             f"    if(argc != {len(fields)}) return verror(\"{struct_name}_new: expected {len(fields)} arguments, got %i\", argc);",
             f"    {struct_name} tmp;"]

    for i, (ftype, fname) in enumerate(fields):
        # Handle arrays
        if ftype.kind == cindex.TypeKind.CONSTANTARRAY:
            elem_type = ftype.get_array_element_type()
            size = ftype.get_array_size()
            str_ftype = normalize_arrays(ftype).spelling
            str_ftype = mila_to_c.get(str_ftype, f"{{value}}->v.opaque").format(value=f"argv[{i}]")
            # Use memcpy from argv[i] opaque value
            lines.append(f"    memcpy(tmp.{fname}, {str_ftype}, sizeof(tmp.{fname}));")

        # Handle nested structs
        elif ftype.kind == cindex.TypeKind.RECORD and ftype.get_declaration().kind == cindex.CursorKind.STRUCT_DECL:
            nested_name = ftype.spelling or ftype.get_declaration().spelling
            lines.append(f"    tmp.{fname} = *({nested_name}*)argv[{i}]->v.opaque;")

        # Handle numeric / pointer types
        else:
            conv = mila_to_c.get(ftype.spelling, f"*(({ftype.spelling}*)argv[{i}]->v.opaque)")
            lines.append(f"    tmp.{fname} = {conv.format(value=f'argv[{i}]')};")

    lines.append(f"    {struct_name}* ret = ({struct_name}*)mila_malloc(sizeof({struct_name}));")
    lines.append("    memcpy(ret, &tmp, sizeof(tmp));")
    lines.append(f"    return vowned_opaque_extra(ret, NULL, \"struct {struct_name}\");")
    lines.append("}")
    
    return "\n".join(lines)
# -------------------
# TYPE COLLECTION
# -------------------

def collect_types(cursor, tu):
    def visit(node):
        # Typedefs
        if node.kind == cindex.CursorKind.TYPEDEF_DECL and node.spelling:
            decl_type = node.underlying_typedef_type
            decl_cursor = decl_type.get_declaration()
            
            # Function pointer typedef
            if decl_type.kind == cindex.TypeKind.POINTER and decl_type.get_pointee().kind == cindex.TypeKind.FUNCTIONPROTO:
                typedefs.add(node.spelling)
                print(node.spelling)
                mila_to_c[node.spelling] = "(void*){value}->v.opaque"
                c_to_mila[node.spelling] = "vopaque((void*){value})"
                type_maps[node.spelling] = "void*"  # treat function pointers as opaque
            # Normal typedef
            elif decl_cursor.spelling:
                type_maps[node.spelling] = decl_cursor.spelling
                typedefs.add(node.spelling)
            else:
                typedefs.add(node.spelling)
                type_maps[node.spelling] = "void*"

        # Structs
        elif node.kind == cindex.CursorKind.STRUCT_DECL and node.spelling:
            structs.add(node.spelling)

        # Enums
        elif node.kind == cindex.CursorKind.ENUM_DECL and node.spelling:
            enums.add(node.spelling)
            for const in node.get_children():
                if const.kind == cindex.CursorKind.ENUM_CONSTANT_DECL:
                    enums_value[f"{node.spelling}.{const.spelling}"] = const.enum_value

        # Recurse
        for c in node.get_children():
            visit(c)

    visit(cursor)
    return enums

def normalize_arrays(t):
    if t.kind in (cindex.TypeKind.CONSTANTARRAY, cindex.TypeKind.INCOMPLETEARRAY):
        elem_type = t.get_array_element_type()
        # create a pointer type by getting canonical and adding pointer
        # Note: clang.cindex does not allow creating a new Type object, so 
        # in practice, you just treat it as "elem_type*" in spelling
        # For the purpose of wrapper generation, returning the element type
        # with an annotation works:
        class PointerType:
            def __init__(self, base):
                self.base = base
                self.spelling = base.spelling + " *"
        return PointerType(elem_type)
    return t
def normalize_type(t):
    t = type_maps.get(t, t)
    return t

def parse_functions(filename):
    index = cindex.Index.create()
    tu = index.parse(filename,
    args=[
        "-nostdinc",
        "-I.",
    ],
    options=cindex.TranslationUnit.PARSE_INCOMPLETE)

    collect_types(tu.cursor, tu)
    functions = []

    def visit(node):
        if node.kind == cindex.CursorKind.FUNCTION_DECL:
            func = {
                "name": node.spelling,
                "return": normalize_type(node.result_type.spelling),
                "params": []
            }
            for arg in node.get_arguments():
                t = arg.type.spelling
                func["params"].append({
                    "name": arg.spelling,
                    "type": normalize_type(t)
                })
            functions.append(func)
        for child in node.get_children():
            visit(child)

    visit(tu.cursor)
    return functions

# -------------------
# WRAPPER GENERATION
# -------------------
def gen_wrap(file_path, module_name=None):
    functions = parse_functions(file_path)

    code = []
    names = []

    # generate function wrappers
    for func in functions:
        if func["name"].startswith("_"): continue
        func_name = func["name"]
        ret = func["return"]
        param = []
        types = []
        for pos, p in enumerate(func["params"]):
            type_ = p["type"]
            types.append(f"{type_} {p['name']}")
            if type_ == "va_list":
                print("WHAT")
            elif type_ in typedefs or type_ in structs:
                param.append(f"*(({type_}*)(argv[{pos}]->v.opaque))")
            elif type_ in enums:
                param.append(f"({type_})(argv[{pos}]->v.i)")
            else:
                param.append(mila_to_c.get(type_, f"*(({type_}*){{value}}->v.opaque)").format(value=f"argv[{pos}]"))

        # function body generation (kept from your existing code)
        if ret == "void":
            code.append(
f"""Value* mila_{func_name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if (argc != {len(param)})
        return verror("{func_name}({', '.join(types)}): Expected {len(param)} arguments but got %i", argc);
    {func_name}({', '.join(param)});
    return vnull();
}}
""")
        elif ret in structs:
            code.append(
f"""Value* mila_{func_name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if (argc != {len(param)})
        return verror("{func_name}({', '.join(types)}): Expected {len(param)} arguments but got %i", argc);
    {ret} res = {func_name}({', '.join(param)});
    {ret}* tmp = ({ret}*)mila_malloc(sizeof({ret}));
    memcpy(tmp, &res, sizeof({ret}));
    return vopaque(tmp);
}}
""")
        else:
            code.append(f"""\
Value* mila_{func_name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if (argc != {len(param)})
        return verror("{func_name}({', '.join(types)}): Expected {len(param)} arguments but got %i", argc);
    {ret} res = {func_name}({', '.join(param)});
    return {c_to_mila.get(ret, "vopaque({value})").format(value='res')};
}}
""")
        names.append((func_name, f"mila_{func_name}"))

    # generate struct constructors
    index = cindex.Index.create()
    tu = index.parse(file_path, args=["-I."])
    for cursor in tu.cursor.get_children():
        if cursor.kind == cindex.CursorKind.STRUCT_DECL and cursor.spelling in structs:
            constructor_code = make_struct_constructor(cursor)
            if constructor_code:
                code.append(constructor_code)
                names.append((f"{cursor.spelling}", f"mila_{cursor.spelling}_new"))

    # generate native entries
    res = "\n".join(code)
    res += "\nconst NativeEntry lib_function_entries[] = {\n"
    for name, alias in names:
        if module_name:
            name = f"{module_name}.{name}"
        res += f'    {{"{name}", {alias}}},\n'
    res += "    {NULL, NULL}\n};\n"

    # generate enum constants
    res += "\nvoid _mila_lib_init(Env* e) {\n"
    for name, value in enums_value.items():
        if module_name:
            name = f"{module_name}.{name}"
        res += f'    env_set_raw(e, "{name}", vint({value}));\n'
    res += "}\n"

    return res

# -------------------
# MAIN
# -------------------
if len(sys.argv) >= 2:
    f = os.path.abspath(sys.argv[1])
    d = os.path.join(
        os.path.dirname(f),
        os.path.basename(f)[:-2] + ".mila-wrap.c"
    )

    module_name = sys.argv[2] if len(sys.argv) == 3 else None

    res = f'#define ML_LIB\n#include "mila.c"\n#include "{f}"\n\n'
    res += gen_wrap(f, module_name) + "\n"

    with open(d, "w") as out:
        out.write(res)