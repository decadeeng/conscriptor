#ifndef IMPORTBDF_H
#define IMPORTBDF_H

#include <QDialog>

#include "ui_importbdf.h"
#include "conscriptor.h"

class ImportDialog : public QDialog, public Ui::ImportDialog
{
    Q_OBJECT

public:
    ImportDialog(QWidget *parent = 0);

private:
    Conscriptor *conscriptor;

//private slots:
//    void on_buttonOK_clicked ();

};

#endif

