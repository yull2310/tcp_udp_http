#include "link_list.h"
#include <stdio.h>

bool AddNodeFromHead(LinkInfo* pLinkInfo, LinkNode* pCurrNode)
{
    if(!pLinkInfo || !pCurrNode) return false;

    pCurrNode->pPrevNode = NULL;
    pCurrNode->pNextNode = NULL;

    if(!(pLinkInfo->pHeadNode)) {
        pLinkInfo->pHeadNode = pCurrNode;
        pLinkInfo->pTailNode = pCurrNode;
    } else {
        pCurrNode->pNextNode = pLinkInfo->pHeadNode;
        pCurrNode->pPrevNode = NULL;
        pLinkInfo->pHeadNode->pPrevNode = pCurrNode;
        pLinkInfo->pHeadNode = pCurrNode;

        if(!(pLinkInfo->pTailNode->pPrevNode)) {
            pLinkInfo->pTailNode->pPrevNode = pCurrNode;
        }
    }

    pLinkInfo->nNodeCount++;
    return true;
}

bool AddNodeFromTail(LinkInfo* pLinkInfo, LinkNode* pCurrNode)
{
    if(!pLinkInfo || !pCurrNode) return false;

    pCurrNode->pPrevNode = NULL;
    pCurrNode->pNextNode = NULL;

    if(!(pLinkInfo->pHeadNode)) {
        pLinkInfo->pHeadNode = pCurrNode;
        pLinkInfo->pTailNode = pCurrNode;
    } else {
        pLinkInfo->pTailNode->pNextNode = pCurrNode;
        pCurrNode->pPrevNode = pLinkInfo->pTailNode;
        pLinkInfo->pTailNode = pCurrNode;
    }

    pLinkInfo->nNodeCount++;
    return true;
}

bool DelNode(LinkInfo* pLinkInfo, LinkNode* pCurrNode)
{
    if(!pLinkInfo || !pCurrNode) return false;

    LinkNode* pNodeTmp = pLinkInfo->pHeadNode;
    while(pNodeTmp) {
        if(pCurrNode == pNodeTmp) {
            if(pCurrNode->pNextNode) {
                pCurrNode->pNextNode->pPrevNode = pCurrNode->pPrevNode;
            } else {
                pLinkInfo->pTailNode = pCurrNode->pPrevNode;
            }

            if(pCurrNode->pPrevNode) {
                pCurrNode->pPrevNode->pNextNode = pCurrNode->pNextNode;
            } else {
                pLinkInfo->pHeadNode = pCurrNode->pNextNode;
            }

            pLinkInfo->nNodeCount--;
            break;
        }

        pNodeTmp = pNodeTmp->pNextNode;
    }

    return true;
}

LinkNode* PopNodeFromHead(LinkInfo* pLinkInfo)
{
    if(!pLinkInfo || !(pLinkInfo->pHeadNode)) return NULL;

    LinkNode* pRetNode = pLinkInfo->pHeadNode;
    if(pLinkInfo->pHeadNode->pNextNode) {
        pLinkInfo->pHeadNode = pLinkInfo->pHeadNode->pNextNode;
        pLinkInfo->pHeadNode->pPrevNode = NULL;
    } else {
        pLinkInfo->pHeadNode = NULL;
        pLinkInfo->pTailNode = NULL;
    }

    pLinkInfo->nNodeCount--;
    return pRetNode;
}

LinkNode* PopNodeFromTail(LinkInfo* pLinkInfo)
{
    if(!pLinkInfo || !(pLinkInfo->pHeadNode)) return NULL;

    LinkNode* pRetNode = pLinkInfo->pTailNode;

    if(pLinkInfo->pTailNode->pPrevNode) {
       pLinkInfo->pTailNode = pLinkInfo->pTailNode->pPrevNode;
       pLinkInfo->pTailNode->pNextNode = NULL;
    } else {
        pLinkInfo->pHeadNode = NULL;
        pLinkInfo->pTailNode = NULL;
    }

    pLinkInfo->nNodeCount--;
    return pRetNode;
}

bool ShowLinkList(LinkInfo* pLinkInfo)
{
    if(!pLinkInfo) return false;

    LinkNode* pNodeTmp = pLinkInfo->pHeadNode;
    while(pNodeTmp) {
        printf("%d ", pNodeTmp->peerfd);
        pNodeTmp = pNodeTmp->pNextNode;
    }

    printf("\n");
    return true;
}