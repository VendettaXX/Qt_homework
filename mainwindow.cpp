#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui_init();
    QTimer  *timer=new QTimer(this);
    timer->setInterval(1000);
    timer->start();
    model=new QStandardItemModel(this);
    init_table(model);
    Channel * work_thread=new Channel(this);// 必须new，如果是Channel c，再在connect里面用&c，不知道为什么，槽函数不能得到执行
    p_channel=work_thread;
    thread=new QThread(this);
    work_thread->moveToThread(thread);
    thread->start();
    /*channel 发送message信号给GUI线程的显示数据 */
    connect(work_thread,SIGNAL(text_message(DataItem *)),this,SLOT(display_mesg(DataItem *)));
    /*pause_resume_btn 按钮 点击事件 绑定 GUI线程 pause_resume*/
    connect(ui->pause_resume_btn,SIGNAL(clicked()),this,SLOT(pause_resume()));
    /*按下STOP按钮,发送stop信息 */
    connect(ui->stop_button,SIGNAL(clicked()),this,SLOT(stop()));
    /*channel发送over_box_message,绑定GUI线程弹出窗口 */
    connect(work_thread,SIGNAL(over_box_message()),this,SLOT(display_result()));
    connect(thread,&QThread::started,[=](){work_thread->run();});
    connect(ui->confirm_protocal,SIGNAL(clicked()),this,SLOT(confirm_protocal()));
    connect(ui->select_protocal,SIGNAL(activated(int)),this,SLOT(select_protocal_activited(int)));
//#ifdef PURE
//    connect(thread,&QThread::started,[=](){work_thread->run_pure();});
//#else
//    connect(thread,&QThread::started,[=](){work_thread->run_slot();});
//#endif
    connect(timer,SIGNAL(timeout()),this,SLOT(time_out()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_table(QStandardItemModel * model)
{
    this->table_view_status=true;
    model->setColumnCount(5);
    model->setHeaderData(0,Qt::Horizontal, "时间");
    model->setHeaderData(1,Qt::Horizontal, "节点ID");
    model->setHeaderData(2,Qt::Horizontal, "碰撞情况");
    model->setHeaderData(3,Qt::Horizontal, "帧是否有效");
    model->setHeaderData(4,Qt::Horizontal, "发送有效帧数目");


//    model->item(0,0)->setTextAlignment(Qt::AlignCenter);
//    model->item(0,1)->setTextAlignment(Qt::AlignCenter);
//    model->item(0,2)->setTextAlignment(Qt::AlignCenter);
    QFont font = ui->table_view->horizontalHeader()->font();
    font.setBold(true);
    ui->table_view->setModel(model);
    ui->table_view->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void MainWindow::ui_init()
{
//    ui->pause_resume_btn->setDisabled(true);
    //    ui->stop_button->setDisabled(true);
}

void MainWindow::confirm_protocal()
{
    if(ui->confirm_protocal->text()=="确认")
    {
        if(ui->select_protocal->currentText()=="Pure_aloha")
        {
            p_channel->proto_flg="pure";
        }
        else {
            p_channel->proto_flg="slotted";
        }
        ui->confirm_protocal->setText("更改协议");
        ui->select_protocal->setDisabled(true);
    }
    else{
        ui->select_protocal->setEnabled(true);
        if(ui->select_protocal->currentText()=="Pure_aloha")
        {
            p_channel->proto_flg="pure";
        }
        else {
            p_channel->proto_flg="slotted";
        }
    }
}

void MainWindow::select_protocal_activited(int)
{
    qDebug()<<__func__<<__LINE__<<endl;
    ui->confirm_protocal->setText("确认");
}

void MainWindow::time_out()
{
   static int value=0;
   if(Channel::RUN!=Channel::run_flg){
       //qDebug()<<"value is %d"<<value++<<endl;
   }
}

void MainWindow::display_mesg(DataItem * data_item)
{
    //#ifdef PURE
    if("pure"==p_channel->proto_flg)
    {
        qDebug()<<__func__<<__LINE__<<"table_view_status="<<table_view_status<<endl;
        /*table_view_status 表征此时是暂停还是停止，num是否清零*/
        if(table_view_status==true)
        {
            qDebug()<<__func__<<"num="<<num<<endl;
            foreach(UserInfo * p,data_item->collusion_list)
            {
                qDebug()<<"p->user_id="<<p->user_id<<"p->current_time="<<p->current_time<<endl;
            }
            qDebug()<<__func__<<__LINE__<<endl;
            model->setItem(num,0,new QStandardItem(QString::number(data_item->time,10)+"ms"));
            model->setItem(num,1,new QStandardItem(data_item->user_id));
            if(data_item->st==true)
            {
                model->setItem(num,2,new QStandardItem("非碰撞"));
                model->setItem(num,3,new QStandardItem("有效"));
                model->item(num,0)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,1)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,2)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,3)->setBackground(QBrush(QColor(0,255,0)));
                //model->item(num,4)->setBackground(QBrush(QColor(0,255,0)));
                //model->item(0)->setBackground(Qt::BrushStyle(QRgb(qRed(20))));
            }
            else{
                QString s;
                foreach(UserInfo * p,data_item->collusion_list)
                {
                    s=s+"本帧与"+QString::number(p->current_time,10)+p->user_id+"碰撞"+"/";
                }
                model->setItem(num,2,new QStandardItem(s));
                model->setItem(num,3,new QStandardItem("无效"));
                model->item(num,0)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,1)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,2)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,3)->setBackground(QBrush(QColor(252,230,202)));
                //model->item(num,4)->setBackground(QBrush(QColor(128,128,0)));
            }
            model->setItem(num,4,new QStandardItem(QString::number(Channel::frame_total_cnt,10)));


            num++;
            ui->table_view->scrollToBottom();
        }
        else{
            num=0;
            table_view_status=true;
        }
    }
    else
    {
           /*table_view_status 表征此时是暂停还是停止，num是否清零*/
        qDebug()<<"table_view_status="<<table_view_status<<endl;
        if(table_view_status==true)
        {
            QString user_id;
            model->setItem(num,0,new QStandardItem(QString::number(data_item->time,10)+"ms"));
            if(!data_item->collusion_list.isEmpty())
            {
                qDebug()<<"collusion_list.size="<<data_item->collusion_list.size()<<endl;
                for(int i=0;i<data_item->collusion_list.size();i++)
                {
                    user_id+=(data_item->collusion_list.at(i)->user_id+'\t');
                }
            }
            //model->setItem(num,0,new QStandardItem(QString::number(data_item->collusion_list,10)+"ms"));
            model->setItem(num,1,new QStandardItem(user_id));
            if(data_item->collusion_list.size()==1)
            {
                model->setItem(num,2,new QStandardItem("非碰撞"));
                model->setItem(num,3,new QStandardItem("有效"));
                model->item(num,0)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,1)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,2)->setBackground(QBrush(QColor(0,255,0)));
                model->item(num,3)->setBackground(QBrush(QColor(0,255,0)));
                //model->item(num,4)->setBackground(QBrush(QColor(0,255,0)));
                //model->item(0)->setBackground(Qt::BrushStyle(QRgb(qRed(20))));
            }
            else{
                QString s;
                model->setItem(num,2,new QStandardItem(s));
                model->setItem(num,3,new QStandardItem("无效"));
                model->item(num,0)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,1)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,2)->setBackground(QBrush(QColor(252,230,202)));
                model->item(num,3)->setBackground(QBrush(QColor(252,230,202)));
                //model->item(num,4)->setBackground(QBrush(QColor(128,128,0)));
            }
            model->setItem(num,4,new QStandardItem(QString::number(Channel::frame_total_cnt,10)));


            num++;
            ui->table_view->scrollToBottom();
        }
        else{
            num=0;
            table_view_status=true;
        }
    }
}

