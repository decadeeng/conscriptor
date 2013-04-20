#ifndef PRINTT_H
#define PRINT_H

#include <QDialog>

#include "ui_print.h"
#include "conscriptor.h"

class PrintDialog : public QDialog, public Ui::PrintDialog
{
    Q_OBJECT

public:
    PrintDialog(QWidget *parent = 0);
    Conscriptor *conscriptor;

private slots:

};

#endif

