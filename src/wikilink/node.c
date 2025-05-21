#include "./shared.h"
#include <gemtext/extension.h>
#include <string.h>


typedef struct {
    union {
        GemtextWikilinkVTable const * vtable;
        GemtextNodeImpl node;
        GemtextWikilink interface;
    } as;
    UPTR(char) target;
    UPTR(char) section;
    UPTR(char) block;
    UPTR(char) alias;
} GemtextWikilinkImpl;


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
static char const * get_target(GemtextWikilink const *);
static GemtextErrType set_target(GemtextWikilink *, char const *);
static char const * get_section(GemtextWikilink const *);
static GemtextErrType set_section(GemtextWikilink *, char const *);
static char const * get_block(GemtextWikilink const *);
static GemtextErrType set_block(GemtextWikilink *, char const *);
static char const * get_alias(GemtextWikilink const *);
static GemtextErrType set_alias(GemtextWikilink *, char const *);
static GemtextErrType copy_str(GemtextConfigMem, char **, char const *);
static GemtextWikilinkVTable vtable = {
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
    .get_target=&get_target,
    .set_target=&set_target,
    .get_section=&get_section,
    .set_section=&set_section,
    .get_block=&get_block,
    .set_block=&set_block,
    .get_alias=&get_alias,
    .set_alias=&set_alias,
};


inline UPTR(GemtextWikilink) gemtext_wikilink_make(void) {
    GemtextConfig const * config = gemtext_config_default_get();
    return gemtext_wikilink_make_w_config(config);
}


UPTR(GemtextWikilink) gemtext_wikilink_make_w_config(GemtextConfig const *config) {
    GemtextConfigMem const * mem = &config->memory_strat;
    UPTR(GemtextWikilinkImpl) impl = mem->alloc(sizeof(GemtextWikilinkImpl));
    if (!impl) {
        return NULL;
    }

    GemtextErrType err;
    err = gemtext_node_init_w_config(&impl->as.node, &vtable.node, config);
    if (err != GEMTEXT_ERR__OK) {
        mem->free(impl);
        return NULL;
    }

    impl->target = NULL;
    impl->section = NULL;
    impl->block = NULL;
    impl->alias = NULL;

    return &impl->as.interface;
}


size_t vtable_destroy(UPTR(GemtextNode) interface) {
    if (!interface) {
        return 0;
    }
    UPTR(GemtextWikilinkImpl) impl = (UPTR(GemtextWikilinkImpl)) interface;
    GemtextConfigMem const *mem = &impl->as.node.memory_strat;

    if (impl->target) {
        mem->free(impl->target);
        impl->target = NULL;
    }
    if (impl->section) {
        mem->free(impl->section);
        impl->section = NULL;
    }
    if (impl->block) {
        mem->free(impl->block);
        impl->block = NULL;
    }
    if (impl->alias) {
        mem->free(impl->alias);
        impl->alias = NULL;
    }

    return gemtext_node_vtable_destroy(interface);
};


int vtable_is_empty(GemtextNode const *node) {
    if (!node) {
        return 1;
    }
    GemtextWikilinkImpl const * impl = (void const *) node;

    return (impl->target && strlen(impl->target))
        || (impl->alias && strlen(impl->alias))
        || (impl->section && strlen(impl->section))
        || (impl->block && strlen(impl->block))
    ;
};


