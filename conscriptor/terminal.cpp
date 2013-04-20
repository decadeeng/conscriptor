#include <QtGui>
#include "terminal.h"
#include "conscriptor.h"

/*-------------------------------------------------------------------------------------------------
 * TerminalDialog --
 *	can't callaccept in the constructor
 *-----------------------------------------------------------------------------------------------*/
TerminalDialog::TerminalDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    conscriptor = qobject_cast<Conscriptor*>(parent);

    timer = new QTimer(this);
        timer->setSingleShot (false);

    conscriptor->setTtyPort ();
    if (conscriptor->commPort == Conscriptor::NONE) {
	conscriptor->setTtyDebug ();
	if (conscriptor->commPort == Conscriptor::NONE) {
	    conscriptor->statusBar()->showMessage(tr("Terminal Dialog failed: no BOB found"), 2000);
	    QApplication::postEvent( this, new QCloseEvent() );
	}
    }

    if (conscriptor->commPort == Conscriptor::DEBUGP)
	buttonCSI->setEnabled (false);

    oldSize = 0;
    expecting = false;

    connect (buttonQuit, SIGNAL(clicked()), this,   SLOT(accept ()));
    connect(timer, SIGNAL(timeout()), SLOT(terminal_readyRead()));
    timer->start(300);             
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonCSI_clicked --
 *	on_textChanged called immediately it changes
 *-----------------------------------------------------------------------------------------------*/
void TerminalDialog::on_buttonCSI_clicked ()
{
char str[8];

    oldSize += 5;
    terminal->insertPlainText ("<CSI>");
    QString terminalWindow = terminal->toPlainText ();
    sprintf (str, "\x1b" "[");
    conscriptor->ttyPort->write (str, strlen (str)); 
    terminal->setFocus ();
}

/*-------------------------------------------------------------------------------------------------
 * on_textChanged ()
 *-----------------------------------------------------------------------------------------------*/
void TerminalDialog::on_terminal_textChanged ()
{
    //qDebug ("[on_terminal_textChanged]");
    QApplication::processEvents();		// show the last character pressed
    terminalWindow = terminal->toPlainText ();
    int newSize = terminalWindow.size ();

    if ((expecting)) {				// data received on tty and written to display
	//qDebug ("  [expecting] received on tty don't resend");
	expecting = false;
	oldSize = newSize;
	return;
    }
    
    /*---------------------------------------------------------------------------------------------
     * terminalWindow is what was typed or pasted
     * iSizes equal happens eg when CSI entered
     *-------------------------------------------------------------------------------------------*/
    //qDebug ("  2  oldSize %d newSize %d", oldSize, newSize);
    if ((oldSize == newSize) ||	(!terminalWindow.endsWith ("\n")))
	return;

    sendStr = terminalWindow.right (newSize - oldSize);
    oldSize = newSize;

    //qDebug () << "  3  [write to tty]" << sendStr;
    conscriptor->ttyPort->write (sendStr.toAscii ());
}

/*-------------------------------------------------------------------------------------------------
 * on_readyRead ()
 *-----------------------------------------------------------------------------------------------*/
void TerminalDialog::terminal_readyRead ()
{
    recvStr = conscriptor->ttyPort->readAll();
    int readSize = recvStr.size();

    if ((readSize)) {				// got something
	//qDebug () << "[readyRead" << readSize << "]" << recvStr; 
	recvStr.replace ("\r\n", "\n");
	recvStr.replace ("\n\r", "\n");

	expecting = true;
	//qDebug ("  [set expecting]");
	terminal->insertPlainText (recvStr);
	terminal->ensureCursorVisible ();
	terminalWindow = terminal->toPlainText ();
	oldSize = terminalWindow.size ();
	//qDebug () << "  [set size]" << oldSize;
    }
}

