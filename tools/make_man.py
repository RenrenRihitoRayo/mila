#!/usr/bin/env python3
import re
import sys
from pathlib import Path

main_man = """
.TH MILA 1 MILA
.SH NAME
mila - a small interpreting programming language for automation and embedding
.SH SYNOPSIS
mila [-r code] [-h] [--check file] [--version] [--dry] [--info] [--help] [file] [arguments...]
.SH DESCRIPTION
A small C-based programming language interpreter meant for automation, scripting and embedding.
.SH COMMAND LINE OPTIONS
.TP
.B \\-\\-info
Show information about the build of the interpreter
.TP
.B \\-\\-help | \\-h
Help
.TP
.B \\-\\-check [file]
Check the syntax of the file without running it.
.TP
.B \\-\\-dry
Dry run the interpreter, full init, print hello world, then deinit
.TP
.B \\-\\-version | \\-v
Output the version
.TP
.B \\-r [code]
Run the string as code.
.SH SMALL EXAMPLES
.TP
.B Hello world
.EX
println("Hello, world!");
.EE
.TP
.B Setting variables
.EX
var name; // initializes name to 'none'
set name = "Ren"; // set name
const name = name; // make name constant
// reading unset / mising values return 'null'
.EE
.TP
.B Functions
.EX
var lambda = fn() {
    println("Hello from lambda!");
}

fn function() {
    println("Hello from function!");
}
.EE
.TP
.B Math
.EX
var test = 90;
var q = test / 90; // 1.0
var p = q * 90; // 90.0
var s = p + 80; // 170.0
var d = s - 70; // 100.0
var r = cast.i2f(d) % 1; // 0

/*
    Math functions also exist if you check the BUILTINS>MATH section.
*/
.EE
.TP
.B Control flow
.EX
if (cond) {
    body;
} elif (cond2) {
    body;
} else {
    body;
}

while (cond) {
    body;
}

foreach item : list {
    body;
}
.EE
.TP
.B Collections
.EX
Lists [item1, item2, item3, ...]
// We use [@ ...] as its already used for code blocks
Dicts [@ key1=val1, key2=val2, key3=val3, ...]
.EE
.SH EXIT STATUS
.TP
.B 1
Syntax Error (E_SYNTAX_ERROR)
.TP
.B 2
Preruntime Error (E_PRE_RUNTIME)
.TP
.B 3
Runtime Error (E_RUNTIME)
.TP
.B 4
Assertion Error (E_ASSERT)
.TP
.B 5
Thread Halt (E_THREAD_HALT)
.TP
.B 6
Exit (E_EXIT)
.TP
.B NOTE
Programs can use their own exit codes, but in the case of errors
these are the standard exit codes.
.SH BUILTINS
"""


def parse_doc(content):
    """Parse markdown doc into sections with functions."""
    sections = {}
    current_section = None
    lines = content.split('\n')
    i = 0
    
    while i < len(lines):
        line = lines[i]
        
        # Parse section header
        if line.startswith('## <a id="'):
            match = re.search(r'id="([^"]+)"', line)
            if match:
                current_section = match.group(1)
                sections[current_section] = {}

        if current_section == "str-pattern":
            i += 1
            sections.pop(current_section)
            current_section = None
            continue
        
        # Parse function entry
        elif line.startswith('* `') and current_section:
            match = re.match(
                r'\* `([A-Za-z_][A-Za-z0-9_.]*\([^`]*\)(?: -> "[^"]+")?)`',
                line
            )
            if match:
                func_sig = match.group(1)
                func_name = func_sig.split('(')[0]
                
                # Get description from following lines
                desc_lines = []
                i += 1
                
                # Skip blank lines
                while i < len(lines) and not lines[i].strip():
                    i += 1
                
                # Collect description (indented lines)
                while i < len(lines):
                    desc_line = lines[i]
                    if desc_line and not desc_line.startswith('* `') and not desc_line.startswith('## '):
                        desc_lines.append(desc_line.strip())
                        i += 1
                    else:
                        break
                desc = '\n'.join(desc_lines).strip()
                sections[current_section][func_name] = {"sig": func_sig, "desc": desc}
                continue
        
        i += 1
    
    return sections

def make_man_page(name, sig, synopsis, desc):
    """Generate man page content."""
    upper = f'MILA_{name.upper().replace(".", "_")}'
    
    man = f""".TH {upper} 1 "MiLa Builtins"
.SH NAME
.B {name}
.SH SYNOPSIS
.EX
\\fB{sig}\\fR
.EE
.SH DESCRIPTION
.EX
{repr(desc)[1:-1]}
.EE
.SH SEE ALSO
.BR mila (1)
""" if desc else f""".TH {upper} 1 "MiLa Builtins"
.SH NAME
.B {name}
.SH SYNOPSIS
.EX
\\fB{sig}\\fR
.EE
.SH DESCRIPTION
\\fINo description\\fR
.SH SEE ALSO
.BR mila (1)
"""
    return man

def main():
    global main_man
    # Get script directory
    script_dir = Path(__file__).parent.resolve()
    
    # Default doc path relative to script
    doc_file = script_dir / '../docs/Builtins.md'
    
    # Command line override
    if len(sys.argv) > 1:
        doc_file = Path(sys.argv[1]).resolve()
    
    if not doc_file.exists():
        print(f"Error: {doc_file} not found", file=sys.stderr)
        sys.exit(1)
    
    content = doc_file.read_text()
    sections = parse_doc(content)
    
    out_dir = Path('mila-man').resolve()
    if out_dir.exists():
        import shutil
        shutil.rmtree(out_dir)
    out_dir.mkdir(exist_ok=True)
    
    count = 0
    for section, funcs in sections.items():
        main_man += f"\n.SS {section.upper()}"
        for func_name, desc in funcs.items():
            if not desc:
                continue
            
            clean_name = func_name.replace('.', '-')
            man_file = out_dir / f'mila-{clean_name}.1'
            
            man_content = make_man_page(func_name, desc["sig"], f'mila-{clean_name}()', desc["desc"])
            man_file.write_text(man_content)
            count += 1
            main_man += f"\n.TP\n.B mila-{clean_name}(1)\n{desc['sig']}"

    main_man += "\n"
    (out_dir / "mila.1").write_text(main_man)
    
    print(f"Generated {count+1} man pages in {len(sections)} sections")
    print(f"Output: {out_dir}")

if __name__ == '__main__':
    main()
