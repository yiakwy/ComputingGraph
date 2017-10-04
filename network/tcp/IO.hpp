//
//  IO.hpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 17/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#ifndef IO_hpp
#define IO_hpp

#include <cstdio>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#define RIO_BUFSIZE 512// 8192

typedef struct _robust_io {
    int fd;
    size_t cnt=0;
    char* bufptr;
    char buf[RIO_BUFSIZE];
} rio_t;

// buffered io
void rio_readinitb(rio_t *rio, int fd);
static ssize_t rio_read(rio_t *rio, char *buf, size_t max_len);
static ssize_t rio_write(rio_t *rio, char *buf, size_t max_len);
// line buffered io
ssize_t rio_readlineb(rio_t *rio, void *buf, size_t max_len);
// n bytes io, this function invented by Lei for easy wrting with cache, ALL RIGHTS RESERVED
ssize_t rio_write_n(rio_t *rio, char* buf, size_t size);

#endif /* IO_hpp */
