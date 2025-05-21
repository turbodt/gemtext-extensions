#ifndef GEMTEXT_EXTENSIONS_WIKILINK_H
#define GEMTEXT_EXTENSIONS_WIKILINK_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextWikilink GemtextWikilink;


typedef struct GemtextWikilinkVTable {
    GemtextNodeVTable node;
    char const * (*get_target)(GemtextWikilink const *);
    GemtextErrType (*set_target)(GemtextWikilink *, char const *);
    char const * (*get_section)(GemtextWikilink const *);
    GemtextErrType (*set_section)(GemtextWikilink *, char const *);
    char const * (*get_block)(GemtextWikilink const *);
    GemtextErrType (*set_block)(GemtextWikilink *, char const *);
    char const * (*get_alias)(GemtextWikilink const *);
    GemtextErrType (*set_alias)(GemtextWikilink *, char const *);
    char const * (*get_title)(GemtextWikilink const *);
    GemtextErrType (*set_title)(GemtextWikilink *, char const *);
} GemtextWikilinkVTable;


struct GemtextWikilink {
    union {
        GemtextWikilinkVTable const * wikilink;
        GemtextNode node;
    } as;
};


GemtextMiddleware * gemtext_wikilink_line_get_middleware(void);
unsigned int gemtext_wikilink_get_id(void);


UPTR(GemtextWikilink) gemtext_wikilink_make(void);
UPTR(GemtextWikilink) gemtext_wikilink_make_w_config(GemtextConfig const *);
UPTR(GemtextWikilink) gemtext_wikilink_make_from_iter_w_config(
    CharIterWithSavePoints *,
    GemtextConfig const *
);
size_t gemtext_wikilink_destroy(UPTR(GemtextWikilink));
char const * gemtext_wikilink_get_target(GemtextWikilink const *);
GemtextErrType gemtext_wikilink_set_target(GemtextWikilink *, char const *);
char const * gemtext_wikilink_get_section(GemtextWikilink const *);
GemtextErrType gemtext_wikilink_set_section(GemtextWikilink *, char const *);
char const * gemtext_wikilink_get_block(GemtextWikilink const *);
GemtextErrType gemtext_wikilink_set_block(GemtextWikilink *, char const *);
char const * gemtext_wikilink_get_alias(GemtextWikilink const *);
GemtextErrType gemtext_wikilink_set_alias(GemtextWikilink *, char const *);


#endif
