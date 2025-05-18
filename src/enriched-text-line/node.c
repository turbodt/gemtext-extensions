#include "./shared.h"
#include <gemtext/extension.h>


typedef UPTR(GemtextNode)(*ChildFactory)(
    CharIterWithSavePoints *,
    GemtextConfig const *
);
typedef struct {
    size_t position;
} ChildInfo;


typedef struct {
    union {
        GemtextEnrichedTLVTable const * vtable;
        GemtextNodeImpl node;
        GemtextContainerImpl container;
        GemtextEnrichedTL interface;
    } as;
    char * text;
    size_t text_size;
    size_t text_capacity;
    ChildInfo * children_info;
} GemtextEnrichedTLImpl;


static size_t vtable_destroy(UPTR(GemtextNode));
static int vtable_is_empty(GemtextNode const *);
static size_t vtable_write_content_chunk(
    GemtextNode const *,
    size_t,
    char *,
    size_t
);
static size_t vtable_get_content_size(GemtextNode const *);
static GemtextErrType vtable_append_at(
    GemtextContainer *,
    GemtextNode*,
    size_t
);
static GemtextErrType vtable_trim_content(GemtextNode *);
static GemtextNodeType vtable_get_type(GemtextNode const *);
static GemtextErrType vtable_set_content_from_iter(
    GemtextEnrichedTL *,
    CharIterWithSavePoints *
);
static char const * vtable_get_text(GemtextEnrichedTL const *);
static size_t vtable_get_text_size(GemtextEnrichedTL const *);
static size_t vtable_get_child_position(GemtextEnrichedTL const *, size_t);
static UPTR(GemtextNode) get_child_from_buffer(
    CharIterWithSavePoints *,
    GemtextConfig const *
);
static GemtextErrType append_child(GemtextEnrichedTLImpl *, UPTR(GemtextNode));
static size_t remove_children(GemtextEnrichedTLImpl *);
static GemtextErrType trim_text(GemtextEnrichedTLImpl *);
static GemtextEnrichedTLVTable vtable = {
    .container = {
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
        .get_children_count=&gemtext_container_vtable_get_children_count,
        .get_child=&gemtext_container_vtable_get_child,
        .getc_child=&gemtext_container_vtable_getc_child,
        .append_at=&vtable_append_at,
        .append=&gemtext_container_vtable_append,
        .detach_child=&gemtext_container_vtable_detach_child,
    },
    .set_content_from_iter=&vtable_set_content_from_iter,
    .get_text=&vtable_get_text,
    .get_text_size=&vtable_get_text_size,
    .get_child_position=&vtable_get_child_position,
};


UPTR(GemtextEnrichedTL) gemtext_enriched_tl_make(void) {
    GemtextConfig const * config = gemtext_config_default_get();
    return gemtext_enriched_tl_make_w_config(config);
}


UPTR(GemtextEnrichedTL) gemtext_enriched_tl_make_w_config(
    GemtextConfig const *config
) {
    GemtextConfigMem const * memory_strat = &config->memory_strat;
    UPTR(GemtextEnrichedTLImpl) impl =
        memory_strat->alloc(sizeof(GemtextEnrichedTLImpl));
    if (!impl) {
        return NULL;
    }
    GemtextEnrichedTL *interface = &impl->as.interface;

    impl->text = NULL;
    impl->text_size = 0;
    impl->text_capacity = 0;
    if (
        buffer_ops->ensure.has_capacity(
            *memory_strat,
            (void **)&impl->text,
            &impl->text_capacity,
            1
        )
    ) {
        memory_strat->free(impl);
        return NULL;
    }
    impl->text[0] = '\0';
    impl->children_info = NULL;

    GemtextErrType err;
    err = gemtext_container_init(&impl->as.container, &vtable.container);
    if (err != GEMTEXT_ERR__OK) {
        memory_strat->free(impl);
        memory_strat->free(impl->text);
        return NULL;
    }

    return interface;
};


size_t vtable_destroy(UPTR(GemtextNode) interface_node) {
    if (!interface_node) {
        return 0;
    }
    UPTR(GemtextEnrichedTLImpl) impl = (UPTR(void)) interface_node;
    GemtextConfigMem const *mem = &impl->as.node.memory_strat;
    if (impl->text) {
        mem->free(impl->text);
        impl->text_size = 0;
        impl->text_capacity = 0;
    }

    return remove_children(impl)
        + gemtext_node_vtable_destroy(interface_node);
};


