#include <QtGui>
#include "font.h"
#include "conscriptor.h"
#include "importimage.h"

extern int debug;

/*-------------------------------------------------------------------------------------------------
 * constructor --
 *	theImage     -- original image
 *	displayImage -- original image scaled to (upto255 x upto255);
 *	refImage     -- image used to source fiddles. FixPAl FixNTSC Adjusts this image
 *	importImage  -- image (fiddled) to make bdf font
 *-----------------------------------------------------------------------------------------------*/
ImportImageDialog::ImportImageDialog(QWidget *parent)
    : QDialog(parent)
{
    QImage tmpImage;
    setupUi(this);

    b4font = qobject_cast<B4Font*>(parent);
    readSettings ();
    setLayout (horizontalLayout);

    QString imageFile = QFileDialog::getOpenFileName(this, tr ("read image"), "",
	tr ("images (*.bmp *.gif *.jpg *.jpeg *.png *.pbm *.pgm *.ppm *.tiff *.xbm *.xpm)"));
    theImage = new QImage (imageFile);

    if (theImage->isNull()) {
	b4font->conscriptor->statusBar()->showMessage("Null Image", 10000);
	QApplication::postEvent( this, new QCloseEvent() );
    }

    /*---------------------------------------------------------------------------------------------
     * image too big
     *-------------------------------------------------------------------------------------------*/
    if ((theImage->width() > 255) || (theImage->height() > 255)) {
	if (theImage->width() > theImage->height())
	   tmpImage = theImage->scaledToWidth(255, Qt::SmoothTransformation);
	else
	   tmpImage = theImage->scaledToHeight(255, Qt::SmoothTransformation);

	char str[32];
	sprintf (str, "Image scaled to %dx%d", tmpImage.width(), tmpImage.height());
	b4font->conscriptor->statusBar()->showMessage(str, 10000);
    }
    /*---------------------------------------------------------------------------------------------
     * Image big enough or too small: we leave it be
     *-------------------------------------------------------------------------------------------*/
    else {
	tmpImage = theImage->convertToFormat (QImage::Format_ARGB32, Qt::MonoOnly);
    }

    displayImage  = new QImage (tmpImage.width(), tmpImage.height(), QImage::Format_ARGB32);
    importImage   = new QImage (tmpImage.width(), tmpImage.height(), QImage::Format_ARGB32);
    refImage      = new QImage (tmpImage.width(), tmpImage.height(), QImage::Format_ARGB32);
    *displayImage = tmpImage.convertToFormat (QImage::Format_ARGB32, Qt::MonoOnly);
    *refImage     = tmpImage.convertToFormat (QImage::Format_ARGB32, Qt::MonoOnly);

    if ((debug & TRACEG)) {
	qDebug ("[theImage]     %dx%d:%d", theImage->width(),
						    theImage->height(), theImage->depth());
	qDebug ("[displayImage] %dx%d:%d", displayImage->width(),
						    displayImage->height(), displayImage->depth());
    }
    /*---------------------------------------------------------------------------------------------
     * convert displayImage and refImage to 32 bit all gray (R = G = B)
     *-------------------------------------------------------------------------------------------*/
    QRgb pixel;
    int  grey;
    QRgb result;
    for (int row = 0; row < displayImage->height(); row++)
	for (int col = 0; col < displayImage->width(); col++) {
	    pixel = displayImage->pixel(col, row);
	    grey = (((pixel >> 16) & 0xff) * 11 +
		    ((pixel >> 8)  & 0xff) * 16 +
		     (pixel & 0xff) * 5)
		   / 32;
	    result = ((grey << 16) | (grey << 8) | grey) | (pixel & 0xff000000);
            displayImage->setPixel (col, row, result);
            refImage->setPixel (col, row, result);
	}

    frame_1->setFrameStyle (QFrame::NoFrame);
    setPicture ();

    connect (radioBW,      SIGNAL(clicked()), this,   SLOT(setPicture ()));
    connect (buttonCancel, SIGNAL(clicked()), this,   SLOT(writeExit ()));
    connect (buttonOK,     SIGNAL(clicked()), this,   SLOT(writeSettings ()));
    connect (sliderWhite,  SIGNAL(valueChanged (int)), this,   SLOT(setPicture ()));
    connect (sliderBlack,  SIGNAL(valueChanged (int)), this,   SLOT(setPicture ()));
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonSaveImage_clicked --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_buttonSaveImage_clicked ()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), ".",
	    tr("Image File (*.jpg *.bmp *.jpeg *.ppm *.tiff *.xbm *.xpm)"));

    importImage->save (fileName);
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonInvertColor_clicked --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_buttonInvertColor_clicked ()
{
    if (radioBW->isChecked()) {
	importImage->invertPixels ();
    }
    else {
	QRgb opaquePix = b4font->conscriptor->opaqueColor.rgb ();
	QRgb clearPix  = b4font->conscriptor->clearColor.rgb ();
	int ww = importImage->width();
	int hh = importImage->height();
	for (int row = 0; row < hh; row++)
	    for (int col = 0; col < ww; col++) {
		if (importImage->pixel (col, row) == opaquePix)
		    importImage->setPixel (col, row, clearPix);
		 else if (importImage->pixel (col, row) == clearPix)
		    importImage->setPixel (col, row, opaquePix);
	    }
     }
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonFixPAL_clicked --
 *	both displayImage and refImage are change
 *	button is disabled - you can't do it again (no reason, just dumb to do so)
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_buttonFixPAL_clicked ()
{
int newWidth;
QImage tmpImportImage;
QImage tmpRefImage;

    buttonFixPAL->setEnabled (false);
    buttonFixNTSC->setEnabled (false);

    if ((debug & TRACEG))
	qDebug ("[FixPAL] ref:%dx%d", refImage->width(), refImage->height());
    newWidth = refImage->width() * 4 / 3;

    /*---------------------------------------------------------------------------------------------
     * Trim height
     *-------------------------------------------------------------------------------------------*/
    if (newWidth > 255) {
	int sameWidth = refImage->width();
	int newHeight = refImage->height() * 3 / 4;
	tmpImportImage = importImage->scaled(sameWidth, newHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	tmpRefImage = refImage->scaled(sameWidth, newHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if ((debug & TRACEG))
	    qDebug ("image scaled to %dx%d WxH", tmpImportImage.width(), tmpImportImage.height());
    }
    /*---------------------------------------------------------------------------------------------
     * OR Expand Width
     *-------------------------------------------------------------------------------------------*/
    else {
	int sameHeight = refImage->height();
	tmpImportImage = importImage->scaled(newWidth, sameHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	tmpRefImage = refImage->scaled(newWidth, sameHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if ((debug & TRACEG))
	    qDebug ("image scaled to %dx%d WxH", tmpImportImage.width(), tmpImportImage.height());
    }

    *importImage = tmpImportImage.copy (QRect (0,0,0,0));
    *refImage = tmpRefImage.copy (QRect (0,0,0,0));
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonFixNTSC_clicked --
 *	both displayImage and refImage are change
 *	button is disabled - you can't do it again (no reason, just dumb to do so)
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_buttonFixNTSC_clicked ()
{
int newWidth;
QImage tmpImportImage;
QImage tmpRefImage;

    buttonFixPAL->setEnabled (false);
    buttonFixNTSC->setEnabled (false);

    if ((debug & TRACEG))
	qDebug ("[FixNTSC] ref:%dx%d", refImage->width(), refImage->height());
    newWidth = refImage->width() * 3 / 2;

    /*---------------------------------------------------------------------------------------------
     * Trim height
     *-------------------------------------------------------------------------------------------*/
    if (newWidth > 255) {
	int sameWidth = refImage->width();
	int newHeight = refImage->height() * 2 / 3;
	tmpImportImage = importImage->scaled(sameWidth, newHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	tmpRefImage = refImage->scaled(sameWidth, newHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if ((debug & TRACEG))
	    qDebug ("image scaled to %dx%d WxH", tmpImportImage.width(), tmpImportImage.height());
    }
    /*---------------------------------------------------------------------------------------------
     * OR Expand Width
     *-------------------------------------------------------------------------------------------*/
    else {
	int sameHeight = refImage->height();
	tmpImportImage = importImage->scaled(newWidth, sameHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	tmpRefImage = refImage->scaled(newWidth, sameHeight,
						Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	if ((debug & TRACEG))
	    qDebug ("image scaled to %dx%d WxH", tmpImportImage.width(), tmpImportImage.height());
    }

    *importImage = tmpImportImage.copy (QRect (0,0,0,0));
    *refImage = tmpRefImage.copy (QRect (0,0,0,0));
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radio1BB_clicked --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_radio1BPP_clicked ()
{
    sliderWhite->setHidden (true);
    label_2->setHidden (true);
    setPicture ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radio2BB_clicked --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::on_radio2BPP_clicked ()
{
    sliderWhite->setHidden (false);
    label_2->setHidden (false);
    setPicture ();
}

/*-------------------------------------------------------------------------------------------------
 * setPicture --
 *	black or white level slider changed
 *	slider values on BLUE value. For gray images R == G == B
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::setPicture()
{
QRgb val;

    if ((debug & TRACEG))
	qDebug ("[setPicture]");
    int ww = importImage->width();
    int hh = importImage->height();
    QRgb opaquePix;
    QRgb cthruPix;
    QRgb clearPix;

    if (radioBW->isChecked()) {
	if (buttonInvertColor->isChecked()) {
	    opaquePix = 0xff000000;
	    cthruPix  = 0xff808080;
	    clearPix  = 0xffffffff;
	}
	else {
	    opaquePix = 0xffffffff;
	    cthruPix  = 0xff808080;
	    clearPix  = 0xff000000;
	}
    }
    else {
	if (buttonInvertColor->isChecked()) {
	    opaquePix = b4font->conscriptor->clearColor.rgb ();
	    cthruPix  = b4font->conscriptor->cthruColor.rgb ();
	    clearPix  = b4font->conscriptor->opaqueColor.rgb ();
	}
	else {
	    opaquePix = b4font->conscriptor->opaqueColor.rgb ();
	    cthruPix  = b4font->conscriptor->cthruColor.rgb ();
	    clearPix  = b4font->conscriptor->clearColor.rgb ();
	}
    }

    int black = sliderBlack->value();
    if ((radio1BPP->isChecked())) {			// 1 BPP
	for (int row = 0; row < hh; row++)
	    for (int col = 0; col < ww; col++) {
		val = refImage->pixel (col, row);
		if ((val & 0xff) > black)
		    importImage->setPixel (col, row, opaquePix);
		else
		    importImage->setPixel (col, row, clearPix);
	    }
	}
    else {						// 2 BPP
	int white = sliderWhite->value();
	for (int row = 0; row < hh; row++)
	    for (int col = 0; col < ww; col++) {
		val = refImage->pixel (col, row);
		if ((val & 0xff) > white) {
		   importImage->setPixel (col, row, opaquePix);
		}
		else if ((val & 0xff) > black) {
		    val &= 0xff000000;
		    importImage->setPixel (col, row, cthruPix);
		}
		else {
		    importImage->setPixel (col, row, clearPix);
		}
	    }
    }
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * readSettings --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::readSettings()
{
    QSettings settings("Decade", "Conscriptor");
    restoreGeometry(settings.value("ImportImageGeometry").toByteArray());
}

/*-------------------------------------------------------------------------------------------------
 * writeExit --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::writeExit()
{
    QSettings settings("Decade", "Conscriptor");
    settings.setValue("ImportImageGeometry", saveGeometry());
    reject ();
}

/*-------------------------------------------------------------------------------------------------
 * writeSettings --
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::writeSettings()
{
    QSettings settings("Decade", "Conscriptor");
    settings.setValue("ImportImageGeometry", saveGeometry());
    radioBW->setChecked (false);	//Set our colours not B&W for b4f file
    setPicture ();
    b4font->thePicture = importImage;
    accept ();
}

/*-------------------------------------------------------------------------------------------------
 * paintEvent --
 *	theImage is the original picture
 *	displayImage is 255x255 grey version
 *	importImage is 255x255 1/2 color + settings
 *-----------------------------------------------------------------------------------------------*/
void ImportImageDialog::paintEvent(QPaintEvent *event)
{
    float scale;
    //qDebug ("PE");

    QPainter painter(this);
    QRect r = frame_1->frameRect ();

    /*---------------------------------------------------------------------------------------------
     * theImage
     *-------------------------------------------------------------------------------------------*/
    scale = qMax ((2 * (float)(theImage->width())) / (float)r.width(),
		  (float)(theImage->height()) / (float)r.height());
    int theW = theImage->width() / scale;
    int theH = theImage->height() / scale;

    /*---------------------------------------------------------------------------------------------
     * importImage
     *-------------------------------------------------------------------------------------------*/
    scale = qMax ((2 * (float)(importImage->width())) / (float)r.width(),
		  (float)(importImage->height()) / (float)r.height());
    int importW = importImage->width() / scale;
    int importH = importImage->height() / scale;

   /*----------------------------------------------------------------------------------------------
    * Draw
    qDebug ("draWbox: %dx%d\n"
	    "original:%dx%d scale:%f\n"
	    "displayI:%dx%d",
			    r.width(), r.height(),
			    theImage->width(), theImage->height(), scale,
			    w, h);
    *--------------------------------------------------------------------------------------------*/ 

    painter.drawImage (QRect (r.x(), r.y(), theW, theH), (*theImage));
    painter.drawImage (QRect (r.width() / 2, r.y(), importW, importH), (*importImage));

    //qDebug ("/PE");
}
