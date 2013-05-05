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

    spinScreenWidth->setVisible (false);		// Maybe do this someday
    label_5->setVisible (false);

    conscriptor->isBOB ();
}

/*-------------------------------------------------------------------------------------------------
 * formatWindow --
 *-----------------------------------------------------------------------------------------------*/
void BootscriptDialog::formatWindow (int newWidth)
{
    if ((ignore)) {
	ignore = false;
	qDebug ("[ignore] - set false - & - return");
	return;
    }

    QString data = plainTextEdit->toPlainText();
    /*---------------------------------------------------------------------------------------------
     * this function runs if width is set
     * and for character entry if width is not 0
     *-------------------------------------------------------------------------------------------*/
    data.remove ('\r');
    data.remove ('\n');

    /*---------------------------------------------------------------------------------------------
     * screenWidth set to something
     * so format the line length
     *-------------------------------------------------------------------------------------------*/
    if (spinScreenWidth->value () > 0) {
	int thisEnd = 0;
	int thisStart;
	int curIndex;
	int crlfIndex = -1;
	int crIndex = -1;
	int lfIndex = -1;
	int escqIndex = -1; 
	int escrIndex = -1;
	int csiIndex = -1;
	int csiEnd = -1;

	/*-----------------------------------------------------------------------------------------
	 * foreach line  thisStart..............thisEnd
	 *---------------------------------------------------------------------------------------*/
	for ( ;thisEnd < data.size ();) {

	    thisStart = thisEnd;
	    thisEnd += spinScreenWidth->value ();
	    thisEnd = qMin (thisEnd, data.size ());
	    qDebug ("----------editLoop\n[Size %d] thisStart:%d thisEnd:%d screenWidth:%d",
			    data.size (), thisStart, thisEnd, spinScreenWidth->value ());

	    //------------------- Show the window data
	    for (QChar *it=data.begin(); it!=data.end(); ++it) {
		char ch = it->toAscii();
		if ((ch >= ' ') && (ch <= 'z'))
		    printf ("%c ", ch);
		else if (ch == '\n')
		    printf ("0a\n");
		else
		    printf ("%02x ", ch);
	    }
	    printf ("\n");

	    /*-------------------------------------------------------------------------------------
	     * Workout thisLine length
	     *-----------------------------------------------------------------------------------*/
	    curIndex = data.indexOf  ("<",  thisStart);			// find 1st tag
	    if ((curIndex != -1) && (curIndex < thisEnd)) {		//   which is in this line

		/*---------------------------------------------------------------------------------
		 * Find index's if they exist
		 *-------------------------------------------------------------------------------*/
		if (crlfIndex < thisStart)
		    crlfIndex = data.indexOf ("<CRLF", thisStart);
		if (crIndex < thisStart)
		    crIndex = data.indexOf   ("<CR>",  thisStart);
		if (lfIndex < thisStart)
		    lfIndex = data.indexOf   ("<LF",   thisStart);
		if (escqIndex < thisStart)
		    escqIndex = data.indexOf ("<ESCQ", thisStart);
		if (escrIndex < thisStart)
		    escrIndex = data.indexOf ("<ESCR", thisStart);
		if (csiIndex < thisStart)
		    csiIndex = data.indexOf  ("<CSI",  thisStart);
		if (csiIndex != -1)
		    csiEnd = data.indexOf (QRegExp ("[A-DHJ-MUXfmnqstuvwxz}{|]"),  csiIndex + 5);

		//------------------ Show the Index's
		qDebug ("[cur]    %d", crlfIndex);
		qDebug ("[crlf]   %d", crlfIndex);
		qDebug ("[cr]     %d", crIndex);
		qDebug ("[lf]     %d", lfIndex);
		qDebug ("[escq]   %d", escqIndex);
		qDebug ("[escr]   %d", escrIndex);
		qDebug ("[csi]    %d", csiIndex);
		qDebug ("[csiEnd] %d", csiEnd);

		if (curIndex == escqIndex) {			// if it is STRstart
		    if (curIndex != thisStart) {		// NOT at start of line
			thisEnd = escqIndex + 1;		// upto STRstart is the line
			data.insert (escqIndex, '\n');		// STRstart is a new line
		    }
		    else {					// IS at start of line
			if (escrIndex == -1) {			// no STRend
			    thisEnd = data.size ();		//  EVERYTHING is the string
			    //done = true;
			}
			else {
			    thisEnd = escrIndex + 1;		// is STRend
			    data.insert (escrIndex, '\n');	//  EVERYTHING to here
			}
		    }
		}
			
		else if (curIndex == escrIndex) {		// if it is STRend
		    thisEnd = curIndex + 7;
		    data.insert (curIndex + 6, '\n');
		}

		else if (curIndex == crlfIndex) {		// if it is CRLF
		    thisEnd = curIndex + 7;
		    data.insert (curIndex + 6, '\n');
		}

		else if (curIndex == lfIndex) {			// if it is LF
		    thisEnd = curIndex + 5;
		    data.insert (curIndex + 4, '\n');
		}

		else if (curIndex == crIndex) {			// if it is CR
		    thisEnd = curIndex + 5;
		    data.insert (curIndex + 4, '\n');
		}

		else if (curIndex == csiIndex) {		// if it is CSI
		    if (csiEnd == -1)				//    bare CSI
			thisEnd += 5;
		    else
			thisEnd += (csiEnd - csiIndex);		//    to CSIend
		}
		continue;
	    }
	    data.insert (thisEnd++, '\n');
if (thisEnd > 100)
break;
	} 	// EditLoop
    }		// if (screenWidth 1..n

    else {	// if (screenWidth 0
	/*-----------------------------------------------------------------------------------------
	 * screenWidth set to 0
	 * this code runs only when some width is reset to 0, not when a char is entered
	 * so restore the basic formatting
	 *---------------------------------------------------------------------------------------*/
	data.replace ("<CRLF>", "<CRLF>\n");
	data.replace ("<CR>", "<CR>\n");
	data.replace ("<LF>", "<LF>\n");
	data.replace ("<ESCQ>", "\n<ESCQ>");			// This can mess the start
	data.replace ("<ESCR>", "<ESCR>\n");

	if (data[0] == QChar ('\n'))				// ... so fix it
	    data.remove (0, 1);
    }

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
    plainTextEdit->insertPlainText ("<CSI>");
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
