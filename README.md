C++ Context
===========

_C++ Context_ was written to aid software developers _quickly_ and _correctly_ comprehend C++ code.

_C++ Context_ takes as input a position in a C++ source file and outputs the corresponding "context", viz. the scope (namespace, class, and/or function). _C++ Context_ effectively "filters" context information from the source code text reducing the amount of text for the software developer to read.

_C++ Context_ could be used:
* to annotate the output of a search tool such as 'grep'.
* to annotate a text editor's "tag stack", adding context to the "jumped from" locations.
* when viewing long, unfamiliar function or class definitions - context could be shown in a status bar or at the top of the window.

_C++ Context_ uses libclang.
