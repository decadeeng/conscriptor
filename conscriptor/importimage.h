#ifndef IMPORTIMAGE_H
#define IMPORTIMAGE_H

#include <QDialog>

#include "ui_importimage.h"
#include "font.h"

class ImportImageDialog : public QDialog, public Ui::ImportImageDialog
{
    Q_OBJECT

public:
    ImportImageDialog(QWidget *parent = 0);

protected:
    void paintEvent(QPaintEvent *event);
    //void resizeEvent(QResizeEvent *event);

private:
    B4Font *b4font;
    void readSettings ();
    int viewportWidth;
    int viewportHeight;
    QImage *theImage;			// original picture
    QImage *displayImage;		// original scaled to (255x255)
    QImage *refImage;			// original used for BW slider values. Adjusted by FixAspect
    QImage *importImage;		// image displayed and to be imported

private slots:
    void writeSettings ();
    void writeExit ();
    void setPicture ();
    void on_radio1BPP_clicked ();
    void on_radio2BPP_clicked ();
    void on_buttonSaveImage_clicked ();
    void on_buttonInvertColor_clicked ();
    void on_buttonFixPAL_clicked ();
    void on_buttonFixNTSC_clicked ();

};

#endif

