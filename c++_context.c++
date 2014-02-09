#include "libclang++.h++"
#include <cstring> // strlen


std::string get_context(const char* source_code, size_t query_offset)
{
    Libclang::TranslationUnitContext translation_unit_context;
    Libclang::TranslationUnit translation_unit(translation_unit_context, "test.c++",
            /*command_line_args*/ {},
            /*unsaved_files*/ {{"test.c++", source_code, strlen(source_code)}},
            /*options*/ CXTranslationUnit_None);

    std::string result;

    Libclang::visit_children(translation_unit.get_cursor(),
            [source_code, query_offset, &result](const CXCursor& cursor, const CXCursor& /*parent*/)
            {
                const auto cursor_extent = clang_getCursorExtent(cursor);
                if (Libclang::get_end_offset(cursor_extent) < query_offset)
                {
                    return Libclang::NextNode::Sibling;
                }
                if (Libclang::get_start_offset(cursor_extent) > query_offset)
                {
                    return Libclang::NextNode::None;
                }

                const auto cursor_kind = clang_getCursorKind(cursor);
                if (CXCursor_FunctionDecl == cursor_kind)
                {
                    result += Libclang::get(source_code, clang_Cursor_getSpellingNameRange(cursor, 0, 0)) + "()\n";
                }
                else if (CXCursor_Namespace == cursor_kind)
                {
                    result += "namespace " + Libclang::get(source_code, clang_Cursor_getSpellingNameRange(cursor, 0, 0)) + "\n";
                }
                return Libclang::NextNode::Child;
            });

    return result;
}
