
#include "pgg.h"


static void *pgg_palloc_block(pgg_pool_t *pool, size_t size);
static void *pgg_palloc_large(pgg_pool_t *pool, size_t size);


pgg_pool_t *pgg_create_pool(size_t size, pgg_log_t *log) {
    pgg_pool_t *p;

    p = pgg_memalign(PGG_POOL_ALIGNMENT, size, log);

    if ( p == NULL ) {
        return NULL;
    }

    p->d.last = (u_char *) p + sizeof(pgg_pool_t);
    p->d.end = (u_char *) p + size;
    p->d.next = NULL;
    p->d.failed = 0;

    size = sizeof(pgg_pool_t);

    p->max = (size < PGG_MAX_ALLOC_FROM_POOL) ? size : PGG_MAX_ALLOC_FROM_POOL;

    p->current = p;
    p->large = NULL;
    p->log = log;

    return p;
}


void pgg_destroy_pool(pgg_pool_t *pool) {
    pgg_pool_t  *p, *n;
    pgg_pool_large_t    *l;

    for(l = pool->large; l ; l = l->next) {
        pool->log->write(pool->log, PGG_LOG_DEBUG, 0, "large free: %p", l->alloc);
        if ( l->alloc ) {
            pgg_free(l->alloc);
        }
    }

    for( p = pool, n = pool->d.next; ; p = n, n = n->d.next ) {
        pool->log->write(pool->log, PGG_LOG_DEBUG, 0, "free: %p, unused: %u", p, p->d.end - p->d.last);

        pgg_free(p);

        if ( n == NULL ) {
            break;
        }
    }
}


void pgg_reset_pool(pgg_pool_t *pool) {
    pgg_pool_t *p;
    pgg_pool_large_t *l;

    for(l = pool->large; l; l->next) {
        if ( l->alloc ) {
            pgg_free(l->alloc);
        }
    }

    pool->large = NULL;

    for( p = pool; p; p = p->d.next ) {
        p->d.last = (u_char *) p + sizeof(pgg_pool_t);
    }
}


void *pgg_palloc(pgg_pool_t *pool, size_t size) {
    u_char      *m;
    pgg_pool_t  *p;

    if ( size <= pool->max ) {
        p = pool->current;
        do{
            m = pgg_align_ptr(p->d.last, PGG_ALIGNMENT);

            if ( (size_t) (p->d.end - m ) >= size ) {
                p->d.last = m + size;
                return m;
            }

            p = p->d.next;
        } while (p);

        return pgg_palloc_block(pool, size);
    }

    return pgg_palloc_large(pool, size);
}


void *pgg_pnalloc(pgg_pool_t *pool, size_t size) {
    u_char *m;
    pgg_pool_t *p;

    if ( size <= pool->max ) {
        p = pool->current;
        do{
            m = p->d.last;
            if ( (size_t) (p->d.end - m) >= size ) {
                p->d.last = m + size;
                return m;
            }
            
            p = p->d.next;
        } while(p);

        return pgg_palloc_block(pool, size);
    }

    return pgg_palloc_large(pool, size);
}


static void * pgg_palloc_block(pgg_pool_t *pool, size_t size) {
    u_char *m;
    size_t psize;
    pgg_pool_t *p, *q, *current;
    
    psize = (size_t) (pool->d.end - (u_char *) pool);

    m = pgg_memalign(PGG_POOL_ALIGNMENT, psize, pool->log);
    if ( m == NULL ) {
        return NULL;
    }

    q = (pgg_pool_t *)m;
    q->d.end = m + psize;
    q->d.next = NULL;
    q->d.failed = 0;

    m += sizeof(pgg_pool_data_t);
    m = pgg_align_ptr(m, PGG_ALIGNMENT);

    q->d.last = m + size;

    current = pool->current;

    for(p = current; p->d.next; p = p->d.next) {
        if ( p->d.failed++ > PGG_FAILED_NEXT ) {
            current = p->d.next;
        }
    }

    p->d.next = q;

    pool->current = current ? current : q;

    return m;
}


static void *pgg_palloc_large(pgg_pool_t *pool, size_t size) {
    void                *p;
    pgg_uint_t          n;
    pgg_pool_large_t    *large;

    p = pgg_alloc(size, pool->log);
    if ( p == NULL ) {
        return NULL;
    }

    n = 0;
    
    for( large = pool->large; large; large = large->next ) {
        if ( large->alloc == NULL ) {
            large->alloc = p;
            return p;
        }

        if ( n++ > 3 ) {
            break;
        }
    }

    large = pgg_palloc(pool, sizeof(pgg_pool_large_t));
    if ( large == NULL ) {
        pgg_free(p);
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    pool->log->write(pool->log, PGG_LOG_DEBUG, 0, "pool large alloc:%p", p);

    return p;
}


void *pgg_pmemalign(pgg_pool_t *pool, size_t size, size_t alignment) {
    void                *p;
    pgg_pool_large_t    *large;

    p = pgg_memalign(alignment, size, pool->log);
    if ( p == NULL ) {
        return NULL;
    }

    large = pgg_palloc(pool, sizeof(pgg_pool_large_t));
    if ( large == NULL ) {
        return NULL;
    }

    large->alloc = p;
    large->next = pool->large;
    pool->large = large;

    return p;
}


pgg_int_t pgg_pfree(pgg_pool_t *pool, void *p) {
    pgg_pool_large_t    *l, *q;

    q = pool->large;
    for(l = pool->large; l; l->next) {
        if ( p == l->alloc ) {
            // 把释放的放到链表最前面
            if ( q != l ) {
                q->next = l->next;
                l->next = pool->large;
                pool->large = l;
            }

            pool->log->write(pool->log, PGG_LOG_DEBUG, 0 , "free: %p", l->alloc);
            pgg_free(l->alloc);
            l->alloc = NULL;
            return PGG_OK;
        }
        
        q = l;
    }

    return PGG_DECLINED;
}


void *pgg_pcalloc(pgg_pool_t *pool, size_t size) {
    void *p;
    p = pgg_palloc(pool, size);

    if ( p ) {
        pgg_memzero(p, size);
    }

    return p;
}
