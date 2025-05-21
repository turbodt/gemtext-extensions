#include "./shared.h"


inline unsigned int gemtext_wikilink_get_id(void) {
    return gemtext_ext_get_current_config().wikilink.id;
};


inline size_t gemtext_wikilink_destroy(UPTR(GemtextWikilink) wl) {
    return gemtext_node_destroy(&wl->as.node);
};


inline char const * gemtext_wikilink_get_target(GemtextWikilink const *wl) {
    return wl->as.wikilink->get_target(wl);
};


inline GemtextErrType gemtext_wikilink_set_target(
    GemtextWikilink *wl,
    char const *str
) {
    return wl->as.wikilink->set_target(wl, str);
};


inline char const * gemtext_wikilink_get_section(GemtextWikilink const *wl) {
    return wl->as.wikilink->get_section(wl);
};


inline GemtextErrType gemtext_wikilink_set_section(
    GemtextWikilink *wl,
    char const *str
) {
    return wl->as.wikilink->set_section(wl, str);
};


inline char const * gemtext_wikilink_get_block(GemtextWikilink const *wl) {
    return wl->as.wikilink->get_block(wl);
};


inline GemtextErrType gemtext_wikilink_set_block(
    GemtextWikilink *wl,
    char const *str
) {
    return wl->as.wikilink->set_block(wl, str);
};


inline char const * gemtext_wikilink_get_alias(GemtextWikilink const *wl) {
    return wl->as.wikilink->get_alias(wl);
};


inline GemtextErrType gemtext_wikilink_set_alias(
    GemtextWikilink *wl,
    char const *str
) {
    return wl->as.wikilink->set_alias(wl, str);
};
