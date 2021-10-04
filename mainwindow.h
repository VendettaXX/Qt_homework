#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QTimer>
#include <QThread>
#include "channel.h"
#include <QString>
#include <QDebug>
#include "infowindow.h"
#include "ui_infowindow.h"
#include <QStandardItem>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTimer *timer;

private:
    QStandardItemModel * model;
    void init_table(QStandardItemModel *);
    Ui::MainWindow *ui;
    Channel  * p_channel;
    QThread *thread;
    InfoWindow *iw;
public slots:
    //void on_timer_out();
    void display_mesg(DataItem *);
    void pause_resume();
    void display_result();
    void stop();
};

#endif // MAINWINDOW_H
