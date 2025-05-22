#include "./shared.h"


static GemtextErrType parse_body(
    GemtextHashtag *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
inline static int is_valid_first_char(char c);
inline static int is_valid_char(char c);


UPTR(GemtextHashtag) gemtext_hashtag_make_from_iter_w_config(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    GemtextErrType err = GEMTEXT_ERR__OK;
    GemtextConfigMem const *mem_strat = &config->memory_strat;

    if (iter->get(iter) != '#') {
        return NULL;
    }

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    size_t buffer_size = hashtag_advance_iter(iter);
    if (!buffer_size) {
        goto FormatIncorrect;
    }
    buffer_size--;

    iter->restore(iter);
    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    UPTR(char) buffer = mem_strat->alloc(sizeof(char) *(buffer_size+1));
    if (!buffer) {
        goto BufferAllocFailed;
    }

    UPTR(GemtextHashtag) hashtag = gemtext_hashtag_make_w_config(config);
    if (!hashtag) {
        goto HashtagAllocFailed;
    }

    iter->next(iter);

    err = parse_body(hashtag, iter, buffer_size-1, buffer);
    if (err != GEMTEXT_ERR__OK) {
        goto HashtagBodyParsingFailed;
    };

    mem_strat->free(buffer);
    iter->drop(iter);
    return hashtag;

HashtagBodyParsingFailed:
    gemtext_hashtag_destroy(hashtag);
HashtagAllocFailed:
    mem_strat->free(buffer);
BufferAllocFailed:
FormatIncorrect:
    iter->restore(iter);
    return NULL;
};


GemtextErrType parse_body(
    GemtextHashtag *hashtag,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    GemtextErrType err = GEMTEXT_ERR__OK;

    size_t writed_size = 0;
    while (is_valid_char(iter->get(iter))) {
        buffer[writed_size] = iter->get(iter);
        writed_size++;

        iter->next(iter);
        segment_size--;
    }
    buffer[writed_size] = '\0';

    err = hashtag->as.hashtag->set_text(hashtag, buffer);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }
    return err;
};


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
