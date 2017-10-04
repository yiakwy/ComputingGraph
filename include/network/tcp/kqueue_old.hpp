//
//  kqueue.hpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 18/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#ifndef kqueue_hpp
#define kqueue_hpp

#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <sys/event.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "IO.hpp"
#include "utils.hpp"

void init_epool(struct kevent* evpool, int* clientfd, char* ips[], char* ip, int* free_slots, int size, int serverfd);
void init_conn(struct kevent* evpool, int* clientfd, int* free_slots, int* j, int size, int connfd);
int getContextId(int* clientfd, int size, int fd);
int kqueue_server(int argc, char **argv);

#endif /* kqueue_hpp */
