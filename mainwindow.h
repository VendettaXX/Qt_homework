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
#include <QMessageBox>
namespace Ui {
class MainWindow;
}
class InfoWindow;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTimer *timer;
    bool table_view_status;
    Ui::MainWindow *ui;
    QStandardItemModel * model;

private:
    void init_table(QStandardItemModel *);
    Channel  * p_channel;
    QThread *thread;
    InfoWindow *iw;
    int num;
public slots:
    void time_out();
    void display_mesg(DataItem *);
    //void display_mesg_slot(SlotData *);
    void pause_resume();
    void display_result();
    void stop();
};

#endif // MAINWINDOW_H
