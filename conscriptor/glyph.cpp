#include "glyph.h"
#include "conscriptor.h"
#include <QDebug>
#include <QPainter>

class B4Font;

/*-------------------------------------------------------------------------------------------------
 * constructor --
 *-----------------------------------------------------------------------------------------------*/
Glyph::Glyph (Conscriptor *p)
{
   gbox = 0;
   conscriptor = p;
}

/*-------------------------------------------------------------------------------------------------
 * destructor --
 *-----------------------------------------------------------------------------------------------*/
Glyph::~Glyph ()
{
   delete (gbox);
}

/*-------------------------------------------------------------------------------------------------
 * setXY -- set screen co-ords of the glypf
 *      called  when glypfs are allocated
 *              after change macro magnification
 *              after edit chages height of the font set
 *-----------------------------------------------------------------------------------------------*/
void Glyph::setXY(int x, int y, int lineLength, int viewportWidth)
{
    //qDebug ("x[%3d] y[%3d]  max[%d] vp[%d]", x, y, lineLength, viewportWidth);
    X = x;
    Y = y;
    width  = (gbox->width()  * viewportWidth) / lineLength;
    height = (gbox->height() * viewportWidth) / lineLength;
}

/*-------------------------------------------------------------------------------------------------
 * outline --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::outline (QRgb clear, QRgb opaque, QRgb cthru)
{
    int w = gbox->width();
    int h = gbox->height();
    for (int x = 0; x < w; x++)
	for (int y = 0; y < h; y++) {
	    if (gbox->pixel (x, y) == clear)
		continue;
	    if (gbox->pixel (x, y) == opaque) {
		if ((y != 0) && (gbox->pixel (x, y-1) == clear))	// all but top row
		    gbox->setPixel (x, y-1, cthru);
		if ((y != (h -1)) && (gbox->pixel (x, y+1) == clear))	// all but bottom row
		    gbox->setPixel (x, y+1, cthru);
		if ((x != 0) && (gbox->pixel (x -1, y) == clear))	// all but left col
		    gbox->setPixel (x -1, y, cthru);
		if ((x != (w -1)) && (gbox->pixel (x +1, y) == clear))	// all but right col
		    gbox->setPixel (x +1, y, cthru);
	    }
	}
}

/*-------------------------------------------------------------------------------------------------
 * setPixel --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::setPixel (int x, int y, QRgb color)
{
    gbox->setPixel (x, y, color);
}

/*-------------------------------------------------------------------------------------------------
 * changeColor --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::changeColor (QRgb was, QRgb is)
{
    for (int x = 0; x < gbox->width(); x++)
	for (int y = 0; y < gbox->height(); y++)
	    if (gbox->pixel (x, y) == was)
		gbox->setPixel (x, y, is);
}

/*-------------------------------------------------------------------------------------------------
 * newGlyph --
 *      called  when glypfs are allocated on file read
 *-----------------------------------------------------------------------------------------------*/
void Glyph::newGlyph (int w, int h, char *cp, int pixWidth, int cno)
{
    gbox = new QImage (w, h, QImage::Format_RGB32);

    for (int hh=0; hh < h; cp += pixWidth, hh++)
        for (int ww=0; ww < w; ww++)
            if (*(cp + ww) == '-')
                gbox->setPixel (ww, hh, conscriptor->opaqueColor.rgb());
            else if (*(cp + ww) == '#')
                gbox->setPixel (ww, hh, conscriptor->cthruColor.rgb());
            else
                gbox->setPixel (ww, hh, conscriptor->clearColor.rgb());
    charNo = cno;
    visible = true;
}

/*-------------------------------------------------------------------------------------------------
 * newGlyph --
 *      called when a new blank glypf is inserted
 *	called for new glyphs (file->new)
 *-----------------------------------------------------------------------------------------------*/
void Glyph::newGlyph (int w, int h, int cno)
{
    gbox = new QImage (w, h, QImage::Format_RGB32);
    gbox->fill (conscriptor->clearColor.rgb());
    //qDebug ("[glypf] newGlyph w%d h%d c%d", w, h, cno);
    charNo = cno;
    visible = true;
}

