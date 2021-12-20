#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include<sys/ioctl.h>
#include <stdbool.h>
#include "commfunc.h"
#include "link_list.h"

#define MSG_MAX_LEN 1024
#define IP_LEN 16
#define PROTOCOL_TYPE_TCP 1
#define PROTOCOL_TYPE_HTTP 2

typedef struct _ClientInfo {
	char clientAddr[IP_LEN];
	int clientPort;
	int clientfd;
	int protocolType;
}ClientInfo;

int g_udpServfd = -1;
int g_tcpServfd = -1;
int g_httpServfd = -1;

LinkInfo g_LinkInfo;

void* startServer(void* pArg);
void* recvTcpData(void* pArg);

pthread_mutex_t g_readDataLock;
pthread_cond_t g_readDataCond;

bool RecvConnectFromClient(ClientInfo arrClientfd[], int, int*, int* );
bool RecvUdpDataFromClient();

int main(int argc, char* argv[])
{
	if(CheckRunning() < 0) { 
		return -1;
	}

	char strIp[56] = { 0 };
	if(GetCfgInfoString("[Role-Server]", "HostIp", strIp) < 0) {
		printf("Get HostIp failed, %s,%s,%d\n", FFL);
		return -1;
	}

	int udpPort = -1;
	if(GetCfgInfoInt("[Role-Server]", "UdpPort", &udpPort) < 0) {
		printf("Get UdpPort failed, %s,%s,%d\n", FFL);
		return -1;
	}

	int tcpPort = -1;
	if(GetCfgInfoInt("[Role-Server]", "TcpPort", &tcpPort) < 0) {
		printf("Get TcpPort failed, %s,%s,%d\n", FFL);
		return -1;
	}

	int httpPort = -1;
	if(GetCfgInfoInt("[Role-Server]", "HttpPort", &httpPort) < 0) {
		printf("Get HttpPort failed, %s,%s,%d\n", FFL);
		return -1;
	}

	printf("Ip:%s, udpPort:%d, tcpPort:%d, httpPort:%d, %s,%s,%d\n", strIp, udpPort, tcpPort, httpPort, FFL);

	g_udpServfd = CreateUdpServer(strIp, udpPort);
	if(g_udpServfd < 0) {
		printf("Create udp Server failed, %s,%s,%d\n", FFL);
		return -1;
	}
	
	printf("Create udp server success udpPort:%d, %s,%s,%d\n", udpPort, FFL);	

	g_tcpServfd = CreateTcpServer(strIp, tcpPort);
	if(g_tcpServfd < 0) {
		printf("Create tcp Server failed!\n");
		return -1;
	}

	printf("Create tcp server success tcpPort:%d, %s,%s,%d\n", tcpPort, FFL);	
	
	g_httpServfd = CreateTcpServer(strIp, httpPort);
	if(g_httpServfd < 0) {
		printf("Create http Server failed!\n");
		return -1;
	}

	printf("Create http server success httpPort:%d, %s,%s,%d\n", httpPort, FFL);	
	printf("g_udpServfd:%d, g_tcpServfd:%d, g_httpServfd:%d\n", g_udpServfd, g_tcpServfd, g_httpServfd);
	
	pthread_mutex_init(&g_readDataLock, NULL);
	pthread_cond_init(&g_readDataCond, NULL);

	g_LinkInfo.nNodeCount = 0;
	g_LinkInfo.pHeadNode = NULL;
	g_LinkInfo.pTailNode = NULL;

	pthread_t tid1, tid2;
	pthread_create(&tid1, NULL, startServer, NULL);
	pthread_create(&tid2, NULL, recvTcpData, NULL);

	pthread_detach(tid2);
	pthread_join(tid1, NULL);
	
	pthread_mutex_destroy(&g_readDataLock);
    pthread_cond_destroy(&g_readDataCond);
	return 0;
}

