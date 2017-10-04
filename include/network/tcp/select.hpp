//
//  select.hpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 23/7/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//
//  The module performs Non-Blocking IO in a single thread, with readiness notification methods.
//

#ifndef select_hpp
#define select_hpp

#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "IO.hpp"
#include "utils.hpp"

#define N 1024
static char* host_ip[N] = {0};

void init_fdpool(fd_set * fd_pool);
void init_conn(int* client_fdptr, rio_t* client_bufptr, int connfd, char* ips[], char* ip, int* ready_ctn, int* max_index);
// entry of the module
int select_server(int argc, char **argv);
#endif /* select_hpp */
