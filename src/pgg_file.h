#ifndef _PGG_FILE_H
#define _PGG_FILE_H

#include "pgg.h"


#define pgg_open_file(name, mode, create, access)                   \
        open((const char *)name, mode | create , access)
#define pgg_close_file              close
#define pgg_delete_file(name)       unlink((const char *)name)
#define pgg_read_fd                 read
#define pgg_is_dir(sb)              (S_ISDIR((sb)->st_mode))
#define pgg_is_file(sb)             (S_ISREG((sb)->st_mode))
#define pgg_is_link(sb)             (S_ISLNK((sb)->st_mode))
#define pgg_is_exec(sb)             (((sb)->st_mode & S_IXUSR) == S_IXUSR)
#define pgg_file_access(sb)         ((sb)->st_mode & 0777)
#define pgg_file_size(sb)           (sb)->st_size
#define pgg_file_fs_size(sb)        pgg_max((sb)->st_size, (sb)->st_blocks * 512)
#define pgg_file_mtime(sb)          (sb)->st_mtime
#define pgg_file_uniq(sb)           (sb)->st_ino
#define pgg_write_console           write


#define PGG_INVALID_FILE            -1
#define PGG_FILE_ERROR              -1


typedef struct stat                 pgg_file_info_t;
typedef int                         pgg_fd_t;

struct pgg_file_s {
    pgg_fd_t            fd;
    pgg_str_t           name;
    pgg_file_info_t     info;
    
    off_t               offset;
    off_t               sys_offset;

    pgg_log_t           *log;
};

struct pgg_open_file_s {
    pgg_fd_t    fd;
    pgg_str_t   name;

    void        (*flush)(pgg_open_file_t *file, pgg_log_t *log);
    void        *data;
};


ssize_t pgg_read_file(pgg_file_t *file, u_char *buf, size_t size, off_t offset);
ssize_t pgg_write_file(pgg_file_t *file, u_char *buf, size_t size, off_t offset);


#endif /* _PGG_FILE_H */
