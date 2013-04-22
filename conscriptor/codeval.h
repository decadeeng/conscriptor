#ifndef CODEVAL_H
#define CODEVAL_H

#include <QDialog>

#include "ui_codeval.h"
#include "font.h"

class CodePointDialog : public QDialog, public Ui::CodePointDialog
{
    Q_OBJECT

public:
    CodePointDialog(QWidget *parent = 0);

private:
    B4Font *b4font;

private slots:
    void on_buttonOK_clicked ();
    void on_editDecimal_textEdited (QString str);
    void on_editHex_textEdited (QString str);
};

#endif

