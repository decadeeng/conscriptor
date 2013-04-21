#include <QtGui>
#include "glyph.h"
#include "codeval.h"
//#include "conscriptor.h"

extern int debug;

/*-----------------------------------------------------------------------------------------------
 * constructor
 *---------------------------------------------------------------------------------------------*/
CodePointDialog::CodePointDialog(QWidget *parent)
    : QDialog(parent)
{
char str[20];

    b4font = qobject_cast<B4Font*>(parent);
    setupUi(this);
    connect (buttonCancel, SIGNAL(clicked()), this, SLOT(reject ()));
    //qDebug ("%d", b4font->macroGlyph);
    sprintf (str, "%d", b4font->allglyphs[b4font->macroGlyph]->getCodePoint()); 
    b4font->cp2utf (b4font->allglyphs[b4font->macroGlyph]->getCodePoint());
    editDecimal->setText (str);
    sprintf (str, "%02x", b4font->allglyphs[b4font->macroGlyph]->getCodePoint()); 
    editHex->setText (str);
    b4font->cp2utf (b4font->allglyphs[b4font->macroGlyph]->getCodePoint());
    editUTF8->setText (b4font->utf);

    QValidator *dvalidator = new QIntValidator (this);
    editDecimal->setValidator (dvalidator);
    QRegExp rx ("[0-9a-fA-F]*");
    QValidator *hvalidator = new QRegExpValidator (rx, this);
    editHex->setValidator (hvalidator);
}

/*-----------------------------------------------------------------------------------------------
 * on_buttonOK_clicked --
 *---------------------------------------------------------------------------------------------*/
void CodePointDialog::on_buttonOK_clicked ()
{
    if ((debug))
	qDebug ("[codeval]-on_buttonOK----");
    accept ();
}

/*-----------------------------------------------------------------------------------------------
 * on_editDecimal_textEdited --
 *---------------------------------------------------------------------------------------------*/
void CodePointDialog::on_editDecimal_textEdited (QString str)
{
bool ok;
char s[20];

    b4font->value = str.toInt (&ok, 10);
    sprintf (s, "%02x", b4font->value);
    editHex->setText (s);
    b4font->cp2utf (b4font->value);
    editUTF8->setText (b4font->utf);
}

/*-----------------------------------------------------------------------------------------------
 * on_editHex_textEdited --
 *---------------------------------------------------------------------------------------------*/
void CodePointDialog::on_editHex_textEdited (QString str)
{
bool ok;
char s[20];

    b4font->value = str.toInt (&ok, 16);
    sprintf (s, "%d", b4font->value);
    editDecimal->setText (s);
    b4font->cp2utf (b4font->value);
    editUTF8->setText (b4font->utf);
}
