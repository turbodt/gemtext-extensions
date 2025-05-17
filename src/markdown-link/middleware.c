#include "./shared.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static int is_single_md_link(CharIterWithSavePoints *);
static UPTR(GemtextNode) parse_single_md_link(
    CharIterWithSavePoints *,
    GemtextConfig const *
);

static GemtextMiddleware middleware = {.call=call};


GemtextMiddleware * gemtext_md_link_line_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    if (!is_single_md_link(state->iter)) {
        return context->next(context, state);
    }

    GemtextContainer const *parent = state->parent;
    if (!parent) {
        return parse_single_md_link(state->iter, &state->parser->config);
    }

    GemtextNodeType const parent_type = gemtext_node_get_type(&parent->as.node);
    switch (parent_type) {
        case GEMTEXT_NODE_TYPE__PARAGRAPH:
            return NULL;
        case GEMTEXT_NODE_TYPE__GROUP:
            return parse_single_md_link(state->iter, &state->parser->config);
        default:
        break;
    }

    return context->next(context, state);
};


int is_single_md_link(CharIterWithSavePoints *iter) {
    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return 0;
    }

    iter_ops->advance.while_wsp(iter);

    size_t md_link_size = md_link_advance_iter(iter);
    if (!md_link_size) {
        iter->restore(iter);
        return 0;
    }

    if (!iter_ops->is.blank_line(iter)) {
        iter->restore(iter);
        return 0;
    }

    iter->restore(iter);
    return 1;
};


UPTR(GemtextNode) parse_single_md_link(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return NULL;
    }

    iter_ops->advance.while_wsp(iter);
    UPTR(GemtextMDLink) md_link = gemtext_md_link_make_from_iter_w_config(
        iter,
        config
    );
    if (!md_link) {
        iter->restore(iter);
        return NULL;
    }

    iter_ops->advance.until_next_line(iter);

    iter->drop(iter);
    return &md_link->as.node;
};
