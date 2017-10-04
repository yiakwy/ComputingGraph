//
//  utils.cpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 19/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#include "utils.hpp"

static char* host_ip[N] = {0};

int open_clientfd(char* host, char* port)
{
    struct addrinfo *p, *servinfo, hints;
    int clientfd, retFlag;
    
    memset(&hints, 0, ADDR_INFO_LEN);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV;
    hints.ai_flags |= AI_ADDRCONFIG;
    
    if ( (retFlag = getaddrinfo(host, nullptr, &hints, &servinfo)) != 0 ) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(retFlag));
    }
    
    for (p=servinfo; p!=nullptr; p=p->ai_next) {
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != UNSUCCESS) break;
        close(clientfd);
    }
    
    freeaddrinfo(servinfo);
    if (p == nullptr) return UNSUCCESS;
    else return clientfd;
}

int open_listenerfd(const char* port)
{
    struct addrinfo *p, *servinfo, hints;
    int listenfd, optval=1, retFlag;
    char buf[N], ip[N];
    
    memset(&hints, 0, ADDR_INFO_LEN);
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    hints.ai_flags |= AI_NUMERICSERV;
    
    if ( (retFlag = getaddrinfo(nullptr, port, &hints, &servinfo)) != 0 ) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(retFlag));
    }
    
    
    for(p=servinfo; p!=nullptr; p=p->ai_next)
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, ip, N, nullptr, 0, NI_NUMERICHOST);
        sprintf(buf, "<%s>\n", ip);
        std::cout << "IP:" << " " << buf;
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, INT32_LEN);
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
        close(listenfd);
    }
    
    freeaddrinfo(servinfo);
    if (p == nullptr) return UNSUCCESS;
    else
        if (listen(listenfd, LISTENQ) < 0 ){
            close(listenfd);
            return UNSUCCESS;
        } else {
            memcpy(host_ip, ip, strlen(ip));
            return listenfd;
        }
}

void Welcome_Request(rio_t * rio, char* ip)
{
    char out[N];
    sprintf(out, "server@%s> ", host_ip);
    rio_write_n(rio, out, strlen(out));
    
}

void Close_Conn(rio_t * rio, char* ip)
{
    char out[N];
    sprintf(out, "server@%s> %s", host_ip, "Good Bye!");
    rio_write_n(rio, out, strlen(out));
    
}

// command runtime state machine
void Process_Request(rio_t * rio, const char* buf, char* ip)
{
    char out[N], *command= Malloc(char, strlen(buf) - 2 + 1); // remove "/r/n"
    if (command == nullptr) {
        char* err = "Commands processing failed.";
        sprintf(out, "server@%s> %s\r\n", host_ip, err);
        rio_write_n(rio, out, strlen(out));
        fprintf(stderr, "%s", out);
        return;
    }
    strcpy(command, buf);
    command[strlen(buf)-2] = '\0';
    
    
    sprintf(out, "server@%s> calling \"%s\" ... \r\n", host_ip, command);
    rio_write_n(rio, out, strlen(out));
    fprintf(stdout, "server@%s> calling %s from %s\r\n", host_ip, command, ip);
    
    char *ret = exec(command);
    
    sprintf(out, "server@%s> \n <processed ret: %s of \"%s\">\r\n", host_ip, ret, command);
    rio_write_n(rio, out, strlen(out));
    sprintf(out, "{process ret of <%s>}", ip);
    fprintf(stdout, "server@%s> \n %s\r\n", host_ip, out);
    
    sprintf(out, "server@%s> ", host_ip);
    rio_write_n(rio, out, strlen(out));
}

char* exec(const char* command)
{
    return "~";
}


