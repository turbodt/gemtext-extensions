#ifndef GEMTEXT_EXTENSIONS_MARKDOWN_LINK_SHARED_H
#define GEMTEXT_EXTENSIONS_MARKDOWN_LINK_SHARED_H


#include "../shared.h"


PROTECTED size_t md_link_advance_iter(CharIterWithSavePoints *);
PROTECTED size_t md_link_seg1_advance_iter(CharIterWithSavePoints *);
PROTECTED size_t md_link_seg2_advance_iter(CharIterWithSavePoints *);


#endif
