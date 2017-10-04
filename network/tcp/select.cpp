//
//  select.cpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 20/7/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//
//  Reference: Computer Systems, A Programmers Perpective, Third Edition, Chapter 12, Concurrent Network Programming
//

#include "select.hpp"

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
#define MAXLINE 1024

#endif

#define ADDR_IN(client_addr) ((struct sockaddr_in *)client_addr)
#define Malloc(t, l) (t*)malloc(sizeof(t))

void init_fdpool(fd_set * fd_pool, int* client_fdptr, int serverfd) {
    int i;
    for (i=0; i<FD_SETSIZE; i++) {
        client_fdptr[i] = -1;
    }
    
    FD_ZERO(fd_pool);
    FD_SET(serverfd, fd_pool);
}

void
init_conn(int* client_fdptr, rio_t* client_bufptr, int connfd, char* ips[], char* ip, int* ready_ctn, int* max_index)
{
    int i;
    *ready_ctn--;
    for (i=0; i<FD_SETSIZE;i++) {
        if (client_fdptr[i] == SLOT_EMPTY) {
            client_fdptr[i] = connfd;
            ips[i] = Malloc(char, strlen(ip)+1);
            strcpy(ips[i], ip);
            rio_readinitb(&client_bufptr[i], connfd);
            if (*max_index < i) {
                *max_index = i;
            }
            break;
        }
    }
    client_bufptr[i].buf[0] = '\0';
}

int select_server(int argc, char **argv)
{
    fd_set fd_pool, ready_set;
    int listenfd, // server file descriptor
        connfd, // temp conn file descriptor
        fdmax, // max file descriptor index
        clientfd[FD_SETSIZE]; // clients file descriptor array
    rio_t client_buf[FD_SETSIZE], // robust io array
          rio; //  temp rubust io variable
    // client_buf = Malloc(rio_t, FD_SETSIZE);
    socklen_t addrlen = ADDR_STORAGE_LEN;
    struct sockaddr_storage client_addr;
    int connect_cnt=0; // traceing back how many connections we have for the moment
    char buf[N], // buffer
         *ip, // clients ip array
         *ips[N];
    const char* host="12.0.0.1", *port="8081";
    int retFlag, ready_ctn, max_index=-1,
        readb;
    int i;

    
    if (argc != 2) {
        fprintf(stdout, "Usage: %s {host}=%s:{port}=%s\n", __SHORT_FILE(argv[0]), host, port);
        fprintf(stdout, "Default config is used: %s %s:%s\n", __SHORT_FILE(argv[0]), host, port);
    } else {
        host = argv[1];
        port = argv[2];
    }
    
    listenfd = open_listenerfd(port);
    init_fdpool(&fd_pool, clientfd, listenfd);
    fdmax = listenfd;
    
    fprintf(stdout, "server> ");
    fflush(stdout);
    while (1) {
        ready_set = fd_pool; // copy from master fd set
        if ((ready_ctn = select(fdmax+1, &ready_set, nullptr, nullptr, nullptr)) < 0) {
            const char* reason = "no fd ready for read.";
            fprintf(stderr, "[multiplexing failed]: %s. %s, %d\n", reason, __SHORT_FILE(argv[0]), __LINE__);
            exit(1);
        }
        
        if (FD_ISSET(listenfd, &ready_set)) {
            ready_ctn--;
            addrlen = ADDR_STORAGE_LEN;
            if((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addrlen)) < 0) {
                fprintf(stderr, "[accept error]: [%s] line %d\n", __SHORT_FILE(__FILE__), __LINE__);
                exit(1);
            } else {
                fprintf(stdout, "connection from %s on socket %d is ok!: [%s] line %d\n", inet_ntoa(ADDR_IN(&client_addr)->sin_addr), connfd, __SHORT_FILE(__FILE__), __LINE__);
                ip = inet_ntoa(ADDR_IN(&client_addr)->sin_addr);
            }
            
            // add clients
            FD_SET(connfd, &fd_pool);
            if (connfd > fdmax) {
                fdmax = connfd;
            }
            connect_cnt++;
            fprintf(stdout, "server> Total %d connections connected!\n", connect_cnt);
            rio_t rio;
            rio_readinitb(&rio, connfd);
            Welcome_Request(&rio, ip);
            
            init_conn(clientfd, client_buf, connfd, ips, ip, &ready_ctn, &max_index);
            
        }
        

        // process ready fd subset
        for (i=0; i<=max_index && ready_ctn > 0; i++) {
            connfd = clientfd[i];
            rio = client_buf[i];
            ip = ips[i];
            rio_t target;
            rio_readinitb(&target, rio.fd);
            
            if (connfd > 0 && (FD_ISSET(connfd, &ready_set))) {
                ready_ctn--;
                if ( (readb = rio_readlineb(&rio, buf, MAXLINE)) != 0 ) {
                    Process_Request(&target, buf, ip);
                } else {
                    connect_cnt--;
                    close(connfd);
                    FD_CLR(connfd, &fd_pool);
                    clientfd[i] = SLOT_EMPTY;
                    free(ips[i]);
                    ip = ips[i] = nullptr;
                }
                
            }
            
        } // endfor
        
    }// endwhile
    
    return 1;
    
}

