#ifndef BAUDSEL_H
#define BAUDSEL_H

#include <QDialog>

#include "ui_baudselect.h"
class Conscriptor;

class BaudSelectDialog : public QDialog, public Ui::BaudSelectDialog
{
    Q_OBJECT

public:
    BaudSelectDialog(QWidget *parent = 0);

private:
    Conscriptor *conscriptor;

private slots:
    void on_radio1200_pressed ();
    void on_radio2400_pressed ();
    void on_radio4800_pressed ();
    void on_radio9600_pressed ();
    void on_radio19200_pressed ();
    void on_radio38400_pressed ();
    void on_radio115200_pressed ();
};

#endif

