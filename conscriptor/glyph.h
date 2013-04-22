#ifndef GLYPH_H
#define GLYPH_H

#include <QImage>

class Conscriptor;

class Glyph
{

public:
    Glyph (Conscriptor *p);
    ~Glyph ();
    void changeColor (QRgb was, QRgb is);
    void outline (QRgb clear, QRgb opaque, QRgb cthru);
    void columnLeft ();
    void columnRight ();
    void deleteColumn ();
    void deleteRow ();
    unsigned int getCodePoint ();
    int getCharNo ();
    QImage &getImage ();
    int getX ();
    int getY ();
    int getWidth ();
    int getHeight ();
    int getPixWidth ();
    int getPixHeight ();
    QRgb getPixel (int x, int y);
    void insertColumn ();
    void insertRow ();
    void newGlyph  (int w, int h, char *cp, int pixWidth, int cno);
    void newGlyph  (int w, int h, int cno);
    void rowDown ();
    void rowUp ();
    void setCodePoint (int n);
    void setImage (QImage &pic);
    void setPixel (int x, int y, QRgb color);
    void setupGlyph ();
    void setXY  (int x, int y, int lineLength, int viewportWidth);

    bool visible;

private:
    int charNo;
    int width;
    int height;
    int X;
    int Y; 
    QImage *gbox;
    Conscriptor *conscriptor;
};

#endif

