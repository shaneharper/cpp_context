#include "libclang++.h++"
#include <cstring> // strlen


static std::string get_context(const CXCursor& cursor, const size_t query_offset)
{
    std::string result;

    Libclang::visit_children(cursor,
            [query_offset, &result](const CXCursor& cursor, const CXCursor& /*parent*/)
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
                    result += Libclang::get_display_name(cursor) + "\n";
                }
                else if (CXCursor_Namespace == cursor_kind)
                {
                    result += "namespace " + Libclang::get_display_name(cursor) + "\n";
                }
                return Libclang::NextNode::Child;
            });

    return result;
}

std::string get_context(const char* source_code, const size_t query_offset)
{
    Libclang::TranslationUnitContext translation_unit_context;
    Libclang::TranslationUnit translation_unit(translation_unit_context, "test.c++",
            /*command_line_args*/ {},
            /*unsaved_files*/ {{"test.c++", source_code, strlen(source_code)}},
            /*options*/ CXTranslationUnit_None);

    return get_context(translation_unit.get_cursor(), query_offset);
}
