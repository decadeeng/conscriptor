#include <QtGui>
#include "setup.h"

SetupDialog::SetupDialog(QWidget *parent)
    : QDialog(parent)
{
    conscriptor = qobject_cast<Conscriptor*>(parent);
    setupUi(this);

    conscriptor->isBOB ();
    buttonDownload->setEnabled (false);

    connect (buttonExit, SIGNAL(clicked()), this, SLOT(accept ()));
    connect (bootEdit,   SIGNAL(textChanged (const QString &)), this, SLOT(textChanged ()));
    connect (configEdit, SIGNAL(textChanged (const QString &)), this, SLOT(textChanged ()));
    connect (fontEdit,   SIGNAL(textChanged (const QString &)), this, SLOT(textChanged ()));
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadFile_clicked --
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonLoadFile_clicked ()
{
QByteArray ba;
int bdx;
int cdx;
int fdx;

    QString fileName = QFileDialog::getOpenFileName(this, "List of setup files", "",
									    "setup (*.b4s)");
    if (fileName.isEmpty() == true)
	return;

    QFile file (fileName);
    if (!file.open(QIODevice::ReadWrite)) {
	QMessageBox::warning(this, tr ("conscriptor"),
		tr ("Cannot read file %1 %2.")
		.arg(fileName)
		.arg(file.errorString()));
	return;
    }

    ba = file.readAll ();
    file.close ();
    bdx = ba.indexOf ("boot=") + 5;
    cdx = ba.indexOf ("config=") + 7;
    fdx = ba.indexOf ("font=") + 5;

    //qDebug () << "index b c f" << bdx << cdx << fdx;
    /*---------------------------------------------------------------------------------------------
     *  bdx < (cdx | fdx)
     *-------------------------------------------------------------------------------------------*/
    if ((bdx < cdx) && (bdx < fdx)) {
	if (cdx < fdx) {
	    bootEdit->setText   (ba.mid (bdx, cdx - bdx -7));
	    configEdit->setText (ba.mid (cdx, fdx - cdx -5));
	    fontEdit->setText   (ba.mid (fdx));
	}
	else {
	    bootEdit->setText   (ba.mid (bdx, fdx - bdx -5));
	    configEdit->setText (ba.mid (cdx));
	    fontEdit->setText   (ba.mid (fdx, cdx - fdx -7));
	}
    }
    /*---------------------------------------------------------------------------------------------
     * cdx < (bdx | fdx)
     *-------------------------------------------------------------------------------------------*/
    else if ((cdx < bdx) && (cdx < fdx)) {
	if (bdx < fdx) {
	    bootEdit->setText   (ba.mid (bdx, fdx - bdx -5));
	    configEdit->setText (ba.mid (cdx, bdx - cdx -5));
	    fontEdit->setText   (ba.mid (fdx));
	}
	else {
	    bootEdit->setText   (ba.mid (bdx));
	    configEdit->setText (ba.mid (cdx, fdx - cdx -5));
	    fontEdit->setText   (ba.mid (fdx, bdx - fdx -5));
	}
    }
    /*---------------------------------------------------------------------------------------------
     * fdx < (bdx | cdx)
     *-------------------------------------------------------------------------------------------*/
    else {
	if (bdx < cdx) {
	    bootEdit->setText   (ba.mid (bdx, cdx - bdx -7));
	    configEdit->setText (ba.mid (cdx));
	    fontEdit->setText   (ba.mid (fdx, bdx - fdx -5));
	}
	else {
	    bootEdit->setText   (ba.mid (bdx));
	    configEdit->setText (ba.mid (cdx, bdx - cdx -5));
	    fontEdit->setText   (ba.mid (fdx, cdx - fdx -7));
	}
    }
    if (conscriptor->commPort == Conscriptor::NONE)
	buttonDownload->setEnabled (false);
    else
	buttonDownload->setEnabled (true);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadB4B_clicked -- bootscript
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonLoadB4B_clicked ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load BOB-4 bootscipt files", "",
									"bootscript (*.b4j)");
    if (fileName.isEmpty() == false)
	bootEdit->setText (fileName);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadB4C_clicked -- config
 *	fileName.remove (0, fileName.lastIndexOf("/") +1);
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonLoadB4C_clicked ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load BOB-4 config files", "",
									    "config (*.b4c)");
    if (fileName.isEmpty() == false)
	configEdit->setText (fileName);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonLoadB4F_clicked -- font
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonLoadB4F_clicked ()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Load BOB-4 font files", "",
									    "font (*.b4f)");
    if (fileName.isEmpty() == false)
	fontEdit->setText (fileName);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonSaveFile_clicked --
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonSaveFile_clicked ()
{
    QByteArray ba = "Font=";
    ba += fontEdit->text ();
    ba += "\nConfig=";
    ba += configEdit->text ();
    ba += "\nBoot=";
    ba += bootEdit->text ();
    ba += '\n';

    QString fileName = QFileDialog::getSaveFileName(this, "setup save file name", "",
									    "setup (*.b4s)");
    if (!fileName.endsWith (".b4s"))
	fileName.append (".b4s");

    QFile file (fileName);;
    if (!file.open(QIODevice::WriteOnly)) {
	QMessageBox::warning(this, tr ("setup file"),
		tr ("Cannot create file %1:%2.")
		.arg(fileName)
		.arg(file.errorString()));
	return;
    }

    file.write (ba);
    file.close ();
}

