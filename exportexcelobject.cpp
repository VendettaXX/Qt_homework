#include "exportexcelobject.h"


int ExportExcelObject::export2Excel()
{
    //QMessageBox::about(nullptr,"Title","傻逼");
    qDebug()<<"field_list="<<field_list.size()<<endl;
    if(field_list.size() <= 0)
    {
        qDebug() << "ExportExcelObject::export2Excel failed: No fields defined.";
        return -1;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "excelexport");
    if(!db.isValid())
    {
        qDebug() << "ExportExcelObject::export2Excel failed: QODBC not supported.";
        return -2;
    }
    // set the dsn string
    QString dsn = QString("DRIVER={Microsoft Excel Driver (*.xls)};DSN='';FIRSTROWHASNAMES=1;READONLY=FALSE;CREATE_DB=\"%1\";DBQ=%2").
                  arg(excel_file_path).arg(excel_file_path);
    db.setDatabaseName(dsn);
    if(!db.open())
    {
        qDebug() << "ExportExcelObject::export2Excel failed: Create Excel file failed by DRIVER={Microsoft Excel Driver (*.xls)}.";
        //QSqlDatabase::removeDatabase("excelexport");
        return -3;
    }

    QSqlQuery query(db);

    //drop the table if it's already exists
    QString s, sSql = QString("DROP TABLE [%1] (").arg(sheet_name);
    query.exec(sSql);

    //create the table (sheet in Excel file)
    sSql = QString("CREATE TABLE [%1] (").arg(sheet_name);
    for (int i = 0; i < field_list.size(); i++)
    {
        s = QString("[%1] %2").arg(field_list.at(i)->field_name).arg(field_list.at(i)->field_type);
        sSql += s;
        if(i < field_list.size() - 1)
            sSql += " , ";
    }

    sSql += ")";
    query.prepare(sSql);

    if(!query.exec())
    {
        qDebug() << "ExportExcelObject::export2Excel failed: Create Excel sheet failed.";
        //db.close();
        //QSqlDatabase::removeDatabase("excelexport");
        return -4;
    }

    //add all rows
    sSql = QString("INSERT INTO [%1] (").arg(sheet_name);
    for (int i = 0; i < field_list.size(); i++)
    {
        sSql += field_list.at(i)->field_name;
        if(i < field_list.size() - 1)
            sSql += " , ";
    }
    sSql += ") VALUES (";
    for (int i = 0; i < field_list.size(); i++)
    {
        sSql += QString(":data%1").arg(i);
        if(i < field_list.size() - 1)
            sSql += " , ";
    }
    sSql += ")";

    qDebug() << sSql;

    int r, iRet = 0;
    for(r = 0 ; r < table_view->model()->rowCount() ; r++)
    {
        query.prepare(sSql);
        for (int c = 0; c < field_list.size(); c++)
        {
            query.bindValue(QString(":data%1").arg(c), table_view->model()->data(table_view->model()->index(r, field_list.at(c)->i_col)));
        }

        if(query.exec())
            iRet++;

        if(r % 10 == 0)
            emit exportedRowCount(r);
    }

    emit exportedRowCount(r);

    return iRet;
}
