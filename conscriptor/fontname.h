#ifndef FONTNAME_H
#define FONTNAME_H

#include <QDialog>

#include "ui_fontname.h"
//#include "conscriptor.h"

class FontNameDialog : public QDialog, public Ui::FontNameDialog
{
    Q_OBJECT

public:
    FontNameDialog(QWidget *parent = 0);

private slots:
    void On_buttonOK ();
};

#endif