void* startServer(void* pArg)
{
	ClientInfo arrClientfd[FD_SETSIZE];
	int listenMaxfd = -1;
	int listenMaxClientIndex = -1;

	fd_set readfdset;
	FD_ZERO(&readfdset);
	FD_SET(g_udpServfd, &readfdset);
	FD_SET(g_tcpServfd, &readfdset);
	FD_SET(g_httpServfd, &readfdset);
	listenMaxfd = g_httpServfd;

	int i;
	for(i = 0; i < FD_SETSIZE; i++) {
		arrClientfd[i].clientfd = -1;
	}
	
	while(1) {
		fd_set readsetTmp = readfdset;
		int nReady = select(listenMaxfd + 1, &readsetTmp, NULL, NULL, NULL);
		if (nReady < 0) {
			printf("select error, errno:%d\n", errno);
			return (void*)-1;
		}
		
		if(FD_ISSET(g_udpServfd, &readsetTmp)) {
			if(!RecvUdpDataFromClient()) return (void*)-2;
		}

		if(FD_ISSET(g_tcpServfd, &readsetTmp)) {
			int clientfd = -1, clientfdIndex = -1;
			if(!RecvConnectFromClient(arrClientfd, PROTOCOL_TYPE_TCP, &clientfd, &clientfdIndex)) return (void*)-3;
			FD_SET(clientfd, &readfdset);
			listenMaxfd = clientfd > listenMaxfd ? clientfd:listenMaxfd;
			listenMaxClientIndex = clientfdIndex > listenMaxClientIndex ? clientfdIndex:listenMaxClientIndex;
			if(--nReady == 0) {
				continue;
			}
		}

		if(FD_ISSET(g_httpServfd, &readsetTmp)) {
			int clientfd = -1, clientfdIndex = -1;
			if(!RecvConnectFromClient(arrClientfd, PROTOCOL_TYPE_HTTP, &clientfd, &clientfdIndex)) return (void*)-4;
			FD_SET(clientfd, &readfdset);
			listenMaxfd = clientfd > listenMaxfd ? clientfd:listenMaxfd;
			listenMaxClientIndex = clientfdIndex > listenMaxClientIndex ? clientfdIndex:listenMaxClientIndex;
			if(--nReady == 0) {
				continue;
			}
		}
		
		for(i = 0; i <= listenMaxClientIndex; i++) {
			int sockfd = arrClientfd[i].clientfd;
			if(sockfd > 0 && FD_ISSET(sockfd, &readsetTmp)) {
				int nNeedRecv = -1;
				ioctl(sockfd, FIONREAD, &nNeedRecv);
				if(nNeedRecv == 0) {
					printf("client[%s:%d] close connect\n", arrClientfd[i].clientAddr, arrClientfd[i].clientPort);
					close(sockfd);
					arrClientfd[i].clientfd = -1;
					FD_CLR(sockfd, &readfdset);
					continue;
				}

				char* pMsgBuf = (char* )calloc(1, nNeedRecv+1);
				int nRecvCount = recv(sockfd, pMsgBuf, nNeedRecv, 0);
				if(nRecvCount < 0) {
					printf("recv data failed from client[%s:%d]\n", arrClientfd[i].clientAddr, arrClientfd[i].clientPort);
					continue;
				}

				//actually  nRecvCount = nNeedRecv
				LinkNode* pLinkNode = (LinkNode*)calloc(1, sizeof(LinkNode));
				pLinkNode->peerfd = sockfd;
				pLinkNode->protocolType = arrClientfd[i].protocolType;
				pLinkNode->pMsg = (char* )calloc(1, nRecvCount+1);
				memcpy(pLinkNode->pMsg, pMsgBuf, nRecvCount);
				pLinkNode->msgLen = nRecvCount;

				pthread_mutex_lock(&g_readDataLock);
				AddNodeFromTail(&g_LinkInfo, pLinkNode);
				pthread_mutex_unlock(&g_readDataLock);
				pthread_cond_signal(&g_readDataCond);
			}
		}
	}

	return NULL;
}

