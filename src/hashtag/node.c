#include "./shared.h"
#include <gemtext/extension.h>
#include <string.h>


typedef struct {
    union {
        GemtextHashtagVTable const * vtable;
        GemtextNodeImpl node;
        GemtextHashtag interface;
    } as;
    UPTR(char) text;
} GemtextHashtagImpl;


static size_t vtable_destroy(UPTR(GemtextNode));
static int vtable_is_empty(GemtextNode const *);
static GemtextNodeType vtable_get_type(GemtextNode const *);
static size_t vtable_write_content_chunk(
    GemtextNode const *,
    size_t,
    char *,
    size_t
);
static size_t vtable_get_content_size(GemtextNode const *);
static GemtextErrType vtable_trim_content(GemtextNode *);
static char const * get_text(GemtextHashtag const *);
static GemtextErrType set_text(GemtextHashtag *, char const *);
static GemtextErrType copy_str(GemtextConfigMem, char **, char const *);
static GemtextHashtagVTable vtable = {
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
    .get_text=&get_text,
    .set_text=&set_text,
};


inline UPTR(GemtextHashtag) gemtext_hashtag_make(void) {
    GemtextConfig const * config = gemtext_config_default_get();
    return gemtext_hashtag_make_w_config(config);
}


UPTR(GemtextHashtag) gemtext_hashtag_make_w_config(GemtextConfig const *config) {
    GemtextConfigMem const * mem = &config->memory_strat;
    UPTR(GemtextHashtagImpl) impl = mem->alloc(sizeof(GemtextHashtagImpl));
    if (!impl) {
        return NULL;
    }

    GemtextErrType err;
    err = gemtext_node_init_w_config(&impl->as.node, &vtable.node, config);
    if (err != GEMTEXT_ERR__OK) {
        mem->free(impl);
        return NULL;
    }

    impl->text = NULL;

    return &impl->as.interface;
}


size_t vtable_destroy(UPTR(GemtextNode) interface) {
    if (!interface) {
        return 0;
    }
    UPTR(GemtextHashtagImpl) impl = (UPTR(GemtextHashtagImpl)) interface;
    GemtextConfigMem const *mem = &impl->as.node.memory_strat;

    if (impl->text) {
        mem->free(impl->text);
        impl->text = NULL;
    }

    return gemtext_node_vtable_destroy(interface);
};


int vtable_is_empty(GemtextNode const *node) {
    if (!node) {
        return 1;
    }
    GemtextHashtagImpl const * impl = (void const *) node;

    return (impl->text && strlen(impl->text));
};


GemtextNodeType vtable_get_type(GemtextNode const *interface) {
    (void) interface;
    return gemtext_ext_get_current_config().hashtag.id;
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
    GemtextHashtagImpl const *impl = (GemtextHashtagImpl const *) interface;

    size_t const len_text = impl->text ? strlen(impl->text) : 0;

    size_t writed_size = 0;

    writed_size += buffer_ops->write.with_offset_from_str(
        &offset,
        (void **)&buffer,
        &max_size,
        "#"
    );

    if (max_size && len_text) {
        writed_size += buffer_ops->write.with_offset_from_str(
            &offset,
            (void **)&buffer,
            &max_size,
            impl->text
        );
    }
    return writed_size;
};


size_t vtable_get_content_size(GemtextNode const *interface) {
    if (!interface) {
        return 0;
    }
    GemtextHashtagImpl const * impl = (GemtextHashtagImpl const *) interface;

    size_t const len_text = impl->text ? strlen(impl->text) : 0;

    size_t content_size = 1;

    if (len_text) {
        content_size += len_text;
    }

    return content_size;
};


GemtextErrType vtable_trim_content(GemtextNode *interface) {
    if (!interface) {
        return 0;
    }
    GemtextHashtagImpl * impl = (GemtextHashtagImpl *) interface;
    GemtextConfigMem const *mem = &impl->as.node.memory_strat;

    buffer_ops->str.trim_realloc(*mem, &impl->text);

    return GEMTEXT_ERR__OK;
};


char const * get_text(GemtextHashtag const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextHashtagImpl const *impl = (GemtextHashtagImpl const *) interface;
    return impl->text;
};


GemtextErrType set_text(GemtextHashtag *interface, char const *src) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextHashtagImpl *impl = (GemtextHashtagImpl *) interface;
    return copy_str(impl->as.node.memory_strat, &impl->text, src);
};


inline GemtextErrType copy_str(
    GemtextConfigMem memory_strat,
    char **p_dst,
    char const *src
) {
    if (!src) {
        memory_strat.free(*p_dst);
        *p_dst = NULL;
        return GEMTEXT_ERR__OK;
    }

    size_t new_len = strlen(src);

    char *new_ptr = memory_strat.realloc(*p_dst, new_len + 1);
    if (!new_ptr) {
        return GEMTEXT_ERR__ALLOC;
    }
    *p_dst = new_ptr;

    strncpy(*p_dst, src, new_len + 1);
    return GEMTEXT_ERR__OK;
};
