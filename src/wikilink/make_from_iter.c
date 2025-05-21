#include "./shared.h"


static GemtextErrType parse_section_block(
    GemtextWikilink *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
static GemtextErrType parse_body(
    GemtextWikilink *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
static size_t set_target_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_alias_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_section_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_block_in_buffer(CharIterWithSavePoints *, char *, size_t);


UPTR(GemtextWikilink) gemtext_wikilink_make_from_iter_w_config(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    GemtextErrType err = GEMTEXT_ERR__OK;
    GemtextConfigMem const *mem_strat = &config->memory_strat;

    if (iter->get(iter) != '[') {
        return NULL;
    }

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    size_t buffer_size = wikilink_advance_iter(iter);
    if (!buffer_size) {
        goto FormatIncorrect;
    }
    buffer_size -= 3;

    iter->restore(iter);
    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    UPTR(char) buffer = mem_strat->alloc(sizeof(char) *buffer_size);
    if (!buffer) {
        goto BufferAllocFailed;
    }

    UPTR(GemtextWikilink) wikilink = gemtext_wikilink_make_w_config(config);
    if (!wikilink) {
        goto WikilinkAllocFailed;
    }

    iter->next(iter);
    iter->next(iter);

    err = parse_body(wikilink, iter, buffer_size-1, buffer);
    if (err != GEMTEXT_ERR__OK) {
        goto WikilinkBodyParsingFailed;
    };

    iter->next(iter);
    iter->next(iter);

    mem_strat->free(buffer);
    iter->drop(iter);
    return wikilink;

WikilinkBodyParsingFailed:
    gemtext_wikilink_destroy(wikilink);
WikilinkAllocFailed:
    mem_strat->free(buffer);
BufferAllocFailed:
FormatIncorrect:
    iter->restore(iter);
    return NULL;
};


GemtextErrType parse_body(
    GemtextWikilink *wikilink,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    GemtextErrType err = GEMTEXT_ERR__OK;

    size_t target_size = set_target_in_buffer(iter, buffer, segment_size);
    buffer[target_size] = '\0';

    err = wikilink->as.wikilink->set_target(wikilink, buffer);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }

    if (iter->get(iter) == '#') {
        err = parse_section_block(wikilink, iter, segment_size, buffer);
        if (err != GEMTEXT_ERR__OK) {
            return err;
        }
    }

    if (iter->get(iter) == '|') {
        iter->next(iter);
        size_t target_size = set_alias_in_buffer(iter, buffer, segment_size);
        buffer[target_size] = '\0';

        err = wikilink->as.wikilink->set_alias(wikilink, buffer);
        if (err != GEMTEXT_ERR__OK) {
            return err;
        }
    }
    return err;
};


GemtextErrType parse_section_block(
    GemtextWikilink *wikilink,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    GemtextErrType err = GEMTEXT_ERR__OK;

    size_t section_size = set_section_in_buffer(iter, buffer, segment_size);
    buffer[section_size] = '\0';

    err = wikilink->as.wikilink->set_section(wikilink, buffer);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }

    if (iter->get(iter) == '^') {
        iter->next(iter);
        size_t block_size = set_block_in_buffer(iter, buffer, segment_size);
        buffer[block_size] = '\0';

        err = wikilink->as.wikilink->set_block(wikilink, buffer);
        if (err != GEMTEXT_ERR__OK) {
            return err;
        }
    }

    return err;
};


size_t set_target_in_buffer(
    CharIterWithSavePoints *iter,
    char *buffer,
    size_t buffer_size
) {
    size_t writed_size = 0;
    int is_escaped = 0;
    int balance = 0;
    while (buffer_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writed_size] =  curr_char;
            writed_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '[') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && !balance) {
            break;
        } else if (curr_char == '#') {
            break;
        } else if (curr_char == '|') {
            break;
        } else {
            buffer[writed_size] =  curr_char;
            writed_size++;
        }

        iter->next(iter);
        buffer_size--;
    }
    buffer[writed_size] = '\0';
    return writed_size;
};


size_t set_alias_in_buffer(
    CharIterWithSavePoints *iter,
    char *buffer,
    size_t buffer_size
) {
    size_t writed_size = 0;
    int is_escaped = 0;
    int balance = 0;
    while (buffer_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writed_size] =  curr_char;
            writed_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '[') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && !balance) {
            break;
        } else {
            buffer[writed_size] =  curr_char;
            writed_size++;
        }

        iter->next(iter);
        buffer_size--;
    }
    buffer[writed_size] = '\0';
    return writed_size;
};


size_t set_section_in_buffer(
    CharIterWithSavePoints *iter,
    char *buffer,
    size_t buffer_size
) {
    size_t writed_size = 0;
    int is_escaped = 0;
    int balance = 0;
    while (buffer_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writed_size] =  curr_char;
            writed_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '[') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && !balance) {
            break;
        } else if (curr_char == '^') {
            break;
        } else if (curr_char == '|') {
            break;
        } else {
            buffer[writed_size] =  curr_char;
            writed_size++;
        }

        iter->next(iter);
        buffer_size--;
    }
    buffer[writed_size] = '\0';
    return writed_size;
};


size_t set_block_in_buffer(
    CharIterWithSavePoints *iter,
    char *buffer,
    size_t buffer_size
) {
    size_t writed_size = 0;
    int is_escaped = 0;
    int balance = 0;
    while (buffer_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writed_size] =  curr_char;
            writed_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '[') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ']' && !balance) {
            break;
        } else if (curr_char == '|') {
            break;
        } else {
            buffer[writed_size] =  curr_char;
            writed_size++;
        }

        iter->next(iter);
        buffer_size--;
    }
    buffer[writed_size] = '\0';
    return writed_size;
};
