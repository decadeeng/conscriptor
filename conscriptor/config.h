#ifndef CONFIG_H
#define CONFIG_H

#include <QDialog>
#include "ui_config.h"
class Conscriptor;

class ConfigDialog : public QDialog, public Ui::ConfigDialog
{
    Q_OBJECT

public:
    ConfigDialog(QWidget *parent = 0);
    QByteArray ba;				// All config data stored here

private:
    QString curFile;
    bool modified;
    Conscriptor *conscriptor;
    void loadBA ();

private slots:
    void on_buttonLoadFile_clicked ();
    void on_buttonSaveFile_clicked ();
    void on_buttonDownload_clicked ();
    void setModified ();
    void exitCE ();
};

#endif

