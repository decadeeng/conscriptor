#ifndef SETUP_H
#define SETUP_H

#include <QDialog>

#include "ui_setup.h"
#include "conscriptor.h"

class SetupDialog : public QDialog, public Ui::SetupDialog
{
    Q_OBJECT

public:
    SetupDialog(QWidget *parent = 0);
    Conscriptor *conscriptor;

private slots:
    void on_buttonLoadFile_clicked ();
    void on_buttonSaveFile_clicked ();
    void on_buttonDownload_clicked ();
    void on_buttonLoadB4B_clicked ();
    void on_buttonLoadB4C_clicked ();
    void on_buttonLoadB4F_clicked ();
    void textChanged ();
};

#endif

