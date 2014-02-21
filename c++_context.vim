let cpp_context_path = expand('<sfile>:p:h')

function! Cursor_byte_offset_from_start_of_file()
    return (line2byte(line("."))-1) + (col(".")-1)
endfunction


" XXX Unsaved changes will be ignored! XXX
autocmd FileType c,cpp
    \ nnoremap <buffer> <silent> <localleader>c
    \   :echo "-----\n"
    \    .system(shellescape(cpp_context_path."/c++_context")
    \            ." ".shellescape(expand('%:p'))
    \            ." ".Cursor_byte_offset_from_start_of_file())<CR>
