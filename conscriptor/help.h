#ifndef HELP_H
#define HELP_H

#include <QDialog>

#include "ui_help.h"

class HelpDialog : public QDialog, public Ui::HelpDialog
{
    Q_OBJECT

public:
    HelpDialog(QWidget *parent = 0);

private slots:
    void on_buttonBack_clicked ();
    void on_buttonForward_clicked ();
    void on_buttonHome_clicked ();
};

#endif

