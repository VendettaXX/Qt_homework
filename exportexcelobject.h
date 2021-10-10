#ifndef EXPORTEXCELOBJECT_H
#define EXPORTEXCELOBJECT_H

#include <QObject>
#include <QTableView>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>




class EEOField
{
public:
    EEOField(const int ic, const QString &sf, const QString &st):
            i_col(ic),field_name(sf),field_type(st){}
    int i_col;
    QString field_name;
    QString field_type;
};

class ExportExcelObject : public QObject
{
    Q_OBJECT
public:
    ExportExcelObject(const QString &filepath, const QString &sheettitle,
                      QTableView *tableview):excel_file_path(filepath),
                      sheet_name(sheettitle), table_view(tableview){}
    void setOutputFilePath(const QString &spath) {excel_file_path = spath;}
    void setOutputSheetTitle(const QString &ssheet) {sheet_name = ssheet;}
    void setTableView(QTableView *table_view) {table_view = table_view;}

    void addField(const int i_col, const QString &field_name, const QString &field_type)
         {field_list << new EEOField(i_col, field_name, field_type);}

    void removeAllFields()
         {while (!field_list.isEmpty()) delete field_list.takeFirst();}

    int export2Excel();

signals:
    void exportedRowCount(int row);
public slots:

private:
    QString excel_file_path;
    QString sheet_name;
    QTableView *table_view;
    QList<EEOField *> field_list;
};

#endif // EXPORTEXCELOBJECT_H
