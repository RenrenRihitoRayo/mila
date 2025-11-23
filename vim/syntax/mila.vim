" Vim syntax file for MiLa (Minimal Language)
" Save as ~/.vim/syntax/mila.vim

" Keywords
syntax keyword milaKeyword if elif else while fn let 

syntax keyword keyword_indent "{"
syntax keyword keyword_dedent "}"

" Builtins
syntax keyword milaBuiltins xor and or not
syntax keyword milaBuiltins println printr print input
syntax keyword milaBuiltins array dict typeof _typeof
syntax match milaBuiltins "cast\\.(int\\|float\\|string)"
syntax match milaBuiltins "array\\.(get\|set\\|len\\|free)"
syntax match milaBuiltins "dict\\.(get\\|set\\|rem\\|free)"
syntax keyword milaBuiltins open fread fprint ftell fseek
syntax keyword milaBuiltins floor ceil pow tan cos sin atan2 sqrt
syntax match milaBuiltins "str\\.(slice\\|index\\|patch\\|length\\|pop_f\\|pop_b)"
syntax match milaBuiltins "ascii\\.(to\\|from)"

" Constants
syntax keyword milaConstant true false null none
syntax keyword milaConstant stderr stdout SEEK_SET SEEK_END SEEK_CUR

" Numbers
syntax match milaNumber "\v<\d+(\.\d+)?>"

" Operators
syntax match milaOperator "[-+*/=<>!:]=\?"

" Strings

syntax region milaString start='"' end='"' contains=milaEscape
syntax match milaEscape "\\[nrt\"'\\]" contained

" Comments
syntax keyword milaTodo TODO FIXME contained
syntax match milaComment "//.*$" contains=milaTodo containedin=ALL
syntax region milaMLComment start="/\\*" end="\\*/" contains=milaTodo


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
