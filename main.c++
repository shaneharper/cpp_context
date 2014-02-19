#include "libclang++.h++"
#include <clang/Tooling/CompilationDatabase.h>
#include <cstring> // strerror
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unistd.h> // chdir
#include <vector>


using CompilationEnv = clang::tooling::CompileCommand;

std::vector<const char*> get_environment_arguments(const CompilationEnv& env)
{
    std::vector<const char*> args;

    for (auto arg_it = env.CommandLine.begin() + 1 /*skip compiler*/;
         arg_it != env.CommandLine.end();
         ++arg_it)
    {
        if ("-o" == *arg_it or "-c" == *arg_it)
        {
            ++arg_it;
        }
        else
        {
            args.push_back(arg_it->c_str());
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
    const char* usage_message = "Usage: pathname zero-based_offset\n";

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
