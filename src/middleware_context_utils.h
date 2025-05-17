#ifndef GEMTEXT_EXTENSIONS_MIDDLEWARE_CONTEXT_UTILS_H
#define GEMTEXT_EXTENSIONS_MIDDLEWARE_CONTEXT_UTILS_H


#include "./base.h"


typedef struct {
    struct {
        int (*group)(GemtextMiddlewareContext const *);
        int (*list)(GemtextMiddlewareContext const *);
        int (*quote)(GemtextMiddlewareContext const *);
        int (*paragraph)(GemtextMiddlewareContext const *);
    } const is_parent;
} GemtextExtensionsContextUtils;
extern PROTECTED GemtextExtensionsContextUtils const *gemtext_ext_context_utils;


#endif