/*-------------------------------------------------------------------------------------------------
 * columnLeft --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::columnLeft ()
{
int scol;
int dcol;
    int pixelWidth = gbox->width ();
    int pixelHeight = gbox->height ();

    for (int row = 0; row < pixelHeight; row++) {
	for (dcol = 0, scol = 1; scol < pixelWidth; dcol++, scol++)
	    gbox->setPixel (dcol, row, gbox->pixel (scol, row));
	gbox->setPixel (dcol, row, conscriptor->clearColor.rgb());
    }
}

/*-------------------------------------------------------------------------------------------------
 * columnRight --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::columnRight ()
{
int scol;
int dcol;
    int pixelWidth = gbox->width ();
    int pixelHeight = gbox->height ();

    for (int row = 0; row < pixelHeight; row++) {
	for (dcol = pixelWidth-1, scol = pixelWidth-2; dcol > 0; dcol--, scol--)
	    gbox->setPixel (dcol, row, gbox->pixel (scol, row));
	gbox->setPixel (dcol, row, conscriptor->clearColor.rgb());
    }
}

/*-------------------------------------------------------------------------------------------------
 * rowUp --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::rowUp ()
{
int srow;
int drow;
    int pixelWidth = gbox->width ();
    int pixelHeight = gbox->height ();

    for (int col = 0; col < pixelWidth; col++) {
	for (drow = 0, srow = 1; srow < pixelHeight; drow++, srow++)
	    gbox->setPixel (col, drow, gbox->pixel (col, srow));
	gbox->setPixel (col, drow, conscriptor->clearColor.rgb());
    }
}

/*-------------------------------------------------------------------------------------------------
 * rowDown --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::rowDown ()
{
int srow;
int drow;
    int pixelWidth = gbox->width ();
    int pixelHeight = gbox->height ();

    for (int col = 0; col < pixelWidth; col++) {
	for (drow = pixelHeight-1, srow = pixelHeight-2; drow > 0; drow--, srow--)
	    gbox->setPixel (col, drow, gbox->pixel (col, srow));
	gbox->setPixel (col, drow, conscriptor->clearColor.rgb());
    }
}

/*-------------------------------------------------------------------------------------------------
 * deleteColumn --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::deleteColumn ()
{
    if (gbox->width() > 1)
	(*gbox) = gbox->copy (0, 0, gbox->width() - 1, gbox->height());
}

/*-------------------------------------------------------------------------------------------------
 * deleteRow --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::deleteRow ()
{
    if (gbox->height() > 1)
	(*gbox) = gbox->copy (0, 0, gbox->width(), gbox->height() -1);
}

/*-------------------------------------------------------------------------------------------------
 * insertColumn --
 *	after inserting column pixelWidth IS right hand column
 *-----------------------------------------------------------------------------------------------*/
void Glyph::insertColumn ()
{
int pixelWidth = gbox->width ();
int pixelHeight = gbox->height ();

    if (pixelWidth < 254) {
	(*gbox) = gbox->copy (0, 0, pixelWidth + 1, pixelHeight);
	for (int row = 0; row < pixelHeight; row++)
	    gbox->setPixel (pixelWidth, row, conscriptor->clearColor.rgb());	    
    }
}

/*-------------------------------------------------------------------------------------------------
 * insertRow --
 *	after inserting row pixelHeight IS last row
 *-----------------------------------------------------------------------------------------------*/
void Glyph::insertRow ()
{
int pixelWidth = gbox->width ();
int pixelHeight = gbox->height ();

    if (pixelHeight < 254)
	(*gbox) = gbox->copy (0, 0, gbox->width(), gbox->height() +1);
	for (int col = 0; col < pixelWidth; col++)
	    gbox->setPixel (col, pixelHeight, conscriptor->clearColor.rgb());	    
}

/*-------------------------------------------------------------------------------------------------
 * setCodePoint --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::setCodePoint (int n)
{
    //qDebug ("set CP %d", n);
    charNo = n - conscriptor->header.chFirst;
}

/*-------------------------------------------------------------------------------------------------
 * getCharNo --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getCharNo ()
{
    //qDebug ("get C# %d", charNo);
    return charNo;
}

/*-------------------------------------------------------------------------------------------------
 * getCodePoint --
 *-----------------------------------------------------------------------------------------------*/
unsigned int Glyph::getCodePoint ()
{
    //qDebug ("get CP %d", charNo + conscriptor->header.chFirst);
    return charNo + conscriptor->header.chFirst;
}

/*-------------------------------------------------------------------------------------------------
 * getPixel --
 *-----------------------------------------------------------------------------------------------*/
QRgb Glyph::getPixel (int x, int y)
{
   return gbox->pixel (x, y);
}

/*-------------------------------------------------------------------------------------------------
 * getPixWidth --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getPixWidth ()
{
    return gbox->width ();
}

/*-------------------------------------------------------------------------------------------------
 * getPixHeight --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getPixHeight ()
{
    return gbox->height ();
}

/*-------------------------------------------------------------------------------------------------
 * getX --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getX ()
{
   return X;
}

/*-------------------------------------------------------------------------------------------------
 * getY --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getY ()
{
   return Y;
}

/*-------------------------------------------------------------------------------------------------
 * getWidth --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getWidth ()
{
   return width;
}

/*-------------------------------------------------------------------------------------------------
 * getHeight --
 *-----------------------------------------------------------------------------------------------*/
int Glyph::getHeight ()
{
   return height;
}

/*-------------------------------------------------------------------------------------------------
 * setImage --
 *-----------------------------------------------------------------------------------------------*/
void Glyph::setImage (QImage &pic)
{
   (*gbox) = pic;
}

/*-------------------------------------------------------------------------------------------------
 * getImage --
 *-----------------------------------------------------------------------------------------------*/
QImage &Glyph::getImage ()
{
   return (*gbox);
}
