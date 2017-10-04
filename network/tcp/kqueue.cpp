//
//  kqueue.cpp
//  SimpleHTTPServer
//
//  Created by Wang Yi on 18/9/17.
//  Copyright © 2017 Wang Lei. All rights reserved.
//

#include "kqueue.hpp"

#define __SHORT_FILE(NAME) (strrchr(NAME, '/')+1)
#define ADDR_INFO_LEN sizeof(struct addrinfo)
#define ADDR_STORAGE_LEN sizeof(struct sockaddr_storage)
#define INT32_LEN sizeof(int)

#define UNSUCCESS -1
#define SLOT_EMPTY -1
#define NOT_FOUND -1
#define LISTENQ 1024 // allow {LISTENQ} requests to queue up
#define SERV_PORT 80
#define N 1024
#define MAXLINE 1024

static char host_ip[N] = {0};

#define ADDR_IN(client_addr) ((struct sockaddr_in *)client_addr)
#define Malloc(t, l) (t*)malloc(sizeof(t))

void init_epool(struct kevent* evpool, ClientsTab& clientfd, int* free_slots, int size, int serverfd)
{
    int i;
    for (i=0; i < size; i++) {
        EV_SET(evpool+i, serverfd, EVFILT_READ, EV_ADD | EV_ENABLE , 0, 0, nullptr);
        free_slots[i] = i;
    }
}

// Unlike what I did in "select_server", I use a free_slots to trace file descriptor because there is no need to perform linear scan; one can also use fiel descriptor from kernel as a reverse key to query data.

void init_conn(struct kevent* evpool, ClientsTab& clientfd, char* ip, int* free_slots, int* j, int size, int connfd)
{
    *j = *j+1;
    int slot = free_slots[*j % size];
    EV_SET(&evpool[slot], connfd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    Client * cli = Malloc(Client, 1);
    if (cli == nullptr) {
        perror("server> [Not Enough Space!]: Client allocating failed.");
        return;
    }
    cli->event_id = slot;
    cli->connfd = connfd;
    cli->ip = Malloc(char, strlen(ip)+1);
    if (cli == nullptr) {
        perror("server> [Not Enough Space!]: IP addr allocating failed.");
        return;
    }
    strcpy(cli->ip, ip);
    clientfd[connfd] = cli;
}

int
getContextId(ClientsTab& clientfd, int size, int fd){
    if (clientfd.find(fd) != clientfd.end()) {
        return clientfd[fd]->event_id;
    }
    return NOT_FOUND;
}


int kqueue_server(int argc, char **argv)
{
    // network prologue
    int kq = kqueue(); // kernel_queue
    struct kevent events_pool[N], targets[N];
    socklen_t addrlen = ADDR_STORAGE_LEN;
    struct sockaddr_storage client_addr;
    int listenfd, connfd,
        free_slots[N]; // empty_slot queue
    int connect_cnt=0; // traceing back how many connections we have for the moment
    ClientsTab clientfd;
    char buf[N], // buffer
         *ip; // clients ip array
    
    const char* host="12.0.0.1", *port="8081";
    int retFlag, ready_ctn, readb;
    int i,j=-1, k=-1;

    if (argc != 2) {
        fprintf(stdout, "Usage: %s {host}=%s:{port}=%s\n", __SHORT_FILE(argv[0]), host, port);
        fprintf(stdout, "Default config is used: %s %s:%s\n", __SHORT_FILE(argv[0]), host, port);
    } else {
        host = argv[1];
        port = argv[2];
    }
    
    listenfd = open_listenerfd(port);
    init_epool(events_pool, clientfd, free_slots, N, listenfd);
    
    fprintf(stdout, "server> ");
    fflush(stdout);
    while (1) {
        if ((ready_ctn = kevent(kq, events_pool, N, targets, N, nullptr)) < 0) {
            const char* reason = "no fd ready for read.";
            fprintf(stderr, "[multiplexing failed]: %s. %s, %d\n", reason, __SHORT_FILE(argv[0]), __LINE__);
            exit(1);
        }
        
        // linear scan
        for (i=0; i < ready_ctn; i++) {
            
            // server ready
            if (targets[i].ident == listenfd) {
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
                connect_cnt++;
                fprintf(stdout, "server> Total %d connections connected!\n", connect_cnt);
                rio_t target;
                rio_readinitb(&target, connfd);
                Welcome_Request(&target, ip);
                
                if ((j-k) >= N)
                {
                    fprintf(stdout, "server> No slots available\n", connect_cnt);
                }
                init_conn(events_pool, clientfd, ip, free_slots, &j, N, connfd);
                    
            } else
            if (targets[i].flags & EV_EOF) {
                int connfd = targets[i].ident;
                int slot = getContextId(clientfd, N, connfd);
                rio_t target;
                rio_readinitb(&target, connfd);
                ip = clientfd[connfd]->ip;
                
                Close_Conn(&target, ip);
                
                ready_ctn--;
                connect_cnt--;
                // clear the fd
                close(connfd);
                
                EV_SET(&events_pool[slot], connfd, EVFILT_READ, EV_DELETE, 0, 0, nullptr); // !important
                free_slots[++k%N] = slot;
                free(clientfd[connfd]->ip);
                free(clientfd[connfd]);
            } else
            if (targets[i].flags & EVFILT_READ) {
                int connfd = targets[i].ident;
                
                ready_ctn--;
                ip = clientfd[connfd]->ip;
                rio_t from, target;
                rio_readinitb(&from, connfd);
                rio_readinitb(&target, connfd);
                
                if ((readb = rio_readlineb(&from, buf, MAXLINE)) != 0) {
                    Process_Request(&target, buf, ip);
                } else {
                    int slot = getContextId(clientfd, N, connfd);
                    if (slot < 0) continue;
                    connect_cnt--;
                    // clear the fd
                    close(connfd);
                    
                    EV_SET(&events_pool[slot], connfd, EVFILT_READ, EV_DELETE, 0, 0, nullptr); // !important
                    free_slots[++k%N] = slot;
                    free(clientfd[connfd]->ip);
                    free(clientfd[connfd]);
                }
                
            }
            
        } // endfor
        
    } // endwhile
    
    return 1;
}
