#ifndef SETMEM_H
#define SETMEM_H

#include <QDialog>

#include "ui_setmem.h"

class Conscriptor;

class SetmemDialog : public QDialog, public Ui::SetmemDialog
{
    Q_OBJECT

public:
    SetmemDialog(QWidget *parent = 0);

private:
    Conscriptor *conscriptor;

private slots:
    void buttonOK ();
};

#endif

