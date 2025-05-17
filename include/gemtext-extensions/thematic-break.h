#ifndef GEMTEXT_EXTENSIONS_THEMATIC_BREAK_H
#define GEMTEXT_EXTENSIONS_THEMATIC_BREAK_H


#include "./config.h"
#include <gemtext.h>


typedef struct GemtextThBreak GemtextThBreak;


typedef struct GemtextThBreakVTable {
    GemtextNodeVTable node;
    char const * (*get_text)(GemtextThBreak const *);
    GemtextErrType (*set_text)(GemtextThBreak *, char const *);
    GemtextErrType (*to_asterisc)(GemtextThBreak *);
    GemtextErrType (*to_dash)(GemtextThBreak *);
    GemtextErrType (*to_underscore)(GemtextThBreak *);
} GemtextThBreakVTable;


struct GemtextThBreak {
    union {
        GemtextThBreakVTable const * th_break;
        GemtextNode node;
    } as;
};


GemtextMiddleware * gemtext_th_break_get_middleware(void);
unsigned int gemtext_th_break_get_id(void);


UPTR(GemtextThBreak) gemtext_th_break_make(void);
UPTR(GemtextThBreak) gemtext_th_break_make_w_config(GemtextConfig const *);
size_t gemtext_th_break_destroy(UPTR(GemtextThBreak));
char const * gemtext_th_break_get_text(GemtextThBreak const *);
GemtextErrType gemtext_th_break_set_text(GemtextThBreak *, char const *);
GemtextErrType gemtext_th_break_to_asterisc(GemtextThBreak *);
GemtextErrType gemtext_th_break_to_dash(GemtextThBreak *);
GemtextErrType gemtext_th_break_to_underscore(GemtextThBreak *);


#endif
