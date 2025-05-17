#include "../shared.h"
#include "../char_iter_operations.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static UPTR(GemtextNode) parse_list(GemtextParseState const *);
static int is_list_item(CharIterWithSavePoints *);


static GemtextMiddleware middleware = {.call=call,};


GemtextMiddleware * gemtext_smart_list_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    if (!state) {
        return NULL;
    }

    GemtextContainer const *parent = state->parent;

    if (is_list_item(state->iter)) {

        if (ctx_utils->is_parent.list(context)) {
            state->iter->next(state->iter);
            return context->next(context, state);
        } else if (!parent || ctx_utils->is_parent.group(context)) {
            return parse_list(state);
        } else if (ctx_utils->is_parent.paragraph(context)) {
            return NULL;
        }

        return context->next(context, state);

    } else if (!ctx_utils->is_parent.list(context)) {
        return context->next(context, state);
    } else if (!iter_ops->is.blank_line(state->iter)) {
        return NULL;
    } else if (container_utils->children.last_is_blank_line(parent)) {
        return NULL;
    }

    return context->next(context, state);
};


UPTR(GemtextNode) parse_list(GemtextParseState const *state) {
    GemtextNode * child_node = NULL;
    GemtextParser *parser = state->parser;
    CharIterWithSavePoints * iter = state->iter;
    GemtextConfig const * config = &state->parser->config;

    UPTR(GemtextList) parent = gemtext_list_make_w_config(config);
    if (!parent) {
        *state->p_err = GEMTEXT_ERR__ALLOC;
        return NULL;
    }
    char curr_char = iter->get(iter);
    while (curr_char != '\0') {
        GemtextParseState child_state = *state;
        child_state.parent = &parent->as.container;

        child_node = parser->parse(&child_state);

        if (*child_state.p_err != GEMTEXT_ERR__OK) {
            gemtext_node_destroy(child_node);
            break;
        }
        if (!child_node) {
            break;
        }

        *state->p_err = gemtext_container_append(
            &parent->as.container,
            child_node
        );

        if (*state->p_err != GEMTEXT_ERR__OK) {
            gemtext_node_destroy(child_node);
            break;
        }
        curr_char = iter->get(iter);
    }

    if (*state->p_err != GEMTEXT_ERR__OK) {
        gemtext_list_destroy(parent);
        return NULL;
    }

    return &parent->as.node;
};


inline int is_list_item(CharIterWithSavePoints *iter) {
    return iter->get(iter) == '*' || iter->get(iter) == '-';
};
