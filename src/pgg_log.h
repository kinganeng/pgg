#ifndef _PGG_LOG_H
#define _PGG_LOG_H

#include "pgg.h"

/**
 * log level
 */
#define PGG_LOG_DEBUG       1 << 0
#define PGG_LOG_ERROR       1 << 1
#define PGG_LOG_WARN        1 << 2
#define PGG_LOG_NOTICE      1 << 3
#define PGG_LOG_ALERT       1 << 4
#define PGG_LOG_INFO        1 << 5
#define PGG_LOG_CRIT        1 << 6
#define PGG_LOG_PHP         1 << 7

#define pgg_write_console   write 


struct pgg_log_s {
    pgg_uint_t          log_level;
    pgg_open_file_t     *file;

    void                (*write)(pgg_log_t *log, int level, int error, u_char *msg, ...);
};


pgg_log_t* pgg_log_create(pgg_master_t *master, pgg_str_t *name);
void pgg_log_init(pgg_master_t *master);
void pgg_log_write(pgg_log_t *log, int level, int error, u_char *msg, ...);
void pgg_log_stderr(pgg_err_t err, const char *fmt, ...);

#endif
