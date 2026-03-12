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

def collect_enums(cursor, tu):
    def visit(node):
        if node.kind == cindex.CursorKind.TYPEDEF_DECL and node.spelling:
            print(node.spelling)
            decl = node.underlying_typedef_type.get_declaration()
            # print(decl.kind, node.spelling, " ".join(t.spelling for t in tu.get_tokens(extent=node.extent)))
            if decl.spelling:
                mila_to_c[normalize_type(node.spelling)] = f"*(({normalize_type(node.spelling)}*)({{value}}->v.opaque))"
                type_maps[normalize_type(node.spelling)] = normalize_type(decl.spelling)
                typedefs.add(normalize_type(node.spelling))
            else:
                mila_to_c[normalize_type(node.spelling)] = f"({" ".join(t.spelling for t in tu.get_tokens(extent=node.extent))})({{value}}->v.opaque)"
                typedefs.add(node.spelling)
                type_maps[node.spelling] = "void*"
        elif node.kind == cindex.CursorKind.STRUCT_DECL and node.spelling:
            structs.add(node.spelling)
            mila_to_c[node.spelling] = f"{node.spelling}*"
        elif node.kind == cindex.CursorKind.ENUM_DECL and node.spelling:
            enums.add(node.spelling)

        for c in node.get_children():
            visit(c)

    visit(cursor)
    return enums


def normalize_type(t):
    return type_maps.get(t, t)

def parse_functions(filename):
    index = cindex.Index.create()
    tu = index.parse(filename,
    args=[
        "-nostdinc",  # optional: skip standard includes
        "-I.",        # only include current directory
    ],
    options=cindex.TranslationUnit.PARSE_INCOMPLETE)

    enums = collect_enums(tu.cursor, tu)
    functions = []

    def visit(node):
        if node.kind == cindex.CursorKind.FUNCTION_DECL:
            func = {
                "name": node.spelling,
                "return": normalize_type(node.result_type.spelling),
                "params": []
            }

            # print(func["name"])
            for arg in node.get_arguments():
                t = arg.type.spelling
                # print(" ", arg.spelling, arg.type.spelling, "=>", t)

                func["params"].append({
                    "name": arg.spelling,
                    "type": normalize_type(t)
                })

            functions.append(func)

        for child in node.get_children():
            visit(child)

    visit(tu.cursor)
    return functions

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
    "_Bool": "{value}->v.b"
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
    "_Bool": "vbool({value})"  
}


# def parse_functions(filename):
#     index = cindex.Index.create()
#     tu = index.parse(filename)

#     functions = []

#     def visit(node):
#         if node.kind == cindex.CursorKind.FUNCTION_DECL:
#             func = {
#                 "name": node.spelling,
#                 "return": node.result_type.spelling,
#                 "params": []
#             }

#             for arg in node.get_arguments():
#                 func["params"].append({
#                     "name": arg.spelling,
#                     "type": arg.type.spelling
#                 })

#             functions.append(func)

#         for child in node.get_children():
#             visit(child)

#     visit(tu.cursor)
#     return functions

def gen_wrap(file_path, module_name=None):
    functions = parse_functions(file_path)

    code = []
    names = []

    for func in functions:
        func_name = func["name"]
        ret = func["return"]

        param = []
        types = []

        for pos, p in enumerate(func["params"]):
            type_ = p["type"]
            types.append(f"{type_} {p['name']}")
            if type_ in typedefs:
                param.append(
                    f"*(({type_}*)(argv[{pos}]->v.opaque))"
                )
            elif type_ in structs:
                param.append(
                    f"*(({type_}*)(argv[{pos}]->v.opaque))"
                )
            elif type_ in enums:
                param.append(
                    f"({type_})(argv[{pos}]->v.i)"
                )
            else:
                param.append(
                    mila_to_c.get(type_, f"*(({type_}*){{value}}->v.opaque)").format(value=f"argv[{pos}]")
                )

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
        elif ret in structs and ret.endswith("*"):
            code.append(
f"""Value* mila_{func_name}(Env* e, int argc, Value** argv) {{
    (void)e;
    if (argc != {len(param)})
        return verror("{func_name}({', '.join(types)}): Expected {len(param)} arguments but got %i", argc);
    {ret} res = {func_name}({', '.join(param)});
    return vopaque(res);
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

    res = "\n".join(code)

    res += "\nconst NativeEntry lib_function_entries[] = {\n"
    for name, alias in names:
        if module_name:
            name = f"{module_name}.{name}"
        res += f'    {{"{name}", {alias}}},\n'
    res += "    {NULL, NULL}\n};"

    return res


if len(sys.argv) == 2:
    f = os.path.abspath(sys.argv[1])
    d = os.path.join(
        os.path.dirname(f),
        os.path.basename(f)[:-2] + ".mila-wrap.c"
    )

    res = f'#include "{f}"\n#include "mila.c"\n\n'
    res += gen_wrap(f) + "\n"

    with open(d, "w") as out:
        out.write(res)

elif len(sys.argv) == 3:
    f = os.path.abspath(sys.argv[1])
    d = os.path.join(
        os.path.dirname(f),
        os.path.basename(f)[:-2] + ".mila-wrap.c"
    )

    res = f'#include "{f}"\n#include "mila.c"\n\n'
    res += gen_wrap(f, sys.argv[2]) + "\n"

    with open(d, "w") as out:
        out.write(res)