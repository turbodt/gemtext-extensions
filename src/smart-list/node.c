#include "./shared.h"
#include <gemtext.h>
#include <gemtext/extension.h>


static struct {
    GemtextListVTable vtable;
    int is_loaded;
} custom_vtable = {
    .vtable = {0},
    .is_loaded = 0,
};
static void load_vtable(GemtextListVTable const *);
static GemtextErrType node_list_append_at(
    GemtextContainer *,
    GemtextNode*,
    size_t
);


UPTR(GemtextList) gemtext_ext_list_make_w_config(GemtextConfig const *config) {
    UPTR(GemtextList) list = gemtext_list_make_w_config(config);
    if (!list) {
        return NULL;
    }

    load_vtable(list->as.list);
    list->as.list = &custom_vtable.vtable;

    return list;
}


inline void load_vtable(GemtextListVTable const *src) {
    if (custom_vtable.is_loaded) {
        return;
    }
    custom_vtable.vtable = *src;
    custom_vtable.vtable.container.append_at = &node_list_append_at;
    custom_vtable.is_loaded = 1;
};


GemtextErrType node_list_append_at(
    GemtextContainer *i_container,
    GemtextNode *i_child,
    size_t index
) {
    if (!i_container) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextNodeType child_type = gemtext_node_get_type(i_child);
    if (
        child_type != GEMTEXT_NODE_TYPE__TEXT_LINE
        && child_type != gemtext_checkbox_get_id()
    ) {
        return GEMTEXT_ERR__INVALID_OP;
    }

    return gemtext_container_vtable_append_at(i_container, i_child, index);
};
