" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn set var export return break continue contextual forget foreach

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax match milaBuiltins "\<dump_mila_search_paths\>\|\<thread.get_pthread_id\>\|\<str.caseless_contains\>\|\<thread.check_cancel\>\|\<_debug.get_weakrefs\>\|\<thread.mutex_unlock\>\|\<str.caseless_find\>\|\<str.match_replace\>\|\<ascii.from_string\>\|\<thread.set_daemon\>"
syntax match milaBuiltins "\<thread.mutex_lock\>\|\<sys.get_platform\>\|\<list.deconstruct\>\|\<fread_all_bytes\>\|\<str.match_find\>\|\<hash._get_seed\>\|\<str.startswith\>\|\<ascii.from_int\>\|\<mjson.dumps_io\>\|\<_debug.get_mem\>"
syntax match milaBuiltins "\<list.contains\>\|\<thread.status\>\|\<thread.cancel\>\|\<hash.set_seed\>\|\<time_sleep_ms\>\|\<report_tagged\>\|\<json.dumps_io\>\|\<file.list_dir\>\|\<get_tm_local\>\|\<sys.get_arch\>"
syntax match milaBuiltins "\<fprint_bytes\>\|\<str.contains\>\|\<str.endswith\>\|\<thread.mutex\>\|\<file.is_file\>\|\<mjson.dumps\>\|\<thread.join\>\|\<mjson.loads\>\|\<str.tolower\>\|\<from_opaque\>"
syntax match milaBuiltins "\<file.exists\>\|\<str.toupper\>\|\<file.is_dir\>\|\<_breakpoint\>\|\<list.append\>\|\<sys.get_pid\>\|\<thread.dump\>\|\<thread.make\>\|\<fread_bytes\>\|\<sys.getenv\>"
syntax match milaBuiltins "\<fdredirect\>\|\<list.index\>\|\<list.slice\>\|\<array.from\>\|\<cast.float\>\|\<is_numeric\>\|\<json.loads\>\|\<json.dumps\>\|\<time_sleep\>\|\<get_tm_gmt\>"
syntax match milaBuiltins "\<sys.setenv\>\|\<str.split\>\|\<str.patch\>\|\<str.index\>\|\<str.pop_b\>\|\<dict.keys\>\|\<str.slice\>\|\<fread_all\>\|\<str.pop_f\>\|\<as_opaque\>"
syntax match milaBuiltins "\<array.len\>\|\<repr_raw\>\|\<strftime\>\|\<get_time\>\|\<list.len\>\|\<list.pop\>\|\<cast.int\>\|\<str.copy\>\|\<str.join\>\|\<cast.i2f\>"
syntax match milaBuiltins "\<cast.i2u\>\|\<cast.u2i\>\|\<str.find\>\|\<cast.f2i\>\|\<dict.rem\>\|\<cast.str\>\|\<srandom\>\|\<crandom\>\|\<_typeof\>\|\<require\>"
syntax match milaBuiltins "\<istring\>\|\<println\>\|\<str.len\>\|\<assert\>\|\<fdopen\>\|\<printr\>\|\<report\>\|\<fflush\>\|\<isatty\>\|\<random\>"
syntax match milaBuiltins "\<readch\>\|\<system\>\|\<invoke\>\|\<typeof\>\|\<fprint\>\|\<fclose\>\|\<print\>\|\<ftell\>\|\<fseek\>\|\<fread\>"
syntax match milaBuiltins "\<close\>\|\<floor\>\|\<sqrtf\>\|\<atan2\>\|\<abort\>\|\<unown\>\|\<input\>\|\<range\>\|\<vkill\>\|\<qsort\>"
syntax match milaBuiltins "\<noise\>\|\<array\>\|\<fabs\>\|\<rand\>\|\<dict\>\|\<open\>\|\<sqrt\>\|\<load\>\|\<eval\>\|\<copy\>"
syntax match milaBuiltins "\<hash\>\|\<ceil\>\|\<repr\>\|\<exit\>\|\<list\>\|\<xor\>\|\<pow\>\|\<tan\>\|\<cos\>\|\<own\>"
syntax match milaBuiltins "\<not\>\|\<and\>\|\<run\>\|\<sin\>\|\<abs\>\|\<is\>\|\<or\>"

" Constants
syntax keyword milaConstant true false null none
syntax keyword milaConstant stderr stdout SEEK_SET SEEK_END SEEK_CUR

" Numbers
syntax match milaNumber "\v<\d+(\.\d+)?u?>"

" Operators
syntax match milaOperator "[-+*/=<>!:%?]=\?"
syntax match milaOperator "??"

" Strings

syntax region milaString start='"' end='"' contains=milaEscape
syntax match milaEscape "\\[nrt\"'\\]" contained

" Comments
syntax keyword milaTodo TODO FIXME NOTE contained
syntax region milaMLComment start="//" end="$" contains=milaTodo
" syntax match milaComment "//.*\$" contains=milaTodo containedin=ALL
syntax region milaMLComment start="/\*" end="\*/" contains=milaTodo


" Define Highlighting
highlight link milaKeyword Keyword
highlight link milaConstant Constant
highlight link milaNumber Number
highlight link milaOperator Operator
highlight link milaString String
highlight link milaEscape SpecialChar
highlight link milaComment Comment
highlight link milaMLComment Comment
highlight link milaTodo Todo
highlight link milaBuiltins Function

function! GetMyIndent()
    let lnum = v:lnum
    let prev_lnum = prevnonblank(lnum - 1)
    if prev_lnum <= 0
        return 0
    endif

    let prev_line = getline(prev_lnum)
    let prev_indent = indent(prev_lnum)

    " --- 1) Try syntax-based detection ---
    let syn_id = synID(prev_lnum, strlen(prev_line), 1)
    let syn_name = synIDattr(syn_id, "name")

    let is_indent_syn = syn_name =~# 'keyword_indent'
    let is_dedent_syn = syn_name =~# 'keyword_dedent'

    " --- 2) Text-based fallback (stable) ---
    let is_indent_txt = prev_line =~ '{\s*$'
    let is_dedent_txt = prev_line =~ '}\s*$'

    " If previous line opens a block
    if is_indent_syn || is_indent_txt
        return prev_indent + &shiftwidth
    endif

    " If current line starts with a closing brace
    let cur_line = getline(lnum)
    if cur_line =~ '^\s*}'
        return prev_indent - &shiftwidth
    endif

    return prev_indent
endfunction

setlocal indentexpr=GetMyIndent()
setlocal autoindent
set shiftwidth=4
let b:current_syntax = "mila"