bool RecvUdpDataFromClient()
{
	char msgBuf[MSG_MAX_LEN] = { 0 };
	struct sockaddr_in struClientAddr;
	socklen_t socklen = sizeof(struct sockaddr_in);

	int nRecvLen = recvfrom(g_udpServfd, msgBuf, sizeof(msgBuf), 0, (struct sockaddr *)&struClientAddr, &socklen);
	if(nRecvLen < 0) {
		printf("recvfrom error, errno:%d\n", errno);
		return false;
	}

	char peerIp[IP_LEN] = { 0 };
	inet_ntop(AF_INET, &struClientAddr.sin_addr, peerIp, sizeof(peerIp));
	printf("udp server received data from client[%s:%d]\n", peerIp, ntohs(struClientAddr.sin_port));

	printf("udpData:\n");
	HexDump(msgBuf, nRecvLen);
	return true;
}

bool RecvConnectFromClient(ClientInfo arrClientfd[], int protocolType, int* clientfd, int* clientfdIndex)
{
	struct sockaddr_in struClientAddr;
	socklen_t socklen = sizeof(struct sockaddr_in);

	if(protocolType == PROTOCOL_TYPE_TCP)
		*clientfd = accept(g_tcpServfd, (struct sockaddr*)&struClientAddr, &socklen);
	else 
		*clientfd = accept(g_httpServfd, (struct sockaddr*)&struClientAddr, &socklen);

	if(*clientfd < 0) {
		printf("accept error, errno:%d\n", errno);
		return false;
	}

	int index = 0;
	for (index = 0; index < FD_SETSIZE; index++) {
		if(arrClientfd[index].clientfd < 0) {
			arrClientfd[index].clientfd = *clientfd;
			arrClientfd[index].protocolType = protocolType;
			break;
		}
	}

	*clientfdIndex = index;
	inet_ntop(AF_INET, &struClientAddr.sin_addr, arrClientfd[index].clientAddr, IP_LEN);
	arrClientfd[index].clientPort = ntohs(struClientAddr.sin_port);
	if(protocolType == PROTOCOL_TYPE_TCP) 
		printf("Tcp server receive connect from client[%s:%d]\n", arrClientfd[index].clientAddr, arrClientfd[index].clientPort);
	else
		printf("Http server receive connect from client[%s:%d]\n", arrClientfd[index].clientAddr, arrClientfd[index].clientPort);

	return true;
}

void* recvTcpData(void* pArg)
{
	while(1) {
		pthread_mutex_lock(&g_readDataLock);
		while(g_LinkInfo.nNodeCount <= 0 ) {
			pthread_cond_wait(&g_readDataCond, &g_readDataLock);
		}
		LinkNode* pLinkNode = PopNodeFromHead(&g_LinkInfo);
        pthread_mutex_unlock(&g_readDataLock);

		if(!pLinkNode) continue;

		if(pLinkNode->protocolType == PROTOCOL_TYPE_HTTP) {
			printf("Recv Http Msg:\n");
			HexDump(pLinkNode->pMsg, pLinkNode->msgLen);
			char arrResponse[1024] = { 0 };
			sprintf(arrResponse + strlen(arrResponse), "%s", "HTTP/1.1 200 OK\r\n");
			sprintf(arrResponse + strlen(arrResponse), "%s", "Content-Encoding: gzip\r\n");
			sprintf(arrResponse + strlen(arrResponse), "%s", "Content-Type: text/html;charset=utf-8\r\n");
			sprintf(arrResponse + strlen(arrResponse), "%s", "Content-Length: 0\r\n\r\n");
			send(pLinkNode->peerfd, arrResponse, strlen(arrResponse), 0);
		} else {
			printf("Recv Tcp Msg:\n");
			HexDump(pLinkNode->pMsg, pLinkNode->msgLen);
		}

		if(pLinkNode->pMsg) free(pLinkNode->pMsg);
		if(pLinkNode) free(pLinkNode);
	}
}
