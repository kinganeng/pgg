
#include "pgg.h"

ssize_t pgg_read_file(pgg_file_t *file, u_char *buf, size_t size, off_t offset) {
    ssize_t n;
    file->log->write(file->log, PGG_LOG_DEBUG, 0, "read: %d, %p, %u, %lld", file->fd, buf, size, offset);

#if PGG_HAVE_PREAD
    n = pread(file->fd, buf, size, offset);

    if ( n == -1 ) {
        file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "pread() \"%s\" failed", file->name.data);
        return PGG_ERROR;
    }
#else
    if ( file->sys_offset != offset ) {
        if ( lseek(file->fd, offset, SEEK_SET) == -1 ) {
            file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "lseek() \"%s\" failed", file->name.data);
            return PGG_ERROR;
        }

        file->sys_offset = offset;
    }

    n = read(file->fd, buf, size);
    if ( n == -1 ) {
        file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "read() \"%s\" failed", file->name.data);
        return PGG_ERROR;
    }

    file->sys_offset += n;
#endif
    file->offset += n;

    return n;
}

ssize_t pgg_write_file(pgg_file_t *file, u_char *buf, size_t size, off_t offset) {
    ssize_t n, written;

    file->log->write(file->log, PGG_LOG_DEBUG, 0, "write:%d , %p, %u, %O", file->fd, buf, size, offset);

    written = 0;

#if PGG_HAVE_PWRITE
    for( ; ; ) {
        n = pwrite(file->fd, buf + written, size, offset);
        if ( n == -1 ) {
            file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "pwrite() \"%s\" failed", file->name.data);
            return PGG_ERROR;
        }

        file->offset += n;
        written += n;

        if ( ssize_t n == size ) {
            return written;
        }

        offset += n;
        size -= n;
    }
#else
    if ( file->sys_offset != offset ) {
        if ( lseek(file->fd, offset, SEEK_SET) == -1 ) {
            file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "lseek() \"%s\" failed", file->name.data);
            return PGG_ERROR;
        }

        file->sys_offset = offset;
    }

    for( ; ; ) {
        n = write(file->fd, buf + written, size);
        if ( n == -1 ) {
            file->log->write(file->log, PGG_LOG_ERROR, pgg_errno, "write() \"%s\" failed", file->name.data);
            return PGG_ERROR;
        }

        file->offset += n;
        written += n;
        if ( (size_t)n == size ) {
            return written;
        }
        size -= n;
    }
#endif
}
