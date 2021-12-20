#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__
#include <stdbool.h>

typedef struct _LinkNode
{
    struct _LinkNode* pPrevNode;
    struct _LinkNode* pNextNode;
    char* pMsg;
    int msgLen;
    int peerfd;
    int protocolType;
}LinkNode;

typedef struct _LinkInfo
{
    int nNodeCount;
    LinkNode* pHeadNode;
    LinkNode* pTailNode;
}LinkInfo;

bool AddNodeFromHead(LinkInfo* pLinkInfo, LinkNode* pCurrNode);
bool AddNodeFromTail(LinkInfo* pLinkInfo, LinkNode* pCurrNode);

LinkNode* PopNodeFromHead(LinkInfo* pLinkInfo);
LinkNode* PopNodeFromTail(LinkInfo* pLinkInfo);

bool DelNode(LinkInfo* pLinkInfo, LinkNode* pCurrNode);

bool ShowLinkList(LinkInfo* pLinkInfo);

#endif