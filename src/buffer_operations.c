#include "./shared.h"
#include <string.h>


#ifndef MAX
#define MAX(a, b) (b > a ? b : a)
#endif


#define __ALLOC_MIN_SIZE 8


static int ensure_has_capacity(GemtextConfigMem, void **, size_t *, size_t);
static size_t write_with_offset_from_str(
    size_t *,
    void **,
    size_t *,
    char const *
);
static size_t write_with_offset_from_buffer(
    size_t *,
    void **,
    size_t *,
    void const *,
    size_t
);
static size_t str_trim(char *);
static size_t str_trim_realloc(GemtextConfigMem, char **);
static int str_set(GemtextConfigMem, char **, char const *);
static size_t buffer_trim(void *, size_t);
static inline int is_trimable(char c);
static size_t get_pow2_upperbound(size_t);
static GemtextExtensionsBufferOperations buffer_ops_value = {
    .ensure = {
        .has_capacity=&ensure_has_capacity,
    },
    .write = {
        .with_offset_from_str=&write_with_offset_from_str,
        .with_offset_from_buffer=&write_with_offset_from_buffer,
    },
    .str = {
        .trim=&str_trim,
        .trim_realloc=&str_trim_realloc,
        .set=&str_set,
    },
};
GemtextExtensionsBufferOperations const * gemtext_ext_buffer_operations =
    &buffer_ops_value;


int ensure_has_capacity(
    GemtextConfigMem mem_strat,
    void **p_buffer,
    size_t *p_curr_capacity,
    size_t target_capacity
) {
    size_t const curr_capacity = *p_curr_capacity;
    target_capacity = get_pow2_upperbound(target_capacity);
    target_capacity = MAX(target_capacity, __ALLOC_MIN_SIZE);

    if (curr_capacity == target_capacity) {
        return 0;
    }

    void *new_ptr = NULL;
    if (*p_buffer) {
        new_ptr = mem_strat.realloc(*p_buffer, sizeof(char)*target_capacity);
    } else {
        new_ptr = mem_strat.alloc(sizeof(char)*target_capacity);
    }
    if (!new_ptr) {
        return 1;
    }

    *p_buffer = new_ptr;
    *p_curr_capacity = target_capacity;

    return 0;
}


size_t write_with_offset_from_str(
    size_t *p_offset,
    void **p_buffer,
    size_t *p_max_size,
    char const *source
) {
    if (!source) {
        return 0;
    }
    return write_with_offset_from_buffer(
        p_offset,
        p_buffer,
        p_max_size,
        source,
        strlen(source)
    );
}


size_t write_with_offset_from_buffer(
    size_t *p_offset,
    void **p_buffer,
    size_t *p_max_size,
    void const *source,
    size_t source_size
) {
    size_t writed_size = 0;

    if (source_size <= *p_offset) {
        *p_offset -= source_size;
        return writed_size;
    }

    for (
        size_t j = *p_offset;
        j < source_size && *p_max_size > 0;
        j++
    ) {
        ((char *)*p_buffer)[0] = ((char *)source)[j];
        (*p_buffer)++;
        (*p_max_size)--;
        writed_size++;
    }
    *p_offset = 0;

    return writed_size;
};


size_t str_trim(char *content) {
    if (!content) {
        return 0;
    }
    size_t content_len = strlen(content);
    return buffer_trim(content, content_len);
}


size_t str_trim_realloc(GemtextConfigMem mem_strat, char **p_content) {
    if (!(*p_content)) {
        return 0;
    }
    size_t content_len = strlen(*p_content);
    if (!content_len) {
        return 0;
    }
    size_t trimed_size = buffer_trim(*p_content, content_len);

    char *new_ptr = mem_strat.realloc(
        *p_content,
        sizeof(char)*(content_len - trimed_size)
    );
    if (new_ptr) {
        *p_content = new_ptr;
    }

    return trimed_size;
}


int str_set(GemtextConfigMem mem_strat, char **p_dst, char const *src) {
    int err = 0;

    size_t const src_len = src ? strlen(src): 0;
    if (!src_len && *p_dst) {
        mem_strat.free(*p_dst);
        *p_dst = NULL;
        return 0;
    }

    size_t curr_len = *p_dst ? strlen(*p_dst) : 0;
    err = ensure_has_capacity(
        mem_strat,
        (void **)p_dst,
        &curr_len,
        (src_len + 1)
    );
    if (err) {
        return err;
    }

    memcpy(*p_dst, src, src_len);
    (*p_dst)[src_len] = '\0';

    return err;
};


size_t buffer_trim(void *content, size_t content_size) {
    size_t i = 0;
    size_t trimed_size = 0;
    char *a = content;
    char *b = content;
    int has_cr = 0;

    while (i < content_size && is_trimable(*b)) {
        b++;
        i++;
        trimed_size++;
    }

    while (i < content_size && trimed_size > 0) {
        *a = *b;
        a++;
        b++;
        i++;
    }
    if (!trimed_size) {
        b = content + content_size;
        a = b;
    }

    has_cr = (content_size > 0) && std68->is.cr(*(b-1));

    while (a != b) {
        *a = '\0';
        a++;
    }

    a = content + content_size - trimed_size;
    if (has_cr) {
        a--;
    }
    b = a;
    while (a != content && is_trimable(*(a-1))) {
        if (has_cr) {
            *a = '\0';
            *(a-1) = '\r';
        } else {
            *(a-1) = '\0';
        }
        a--;
        trimed_size++;
    }

    return trimed_size;
};


inline int is_trimable(char c) {
    return std68->is.wsp(c) || c == '\0';
};


size_t get_pow2_upperbound(size_t x) {
    if (!x) {
        return 0;
    }
    x--;

    size_t bound = 1;
    while (x) {
        bound = bound << 1;
        x = x >> 1;
    }
    return bound;
};
