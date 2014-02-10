#include "libclang++.h++"
#include <cstring> // strlen
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
        { CXCursor_ClassTemplatePartialSpecialization, template_class_or_struct_or_union}
    };
    auto it = cursor_kinds_that_introduce_a_named_scope.find(clang_getCursorKind(cursor));
    if (it != cursor_kinds_that_introduce_a_named_scope.end())
    {
        if (template_class_or_struct_or_union == it->second)
        {
            it = cursor_kinds_that_introduce_a_named_scope.find(clang_getTemplateCursorKind(cursor));
            if (it == cursor_kinds_that_introduce_a_named_scope.end()) { return "ERROR determining template type.\n"; }
        }
        return it->second + Libclang::get_display_name(cursor) + "\n";
    }
    return "";
}

std::string get_context(const CXCursor& cursor, const size_t query_offset)
{
    std::string result;

    Libclang::visit_children(cursor,
            [query_offset, &result](const CXCursor& cursor, const CXCursor& /*parent*/)
            {
                const auto cursor_extent = clang_getCursorExtent(cursor);
                if (Libclang::get_one_beyond_end_offset(cursor_extent) <= query_offset)
                {
                    return Libclang::NextNode::Sibling;
                }
                if (Libclang::get_start_offset(cursor_extent) > query_offset)
                {
                    return Libclang::NextNode::None;
                }

                result += scope_name(cursor);
                return Libclang::NextNode::Child;
            });

    return result;
}
