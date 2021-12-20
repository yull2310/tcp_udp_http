#include "commfunc.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>

#define LINE_MAX_LEN 2048

int CheckRunning()
{
	char buf[LINE_MAX_LEN] = { 0 };
	int ret = readlink("/proc/self/exe", buf, sizeof(buf));
	if(ret == -1) {
		return -1;
	}

	char appName[56] = { 0 };
	strcpy(appName, basename(buf));

	char appPath[962] = { 0 };
	strcpy(appPath, dirname(buf));

	char fileLock[1024] = { 0 };
	sprintf(fileLock, "%s/.%s.lock", appPath, appName);
	int fd = open(fileLock, O_CREAT|O_TRUNC|O_WRONLY, S_IWRITE);
	if(fd < 0) {
		return -2;
	}

	if(lockf(fd, F_TLOCK, 0) <  0) {
		printf("%s is running\n", appName);
		return -3;
	}
	
	return 0;
}

int RegisterSignal()
{
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
}

bool ReadCfgFile(char* pSection, char* pKey, char* pValue)
{
	if(!pSection || !pKey || !pValue) return false;

	char* pPath = "./HostInfo.cfg";
	FILE* pFile = fopen(pPath, "r");
	if(!pFile) return false;

	char szLineData[LINE_MAX_LEN] = { 0 };
	bool bSectionFlag = false;
	while(fgets(szLineData, sizeof(szLineData), pFile))
	{
		if(szLineData[strlen(szLineData)-1] == '\n') {
			szLineData[strlen(szLineData)-1] = '\0';
		}

		if(!strcmp(szLineData, pSection)) {
			bSectionFlag = true;
			memset(szLineData, 0, sizeof(szLineData));
			continue;
		}

		if(bSectionFlag) {
			char szTmp[LINE_MAX_LEN] = { 0 };
			int i = 0;
			int j = 0;
			for(i = 0; i < strlen(szLineData); i++) {
				if(szLineData[i] != ' ') {
					szTmp[j++] = szLineData[i];
				}
			}

			char* pResult = strstr(szTmp, "=");
			if(!pResult) return false;
			*pResult++ = '\0';
			if(!strcmp(szTmp, pKey)) {
				strcpy(pValue, pResult);
				return true;
			}
		}
		
		memset(szLineData, 0, sizeof(szLineData));
	}
	
	fclose(pFile);
	return false;
}

int GetCfgInfoInt(char* pSection, char* pKey, int* pValue)
{
	char arrResult[LINE_MAX_LEN] = { 0 };
	if(!ReadCfgFile(pSection, pKey, arrResult)) return -1;

	*pValue = atoi(arrResult);
	return 0;
}

int GetCfgInfoString(char* pSection, char* pKey, char* pValue)
{
	char arrResult[LINE_MAX_LEN] = { 0 };
	if(!ReadCfgFile(pSection, pKey, arrResult)) return -1;

	strcpy(pValue, arrResult);
	return 0;
}

int CreateUdpServer(char* pHostIp, int udpPort)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) {
		perror("socket error");
		return -1;
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, pHostIp, &servaddr.sin_addr.s_addr);
	servaddr.sin_port = htons(udpPort);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind error");
		return -2;
	}

	return sockfd;
}

int CreateTcpServer(char* pHostIp, int tcpPort)
{
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("socket error");
		return -1;
	}

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, pHostIp, &servaddr.sin_addr.s_addr);
	servaddr.sin_port = htons(tcpPort);

	int op = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&op , sizeof(int));

	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind error");
		return -2;
	}

	if(listen(sockfd, 20) < 0) {
		perror("listen error");
		return -3;
	}

	return sockfd;
}

