/*��һ���֣��궨��*/


//#define AF_INET         2
//#define SOCK_DGRAM      2
#ifndef VOS_OK
#define VOS_OK          0
#endif

#ifndef VOS_ERR
#define VOS_ERR         1
#endif

#define ERR             (-1)

#define MAX_SOCKET_MSG_SIZE     65536
#define FOUND                   1
#define NOTFOUND                0
#define OPENFLOW_ETH_TYPE       (0x1211)
#define SOCKET_MAC_LEN          6

typedef struct openflow_socket_mac_head
{
    unsigned char   aucDMac[SOCKET_MAC_LEN];   //Ŀ��MAC��ַ
    unsigned char   aucSMac[SOCKET_MAC_LEN];   //ԴMAC��ַ
    unsigned short  usEthType;                           //Э������0x1010
}openflow_socket_mac_head;

/*�ڶ����֣��ṹ��*/
typedef struct openflow_in_addr
{
    unsigned int s_addr;       /* IP address of the socket */
}openflow_in_addr;

typedef struct openflow_sockaddr_in
{
    unsigned char sin_len;      /* zy */
    unsigned char sin_family;   /* must be AF_INET */
    unsigned short sin_port;    /* 16-bit port number */
    struct openflow_in_addr sin_addr;    /* 32-bit IP address */
    unsigned char sin_zero[8];           /* must be 0 */
}openflow_sockaddr_in;

// ����mla��
typedef struct tagMLA_FRAME_SOCKET_TLV_HEAD
{
    unsigned int       uiIndex;                 /* ����id             */
    unsigned int       uiPid;                   /* ����               */
    unsigned int       uiReverse;               /* ����               */
    unsigned short     usProgram;               /* �������           */
    unsigned short     usLength;                /* ����Ϣͷ����Ϣ���� */
    unsigned short     usMsgType;               /* ��Ϣ����           */
    unsigned short     usDataLength;            /* ���ݳ���           */

}MLA_FRAME_SOCKET_TLV_HEAD_S;



/*�������֣���������*/

// ��ʱ��� uiReverse �������Ͻӿ�
int Sock_Sendto_V8(void *recvdata, unsigned short revnum, unsigned int uiReverse);
int SOCK_Recvdata_Process(char *recvdata, unsigned int revnum);
void openflow_recvdata(void *pArg);
int Socket_Initial();
void Socket_Close();

/*The end*/
