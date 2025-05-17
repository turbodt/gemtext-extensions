#include "./shared.h"


size_t md_link_advance_iter(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '[') {
        return 0;
    }

    GemtextErrType err = GEMTEXT_ERR__OK;

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return 0;
    };

    size_t seg1_size = md_link_seg1_advance_iter(iter);
    if (!seg1_size) {
        iter->restore(iter);
        return 0;
    }

    iter->next(iter);

    size_t seg2_size = md_link_seg2_advance_iter(iter);
    if (!seg2_size) {
        iter->restore(iter);
        return 0;
    }
    iter->next(iter);

    iter->drop(iter);
    return seg1_size + seg2_size;
}


size_t md_link_seg1_advance_iter(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '[') {
        return 0;
    }

    size_t readed_size = 0;
    GemtextErrType err = GEMTEXT_ERR__OK;

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        iter->restore(iter);
        return 0;
    };

    iter->next(iter);
    readed_size++;

    readed_size += iter_ops->advance.until_unbalanced_r_sq_bracket_eol(iter);

    if (iter->get(iter) != ']') {
        iter->restore(iter);
        return 0;
    }

    iter->drop(iter);
    return readed_size;
};


size_t md_link_seg2_advance_iter(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '(') {
        return 0;
    }

    size_t readed_size = 0;
    GemtextErrType err = GEMTEXT_ERR__OK;

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return 0;
    };

    char curr_char = '\0';
    int is_escaped = 0;
    int balance = 0;
    while (!iter_ops->is.eol(iter)) {
        curr_char = iter->next(iter);
        readed_size++;

        if (is_escaped) {
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '(') {
            balance++;
        } else if (curr_char == ')' && balance > 0) {
            balance--;
        } else if (curr_char == ')' && balance == 0) {
            break;
        } else if (std68->is.wsp(curr_char)) {
            break;
        }
    }

    if (std68->is.wsp(curr_char)) {
        readed_size += iter_ops->advance.while_wsp(iter);
        char delim = iter->get(iter);
        if (delim != '(' && delim != '"' && delim != '\'') {
            iter->restore(iter);
            return 0;
        }
        if (delim == '(') {
            delim = ')';
        }

        while (!iter_ops->is.eol(iter)) {
            curr_char = iter->next(iter);
            readed_size++;

            if (is_escaped) {
                is_escaped = 0;
            } else if (curr_char == '\\') {
                is_escaped = 1;
            } else if (curr_char == delim) {
                break;
            }
        }

        if (curr_char != delim) {
            iter->restore(iter);
            return 0;
        }

        iter->next(iter);
        readed_size++;
    }

    if (iter->get(iter) != ')') {
        iter->restore(iter);
        return 0;
    }

    iter->drop(iter);
    return readed_size;
}
