
#include "pgg.h"

pgg_uint_t pgg_pagesize;
pgg_uint_t pgg_pagesize_shift;
pgg_uint_t pgg_cacheline_size;

void *pgg_alloc(size_t size, pgg_log_t *log) {
    void *p;
    p = malloc(size);
    if ( p == NULL ) {
        log->write(log, PGG_LOG_ERROR, pgg_errno, "malloc(%u) failed", size);
    }

    log->write(log, PGG_LOG_DEBUG, 0, "malloc : %p : %u", p, size);

    return p;
}

void *pgg_calloc(size_t size, pgg_log_t *log) {
    void *p;
    p = pgg_alloc(size, log);

    if ( p ) {
        pgg_memzero(p, size);
    }

    return p;
}

#if (PGG_HAVE_POSIX_MEMALIGN)
void *pgg_memalign(size_t alignment, size_t size, pgg_log_t *log) {
    void *p;
    int err;
    err = posfix_memalign(&p, alignment, size);

    if ( err ) {
        log->write(log, PGG_LOG_ERROR, err, "posfix_memalign(%u, %u) failed", alignment, size);
        p = NULL;
    }

    log->write(log, PGG_LOG_DEBUG, 0, "posfix_memalign : %p : %u @%u", p, size, alignment);
    return p;
}
#elif (PGG_HAVE_MEMALIGN)
void *pgg_memalign(size_t alignment, size_t size, pgg_log_t *log) {
    void *p;
    int err;
    err = memalign(&p, alignment, size);

    if ( err ) {
        log->write(log, PGG_LOG_ERROR, err, "memalign(%u, %u) failed", alignment, size);
        p = NULL;
    }

    log->write(log, PGG_LOG_DEBUG, 0, "memalign : %p : %u @%u", p, size, alignment);
    return p;
}
#endif
