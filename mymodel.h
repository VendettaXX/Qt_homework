#ifndef MYMODEL_H
#define MYMODEL_H
#include <QVariant>
#include <QStandardItemModel>

class MyModel
{
public:
    MyModel();
    QVariant data(const QModelIndex &index,int role=Qt::DisplayRole) const;
};

#endif // MYMODEL_H
