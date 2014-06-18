
#ifndef _PGG_ALLOC_H
#define _PGG_ALLOC_H

void *pgg_alloc(size_t size, pgg_log_t *log);
void *pgg_calloc(size_t size, pgg_log_t *log);

#define pgg_free    free

#if (PGG_HAVE_POSIX_MEMALIGN || PGG_HAVE_MEMALIGN ) 
void *pgg_memalign(size_t alignment, size_t size, size_t pgg_log_t *log);
#else
#define pgg_memalign(alignment, size, log)  pgg_alloc(size, log)
#endif

extern pgg_uint_t   pgg_pagesize;
extern pgg_uint_t   pgg_pagesize_shift;
extern pgg_uint_t   pgg_cacheline_size;

#endif /* _PGG_ALLOC_H */

