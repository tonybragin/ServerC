//
//  client.c
//  EL
//
//  Created by TONY on 02/03/2019.
//  Copyright © 2019 TONY. All rights reserved.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

#define SS struct sockaddr
#define MAXLINE 4096

int main (int argc, const char * argv[]) {
    int sockfd;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE], recvline[MAXLINE];
    
    
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(88);
    inet_aton("0.0.0.0", &servaddr.sin_addr);
    
    int type = atoi(argv[1]);
    switch (type) {
        case 0: { // tcp
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            
            connect(sockfd, (SS*) &servaddr, sizeof(servaddr));
            
            while (fgets(sendline, MAXLINE, stdin) != NULL) {
                write(sockfd, sendline, strlen(sendline));
                if (read(sockfd, recvline, MAXLINE) == 0) {
                    perror("server terminated prematurely");
                    return 0;
                }
                
                fputs(recvline, stdout);
            }
            break;
        }
            
        case 1: { // udp
            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            
            while (fgets(sendline, MAXLINE, stdin) != NULL) {
                socklen_t servlen = sizeof(servaddr);
                sendto(sockfd, sendline, sizeof(sendline), 0, (SS*) &servaddr, servlen);
                
                if (recvfrom(sockfd, recvline, MAXLINE, 0, (SS*) &servaddr, &servlen) < 0) {
                    perror("server terminated prematurely");
                    return 0;
                }
                
                fputs(recvline, stdout);
            }
        }
            
        default:
            return 0;
    }
    
    
}
