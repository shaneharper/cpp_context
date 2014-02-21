# C++ Context

_C++ Context_ was written to aid software developers _quickly_ and _accurately_ comprehend C++ code.

_C++ Context_ takes as input a position in a C++ source file and outputs the corresponding "context", viz. the scope (namespace, class, and/or function). _C++ Context_ effectively "filters" context information from the source code text reducing the amount of text for the software developer to read.

_C++ Context_ could be used:
* to annotate the output of a search tool such as 'grep'.
* to annotate a text editor's "tag stack", adding context to the "jumped from" locations.
* when viewing long, unfamiliar function or class definitions - context could be shown in a status bar or at the top of the window.

_C++ Context_ uses libclang.


## Getting Started

On Ubuntu 13.10:

    sudo apt-get install clang-3.4 clang-3.4-dev
    cd cpp_context && make


### Compilation database

`c++_context` uses a `compile_commands.json` file to find which compiler options are needed for each source file. `c++_context` searches for `compile_commands.json` in the current directory, and then the parent directories.

See:
* [Bear](https://github.com/rizsotto/Bear). (I had to `sudo apt-get install libconfig8-dev` before building bear.)
* http://clang.llvm.org/docs/HowToSetupToolingForLLVM.html

At the time of writing, cmake and bear don't output entries in `compile_commands.json` for header files. :-(


### Using _C++ Context_ with Vim editor

`c++_context.vim` maps `<localleader>c` to display the context for the current cursor position. (`<localleader>` defaults to `\`.)

    :source /path/to/c++_context.vim
can be added to your .vimrc.
