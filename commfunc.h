#ifndef __COMMFUNC_H__
#define __COMMFUNC_H__

#define FFL __FILE__,__func__,__LINE__

int CheckRunning();
int RegisterSignal();
int GetCfgInfoInt(char* pSection, char* pKey, int* pValue);
int GetCfgInfoString(char* pSection, char* pKey, char* pValue);
int CreateUdpServer(char* pHostIp, int udpPort);
int CreateTcpServer(char* pHostIp, int tcpPort);
int ConvertByteString2Hex(const char* pCharArr, unsigned char* pHexArr, int* hexArrLen);
int HexDump(const char* pSrc, int len);
char* ReadHexString(const char* pFilePath);

unsigned short make_crc16(unsigned char* msg, unsigned short len);

#endif
