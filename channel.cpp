#include "channel.h"

Channel::Channel(QObject *parent):QObject(parent)
{
    init_channel();
    for(int i=0;i<USERNUM;i++)
    {
        QString name="USER"+QString("%1").arg(i,4,10,QLatin1Char('0'));
        UserNode  * NAME(i)=new UserNode(name);
        //qDebug()<<NAME(i)->name<<endl;
        user_idle_map.insert(NAME(i)->name,NAME(i));
    }
    QMap<QString,UserNode*>::iterator iter=user_idle_map.begin();
    while(iter!=user_idle_map.end())
    {
        //qDebug()<<"Iterator"<<iter.key()<<":"<<iter.value();
        iter++;
    }
}

double Channel::next_time(double lambda)
{
    QTime time;
    time=QTime::currentTime();
    qsrand(static_cast<unsigned int>(time.msec()+time.second()*1000));
    double x=0.0;
    double pv;
    while((x=qrand()%100/100.0-0)==0.0);
    pv=(-1/lambda)*qLn(1-x);
   // qDebug()<<"x="<<x<<"pv="<<pv<<endl;
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

void Channel::init_channel()
{
    work_usr_cnt=0;
    frame_time=100;
    frame_total_cnt=0;
    en_stop_btn=false;
    setAb_time(0);
}


void * Channel::run_pure()
{
    while(true)  //可能会和send_over 冲突，所以加锁
    {
        while(RUN==run_flg)
        {
            double n_t;
            unsigned int i=0;
            n_t=Channel::next_time(LAMBDA);

            //qDebug()<<__func__<<__LINE__<<endl;
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
            locker.lock();
            en_stop_btn=false;
            emit(over_box_message());
            locker.unlock();
        }
    }
//    qDebug()<<"共消耗"<<Channel::getAb_time()+100<<"时间"<<endl;
//    qDebug()<<"总共有"<<Channel::frame_total_cnt<<"有效帧"<<"发送"<<endl;
//    qDebug()<<"单位时间的吞吐量为"<<(Channel::frame_total_cnt*100)/(Channel::getAb_time()*1.0)<<endl;
    return static_cast<void *>(nullptr);
}
void Channel::run_slot()
{
    //不会和任何其他线程或者定时器冲突，所以不用加锁
    unsigned int ab_time=0;
    unsigned int n_t,barrier;
    unsigned int a=0;
    DataItem * data_item=new DataItem(0,"default",true);
    QList<QString> work_list;
    Item * item=new Item(0,1);
    QString user_name;
    n_t=static_cast<unsigned int>(next_time(LAMBDA)*1000);

    ab_time+=n_t;

    unsigned int temp;
    temp=(ab_time%100==0)?ab_time:(ab_time/100+1)*100;
    delay_msec(temp);
    qDebug()<<temp<<endl;
    while(true)
    {
        while(RUN==run_flg)
        {
            //清空work_list
            if(!work_list.isEmpty())
                work_list.clear();
            if(!data_item->collusion_list.isEmpty())
                data_item->collusion_list.clear();
            unsigned int i=0;
            if(ab_time%100!=0){
                barrier=(ab_time/100+1)*100;
            }
            else {
                barrier=ab_time;
            }
            // Item * item=new Item(barrier/100,1);
            item->index=barrier/100;
            item->cnt=1;
            user_name="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
            qDebug()<<"user_name="<<user_name<<endl;
            data_item->collusion_list.push_back(new UserInfo(ab_time,user_name));
            while((ab_time+=(static_cast<unsigned int>(next_time(LAMBDA)*1000)))<barrier)
            {
                //每次next_time 到的时候随机选取一个userid，如果选取的userid已经在work_list里面则再次随机一个名字
                //                while(true)
                //                {
                //                    user_name="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                //                    QList<QString>::iterator iter;
                //                    if(!work_list.isEmpty())
                //                    {

                //                        for(iter=work_list.begin();iter!=work_list.end();iter++)
                //                        {
                //                            if((*iter)==user_name)
                //                                break;
                //                        }
                //                    }
                //                    else
                //                    {
                //                        work_list.append(user_name);
                //                    }
                //                    if(iter!=work_list.end())
                //                    {
                //                        work_list.append(user_name);
                //                        break;
                //                    }
                //                    data_item->collusion_list.push_back(new UserInfo(ab_time,user_name));
                //                }



                user_name="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                qDebug()<<"user_name="<<user_name<<endl;
                data_item->collusion_list.push_back(new UserInfo(ab_time,user_name));
                item->cnt++;
                delay_msec(qrand()%30);
            }


            data_item->time=barrier;
            qDebug()<<"barrier="<<barrier<<endl;
            emit(text_message(data_item));
            //QString id_name="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
            slot_list.append(item);
            if (slot_list.at(slot_list.size()-1)->cnt==1)
                frame_total_cnt++;
            //slot_list.at(slot_list.size()-1)->name_list.push_back(id_name);
            temp=(ab_time%100==0)?ab_time:(ab_time/100+1)*100;
            delay_msec(temp-barrier);
            qDebug()<<"temp-barrier"<<temp-barrier<<"/t"<<"i="<<item->cnt<<endl;


        }
//        for(QList<Item *>::iterator iter=slot_list.begin();iter!=slot_list.end();iter++)
//        {
//            if((*iter)->cnt==1){
//            	frame_total_cnt++;
//                a++;
//            }
//        }
        if(BREAK==run_flg && en_stop_btn==true )
        {
            locker.lock();
            en_stop_btn=false;
            //qDebug()<<"a="<<a<<endl;
            qDebug()<<"frame_total_cnt="<<frame_total_cnt<<endl;
            //qDebug()<<"总共"<<slot_list.last()->index<<"帧"<<endl;
            qDebug()<<"总共"<<item->index<<"帧"<<endl;
            //qDebug()<<a*1.0/slot_list.last()->index<<endl;
            //qDebug()<<frame_total_cnt*1.0/slot_list.last()->index<<endl;
            qDebug()<<frame_total_cnt*1.0/item->index<<endl;
            emit(over_box_message());
            locker.unlock();
        }
    }
}

void Channel::delay_msec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


void Channel::send_over()  //QTimer oneshot 从结点发送帧开始 定时结束
{
    UserNode *temp;
    //if(RUN==run_flg)
    {
        locker.lock();
        temp=user_work_list.takeFirst();//取出user_work_list中的 usrnode
        bool state=temp->st;
        user_idle_map.insert(temp->name,temp);
        work_usr_cnt--;
        if(true==state){
            Channel::frame_total_cnt++;
        }
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
unsigned  int Channel::slot_cnt=0;         //时间轴上时隙点的数目
unsigned  int Channel::ab_time=0;          //信道持续的时间
Channel::status  Channel::run_flg=Channel::STOP;