int ConvertByteString2Hex(const char* pSrcString, unsigned char* pDstString, int* pDstStringLen)
{
	if(!pSrcString || !pDstString || !pDstStringLen) return -1;

	*pDstStringLen = 0;
	char hex[2] = { 0 };
	int i = 0, j = 0;
	const char* pTmp = NULL;

	while(i < strlen(pSrcString)) {
		if(pSrcString[i] == ' ') {
			i += 1;
			continue;
		}

		pTmp = pSrcString + i;
		memcpy(hex, pTmp, sizeof(hex));

		for(j = 0; j < 2; j++) {
			int num = 0;
			if (hex[j] >= '0' && hex[j] <= '9') num = hex[j] - '0';
			if (hex[j] >= 'a' && hex[j] <= 'f') num = hex[j] - 87;
			if (hex[j] >= 'A' && hex[j] <= 'F') num = hex[j] - 55;
			if(j == 0) pDstString[*pDstStringLen] = num*16;
			if(j == 1) pDstString[*pDstStringLen] += num;
		}

		(*pDstStringLen)++;
		char* pSpacePos = strstr(pTmp, " ");
		i += (pSpacePos - pTmp + 1);
	}

	return 0;
}

unsigned short make_crc16(unsigned char* msg, unsigned short len)
{
    unsigned short crc16 = 0xFFFF;
    unsigned short i, j = 0;
    unsigned char c15, bit;

    for (i = 0; i < len ; i++)
    {
        for (j = 0; j < 8; j++)
        {
            c15 = ((crc16 >> 15 & 1) == 1);
            bit = ((msg[i] >> (7 - j) & 1) == 1);
            crc16 <<= 1;

            if (c15 ^ bit)
            {
                crc16 ^= 0x1021;
            }
        }
    }

    return crc16;
}

int HexDump(const char* pSrc, int len)
{
	if(!pSrc || !len) return -1;

	int i = 0;
	int loopCount = len / 16;
	int remainder = len % 16;
	for(i = 0; i < loopCount; i++) {
		int j = 0;
		for(j = 0; j < 16; j++) {
			printf("%02X ", (unsigned char)pSrc[i*16+j]);
		}

		printf("  |");

		for(j = 0; j < 16; j++) {
			if(pSrc[i*16+j] >= 32 && pSrc[i*16+j] < 127) {
				printf("%c", pSrc[i*16+j]);
			} else {
				printf(".");
			}
		}

		printf("|\n");
	}

	for(i = 0; i < remainder; i++) {
		printf("%02X ", (unsigned char)pSrc[loopCount*16+i]);
	}

	for(i = 0; i < (50-3*remainder); i++) {
		printf(" ");
	}

	printf("|");

	for(i = 0; i < remainder; i++) {
		if(pSrc[loopCount*16+i] >= 32 && pSrc[loopCount*16+i] < 127) {
			printf("%c", pSrc[loopCount*16+i]);
		} else {
			printf(".");
		}
	}

	printf("|\n");
}

int GetFileSize(const char *pFilePath)
{
    struct stat statbuff;
    memset(&statbuff, 0, sizeof(struct stat));
	if (stat(pFilePath, &statbuff) < 0)  return 0;
	return statbuff.st_size;
}

char* ReadHexString(const char* pFilePath)
{
	if(!pFilePath) return NULL;

	int size = GetFileSize(pFilePath);
	if(!size) return NULL;

	char* pCont = (char*)calloc(1, size+1);
	if(!pCont) return NULL;

	FILE* pFile = fopen(pFilePath, "r");
	if(!pFile) return NULL;

	char lineData[LINE_MAX_LEN] = { 0 };
	while(fgets(lineData, LINE_MAX_LEN, pFile)) {
		if(lineData[strlen(lineData)-1] == '\n') {
			lineData[strlen(lineData)-1] = '\0';
		}
		
		if(lineData[strlen(lineData)-1] == '\r') {
			lineData[strlen(lineData)-1] = '\0';
		}

		if(strlen(pCont) == 0)
			sprintf(pCont + strlen(pCont), "%s", lineData);
		else
			sprintf(pCont + strlen(pCont), " %s", lineData);
	}

	return pCont;
}