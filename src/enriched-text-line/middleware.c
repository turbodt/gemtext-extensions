#include "./shared.h"


static UPTR(GemtextNode) call(GemtextMiddlewareContext const *);
static UPTR(GemtextNode) parse_paragraph(GemtextParseState const *);
static UPTR(GemtextNode) parse_text_line(GemtextParseState const *);


static GemtextMiddleware middleware = {.call=call,};


GemtextMiddleware * gemtext_enriched_tl_get_middleware(void) {
    return &middleware;
};


UPTR(GemtextNode) call(GemtextMiddlewareContext const *context) {
    GemtextParseState const *state = context->state;
    if (!state) {
        return NULL;
    }

    GemtextContainer const *parent = state->parent;

    int is_bl_parent_pg =
        iter_ops->is.blank_line(state->iter)
        | (ctx_utils->is_parent.paragraph(context) << 1);

    switch (is_bl_parent_pg) {
        case 0: {
            if (!parent || ctx_utils->is_parent.group(context)) {
                return parse_paragraph(state);
            }
        } break;
        case 1:
            return parse_text_line(state);
        case 2:
            if (container_utils->children.last_is_blank_line(parent)) {
                return NULL;
            }
            return parse_text_line(state);
        case 3: {
            if (container_utils->children.last_is_blank_line(parent)) {
                return NULL;
            }
        } break;
    }
    return parse_text_line(state);
};


UPTR(GemtextNode) parse_paragraph(GemtextParseState const *state) {
    GemtextNode * child_node = NULL;
    GemtextParser *parser = state->parser;
    CharIterWithSavePoints * iter = state->iter;
    GemtextConfig config = parser->config;

    UPTR(GemtextParagraph) parent = gemtext_paragraph_make_w_config(&config);
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
        gemtext_paragraph_destroy(parent);
        return NULL;
    }

    return &parent->as.node;
};


UPTR(GemtextNode) parse_text_line(GemtextParseState const *state) {
    GemtextConfig config = state->parser->config;
    UPTR(GemtextEnrichedTL) new_node = gemtext_enriched_tl_make_w_config(&config);
    if (!new_node) {
        *state->p_err = GEMTEXT_ERR__ALLOC;
        return NULL;
    }

    gemtext_enriched_tl_set_content_from_iter(new_node, state->iter);

    if (*state->p_err != GEMTEXT_ERR__OK) {
        gemtext_node_destroy(&new_node->as.node);
        return NULL;
    }

    return &new_node->as.node;
};
