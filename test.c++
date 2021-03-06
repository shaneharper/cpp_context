#include "libclang++.h++"
#include <cstring> // strlen
#include <iostream>
#include <string>


unsigned test_failure_count = 0;


std::string get_context(const char* header_text, const char* source_text, const size_t source_text_offset)
{
    Libclang::TranslationUnitContext translation_unit_context;
    Libclang::TranslationUnit translation_unit(translation_unit_context, "test_program.c++",
            /*command_line_args*/ {"-std=c++11"},
            /*unsaved_files*/ {{"test_program.c++", source_text},
                               {"/header.h++", header_text}},
            /*options*/ CXTranslationUnit_None);

    return clang_getNumDiagnostics(translation_unit) ? "Libclang generated diagnostic message/s."
        : get_context(translation_unit, source_text_offset);
}


void test(const char* test_name,
          const char* header_text,
          const char* source_text,
          size_t      source_text_offset,
          const char* expected_output)
{
    const auto output = get_context(header_text, source_text, source_text_offset);
    if (output != expected_output)
    {
        ++test_failure_count;
        std::cout << test_name << " test failed." << std::endl
              << "Expected: " << std::endl << expected_output
              << "Actual Output: " << std::endl << output
              << std::endl << std::endl;
    }
}


void test(const char* test_name,
          const char* header_text,
          const char* source_text_with_HERE_denoting_query_position,
          const char* expected_output)
{
    const auto source_text_offset = std::string(source_text_with_HERE_denoting_query_position).find("HERE>");

    if (source_text_offset == std::string::npos)
    {
        ++test_failure_count;
        std::cout << test_name << " test is broken." << std::endl << std::endl;
        return;
    }

    test(test_name, header_text,
         std::string(source_text_with_HERE_denoting_query_position).replace(source_text_offset, strlen("HERE>"), "").c_str(),
         source_text_offset,
         expected_output);
}


void test(const char* test_name,
          const char* source_text_with_HERE_denoting_query_position,
          const char* expected_output)
{
    test(test_name, /*header_text*/ "", source_text_with_HERE_denoting_query_position, expected_output);
}


void test_global_scope()
{
    test("Global scope",
            "HERE>\nint main(int argc, char* argv[]) { }\n",
         "");

    test("Global scope - after a function",
            "\nint main() { }HERE>\n\n",
         "");
}

void test_functions()
{
    test("function",
            "int main(int argc, char* argv[])\n"
            "{HERE> return 0; }\n",
         /*XXX "int " ?*/ "main(int, char **)\n");

    test("function - edge case, on closing brace",
            "int main() { return 0; HERE>}\n",
         "main()\n");

    test("function - second function definition",
            "void fn1() { }\n"
            "int main()\n"
            "{HERE> return 0; }\n"
            "void fn2() { }\n",
         "main()\n");

    test("template function",
            "template<typename T> T fn(const T& v) { HERE>return v+1; }\n",
         "fn" /*XXX "<T>"*/ "(const T &)\n");

    test("lambda function",
            "int main()\n"
            "{ auto f = [](bool){ HERE>return 42; }; }\n",
          "main()\n[]"/*XXX bool */"\n");

    // XXX Test fails: Libclang passes an CXCursor_UnexposedDecl for the friend function definition.
//    test("friend function defined inside friend class",
//            "class C {\n"
//            "    static int secret;\n"
//            "  public:\n"
//            "    friend int get() { HERE> return secret; }\n"
//            "};\n",
//         "class C\nfriend get()\n");

    test("friend function",
            "class C {\n"
            "    static int secret;\n"
            "  public:\n"
            "    friend int get();\n"
            "};\n"
            "int get() { HERE> return C::secret; }\n",
         "get()\n");
}

