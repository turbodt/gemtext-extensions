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
    .enriched_text_line = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2030,
        .allowed_children_ids = (unsigned int []){
            GEMTEXT_NODE_TYPE__EXTENSION + 2026,
            0,
        },
        .children_factories = (ChildFactory[]) {
            (ChildFactory) &gemtext_md_link_make_from_iter_w_config,
            NULL,
        }
    },
};


GemtextExtensionsConfig gemtext_ext_get_current_config(void) {
    return config;
};


void gemtext_ext_set_config(GemtextExtensionsConfig new_config) {
    config.thematic_break = new_config.thematic_break;
};
