#ifndef GEMTEXT_EXTENSIONS_MARKDOWN_LINK_H
#define GEMTEXT_EXTENSIONS_MARKDOWN_LINK_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextMDLink GemtextMDLink;


typedef struct GemtextMDLinkVTable {
    GemtextNodeVTable node;
    char const * (*get_target)(GemtextMDLink const *);
    GemtextErrType (*set_target)(GemtextMDLink *, char const *);
    char const * (*get_section)(GemtextMDLink const *);
    GemtextErrType (*set_section)(GemtextMDLink *, char const *);
    char const * (*get_block)(GemtextMDLink const *);
    GemtextErrType (*set_block)(GemtextMDLink *, char const *);
    char const * (*get_alias)(GemtextMDLink const *);
    GemtextErrType (*set_alias)(GemtextMDLink *, char const *);
    char const * (*get_title)(GemtextMDLink const *);
    GemtextErrType (*set_title)(GemtextMDLink *, char const *);
} GemtextMDLinkVTable;


struct GemtextMDLink {
    union {
        GemtextMDLinkVTable const * md_link;
        GemtextNode node;
    } as;
};


GemtextMiddleware * gemtext_md_link_line_get_middleware(void);
unsigned int gemtext_md_link_get_id(void);


UPTR(GemtextMDLink) gemtext_md_link_make(void);
UPTR(GemtextMDLink) gemtext_md_link_make_w_config(GemtextConfig const *);
UPTR(GemtextMDLink) gemtext_md_link_make_from_iter_w_config(
    CharIterWithSavePoints *,
    GemtextConfig const *
);
size_t gemtext_md_link_destroy(UPTR(GemtextMDLink));
char const * gemtext_md_link_get_target(GemtextMDLink const *);
GemtextErrType gemtext_md_link_set_target(GemtextMDLink *, char const *);
char const * gemtext_md_link_get_section(GemtextMDLink const *);
GemtextErrType gemtext_md_link_set_section(GemtextMDLink *, char const *);
char const * gemtext_md_link_get_block(GemtextMDLink const *);
GemtextErrType gemtext_md_link_set_block(GemtextMDLink *, char const *);
char const * gemtext_md_link_get_alias(GemtextMDLink const *);
GemtextErrType gemtext_md_link_set_alias(GemtextMDLink *, char const *);
char const * gemtext_md_link_get_title(GemtextMDLink const *);
GemtextErrType gemtext_md_link_set_title(GemtextMDLink *, char const *);


#endif
