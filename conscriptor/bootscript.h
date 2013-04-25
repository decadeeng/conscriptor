#ifndef BOOTSCRIPT_H
#define BOOTSCRIPT_H

#include <QDialog>

#include "ui_bootscript.h"
class Conscriptor;

class BootscriptDialog : public QDialog, public Ui::BootscriptDialog
{
    Q_OBJECT

public:
    BootscriptDialog(QWidget *parent = 0);
    Conscriptor *conscriptor;

private slots:
    void on_buttonLoadfile_clicked ();
    void on_buttonSavefile_clicked ();
    void on_buttonDownload_clicked ();
    void on_buttonCSI_clicked ();
    void on_buttonCRLF_clicked ();
    void on_buttonStrString_clicked ();
    void on_buttonEndString_clicked ();
    void setModified ();
    void exitBSE ();
private:
    QString curFile;
    bool modified;
};

#endif

