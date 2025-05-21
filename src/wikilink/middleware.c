#include "./shared.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static int is_single_wikilink(CharIterWithSavePoints *);
static UPTR(GemtextNode) parse_single_wikilink(
    CharIterWithSavePoints *,
    GemtextConfig const *
);

static GemtextMiddleware middleware = {.call=call};


GemtextMiddleware * gemtext_wikilink_line_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    if (!is_single_wikilink(state->iter)) {
        return context->next(context, state);
    }

    GemtextContainer const *parent = state->parent;
    if (!parent) {
        return parse_single_wikilink(state->iter, &state->parser->config);
    }

    GemtextNodeType const parent_type = gemtext_node_get_type(&parent->as.node);
    switch (parent_type) {
        case GEMTEXT_NODE_TYPE__PARAGRAPH:
            return NULL;
        case GEMTEXT_NODE_TYPE__GROUP:
            return parse_single_wikilink(state->iter, &state->parser->config);
        default:
        break;
    }

    return context->next(context, state);
};


int is_single_wikilink(CharIterWithSavePoints *iter) {
    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return 0;
    }

    iter_ops->advance.while_wsp(iter);

    size_t wikilink_size = wikilink_advance_iter(iter);
    if (!wikilink_size) {
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


UPTR(GemtextNode) parse_single_wikilink(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return NULL;
    }

    iter_ops->advance.while_wsp(iter);
    UPTR(GemtextWikilink) wikilink = gemtext_wikilink_make_from_iter_w_config(
        iter,
        config
    );
    if (!wikilink) {
        iter->restore(iter);
        return NULL;
    }

    iter_ops->advance.until_next_line(iter);

    iter->drop(iter);
    return &wikilink->as.node;
};
