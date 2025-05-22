#include "./shared.h"

inline static int is_valid_first_char(char c);
inline static int is_valid_char(char c);

size_t hashtag_advance_iter(CharIterWithSavePoints *iter) {
    if (iter->get(iter) != '#') {
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

    if (!is_valid_first_char(iter->get(iter))) {
        goto FormatInvalid;
    }
    iter->next(iter);
    readed_size++;

    while(is_valid_char(iter->get(iter))) {
        iter->next(iter);
        readed_size++;
    }

    iter->drop(iter);
    return readed_size;

FormatInvalid:
    iter->restore(iter);
    return 0;
}


int is_valid_first_char(char c) {
    return (c >= 'A' && c <= 'Z')
    || (c >= 'a' && c <= 'z')
    || (c == '-')
    || (c == '_')
    || (c == '/');
};


int is_valid_char(char c) {
    return is_valid_first_char(c)
    || (c >= '0' && c <= '9');
};
