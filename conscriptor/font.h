#ifndef B4FONT_H
#define B4FONT_H

#include <QColor>
#include <QImage>
#include <QPoint>
#include <QWidget>
#include <QPrinter>

#include "conf.h"

class Conscriptor;
class Glyph;

class B4Font : public QWidget
{
    Q_OBJECT

public:
    B4Font(QWidget *parent = 0);	// defaults to 0, you may set one

    char utf[32];			// utf result
    int value;				// change CodePoint result

    int lineLength;
    int viewportWidth;
    int viewportHeight;

    int nGlyphs;			// Number of glyphs
    int firstGlyph;			// first row first glyph: glyph number

    int macroGlyph;			// which glyph is selected for macro
    int macroWidth;			// width of macro image
    int macroHeight;			// hright of macro image
    int macroMagnification;		// magnification of macro image

    enum colortag {clearCol, opaqueCol, cthruCol};
    colortag colorFlag;

    char *amBase;
    Conscriptor *conscriptor;

    void cleanup ();
    void pix2asc ();
    void setupGlyphs ();
    void resetXY ();
    void resetColor (QRgb was, QRgb is);
    void makePixmap ();
    void makeAsciiMap ();
    void setNewGlyphs(int w, int h, int n);
    void findWidth ();
    void cp2utf (int n);
    void setPixmapWidth ();
    void setPixmapCount ();
    void setMetricCount ();

    void testG (int ch);
    void wheelEvent (QWheelEvent * event);

    bool editGlyph (int x, int y);
    int selectGlyph (int x, int y);

    Glyph *allglyphs[MAXGLYPHS];
    QImage *thePicture;
    
    QPrinter printer;

public slots:
    void on_actionImportImage ();
    void on_buttonImportBDF ();
    void on_actionCodePoint ();
    void on_actionRollUp ();
    void on_actionRollDown ();
    void on_actionMoveDown ();
    void on_actionMoveUp ();
    void on_actionMoveRight ();
    void on_actionMoveLeft ();
    void on_actionCut ();
    void on_actionCopy ();
    void on_actionPaste ();
    void on_actionClear ();
    void on_actionInsertRow ();
    void on_actionInsertColumn ();
    void on_actionInsertChar ();
    void on_actionInsertOutline ();
    void on_actionDeleteRow ();
    void on_actionDeleteColumn ();
    void on_actionDeleteChar ();
    void on_actionOutline ();
    void on_buttonPrintScreen ();
    void on_buttonPrintAll ();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
private:
    void resizeImage(QImage *image, const QSize &newSize);
    bool getbit (int col);
    void sortGlyphs ();

    QImage image;
    QImage pasteBuffer;
//    char *bitmapPtr;
//    int   bitmapCount;
    char  bitmapVal;

    QString bdfFile;
    int tmpIndx;			// where in the file the data is
};

#endif
