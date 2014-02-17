#include "libclang++.h++"
#include <map>


static std::string scope_name(const CXCursor& cursor)
{
    static const char* template_class_or_struct_or_union = "template_class_or_struct_or_union";
    static const std::map<CXCursorKind, const char* /*display name*/> cursor_kinds_that_introduce_a_named_scope =
    {
        { CXCursor_FunctionDecl,        "" },
        { CXCursor_Namespace,           "namespace " },
        { CXCursor_ClassDecl,           "class " },
        { CXCursor_StructDecl,          "struct " },
        { CXCursor_UnionDecl,           "union " },
        { CXCursor_EnumDecl,            "enum " },
        { CXCursor_CXXMethod,           ""},
        { CXCursor_Constructor,         ""},
        { CXCursor_Destructor,          ""},
        { CXCursor_ConversionFunction,  ""},
        { CXCursor_FunctionTemplate,    ""},
        { CXCursor_ClassTemplate,       template_class_or_struct_or_union},
        { CXCursor_ClassTemplatePartialSpecialization, template_class_or_struct_or_union},
        { CXCursor_LambdaExpr,          "[]"},
// XXX        { CXCursor_TypedefDecl,         "typedef "}
    };
    auto it = cursor_kinds_that_introduce_a_named_scope.find(clang_getCursorKind(cursor));
    if (it == cursor_kinds_that_introduce_a_named_scope.end())
        return "";

    if (template_class_or_struct_or_union == it->second)
    {
        it = cursor_kinds_that_introduce_a_named_scope.find(clang_getTemplateCursorKind(cursor));
        if (it == cursor_kinds_that_introduce_a_named_scope.end()) { return "ERROR determining template type.\n"; }
    }
    return it->second + Libclang::get_display_name(cursor) + "\n";
}


std::string get_context(/*const*/ Libclang::TranslationUnit& translation_unit, const size_t main_file_offset)
{
    std::string result;

#if CINDEX_VERSION < CINDEX_VERSION_ENCODE(0, 20)
    const CXFile main_file {Libclang::get_main_file(translation_unit)};
    const auto clang_Location_isFromMainFile = [&main_file](const CXSourceLocation &location){ return main_file == Libclang::get_file(location); }; // XXX is it ok to compare CXFiles with ==? Should I be comparing CXFileUniqueIDs (and using clang_getFileUniqueID()?)
#endif

    Libclang::visit_children(translation_unit.get_cursor(),
            [&](const CXCursor& cursor, const CXCursor& /*parent*/)
            {
                const auto cursor_extent = clang_getCursorExtent(cursor);

                const bool is_cursor_start_in_main_file = clang_Location_isFromMainFile(clang_getRangeStart(cursor_extent));
                const bool is_cursor_end_in_main_file = clang_Location_isFromMainFile(clang_getRangeEnd(cursor_extent));

                if (not (is_cursor_start_in_main_file or is_cursor_end_in_main_file)) // "if cursor is not for an AST element in the 'main' file"  XXX This condition doesn't correctly handle the (very unlikely) case where an AST element relates to part of the main file even though the start and end of the AST element are not in the main file.
                {
                    return Libclang::NextNode::Sibling;
                }
                if (is_cursor_end_in_main_file
                    and Libclang::get_one_beyond_end_offset(cursor_extent) <= main_file_offset)
                {
                    return Libclang::NextNode::Sibling;
                }
                if (is_cursor_start_in_main_file
                    and Libclang::get_start_offset(cursor_extent) > main_file_offset)
                {
                    return Libclang::NextNode::None;
                }

                result += scope_name(cursor);
                return Libclang::NextNode::Child;
            });

    return result;
}
