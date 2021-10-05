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
    QString fileName = QFileDialog::getSaveFileName(this, tr("Excel file"), qApp->applicationDirPath (),
                                                    tr("Excel Files (*.xls)"));
    if (fileName.isEmpty())
        return;
    ExportExcelObject obj(fileName, "mydata", this->p_main->ui->table_view);

    //    // you can change the column order and
    //    // choose which colum to export
    obj.addField(0, "colum1", "char(20)");
    obj.addField(3, "colum4", "char(20)");
    obj.addField(1, "colum2", "char(20)");
    obj.addField(2, "colum3", "char(20)");

    obj.addField(4, "colum5", "char(20)");
    obj.addField(5, "colum6", "char(20)");
    obj.addField(6, "colum7", "char(20)");

//    ui->progressBar->setValue(0);
//    ui->progressBar->setMaximum(ui->tableView->model()->rowCount());

//    connect(&obj, SIGNAL(exportedRowCount(int)), ui->progressBar, SLOT(setValue(int)));

	int retVal = obj.export2Excel();
	if( retVal > 0)
	{
		QMessageBox::information(this, tr("Done"),
		                         QString(tr("%1 records exported!")).arg(retVal)
		                         );
	}
}

InfoWindow::~InfoWindow()
{
    delete ui;
}



InfoWindow * InfoWindow::m_instance=nullptr;