int vtable_is_empty(GemtextNode const *node) {
    GemtextEnrichedTLImpl const *impl = (GemtextEnrichedTLImpl const *) node;

    if (!gemtext_container_vtable_is_empty(node)) {
        return 0;
    }

    for (size_t i = 0; i < impl->text_size && impl->text[i] != '\0'; i++) {
        if (!std68->is.wsp(impl->text[i])) {
            return 0;
        }
    }
    return 1;
};


size_t vtable_write_content_chunk(
    GemtextNode const *base_interface,
    size_t offset,
    char *buffer,
    size_t max_size
) {
    if (!base_interface) {
        return 0;
    }

    GemtextEnrichedTLImpl const *impl = (void const *) base_interface;
    GemtextEnrichedTL const *interface = &impl->as.interface;
    GemtextContainer const *container = &interface->as.container;
    GemtextEnrichedTLVTable const * vtable = impl->as.vtable;
    size_t const text_size = vtable->get_text_size(interface);
    char const * text = vtable->get_text(interface);
    size_t const children_count =
        gemtext_container_get_children_count(container);

    size_t writed_size = 0;
    size_t text_consumed_size = 0;
    GemtextNode const *child = NULL;
    size_t child_index = -1;
    size_t next_pos;
    do {
        child_index++;
        if (child_index < children_count) {
            child = gemtext_container_getc_child(container, child_index);
            next_pos = vtable->get_child_position(interface, child_index);
        } else {
            child = NULL;
            next_pos = text_size;
        }

        if (text_consumed_size < next_pos) {
            writed_size += buffer_ops->write.with_offset_from_buffer(
                &offset,
                (void **)(&buffer),
                &max_size,
                text + text_consumed_size,
                next_pos - text_consumed_size
            );
            text_consumed_size = next_pos;
        }

        if (!child) {
            continue;
        }

        size_t const child_content_size = gemtext_node_get_content_size(child);
        if (offset < child_content_size) {
            size_t child_writed_size = gemtext_node_write_content_chunk(
                child,
                offset,
                buffer,
                max_size
            );
            buffer += child_writed_size;
            max_size -= child_writed_size;
            writed_size += child_writed_size;
            offset = 0;
        } else {
            offset -= child_content_size;
        }
    } while (max_size && text_consumed_size < text_size);

    return writed_size;
};


size_t vtable_get_content_size(GemtextNode const *interface_node) {
    if (!interface_node) {
        return 0;
    }

    GemtextEnrichedTLImpl const *impl = (void const *) interface_node;
    GemtextEnrichedTL const *interface = &impl->as.interface;
    GemtextContainer const *container = &interface->as.container;
    size_t const children_count =
        gemtext_container_get_children_count(container);

    size_t content_size = 0;
    content_size += impl->text_size;

    for (size_t i = 0; i < children_count; i++) {
        GemtextNode const *child = gemtext_container_getc_child(container, i);
        content_size += gemtext_node_get_content_size(child);
    }

    return content_size;
 }


GemtextErrType vtable_trim_content(GemtextNode *interface_node) {
    if (!interface_node) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextErrType err = GEMTEXT_ERR__OK;
    GemtextEnrichedTLImpl *impl = (void *) interface_node;
    err = trim_text(impl);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }

    err = gemtext_container_vtable_trim_content(interface_node);
    return err;
};


GemtextNodeType vtable_get_type(GemtextNode const *interface) {
    (void) interface;
    return (GemtextNodeType) GEMTEXT_NODE_TYPE__TEXT_LINE;
};


GemtextErrType vtable_set_content_from_iter(
    GemtextEnrichedTL *interface,
    CharIterWithSavePoints *iter
) {
    int realloc_err = 0;
    UPTR(GemtextNode) child;
    GemtextErrType err = GEMTEXT_ERR__OK;
    if (!interface || !iter) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextEnrichedTLImpl *impl = (GemtextEnrichedTLImpl *)interface;
    GemtextConfigMem const *memory_strat = &impl->as.node.memory_strat;

    remove_children(impl);

    char curr_char = iter->get(iter);
    char prev_char = '\0';
    while(err == GEMTEXT_ERR__OK) {
        if (std68->is.lf(curr_char) || curr_char == '\0') {
            iter->next(iter);
            if (std68->is.cr(prev_char)) {
                impl->text[impl->text_size - 1] = '\0';
                impl->text_size--;
            }
            break;
        }
        // TODO: properly pass config
        child = get_child_from_buffer(iter, gemtext_config_default_get());

        if (child) {
            err = append_child(impl, child);
            curr_char = iter->get(iter);
        } else {
            realloc_err = buffer_ops->ensure.has_capacity(
                *memory_strat,
                (void **) &impl->text,
                &impl->text_capacity,
                impl->text_size + 2
            );
            if (realloc_err) {
                return GEMTEXT_ERR__ALLOC;
            }
            impl->text[impl->text_size] = curr_char;
            impl->text[impl->text_size+1] = '\0';
            impl->text_size++;
            prev_char = curr_char;
            curr_char = iter->next(iter);
        }
    }

    if (err != GEMTEXT_ERR__OK) {
        remove_children(impl);
        memory_strat->free(impl->text);
        impl->text_size = 0;
        impl->text_capacity = 0;
    }

    return err;
};



