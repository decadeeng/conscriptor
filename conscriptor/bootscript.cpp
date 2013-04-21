#include <QtGui>
#include "conscriptor.h"
#include "bootscript.h"

BootscriptDialog::BootscriptDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    conscriptor = qobject_cast<Conscriptor*>(parent);

    connect (buttonExit,     SIGNAL(clicked()), this, SLOT(exitBSE ()));
    connect (plainTextEdit,  SIGNAL(textChanged()), this, SLOT(setModified ()));
    modified = false;
    buttonDownload->setEnabled (false);
    conscriptor->isBOB ();
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadfile_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonLoadfile_clicked ()
{
    curFile = QFileDialog::getOpenFileName(this, tr ("read bootscript files"), "", tr ("(*.b4b)"));
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
    QTextStream fil(&file);
    QString data;
    for (;;) {
	data = fil.readLine (100);
	if (data.isEmpty())
	    break;
	plainTextEdit->insertPlainText (data);
    }
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
	    tr("booscript files (*.b4b)"));

    if (!fileName.endsWith (".b4b"))
	fileName.append (".b4b");

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot read file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }

    QTextStream out (&file);
    QString data = plainTextEdit->toPlainText();
    out << data.toAscii();
    modified = false;
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonDownload_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonDownload_clicked ()
{
    conscriptor->statusBar()->showMessage(tr("Download bootscript"), 6000);
    ba = plainTextEdit->toPlainText ().toAscii ();
    if ((conscriptor->bootscriptDownload (ba)))
	buttonDownload->setEnabled (false);
    conscriptor->statusBar()->showMessage(tr("Download complete"), 6000);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonCSI_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonCSI_clicked ()
{
    plainTextEdit->insertPlainText ("[");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonCRLF_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonCRLF_clicked ()
{
    plainTextEdit->insertPlainText ("\r\n");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonStrString_clicked --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonStrString_clicked ()
{
    plainTextEdit->insertPlainText ("Q");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 *
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::on_buttonEndString_clicked ()
{
    plainTextEdit->insertPlainText ("R");
    plainTextEdit->setFocus (Qt::OtherFocusReason);
}

/*-------------------------------------------------------------------------------------------------
 * setModified --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::setModified ()
{
    modified = true;
    buttonDownload->setEnabled (true);
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
