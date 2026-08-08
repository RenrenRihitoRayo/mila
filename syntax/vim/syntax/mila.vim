" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn set var export return break continue contextual forget foreach alias const catch

" RT-Statements
syntax match milaKeyword "\v\@[a-zA-Z0-9._]+"

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax match milaBuiltins "\<str.caseless_contains\>\|\<thread.get_pthread_id\>\|\<thread.check_cancel\>\|\<thread.mutex_unlock\>\|\<_debug.get_weakrefs\>\|\<str.caseless_find\>\|\<str.match_replace\>\|\<ascii.from_string\>\|\<thread.set_daemon\>\|\<thread.mutex_lock\>"
syntax match milaBuiltins "\<list.deconstruct\>\|\<sys.get_platform\>\|\<fread_all_bytes\>\|\<str.match_findx\>\|\<hash._get_seed\>\|\<mjson.dumps_io\>\|\<str.startswith\>\|\<str.match_find\>\|\<str.substitute\>\|\<ascii.from_int\>"
syntax match milaBuiltins "\<_debug.get_mem\>\|\<hash.set_seed\>\|\<env.set_local\>\|\<env.get_names\>\|\<file.list_dir\>\|\<list.contains\>\|\<json.dumps_io\>\|\<report_tagged\>\|\<time_sleep_ms\>\|\<thread.cancel\>"
syntax match milaBuiltins "\<thread.status\>\|\<fprint_bytes\>\|\<file.is_file\>\|\<str.endswith\>\|\<str.contains\>\|\<get_tm_local\>\|\<sys.get_arch\>\|\<thread.mutex\>\|\<fread_bytes\>\|\<file.exists\>"
syntax match milaBuiltins "\<file.is_dir\>\|\<list.append\>\|\<from_opaque\>\|\<mjson.loads\>\|\<mjson.dumps\>\|\<str.toupper\>\|\<str.tolower\>\|\<_breakpoint\>\|\<sys.get_pid\>\|\<thread.make\>"
syntax match milaBuiltins "\<thread.join\>\|\<thread.dump\>\|\<fdredirect\>\|\<list.index\>\|\<list.slice\>\|\<array.from\>\|\<cast.float\>\|\<is_numeric\>\|\<json.loads\>\|\<json.dumps\>"
syntax match milaBuiltins "\<time_sleep\>\|\<get_tm_gmt\>\|\<sys.setenv\>\|\<sys.getenv\>\|\<readch_nb\>\|\<fread_all\>\|\<array.len\>\|\<dict.keys\>\|\<as_opaque\>\|\<str.slice\>"
syntax match milaBuiltins "\<str.index\>\|\<str.patch\>\|\<str.pop_f\>\|\<str.pop_b\>\|\<str.split\>\|\<repr_raw\>\|\<list.pop\>\|\<list.len\>\|\<dict.rem\>\|\<cast.int\>"
syntax match milaBuiltins "\<cast.str\>\|\<cast.i2f\>\|\<cast.i2u\>\|\<cast.u2i\>\|\<cast.f2i\>\|\<str.copy\>\|\<str.join\>\|\<str.find\>\|\<get_time\>\|\<strftime\>"
syntax match milaBuiltins "\<srandom\>\|\<crandom\>\|\<env.set\>\|\<env.get\>\|\<println\>\|\<str.len\>\|\<istring\>\|\<require\>\|\<random\>\|\<printr\>"
syntax match milaBuiltins "\<readch\>\|\<fdopen\>\|\<fileno\>\|\<fclose\>\|\<fprint\>\|\<fflush\>\|\<isatty\>\|\<typeof\>\|\<export\>\|\<report\>"
syntax match milaBuiltins "\<assert\>\|\<system\>\|\<invoke\>\|\<range\>\|\<qsort\>\|\<print\>\|\<input\>\|\<close\>\|\<fread\>\|\<fseek\>"
syntax match milaBuiltins "\<ftell\>\|\<array\>\|\<unown\>\|\<floor\>\|\<sqrtf\>\|\<atan2\>\|\<abort\>\|\<copy\>\|\<repr\>\|\<hash\>"
syntax match milaBuiltins "\<open\>\|\<list\>\|\<dict\>\|\<ceil\>\|\<sqrt\>\|\<rand\>\|\<fabs\>\|\<exit\>\|\<eval\>\|\<load\>"
syntax match milaBuiltins "\<and\>\|\<xor\>\|\<not\>\|\<own\>\|\<sin\>\|\<cos\>\|\<tan\>\|\<pow\>\|\<abs\>\|\<run\>"
syntax match milaBuiltins "\<or\>"

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
