
#include "pgg.h"

u_char* pgg_cpystrn(u_char *dst, u_char *src, size_t n) {
    if ( n == 0 ) {
        return dst;
    }

    while(--n) {
        *dst = *src;
        if ( *dst == '\0' ) {
            return dst;
        }

        dst++;
        src++;
    }

    dst = '\0';
    return dst;
}

u_char* pgg_pstrdup(pgg_pool_t *pool, pgg_str_t *src) {
    u_char *data;
    data = pgg_palloc(pool, sizeof(u_char *) * src->len);
    data = strncpy(data, src->data, src->len);
    return data;
}


pgg_int_t pgg_atoi(u_char *line, size_t n) {
    pgg_int_t  value;

    if (n == 0) {
        return PGG_ERROR;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return PGG_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}


/* parse a fixed point number, e.g., pgg_atofp("10.5", 4, 2) returns 1050 */

pgg_int_t pgg_atofp(u_char *line, size_t n, size_t point) {
    pgg_int_t   value;
    pgg_uint_t  dot;

    if (n == 0) {
        return PGG_ERROR;
    }

    dot = 0;

    for (value = 0; n--; line++) {

        if (point == 0) {
            return PGG_ERROR;
        }

        if (*line == '.') {
            if (dot) {
                return PGG_ERROR;
            }

            dot = 1;
            continue;
        }

        if (*line < '0' || *line > '9') {
            return PGG_ERROR;
        }

        value = value * 10 + (*line - '0');
        point -= dot;
    }

    while (point--) {
        value = value * 10;
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}


ssize_t pgg_atosz(u_char *line, size_t n) {
    ssize_t  value;

    if (n == 0) {
        return PGG_ERROR;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return PGG_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}


off_t pgg_atoof(u_char *line, size_t n) {
    off_t  value;

    if (n == 0) {
        return PGG_ERROR;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return PGG_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}


time_t pgg_atotm(u_char *line, size_t n) {
    time_t  value;

    if (n == 0) {
        return PGG_ERROR;
    }

    for (value = 0; n--; line++) {
        if (*line < '0' || *line > '9') {
            return PGG_ERROR;
        }

        value = value * 10 + (*line - '0');
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}


pgg_int_t pgg_hextoi(u_char *line, size_t n) {
    u_char     c, ch;
    pgg_int_t  value;

    if (n == 0) {
        return PGG_ERROR;
    }

    for (value = 0; n--; line++) {
        ch = *line;

        if (ch >= '0' && ch <= '9') {
            value = value * 16 + (ch - '0');
            continue;
        }

        c = (u_char) (ch | 0x20);

        if (c >= 'a' && c <= 'f') {
            value = value * 16 + (c - 'a' + 10);
            continue;
        }

        return PGG_ERROR;
    }

    if (value < 0) {
        return PGG_ERROR;

    } else {
        return value;
    }
}
