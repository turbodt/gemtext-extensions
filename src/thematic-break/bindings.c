#include "./shared.h"


inline unsigned int gemtext_th_break_get_id(void) {
    return gemtext_ext_get_current_config().thematic_break.id;
};


inline size_t gemtext_th_break_destroy(UPTR(GemtextThBreak) tb) {
    return gemtext_node_destroy(&tb->as.node);
};


inline char const * gemtext_th_break_get_text(GemtextThBreak const *th) {
    return th->as.th_break->get_text(th);
}


inline GemtextErrType gemtext_th_break_set_text(
    GemtextThBreak *th,
    char const *src
) {
    return th->as.th_break->set_text(th, src);
}


inline GemtextErrType gemtext_th_break_to_asterisc(GemtextThBreak *tb) {
    return tb->as.th_break->to_asterisc(tb);
};


inline GemtextErrType gemtext_th_break_to_dash(GemtextThBreak *tb) {
    return tb->as.th_break->to_dash(tb);
};


inline GemtextErrType gemtext_th_break_to_underscore(GemtextThBreak *tb) {
    return tb->as.th_break->to_underscore(tb);
};
