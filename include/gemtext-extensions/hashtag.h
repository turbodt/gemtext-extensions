#ifndef GEMTEXT_EXTENSIONS_HASHTAG_H
#define GEMTEXT_EXTENSIONS_HASHTAG_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextHashtag GemtextHashtag;


typedef struct GemtextHashtagVTable {
    GemtextNodeVTable node;
    char const * (*get_text)(GemtextHashtag const *);
    GemtextErrType (*set_text)(GemtextHashtag *, char const *);
} GemtextHashtagVTable;


struct GemtextHashtag {
    union {
        GemtextHashtagVTable const * hashtag;
        GemtextNode node;
    } as;
};


unsigned int gemtext_hashtag_get_id(void);


UPTR(GemtextHashtag) gemtext_hashtag_make(void);
UPTR(GemtextHashtag) gemtext_hashtag_make_w_config(GemtextConfig const *);
UPTR(GemtextHashtag) gemtext_hashtag_make_from_iter_w_config(
    CharIterWithSavePoints *,
    GemtextConfig const *
);
size_t gemtext_hashtag_destroy(UPTR(GemtextHashtag));
char const * gemtext_hashtag_get_text(GemtextHashtag const *);
GemtextErrType gemtext_hashtag_set_text(GemtextHashtag *, char const *);


#endif
