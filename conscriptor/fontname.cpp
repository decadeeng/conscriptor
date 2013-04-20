#include <QtGui>
#include "fontname.h"
#include "conscriptor.h"

/*-----------------------------------------------------------------------------------------------
 * constructor
 *---------------------------------------------------------------------------------------------*/
FontNameDialog::FontNameDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect (buttonBox, SIGNAL(accepted()), this, SLOT(On_buttonOK ()));
    connect (buttonBox, SIGNAL(accepted()), this->parentWidget(), SLOT(setModified ()));
    connect (buttonBox, SIGNAL(rejected()), this, SLOT(accept ()));
    Conscriptor* conscriptor = qobject_cast<Conscriptor*>(this->parent());
    lineEdit->setText ((char *)conscriptor->header.name);
}

/*-----------------------------------------------------------------------------------------------
 * On_buttonOK --
 *---------------------------------------------------------------------------------------------*/
void FontNameDialog::On_buttonOK ()
{
    Conscriptor* conscriptor = qobject_cast<Conscriptor*>(this->parent());
    strncpy ((char *)conscriptor->header.name, lineEdit->text().toLocal8Bit(), 12);
    accept ();
}
