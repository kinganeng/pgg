
#ifndef _PGG_H
#define _PGG_H

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <pwd.h>
#include <grp.h>
#include <sys/wait.h>


#define PGG_OK                      0
#define PGG_ERROR                  -1
#define PGG_AGAIN                  -2
#define PGG_BUSY                   -3
#define PGG_DONE                   -4
#define PGG_DECLINED               -5
#define PGG_ABORT                  -6

#define PGG_ESRCH                   ESRCH
#define PGG_EAGAIN                  EAGAIN
#define PGG_EINTR                   EINTR
#define PGG_ECHILD                  ECHILD

#define LF                          (u_char) 10
#define CR                          (u_char) 13
#define CRLF                        "\0xd\0xa"

#define PGG_MAX_ERROR_STR           10240
#define PGG_EXIT_OK                 0
#define PGG_EXIT_USAGE              64
#define PGG_EXIT_SOFTWARE           70

#ifndef PGG_ALIGNMENT
#define PGG_ALIGNMENT   sizeof(unsigned long)
#endif


#define pgg_errno                   errno
#define pgg_socket_errno            errno
#define pgg_stderr                  STDERR_FILENO
#define pgg_set_errno(err)          errno = err
#define pgg_set_socket_errno(err)   errno = err


#define pgg_abs(value)              (((value) >= 0) ? (value) : - (value))
#define pgg_max(a, b)               ((a < b) ? (b) : (a))
#define pgg_min(a, b)               ((a > b) ? (b) : (a))
#define pgg_align(d, a)             (((d) + (a - 1)) & ~(a - 1))
#define pgg_align_ptr(p, a)                                                         \
        (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1 )) & ~((uintptr_t) a - 1 ))


typedef intptr_t        pgg_int_t;
typedef uintptr_t       pgg_uint_t;
typedef intptr_t        pgg_flag_t;
typedef int             pgg_err_t;
typedef pgg_uint_t      pgg_msec_t;
typedef pgg_int_t       pgg_msec_int_t;
typedef uid_t           pgg_uid_t;
typedef gid_t           pgg_gid_t;

typedef struct pgg_master_s         pgg_master_t;
typedef struct pgg_pool_s           pgg_pool_t;
typedef struct pgg_log_s            pgg_log_t;
typedef struct pgg_open_file_s      pgg_open_file_t;
typedef struct pgg_file_s           pgg_file_t;
typedef struct pgg_array_s          pgg_array_t;


/**
 * pgg head file
 */
#include "atomic.h"
#include "pgg_string.h"
#include "pgg_file.h"
#include "pgg_log.h"
#include "pgg_alloc.h"
#include "pgg_palloc.h"


#endif /** _PGG_H */
