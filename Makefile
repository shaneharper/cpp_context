test: test.c++ c++_context.c++ libclang++.h++ Makefile
	clang++ -Wall -Wextra -pedantic -std=c++11 test.c++ -o test -include c++_context.c++ -I/usr/lib/llvm-3.4/include /usr/lib/llvm-3.4/lib/libclang.so && ./test
