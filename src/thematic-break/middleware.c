#include "./shared.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static UPTR(GemtextNode) parse_th_break(GemtextParseState const *);


static GemtextMiddleware middleware = {.call=call};


GemtextMiddleware * gemtext_th_break_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;

    if (!is_valid_th_break_from_iter(state->iter)) {
        return context->next(context, state);
    }

    GemtextContainer const *parent = state->parent;
    if (!parent) {
        return parse_th_break(state);
    }

    GemtextNodeType const parent_type = gemtext_node_get_type(&parent->as.node);
    switch (parent_type) {
        case GEMTEXT_NODE_TYPE__GROUP:
            if (state->section_level > 0) {
                break;
            }
            return parse_th_break(state);
        default:
        break;
    }
    return NULL;
};


UPTR(GemtextNode) parse_th_break(GemtextParseState const *state) {
    GemtextConfig const * config = &state->parser->config;
    UPTR(GemtextThBreak) new_node = gemtext_th_break_make_w_config(config);
    if (!new_node) {
        *state->p_err = GEMTEXT_ERR__ALLOC;
        return NULL;
    }

    UPTR(char) line = iter_ops->fetch.line(
        state->iter,
        config->memory_strat.alloc
    );

    if (!line) {
        gemtext_node_destroy(&new_node->as.node);
        *state->p_err = GEMTEXT_ERR__ALLOC;
        return NULL;
    }

    *state->p_err = gemtext_th_break_set_text(new_node, line);
    config->memory_strat.free(line);

    if (*state->p_err != GEMTEXT_ERR__OK) {
        gemtext_th_break_destroy(new_node);
        return NULL;
    }

    return &new_node->as.node;
};
