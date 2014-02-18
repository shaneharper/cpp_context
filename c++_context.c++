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



CXCursor get_cursor(CXTranslationUnit translation_unit, CXFile file, size_t byte_offset)
{
    return clang_getCursor(translation_unit, clang_getLocationForOffset(translation_unit, file, byte_offset));
}

std::string get_context(CXCursor cursor)
{
    // CXCursor_TypeRef, which can appear in a template defintion, has no parent, so use another cursor:
    if /*XXX while */ (CXCursor_TypeRef == clang_getCursorKind(cursor))
    {
        const auto extent = clang_getCursorExtent(cursor);
        cursor = get_cursor(clang_Cursor_getTranslationUnit(cursor),
                            Libclang::get_file(clang_getRangeEnd(extent)),
                            Libclang::get_one_beyond_end_offset(extent) /*I assume "one beyond" is never the end of file.*/);
    }

    return clang_Cursor_isNull(cursor) ? ""
        : (get_context(clang_getCursorSemanticParent(cursor)) + scope_name(cursor));
}
