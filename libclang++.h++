// C++ classes and utility functions built upon the C libclang API.

#pragma once

#include <clang-c/Index.h>
#include <cstring> // strlen
#include <stdexcept>
#include <vector>


namespace Libclang
{
    class Index
    {
        CXIndex index;
      public:
        Index(bool exclude_declarations_from_PCH = true, bool display_diagnostics = true)
          : index{clang_createIndex(exclude_declarations_from_PCH, display_diagnostics)}
        {
            if (!index)
            {
                throw std::runtime_error("clang_createIndex() failed.");
            }
        }

        ~Index()
        {
            clang_disposeIndex(index);
        }

        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;

        operator CXIndex()
        {
            return index;
        }
    };

    using TranslationUnitContext = Index;


    struct UnsavedFile : CXUnsavedFile
    {
        UnsavedFile(const char* filename, const char* contents)
          : UnsavedFile{filename, contents, strlen(contents)}
        {
        }
        UnsavedFile(const char* filename, const char* contents, size_t content_length)
          : CXUnsavedFile{filename, contents, content_length}
        {
        }
    };


    class TranslationUnit
    {
        CXTranslationUnit translation_unit;
      public:
        TranslationUnit(
                TranslationUnitContext& translation_unit_context,
                const char *source_filename,
                const std::vector<const char*>& command_line_args,
                /*const*/ std::vector<UnsavedFile> unsaved_files /* contents and filenames as specified by CXUnsavedFile are copied when necessary - client only needs to guarantee their validity until the call to this function returns. */,
                unsigned /*See enum CXTranslationUnit_Flags*/ options)
          : translation_unit{clang_parseTranslationUnit(
                      translation_unit_context, source_filename,
                      command_line_args.data(), command_line_args.size(),
                      unsaved_files.data(), unsaved_files.size(),
                      options)}
        {
            if (!translation_unit)
            {
                throw std::runtime_error("clang_parseTranslationUnit() failed.");
            }
        }

        ~TranslationUnit()
        {
            clang_disposeTranslationUnit(translation_unit);
        }

        TranslationUnit(const TranslationUnit&) = delete;
        TranslationUnit& operator=(const TranslationUnit&) = delete;

        operator CXTranslationUnit() { return translation_unit; }

        CXCursor get_cursor() const
        {
            return clang_getTranslationUnitCursor(translation_unit);
        }
    };


    class String
    {
        CXString string;
      public:
        String(const CXString& string) : string(string) { }
        ~String()
        {
            clang_disposeString(string);
        }
        operator const char*() const
        {
            return clang_getCString(string);
        }
    };


    enum class NextNode { None = CXChildVisit_Break,
                          Sibling /*without visiting current node's children*/ = CXChildVisit_Continue,
                          Child = CXChildVisit_Recurse };

    template<typename T_Function>
    void visit_children(CXCursor cursor, T_Function function)
    {
        clang_visitChildren(cursor,
            [](CXCursor cursor, CXCursor parent, CXClientData visit_fn_)
            {
                const T_Function& visit_fn = *static_cast<T_Function*>(visit_fn_);
                const NextNode next_node = visit_fn(cursor, parent);
                return static_cast<CXChildVisitResult>(next_node);
            },
            &function);
    }


    String get_display_name(const CXCursor& cursor)
    {
        return clang_getCursorDisplayName(cursor);
    }


    size_t get_offset(const CXSourceLocation& location)
    {
        unsigned offset;
        clang_getFileLocation(location, /*file*/ NULL, /*line*/ NULL, /*column*/ NULL, &offset);
        return offset;
    }

    CXFile get_file(const CXSourceLocation& location)
    {
        CXFile file;
        clang_getFileLocation(location, &file, /*line*/ NULL, /*column*/ NULL, /*offset*/ NULL);
        return file;
    }


    size_t get_start_offset(const CXSourceRange& extent)
    {
        return get_offset(clang_getRangeStart(extent));
    }

    size_t get_one_beyond_end_offset(const CXSourceRange& extent)
    {
        return get_offset(clang_getRangeEnd(extent));
    }


    std::string operator+(const String& s, const char* t)
    {
        return static_cast<const char*>(s) + std::string{t};
    }

    std::string operator+(const char* s, const String& t)
    {
        return std::string{s} + static_cast<const char*>(t);
    }


    CXFile get_file(CXTranslationUnit translation_unit, const char* file_name)
    {
        if (CXFile f = clang_getFile(translation_unit, file_name))
            return f;
        throw std::runtime_error("clang_getFile() failed.");
    }
}
