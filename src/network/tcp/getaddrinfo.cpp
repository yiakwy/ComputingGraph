//
//  main.cpp
//  getaddrinfo
//
//  Created by Wang Yi on 14/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#include <iostream>
#include <cstdio>

#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#define __SHORT_FILE(NAME) (strrchr(NAME, '/')+1)

#define N 1024
#define ADDR_INFO_LEN sizeof(struct addrinfo)

int main(int argc, const char * argv[]) {
    struct addrinfo *p, *listp, hints;
    const char *host, *default_="twitter.com";
    char buf[N], ip[N];
    int retFlag, flags;
    
    if (argc != 2) {
        retFlag = sprintf(buf, "Usage: %s {domain_name}\n", __SHORT_FILE(argv[0]));
        std::cout << buf;
        host = default_;
        retFlag = sprintf(buf, "default {domain_name} for %s is %s\n", __SHORT_FILE(argv[0]), default_);
        std::cout << buf;
    }
    
    memset(&hints, 0, ADDR_INFO_LEN);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    if ( (retFlag = getaddrinfo(host, nullptr, &hints, &listp)) != 0 ) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(retFlag));
    }
    
    flags = NI_NUMERICHOST;
    for (p=listp; p; p=p->ai_next) {
        getnameinfo(p->ai_addr, p->ai_addrlen, ip, N, nullptr, 0, flags);
        retFlag = sprintf(buf, "%s\n", ip);
        std::cout << "IP:" << " " << buf;
    }
    
    freeaddrinfo(listp);
}
