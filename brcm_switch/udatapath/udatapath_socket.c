#include <config.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "vlog.h"
#include "Hybrid_Framework_Common.h"
#include "udatapath_socket.h"

#define LOG_MODULE VLM_socket

/* V8-datapath��socket fd */
int g_uiListenfd = -1;

char g_RcvBuf[MAX_SOCKET_MSG_SIZE];     /* OPENFLOW���Ľ��ջ�����*/

int g_SockFd = 0;   //linux��FEIͨ�������SockFd���Ǹ�ȫ�ֱ�������ʼ��ʱ����һ�Σ�����������½�����Ҫ����
int g_RevData_Len=0;  //���յ����ݱ��ĵĳ��ȣ����ֵΪ65495
int g_Socket_Thread_Alive_Flag = 1;       //socket ����߳��˳���־

extern HYBRID_INFOCHANGE_BUFFER_S g_ua_info_buf[HYBRID_INFOCHANGE_BUFFER_LENGTH];

int udatapath_GetLocalMac(char *pucMac)
{
    int iIfreqFd = 0;
    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));

    /* ѭ���жϣ��ȴ�vethof�˿ڴ��� */
    if ((iIfreqFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        VLOG_ERR(LOG_MODULE, "fail to open socket.");
        return -1;
     }

    (void)strncpy(ifr.ifr_name, "eth0", strlen("eth0")+1);
    if ((ioctl(iIfreqFd, SIOCGIFHWADDR, &ifr) < 0))
    {
        VLOG_ERR(LOG_MODULE, "fail to read vethof: %s\n", strerror(errno));
        (void)close(iIfreqFd);
        return -1;
    }

    (void)memcpy(pucMac, ifr.ifr_hwaddr.sa_data, SOCKET_MAC_LEN);
    (void)close(iIfreqFd);
    return 0;
}

int send_socket_msg(char* pcMsg, unsigned short usMsgLen)
{
    char*              pcSendBuff  = NULL;
    struct ifreq       ifr;
    struct sockaddr_ll stServaddr;
    int recv_len = 0;
    openflow_socket_mac_head *pstMacHead = NULL;
    char aucSrcMac[SOCKET_MAC_LEN] = {0};

    memset(&ifr, 0, sizeof(ifr));

    if (-1 == udatapath_GetLocalMac(aucSrcMac))
    {
        VLOG_ERR(LOG_MODULE, "get mac failed.");
        return -1;
    }

    pcSendBuff = malloc(usMsgLen + sizeof(openflow_socket_mac_head));
    if(NULL == pcSendBuff)
    {
        VLOG_ERR(LOG_MODULE, "Apply Memory SendBuff Error!\n");
        return -1;
    }
    memset(pcSendBuff, 0, (usMsgLen + sizeof(openflow_socket_mac_head)));
    pstMacHead = (openflow_socket_mac_head *)(void *)pcSendBuff;
    memset(pstMacHead->aucDMac, 0xFF, SOCKET_MAC_LEN);
    (void)memcpy(pstMacHead->aucSMac, aucSrcMac, SOCKET_MAC_LEN);
    pstMacHead->usEthType = (unsigned short)htons(OPENFLOW_ETH_TYPE);

    (void)memcpy(pcSendBuff + sizeof(openflow_socket_mac_head), pcMsg, usMsgLen);

    /* ��vethof�˿� */
    memset(&stServaddr, 0, sizeof(stServaddr));
    stServaddr.sll_family   = AF_PACKET;
    stServaddr.sll_protocol = (unsigned short)htons(OPENFLOW_ETH_TYPE);
    stServaddr.sll_ifindex  = if_nametoindex("eth0");
    (void)memcpy(stServaddr.sll_addr, pstMacHead->aucDMac, SOCKET_MAC_LEN);

    /* �ж�RAWģʽsocket�Ƿ���� */
    if (-1 == g_uiListenfd)
    {
        VLOG_ERR(LOG_MODULE, "Listen ID is illegal.");
        (void)free(pcSendBuff);
        return -1;
    }

    recv_len = sendto(g_uiListenfd, pcSendBuff, usMsgLen + sizeof(openflow_socket_mac_head), MSG_DONTWAIT,
                      (struct sockaddr *)&stServaddr, sizeof(stServaddr));
    if(0 >= recv_len)
    {
        VLOG_ERR(LOG_MODULE, "openflow send socket fail!\n");
        (void)free(pcSendBuff);
        return -1;
    }

    (void)free(pcSendBuff);
    return 0;
}

