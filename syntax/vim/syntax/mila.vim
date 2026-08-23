" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn set var const return break continue contextual forget foreach alias const catch alias

" RT-Statements
syntax match milaKeyword "\v\@[a-zA-Z0-9._]+"

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax match milaBuiltins "\<or\>\|\<map\>\|\<and\>\|\<xor\>\|\<not\>\|\<own\>\|\<sin\>\|\<cos\>\|\<tan\>\|\<pow\>"
syntax match milaBuiltins "\<abs\>\|\<run\>\|\<copy\>\|\<repr\>\|\<hash\>\|\<open\>\|\<list\>\|\<dict\>\|\<ceil\>\|\<sqrt\>"
syntax match milaBuiltins "\<rand\>\|\<fabs\>\|\<exit\>\|\<eval\>\|\<load\>\|\<range\>\|\<qsort\>\|\<print\>\|\<input\>\|\<close\>"
syntax match milaBuiltins "\<fread\>\|\<fseek\>\|\<ftell\>\|\<array\>\|\<unown\>\|\<floor\>\|\<sqrtf\>\|\<atan2\>\|\<abort\>\|\<random\>"
syntax match milaBuiltins "\<export\>\|\<printr\>\|\<fdopen\>\|\<fileno\>\|\<fclose\>\|\<fprint\>\|\<fflush\>\|\<isatty\>\|\<typeof\>\|\<report\>"
syntax match milaBuiltins "\<assert\>\|\<system\>\|\<invoke\>\|\<srandom\>\|\<crandom\>\|\<env.set\>\|\<env.get\>\|\<println\>\|\<str.len\>\|\<istring\>"
syntax match milaBuiltins "\<require\>\|\<repr_raw\>\|\<list.pop\>\|\<list.len\>\|\<dict.rem\>\|\<cast.int\>\|\<cast.str\>\|\<cast.i2f\>\|\<cast.i2u\>\|\<cast.u2i\>"
syntax match milaBuiltins "\<cast.f2i\>\|\<str.copy\>\|\<str.join\>\|\<str.find\>\|\<get_time\>\|\<strftime\>\|\<fread_all\>\|\<array.len\>\|\<dict.keys\>\|\<as_opaque\>"
syntax match milaBuiltins "\<str.slice\>\|\<str.index\>\|\<str.patch\>\|\<str.pop_f\>\|\<str.pop_b\>\|\<str.split\>\|\<fdredirect\>\|\<list.index\>\|\<list.slice\>\|\<array.from\>"
syntax match milaBuiltins "\<cast.float\>\|\<is_numeric\>\|\<json.loads\>\|\<json.dumps\>\|\<time_sleep\>\|\<get_tm_gmt\>\|\<sys.setenv\>\|\<sys.getenv\>\|\<fread_bytes\>\|\<file.exists\>"
syntax match milaBuiltins "\<file.is_dir\>\|\<list.append\>\|\<from_opaque\>\|\<mjson.loads\>\|\<mjson.dumps\>\|\<str.toupper\>\|\<str.tolower\>\|\<_breakpoint\>\|\<sys.get_pid\>\|\<thread.make\>"
syntax match milaBuiltins "\<thread.join\>\|\<thread.dump\>\|\<env.get_type\>\|\<fprint_bytes\>\|\<file.is_file\>\|\<file.resolve\>\|\<str.endswith\>\|\<str.contains\>\|\<get_tm_local\>\|\<sys.get_arch\>"
syntax match milaBuiltins "\<thread.mutex\>\|\<hash.set_seed\>\|\<env.set_local\>\|\<env.get_names\>\|\<file.list_dir\>\|\<list.contains\>\|\<json.dumps_io\>\|\<report_tagged\>\|\<time_sleep_ms\>\|\<thread.cancel\>"
syntax match milaBuiltins "\<thread.status\>\|\<hash._get_seed\>\|\<mjson.dumps_io\>\|\<str.startswith\>\|\<str.match_find\>\|\<str.substitute\>\|\<ascii.from_int\>\|\<_debug.get_mem\>\|\<fread_all_bytes\>\|\<str.match_findx\>"
syntax match milaBuiltins "\<list.deconstruct\>\|\<sys.get_platform\>\|\<str.caseless_find\>\|\<str.match_replace\>\|\<ascii.from_string\>\|\<thread.set_daemon\>\|\<thread.mutex_lock\>\|\<thread.check_cancel\>\|\<thread.mutex_unlock\>\|\<_debug.get_weakrefs\>"
syntax match milaBuiltins "\<str.caseless_contains\>\|\<thread.get_pthread_id\>"

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
