" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn set var export return break continue contextual forget foreach

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax match milaBuiltins "\<or\>\|\<is\>\|\<own\>\|\<abs\>\|\<sin\>"
syntax match milaBuiltins "\<pow\>\|\<and\>\|\<xor\>\|\<not\>\|\<tan\>"
syntax match milaBuiltins "\<cos\>\|\<run\>\|\<dict\>\|\<open\>\|\<copy\>"
syntax match milaBuiltins "\<list\>\|\<exit\>\|\<rand\>\|\<eval\>\|\<hash\>"
syntax match milaBuiltins "\<load\>\|\<fabs\>\|\<sqrt\>\|\<ceil\>\|\<repr\>"
syntax match milaBuiltins "\<range\>\|\<sqrtf\>\|\<floor\>\|\<noise\>\|\<qsort\>"
syntax match milaBuiltins "\<print\>\|\<input\>\|\<close\>\|\<fread\>\|\<fseek\>"
syntax match milaBuiltins "\<ftell\>\|\<abort\>\|\<unown\>\|\<array\>\|\<atan2\>"
syntax match milaBuiltins "\<printr\>\|\<fprint\>\|\<fclose\>\|\<typeof\>\|\<system\>"
syntax match milaBuiltins "\<invoke\>\|\<random\>\|\<assert\>\|\<report\>\|\<fflush\>"
syntax match milaBuiltins "\<fdopen\>\|\<_typeof\>\|\<crandom\>\|\<istring\>\|\<require\>"
syntax match milaBuiltins "\<str.len\>\|\<srandom\>\|\<println\>\|\<get_time\>\|\<list.pop\>"
syntax match milaBuiltins "\<list.len\>\|\<str.join\>\|\<str.find\>\|\<dict.rem\>\|\<cast.int\>"
syntax match milaBuiltins "\<cast.str\>\|\<cast.i2f\>\|\<cast.i2u\>\|\<cast.u2i\>\|\<cast.f2i\>"
syntax match milaBuiltins "\<repr_raw\>\|\<str.copy\>\|\<strftime\>\|\<str.slice\>\|\<str.patch\>"
syntax match milaBuiltins "\<str.split\>\|\<dict.keys\>\|\<array.len\>\|\<str.pop_f\>\|\<str.pop_b\>"
syntax match milaBuiltins "\<fread_all\>\|\<as_opaque\>\|\<str.index\>\|\<array.from\>\|\<fdredirect\>"
syntax match milaBuiltins "\<list.index\>\|\<list.slice\>\|\<cast.float\>\|\<is_numeric\>\|\<json.loads\>"
syntax match milaBuiltins "\<json.dumps\>\|\<time_sleep\>\|\<get_tm_gmt\>\|\<sys.get_pid\>\|\<fread_bytes\>"
syntax match milaBuiltins "\<mjson.dumps\>\|\<mjson.loads\>\|\<from_opaque\>\|\<thread.join\>\|\<list.append\>"
syntax match milaBuiltins "\<file.is_dir\>\|\<thread.make\>\|\<file.exists\>\|\<thread.dump\>\|\<_breakpoint\>"
syntax match milaBuiltins "\<sys.get_arch\>\|\<thread.mutex\>\|\<file.is_file\>\|\<str.endswith\>\|\<fprint_bytes\>"
syntax match milaBuiltins "\<get_tm_local\>\|\<str.contains\>\|\<list.contains\>\|\<file.list_dir\>\|\<thread.status\>"
syntax match milaBuiltins "\<thread.cancel\>\|\<time_sleep_ms\>\|\<hash.set_seed\>\|\<report_tagged\>\|\<ascii.from_int\>"
syntax match milaBuiltins "\<str.startswith\>\|\<hash._get_seed\>\|\<str.match_find\>\|\<fread_all_bytes\>\|\<list.deconstruct\>"
syntax match milaBuiltins "\<sys.get_platform\>\|\<ascii.from_string\>\|\<thread.mutex_lock\>\|\<thread.set_daemon\>\|\<str.caseless_find\>"
syntax match milaBuiltins "\<str.match_replace\>\|\<thread.check_cancel\>\|\<thread.mutex_unlock\>\|\<str.caseless_contains\>\|\<thread.get_pthread_id\>"syntax match milaBuiltins "\<dump_mila_search_paths\>"

" Constants
syntax keyword milaConstant true false null none
syntax keyword milaConstant stderr stdout SEEK_SET SEEK_END SEEK_CUR

" Numbers
syntax match milaNumber "\v<\d+(\.\d+)?u?>"

" Operators
syntax match milaOperator "[-+*/=<>!:%]=\?"
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
