#ifndef GEMTEXT_EXTENSIONS_CHAR_ITER_OPERATIONS_H
#define GEMTEXT_EXTENSIONS_CHAR_ITER_OPERATIONS_H


#include "./base.h"
#include <gemtext.h>


typedef struct {
    struct {
        UPTR(char) (*line)(
            CharIterWithSavePoints *,
            UPTR(void) (*)(size_t)
        );
    } fetch;
    struct {
        size_t (*until_next_line)(CharIterWithSavePoints *);
        size_t (*until_eol)(CharIterWithSavePoints *);
        size_t (*until_unbalanced_r_par_eol)(CharIterWithSavePoints *);
        size_t (*until_unbalanced_r_sq_bracket_eol)(CharIterWithSavePoints *);
        size_t (*while_wsp)(CharIterWithSavePoints *);
    } advance;
    struct {
        int (*eol)(CharIterWithSavePoints const *);
        int (*cr)(CharIterWithSavePoints const *);
        int (*lf)(CharIterWithSavePoints const *);
        int (*wsp)(CharIterWithSavePoints const *);
        int (*blank_line)(CharIterWithSavePoints *);
        int (*preformat)(CharIterWithSavePoints *);
        int (*link)(CharIterWithSavePoints *);
        int (*list_item)(CharIterWithSavePoints *);
        int (*heading)(CharIterWithSavePoints *);
        int (*quote_line)(CharIterWithSavePoints *);
    } is;
} GemtextExtensionsIterOperations;


extern PROTECTED GemtextExtensionsIterOperations const * gemtext_ext_iterable_operations;

#endif
