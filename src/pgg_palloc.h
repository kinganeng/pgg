
#ifndef _PGG_PALLOC_H
#define _PGG_PALLOC_H

#include "pgg.h"

#define PGG_FAILED_NEXT         4

#define PGG_MAX_ALLOC_FROM_POOL (pgg_pagesize - 1)
#define PGG_DEFAULT_POOL_SIZE   (16 * 1024)
#define PGG_POOL_ALIGNMENT      16
#define PGG_MIN_POOL_SIZE                           \
        pgg_align((sizeof(pgg_pool_t) + 2 * sizeof(pgg_pool_large_t)), PGG_POOL_ALIGNMENT)


typedef struct pgg_pool_large_s pgg_pool_large_t;

struct pgg_pool_large_s {
    pgg_pool_large_t    *next;
    void                *alloc;
};

typedef struct {
    u_char      *last;
    u_char      *end;
    pgg_pool_t  *next;
    pgg_uint_t  failed;
} pgg_pool_data_t;

struct pgg_pool_s {
    pgg_pool_data_t         d;
    size_t                  max;
    pgg_pool_t              *current;
    pgg_pool_large_t        *large;
    pgg_log_t               *log;
};


pgg_pool_t* pgg_create_pool(size_t size, pgg_log_t *log);
void pgg_destroy_pool(pgg_pool_t *pool);
void pgg_reset_pool(pgg_pool_t *pool);
void* pgg_palloc(pgg_pool_t *pool, size_t size);
void* pgg_pnalloc(pgg_pool_t *pool, size_t size);
void* pgg_pcalloc(pgg_pool_t *pool, size_t size);
void* pgg_pmemalign(pgg_pool_t *pool, size_t size, size_t alignment);
pgg_int_t pgg_pfree(pgg_pool_t *pool, void *p);


#endif /** _PGG_PALLOC_H */
