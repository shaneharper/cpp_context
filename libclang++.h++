// C++ classes and utility functions built upon the C libclang API.

#include <clang-c/Index.h>
#include <stdexcept>
#include <vector>


namespace Libclang
{
    class Index
    {
        CXIndex index;
      public:
        Index(bool exclude_declarations_from_PCH = true, bool display_diagnostics = true)
          : index(clang_createIndex(exclude_declarations_from_PCH, display_diagnostics))
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


    class TranslationUnit
    {
        CXTranslationUnit translation_unit;
      public:
        TranslationUnit(
                TranslationUnitContext& translation_unit_context,
                const char *source_filename,
                const std::vector<const char*>& command_line_args,
                /*const*/ std::vector<CXUnsavedFile> unsaved_files /* contents and filenames as specified by CXUnsavedFile are copied when necessary - client only needs to guarantee their validity until the call to this function returns. */,
                unsigned /*See enum CXTranslationUnit_Flags*/ options)
          : translation_unit(clang_parseTranslationUnit(
                      translation_unit_context, source_filename,
                      command_line_args.data(), command_line_args.size(),
                      unsaved_files.data(), unsaved_files.size(),
                      options))
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

//        operator CXTranslationUnit() { return translation_unit; }

        CXCursor get_cursor() const
        {
            return clang_getTranslationUnitCursor(translation_unit);
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


    size_t get_offset(const CXSourceLocation& location)
    {
        unsigned offset;
        clang_getExpansionLocation(location, /*file*/ NULL, /*line*/ NULL, /*column*/ NULL, &offset);
        return offset;
    }

    size_t get_start_offset(const CXSourceRange& extent)
    {
        return get_offset(clang_getRangeStart(extent));
    }

    size_t get_end_offset(const CXSourceRange& extent)
    {
        return get_offset(clang_getRangeEnd(extent));
    }

    std::string get(const char* source_code, const CXSourceRange& extent)
    {
        const size_t start_offset {get_offset(clang_getRangeStart(extent))};
        return {source_code + start_offset,
                get_offset(clang_getRangeEnd(extent)) - start_offset};
    }
}
