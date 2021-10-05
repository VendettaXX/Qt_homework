#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include "ui_mainwindow.h"
#include "exportexcelobject.h"

namespace Ui {
class InfoWindow;
}
class MainWindow;
class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    ~InfoWindow();
    static InfoWindow * get_instance(MainWindow *p)
    {
        if(nullptr==m_instance)
        {
            m_instance=new InfoWindow(p);
            static InfoInClass cl;
        }
        return m_instance;
    }
    class InfoInClass{
    public:
        ~InfoInClass()
        {
            if(InfoWindow::m_instance)
            {
                delete InfoWindow::m_instance;
                InfoWindow::m_instance=nullptr;
            }
        }
        void fun()
        {
            qDebug()<<"test"<<endl;
        }

    };

    Ui::InfoWindow *ui;
private:
    static InfoWindow * m_instance;
    MainWindow *p_main;
    InfoWindow(MainWindow * p,QWidget *parent = nullptr );

   // friend class MainWindow;
private slots:
    void save_btn_clicked();
};

#endif // INFOWINDOW_H
