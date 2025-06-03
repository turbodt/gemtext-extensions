#include "./base.h"
#include "./shared.h"


typedef UPTR(GemtextNode)(*ChildFactory)(
    CharIterWithSavePoints *,
    GemtextConfig const *
);


static GemtextExtensionsConfig config = {
    .thematic_break = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2025,
    },
    .markdown_link = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2026,
    },
    .wikilink = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2027,
    },
    .hashtag = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2028,
    },
    .enriched_text_line = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2050,
        .allowed_children_ids = (unsigned int []){
            GEMTEXT_NODE_TYPE__EXTENSION + 2026,
            GEMTEXT_NODE_TYPE__EXTENSION + 2027,
            GEMTEXT_NODE_TYPE__EXTENSION + 2028,
            0,
        },
        .children_factories = (ChildFactory[]) {
            (ChildFactory) &gemtext_md_link_make_from_iter_w_config,
            (ChildFactory) &gemtext_wikilink_make_from_iter_w_config,
            (ChildFactory) &gemtext_hashtag_make_from_iter_w_config,
            NULL,
        }
    },
    .checkbox = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2051,
    },
};


GemtextExtensionsConfig gemtext_ext_get_current_config(void) {
    return config;
};


void gemtext_ext_set_config(GemtextExtensionsConfig new_config) {
    config.thematic_break = new_config.thematic_break;
    config.checkbox = new_config.checkbox;
    config.enriched_text_line = new_config.enriched_text_line;
    config.hashtag = new_config.hashtag;
    config.wikilink = new_config.wikilink;
    config.markdown_link = new_config.markdown_link;
};
