
#ifndef __HYBRID_FRAMEWORK_COMMON_H_
#define __HYBRID_FRAMEWORK_COMMON_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#include <semaphore.h>

/*-----------------------------------------------*/
/*             1���������ö�ٶ���               */
/*-----------------------------------------------*/
typedef int INT32;
typedef unsigned int UINT32;
typedef void VOID;
typedef char CHAR;
typedef unsigned char UCHAR;
typedef long LONG;
typedef unsigned long ULONG32;
typedef unsigned short USHORT;

#define VOS_OK     0
#define VOS_ERR    1
//#define NULL 0
#define VOS_NULL 0

#define HYBRID_INFOCHANGE_VERSION  1
#define UNIT_ID    0


#define ACTION_CREAT  1
#define ACTION_DELETE  2
#define ACTION_OPNFLOWSET 3

#define    HYBRID_INFOCHANGE_WAITV8REPLY    1

#define    HYBRID_INFOCHANGE_BUFFER_LENGTH    1
#define    HYBRID_INFOCHANGE_VALUE_USED       1
#define    HYBRID_INFOCHANGE_VALUE_UNUSED     0


/*-----------------------------------------------*/
/*             2�����ݽṹ����                   */
/*-----------------------------------------------*/

typedef struct tagHybrid_Info_Change
{
    UINT32 ui_identifier;
    UINT32 ui_ver;
    UINT32 ui_method;
    UINT32 ui_module;
    UINT32 ui_action;
    UINT32 ui_length;     //�����ݵĳ��ȣ�puc_value��ʼ�ĳ��ȣ���λB
}HYBRID_INFO_CHANGE_S;


typedef struct tagHybrid_Info_Change_Buffer_Struct
{
    HYBRID_INFO_CHANGE_S *pst_info;
    UINT32 data_len;   //������Ȱ�����ͷ
    sem_t  sem_isused;
    INT32  wait_v8reply_flag;
    UINT32 ui_identifier;  
}HYBRID_INFOCHANGE_BUFFER_S;


/*-----------------------------------------------*/
/*             3�����������궨��                 */
/*-----------------------------------------------*/

#if 0
#define FEISW_SFM_SET_STACKINFO_KEY(stSfmStackKey, pstFEIISfmStackInfo) \
{\
    (stSfmStackKey).uiPortIndex = (pstFEIISfmStackInfo)->uiPortIndex;\
    (stSfmStackKey).uiChassisID = (pstFEIISfmStackInfo)->uiStackID;\
}
#endif


/*-----------------------------------------------*/
/*             4����������                       */
/*-----------------------------------------------*/

UINT32 Hybird_Frame_Process_UnknownIdentifier_Packet(CHAR *recvdata, UINT32 revnum);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif     /* #ifndef __HYBRID_FRAMEWORK_COMMON_H_ */

