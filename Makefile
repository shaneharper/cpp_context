LLVM_CONFIG=llvm-config-3.4 # XXX Hardcoded version. (Consider extracting version from `clang --version`.)

c++_context: main.c++ c++_context.c++ libclang++.h++ Makefile
	# XXX Yuck. Hardcoded lib paths for llvm v3.4
	clang++ -I`$(LLVM_CONFIG) --includedir` -std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -fPIC -fvisibility-inlines-hidden -Wall -W -Wno-unused-parameter -Wwrite-strings -Wmissing-field-initializers -pedantic -Wno-long-long -Wcovered-switch-default -Wnon-virtual-dtor -fcolor-diagnostics -ffunction-sections -fdata-sections -fno-common -Woverloaded-virtual -Wcast-qual -fno-strict-aliasing -Wno-nested-anon-types  -Wl,--gc-sections main.c++ -include c++_context.c++ -o c++_context /usr/lib/llvm-3.4/lib/libLLVMOption.a /usr/lib/llvm-3.4/lib/libLLVMSupport.a -lrt -ldl -ltinfo -lpthread -lz /usr/lib/llvm-3.4/lib/libclangAST.a /usr/lib/llvm-3.4/lib/libclangBasic.a /usr/lib/llvm-3.4/lib/libclangTooling.a /usr/lib/llvm-3.4/lib/libclangFrontend.a /usr/lib/llvm-3.4/lib/libclangDriver.a /usr/lib/llvm-3.4/lib/libclangParse.a /usr/lib/llvm-3.4/lib/libLLVMMCParser.a /usr/lib/llvm-3.4/lib/libclangSerialization.a /usr/lib/llvm-3.4/lib/libclangSema.a /usr/lib/llvm-3.4/lib/libclangEdit.a /usr/lib/llvm-3.4/lib/libclangAnalysis.a /usr/lib/llvm-3.4/lib/libLLVMBitReader.a /usr/lib/llvm-3.4/lib/libLLVMCore.a /usr/lib/llvm-3.4/lib/libclangAST.a /usr/lib/llvm-3.4/lib/libclangLex.a /usr/lib/llvm-3.4/lib/libclangBasic.a /usr/lib/llvm-3.4/lib/libLLVMMC.a /usr/lib/llvm-3.4/lib/libLLVMObject.a /usr/lib/llvm-3.4/lib/libLLVMSupport.a /usr/lib/llvm-3.4/lib/libclang.so

test: test.c++ c++_context.c++ libclang++.h++ Makefile precompiled_headers.h++.pch
	clang++ -include precompiled_headers.h++ -Wall -Wextra -pedantic -std=c++11 test.c++ -o test -include c++_context.c++ -I`$(LLVM_CONFIG) --includedir` `$(LLVM_CONFIG) --libdir`/libclang.so && ./test

precompiled_headers.h++.pch: precompiled_headers.h++ Makefile
	clang++ -x c++-header -std=c++11 -I`$(LLVM_CONFIG) --includedir` precompiled_headers.h++ -o precompiled_headers.h++.pch
