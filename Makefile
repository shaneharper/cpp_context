test: test.c++ c++_context.c++ libclang++.h++ Makefile precompiled_headers.h++.pch
	clang++ -include precompiled_headers.h++ -Wall -Wextra -pedantic -std=c++11 test.c++ -o test -include c++_context.c++ -I/usr/lib/llvm-3.4/include /usr/lib/llvm-3.4/lib/libclang.so && ./test

precompiled_headers.h++.pch: precompiled_headers.h++ Makefile
	clang++ -x c++-header -std=c++11 -I/usr/lib/llvm-3.4/include precompiled_headers.h++ -o precompiled_headers.h++.pch
