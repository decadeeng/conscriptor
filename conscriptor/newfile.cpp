#include <QtGui>
#include "newfile.h"
#include "conf.h"
#include "conscriptor.h"
#include "font.h"

extern int debug;

/*-----------------------------------------------------------------------------------------------
 * constructor
 *---------------------------------------------------------------------------------------------*/
NewfileDialog::NewfileDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    connect (buttonCancel, SIGNAL(clicked()), this, SLOT(reject ()));
}

/*-----------------------------------------------------------------------------------------------
 * on_buttonOK_clicked --
 * calloc vs new used to enable 'realloc' later when editing is performed
 * conscriptor->header.chWidth  = spinWidth->value();
 * again if ever chDepth is not 0, 1 then here be dragons
 *---------------------------------------------------------------------------------------------*/
void NewfileDialog::on_buttonOK_clicked ()
{
QString str;
int count;
int height;
int width;

    if ((debug))
	qDebug ("[newfile]     dialog on_buttonOK_clicked");

    Conscriptor* conscriptor = qobject_cast<Conscriptor*>(this->parent());

    if ((count = spinCinF->value()) > NEWGLYPHS) {
	str.setNum (NEWGLYPHS);
	str.prepend (tr("The maximum number of new characters are "));
	str.append (tr("\nYou can set the CodePoint of any character to any value"));
	QMessageBox::warning(this, tr("Conscriptor"), str, QMessageBox::Ok);
	return;
    }

    if (((height = spinHeight->value()) > 255) || ((width = spinWidth->value()) > 255)) {
	str = (tr("The maximum width and height of a charater is 255 pixels"));
	QMessageBox::warning(this, tr("Conscriptor"), str, QMessageBox::Ok);
	return;
    }

    conscriptor->header.chHeight = spinHeight->value();
    conscriptor->header.chAscent = spinAscent->value();
    conscriptor->header.chCount  = spinCinF->value();
    conscriptor->header.chFirst  = spinFirst->value();
    conscriptor->header.fontVersion  = VERSIONNO;
 
    if ((radio1BPP->isChecked()))
	conscriptor->header.chDepth = 0;
    else
	conscriptor->header.chDepth = 1;

    conscriptor->header.skip = 0;

    memset (conscriptor->header.name, 0, 16);			// spec
    str = lineEdit->text();
    strncpy ((char *)conscriptor->header.name, str.toAscii().constData(), 12);
    if (conscriptor->curFile.isEmpty()) {
	conscriptor->curFile = str;
	if (!(str.endsWith (".b4f", Qt::CaseSensitive)))
	    conscriptor->curFile.append (".b4f");
	}

    conscriptor->b4font->setNewGlyphs (width, conscriptor->header.chHeight, conscriptor->header.chCount);
qDebug ("              /dialog");
    accept ();
}
