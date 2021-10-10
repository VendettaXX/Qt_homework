#include "infowindow.h"
#include "mainwindow.h"
#include "ui_infowindow.h"

InfoWindow::InfoWindow(MainWindow * p,QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::InfoWindow)
{
    ui->setupUi(this);
    p_main=p;
    /* 保存按钮关联函数*/
    connect(ui->save_btn,SIGNAL(clicked()),this,SLOT(save_btn_clicked()));
}

void InfoWindow::save_btn_clicked()
{
    QString text_data;
    unsigned int rows,colums;
    QString fileName = QFileDialog::getSaveFileName(this, tr("csv file"), qApp->applicationDirPath (),
                                                    tr("Csv Files (*.csv)"));
    if (fileName.isEmpty())
        return;
    rows=static_cast<unsigned int>(p_main->model->rowCount());
    colums=static_cast<unsigned int>(p_main->model->columnCount());
    for(unsigned int i=0;i<rows;i++)
    {
        for(unsigned int j=0;j<colums;j++)
        {
            text_data += p_main->model->data(p_main->model->index(i,j)).toString();
            text_data+=",";
        }
        text_data+="\n";
    }
    qDebug()<<"text_data"<<text_data<<endl;
    QFile csv_file(fileName);
    if(csv_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QTextStream out(&csv_file);
        out << text_data;
        csv_file.close();
    }
    this->close();
}

InfoWindow::~InfoWindow()
{
    delete ui;
}



InfoWindow * InfoWindow::m_instance=nullptr;
