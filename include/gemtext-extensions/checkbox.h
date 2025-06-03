#ifndef GEMTEXT_EXTENSIONS_CHECKBOX_H
#define GEMTEXT_EXTENSIONS_CHECKBOX_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextCheckbox GemtextCheckbox;


typedef struct GemtextCheckboxVTable {
    GemtextNodeVTable node;
    GemtextNode const * (*getc_content)(GemtextCheckbox const *);
    GemtextNode * (*get_content)(GemtextCheckbox *);
    GemtextErrType (*set_content)(GemtextCheckbox *, UPTR(GemtextNode));
    GemtextErrType (*set_separator)(GemtextCheckbox *, char);
    int (*is_checked)(GemtextCheckbox const *);
    void (*toggle)(GemtextCheckbox *);
    void (*check)(GemtextCheckbox *);
    void (*uncheck)(GemtextCheckbox *);
} GemtextCheckboxVTable;


struct GemtextCheckbox {
    union {
        GemtextCheckboxVTable const * checkbox;
        GemtextNode node;
    } as;
};


extern GemtextMiddleware * gemtext_checkbox_get_middleware(void);
unsigned int gemtext_checkbox_get_id(void);


UPTR(GemtextCheckbox) gemtext_checkbox_make(void);
UPTR(GemtextCheckbox) gemtext_checkbox_make_w_config(GemtextConfig const *);
GemtextNode const * gemtext_checkbox_getc_content(GemtextCheckbox const *);
GemtextNode * gemtext_checkbox_get_content(GemtextCheckbox *);
GemtextErrType gemtext_checkbox_set_content(GemtextCheckbox *, UPTR(GemtextNode));
GemtextErrType gemtext_checkbox_set_separator(GemtextCheckbox *, char);
int gemtext_checkbox_is_checked(GemtextCheckbox const *);
void gemtext_checkbox_check(GemtextCheckbox *);
void gemtext_checkbox_uncheck(GemtextCheckbox *);
void gemtext_checkbox_toggle(GemtextCheckbox *);
size_t gemtext_checkbox_destroy(UPTR(GemtextCheckbox));


#endif
