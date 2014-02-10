#include "libclang++.h++"
#include <cstring> // strlen
#include <iostream>
#include <string>


static unsigned num_test_failures = 0;

static std::string get_context(const char* source_code, const size_t query_offset)
{
    Libclang::TranslationUnitContext translation_unit_context;
    Libclang::TranslationUnit translation_unit(translation_unit_context, "unsaved.c++",
            /*command_line_args*/ {"-std=c++11"},
            /*unsaved_files*/ {{"unsaved.c++", source_code, strlen(source_code)}},
            /*options*/ CXTranslationUnit_None);

    return clang_getNumDiagnostics(translation_unit) ? "Libclang generated diagnostic messages."
        : get_context(translation_unit.get_cursor(), query_offset);
}

void test(const char* test_name,
          const char* source_code_with_HERE_denoting_query_position,
          const char* expected_output)
{
    const auto query_offset = std::string(source_code_with_HERE_denoting_query_position).find("HERE>");

    if (query_offset == std::string::npos)
    {
        std::cout << test_name << " test is broken." << std::endl << std::endl;
        ++num_test_failures;
        return;
    }

    const auto output = get_context(
        std::string(source_code_with_HERE_denoting_query_position).replace(query_offset, strlen("HERE>"), "").c_str(),
        query_offset);

    if (output != expected_output)
    {
        std::cout << test_name << " test failed." << std::endl
              << "Expected: " << std::endl << expected_output //<< std::endl
              << "Actual Output: " << std::endl << output << std::endl
              << std::endl;
        ++num_test_failures;
    }
}


int main()
{
    test("Global scope",
            "HERE>\nint main(int argc, char* argv[]) { }\n",
         "");

    test("Global scope - after a function",
            "\nint main() { }HERE>\n\n",
         "");


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

    test("member function",
            "struct S { void doit() { HERE>; } };\n",
         "struct S\ndoit()\n");

    test("constructor",
            "struct S { int i; S(int i) : i(i) { HERE>; } };\n",
         "struct S\nS(int)\n");

    test("destructor",
            "struct S {  ~S() { HERE>; } };\n",
         "struct S\n~S()\n");

    test("conversion function",
            "struct S { operator int() { HERE>return 42; } };\n",
         "struct S\noperator int()\n");

    test("template function",
            "template<typename T> T fn(const T& v) { HERE>return v+1; }\n",
         "fn(const T &)\n");


    test("namespace",
            "namespace MyNamespace\n"
            "{\n"
            "   void doit()\n"
            "   {HERE> }\n"
            "}\n",
         "namespace MyNamespace\n"
         "doit()\n");


    test("class",
            "class X\n"
            "{\n"
            "   int secret;\n"
            "  HERE>public: X();\n"
            "};\n",
         "class X\n");

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

    test("enum",
            "enum E { ONE, TWO, HERE>THREE };\n",
         "enum E\n");

    test("enum class",
            "enum class E : char { ONE, TWO, HERE>THREE };\n",
         "enum E\n");


    if (num_test_failures == 0)
    {
        std::cout << "Ok" << std::endl;
    }
    return num_test_failures;
}
