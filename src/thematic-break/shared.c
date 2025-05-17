#include "./shared.h"


int is_valid_th_break_from_iter(CharIterWithSavePoints *iter) {
    size_t starting_blank_count = 0;
    size_t no_wsp_count = 0;
    char no_wsp_char = '-';

    iter->set_savepoint(iter);

    starting_blank_count = iter_ops->advance.while_wsp(iter);
    if (starting_blank_count > 3) {
        goto IsValidThBreak_NotValid;
    }

    while (!iter_ops->is.eol(iter)) {
        char c = iter->get(iter);
        if (c != '-' && c != '*' && c != '_') {
            goto IsValidThBreak_NotValid;
        }

        if (no_wsp_count == 0) {
            no_wsp_char = c;
        } else if (c != no_wsp_char) {
            goto IsValidThBreak_NotValid;
        }

        no_wsp_count++;
        iter->next(iter);
        iter_ops->advance.while_wsp(iter);
    }

    if (no_wsp_count < 3) {
        goto IsValidThBreak_NotValid;
    }

    iter->restore(iter);
    return 1;
IsValidThBreak_NotValid:
    iter->restore(iter);
    return 0;
}
