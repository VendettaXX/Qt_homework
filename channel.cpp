#include "channel.h"
#include "mainwindow.h"

Channel::Channel(MainWindow * p,QObject *parent):QObject(parent)
{
    init_channel();
    p_main=p;
    for(int i=0;i<USERNUM;i++)
    {
        QString name="USER"+QString("%1").arg(i,4,10,QLatin1Char('0'));
        UserNode  * NAME(i)=new UserNode(name);
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
    barrier=0;
    en_stop_btn=false;
    setAb_time(static_cast<unsigned int>(next_time(LAMBDA)*1000));
}


void * Channel::run()
{
    while(true)
    {
        if("pure"==proto_flg)
        {
            while(true)  //可能会和send_over 冲突，所以加锁
            {
                double n_t;
                if("slotted"==proto_flg)
                {
                    break;
                }
                n_t=Channel::next_time(LAMBDA);
                if(Channel::getAb_time()==0)   //如果现在的ab_time为0的话，说明信道刚开始
                    Channel::setAb_time(static_cast<unsigned int>(n_t*1000));// 防止初始化的时候 n_t已经赋值过一次了
                UserNode * pre_info= new UserNode("default");
                pre_info->frame_begin_time=-100;

                while(RUN==run_flg)
                {
                    unsigned int i=0;
                    unsigned int current_time;
                    QMap<QString,UserNode *>::iterator _iter=user_idle_map.end(); //_iter是ab_time 所对应的帧
                    if("slotted"==proto_flg)
                    {
                        break;
                    }
                    if(steps<=0)
                    {
                        p_main->ui->pause_resume_btn->setText("RESUME");
                        run_flg=STOP;
                        break;
                    }
                    steps--;

                    // 1.如果 work_list为空，延时到ab_time+100ms（1号位置）的地方，循环计算n_t 推进ab_time超越 延时的1号结点，所有在1号位置之前又没有结束的结点，都append到work_list队列里.延时到的结点说明要结束了，
                   // 从work_list结点里面剔除该结点
                    //2.如果work_list不为空，延时到队列头的frame_begin_time+100ms(位置2)的位置，观察ab_time 和位置2的关系，如果ab_time位于位置2之前，则继续循环计算n_t 推进ab_time超越 位置2，所有在位置2之前有没有结束
                    //的结点，都append到work_list队列里，延时到的结点从work_list里面剔除。如果ab_time位于位置2之后，则继续轮回这个过程。
                    if(!user_work_list.isEmpty())
                    {
                        unsigned int second_position=user_work_list.first()->frame_begin_time+100;
                        delay_msec(user_work_list.first()->frame_begin_time-pre_info->frame_begin_time);
                        /* ab_time 小于work_list队列头的frame_begin_time+100  */
                        while(ab_time<second_position)
                        {

                            QString index="USER "+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                            while((_iter=user_idle_map.find(index))==user_idle_map.end()) //如果在idle里面找不到随机到的user id,继续找
                            {
                                index="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                            }
                            _iter.value()->frame_begin_time=ab_time;
                            _iter.value()->st=false;
                            /* 所有work_list中的结点状态st 改为false，表明为碰撞，再在这些结点的collusion_list中标记上自己的信息 */
                            for(QList<UserNode *>::iterator iter=user_work_list.begin();iter!=user_work_list.end();iter++)
                            {
                                (*iter)->st=false;  //标记该帧为冲突
                                UserInfo * p_info=new UserInfo(_iter.value()->frame_begin_time,index);
                                UserInfo * p_info_old=new UserInfo((*iter)->frame_begin_time,(*iter)->name);//将所有work_list的成员信息添加到该帧的collusion_list队列里面
                                (*iter)->collusion_list.append(p_info);
                                _iter.value()->collusion_list.append(p_info_old);
                            }

                            /* 然后把该ab_time对应的结点从idle map里面剔除，加入到work_list */
                            user_work_list.append(_iter.value());
                            user_idle_map.erase(_iter);
                            work_usr_cnt++;
                            ab_time+=(static_cast<unsigned int>(next_time(LAMBDA)*1000));
                        }
                        /* 把队列头那位的信息发送给主窗口的table_view的显示槽函数*/
                        /* 把队列头的那位，st 清空即置为true，collusion_list 清空，从work_list里面清除，返回到idle map 里面*/
                        /* 把队列头的那位的开始时间信息即frame_begin_time 存放到pre_info中 */
                        DataItem *data_item=new DataItem(_iter.value()->frame_begin_time,_iter.value()->name,_iter.value()->st,_iter.value()->collusion_list);
                        emit(text_message(data_item));

                        _iter.value()->st=true;
                        foreach(UserInfo *info,_iter.value()->collusion_list)
                        {
                            if(info)
                            {
                                _iter.value()->collusion_list.removeOne(info);
                                delete info;
                                info=nullptr;
                            }
                        }
                        pre_info->frame_begin_time=_iter.value()->frame_begin_time;
                        pre_info->name=_iter.value()->name;


                    }
                    else    /*如果work_list 为空，则延时到ab_time+100 ,现在处于的时间点是pre_info的时间点，pre_info记录的是上次结束的结点，此结点现在处于idlemap里面*/
                    {
                        int first_position=ab_time+100;
                        delay_msec(static_cast<int>(ab_time)-pre_info->frame_begin_time);
                        /*ab_time结点加入work_list ,从idle map里面 剔除 */
                        QString index="USER "+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                        while((_iter=user_idle_map.find(index))==user_idle_map.end()) //如果在idle里面找不到随机到的user id,继续找
                        {
                            index="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                        }
                        _iter.value()->frame_begin_time=ab_time;


                    }









                    delay_msec(static_cast<int>(n_t*1000));

                    QString index="USER "+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                    user_id=index;

                    locker.lock();

                    /*从IDLE 队列map里面把随机到的work_user 放入普通队列中 */
                    QMap<QString,UserNode *>::iterator _iter=user_idle_map.end(); //_iter是即将到来的新帧

                    while((_iter=user_idle_map.find(index))==user_idle_map.end()) //如果在idle里面找不到随机到的user id,继续找
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
                            (*iter)->st=false;   //标记该帧为冲突，send_over的时候，用这个来标记自己是否有效
                            UserInfo * p_info=new UserInfo(_iter.value()->frame_begin_time,index); //将所有work_list队列里面的所有成员的collusion_list添加上该帧的信息

                            UserInfo * p_info_old=new UserInfo((*iter)->frame_begin_time,(*iter)->name);//将所有work_list的成员信息添加到该帧的collusion_list队列里面
                            (*iter)->collusion_list.append(p_info);
                            _iter.value()->collusion_list.append(p_info_old);
                        }

                        _iter.value()->st=false;   //别忘了，给自己的usernode中的st打上false
                    }
                    user_work_list.append(_iter.value());
                    user_idle_map.erase(_iter);
                    QTimer::singleShot(100,this,SLOT(send_over()));
                    qDebug()<<__func__<<__LINE__<<endl;
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
                delay_msec(1);  //需要加这个，因为当处于STOP状态时候，此函数空转没有任何让出线程的语句，只好加这个好让channel的线程让出时间让send_over执行
            }
        }
        else
        {
            unsigned int n_t;
            DataItem * data_item=new DataItem(0,"default",true);
            QList<QString> work_list;
            Item * item=new Item(0,1);
            QString user_name;
            n_t=static_cast<unsigned int>(next_time(LAMBDA)*1000);

            ab_time+=n_t;
            qDebug()<<__FUNCTION__<<__LINE__<<"ab_time="<<ab_time<<endl;

            unsigned int temp;
            temp=(ab_time%100==0)?ab_time:(ab_time/100+1)*100;
            delay_msec(temp);
            qDebug()<<temp<<endl;
            while(true)
            {
                if("pure"==proto_flg)
                {
                    break;
                }
                while(RUN==run_flg )
                {
                    if("pure"==proto_flg)
                    {
                        //init_channel();
                        break;
                    }

                    if(steps<=0)
                    {
                        p_main->ui->pause_resume_btn->setText("RESUME");
                        run_flg=STOP;
                        break;
                    }
                    steps--;

                    qDebug()<<__FUNCTION__<<__LINE__<<"ab_time="<<ab_time<<endl;
                    //清空work_list
                    if(!work_list.isEmpty())
                        work_list.clear();
                    if(!data_item->collusion_list.isEmpty())
                        data_item->collusion_list.clear();
                    qDebug()<<__FUNCTION__<<__LINE__<<"ab_time="<<ab_time<<endl;
                    //ab_time=static_cast<unsigned int>(next_time(LAMBDA)*1000);
                    if(ab_time%100!=0){
                        barrier=(ab_time/100+1)*100;
                    }
                    else {
                        barrier=ab_time;
                    }
                    // Item * item=new Item(barrier/100,1);
                    item->index=barrier/100;
                    qDebug()<<"barrier"<<barrier<<endl;
                    item->cnt=1;
                    user_name="USER"+QString("%1").arg(qrand()%USERNUM,4,10,QLatin1Char('0'));
                    qDebug()<<"user_name="<<user_name<<endl;
                    data_item->collusion_list.push_back(new UserInfo(ab_time,user_name));
                    while((ab_time+=(static_cast<unsigned int>(next_time(LAMBDA)*1000)))<barrier)
                    {
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
    }
    return static_cast<void *>(nullptr);
}
void  Channel::delay_msec(unsigned int msec)
{
    QEventLoop loop;//定义一个新的事件循环
    QTimer::singleShot(msec, &loop, SLOT(quit()));//创建单次定时器，槽函数为事件循环的退出函数
    loop.exec();//事件循环开始执行，程序会卡在这里，直到定时时间到，本循环被退出
}


void Channel::send_over()  //QTimer oneshot 从结点发送帧开始 定时结束
{
    UserNode *temp;
    qDebug()<<__func__<<__LINE__<<endl;
    //if(RUN==run_flg)   存在prue_aloha 执行到定时器已经开启了，但是run_flg
    // 改变了，这个定时器还是要完成的
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
        //if(RUN==run_flg)
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
Channel::status  Channel::run_flg=Channel::STOP;  //初始状态为STOP
