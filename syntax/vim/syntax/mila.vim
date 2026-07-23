" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn set var export return break continue contextual forget

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax match milaBuiltins "range\|own\|unown\|copy\|repr\|repr_raw\|random\|srandom\|noise\|crandom\|dump_mila_search_paths\|is\|hash\|hash.set_seed\|hash._get_seed\|print\|printr\|println\|input\|and\|or\|xor\|not\|open\|fdopen\|fdredirect\|fclose\|close\|fprint\|fprint_bytes\|fread\|fread_all\|fread_bytes\|fread_all_bytes\|fseek\|ftell\|fflush\|file.exists\|file.is_file\|file.is_dir\|file.list_dir\|list\|list.pop\|list.len\|list.append\|list.contains\|list.index\|list.slice\|list.deconstruct\|array\|array.from\|array.len\|dict\|dict.rem\|dict.keys\|cast.int\|cast.float\|cast.str\|cast.i2f\|cast.i2u\|cast.u2i\|cast.f2i\|typeof\|_typeof\|is_numeric\|as_opaque\|from_opaque\|mjson.loads\|mjson.dumps\|json.loads\|json.dumps\|str.slice\|str.index\|str.patch\|str.copy\|str.len\|str.pop_f\|str.pop_b\|str.split\|str.join\|str.startswith\|str.endswith\|str.contains\|str.caseless_contains\|str.find\|str.caseless_find\|str.match_replace\|str.match_find\|istring\|ascii.from_int\|ascii.from_string\|floor\|ceil\|sqrt\|sqrtf\|sin\|cos\|tan\|atan2\|pow\|rand\|fabs\|abs\|report\|report_tagged\|assert\|exit\|abort\|get_time\|time_sleep\|time_sleep_ms\|strftime\|get_tm_gmt\|get_tm_local\|_breakpoint\|system\|sys.get_platform\|sys.get_arch\|sys.get_pid\|run\|require\|invoke\|load\|eval\|thread.make\|thread.join\|thread.cancel\|thread.check_cancel\|thread.set_daemon\|thread.get_pthread_id\|thread.status\|thread.mutex\|thread.mutex_unlock\|thread.mutex_lock\|thread.dump"

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
