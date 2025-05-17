#include "./shared.h"


static GemtextErrType parse_segment_1(
    GemtextMDLink *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
static GemtextErrType parse_segment_2(
    GemtextMDLink *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
static GemtextErrType parse_section_block(
    GemtextMDLink *,
    CharIterWithSavePoints *,
    size_t,
    char *
);
static size_t set_target_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_section_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_block_in_buffer(CharIterWithSavePoints *, char *, size_t);
static size_t set_title_in_buffer(CharIterWithSavePoints *, char *, size_t);


UPTR(GemtextMDLink) gemtext_md_link_make_from_iter_w_config(
    CharIterWithSavePoints *iter,
    GemtextConfig const *config
) {
    size_t seg1_size = 0;
    size_t seg2_size = 0;
    GemtextErrType err = GEMTEXT_ERR__OK;
    GemtextConfigMem const *mem_strat = &config->memory_strat;

    if (iter->get(iter) != '[') {
        return NULL;
    }

    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    seg1_size = md_link_seg1_advance_iter(iter);
    if (!seg1_size) {
        goto FormatIncorrect;
    }
    seg1_size--;

    iter->next(iter);

    seg2_size = md_link_seg2_advance_iter(iter);
    if (!seg2_size) {
        goto FormatIncorrect;
    }
    seg2_size--;

    iter->restore(iter);
    err = iter->set_savepoint(iter);
    if (err != GEMTEXT_ERR__OK) {
        return NULL;
    };

    size_t buffer_size = (seg1_size < seg2_size ? seg2_size : seg1_size) + 1;
    UPTR(char) buffer = mem_strat->alloc(sizeof(char) *buffer_size);
    if (!buffer) {
        goto BufferAllocFailed;
    }

    UPTR(GemtextMDLink) md_link = gemtext_md_link_make_w_config(config);
    if (!md_link) {
        goto MDLinkAllocFailed;
    }

    iter->next(iter);

    err = parse_segment_1(md_link, iter, seg1_size, buffer);
    if (err != GEMTEXT_ERR__OK) {
        goto MDLinkSegmentFailed;
    };

    iter->next(iter);
    iter->next(iter);

    err = parse_segment_2(md_link, iter, seg2_size, buffer);
    if (err != GEMTEXT_ERR__OK) {
        goto MDLinkSegmentFailed;
    };

    iter->next(iter);

    mem_strat->free(buffer);
    iter->drop(iter);
    return md_link;

MDLinkSegmentFailed:
    gemtext_md_link_destroy(md_link);
MDLinkAllocFailed:
    mem_strat->free(buffer);
BufferAllocFailed:
FormatIncorrect:
    iter->restore(iter);
    return NULL;
};


GemtextErrType parse_segment_1(
    GemtextMDLink *md_link,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    int is_escaped = 0;
    size_t writen_size = 0;
    while (segment_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writen_size] =  curr_char;
            writen_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else {
            buffer[writen_size] =  curr_char;
            writen_size++;
        }

        iter->next(iter);
        segment_size--;
    }
    buffer[writen_size] = '\0';

    return md_link->as.md_link->set_alias(md_link, buffer);
};


GemtextErrType parse_segment_2(
    GemtextMDLink *md_link,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    GemtextErrType err = GEMTEXT_ERR__OK;

    size_t target_size = set_target_in_buffer(iter, buffer, segment_size);
    buffer[target_size] = '\0';

    err = md_link->as.md_link->set_target(md_link, buffer);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }

    if (iter->get(iter) == '#') {
        err = parse_section_block(md_link, iter, segment_size, buffer);
        if (err != GEMTEXT_ERR__OK) {
            return err;
        }
    }

    if (iter_ops->is.wsp(iter)) {
        size_t title_size = set_title_in_buffer(iter, buffer, segment_size);
        buffer[title_size] = '\0';

        err = md_link->as.md_link->set_title(md_link, buffer);
        if (err != GEMTEXT_ERR__OK) {
            return err;
        }
        iter->next(iter);
    }

    return err;
};


GemtextErrType parse_section_block(
    GemtextMDLink *md_link,
    CharIterWithSavePoints *iter,
    size_t segment_size,
    char *buffer
) {
    GemtextErrType err = GEMTEXT_ERR__OK;

    size_t section_size = set_section_in_buffer(iter, buffer, segment_size);
    buffer[section_size] = '\0';

    err = md_link->as.md_link->set_section(md_link, buffer);
    if (err != GEMTEXT_ERR__OK) {
        return err;
    }

    if (iter->get(iter) == '^') {
        size_t block_size = set_block_in_buffer(iter, buffer, segment_size);
        buffer[block_size] = '\0';

        err = md_link->as.md_link->set_block(md_link, buffer);
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
        } else if (curr_char == '(') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && !balance) {
            break;
        } else if (curr_char == '#') {
            break;
        } else if (std68->is.wsp(curr_char)) {
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
        } else if (curr_char == '(') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && !balance) {
            break;
        } else if (curr_char == '^') {
            break;
        } else if (std68->is.wsp(curr_char)) {
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
    iter->next(iter);

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
        } else if (curr_char == '(') {
            balance++;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && balance > 0) {
            balance--;
            buffer[writed_size] =  curr_char;
            writed_size++;
        } else if (curr_char == ')' && !balance) {
            break;
        } else if (std68->is.wsp(curr_char)) {
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


size_t set_title_in_buffer(
    CharIterWithSavePoints *iter,
    char *buffer,
    size_t buffer_size
) {

    iter_ops->advance.while_wsp(iter);
    char delim = iter->get(iter);
    if (delim == '(') {
        delim = ')';
    }
    iter->next(iter);

    size_t writed_size = 0;
    int is_escaped = 0;
    while (buffer_size) {
        char curr_char = iter->get(iter);

        if (is_escaped) {
            buffer[writed_size] =  curr_char;
            writed_size++;
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == delim) {
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
