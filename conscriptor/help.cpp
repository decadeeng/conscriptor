#include <QtGui>
#include "help.h"
#include "conf.h"

HelpDialog::HelpDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    connect (buttonQuit,     SIGNAL(clicked()), this, SLOT(accept ()));

    QUrl indx;
    indx.setUrl (HELPPATH);
    textBrowser->setSource (indx);
}

/*------------------------------------------------------------------------------------------------
 * on_buttonBack_pushed --
 *----------------------------------------------------------------------------------------------*/
void HelpDialog::on_buttonBack_clicked ()
{
    if (textBrowser->isBackwardAvailable ())
	textBrowser->backward();

    //qDebug () << "Back";
}

/*------------------------------------------------------------------------------------------------
 * on_buttonHome_pushed --
 *----------------------------------------------------------------------------------------------*/
void HelpDialog::on_buttonHome_clicked ()
{
    textBrowser->home();
    //qDebug () << "Home";
}

/*------------------------------------------------------------------------------------------------
 * on_buttonForward_pushed --
 *----------------------------------------------------------------------------------------------*/
void HelpDialog::on_buttonForward_clicked ()
{
    if (textBrowser->isForwardAvailable ())
	textBrowser->forward();

    //qDebug () << "Forward";
}

