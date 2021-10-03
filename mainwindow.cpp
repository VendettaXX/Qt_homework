#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model=new QStandardItemModel(this);
    init_table(model);
    Channel * work_thread=new Channel;// 必须new，如果是Channel c，再在connect里面用&c，不知道为什么，槽函数不能得到执行
    p_channel=work_thread;
    thread=new QThread(this);
    work_thread->moveToThread(thread);
    thread->start();
    //connect(ui->stop_button,&QPushButton::clicked,[=](){Channel::run_flg=Channel::STOP;});
    connect(work_thread,SIGNAL(text_message(DataItem *)),this,SLOT(display_mesg(DataItem *)));
    connect(ui->pause_resume_btn,SIGNAL(clicked()),this,SLOT(pause_resume()));
    connect(ui->stop_button,SIGNAL(clicked()),this,SLOT(stop()));
    connect(work_thread,SIGNAL(over_box_message()),this,SLOT(display_result()));
#ifdef PURE
    connect(thread,&QThread::started,[=](){work_thread->run_pure();});
#else
    connect(thread,&QThread::started,[=](){work_thread->run_slot();});
#endif
    //connect(thread,&QThread::finished,thread,&QObject::deleteLater);
    // connect(thread,SIGNAL(finished),work_thread,SLOT(deleteLater()));
    //connect(thread,SIGNAL(finished),thread,SLOT(deleteLater()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_table(QStandardItemModel * model)
{
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

//void MainWindow::on_timer_out()
//{
//   static int value=0;
//   qDebug()<<"value is %d"<<value++<<endl;
//}

void MainWindow::display_mesg(DataItem * data_item)
{
    foreach(UserInfo * p,data_item->collusion_list)
    {
        qDebug()<<"p->user_id="<<p->user_id<<"p->current_time="<<p->current_time<<endl;
    }
     qDebug()<<__func__<<__LINE__<<endl;
    static int num=0;
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

            s=s+"本帧与"+QString::number(p->current_time,10)+p->user_id+"碰撞"+"\n";

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



    //delete(data_item);
    //data_item=nullptr;
}

void MainWindow::pause_resume()
{
     p_channel->locker.lock();
     if(ui->pause_resume_btn->text()=="PAUSE")
     {
         p_channel->run_flg=Channel::STOP;
         ui->pause_resume_btn->setText("RESUME");
     }
     else{
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
   QMessageBox::about(nullptr,"Title",s);
}

void MainWindow::stop()
{
    p_channel->locker.lock();
    p_channel->en_stop_btn=true;
    p_channel->run_flg=Channel::BREAK;
    ui->pause_resume_btn->setText("Start");
    p_channel->locker.unlock();
}
