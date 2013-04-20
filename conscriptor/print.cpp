#include <QtGui>
#include "font.h"
#include "print.h"

PrintDialog::PrintDialog(QWidget *parent) : QDialog(parent)
{
    setupUi(this);

    conscriptor = qobject_cast<Conscriptor*>(parent);
    B4Font *b4font = conscriptor->b4font;

    connect (buttonCancel,      SIGNAL(clicked()), this,   SLOT(accept ()));
    connect (buttonPrintScreen, SIGNAL(clicked()), b4font, SLOT(on_buttonPrintScreen ()));
    connect (buttonPrintScreen, SIGNAL(clicked()), this,   SLOT(accept ()));
    connect (buttonPrintAll,    SIGNAL(clicked()), b4font, SLOT(on_buttonPrintAll ()));
    connect (buttonPrintAll,    SIGNAL(clicked()), this,   SLOT(accept ()));
}