GemtextErrType vtable_append_at(
    GemtextContainer *i_container,
    GemtextNode *i_child,
    size_t index
) {
    unsigned int type = gemtext_node_get_type(i_child);
    GemtextExtensionsConfig const config = gemtext_ext_get_current_config();
    unsigned int const * p_allowed_type =
        config.enriched_text_line.allowed_children_ids;

    while (*p_allowed_type != 0 && *p_allowed_type != type) {
        p_allowed_type++;
    }
    if (*p_allowed_type == 0) {
        return GEMTEXT_ERR__INVALID_OP;
    }
    return gemtext_container_vtable_append_at(i_container, i_child, index);
};


char const * vtable_get_text(GemtextEnrichedTL const *interface) {
    if (!interface) {
        return NULL;
    }
    GemtextEnrichedTLImpl const *impl = (void const *)interface;
    return impl->text;
};


size_t vtable_get_text_size(GemtextEnrichedTL const *interface) {
    if (!interface) {
        return 0;
    }
    GemtextEnrichedTLImpl const *impl = (void const *)interface;
    return impl->text_size;
};


size_t vtable_get_child_position(
    GemtextEnrichedTL const *interface,
    size_t index
) {
    if (!interface) {
        return 0;
    }
    GemtextEnrichedTLImpl const *impl = (void const *)interface;
    size_t const children_count = gemtext_container_get_children_count(
        &interface->as.container
    );
    return index < children_count
        ? impl->children_info[index].position
        : impl->text_size + 1;
};


UPTR(GemtextNode) get_child_from_buffer(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    GemtextExtensionsConfig const ext_config = gemtext_ext_get_current_config();
    ChildFactory p_factory = *ext_config.enriched_text_line.children_factories;

    UPTR(GemtextNode) child = NULL;
    while (p_factory && !child) {
        child = p_factory(iter, config);
        p_factory++;
    }

    return child;
};


GemtextErrType append_child(
    GemtextEnrichedTLImpl *impl,
    UPTR(GemtextNode) child
) {
    GemtextConfigMem const *memory_strat = &impl->as.node.memory_strat;
    size_t const curr_pos = impl->text_size;
    GemtextContainer *container = &impl->as.interface.as.container;
    size_t const children_count = gemtext_container_get_children_count(
        container
    );

    size_t children_info_capacity = sizeof(ChildInfo) * children_count;
    int alloc_err = buffer_ops->ensure.has_capacity(
        *memory_strat,
        (void **) &impl->children_info,
        &children_info_capacity,
        sizeof(ChildInfo) * (children_count + 1)
    );
    if (alloc_err) {
        gemtext_node_destroy(child);
        return GEMTEXT_ERR__ALLOC;
    }

    impl->children_info[children_count] = (ChildInfo) {
        .position=curr_pos,
    };

    GemtextErrType err = gemtext_container_append(container, child);
    if (err != GEMTEXT_ERR__OK) {
        gemtext_node_destroy(child);
    }
    return err;
};


size_t remove_children(GemtextEnrichedTLImpl *impl) {
    size_t destroy_count = 0;
    GemtextEnrichedTL *tl = &impl->as.interface;
    GemtextContainer *container = &tl->as.container;
    size_t children_count = gemtext_container_get_children_count(container);
    while (children_count) {
        GemtextNode *child;
        child = gemtext_container_get_child(container, children_count - 1);
        destroy_count += gemtext_node_destroy(child);
        children_count = gemtext_container_get_children_count(container);
    }
    if (impl->children_info) {
        impl->as.node.memory_strat.free(impl->children_info);
        impl->children_info = NULL;
    }

    return destroy_count;
};


GemtextErrType trim_text(GemtextEnrichedTLImpl *impl) {
    if (impl->text_size == 0) {
        return GEMTEXT_ERR__OK;
    }

    // TODO: Properly implement
    return GEMTEXT_ERR__OK;
};
