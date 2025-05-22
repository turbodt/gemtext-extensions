#include "./shared.h"


inline unsigned int gemtext_hashtag_get_id(void) {
    return gemtext_ext_get_current_config().hashtag.id;
};


inline size_t gemtext_hashtag_destroy(UPTR(GemtextHashtag) ht) {
    return gemtext_node_destroy(&ht->as.node);
};


inline char const * gemtext_hashtag_get_text(GemtextHashtag const *ht) {
    return ht->as.hashtag->get_text(ht);
};


inline GemtextErrType gemtext_hashtag_set_text(
    GemtextHashtag *ht,
    char const *str
) {
    return ht->as.hashtag->set_text(ht, str);
};
