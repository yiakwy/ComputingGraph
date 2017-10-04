//
//  utils.hpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 19/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#ifndef utils_hpp
#define utils_hpp

#include <iostream>
#include <cstdio>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdlib.h>

#include "IO.hpp"

#ifndef __PROLOGUE_SERVER__
#define __PROLOGUE_SERVER__

#define __SHORT_FILE(NAME) (strrchr(NAME, '/')+1)
#define ADDR_INFO_LEN sizeof(struct addrinfo)
#define ADDR_STORAGE_LEN sizeof(struct sockaddr_storage)
#define INT32_LEN sizeof(int)

#define UNSUCCESS -1
#define SLOT_EMPTY -1
#define LISTENQ 1024 // allow {LISTENQ} requests to queue up
#define SERV_PORT 80
#define N 1024
#define MAXLINE 1024

#endif

#define Malloc(t, l) (t*)malloc(sizeof(t)*l)

int open_listenerfd(const char* port);
int open_clientfd(char* hostname, char* port);

void Welcome_Request(rio_t * rio, char* ip);
void Process_Request(rio_t * rio, const char* buf, char* ip);
void Close_Conn(rio_t * rio, char* ip);
char*
exec(const char* command);
#endif /* utils_hpp */
