//
//  IO.cpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 17/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//
//  Reference: Computer Systems, A Programmers Perpective, Third Edition, Chapter 10, System IO
//

#include "IO.hpp"
#include <errno.h>
#define MIN(a, b) ((a)<(b)?(a):(b))

#define NEWLINE '\n'
#define STOP '\0'

void rio_readinitb(rio_t *rio, int fd_)
{
//    fprintf(stdout, "[INFO]: sizeof(*rio) is <%d> of <%d>\n", sizeof (*rio), sizeof(rio_t));
//    fflush(stdout);
    rio->fd = fd_;
    rio->bufptr = rio->buf;
}

// text line buffered version of robust reader to reduce the kernel data exchange
// just for inner usage
static ssize_t rio_read(rio_t *rio, char *buf, size_t max_len)
{
    size_t cnt;
    while (rio->cnt <= 0) { // refill the buf from kernel
        if ((rio->cnt = read(rio->fd, rio->buf, sizeof(rio->buf))) < 0 ) {
            if (errno = EINTR) {
                fprintf(stderr, "Interrupted by sig handler, return\n");
                return - 1;
            } // else do nothing, read again
        }
        
        if (rio->cnt == 0) {
            return 0;
        }
        
        rio->bufptr = rio->buf; // reset buffer pointer
    }
    
    cnt = MIN(rio->cnt, max_len);
    memcpy(buf, rio->bufptr, cnt);
    rio->bufptr += cnt;
    rio->cnt -= cnt;
    return cnt;
}

static ssize_t rio_write(rio_t *rio, char* buf, size_t max_len)
{
    size_t cnt, written;
    while (rio->cnt >= sizeof(rio->buf)) { // flush to kernel
        if ((written = write(rio->fd, rio->buf, sizeof(rio->buf))) < 0 ) {
            if (errno = EINTR) {
                fprintf(stderr, "Interrupted by sig handler, return\n");
                return -1;
            } else { continue; } // else do nothing, write again
        }
        
        rio->cnt = sizeof(rio->buf) - written;
        if (written == 0) {
            return 0;
        }
        
        memcpy(rio->buf, rio->buf+written, rio->cnt);
        rio->bufptr = rio->buf + rio->cnt; //  reset buffer pointer
    }
    
    cnt = MIN(sizeof(rio->buf) - rio->cnt, max_len);
    memcpy(rio->bufptr, buf, cnt);
    rio->bufptr += cnt;
    rio->cnt += cnt;
    return cnt;
}

// read a line from buffer
ssize_t rio_readlineb(rio_t *rio, void *buf, size_t max_len)
{
    int cnt=0, readb;
    char c, *buf_=(char*)buf;
    bool flag = true;
    while (cnt < max_len && flag) {
        if ( (readb=rio_read(rio, &c, 1)) < 0 ) {
            fprintf(stderr, "Error\n");
            exit(1);
        }
        
        if (readb == 0) {
            fprintf(stdout, "EOF FOUND.");
            break;
        }
        
        switch (c) {
            case NEWLINE:
                // read a line
                flag = false;
            default:
                *buf_++ = c;
        }
        
        cnt++;
        
    }
    
    *buf_ = STOP;
    return cnt;
}

ssize_t rio_write_n(rio_t *rio, char* buf, size_t size)
{
    size_t left = size;
    size_t written;
    char* buf_ = buf;
    
    while (left > 0) {
        if ((written = rio_write(rio, buf_, left)) < 0) {
            fprintf(stderr, "Error\n");
            exit(1);
        }
        
        if (written == 0) {
            fprintf(stdout, "EOF FOUND.");
        }
        
        left -= written;
        buf_ += written;
        
    }
    
    // flush
    if (rio->cnt > 0) {
        if ((written = write(rio->fd, rio->buf, rio->cnt)) < 0) {
            fprintf(stderr, "Error\n");
            exit(1);
        }
        rio->cnt = 0;
        rio->bufptr = rio->buf;
    }
    return size;
}