void test_member_functions()
{
    test("member function",
            "struct S { void doit() { HERE>; } };\n",
         "struct S\ndoit()\n" /*XXX or "S::doit()\n" ?*/);

    test("static member function",
            "struct S { static void doit() { HERE>; } };\n",
         "struct S\nstatic doit()\n");

    test("static member function defined out-of-class",
            "struct S { static void doit(); }; void S::doit() { HERE>; }\n",
         "static S::doit()\n");

    test("member function declaration",
            "struct S { void HERE>doit(); };\n"
            "void S::doit() {}\n",
         "struct S\ndoit()\n" /*XXX or "S::doit()\n", or just "struct S" (as we don't need to be told about the function declaration at the query location)?*/);

    test("member function defined out-of-class",
            "struct S { void doit(); };\n"
            "void S::doit() {HERE> }\n",
         "S::doit()\n");

    test("template member function defined out-of-class",
            "template<int N> struct S { void doit(); };\n"
            "template<int N> void S<N>::doit() {HERE> }\n",
         "S<N>::doit()\n");

    test("member function of nested class defined out-of-class",
            "class S { public: struct T { void doit(); }; };\n"
            "void S::T::doit() {HERE> }\n",
         "S::T::doit()\n");

    test("member function defined out-of-class within a namespace",
            "namespace N {\n"
            "    struct S { void doit(); };\n"
            "    void S::doit() {HERE> }\n"
            "}\n",
         "namespace N\nS::doit()\n");

    test("constructor",
            "struct S { int i; S(int i) : i(i) { HERE>; } };\n",
         "struct S\nS(int)\n");

    test("constructor defined out-of-class",
            "struct S { S(); };\n"
            "S::S() {HERE> }\n",
         "S::S()\n");

    test("destructor",
            "struct S {  ~S() { HERE>; } };\n",
         "struct S\n~S()\n");

    test("destructor defined out-of-class",
            "struct S { ~S(); };\n"
            "S::~S() {HERE> }\n",
         "S::~S()\n");

    test("conversion function",
            "struct S { operator int() { HERE>return 42; } };\n",
         "struct S\noperator int()\n");

    test("conversion function defined out-of-class",
            "struct S { operator int(); };\n"
            "S::operator int() { HERE>return 42; }\n",
         "S::operator int()\n");
}

void test_classes()
{
    test("class",
            "class X\n"
            "{\n"
            "   int secret;\n"
            "  HERE>public: X();\n"
            "};\n",
         "class X\n");

    test("subclass",
            "struct Base1 { int i; };\n"
            "struct Base2 { int j; };\n"
            "class Derived : public Base1, Base2 { HERE>public: int k; };\n",
         "class Derived" /*XXX " : public Base"*/ "\n");

    test("template class",
            "template<typename T> class C { public: HERE>T v; };\n",
          "class C<T>\n");

    test("struct",
            "struct X {HERE> X(); };\n",
         "struct X\n");

    test("template struct",
            "template<typename T> struct S { HERE>T v; };\n",
          "struct S<T>\n");

    test("template specialization",
            "template<typename T> struct S { int doit() { return 1; } };\n"
            "template<> struct S<bool> { int doit() { return 2; }HERE> };\n",
         "struct S<bool>\n");

    test("partial template specialization",
            "template<typename T> struct S { int doit() { return 1; } };\n"
            "template<typename T> struct S<T *> { int doit() { return 2; } HERE> };\n",
         "struct S<T *>\n");

    test("union",
            "union U { short s; HERE>long l; };\n",
         "union U\n");

    test("template union",
            "template<typename T> union U { HERE>T v; bool b; };\n",
          "union U<T>\n");
}

void test_enums()
{
    test("enum",
            "enum E { ONE, TWO, HERE>THREE };\n",
         "enum E\n");

    test("enum class",
            "enum class E : char { ONE, TWO, HERE>THREE };\n",
         "enum E\n");
}

void test_miscellaneous()
{
    test("namespace",
            "namespace MyNamespace\n"
            "{\n"
            "   void doit()\n"
            "   {HERE> }\n"
            "}\n",
         "namespace MyNamespace\n"
         "doit()\n");

    test("include file",
            /*header.h++*/
            "void doit()\n"
            "{\n"
            "   // ------------------------------------------------------------------------\n"
            "}\n",
            /*main file*/
            "#include \"/header.h++\"\n"
            "int main() { HERE>doit(); }\n",
         "main()\n");

    test("include",
            /*header.h++*/ "struct S { int i; };\n",
            /*main file*/  "#include HERE>\"/header.h++\"\n",
         "");

    test("struct spanning two files",
            /*header.h++*/
            "extern int v;\n"
            "struct Never {\n",
            /*main file*/
            "#include \"/header.h++\"\n"
            " HERE>int i; };\n",
         "struct Never\n");

#if 0
    // The "struct" appears twice in the AST: once at the top-level, and once as a child of "typedef C_Struct". At the time of writing, "struct" was returned twice by get_context(), i.e. both "struct" nodes were visited.
    test("typedef of a struct",
            "typedef struct { HERE>int i; } C_Struct;\n",
         /*XXX "typedef C_Struct\n"*/"struct S\n");
#endif

    test("macro expansion",
            "#define MY_STRUCT(NAME) struct My##NAME\n"
            "HERE>MY_STRUCT(JustAnInt) { int i; };\n",
         "struct MyJustAnInt\n");
}


int main()
{
    test_global_scope();
    test_functions();
    test_member_functions();
    test_classes();
    test_enums();
    test_miscellaneous();

    if (test_failure_count == 0)
    {
        std::cout << "Ok" << std::endl;
    }
    return test_failure_count;
}