/*-------------------------------------------------------------------------------------------------
 * textChanged --
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::textChanged ()
{
    if (conscriptor->commPort != Conscriptor::NONE)
	buttonDownload->setEnabled (true);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonDownload_clicked --
 *-----------------------------------------------------------------------------------------------*/
void SetupDialog::on_buttonDownload_clicked ()
{
bool errs = false;

    QByteArray ba;

    if (conscriptor->isBOB () == false) {
	QMessageBox::warning(this, tr("Conscriptor"),
				   tr("No Conscriptor detected"));
	return;
    }

    QString bootName   = bootEdit->text ();
    QString configName = configEdit->text ();
    QString fontName   = fontEdit->text ();

    if ((bootName.endsWith ("\r\n")) || (bootName.endsWith ("\n\r")))
	bootName.chop (2);
    else if (bootName.endsWith ("\n"))
	bootName.chop (1);

    if ((configName.endsWith ("\r\n")) || (configName.endsWith ("\n\r")))
	configName.chop (2);
    else if (configName.endsWith ("\n"))
	configName.chop (1);

    if ((fontName.endsWith ("\r\n")) || (fontName.endsWith ("\n\r")))
	fontName.chop (2);
    else if (fontName.endsWith ("\n"))
	fontName.chop (1);

    if (conscriptor->commPort == Conscriptor::XBOBP) {
	if (!fontName.isEmpty ()) {
	    if (conscriptor->font_main_port (fontName) == false)
		errs = true;
	}
    }
    else if (conscriptor->commPort == Conscriptor::DEBUGP) {
	if (!fontName.isEmpty ()) {
	    if (conscriptor->font_debug_port (fontName) == false)
		errs = true;
	}
    }

    if (!bootName.isEmpty ()) { 
	QFile file (bootName);
	if (!file.open(QIODevice::ReadWrite)) {
	    QMessageBox::warning(this, tr ("conscriptor"),
		    tr ("Cannot read file %1 %2.")
		    .arg(bootName)
		    .arg(file.errorString()));
	}
	else {
	    ba = file.readAll ();
	    file.close ();
	    if (conscriptor->bootscriptDownload (ba) == false)
		errs = true;
	}
    }

    if (!configName.isEmpty ()) {
	QFile file (configName);
	if (!file.open(QIODevice::ReadWrite)) {
	    QMessageBox::warning(this, tr ("conscriptor"),
		    tr ("Cannot read file %1 %2.")
		    .arg(configName)
		    .arg(file.errorString()));
	}
	else {
	    ba = file.readAll ();
	    file.close ();
	    if (conscriptor->configDownload (ba) == false)
		errs = true;
	}
    }
    buttonDownload->setEnabled (false);
}
