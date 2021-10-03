#include "mymodel.h"

MyModel::MyModel()
{	

}

QVariant MyModel::data(const QModelIndex &index, int role) const
{
   if(!index.isValid())
       return QVariant();
   if (role == Qt::TextAlignmentRole)
       {
           return int(Qt::AlignLeft | Qt::AlignVCenter);
       }
//       else if (role == Qt::DisplayRole)
//       {
//             return QStandardItemModel::data(index, role);
//       }
       else if(role == Qt::BackgroundColorRole )
       {
           if(index.row()%5 == 0)
               return QColor(Qt::red);
           else if(index.row()%5 == 1)
               return QColor(Qt::green);
           else if(index.row()%5 == 2)
               return QColor(Qt::blue);
           else if(index.row()%5 == 3)
               return QColor(Qt::yellow);
           else if(index.row()%5 == 4)
               return QColor(Qt::gray);
           else
               return QVariant();
       }

       return QVariant();

       return QVariant();
   }
}

