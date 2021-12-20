#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "commfunc.h"
#define MAXLINE 5000

#define PROTOCOL_TYPE_TCP  1
#define PROTOCOL_TYPE_UDP  2
//#define PROTOCOL_TYPE_HTTP 3

int main(int argc, char *argv[])
{
	printf("1. tcp   2. udp\n");
	printf("Please select protocol: ");
	char input[10] = { 0 };
	fgets(input, 10, stdin);
	int protocol = atoi((char*)input);
	if(protocol < PROTOCOL_TYPE_TCP || protocol > PROTOCOL_TYPE_UDP) {
		printf("select error !\n");
		return -1;
	}

	char strIp[56] = { 0 };
	if(GetCfgInfoString("[Role-Client]", "HostIp", strIp) < 0) {
		printf("Get HostIp failed, %s,%s,%d\n", FFL);
		return -1;
	}

	int udpPort = -1;
	if(protocol == PROTOCOL_TYPE_UDP) {
		if(GetCfgInfoInt("[Role-Client]", "UdpPort", &udpPort) < 0) {
			printf("Get UdpPort failed, %s,%s,%d\n", FFL);
			return -1;
		}
	}

	int tcpPort = -1;
	if(protocol == PROTOCOL_TYPE_TCP) {
		if(GetCfgInfoInt("[Role-Client]", "TcpPort", &tcpPort) < 0) {
			printf("Get TcpPort failed, %s,%s,%d\n", FFL);
			return -1;
		}
	}

	struct sockaddr_in servaddr;
	int sockfd, n;
	char buf[MAXLINE];
	memset(buf, 0, MAXLINE);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, strIp, &servaddr.sin_addr);

	if(protocol == PROTOCOL_TYPE_UDP) {
		printf("Ip:%s, udpPort:%d\n", strIp, udpPort);
		servaddr.sin_port = htons(udpPort);
		sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	} else if(protocol == PROTOCOL_TYPE_TCP) {
		printf("Ip:%s, tcpPort:%d\n", strIp, tcpPort);
		servaddr.sin_port = htons(tcpPort);
		sockfd = socket(AF_INET, SOCK_STREAM, 0);

		if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0) {
			printf("connect server failed, errno:%d, %s,%s,%d\n", errno, FFL);
			return -1;
		}

		printf("connect server[%s:%d] success\n", strIp, tcpPort);
	}

	char arrHttpMsg[1024] = { 0 };

#if 0
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "WPOST / HTTP/1.1\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "Content-Type: text/xml\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "User-Agent: PostmanRuntime/7.28.4\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "Host: 192.168.18.131:20000\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "Connection: keep-alive\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "Content-Length: 12\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "password=\r\n");
	sprintf(arrHttpMsg + strlen(arrHttpMsg), "%s", "&doing=login\r\n");
	printf("HttpMsg:\n%s\n", arrHttpMsg);
#endif

	char* pCont = ReadHexString("./hex_string.txt");
	unsigned char* pHexArr = (unsigned char*)calloc(1, strlen(pCont) + 1);
	int HexCount = -1;
	ConvertByteString2Hex(pCont, pHexArr, &HexCount);
	free(pCont);
	pCont = NULL;

	while(1) {
		char userData[512] = { 0 };
		printf("Please Input Data: ");
		fgets(userData, MAXLINE, stdin);
		userData[strlen(userData)-1] = '\0';
		
		if(protocol == PROTOCOL_TYPE_TCP) {
			if(HexCount > 0) {
				send(sockfd, pHexArr, HexCount, 0);
			} else if(strlen(arrHttpMsg) > 0) {
				send(sockfd, arrHttpMsg, strlen(arrHttpMsg), 0);
			} else {
				send(sockfd, userData, strlen(userData), 0);
			}
		} else if(protocol == PROTOCOL_TYPE_UDP) {
			
			//sendto(sockfd, pHexArr, HexCount, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
			sendto(sockfd, userData, strlen(userData), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		}
	}

	close(sockfd);
	return 0;
}
