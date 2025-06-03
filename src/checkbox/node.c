#include "./shared.h"
#include <gemtext/extension.h>
#include <string.h>


typedef struct {
    union {
        GemtextCheckboxVTable const * vtable;
        GemtextNodeImpl node;
        GemtextCheckbox interface;
    } as;
    UPTR(GemtextNode) child;
    int state;
    char sep_char;
} GemtextCheckboxImpl;


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
static GemtextNode * get_content(GemtextCheckbox *);
static GemtextErrType set_content(GemtextCheckbox *, UPTR(GemtextNode));
static GemtextErrType set_separator(GemtextCheckbox *, char);
static int is_checked(GemtextCheckbox const *);
static void check(GemtextCheckbox *);
static void uncheck(GemtextCheckbox *);
static void toggle(GemtextCheckbox *);
static size_t children_delete(GemtextCheckboxImpl *);
static GemtextCheckboxVTable vtable = {
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
    .get_content = &get_content,
    .getc_content = (GemtextNode const *(*)(GemtextCheckbox const *))&get_content,
    .set_content = &set_content,
    .set_separator = &set_separator,
    .is_checked = &is_checked,
    .check = &check,
    .uncheck = &uncheck,
    .toggle = &toggle,
};


inline UPTR(GemtextCheckbox) gemtext_checkbox_make(void) {
    GemtextConfig const * config = gemtext_config_default_get();
    return gemtext_checkbox_make_w_config(config);
}


UPTR(GemtextCheckbox) gemtext_checkbox_make_w_config(GemtextConfig const *config) {
    GemtextConfigMem const * mem = &config->memory_strat;
    UPTR(GemtextCheckboxImpl) impl = mem->alloc(sizeof(GemtextCheckboxImpl));
    if (!impl) {
        return NULL;
    }

    GemtextErrType err;
    err = gemtext_node_init_w_config(&impl->as.node, &vtable.node, config);
    if (err != GEMTEXT_ERR__OK) {
        mem->free(impl);
        return NULL;
    }

    impl->child = NULL;
    impl->sep_char = ' ';
    return &impl->as.interface;
}


size_t vtable_destroy(UPTR(GemtextNode) interface) {
    if (!interface) {
        return 0;
    }
    UPTR(GemtextCheckboxImpl) impl = (UPTR(GemtextCheckboxImpl)) interface;

    return children_delete(impl) + gemtext_node_vtable_destroy(interface);
};


int vtable_is_empty(GemtextNode const *node) {
    if (!node) {
        return 1;
    }
    return 0;
    /*
    GemtextCheckboxImpl const * impl = (void const *) node;

    return !impl->child || gemtext_node_is_empty(impl->child);
    */
};


GemtextNodeType vtable_get_type(GemtextNode const *interface) {
    (void) interface;
    return gemtext_ext_get_current_config().checkbox.id;
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
    GemtextCheckboxImpl const *impl = (GemtextCheckboxImpl const *) interface;
    size_t writed_size = 0;

    writed_size += buffer_ops->write.with_offset_from_str(
        &offset,
        (void **)&buffer,
        &max_size,
        impl->state ? "[x]" : "[ ]"
    );

    if (!impl->child || !max_size) {
        return writed_size;
    }

    if (offset) {
        offset--;
    } else {
        buffer[0] = impl->sep_char;
        buffer++;
        max_size--;
        writed_size++;
    }

    writed_size +=
        gemtext_node_write_content_chunk(impl->child, offset, buffer, max_size);

    return writed_size;
};


size_t vtable_get_content_size(GemtextNode const *interface) {
    if (!interface) {
        return 0;
    }
    GemtextCheckboxImpl const * impl = (GemtextCheckboxImpl const *) interface;

    size_t content_size = 3;

    if (impl->child) {
        content_size += 1 + gemtext_node_get_content_size(impl->child);
    }

    return content_size;
};


GemtextErrType vtable_trim_content(GemtextNode *interface) {
    if (!interface) {
        return 0;
    }
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;

    if (!impl->child) {
        return GEMTEXT_ERR__OK;
    }

    return gemtext_node_trim_content(impl->child);
};


GemtextNode * get_content(GemtextCheckbox *interface) {
    GemtextCheckboxImpl const *impl = (GemtextCheckboxImpl const *) interface;
    return impl->child;
};


GemtextErrType set_content(
    GemtextCheckbox *interface,
    UPTR(GemtextNode) child
) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;
    children_delete(impl);
    impl->child = child;
    return GEMTEXT_ERR__OK;
};


GemtextErrType set_separator(GemtextCheckbox *interface, char separator) {
    if (!interface) {
        return GEMTEXT_ERR__INVALID_ARG;
    }
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;
    if (!std68->is.wsp(separator)) {
        return GEMTEXT_ERR__INVALID_ARG;
    };
    impl->sep_char = separator;
    return GEMTEXT_ERR__OK;
};


int is_checked(GemtextCheckbox const *interface) {
    GemtextCheckboxImpl const *impl = (GemtextCheckboxImpl const *) interface;
    return impl->state;
};


void check(GemtextCheckbox *interface) {
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;
    impl->state = 1;
};


void uncheck(GemtextCheckbox *interface) {
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;
    impl->state = 0;
};


void toggle(GemtextCheckbox *interface) {
    GemtextCheckboxImpl * impl = (GemtextCheckboxImpl *) interface;
    impl->state = 1 - impl->state;
};


inline size_t children_delete(GemtextCheckboxImpl *impl) {
    if (!impl->child) {
        return 0;
    }
    size_t children_count = gemtext_node_destroy(impl->child);
    impl->child = NULL;
    return children_count;
};
