#ifndef TERMINAL_H
#define TERMINAL_H

#include <QDialog>

#include "ui_terminal.h"
//#include "conscriptor.h"
class Conscriptor;

class TerminalDialog : public QDialog, public Ui::TerminalDialog
{
    Q_OBJECT

public:
    TerminalDialog(QWidget *parent = 0);

private:
    Conscriptor *conscriptor;
    int oldSize;
    bool expecting;
    QString terminalWindow;
    QString sendStr;
    QByteArray recvStr;
    char echo1;
    char echo2;
    QTimer *timer;

private slots:
    void on_buttonCSI_clicked ();
    void on_terminal_textChanged ();
    void terminal_readyRead ();
};

#endif