/* V8���ݷ��ͺ��� */
int Sock_Sendto_V8(void *recvdata, unsigned short revnum, unsigned int uiReverse)
{
    int iRet =0;
    char* pBuf = NULL;
    MLA_FRAME_SOCKET_TLV_HEAD_S* ptmp = NULL;

    // ���һ����Ϣͷ
    pBuf = malloc(revnum + sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S));
    if(NULL == pBuf)
    {
        VLOG_ERR(LOG_MODULE, "Apply Memory pBuf Error for Sock_Sendto_V8\n");
        return -1;
    }
    memset(pBuf, 0, (revnum + sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S)));

    ptmp = (MLA_FRAME_SOCKET_TLV_HEAD_S*)pBuf;

    ptmp->uiIndex = 0;
    ptmp->uiPid = 1;
    ptmp->usProgram = 0;
    ptmp->uiReverse = uiReverse;
    ptmp->usDataLength = revnum;
    ptmp->usLength = sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S) + revnum;
    memcpy(pBuf + sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S), recvdata, revnum);

    iRet = send_socket_msg(pBuf, ptmp->usLength);

    free(pBuf);

    if(iRet < 0)
    {
        VLOG_ERR(LOG_MODULE, "SOCK Send to V8 Fail\n");
        return -1;
    }
    VLOG_DBG(LOG_MODULE, "SOCK Send to V8 success\n");

    return 0;

}


int SockRecv_Find_WaitBuf(unsigned int uiIdentifier, unsigned int *uiBufId)
{
    unsigned int iLoop = 0;
    for (iLoop=0; iLoop<HYBRID_INFOCHANGE_BUFFER_LENGTH; iLoop++)
    {
        if ((uiIdentifier == g_ua_info_buf[iLoop].ui_identifier) &&
            (g_ua_info_buf[iLoop].wait_v8reply_flag == HYBRID_INFOCHANGE_WAITV8REPLY))
        {
            *uiBufId = iLoop;
            return FOUND;
        }
    }

    return NOTFOUND;
}

/*V8���ݴ�����*/
int SOCK_Recvdata_Process(char *recvdata, unsigned int revnum)
{
    openflow_socket_mac_head    *pstMsgMacHead = NULL;
    HYBRID_INFO_CHANGE_S        *pstSendDate  = NULL;
    unsigned int uiIdentifier = 0;
    unsigned int uiBufId = 0;
    unsigned int uiProcessRet = 0;

    if(revnum < sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S) +
            sizeof(HYBRID_INFO_CHANGE_S) + sizeof(openflow_socket_mac_head))
    {
        VLOG_ERR(LOG_MODULE, "SOCK Receive Invalid Data");
        return -1;
    }

    /*�ж���Ϣͷ�Ƿ���openflow����Ϣͷ����������򷵻ش���*/
    pstMsgMacHead = (openflow_socket_mac_head*)recvdata;
    if(OPENFLOW_ETH_TYPE !=  pstMsgMacHead->usEthType)
    {
        VLOG_ERR(LOG_MODULE, "SOCK Receive not openflow Data");
        return -1;
    }
    // ��Ϣͷƫ�ƣ�֮��recvdataָ�벻��ָ��ԭʼ����ͷ��
    pstSendDate = (HYBRID_INFO_CHANGE_S *)(recvdata + sizeof(openflow_socket_mac_head) +
                sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S));

    revnum = revnum - sizeof(MLA_FRAME_SOCKET_TLV_HEAD_S) - sizeof(openflow_socket_mac_head);

    // �յ�����Ϣ�����  ui_identifier �ֶ�
    uiIdentifier = pstSendDate->ui_identifier;

    // �ж��Ƿ�Ϊ��Ҫ�ȴ���Ӧ�ı���
    // û�л�������ֻ����event����
    if(NOTFOUND == SockRecv_Find_WaitBuf(uiIdentifier, &uiBufId))
    {
        // Hybrid��ܴ���V8���͵�EVENT���ģ�����Packet in��LLDP,�˿���Ϣ���˿�״̬���౨��
        uiProcessRet = Hybird_Frame_Process_UnknownIdentifier_Packet((char *)pstSendDate, revnum);
        if(VOS_OK == uiProcessRet)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    // ����Ҫ�ȴ���Ӧ����Ϣ���Ŵ���
    else
    {
        g_ua_info_buf[uiBufId].pst_info = pstSendDate;
        g_ua_info_buf[uiBufId].data_len = revnum;

        /*��ʱ����Ӧ�������ȣ����ݿ�����ɺ��ͷ��ź�����֪ͨ���ȥȡ*/
        sem_post(&(g_ua_info_buf[uiBufId].sem_isused));

        /*�ȴ���ܽ�����ж������,�ͷ��ź��������ȣ�SOCK�̲߳�����������*/
        // ������ܵȰɣ�event���Ķ���������
        // ������ľ����ˣ�û�в���������Ĳ��ᴦ��ģ���select���û������
        //sem_wait(&g_sem_waitframe_restordata);
        return 0;
    }
}

