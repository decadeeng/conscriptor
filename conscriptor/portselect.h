#ifndef PORTSEL_H
#define PORTSEL_H

#include <QDialog>

#include "ui_portselect.h"
class Conscriptor;

class PortSelectDialog : public QDialog, public Ui::PortSelectDialog
{
    Q_OBJECT

public:
    PortSelectDialog(QWidget *parent = 0);

private:
    Conscriptor *conscriptor;

private slots:
    void on_radioTty1_pressed ();
    void on_radioTty2_pressed ();
    void on_radioTty3_pressed ();
    void on_radioTty4_pressed ();
    void on_radioTty5_pressed ();
    void on_radioTty6_pressed ();
    void on_radioTty7_pressed ();
    void on_radioTty8_pressed ();
    void on_radioTty9_pressed ();
};

#endif

