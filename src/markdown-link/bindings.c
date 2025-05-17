#include "./shared.h"


inline unsigned int gemtext_md_link_get_id(void) {
    return gemtext_ext_get_current_config().markdown_link.id;
};


inline size_t gemtext_md_link_destroy(UPTR(GemtextMDLink) mdl) {
    return gemtext_node_destroy(&mdl->as.node);
};


inline char const * gemtext_md_link_get_target(GemtextMDLink const *mdl) {
    return mdl->as.md_link->get_target(mdl);
};


inline GemtextErrType gemtext_md_link_set_target(
    GemtextMDLink *mdl,
    char const *str
) {
    return mdl->as.md_link->set_target(mdl, str);
};


inline char const * gemtext_md_link_get_section(GemtextMDLink const *mdl) {
    return mdl->as.md_link->get_section(mdl);
};


inline GemtextErrType gemtext_md_link_set_section(
    GemtextMDLink *mdl,
    char const *str
) {
    return mdl->as.md_link->set_section(mdl, str);
};


inline char const * gemtext_md_link_get_block(GemtextMDLink const *mdl) {
    return mdl->as.md_link->get_block(mdl);
};


inline GemtextErrType gemtext_md_link_set_block(
    GemtextMDLink *mdl,
    char const *str
) {
    return mdl->as.md_link->set_block(mdl, str);
};


inline char const * gemtext_md_link_get_alias(GemtextMDLink const *mdl) {
    return mdl->as.md_link->get_alias(mdl);
};


inline GemtextErrType gemtext_md_link_set_alias(
    GemtextMDLink *mdl,
    char const *str
) {
    return mdl->as.md_link->set_alias(mdl, str);
};


inline char const * gemtext_md_link_get_title(GemtextMDLink const *mdl) {
    return mdl->as.md_link->get_title(mdl);
};


inline GemtextErrType gemtext_md_link_set_title(
    GemtextMDLink *mdl,
    char const *str
) {
    return mdl->as.md_link->set_title(mdl, str);
};
