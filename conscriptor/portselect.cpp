#include <QtGui>
#include "portselect.h"
#include "conscriptor.h"

PortSelectDialog::PortSelectDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);

    conscriptor = qobject_cast<Conscriptor*>(parent);
# ifdef _WIN32
    radioTty1->setText (tr ("COM1"));
    radioTty2->setText (tr ("COM2"));
    radioTty3->setText (tr ("COM3"));
    radioTty4->setText (tr ("COM4"));
    radioTty5->setText (tr ("COM5"));
    radioTty6->setText (tr ("COM6"));
    radioTty7->setText (tr ("COM7"));
    radioTty8->setText (tr ("COM8"));
    radioTty9->setText (tr ("COM9"));
# else
    radioTty1->setText (tr ("ttyS0"));
    radioTty2->setText (tr ("ttyS1"));
    radioTty3->setText (tr ("ttyS2"));
    radioTty4->setText (tr ("ttyS3"));
    radioTty5->setText (tr ("ttyUSB0"));
    radioTty6->setText (tr ("ttyUSB1"));
    radioTty7->setText (tr ("ttyUSB2"));
    radioTty8->setText (tr ("ttyUSB3"));
    radioTty9->setText (tr ("ttys000"));
# endif
    connect (buttonOK, SIGNAL(clicked()), this,   SLOT(accept ()));
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty1 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty1_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyS0";
# else
    conscriptor->portName = "COM1";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty2 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty2_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyS1";
# else
    conscriptor->portName = "COM2";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty3 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty3_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyS2";
# else
    conscriptor->portName = "COM3";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty4 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty4_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyS3";
# else
    conscriptor->portName = "COM4";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty5 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty5_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyUSB0";
# else
    conscriptor->portName = "COM5";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty6 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty6_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyUSB1";
# else
    conscriptor->portName = "COM6";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty7 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty7_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyUSB2";
# else
    conscriptor->portName = "COM7";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty8 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty8_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttyUSB3";
# else
    conscriptor->portName = "COM8";
# endif
    conscriptor->on_actionTtyQuery ();
}

/*-------------------------------------------------------------------------------------------------
 * on_radioTty9 --
 *-----------------------------------------------------------------------------------------------*/
void PortSelectDialog::on_radioTty9_pressed ()
{
# ifndef _WIN32
    conscriptor->portName = "/dev/ttys000";
# else
    conscriptor->portName = "COM9";
# endif
    conscriptor->on_actionTtyQuery ();
}
