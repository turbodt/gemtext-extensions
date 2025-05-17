#include "./base.h"
#include "./shared.h"


static GemtextExtensionsConfig config = {
    .thematic_break = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2025,
    },
    .markdown_link = {
        .id = GEMTEXT_NODE_TYPE__EXTENSION + 2026,
    }
};


GemtextExtensionsConfig gemtext_ext_get_current_config(void) {
    return config;
};


void gemtext_ext_set_config(GemtextExtensionsConfig new_config) {
    config.thematic_break = new_config.thematic_break;
};
