
#include "pgg.h"

u_char *pgg_log_errno(u_char *buf, u_char *last, pgg_err_t err);

static pgg_str_t debug_levels[] = {
    pgg_string("debug"),
    pgg_string("error"),
    pgg_string("warn"),
    pgg_string("notice"),
    pgg_string("alert"),
    pgg_string("info"),
    pgg_string("crit"),
    pgg_string("php")
};


void pgg_log_init(pgg_master_t *master){
}

void pgg_log_write(pgg_log_t *log, int level, pgg_err_t err, u_char *fmt, ...) {
    u_char  *p, *last;
    ssize_t len;
    va_list args;
    u_char  errstr[PGG_MAX_ERROR_STR];
    time_t localTime; 
    struct tm *newtime;

    if ( (level & log->log_level) != level ) {
        return ;
    }

    time( &localTime ); 
    newtime=localtime(&localTime); 
    len = strftime(errstr, 64, "[%D %T %z]", newtime);        

    p = errstr + len;
    last = errstr + PGG_MAX_ERROR_STR;

    va_start(args, fmt);
    len = vsprintf(p, fmt, args);
    va_end(args);

    p += len;

    if ( err ) {
        p = pgg_log_errno(p, last, err);
    }

    if ( log->file && log->file->fd ) {
        write(log->file->fd, errstr, p - errstr);
    } else {
        pgg_log_stderr(0, errstr);
    }
}


void pgg_log_stderr(pgg_err_t err, const char *fmt, ...) {
    u_char   *p, last;
    va_list   args;
    u_char    errstr[PGG_MAX_ERROR_STR];
    ssize_t   len = 0;

    p = errstr + 9;
    last = errstr + PGG_MAX_ERROR_STR;

    memcpy(errstr, "php-pgg: ", 9);

    va_start(args, fmt);
    len += vsprintf(p, fmt, args);
    va_end(args);

    p += len;

    if ( err ) {
        p = pgg_log_errno(p, last, err);
    }

    pgg_write_console(pgg_stderr, errstr, p - errstr);
}

u_char *pgg_log_errno(u_char *buf, u_char *last, pgg_err_t err) {
    u_char  *p, *errstr;
    ssize_t   len;
    
    errstr = strerror(err);
    len = strlen(errstr);

    if ( buf + len + 10 > last ) {
        buf = last - 13 - len;
        *buf++ = '.';
        *buf++ = '.';
        *buf++ = '.';
    }

    len = sprintf(buf, " (%d: %s)", err, errstr);

    return buf + len;
}
