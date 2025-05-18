#ifndef GEMTEXT_EXTENSIONS_BASE_H
#define GEMTEXT_EXTENSIONS_BASE_H


#include "../include/gemtext-extensions/config.h"
#include "../include/gemtext-extensions/smart-list.h"
#include "../include/gemtext-extensions/thematic-break.h"
#include "../include/gemtext-extensions/markdown-link.h"
#include "../include/gemtext-extensions/enriched-text-line.h"


#ifndef UPTR
#define UPTR(type) type *
#endif


#ifndef PROTECTED
#define PROTECTED __attribute__((visibility("hidden")))
#endif


#endif
