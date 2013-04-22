#include <QtGui>
#include "setmem.h"
#include "conscriptor.h"

SetmemDialog::SetmemDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    conscriptor = qobject_cast<Conscriptor*>(parent);
    connect (buttonBox,     SIGNAL(rejected()), this, SLOT(reject ()));
    connect (buttonBox,     SIGNAL(accepted()), this, SLOT(accept ()));
    connect (buttonBox,     SIGNAL(accepted()), this, SLOT(buttonOK ()));
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonOK --
 *-----------------------------------------------------------------------------------------------*/
void SetmemDialog::buttonOK ()
{
    if ((radioDev0->isChecked()))
	conscriptor->flashDevice = 0;
    else if ((radioDev1->isChecked()))
	conscriptor->flashDevice = 1;
    else if ((radioDev2->isChecked()))
	conscriptor->flashDevice = 2;
    else if ((radioDev3->isChecked()))
	conscriptor->flashDevice = 3;
    else if ((radioDev4->isChecked()))
	conscriptor->flashDevice = 4;

    //qDebug ("[on OK] flashDevice %d", conscriptor->flashDevice);
}
