#define NAME(cnt)  usr_##cnt
#define USERNUM   3000
#define LAMBDA    5
#define PURE
#ifndef CHANNEL_H
#define CHANNEL_H
#include <QTimer>
#include <QObject>
#include <QWidget>
#include <QList>
#include <QMutex>
#include <QDebug>
#include "usernode.h"
#include "multiacpro.h"
#include <QtCore/qmath.h>
#include <QTime>
#include <QThread>
#include <QEventLoop>
#include <QMap>
#include <QMessageBox>
#include "ui_mainwindow.h"
class MainWindow;
enum status{
    NON_BLOCK=0,
    BLOCK,
};

/*slot_aloha 协议中，第几个时隙点，以及该时隙点有几个user碰撞*/
class Item{
public:
    unsigned int index;
    unsigned int cnt;
    Item(unsigned int index,unsigned int cnt):index(index),cnt(cnt){}
    QList<QString> name_list;
};
/*用于pure_aloha 协议中，发送到text_view的条目 */
class DataItem{
public:
    unsigned int time;
    QString  user_id;
    QList<UserInfo *> collusion_list;
    bool  st;
    DataItem(unsigned int time,QString user_id,bool st):time(time),user_id(user_id),st(st){}
    DataItem(unsigned int time,QString user_id,bool st,QList<UserInfo *> &coll_list):time(time),user_id(user_id),st(st)
    {
        foreach(UserInfo *p_info,coll_list)
        {
            if(p_info)
            {
                UserInfo *p=new UserInfo(p_info->current_time,p_info->user_id);
                collusion_list.append(p);
            }
        }
    }
};
class SlotData{
public:
    QList<QString> list;
    unsigned int time;
};

class Channel:public QObject
{
 Q_OBJECT
public:
    enum  status{
        STOP=0,
        RUN=1,
        BREAK
    };
public:
    Channel(MainWindow * ,QObject *parent =nullptr);

    ~Channel()
    {

    }
    static unsigned  int frame_total_cnt;       //信道仿真期间有效帧数目，有效帧值得是传输成功并未发生碰撞的帧
    static unsigned  int slot_cnt;
    static unsigned  int ab_time;         //信道仿真持续的绝对时间

    static  status run_flg;
    unsigned  int frame_time;             //在信道中传输一帧所需时间 值为 frame_len/bit_rate
    unsigned  int bit_rate;               //比特率
    unsigned  int work_usr_cnt=0;           //处于发送状态的用户结点数量
    bool en_stop_btn;
    static double next_time(double lambda);
    unsigned int barrier=0;
    int steps;

    MainWindow *p_main;

    void * run_pure(void);
    void run_slot(void);

    void delay_msec(unsigned int msec);
    QTimer * get_QTimer();
    MultiAcPro * get_protocal();
    static unsigned int getAb_time();
    static void setAb_time(unsigned int value);
    QMutex locker;
    void init_channel();
private:
    unsigned int usr_current_cnt;
    QTimer  * timer;
    QMap <QString,UserNode *> user_idle_map;
    QList <UserNode *> user_work_list;
    QList <Item *> slot_list;
    MultiAcPro * protocal;
    QString user_id;
public slots:
    //void on_time_out();
    void send_over();
    void relay();
signals:
    void text_message(DataItem *);
    void call_send_over(unsigned int);
    void over_box_message();
};





#endif // CHANNEL_H
