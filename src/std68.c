#include "./shared.h"


static int is_std68_cr(char);
static int is_std68_lf(char);
static int is_std68_wsp(char);
static GemtextExtensionsStd68 std68_value = {
    .is = {
        .cr=&is_std68_cr,
        .lf=&is_std68_lf,
        .wsp=&is_std68_wsp,
    },
};


GemtextExtensionsStd68 const *gemtext_ext_std68 = &std68_value;


int is_std68_cr(char c) {
    return c == '\r';
};


int is_std68_lf(char c) {
    return c == '\n';
};


int is_std68_wsp(char c) {
    return c == '\t' || c == ' ';
};
