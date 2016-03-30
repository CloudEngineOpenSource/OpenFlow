#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "status.h"
#define FIFO_FILE_M "/home/pipe_file_m"
#define FIFO_FILE_S "/home/pipe_file_s"
/*��ǰ�����д����������ܵ�:��ѯofprotocol״̬�Ĺܵ���master�ܵ�����ѯ֮��ͨ��slave�ܵ����¼�֪ͨ��ofprotocl����*/
void main(void)
{
    int fd_m = 0;
    int fd_s = 0;
    int flag = 0;
    int bytes_read = 0;
    int Ret = 0;
    char ofprotocol[1] = {0};
    char ofprotocol_new[1] = {0};
    /*����slave�ܵ�*/
    if((-1 == access(FIFO_FILE_S,F_OK)) || (-1 == access(FIFO_FILE_S,F_OK)))            //�ļ��Ƿ����
    {
        printf("Ofprotocol program is not run yet!\n");
        return ;
    }

    /*�򿪲��Ҷ�ȡmaster�ܵ�*/
    fd_m = open(FIFO_FILE_M,O_RDWR | O_NONBLOCK);
    if(-1 == fd_m)
    {
        printf("Failed to open master pipe file, err is %s\n",strerror(errno));
        return ;
    }
    //printf("the master file`s descriptor is %d\n",fd_m);

    (void)read(fd_m,(void*)ofprotocol,sizeof(ofprotocol));
    /*���ݶ�ȡ����ֵ��ӡ����Ӧ��״̬*/
    if (PROTOCOL_CONNECTTED == ofprotocol[0])
    {
        printf("/**********************************************************************/\n");
        printf("/*********************The protocol is connected!***********************/\n");
        printf("/**********************************************************************/\n");
    }
    if (PROTOCOL_DISCONNECT == ofprotocol[0])
    {
        printf("/**********************************************************************/\n");
        printf("/*******************The protocol is not connected!*********************/\n");
        printf("/**********************************************************************/\n");
    }

    /*ͨ��slave�ܵ���״̬������ofprotocl����*/
    fd_s = open(FIFO_FILE_S,O_RDWR | O_NONBLOCK);
    if(-1 == fd_s)
    {
        printf("Failed to open the slave file, err is %s\n",strerror(errno));
        return ;
    }

    ofprotocol_new[0] = PROTOCOL_MAX;
    (void)write(fd_s,(void*)ofprotocol_new,sizeof(ofprotocol_new));

    if(-1 == close(fd_m))
    {
         printf("Failed to close master fd %d:%s\n", fd_m, strerror(errno));
    }

    if(-1 == close(fd_s))
    {
         printf("Failed to close slave fd %d:%s\n", fd_s, strerror(errno));
    }
    return ;
}

