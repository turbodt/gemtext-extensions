#ifndef GEMTEXT_EXTENSIONS_STD68_H
#define GEMTEXT_EXTENSIONS_STD68_H


#include "./base.h"


typedef struct {
    struct {
        int (*cr)(char c);
        int (*lf)(char c);
        int (*wsp)(char c);
    } is;
} GemtextExtensionsStd68;
extern PROTECTED GemtextExtensionsStd68 const *gemtext_ext_std68;


#endif
