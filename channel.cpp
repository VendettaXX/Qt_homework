#include "channel.h"

Channel::Channel(QObject *parent):QObject(parent)
{
    work_usr_cnt=0;
    frame_time=100;
    en_stop_btn=false;
    setAb_time(0);
    for(int i=0;i<USERNUM;i++)
    {
        QString name="USER"+QString("%1").arg(i,4,10,QLatin1Char('0'));
        UserNode  * NAME(i)=new UserNode(name);
        qDebug()<<NAME(i)->name<<endl;
        user_idle_map.insert(NAME(i)->name,NAME(i));
    }
    QMap<QString,UserNode*>::iterator iter=user_idle_map.begin();
    while(iter!=user_idle_map.end())
    {
        qDebug()<<"Iterator"<<iter.key()<<":"<<iter.value();
        iter++;
    }
}

double Channel::next_time(double lambda)
{
    QTime time;
    time=QTime::currentTime();
    qsrand(time.msec()+time.second()*1000);
    double x=0.0;
    double pv;
    while((x=qrand()%100/100.0-0)==0.0);
    pv=(-1/lambda)*qLn(1-x);
    qDebug()<<"x="<<x<<"pv="<<pv<<endl;
    return pv;
}

unsigned int Channel::getAb_time()
{
    return ab_time;
}

void Channel::setAb_time(unsigned int value)
{
    ab_time = value;
}


void Channel::run_pure()
{
    while(true)
    {
        while(RUN==run_flg)
        {
            double n_t;
            unsigned int i=0;
            n_t=Channel::next_time(LAMBDA);

            qDebug()<<__func__<<__LINE__<<endl;
            Channel::setAb_time(Channel::getAb_time()+static_cast<unsigned int>(n_t*1000));

            delay_msec(static_cast<int>(n_t*1000));

            QString index="USER "+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
            user_id=index;

            locker.lock();

            /*从IDLE 队列map里面把随机到的work_user 放入普通队列中 */
            QMap<QString,UserNode *>::iterator _iter=user_idle_map.end(); //_iter是即将到来的新帧

            while((_iter=user_idle_map.find(index))==user_idle_map.end()) //如果在idle里面找不到随机到的user id
            {
                index="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
            }
            _iter.value()->frame_begin_time=Channel::ab_time;
            /*如果此时，工作队列为空，则st任然为false，如果此时工作队列为非空，则所有工作队列中的usernode的st变成false，直到过了100ms
            之后的send_over再改会TRUE，还需要再工作队列中的所有元素里面的 collusion队列加上自己，也需要在自己的collusion队列里加上对方
            的信息*/
            if(!user_work_list.isEmpty())
            {
                for(QList<UserNode *>::iterator iter=user_work_list.begin();iter!=user_work_list.end();iter++)
                {
                    (*iter)->st=false;
                    UserInfo * p_info=new UserInfo(_iter.value()->frame_begin_time,index);
                    UserInfo * p_info_old=new UserInfo((*iter)->frame_begin_time,(*iter)->name);
                    (*iter)->collusion_list.append(p_info);
                    _iter.value()->collusion_list.append(p_info_old);
                }

                _iter.value()->st=false;   //别忘了，给自己的usernode中的st打上false
            }
            user_work_list.append(_iter.value());

            user_idle_map.erase(_iter);
            QTimer::singleShot(100,this,SLOT(send_over()));
            work_usr_cnt++;

            locker.unlock();
        }
        if(BREAK==run_flg && en_stop_btn==true )
        {
            qDebug()<<"2222222222222222222222222222222222"<<endl;
            en_stop_btn=false;
            emit(over_box_message());
        }
    }
    qDebug()<<"共消耗"<<Channel::getAb_time()+100<<"时间"<<endl;
    qDebug()<<"总共有"<<Channel::frame_total_cnt<<"有效帧"<<"发送"<<endl;
    qDebug()<<"单位时间的吞吐量为"<<(Channel::frame_total_cnt*100)/(Channel::getAb_time()*1.0)<<endl;
}
void Channel::run_slot()
{
    unsigned int ab_time=0;
    unsigned int n_t,barrier;
    unsigned int a=0;
    n_t=static_cast<unsigned int>(next_time(LAMBDA)*1000);
    ab_time+=n_t;
    while(RUN==run_flg)
    {
        unsigned int i=0;
        if(ab_time%100!=0){
            barrier=(ab_time/100+1)*100;
        }
        else {
            barrier=ab_time;
        }
        i=1;
        while((ab_time+=(static_cast<unsigned int>(next_time(LAMBDA)*1000)))<barrier){
            i++;
        }
        slot_list.append(new Item(barrier/100,i));
        delay_msec(10);
    }
    for(QList<Item *>::iterator iter=slot_list.begin();iter!=slot_list.end();iter++)
    {
        //qDebug()<<(*iter)->cnt<<endl;
        if((*iter)->cnt==1)
            a++;
    }
    qDebug()<<a<<endl;
    qDebug()<<slot_list.last()->index<<endl;
    qDebug()<<a/(double)slot_list.last()->index<<endl;
}

void Channel::delay_msec(int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


//void Channel::on_time_out()
//{
//    //locker.lock();

//    slot_cnt++;


//    //locker.unlock();

//}

void Channel::send_over()  //QTimer oneshot 从结点发送帧开始 定时结束
{
    UserNode *temp;
   // qDebug()<<__FUNCTION__<<__LINE__<<time<<endl;
    //if(RUN==run_flg)
    {
        locker.lock();
        temp=user_work_list.takeFirst();//取出user_work_list中的 usrnode
        bool state=temp->st;
        user_idle_map.insert(temp->name,temp);
        work_usr_cnt--;
        //qDebug()<<"同时有"<<work_usr_cnt<<"在发送中";
        //qDebug()<<temp->st<<endl;
        if(true==state){
            Channel::frame_total_cnt++;
            //qDebug()<<"frame_total_cnt="<<frame_total_cnt<<endl;
        }
        //DataItem * data_item=new DataItem(temp->frame_begin_time,temp->name,temp->st);
        DataItem * data_item=new DataItem(temp->frame_begin_time,temp->name,temp->st,temp->collusion_list);
        emit(text_message(data_item));
        temp->st=true;    //user_work_list中取出的usernode 中的状态改为true，因为此时此结点已经处于空闲状态
        foreach(UserInfo *p_info,temp->collusion_list)
        {
            if(p_info)
            {
                temp->collusion_list.removeOne(p_info);
                delete p_info;
                p_info=nullptr;
            }
        }
        locker.unlock();
    }
}

void Channel::relay()
{
    emit call_send_over(Channel::ab_time);
}
unsigned  int Channel::frame_total_cnt=0;  //信道开启期间，发送的帧的总数目
unsigned  int Channel::frame_len=200;      //取1200，因为frame_time=1200*8b/9600bps=1s
unsigned  int Channel::slot_cnt=0;         //时间轴上时隙点的数目
unsigned  int Channel::ab_time=0;          //信道持续的时间
//status      Channel::run_flg=Channel::STOP;
Channel::status  Channel::run_flg=Channel::STOP;
Channel::status  Channel::pre_run_flg=Channel::STOP;
