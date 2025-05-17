#ifndef GEMTEXT_EXTENSIONS_NODE_UTILS_H
#define GEMTEXT_EXTENSIONS_NODE_UTILS_H


#include "./base.h"


typedef struct {
    struct {
        int (*last_is_blank_line)(GemtextContainer const *);
    } children;
} GemtextExtensionsContainerUtils;
extern PROTECTED GemtextExtensionsContainerUtils const *gemtext_ext_container_utils;


#endif