GemtextNodeType vtable_get_type(GemtextNode const *interface) {
    (void) interface;
    return gemtext_ext_get_current_config().wikilink.id;
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
    GemtextWikilinkImpl const *impl = (GemtextWikilinkImpl const *) interface;

    size_t const len_target = impl->target ? strlen(impl->target) : 0;
    size_t const len_block = impl->block ? strlen(impl->block) : 0;
    size_t const len_section = impl->section ? strlen(impl->section) : 0;
    size_t const len_alias = impl->alias ? strlen(impl->alias) : 0;

    size_t writed_size = 0;

    writed_size += buffer_ops->write.with_offset_from_str(
        &offset,
        (void **)&buffer,
        &max_size,
        "[["
    );

    if (max_size && len_target) {
        writed_size += buffer_ops->write.with_offset_from_str(
            &offset,
            (void **)&buffer,
            &max_size,
            impl->target
        );
    }

    if (max_size && len_section) {
        writed_size += buffer_ops->write.with_offset_from_str(
            &offset,
            (void **)&buffer,
            &max_size,
            impl->section
        );
    }

    if (max_size && len_block) {
        if (offset) {
            offset--;
        } else {
            buffer[0] = '^';
            buffer++;
            max_size--;
            writed_size++;
        }

        writed_size += buffer_ops->write.with_offset_from_str(
            &offset,
            (void **)&buffer,
            &max_size,
            impl->block
        );
    }

    if (max_size && len_alias) {
        if (offset) {
            offset--;
        } else {
            buffer[0] = '|';
            buffer++;
            max_size--;
            writed_size++;
        }

        writed_size += buffer_ops->write.with_offset_from_str(
            &offset,
            (void **)&buffer,
            &max_size,
            impl->alias
        );
    }


    writed_size += buffer_ops->write.with_offset_from_str(
        &offset,
        (void **)&buffer,
        &max_size,
        "]]"
    );
    return writed_size;
};


size_t vtable_get_content_size(GemtextNode const *interface) {
    if (!interface) {
        return 0;
    }
    GemtextWikilinkImpl const * impl = (GemtextWikilinkImpl const *) interface;

    size_t const len_target = impl->target ? strlen(impl->target) : 0;
    size_t const len_block = impl->block ? strlen(impl->block) : 0;
    size_t const len_section = impl->section ? strlen(impl->section) : 0;
    size_t const len_alias = impl->alias ? strlen(impl->alias) : 0;

    size_t content_size = 4;

    if (len_alias) {
        content_size += len_alias + 1;
    }
    if (len_target) {
        content_size += len_target;
    }
    if (len_section) {
        content_size += len_section;
    }
    if (len_block) {
        content_size += 1 + len_block;
    }

    return content_size;
};


GemtextErrType vtable_trim_content(GemtextNode *interface) {
    if (!interface) {
        return 0;
    }
    GemtextWikilinkImpl * impl = (GemtextWikilinkImpl *) interface;
    GemtextConfigMem const *mem = &impl->as.node.memory_strat;

    buffer_ops->str.trim_realloc(*mem, &impl->target);
    buffer_ops->str.trim_realloc(*mem, &impl->section);
    buffer_ops->str.trim_realloc(*mem, &impl->block);
    buffer_ops->str.trim_realloc(*mem, &impl->alias);

    return GEMTEXT_ERR__OK;
};


char const * get_target(GemtextWikilink const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextWikilinkImpl const *impl = (GemtextWikilinkImpl const *) interface;
    return impl->target;
};


GemtextErrType set_target(GemtextWikilink *interface, char const *src) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextWikilinkImpl *impl = (GemtextWikilinkImpl *) interface;
    return copy_str(impl->as.node.memory_strat, &impl->target, src);
};


char const * get_section(GemtextWikilink const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextWikilinkImpl const *impl = (GemtextWikilinkImpl const *) interface;
    return impl->section;
};


GemtextErrType set_section(GemtextWikilink *interface, char const *src) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextWikilinkImpl *impl = (GemtextWikilinkImpl *) interface;
    return copy_str(impl->as.node.memory_strat, &impl->section, src);
};


char const * get_block(GemtextWikilink const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextWikilinkImpl const *impl = (GemtextWikilinkImpl const *) interface;
    return impl->block;
};


GemtextErrType set_block(GemtextWikilink *interface, char const *src) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextWikilinkImpl *impl = (GemtextWikilinkImpl *) interface;
    return copy_str(impl->as.node.memory_strat, &impl->block, src);
};


char const * get_alias(GemtextWikilink const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextWikilinkImpl const *impl = (GemtextWikilinkImpl const *) interface;
    return impl->alias;
};


GemtextErrType set_alias(GemtextWikilink *interface, char const *src) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextWikilinkImpl *impl = (GemtextWikilinkImpl *) interface;
    return copy_str(impl->as.node.memory_strat, &impl->alias, src);
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
