// XXX Consider using "LibTooling" which provides functionality for parsing
// XXX command line arguments and looking up a "compilation database".
// XXX See http://clang.llvm.org/docs/LibTooling.html

#include "libclang++.h++"
#include <clang/Tooling/CompilationDatabase.h>
#include <algorithm> // find
#include <cstring> // strerror
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h> // chdir
#include <vector>


using CompilationEnv = clang::tooling::CompileCommand;


std::string file_extension(const std::string& s)
{
    const auto i = s.rfind('.');
    return (i == std::string::npos) ? "" : s.substr(i+1);
}

bool has_cpp_source_file_extension(const std::string& s)
{
    static const std::vector<std::string> cpp_source_file_extensions {"cpp", "c++", "cxx", "cc", "C", "c"};
    return std::find(cpp_source_file_extensions.begin(), cpp_source_file_extensions.end(), file_extension(s))
                    != cpp_source_file_extensions.end();
}


std::vector<const char*> get_environment_arguments(const CompilationEnv& env)
{
    std::vector<const char*> args;

    for (auto arg_it = env.CommandLine.begin() + 1 /*skip compiler*/;
         arg_it != env.CommandLine.end();
         ++arg_it)
    {
        // Filter out the source filename as we ultimately pass the source filename via a separate argument to clang_parseTranslationUnit, otherwise it won't create the translation unit.
        if ("-c" == *arg_it or "-o" == *arg_it)
        {
            ++arg_it;
        }
        else if (not ((*arg_it)[0] != '-' and has_cpp_source_file_extension(*arg_it)) /*XXX <- A hack (?) to check if arg isn't source file to compile and link - should probably use clang::tooling::CommonOptionsParser::getSourcePathList() */)
        {
            args.push_back(arg_it->c_str());
            if ("-include" == *arg_it)
            {
                args.push_back((++arg_it /*XXX*/)->c_str()); // (Ensure include files aren't filtered out.)
            }
        }
    }
    return args;
}


class CompilationEnvironments
{
    clang::tooling::CompilationDatabase* p_compilation_database{nullptr};
  public:
    ~CompilationEnvironments()
    {
        if (p_compilation_database)
            delete p_compilation_database;
    }
    CompilationEnvironments()
    {
        std::string error;
        p_compilation_database = clang::tooling::CompilationDatabase::autoDetectFromDirectory(/*SourceDir*/ ".", error);
        if (error.length())
        {
            throw std::runtime_error(error);
        }
    }

    CompilationEnvironments(const CompilationEnvironments&) = delete;
    CompilationEnvironments& operator=(const CompilationEnvironments&) = delete;

    CompilationEnv get_compile_environment(llvm::StringRef file_path)
    {
        auto compilation_commands = p_compilation_database->getCompileCommands(file_path);
        if (compilation_commands.size() == 0)
        {
            throw std::runtime_error("No compilation environment known for " + std::string(file_path));
        }

        if (compilation_commands.size() > 1)
        {
            // XXX maybe some of the vector elements are "equivalent" - remove "redundant" elements, e.g. for an include file that is included multiple times with the same macros, search paths, etc.
            // XXX need to be able to pick which one to use if there are still multiple after filtering "redundant" elements. Perhaps a search string could be provided, e.g. "-DDEBUG", which would hopefully filter out just one match.
        }

        return compilation_commands[0];
    }
};


void change_directory(const char* path)
{
    if (chdir(path))
    {
        throw std::runtime_error(std::string("chdir failed. ") + strerror(errno));
    }
}


void output_context(const char* pathname, const size_t query_offset)
{
    const auto compilation_environment = CompilationEnvironments().get_compile_environment(pathname);
    // Check compiler used?  if (not is_clang(compilation_environment.CommandLine[0])) { XXX }
    change_directory(compilation_environment.Directory.c_str());

    Libclang::TranslationUnitContext translation_unit_context;
    Libclang::TranslationUnit translation_unit(translation_unit_context, pathname,
            get_environment_arguments(compilation_environment),
            /*unsaved_files*/ {},
            /*options*/ CXTranslationUnit_None);

//    if (clang_getNumDiagnostics(translation_unit)) {}
    std::cout << get_context(translation_unit, query_offset);
}


int main(int argc, char* argv[])
{
    const char* usage_message = "Usage: pathname zero-based_offset\n"; // XXX It currently appears that pathname must be an absolute path - or maybe it just has to match character-for-character an entry in the compile_commands.json file. Should clang::tooling::CompilationDatabase::getCompileCommands() expand relative paths?

    if (argc != 2+1)
    {
        std::cerr << usage_message;
        return 1;
    }

    size_t query_offset;
    {
        std::istringstream iss(argv[2]);
        if (not (iss >> query_offset))
        {
            std::cerr << usage_message;
            return 2;
        }
    }

    output_context(/*pathname*/ argv[1], query_offset);
}