/*V8���ݽ����߳�*/
void openflow_recvdata(void *pArg)

{
    int                iRet        = 0;
    int                iIfreqFd    = 0;
    int                recv_len    = 0;
    int                uiListenfd  = -1;
    struct ifreq       ifr;
    struct timespec    stTime;
    struct sockaddr_ll stServaddr;
    fd_set sockFds;

    stTime.tv_sec  = 0;
    stTime.tv_nsec = 100000 * 1000;

    memset(&ifr, 0, sizeof(ifr));
    memset(&stServaddr, 0, sizeof(stServaddr));
    memset(g_RcvBuf, 0, MAX_SOCKET_MSG_SIZE);

    /* ѭ���жϣ��ȴ�vethof�˿ڴ��� */
    if ((iIfreqFd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        VLOG_ERR(LOG_MODULE, "fail to open socket.");
        return;
    }

    (void)strncpy(ifr.ifr_name, "eth0", strlen("eth0")+1);
    while ((ioctl(iIfreqFd, SIOCGIFMTU, &ifr) < 0))
    {
        VLOG_ERR(LOG_MODULE, "fail to read vethof: %s\n", strerror(errno));
        (void)nanosleep(&stTime, &stTime);
    }

    (void)close(iIfreqFd);

    /* ����RAWģʽsocket */
    if ((uiListenfd = socket(AF_PACKET, SOCK_RAW, htons(OPENFLOW_ETH_TYPE))) == -1 )
    {
        VLOG_ERR(LOG_MODULE, "create socket error: %s\n", strerror(errno));
        return;
    }

    /* ��vethof�˿� */
    stServaddr.sll_family   = AF_PACKET;
    stServaddr.sll_protocol = htons(OPENFLOW_ETH_TYPE);
    stServaddr.sll_ifindex  = if_nametoindex("eth0");
    if(bind(uiListenfd, (struct sockaddr *)&stServaddr, sizeof(stServaddr)) != 0)
    {
        VLOG_ERR(LOG_MODULE, "bind socket error: %s\n", strerror(errno));
        (void)close(uiListenfd);
        return;
    }

    for (;;)
    {
        FD_ZERO(&sockFds);
        FD_SET(uiListenfd, &sockFds);

        if (select(uiListenfd + 1, &sockFds, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(uiListenfd, &sockFds))
            {
                VLOG_DBG(LOG_MODULE, "receive a packet!\n");
                /*�ӿڰ�������������ذ����Ӧ����*/
                memset(g_RcvBuf, 0, MAX_SOCKET_MSG_SIZE);
                recv_len = recvfrom(uiListenfd, g_RcvBuf,
                                    MAX_SOCKET_MSG_SIZE, MSG_DONTWAIT, NULL, NULL);
                if(0 < recv_len)
                {
                    VLOG_DBG(LOG_MODULE, "receive data from v8\n");
                    iRet = SOCK_Recvdata_Process(g_RcvBuf, (unsigned int)recv_len);
                    if (0 != iRet)
                    {
                        VLOG_ERR(LOG_MODULE, "Recvdata process fail\n");
                    }
                }
            }
        }
    }

    (void)close(uiListenfd);
    return;
}

/*V8-Linux SOCKET��ʼ������*/
void Socket_Create()
{
    g_uiListenfd = socket(AF_PACKET, SOCK_RAW, htons(OPENFLOW_ETH_TYPE));
    if (-1 == g_uiListenfd)
    {
        VLOG_ERR(LOG_MODULE, "create send socket error: %s\n", strerror(errno));
        return;
    }
}

int Socket_Initial()
{
    pthread_t  tid_socketFEI = 0;
    void*      temppoint     = NULL;

    Socket_Create();

    if (pthread_create(&tid_socketFEI, NULL, (void*)openflow_recvdata, (void*)temppoint))
    {
        VLOG_ERR(LOG_MODULE, "Create Thread Failed\n");
        exit(1);
    }
    return 0;
}

void Socket_Close()
{
    (void)close(g_uiListenfd);

    g_uiListenfd = -1;

    return 0;
}


/*The end*/
