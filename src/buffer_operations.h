#ifndef GEMTEXT_EXTENSIONS_BUFFER_OPERATIONS_H
#define GEMTEXT_EXTENSIONS_BUFFER_OPERATIONS_H


#include "./base.h"
#include "gemtext.h"


typedef struct {
    struct {
        int (*has_capacity)(GemtextConfigMem, void **, size_t *, size_t);
    } ensure;
    struct {
        size_t (*with_offset_from_buffer)(
            size_t *,
            void **,
            size_t *,
            void const *,
            size_t
        );
        size_t (*with_offset_from_str)(
            size_t *,
            void **,
            size_t *,
            char const *
        );
    } write;
    struct {
        size_t (*trim)(char *);
        size_t (*trim_realloc)(GemtextConfigMem, char **);
        int (*set)(GemtextConfigMem, char **, char const *);
    } str;
} GemtextExtensionsBufferOperations;


extern GemtextExtensionsBufferOperations const * gemtext_ext_buffer_operations;


#endif
