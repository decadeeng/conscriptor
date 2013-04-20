#ifndef NEWFILE_H
#define NEWFILE_H

#include <QDialog>

#include "ui_newfile.h"
#include "conscriptor.h"

class NewfileDialog : public QDialog, public Ui::NewfileDialog
{
    Q_OBJECT

public:
    NewfileDialog(QWidget *parent = 0);

private slots:
    void on_buttonOK_clicked ();
};

#endif

