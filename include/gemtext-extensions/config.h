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
    struct {
        unsigned int id;
    } wikilink;
    struct {
        unsigned int id;
    } hashtag;
    struct {
        unsigned int id;
        unsigned int const * allowed_children_ids;
        GemtextNode * (**children_factories)(
            CharIterWithSavePoints *,
            GemtextConfig const *
        );
    } enriched_text_line;
    struct {
        unsigned int id;
    } checkbox;
} GemtextExtensionsConfig;


GemtextExtensionsConfig gemtext_ext_get_current_config(void);
void gemtext_ext_set_config(GemtextExtensionsConfig);


#endif
