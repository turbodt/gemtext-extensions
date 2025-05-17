#include "./shared.h"


static int is_last_child_blank_line(GemtextContainer const *);
static GemtextExtensionsContainerUtils value = {
    .children = {
        .last_is_blank_line = &is_last_child_blank_line
    },
};
GemtextExtensionsContainerUtils const *gemtext_ext_container_utils = &value;


int is_last_child_blank_line(GemtextContainer const *parent) {
    size_t const children_count = gemtext_container_get_children_count(parent);
    if (!children_count) {
        return 0;
    }
    GemtextNode const * last_child = gemtext_container_getc_child(
        parent,
        children_count-1
    );
    GemtextNodeType last_child_type = gemtext_node_get_type(last_child);
    return last_child_type == GEMTEXT_NODE_TYPE__TEXT_LINE
        && gemtext_node_is_empty(last_child);
};
