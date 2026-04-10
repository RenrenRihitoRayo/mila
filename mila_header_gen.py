#!/usr/bin/env python3

from clang import cindex
import sys
import os
import re

typedefs = set()
enums = set()
structs = set()

type_maps = {}
enums_value = {}
use_libffi = False

array_normalize_pattern = re.compile(r"\d+")

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

mila_to_c = {
    "char[]": "{value}->v.s",
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

mila_to_c_ptr = {
    "char[]": "&{value}->v.s",
    "char *": "&{value}->v.s",
    "const char *": "&{value}->v.s",
    "int": "((int*)&{value}->v.i)",
    "long": "&{value}->v.i",
    "float": "&{value}->v.f",
    "double": "&{value}->v.f",
    "void *": "&{value}->v.opaque",
    "const void*": "&{value}->v.opaque",
    "unsigned int": "&{value}->v.i",
    "_Bool": "&{value}->v.b",
    "bool": "&{value}->v.b",
    "unsigned char": "&(unsigned char)({value}->v.i)",
    "Value *": "&{value}"
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
    "Value *": "(Value*){value}",
    "char[]": "vstring_dup({value})"
}

c_to_ffi = {
    "char *": "ffi_type_pointer",
    "long": "ffi_type_slong",
    "unsigned long": "ffi_type_ulong",
    "double": "ffi_type_double",
    "_Bool": "ffi_type_int",
    "bool": "ffi_type_int",
    "void *": "ffi_type_pointer",
    "int": "ffi_type_sint",
    "unsigned int": "ffi_type_uint",
}

# --------------------------------------------------
# HELPERS
# --------------------------------------------------

def remove_array_sizes(text):
    return re.sub(array_normalize_pattern, "", text)

def normalize_type(t):
    return type_maps.get(t, t).replace("const ", "").replace("restrict", "")

def collect_types(cursor):
    def visit(node):

        # typedefs
        if node.kind == cindex.CursorKind.TYPEDEF_DECL and node.spelling:
            decl_type = node.underlying_typedef_type
            decl_cursor = decl_type.get_declaration()

            if decl_cursor.spelling:
                typedefs.add(node.spelling)
                type_maps[node.spelling] = decl_cursor.spelling
            else:
                typedefs.add(node.spelling)
                type_maps[node.spelling] = "void*"

        # structs
        elif node.kind == cindex.CursorKind.STRUCT_DECL and node.spelling:
            structs.add(node.spelling)

        # enums
        elif node.kind == cindex.CursorKind.ENUM_DECL and node.spelling:
            enums.add(node.spelling)
            for const in node.get_children():
                if const.kind == cindex.CursorKind.ENUM_CONSTANT_DECL:
                    enums_value[f"{node.spelling}.{const.spelling}"] = const.enum_value

        for c in node.get_children():
            visit(c)

    visit(cursor)

def make_struct_constructor(cursor):
    if cursor.kind != cindex.CursorKind.STRUCT_DECL:
        raise ValueError("Cursor must be a struct declaration")

    struct_name = cursor.spelling

    if " " in repr(struct_name):
        return "", []

    fields = []
    for child in cursor.get_children():
        if child.kind == cindex.CursorKind.FIELD_DECL:
            fields.append((child.type, child.spelling))

    names = [(struct_name, f"_type_mila_{struct_name}_new")]
    pre_lines = []
    lines = [f"Value* _type_mila_{struct_name}_new(Env* e, int argc, Value** argv) {{",
             f"    (void)e;",
             f"    if(argc != {len(fields)}) return verror(\"{struct_name}_new: expected {len(fields)} arguments, got %i\", argc);",
             f"    {struct_name}* tmp = ({struct_name}*)malloc(sizeof({struct_name}));"]

    for i, (ftype, fname) in enumerate(fields):
        if ftype.kind == cindex.TypeKind.CONSTANTARRAY:
            ftype = type("cindex.Type", (object,), {
                "kind": ftype.kind,
                "get_array_size": ftype.get_array_size,
                "get_array_element_type": ftype.get_array_element_type,
                "spelling": remove_array_sizes(ftype.spelling)
            })
        fname_access = f"(({struct_name}*)(argv[0]->v.opaque))->"
        names.append((f"{struct_name}.get_{fname}", f"_type_mila_{struct_name}_get_{fname}"))
        names.append((f"{struct_name}.set_{fname}", f"_type_mila_{struct_name}_set_{fname}"))
        pre_lines.extend([
            f"Value* _type_mila_{struct_name}_get_{fname}(Env* env, int argc, Value** argv) {{",
            f"    if (argc != 1) verror(\"{struct_name}.get_{fname}({struct_name} s): Expected 1 argument!\");",
            f"    return {c_to_mila.get(ftype.spelling, 'vopaque(' + ('&' if not ftype.spelling.endswith('*') else '' ) + '{value})').format(value=fname_access+fname)};"
             "}",
             ""
        ])
        pre_lines.extend([
            f"Value* _type_mila_{struct_name}_set_{fname}(Env* env, int argc, Value** argv) {{",
            f"    if (argc != 2) verror(\"{struct_name}.get_{fname}({struct_name} s, {ftype.spelling} {fname}): Expected 2 arguments!\");",
            f"    {fname_access+fname} = {mila_to_c.get(ftype.spelling, '{value}').format(value=f'argv[1]')};"
            if ftype.kind != cindex.TypeKind.CONSTANTARRAY else
            (   f"    memcpy({fname_access+fname}, {mila_to_c.get(ftype.spelling, '{value}').format(value=f'argv[1]')}, sizeof({fname_access+fname}));"
                if not ftype.spelling.startswith("char") else
                f"    strncpy({fname_access+fname}, {mila_to_c.get(ftype.spelling, '{value}').format(value=f'argv[1]')}, sizeof({fname_access+fname}));"
            ),
             "    return vnull();",
             "}",
             ""
        ])
        # Handle arrays
        if ftype.kind == cindex.TypeKind.CONSTANTARRAY:
            size = ftype.get_array_size()
            str_ftype = normalize_arrays(ftype).spelling
            str_ftype = mila_to_c.get(str_ftype, f"{{value}}->v.opaque").format(value=f"argv[{i}]")
            # Use memcpy from argv[i] opaque value
            if ftype.spelling.startswith("char"):
                lines.append(f"    strncpy(tmp->{fname}, {str_ftype}, sizeof(tmp->{fname}));")
            else:
                lines.append(f"    memcpy(tmp->{fname}, {str_ftype}, sizeof(tmp->{fname}));")


        # Handle nested structs
        elif ftype.kind == cindex.TypeKind.RECORD and ftype.get_declaration().kind == cindex.CursorKind.STRUCT_DECL:
            nested_name = ftype.spelling or ftype.get_declaration().spelling
            lines.append(f"    tmp->{fname} = *({nested_name}*)argv[{i}]->v.opaque;")

        # Handle numeric / pointer types
        else:
            conv = mila_to_c.get(ftype.spelling, f"*(({ftype.spelling}*)argv[{i}]->v.opaque)")
            if ftype.spelling == "char *":
                lines.append(f"    tmp->{fname} = mila_strdup({conv.format(value=f'argv[{i}]')});")
            else:
                lines.append(f"    tmp->{fname} = {conv.format(value=f'argv[{i}]')};")

    lines.append(f"    return vowned_opaque_extra(tmp, NULL, \"struct {struct_name}\");")
    lines.append("}")
    
    return "\n".join(pre_lines + lines + [""]), names
def gen_variadic_wrapper(func):
    global use_libffi
    use_libffi = True
    name = func["name"]
    ret = func["return"]
    fixed_amount = len(func["params"])

    types = []
    lines = []

    for i, p in enumerate(func["params"]):
        t = p["type"]
        types.append(f"{t} {p['name']}")

        if t in typedefs or t in structs:
            lines.append(f"(({t}*)argv[{i}]->v.opaque)")
        elif t in enums:
            lines.append(f"({t}*)&argv[{i}]->v.i")
        else:
            lines.append(
                mila_to_c_ptr.get(t, "{value}->v.opaque").format(value=f"argv[{i}]")
            )
        lines[-1] = f"types[{i}] = &{c_to_ffi.get(t, 'ffi_type_pointer')};\n    values[{i}] = "\
        + lines[-1] + ";"

    return f"""
Value* native_mila_{name}(Env* e, int argc, Value** argv) {{
    (void)e;

    if (argc < {fixed_amount})
        return verror("{name}({', '.join(types)}, ...): missing required arguments");

    ffi_cif cif;

    int fixed = {fixed_amount};
    int total = argc;

    ffi_type *types[total];
    void *values[total];

    {(chr(10)+'    ').join(lines)}

    for (int i = fixed; i < argc; i++) {{
        Value *v = argv[i];

        switch(v->type) {{
            case T_INT:
                types[i] = &ffi_type_slong;
                values[i] = &v->v.i;
                break;

            case T_UINT:
                types[i] = &ffi_type_ulong;
                values[i] = &v->v.ui;
                break;

            case T_FLOAT:
                types[i] = &ffi_type_double;
                values[i] = &v->v.f;
                break;

            case T_BOOL:
                types[i] = &ffi_type_sint;
                values[i] = &v->v.b;
                break;

            case T_OWNED_OPAQUE:
            case T_OPAQUE:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.opaque;
                break;

            case T_STRING:
                types[i] = &ffi_type_pointer;
                values[i] = &v->v.s;
                break;

            default:
                return verror("Passed an unsuported type `%s` to `{name}`", GET_TYPENAME(argv[i]));
        }}
    }}

    {(ret + ' res;') if ret != "void" else ""}

    ffi_prep_cif_var(
        &cif,
        FFI_DEFAULT_ABI,
        fixed,
        total,
        &{c_to_ffi.get(ret, "ffi_type_void")},
        types
    );

    ffi_call(
        &cif,
        FFI_FN({name}),
        {"&res" if ret != "void" else "NULL"},
        values
    );

    return {c_to_mila.get(ret, "vopaque({value})").format(value="res") if ret != "void" else "vnull()"};
}}
"""

def gen_normal_wrapper(func):
    name = func["name"]
    ret = func["return"]
    params = func["params"]

    arg_list = []
    types = []

    for i, p in enumerate(params):
        t = p["type"]
        types.append(f"{t} {p['name']}")

        if t in typedefs or t in structs:
            arg_list.append(f"*(({t}*)argv[{i}]->v.opaque)")
        elif t in enums:
            arg_list.append(f"({t})argv[{i}]->v.i")
        else:
            arg_list.append(
                mila_to_c.get(t, "{value}->v.opaque").format(value=f"argv[{i}]")
            )

    if ret == "void":
        return f"""
Value* native_mila_{name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if(argc != {len(params)})
        return verror("{name}: wrong arg count");

    {name}({', '.join(arg_list)});
    return vnull();
}}
"""
    elif ret in structs or ret in typedefs:
        return f"""
Value* native_mila_{name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if(argc != {len(params)})
        return verror("{name}: wrong arg count");
    {ret}* res = ({ret}*)malloc(sizeof({ret})); 
    {ret} tmp = {name}({', '.join(arg_list)});
    memcpy(res, &tmp, sizeof({ret}));
    return vopaque(res);
}}
"""
    return f"""
Value* native_mila_{name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if(argc != {len(params)})
        return verror("{name}: wrong arg count");

    {ret} res = {name}({', '.join(arg_list)});
    return {c_to_mila.get(ret, "vopaque({value})").format(value="res")};
}}
"""

def gen_wrapper(func):
    if func["variadic"]:
        return gen_variadic_wrapper(func)
    return gen_normal_wrapper(func)

def gen_wrap(file_path, module_name=None):
    functions = parse_functions(file_path)

    code = []
    names = []

    # generate function wrappers
    for func in functions:
        if func["name"].startswith("_"):
            continue

        func_name = func["name"]
        if func["variadic"]:
            code.append(gen_variadic_wrapper(func))
        else:
            code.append(gen_normal_wrapper(func))

        names.append((func_name, f"native_mila_{func_name}"))

    # generate struct constructors
    index = cindex.Index.create()
    tu = index.parse(file_path, args=["-I."])
    for cursor in tu.cursor.get_children():
        if cursor.kind == cindex.CursorKind.STRUCT_DECL and cursor.spelling in structs:

            ctor_code, ctor_exports = make_struct_constructor(cursor)
            code.append(ctor_code)
            names.extend(ctor_exports)

    # emit native table
    res = "\n".join(code)
    res += "\nconst NativeEntry lib_function_entries[] = {\n"
    for name, alias in names:
        if module_name:
            name = f"{module_name}.{name}"
        res += f'    {{"{name}", {alias}}},\n'
    res += "    {NULL, NULL}\n};\n"

    # emit enum constants
    res += "\nvoid _mila_lib_init(Env* e) {\n"
    for name, value in enums_value.items():
        if module_name:
            name = f"{module_name}.{name}"
        res += f'    env_set_raw(e, "{name}", vint({value}));\n'
    res += "}\n"

    return res

def parse_functions(filename):
    index = cindex.Index.create()
    tu = index.parse(filename, args=["-I.", "-nostdinc"])

    collect_types(tu.cursor)

    functions = []

    def visit(node):
        if node.kind == cindex.CursorKind.FUNCTION_DECL:

            functions.append({
                "name": node.spelling,
                "return": node.result_type.spelling,
                "variadic": node.type.is_function_variadic(),
                "params": [
                    {"name": a.spelling, "type": normalize_type(a.type.spelling)}
                    for a in node.get_arguments()
                ]
            })

        for c in node.get_children():
            visit(c)

    visit(tu.cursor)
    return functions

if len(sys.argv) < 2:
    print("usage: gen.py <file.h> <namespace>")
    sys.exit(1)

file_path = os.path.abspath(sys.argv[1])
out_path = file_path.replace(".h", ".mila-wrap.c")

final = f'#define ML_LIB\n#include "mila.c"\n#include "{file_path}"\n\n'
final += gen_wrap(file_path, sys.argv[2] if len(sys.argv) == 3 else None)
final = ("#include <ffi.h>\n" if use_libffi else "") + final

with open(out_path, "w") as f:
    f.write(final)