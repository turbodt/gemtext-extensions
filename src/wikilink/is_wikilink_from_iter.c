#include "./shared.h"


size_t wikilink_advance_iter(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '[') {
        return 0;
    }

    size_t readed_size = 0;
    GemtextErrType err = GEMTEXT_ERR__OK;

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return 0;
    };
    iter->next(iter);
    readed_size++;

    if (iter->get(iter) != '[') {
        goto FormatInvalid;
    }
    iter->next(iter);
    readed_size++;

    size_t body_size = iter_ops
        ->advance.until_unbalanced_r_sq_bracket_eol(iter);
    readed_size += body_size;
    if (!body_size) {
        goto FormatInvalid;
    }

    if (iter->get(iter) != ']') {
        goto FormatInvalid;
    }
    iter->next(iter);
    readed_size++;
    if (iter->get(iter) != ']') {
        goto FormatInvalid;
    }
    iter->next(iter);
    readed_size++;

    iter->drop(iter);
    return readed_size;

FormatInvalid:
    iter->restore(iter);
    return 0;
}
