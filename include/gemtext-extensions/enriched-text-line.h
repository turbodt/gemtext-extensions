#ifndef GEMTEXT_EXTENSIONS_ENRICHED_TEXT_LINE_H
#define GEMTEXT_EXTENSIONS_ENRICHED_TEXT_LINE_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextEnrichedTL GemtextEnrichedTL;


typedef struct GemtextEnrichedTLVTable {
    GemtextContainerVTable container;
    GemtextErrType (*set_content_from_iter)(
        GemtextEnrichedTL *,
        CharIterWithSavePoints *
    );
    char const * (*get_text)(GemtextEnrichedTL const *);
    size_t (*get_text_size)(GemtextEnrichedTL const *);
    size_t (*get_child_position)(GemtextEnrichedTL const *, size_t);
} GemtextEnrichedTLVTable;


struct GemtextEnrichedTL {
    union {
        GemtextEnrichedTLVTable const * vtable;
        GemtextContainer container;
        GemtextNode node;
    } as;
};


extern GemtextMiddleware * gemtext_enriched_tl_get_middleware(void);
unsigned int gemtext_enriched_tl_get_id(void);


UPTR(GemtextEnrichedTL) gemtext_enriched_tl_make(void);
UPTR(GemtextEnrichedTL) gemtext_enriched_tl_make_w_config(GemtextConfig const *);
char const * gemtext_enriched_tl_get_text(GemtextEnrichedTL const *);
size_t gemtext_enriched_tl_get_text_size(GemtextEnrichedTL const *);
size_t gemtext_enriched_tl_get_child_position(GemtextEnrichedTL const *, size_t);
GemtextErrType gemtext_enriched_tl_set_content_from_iter(
    GemtextEnrichedTL *,
    CharIterWithSavePoints *
);
size_t gemtext_enriched_tl_destroy(UPTR(GemtextEnrichedTL));


#endif
