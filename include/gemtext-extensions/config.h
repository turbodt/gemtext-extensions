#ifndef GEMTEXT_EXTENSIONS_CONFIG_H
#define GEMTEXT_EXTENSIONS_CONFIG_H


#include <gemtext.h>


typedef struct {
    struct {
        unsigned int id;
    } thematic_break;
    struct {
        unsigned int id;
    } markdown_link;
} GemtextExtensionsConfig;


GemtextExtensionsConfig gemtext_ext_get_current_config(void);
void gemtext_ext_set_config(GemtextExtensionsConfig);


#endif
