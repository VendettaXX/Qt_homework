#ifndef INFOWINDOW_H
#define INFOWINDOW_H

#include <QMainWindow>
#include <QDebug>

namespace Ui {
class InfoWindow;
}

class InfoWindow : public QMainWindow
{
    Q_OBJECT

public:
    ~InfoWindow();
    static InfoWindow * get_instance()
    {
        if(nullptr==m_instance)
        {
            m_instance=new InfoWindow();
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

    explicit InfoWindow(QWidget *parent = nullptr);
   // friend class MainWindow;
};

#endif // INFOWINDOW_H
