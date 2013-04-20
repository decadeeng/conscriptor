#include <QtGui>
#include "font.h"
#include "importbdf.h"

//class B4Font;

ImportDialog::ImportDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    conscriptor = qobject_cast<Conscriptor*>(parent);
    B4Font *b4font = conscriptor->b4font;

    connect (buttonCancel, SIGNAL(clicked()), this,   SLOT(accept ()));
    connect (buttonOK,     SIGNAL(clicked()), b4font, SLOT(on_buttonImportBDF ()));
    connect (buttonOK,     SIGNAL(clicked()), this,   SLOT(close ()));
}
