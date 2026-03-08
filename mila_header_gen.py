#!/usr/bin/env python3

import pycparser
from pycparser import c_generator

mila_to_c = {
    "char*": "{value}->v.s",
    "const char*": "{value}->v.s",
    "int": "(int){value}->v.i",
    "long": "{value}->v.i",
    "float": "{value}->v.f",
    "double": "{value}->v.f",
    "void*": "{value}->v.opaque"
}

c_to_mila = {
    "char*": "vstring_dup({value})",
    "const char*": "vstring_take({value})",
    "int": "vint((long){value})",
    "long": "vint({value})",
    "float": "vfloat((double){value})",
    "double": "vfloat({value})",
    "void*": "vopaque({value})"
}

def gen_wrap(code, module_name=None):
    # Parse the C code
    parser = pycparser.CParser()
    
    code = "\n".join(filter(
        lambda s: s.strip() and not s.strip().startswith("#")
                            and not s.strip().startswith("/")
                            and not s.strip().startswith("*"),
        code.split("\n")
    ))
    
    ast = parser.parse(code)

    # Extract function definitions
    functions = []
    for node in ast.ext:
        if isinstance(node, pycparser.c_ast.FuncDef):
            functions.append(node)

    # Analyze each function
    generator = c_generator.CGenerator()
    code = []
    names = []
    for func in functions:
        param = []
        types = []
        func_name = func.decl.name
        ret = get_type_name(func.decl.type.type)
        if func.decl.type.args:
            for pos, p in enumerate(func.decl.type.args.params):
                name = p.name
                type_ = get_type_name(p.type)
                types.append(type_)
                param.append(mila_to_c.get(type_, type_).format(value=f"argv[{pos}]"))
        if ret == "void":
            code.append(f"Value* mila_{func_name}(Env* e, int argc, Value** argv) {{\n    (void)e;\n"
                        f"    if (argc != {len(param)}) return verror(\"{func_name}({', '.join(types)}): Expected {len(param)} arguments but got %i\", argc);\n"
                        f"    {func_name}({', '.join(param)});\n"
                         "    return vnull();\n"
                         "}\n")
        else:
            code.append(f"Value* mila_{func_name}(Env* e, int argc, Value** argv) {{\n    (void)e;\n"
                        f"    if (argc != {len(param)}) return verror(\"{func_name}({', '.join(types)}): Expected {len(param)} argumemts but got %i\", argc);\n"
                        f"    {ret} res = {func_name}({', '.join(param)});\n"
                        f"    return {c_to_mila.get(ret, ret).format(value='res')};\n"
                         "}\n")
        names.append((func_name, f"mila_{func_name}"))
    res = "\n".join(code)
    res += "\nconst NativeEntry lib_function_entries[] = {\n"
    for name, alias in names:
        if module_name:
            name = f"{module_name}.{name}"
        res += f"    {{\"{name}\", {alias}}},\n"
    res = res[:-2] + "\n};"
    return res

def get_type_name(type_node):
    if isinstance(type_node, pycparser.c_ast.IdentifierType):
        type_name = ' '.join(type_node.names)
        return type_name
    elif isinstance(type_node, pycparser.c_ast.PtrDecl):
        return f"{get_type_name(type_node.type)}*"
    elif isinstance(type_node, pycparser.c_ast.TypeDecl):
        type_name = get_type_name(type_node.type)
        if type_node.quals:
            type_name = ' '.join(type_node.quals) + ' ' + type_name
        return type_name
    else:
        return str(type_node)

# Example usage
import sys, os

if len(sys.argv) == 2:
    f = os.path.abspath(sys.argv[1])
    d = os.path.join(os.path.dirname(f), os.path.basename(f) + ".mila-wrap.c")
    code = open(f).read()
    res = f'#include "{f}"\n#include "mila.c"\n\n'+gen_wrap(code, "test")+"\n"
    with open(d, "w") as f:
        f.write(res)