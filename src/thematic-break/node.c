#include "./shared.h"
#include <gemtext/extension.h>
#include <string.h>


#define TH_BREAK__DEFAULT_VALUE "---"


typedef struct {
    union {
        GemtextThBreakVTable const * vtable;
        GemtextNodeImpl node;
        GemtextThBreak interface;
    } as;
    UPTR(char) text;
} GemtextThBreakImpl;


static size_t vtable_destroy(UPTR(GemtextNode));
static GemtextNodeType vtable_get_type(GemtextNode const *);
static int vtable_is_empty(GemtextNode const *);
static size_t vtable_write_content_chunk(
    GemtextNode const *,
    size_t,
    char *,
    size_t
);
static size_t vtable_get_content_size(GemtextNode const *);
static GemtextErrType vtable_trim_content(GemtextNode *);
static char const * vtable_get_text(GemtextThBreak const *);
static GemtextErrType vtable_set_text(GemtextThBreak *, char const *);
static GemtextErrType to_asterisc(GemtextThBreak *);
static GemtextErrType to_dash(GemtextThBreak *);
static GemtextErrType to_underscore(GemtextThBreak *);
static void replace_non_wsp_chars_with(char, char *);


// Virtual table
static GemtextThBreakVTable vtable = {
    .node = {
        .destroy=&vtable_destroy,

        .get_type=&vtable_get_type,
        .is_empty=&vtable_is_empty,
        .write_content_chunk=&vtable_write_content_chunk,
        .get_content_size=&vtable_get_content_size,
        .trim_content=&vtable_trim_content,

        .get_parent=&gemtext_node_vtable_get_parent,
        .getc_parent=&gemtext_node_vtable_getc_parent,
        .attach_at=&gemtext_node_vtable_attach_at,
        .attach=&gemtext_node_vtable_attach,
        .detach=&gemtext_node_vtable_detach,
    },
    .get_text=&vtable_get_text,
    .set_text=&vtable_set_text,
    .to_asterisc=&to_asterisc,
    .to_dash=&to_dash,
    .to_underscore=&to_underscore,
};


UPTR(GemtextThBreak) gemtext_th_break_make(void) {
    GemtextConfig const * config = gemtext_config_default_get();
    return gemtext_th_break_make_w_config(config);
};


UPTR(GemtextThBreak) gemtext_th_break_make_w_config(
    GemtextConfig const *config
) {
    GemtextConfigMem const * mem = &config->memory_strat;
    UPTR(GemtextThBreakImpl) impl = mem->alloc(sizeof(GemtextThBreakImpl));
    if (!impl) {
        return NULL;
    }
    GemtextThBreak *interface = &impl->as.interface;

    GemtextErrType err;
    err = gemtext_node_init_w_config(&impl->as.node, &vtable.node, config);
    if (err != GEMTEXT_ERR__OK) {
        mem->free(impl);
        return NULL;
    }

    // init
    impl->text = NULL;
    vtable_set_text(interface, TH_BREAK__DEFAULT_VALUE);

    return interface;
};


size_t vtable_destroy(UPTR(GemtextNode) interface) {
    if (!interface) {
        return 0;
    };
    UPTR(GemtextThBreakImpl) impl = (UPTR(GemtextThBreakImpl)) interface;

    if (impl->text) {
        impl->as.node.memory_strat.free(impl->text);
        impl->text = NULL;
    }

    return gemtext_node_vtable_destroy(interface);
};


GemtextNodeType vtable_get_type(GemtextNode const *interface) {
    (void) interface;
    return gemtext_ext_get_current_config().thematic_break.id;
};


int vtable_is_empty(GemtextNode const *interface) {
    if (!interface) {
        return 0;
    };
    GemtextThBreakImpl const * node = (GemtextThBreakImpl const *) interface;

    if (!node->text) {
        return 1;
    }

    size_t const content_size = vtable_get_content_size(interface);
    for (size_t i = 0; i < content_size && node->text[i] != '\0'; i++) {
        if (!std68->is.wsp(node->text[i])) {
            return 0;
        }
    }
    return 1;
};


size_t vtable_write_content_chunk(
    GemtextNode const *interface,
    size_t offset,
    char *buffer,
    size_t max_size
) {
    if (!interface) {
        return 0;
    }
    GemtextThBreakImpl const * impl = (GemtextThBreakImpl const *) interface;
    size_t content_size = vtable_get_content_size(interface);

    if (max_size == 0 || content_size <= offset) {
        return 0;
    }

    size_t write_size = content_size - offset;

    if (max_size < write_size) {
        write_size = max_size;
    }

    memcpy(buffer, impl->text + offset, write_size);

    return write_size;
};


size_t vtable_get_content_size(GemtextNode const *interface) {
    if (!interface) {
        return 0;
    }
    GemtextThBreakImpl const * impl = (GemtextThBreakImpl const *) interface;
    return impl->text ? strlen(impl->text) : 0;
};


GemtextErrType vtable_trim_content(GemtextNode *interface) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextThBreakImpl * impl = (GemtextThBreakImpl *) interface;

    buffer_ops->str.trim_realloc(impl->as.node.memory_strat, &impl->text);

    return GEMTEXT_ERR__OK;
};


char const * vtable_get_text(GemtextThBreak const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextThBreakImpl const * impl = (GemtextThBreakImpl const *) interface;
    return impl->text;
};


GemtextErrType vtable_set_text(GemtextThBreak *interface, char const *src) {
    int err = 0;
    if (!interface || !src) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextThBreakImpl * impl = (GemtextThBreakImpl *) interface;

    UPTR(CharIterWithSavePoints) iter = char_iter_with_sp_make_from_buff(
        src,
        strlen(src)
    );
    if (!iter) {
        return GEMTEXT_ERR__ALLOC;
    }

    err = !is_valid_th_break_from_iter(iter);
    iter->destroy(iter);
    if (err) {
        return GEMTEXT_ERR__INVALID_ARG;
    }

    err = buffer_ops->str.set(impl->as.node.memory_strat, &impl->text, src);
    if (err) {
        return GEMTEXT_ERR__ALLOC;
    }

    return GEMTEXT_ERR__OK;
};


GemtextErrType to_asterisc(GemtextThBreak *interface) {
    GemtextThBreakImpl *impl = (GemtextThBreakImpl *) interface;
    replace_non_wsp_chars_with('*', impl->text);
    return GEMTEXT_ERR__OK;
};


GemtextErrType to_dash(GemtextThBreak *interface) {
    GemtextThBreakImpl *impl = (GemtextThBreakImpl *) interface;
    replace_non_wsp_chars_with('-', impl->text);
    return GEMTEXT_ERR__OK;
};


GemtextErrType to_underscore(GemtextThBreak *interface) {
    GemtextThBreakImpl *impl = (GemtextThBreakImpl *) interface;
    replace_non_wsp_chars_with('_', impl->text);
    return GEMTEXT_ERR__OK;
};


inline void replace_non_wsp_chars_with(char c, char *str) {
    size_t len = strlen(str);
    for (; len; len--) {
        if (!std68->is.wsp(str[len-1])) {
            str[len-1] = c;
        }
    }
};
