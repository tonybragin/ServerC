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
            
        case 2: { // raw udp
            sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
            
            struct ip_header {
                int ver_ihl; // 1byte 4/5
                int dos; // 1byte 0
                int total_lenth; // 2bytes dont need to touch
                int id; // 2byteы
                int flag_offset; // 2byte 0/0/0  0
                int ttl; // 1byte 255
                int checksum; // 2byte dont need to touch
                struct in_addr ips; // 4byte
                struct in_addr ipd; // 4byte
            };
            
            struct udp_header {
                int dp;
                int sp;
                int lenth;
                int checksum;
            };
            
            struct ip_header *pih = malloc(sizeof(char)*20);
            pih->id = 12345;
            pih->ttl = 255;
            pih->ipd = servaddr.sin_addr;
            inet_aton("127.0.0.1", &pih->ips);
            
            
            struct udp_header *puh = malloc(sizeof(char)*8);
            puh->dp = 8888;
            puh->sp = 88;
            puh->lenth = 8;
            puh->checksum = 0;
            
            while (fgets(sendline, MAXLINE, stdin) != NULL) {
                socklen_t servlen = sizeof(servaddr);
                
                puh->lenth += sizeof(sendline);
                char *msg = malloc(sizeof(char) * puh->lenth);
                
                msg = pih;
                msg += 20;
                
                msg = puh;
                msg += 8;
                msg = sendline;
                
                sendto(sockfd, msg, sizeof(msg), 0, (SS*) &servaddr, servlen);
                
                if (recvfrom(sockfd, recvline, MAXLINE, 0, (SS*) &servaddr, &servlen) < 0) {
                    perror("server terminated prematurely");
                    return 0;
                }
                
                fputs(recvline, stdout);
            }
            
            break;
        }
            
        default:
            return 0;
    }
    
    
}
