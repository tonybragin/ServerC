//
//  server.c
//  EL
//
//  Created by TONY on 01/03/2019.
//  Copyright © 2019 TONY. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <poll.h>
#include <limits.h>
#include <fcntl.h>
//#include <sys/epoll.h> //can't find file

#define SS struct sockaddr
#define MAXLINE 4096
#define    max(a,b)    ((a) > (b) ? (a) : (b))

struct info_for_thread {
    int connfd;
    int thread_id;
};

void start_tcp(int* listenfd, struct sockaddr_in* servaddr) {

    if ((*listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
    }

    bzero(&servaddr, sizeof(servaddr));
    printf("\n1\n");
    (*servaddr).sin_family = AF_INET; // sig fault
    printf("\n2\n");
    servaddr->sin_port = htons(88);
    
    if (inet_aton("0.0.0.0", &servaddr->sin_addr) == 0) {
        perror("inet_aton error");
    }

    if (bind(*listenfd, (SS*) &servaddr, sizeof(servaddr)) < 0) {
        perror("Bind error");
    }

    if (listen(*listenfd, 1024) < 0) {
        perror("listen error");
    }
}

void start_udp(int* listenfd, struct sockaddr_in* servaddr) {
    if ((*listenfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket error");
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr->sin_family = AF_INET; // sig fault
    servaddr->sin_port = htons(88);
    if (inet_aton("0.0.0.0", &servaddr->sin_addr) == 0) {
        perror("inet_aton error");
    }
    
    if (bind(*listenfd, (SS*) &servaddr, sizeof(servaddr)) < 0) {
        perror("Bind error");
    }
    
}

void serv_staff_tcp(int sockfd) {
    char buf[MAXLINE];
    ssize_t  n;
    
    while ((n = read(sockfd, buf, MAXLINE)) > 0) {
        write(sockfd, buf, n);
    }
    close(sockfd);
}

void process_thread() {
    struct info_for_thread info;
    
    pthread_t childpid[3];
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
    
    int filelist = open("list", O_RDONLY);
    
    while (1) {
        if (read(filelist, &info, sizeof(struct info_for_thread)) > 0) {
            pthread_create(childpid[info.thread_id], &tattr, serv_staff_tcp, info.connfd);
        }
    }
}

int main(int argc, const char * argv[]) {
    
    int listenfd, connfd, udpfd;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
    }
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET; // sig fault
    servaddr.sin_port = htons(88);
    
    if (inet_aton("0.0.0.0", &servaddr.sin_addr) == 0) {
        perror("inet_aton error");
    }
    
    if (bind(listenfd, (SS*) &servaddr, sizeof(servaddr)) < 0) {
        perror("Bind error");
    }
    
    if (listen(listenfd, 1024) < 0) {
        perror("listen error");
    }
    
    int type = atoi(argv[1]);
    switch (type) {
        case 0: { // tcp
            
            while (1) {
                clilen = sizeof(cliaddr);
                if ((connfd = accept(listenfd, (SS *) &cliaddr, &clilen)) < 0) {
                    perror("accept error");
                }
                serv_staff_tcp(connfd);
            }
            break;
        }
            
        case 1: { //udp
            char buf[MAXLINE];
            
            if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket error");
            }
            
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET; // sig fault
            servaddr.sin_port = htons(88);
            if (inet_aton("0.0.0.0", &servaddr.sin_addr) == 0) {
                perror("inet_aton error");
            }
            
            if (bind(udpfd, (SS*) &servaddr, sizeof(servaddr)) < 0) {
                perror("Bind error");
            }
            
            while (1) {
                ssize_t n;
                clilen = sizeof(cliaddr);
                if ((n = recvfrom(udpfd, buf, MAXLINE, 0, (SS*) &cliaddr, &clilen)) < 0) {
                    perror("recvfrom error");
                }
                sendto(udpfd, buf, n, 0, (SS*) &cliaddr, clilen);
            }
            break;
        }
            
        case 2: { // tcp fork
            pid_t childpid;
            
            while (1) {
                clilen = sizeof(cliaddr);
                if ((connfd = accept(listenfd, (SS *) &cliaddr, &clilen)) < 0) {
                    perror("accept error");
                }
                
                if ((childpid = fork()) == 0) {
                    close(listenfd);
                    serv_staff_tcp(connfd);
                    exit(0);
                }
                close(connfd);
            }
            break;
        }
            
        case 3: { // tcp thread fixed
            pthread_t childpid[3];
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            
            int what_pid = 0;
            
            while (1) {
                clilen = sizeof(cliaddr);
                if ((connfd = accept(listenfd, (SS *) &cliaddr, &clilen)) < 0) {
                    perror("accept error");
                }
                
                if ((pthread_create(&childpid[what_pid], &tattr, serv_staff_tcp, connfd)) != 0) {
                    what_pid = (what_pid + 1) % 3;
                }
                
                what_pid = (what_pid + 1) % 3;
            }
            
            break;
        }
            
        case 4: { // tcp thread with buffer
            pthread_t processpid;
            pthread_attr_t tattr;
            pthread_attr_init(&tattr);
            pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
            
            pthread_create(processpid, &tattr, process_thread, NULL);
            
            int what_pid = 0;
            struct info_for_thread info;
            
            mode_t mode = S_IRUSR | S_IWUSR | S_IWGRP;
            mkfifo("list", mode);
            int filelist = open("list", O_WRONLY);
            
            while (1) {
                clilen = sizeof(cliaddr);
                if ((connfd = accept(listenfd, (SS *) &cliaddr, &clilen)) < 0) {
                    perror("accept error");
                }
                
                info.connfd = connfd;
                info.thread_id = what_pid;
                write(filelist, &info, sizeof(struct info_for_thread));
                
                what_pid = (what_pid + 1) % 3;
            }
            
            break;
        }
            
        case 5: { // tcp select
            fd_set rset;
            int nready;
            
            FD_ZERO(&rset);
            
            while (1) {
                FD_SET(listenfd, &rset);
                
                if ((nready = select(listenfd+1, &rset, NULL, NULL, NULL)) < 0) {
                    perror("select error");
                }
                
                if (FD_ISSET(listenfd, &rset)) {
                    socklen_t len = sizeof(cliaddr);
                    connfd = accept(listenfd, (SS*) &cliaddr, &len);
                    
                    close(listenfd);
                    serv_staff_tcp(connfd);
                    close(connfd);
                }
            }
            
            break;
        }
            
        case 6: { // tcp poll
            int sockfd;
            struct pollfd client[OPEN_MAX];
            ssize_t n;
            char buf[MAXLINE];
            
            client[0].fd = listenfd;
            client[0].events = POLLRDNORM;
            
            int i;
            for ( i = 1; i < OPEN_MAX; i++) {
                client[i].fd = -1;
            }
            int maxi = 0;
            
            while (1) {
                int nready = poll(client, maxi+1, -1);
                
                if (client[0].revents & POLLRDNORM) {
                    clilen = sizeof(cliaddr);
                    connfd = accept(listenfd, (SS*) &cliaddr, &clilen);
                    
                    for (i = 1; i < OPEN_MAX; i++)
                        if (client[i].fd < 0) {
                            client[i].fd = connfd;
                            break;
                        }
                    if (i == OPEN_MAX) {
                        perror("too many clients");
                    }
                    
                    client[i].events = POLLRDNORM;
                    if (i > maxi)
                        maxi = i;
                    
                    if (--nready <= 0)
                        continue;
                }
                
                for (i = 1; i <= maxi; i++) {    /* check all clients for data */
                    if ( (sockfd = client[i].fd) < 0)
                        continue;
                    if (client[i].revents & (POLLRDNORM | POLLERR)) {
                        if ( (n = read(sockfd, buf, MAXLINE)) < 0) {
                            perror("readline error");
                        } else if (n == 0) {
                            close(sockfd);
                            client[i].fd = -1;
                        } else {
                            write(sockfd, buf, n);
                        }
                        
                        if (--nready <= 0)
                            break;
                    }
                }
            }
            
            break;
        }
            
        case 7: { // tcp epoll can't find library
            
            /*
             int sockfd;
             int epfd;
             struct epoll_event epevent;
             epoll_data_t epdata;
            
             FD_SET(epevent.events, EPOLLIN);
             epevent.data.fd  = listenfd;
             
             if ((epfd = epoll_create(1)) < 0) {
                perror("epoll_create error");
             }
             if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &epevent) != 0) {
                perror("epoll_ctl error");
             }
             
             while (1) {
                if ((sockfd = epoll_wait(epfd, &epevent, 1, -1)) == 0) {
                    perror("epoll_wait error");
                } else {
                    serv_staff_tcp(sockfd);
                }
             }
             
            */
            
            break;
        }
            
        case 8: { // tcp and udp select
            fd_set rset;
            int nready;
            
            if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket error");
            }
            
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(88);
            if (inet_aton("0.0.0.0", &servaddr.sin_addr) == 0) {
                perror("inet_aton error");
            }
            
            if (bind(udpfd, (SS*) &servaddr, sizeof(servaddr)) < 0) {
                perror("Bind error");
            }
            
            int maxfd = max(listenfd, udpfd) + 1;
            
            while (1) {
                FD_SET(listenfd, &rset);
                FD_SET(udpfd, &rset);
                
                if ((nready = select(maxfd, &rset, NULL, NULL, NULL)) < 0) {
                    perror("select error");
                }
                
                if (FD_ISSET(listenfd, &rset)) {
                    socklen_t len = sizeof(cliaddr);
                    connfd = accept(listenfd, (SS*) &cliaddr, &len);
                    
                    close(listenfd);
                    serv_staff_tcp(connfd);
                    close(connfd);
                }
                
                if (FD_ISSET(udpfd, &rset)) {
                    char buf[MAXLINE];
                    socklen_t len = sizeof(cliaddr);
                    ssize_t n = recvfrom(udpfd, buf, MAXLINE, 0, (SS*) &cliaddr, &len);
                    
                    sendto(udpfd, buf, n, 0, (SS *) &cliaddr, len);
                }
            }
            
            break;
        }
            
            
        default:
            return 0;
    }
}
