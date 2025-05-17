#include "./char_iter_operations.h"
#include "./shared.h"
#include <string.h>


static UPTR(char) read_line(
    CharIterWithSavePoints *,
    UPTR(void) (*)(size_t)
);
static size_t advance_until_next_line(CharIterWithSavePoints *);
static size_t advance_until_eol(CharIterWithSavePoints *);
static size_t advance_until_unbalanced_r_par_eol(CharIterWithSavePoints *);
static size_t advance_until_unbalanced_r_sq_bracket_eol(
    CharIterWithSavePoints *
);
static size_t advance_while_wsp(CharIterWithSavePoints *);
static int is_blank_line(CharIterWithSavePoints *);
static int is_preformat(CharIterWithSavePoints *);
static int is_link(CharIterWithSavePoints *);
static int is_list_item(CharIterWithSavePoints *);
static int is_heading(CharIterWithSavePoints *);
static int is_quote_line(CharIterWithSavePoints *);
static int is_eol(CharIterWithSavePoints const *);
static int is_cr(CharIterWithSavePoints const *);
static int is_lf(CharIterWithSavePoints const *);
static int is_wsp(CharIterWithSavePoints const *);


static GemtextExtensionsIterOperations iter_ops_value = {
    .fetch = {
        .line=&read_line,
    },
    .advance = {
        .until_next_line=&advance_until_next_line,
        .until_eol=&advance_until_eol,
        .until_unbalanced_r_par_eol=&advance_until_unbalanced_r_par_eol,
        .until_unbalanced_r_sq_bracket_eol=
            &advance_until_unbalanced_r_sq_bracket_eol,
        .while_wsp=&advance_while_wsp,
    },
    .is = {
        .eol=&is_eol,
        .cr=&is_cr,
        .lf=&is_lf,
        .wsp=&is_wsp,
        .blank_line=&is_blank_line,
        .preformat=&is_preformat,
        .link=&is_link,
        .list_item=&is_list_item,
        .heading=&is_heading,
        .quote_line=&is_quote_line,
    }
};
GemtextExtensionsIterOperations const * gemtext_ext_iterable_operations = &iter_ops_value;


UPTR(char) read_line(
    CharIterWithSavePoints *iter,
    UPTR(void) (*alloc)(size_t)
) {
    if (iter->set_savepoint(iter) != GEMTEXT_ERR__OK) {
        return NULL;
    };
    size_t line_size = advance_until_eol(iter);
    iter->restore(iter);

    UPTR(char) line = alloc(sizeof(char)*(line_size + 1));
    if (!line) {
        return line;
    }

    size_t index = 0;
    while (index < line_size) {
        line[index] = iter->get(iter);
        iter->next(iter);
        index++;
    }
    line[index] = '\0';

    advance_until_next_line(iter);

    return line;
};


size_t advance_until_eol(CharIterWithSavePoints *iter) {
    size_t size_count = 0;
    iter->set_savepoint(iter);
    char prev_char = '\0';

    while(!is_eol(iter)) {
        prev_char = iter->get(iter);
        iter->next(iter);
        size_count++;
    }
    if (is_lf(iter) && std68->is.cr(prev_char)) {
        size_count--;
        iter->restore(iter);
        for (size_t j = 0; j <size_count; j++) {
            iter->next(iter);
        }
    } else {
        iter->drop(iter);
    }
    return size_count;
};


size_t advance_until_next_line(CharIterWithSavePoints *iter) {
    size_t size_count = 0;
    while(!is_eol(iter)) {
        iter->next(iter);
        size_count++;
    }
    if (is_lf(iter)) {
        iter->next(iter);
        size_count++;
    }
    return size_count;
};


size_t advance_until_unbalanced_r_par_eol(CharIterWithSavePoints *iter) {
    size_t size_count = 0;
    int is_escaped = 0;
    int balance = 0;
    char curr_char = iter->get(iter);
    while (!is_eol(iter) && (is_escaped || balance > 0 || curr_char != ')')) {
        if (is_escaped) {
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '(') {
            balance++;
        } else if (curr_char == ')') {
            balance--;
        }
        curr_char = iter->next(iter);
        size_count++;
    }
    return size_count;
};


size_t advance_until_unbalanced_r_sq_bracket_eol(
    CharIterWithSavePoints *iter
) {
    size_t size_count = 0;
    int is_escaped = 0;
    int balance = 0;
    char curr_char = iter->get(iter);
    while (!is_eol(iter) && (is_escaped || balance > 0 || curr_char != ']')) {
        if (is_escaped) {
            is_escaped = 0;
        } else if (curr_char == '\\') {
            is_escaped = 1;
        } else if (curr_char == '[') {
            balance++;
        } else if (curr_char == ']') {
            balance--;
        }
        curr_char = iter->next(iter);
        size_count++;
    }
    return size_count;
};


size_t advance_while_wsp(CharIterWithSavePoints *iter) {
    size_t size_count = 0;
    while (is_wsp(iter)) {
        iter->next(iter);
        size_count++;
    }
    return size_count;
};


int is_blank_line(CharIterWithSavePoints *iter) {
    iter->set_savepoint(iter);
    char curr_char = iter->get(iter);
    while (!std68->is.lf(curr_char) && curr_char != '\0') {
        if (std68->is.cr(curr_char)) {
            curr_char = iter->next(iter);
            return std68->is.lf(curr_char);
        } else if (!std68->is.wsp(curr_char)) {
            iter->restore(iter);
            return 0;
        }
        curr_char = iter->next(iter);
    }
    iter->restore(iter);
    return 1;
};


int is_preformat(CharIterWithSavePoints *iter) {
    iter->set_savepoint(iter);
    char pre[4] = {0};
    pre[0] = iter->get(iter);
    pre[1] = iter->next(iter);
    pre[2] = iter->next(iter);

    if (strcmp(pre, "```") != 0) {
        iter->restore(iter);
        return 0;
    }

    iter->restore(iter);
    return 1;
}


int is_link(CharIterWithSavePoints *iter) {
    iter->set_savepoint(iter);
    char pre[3] = {0};
    pre[0] = iter->get(iter);
    pre[1] = iter->next(iter);

    if (strcmp(pre, "=>") != 0) {
        iter->restore(iter);
        return 0;
    }

    if (is_blank_line(iter)) {
        iter->restore(iter);
        return 0;
    }

    iter->restore(iter);
    return 1;
};


int is_list_item(CharIterWithSavePoints *iter) {
    return iter->get(iter) == '*';
};


int is_heading(CharIterWithSavePoints *iter) {
    return iter->get(iter) == '#';
};


int is_quote_line(CharIterWithSavePoints *iter) {
    return iter->get(iter) == '>';
};


int is_eol(CharIterWithSavePoints const *iter) {
    char const c = iter->get(iter);
    return std68->is.lf(c) || c == '\0';
};


int is_cr(CharIterWithSavePoints const *iter) {
    return std68->is.cr(iter->get(iter));
};


int is_lf(CharIterWithSavePoints const *iter) {
    return std68->is.lf(iter->get(iter));
};


int is_wsp(CharIterWithSavePoints const *iter) {
    return std68->is.wsp(iter->get(iter));
};
