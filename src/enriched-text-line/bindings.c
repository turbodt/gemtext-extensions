#include "./shared.h"


inline char const * gemtext_enriched_tl_get_text(GemtextEnrichedTL const *tl) {
    return tl->as.vtable->get_text(tl);
}


inline size_t gemtext_enriched_tl_get_text_size(GemtextEnrichedTL const *tl) {
    return tl->as.vtable->get_text_size(tl);
};


inline size_t gemtext_enriched_tl_get_child_position(
    GemtextEnrichedTL const *tl,
    size_t position
) {
    return tl->as.vtable->get_child_position(tl, position);
};


inline GemtextErrType gemtext_enriched_tl_set_content_from_iter(
    GemtextEnrichedTL *tl,
    CharIterWithSavePoints *iter
) {
    return tl->as.vtable->set_content_from_iter(tl, iter);
}


inline size_t gemtext_enriched_tl_destroy(UPTR(GemtextEnrichedTL) tl) {
    return gemtext_node_destroy(&tl->as.node);
};
