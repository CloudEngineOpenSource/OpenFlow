 /*****************************************************
*filename: timer_wheel.h
******************************************************/
#ifndef __TIMER_WHEEL__
#define __TIMER_WHEEL__
#include "list.h"
typedef int timer_expiry( void *user_data);

typedef struct _alta_aclTimer
{
    unsigned char type;
    /** timer for hardware timerout. */
    int  hardtimer;

    /** timer for idle timerout. */
    int  idletimer;

} alta_aclTimer;

struct timer{
    struct list node;
    alta_aclTimer interval; /*��ʱ����ʱֵ*/
    time_t timeout;	 /*��ʱʱ�� */
    timer_expiry *func;	 /*��ʱ���ص�����*/
    void *user_data;	 /*��ʱ���������*/
};

/*��ʱ������*/
struct timerwheel{
    unsigned int   time_accuracy;	 /*��ʱ��ʱ��Ƭ��С*/
    unsigned int   num;	 /*��ʱ���������*/
    unsigned int   cur_index;	 /*��ʱ����ǰʱ��Ƭ��С*/
    time_t  cur_time;	 /*��ʱ����ǰʱ��*/
    time_t  pre_time;	 /*��ʱ����һ��ʱ��*/
    struct list **timer_list;
};

int  timerwheel_init(const unsigned int num, const unsigned int  time_accuracy);
int  timerwheel_del(struct timer *p);
void *  timerwheel_add(struct timer *p,const alta_aclTimer interval, timer_expiry *func, void *user_data);
void  run_timers(void);

#endif

