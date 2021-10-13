#ifndef USERNODE_H
#define USERNODE_H
#include <QList>

#include <QString>
class UserInfo{
public:
    unsigned int current_time;
    QString user_id;
    UserInfo(unsigned int current_time,QString user_id):current_time(current_time),user_id(user_id){}
};

class UserNode
{
public:
    int frame_begin_time;
    unsigned int frame_num;
    UserNode(QString name):frame_num(0),st(true),name(name){}
    //UserNode(bool st):st(st){}
    bool st;
    QList<UserInfo *> collusion_list;
    QString name;
};

#endif // USERNODE_H
