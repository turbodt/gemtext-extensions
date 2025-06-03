#include "./shared.h"


inline unsigned int gemtext_checkbox_get_id(void) {
    return gemtext_ext_get_current_config().checkbox.id;
};


inline size_t gemtext_checkbox_destroy(UPTR(GemtextCheckbox) cb) {
    return gemtext_node_destroy(&cb->as.node);
};


inline GemtextNode const * gemtext_checkbox_getc_content(
    GemtextCheckbox const *cb
) {
    return cb->as.checkbox->getc_content(cb);
};


inline GemtextNode * gemtext_checkbox_get_content(GemtextCheckbox *cb) {
    return cb->as.checkbox->get_content(cb);
};


inline GemtextErrType gemtext_checkbox_set_content(
    GemtextCheckbox *cb,
    UPTR(GemtextNode) child
) {
    return cb->as.checkbox->set_content(cb, child);
};


inline GemtextErrType gemtext_checkbox_set_separator(
    GemtextCheckbox *cb,
    char separator
) {
    return cb->as.checkbox->set_separator(cb, separator);
};


inline int gemtext_checkbox_is_checked(GemtextCheckbox const *cb) {
    return cb->as.checkbox->is_checked(cb);
};


inline void gemtext_checkbox_check(GemtextCheckbox *cb) {
    cb->as.checkbox->check(cb);
};


inline void gemtext_checkbox_uncheck(GemtextCheckbox *cb) {
    cb->as.checkbox->uncheck(cb);
};


inline void gemtext_checkbox_toggle(GemtextCheckbox *cb) {
    cb->as.checkbox->toggle(cb);
};
