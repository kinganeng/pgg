
#ifndef _PGG_STRING_H
#define _PGG_STRING_H

typedef struct {
    size_t      len;
    u_char      *data;
} pgg_str_t;

typedef struct {
    pgg_str_t   key;
    pgg_str_t   value;
} pgg_keyval_t;

#define pgg_string(str)             { sizeof(str) -1, (u_char *) str }
#define pgg_null_string             { 0, NULL }
#define pgg_str_set(str, text)                                      \
        (str)->len = sizeof(text) -1;                                   \
        (str)->data = (u_char *) text
#define pgg_tolower(c)              (u_char) (( c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define pgg_toupper(c)              (u_char) (( c >= 'a' && c <= 'z') ? ( c & ~0x20) : c)
#define pgg_memzero(buff, n)        (void) memset(buff, 0, n);
#define pgg_memset(buff, c, n)      (void) memset(buff, c, n);
#define pgg_memmove(dst, src, n)    (void) memmove(dst, src, n)
#define pgg_movemem(dst, src, n)    (((u_char *) memmove( dst, src, n)) + (n))
#define pgg_memcmp(s1, s2, n)       memcmp((const char *) s1, (const char *) s2, n)
#define pgg_memcpy(dst, src, n)     (void) memcpy(dst, src, n)
#define pgg_cpymem(dst, src, n)     (((u_char *) memcpy(dst, src, n)) + (n))
#define pgg_strcmp(s1, s2)          strcmp((const char *) s1, (const char *) s2)
#define pgg_value_helper(n)         #n
#define pgg_value(n)                pgg_value_helper(n)


u_char *pgg_cpystrn(u_char *dst, u_char *src, size_t n);
u_char* pgg_pstrdup(pgg_pool_t *pool, pgg_str_t *src);
pgg_int_t pgg_atoi(u_char *line, size_t n);
pgg_int_t pgg_atofp(u_char *line, size_t n, size_t point);
ssize_t pgg_atosz(u_char *line, size_t n);
off_t pgg_atoof(u_char *line, size_t n);
time_t pgg_atotm(u_char *line, size_t n);
pgg_int_t pgg_hextoi(u_char *line, size_t n);

#endif /** _PGG_STRING_H */
