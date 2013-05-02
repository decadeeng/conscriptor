#include <QtGui>
#include "conscriptor.h"
#include "bootscript.h"

BootscriptDialog::BootscriptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    conscriptor = qobject_cast<Conscriptor*>(parent);

    connect (buttonExit,     SIGNAL (clicked()), this, SLOT(exitBSE ()));
    connect (plainTextEdit,  SIGNAL (textChanged()), this, SLOT(setModified ()));
    connect (spinScreenWidth, SIGNAL (valueChanged (int)), this, SLOT (screenWidthChanged (int)));

    setLayout (verticalLayout);

    modified = false;
    ignore = false;
    buttonDownload->setEnabled (false);
    conscriptor->isBOB ();
}

/*-------------------------------------------------------------------------------------------------
 * formatWindow --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::formatWindow (int newWidth)
{
int tidx;			// tag index
int lastx;			// end <CSI> sequence

    if ((ignore)) {
	ignore = false;
	return;
    }

    QString data = plainTextEdit->toPlainText();
    //data.remove ('\r');
    data.remove ('\n');

    /*---------------------------------------------------------------------------------------------
     * Always format <tags> 
     *-------------------------------------------------------------------------------------------*/
    data.replace ("<CRLF>", "<CRLF>\n");
    data.replace ("<CR>", "<CR>\n");
    data.replace ("<LF>", "<LF>\n");
    data.replace ("<ESCQ>", "\n<ESCQ>");			// This can mess the start
    data.replace ("<ESCR>", "<ESCR>\n");

    if (data[0] == QChar ('\n'))				// ... so fix it
	data.remove (0, 1);

#if 0
    QByteArray ba;
    qDebug ("[formatWindow] %d", newWidth);

    ba.append (plainTextEdit->toPlainText());
    ba.replace ('\r', "+");
    ba.replace ('\n', " ");

//    if (spinScreenWidth->value () != 0) {
//	ba.replace ("<CRLF>", "");
//    }
//	return;

#endif 

    ignore = true;
    plainTextEdit->setPlainText (data);
    plainTextEdit->moveCursor (QTextCursor::End, QTextCursor::MoveAnchor);
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * screenWidthChanged --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::screenWidthChanged (int newWidth)
{
    formatWindow (newWidth);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadfile_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonLoadfile_clicked ()
{
    curFile = QFileDialog::getOpenFileName(this, tr ("read bootscript files"), "", "(*.b4b *.b4j)");
    if (curFile.isEmpty())
        return;

    QFile file(curFile);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this, tr ("consciptor"),
		tr ("Cannot read file %1:%2.")
		.arg(curFile)
		.arg(file.errorString()));
        return;
    }

    QByteArray ba = file.readAll ();
    /*---------------------------------------------------------------------------------------------
     * Translate b4b files to tagged format b4j
     *-------------------------------------------------------------------------------------------*/
    if ((curFile.endsWith (".b4b"))) {
	ba.replace ("\r\n", "<CRLF>");
	ba.replace ('\r',   "<CR>");
	ba.replace ('\n',   "<LF>");
	ba.replace ("[",  "<CSI>");
	ba.replace ("Q",  "<ESCQ>");
	ba.replace ("R",  "<ESCR>");
    }
    else {
	/*-----------------------------------------------------------------------------------------
	 * Cleanup any editor fiddling on any os b4j's
	 *---------------------------------------------------------------------------------------*/
	ba.replace ('\r', "++");
	ba.replace ('\n', " ");
    }
    QString data (ba);
    ignore = false;
    plainTextEdit->setPlainText (data);
    formatWindow (spinScreenWidth->value());
    plainTextEdit->moveCursor (QTextCursor::End, QTextCursor::MoveAnchor);
    plainTextEdit->setFocus (Qt::OtherFocusReason);
    buttonDownload->setEnabled (true);
    modified = false;
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonSavefile_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonSavefile_clicked ()
{
    QString fileName = QFileDialog::getSaveFileName (this, tr ("Save File"), curFile,
	    tr("booscript files (*.b4j)"));

    if (!fileName.endsWith (".b4j"))
	fileName.append (".b4j");

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot read file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }

    QByteArray ba;
    ba.append (plainTextEdit->toPlainText());
    ba.replace ('\r', "+++");
    ba.replace ('\n', " ");
    file.write (ba);
    file.close ();
    modified = false;
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonDownload_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonDownload_clicked ()
{
    conscriptor->statusBar()->showMessage(tr("Download bootscript"), 6000);
    QByteArray ba = plainTextEdit->toPlainText ().toAscii ();
    /*---------------------------------------------------------------------------------------------
     * resolve the tags
     * sanitize the file of <CR><LF>
     *-------------------------------------------------------------------------------------------*/
    ba.replace ('\r', "");
    ba.replace ('\n', "");
    ba.replace ("<CSI>",  "[");
    ba.replace ("<ESCQ>", "Q");
    ba.replace ("<ESCR>", "R");
    ba.replace ("<CRLF>", "\r\n");
    ba.replace ("<CR>",   "\r");
    ba.replace ("<LF>",   "\n");
    if ((conscriptor->bootscriptDownload (ba)))
	buttonDownload->setEnabled (false);
    conscriptor->statusBar()->showMessage(tr("Download complete"), 6000);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonCSI_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonCSI_clicked ()
{
    QTextCursor cur = plainTextEdit->textCursor ();
    if (cur.positionInBlock() == 0)
	plainTextEdit->insertPlainText ("<CSI>");
    else
	plainTextEdit->insertPlainText ("\n<CSI>");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonCRLF_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonCRLF_clicked ()
{
    plainTextEdit->insertPlainText ("<CRLF>\n");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonStrString_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonStrString_clicked ()
{
    QTextCursor cur = plainTextEdit->textCursor ();
    if (cur.positionInBlock() == 0)
	plainTextEdit->insertPlainText ("<ESCQ>");
    else
	plainTextEdit->insertPlainText ("\n<ESCQ>");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonEndString_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonEndString_clicked ()
{
    plainTextEdit->insertPlainText ("<ESCR>\n");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * setModified --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::setModified ()
{
    modified = true;
    buttonDownload->setEnabled (true);

    if (spinScreenWidth->value() > 0)
	formatWindow (spinScreenWidth->value());
}

/*-------------------------------------------------------------------------------------------------
 * exitBSE --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::exitBSE ()
{
    if ((modified)) {
        int r = QMessageBox::warning(this,
		    tr("Bootscript Editor"),
		    tr("Bootscript has been modified.\n"
		       "Do you want to save it?"),
		    QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes)
	    on_buttonSavefile_clicked ();
    }
    accept ();
}
