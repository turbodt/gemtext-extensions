#include "./shared.h"


static int is_parent_group(GemtextMiddlewareContext const *);
static int is_parent_list(GemtextMiddlewareContext const *);
static int is_parent_quote(GemtextMiddlewareContext const *);
static int is_parent_paragraph(GemtextMiddlewareContext const *);
static int is_parent_type(GemtextMiddlewareContext const *, GemtextNodeType);
static GemtextExtensionsContextUtils context_utils_value = {
    .is_parent = {
        .group=&is_parent_group,
        .list=&is_parent_list,
        .quote=&is_parent_quote,
        .paragraph=&is_parent_paragraph,
    },
};
GemtextExtensionsContextUtils const *gemtext_ext_context_utils =
    &context_utils_value;


int is_parent_group(GemtextMiddlewareContext const *context) {
    return is_parent_type(context, GEMTEXT_NODE_TYPE__GROUP);
};


int is_parent_list(GemtextMiddlewareContext const *context) {
    return is_parent_type(context, GEMTEXT_NODE_TYPE__LIST);
};


int is_parent_quote(GemtextMiddlewareContext const *context) {
    return is_parent_type(context, GEMTEXT_NODE_TYPE__QUOTE);
};


int is_parent_paragraph(GemtextMiddlewareContext const *context) {
    return is_parent_type(context, GEMTEXT_NODE_TYPE__PARAGRAPH);
};

int is_parent_type(
    GemtextMiddlewareContext const *context,
    GemtextNodeType type
) {
    GemtextParseState const *state = context->state;
    GemtextContainer const *parent = state->parent;
    if (!parent) {
        return 0;
    }

    GemtextNodeType const parent_type = gemtext_node_get_type(&parent->as.node);
    return parent_type == type;
};