void MainWindow::pause_resume()
{
    p_channel->locker.lock();
    if(ui->pause_resume_btn->text()=="PAUSE")
    {
        p_channel->run_flg=Channel::STOP;
        ui->pause_resume_btn->setText("RESUME");
    }
    else if (ui->pause_resume_btn->text()=="START"){
        p_channel->init_channel();
        p_channel->steps=(ui->steps_line->currentText().toInt());
        //table_view_status=false;
        table_view_status=true;
        model->removeRows(0,model->rowCount());
        p_channel->run_flg=Channel::RUN;
        ui->pause_resume_btn->setText("PAUSE");
        qDebug()<<__func__<<__LINE__<<endl;
    }
    else{
        p_channel->steps=ui->steps_line->currentText().toInt();
        p_channel->run_flg=Channel::RUN;
        ui->pause_resume_btn->setText("PAUSE");
    }
    p_channel->locker.unlock();
}

void MainWindow::display_result()
{
   QString s;
   s="本次仿真参数:帧时为"+QString::number(p_channel->frame_time,10)+"ms" +"本次仿真共耗时"+QString::number(p_channel->ab_time/1000.0,'f',2)+"s"+'\n'
           +"其中有效帧的个数为"\
           +QString::number(p_channel->frame_total_cnt,10)+'\n'\
           +"每帧时的吞吐量S为"\
           +QString::number(p_channel->frame_total_cnt*100.0/p_channel->ab_time,'f',2);

   //QMessageBox::about(nullptr,"Title",s);
   iw=InfoWindow::get_instance(this);
   iw->ui->frame_time_lable->setText(QString::number(p_channel->frame_time,10));
   iw->ui->lambda_lable->setText(QString::number(10,10));
   iw->ui->G_lable->setText(QString::number(10*p_channel->frame_time/1000,10));
   iw->ui->S_lable->setText(QString::number(p_channel->frame_total_cnt*100.0/p_channel->ab_time,'f',2));
   iw->ui->ab_time_lable->setText(QString::number(p_channel->ab_time,10)+"ms");
   iw->ui->total_valid_cnt_lable->setText(QString::number(p_channel->frame_total_cnt,10));



   iw->setWindowModality(Qt::ApplicationModal);  //阻塞除当前窗体之外的所有窗体
   iw->show();
   //setWindowFlags(Qt::WindowStaysOnTopHint);
}

void MainWindow::stop()
{
    p_channel->locker.lock();
    p_channel->en_stop_btn=true;
    p_channel->run_flg=Channel::BREAK;
    num=0;
    ui->pause_resume_btn->setText("START");
    p_channel->locker.unlock();

}

