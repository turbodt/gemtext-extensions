#include "./shared.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static int is_checkbox(CharIterWithSavePoints *);
static UPTR(GemtextNode) parse_checkbox(GemtextMiddlewareContext const *);

static GemtextMiddleware middleware = {.call=call};


GemtextMiddleware * gemtext_checkbox_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    if (!state) {
        return NULL;
    }

    if (!ctx_utils->is_parent.list(context) || !is_checkbox(state->iter)) {
        return context->next(context, state);
    }

    return parse_checkbox(context);
};


UPTR(GemtextNode) parse_checkbox(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    GemtextNode * child_node = NULL;
    CharIterWithSavePoints * iter = state->iter;
    GemtextConfig const * config = &state->parser->config;

    UPTR(GemtextCheckbox) parent = gemtext_checkbox_make_w_config(config);
    if (!parent) {
        *state->p_err = GEMTEXT_ERR__ALLOC;
        return NULL;
    }

    iter_ops->advance.while_wsp(iter);
    iter->next(iter);
    if (iter_ops->is.wsp(iter)) {
        gemtext_checkbox_uncheck(parent);
    } else {
        gemtext_checkbox_check(parent);
    };
    iter->next(iter);
    gemtext_checkbox_set_separator(parent, iter->next(iter));

    child_node = context->next(context, state);

    if (!child_node) {
        gemtext_checkbox_destroy(parent);
        return NULL;
    }

    if (*state->p_err != GEMTEXT_ERR__OK) {
        gemtext_node_destroy(child_node);
        gemtext_checkbox_destroy(parent);
        return NULL;
    }

    *state->p_err = gemtext_checkbox_set_content(parent, child_node);

    if (*state->p_err != GEMTEXT_ERR__OK) {
        gemtext_node_destroy(child_node);
        gemtext_checkbox_destroy(parent);
        return NULL;
    }

    return &parent->as.node;
};


int is_checkbox(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '[' && !iter_ops->is.wsp(iter)) {
        return 0;
    }

    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return 0;
    }

    iter_ops->advance.while_wsp(iter);
    if (iter->get(iter) != '[') {
        iter->restore(iter);
        return 0;
    }

    char state_char = iter->next(iter);
    if (state_char != 'x' && state_char != 'X' && !std68->is.wsp(state_char)) {
        iter->restore(iter);
        return 0;
    }
    if (iter->next(iter) != ']') {
        iter->restore(iter);
        return 0;
    }
    iter->next(iter);
    if (!iter_ops->is.wsp(iter)) {
        iter->restore(iter);
        return 0;
    }

    iter->restore(iter);
    return 1;
};
