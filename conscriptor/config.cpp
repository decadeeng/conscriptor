#include <QtGui>
#include "config.h"
#include "conscriptor.h"

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi(this);
    conscriptor = qobject_cast<Conscriptor*>(parent);

    if (conscriptor->isBOB () == true)
	buttonDownload->setEnabled (true);
    else
	buttonDownload->setEnabled (false);

    connect (buttonCancel,     SIGNAL(clicked()), this, SLOT (exitCE ()));
    connect (checkRestrictOverscan, SIGNAL (stateChanged (int)), this, SLOT (setModified ()));

    connect (checkVideoSrc,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkString,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkBOB4only,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkOverlay,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkHighPixRate,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkBlink,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));
    connect (checkEchoOn,	SIGNAL (stateChanged (int)), this, SLOT (setModified ()));

    connect (spinDisp1,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDisp2,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDisp3,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDispZ1,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDispZ2,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDispZ3,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDispZ4,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinDispZ5,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinBlink1,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinBlink2,	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinCom2,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinCom4,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinCom1,		SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinCom3,	 	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));
    connect (spinCom5,	 	SIGNAL (valueChanged (int)), this, SLOT (setModified ()));

    connect (radioNTSC,		SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioProgressive,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioLocal,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioCRLF0,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioDefault,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioNoReversal,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radio320x240,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioL0Low,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioL1Low,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioL2Low,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioL3Low,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioFBLow,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioSP1None,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioSP3None,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioSP0None,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioSP2None,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radio1Stop,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioParNone,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radio7Bit,		SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioSPImode0,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));
    connect (radioFlowNone,	SIGNAL (toggled (bool)), this, SLOT (setModified ()));

    modified = false;
}

/*------------------------------------------------------------------------------------------------
 * on_buttonLoadFile_clicked --
 *----------------------------------------------------------------------------------------------*/
void ConfigDialog::on_buttonLoadFile_clicked ()
{
    int val;
    int indx;

    curFile = QFileDialog::getOpenFileName(this, tr ("read config files"), "", tr ("(*.b4c)"));
    if (curFile.isEmpty())
        return;

    QFile file(curFile);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this, tr ("consciptor"),
		tr ("Cannot read file %1 %2.")
		.arg(curFile)
		.arg(file.errorString()));
        return;
    }

    ba = file.readAll ();

    /*--------------------------------
     * Mode
     *------------------------------*/
    if ((ba.contains ("[9;0v")))
	checkBOB4only->setChecked (false);
    else if ((ba.contains ("[9;1v")))
	checkBOB4only->setChecked (true);
	
    if ((ba.contains ("[10;0v")))
	checkString->setChecked (false);
    else if ((ba.contains ("[10;1v")))
	checkString->setChecked (true);
	
    if ((ba.contains ("[11;0v")))
	radioCRLF0->setChecked (true);
    else if ((ba.contains ("[11;1v")))
	radioCRLF1->setChecked (true);
    else if ((ba.contains ("[11;2v")))
	radioCRLF2->setChecked (true);
    else if ((ba.contains ("[11;3v")))
	radioCRLF3->setChecked (true);
	
    if ((ba.contains ("[16;0v")))
	radioNTSC->setChecked (true);
    else if ((ba.contains ("[16;1v")))
	radioPAL->setChecked (true);
	
    if ((ba.contains ("[17;0v")))
	radioProgressive->setChecked (true);
    else if ((ba.contains ("[17;1v")))
	radioInterlaced->setChecked (true);
	
    if ((ba.contains ("[18;1v")))
	radioLocal->setChecked (true);
    else if ((ba.contains ("[18;2v")))
	radioExternal->setChecked (true);
    else if ((ba.contains ("[18;3v")))
	radioAuto->setChecked (true);
	
    if ((ba.contains ("[19;0v")))
	checkVideoSrc->setChecked (false);
    else if ((ba.contains ("[19;1v")))
	checkVideoSrc->setChecked (true);
	
    if ((ba.contains ("[28;0v")))
	checkOverlay->setChecked (false);
    else if ((ba.contains ("[28;1v")))
	checkOverlay->setChecked (true);
    /*--------------------------------
     * Display
     *------------------------------*/
    indx = ba.indexOf ("[29;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinDisp1->setValue (val);
    
    indx = ba.indexOf ("[30;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinDisp2->setValue (val / 1000);
    
    indx = ba.indexOf ("[31;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinDisp3->setValue (val / 1000);
    
    if ((ba.contains ("[20;0v")))
	checkHighPixRate->setChecked (false);
    else if ((ba.contains ("[20;1v")))
	checkHighPixRate->setChecked (true);
	
    if ((ba.contains ("[21;0v")))
	checkRestrictOverscan->setChecked (false);
    else if ((ba.contains ("[21;1v")))
	checkRestrictOverscan->setChecked (true);
	
    if ((ba.contains ("[22;0v")))
        radio320x240->setChecked (true);
    else if ((ba.contains ("[22;1v")))
        radio384x288->setChecked (true);
    else if ((ba.contains ("[22;2v")))
        radio480x240->setChecked (true);
    else if ((ba.contains ("[22;3v")))
        radio480x288->setChecked (true);

    if ((ba.contains ("[23;0v")))
	radioDefault->setChecked (true);
    else if ((ba.contains ("[23;1v")))
	radio5MHz->setChecked (true);
    else if ((ba.contains ("[23;2v")))
	radio5MHz375->setChecked (true);
    else if ((ba.contains ("[23;3v")))
	radio5MHz769->setChecked (true);
    else if ((ba.contains ("[23;4v")))
	radio6MHz25->setChecked (true);
    else if ((ba.contains ("[23;5v")))
	radio6MHz818->setChecked (true);
    else if ((ba.contains ("[23;6v")))
	radio7MHz5->setChecked (true);
    else if ((ba.contains ("[23;7v")))
	radio8MHz33->setChecked (true);
    else if ((ba.contains ("[23;8v")))
	radio9MHz375->setChecked (true);

    if ((ba.contains ("[35;0v")))
        radioNoReversal->setChecked (true);
    else if ((ba.contains ("[35;1v")))
        radioReverseHorz->setChecked (true);
    else if ((ba.contains ("[35;2v")))
	radioReverseVert->setChecked (true);
    else if ((ba.contains ("[35;3v")))
        radioReverseHV->setChecked (true);

    /*--------------------------------
     * Display Size
     *------------------------------*/
    indx = ba.indexOf ("[24;");
    if (ba.data()[indx+5] == '<') {
	sscanf (ba.data() + indx + 6, "%dv", &val);
	spinDispZ1->setValue (-val);
    }
    else {
	sscanf (ba.data() + indx + 5, "%dv", &val);
	spinDispZ1->setValue (val);
    }
    
    indx = ba.indexOf ("[25;");
    if (ba.data()[indx+5] == '<') {
	sscanf (ba.data() + indx + 6, "%dv", &val);
	spinDispZ2->setValue (-val);
    }
    else {
	sscanf (ba.data() + indx + 5, "%dv", &val);
	spinDispZ2->setValue (val);
    }
    
    indx = ba.indexOf ("[26;");
    if (ba.data()[indx+5] == '<') {
	sscanf (ba.data() + indx + 6, "%dv", &val);
	spinDispZ3->setValue (-val);
    }
    else {
	sscanf (ba.data() + indx + 5, "%dv", &val);
	spinDispZ3->setValue (val);
    }
    
    indx = ba.indexOf ("[27;");
    if (ba.data()[indx+5] == '<') {
	sscanf (ba.data() + indx + 6, "%dv", &val);
	spinDispZ4->setValue (-val);
    }
    else {
	sscanf (ba.data() + indx + 5, "%dv", &val);
	spinDispZ4->setValue (val);
    }
    
    indx = ba.indexOf ("[36;");
    if (ba.data()[indx+5] == '<') {
	sscanf (ba.data() + indx + 6, "%dv", &val);
	spinDispZ5->setValue (-val);
    }
    else {
	sscanf (ba.data() + indx + 5, "%dv", &val);
	spinDispZ5->setValue (val);
    }
    
    /*--------------------------------
     * Aux Output
     *------------------------------*/
    if ((ba.contains ("[48;0v")))
        radioL0Low->setChecked (true);
    else if ((ba.contains ("[48;1v")))
        radioL0High->setChecked (true);
    else if ((ba.contains ("[48;2v")))
        radioL0CompSync->setChecked (true);
    else if ((ba.contains ("[48;3v")))
        radioL0VertSync->setChecked (true);

    if ((ba.contains ("[49;0v")))
        radioL1Low->setChecked (true);
    else if ((ba.contains ("[49;1v")))
        radioL1High->setChecked (true);
    else if ((ba.contains ("[49;2v")))
        radioL1HorzSync->setChecked (true);
    else if ((ba.contains ("[49;3v")))
        radioL1VertSync->setChecked (true);

    if ((ba.contains ("[50;0v")))
        radioL2Low->setChecked (true);
    else if ((ba.contains ("[50;1v")))
        radioL2High->setChecked (true);
    else if ((ba.contains ("[50;2v")))
        radioL2HorzSync->setChecked (true);
    else if ((ba.contains ("[50;3v")))
        radioL2CompSync->setChecked (true);

    if ((ba.contains ("[51;0v")))
        radioL3Low->setChecked (true);
    else if ((ba.contains ("[51;1v")))
        radioL3High->setChecked (true);
    else if ((ba.contains ("[51;2v")))
        radioL3CompSync->setChecked (true);
    else if ((ba.contains ("[51;3v")))
        radioL3VertBlank->setChecked (true);

    if ((ba.contains ("[52;0v")))
        radioFBLow->setChecked (true);
    else if ((ba.contains ("[52;1v")))
        radioFBHigh->setChecked (true);
    else if ((ba.contains ("[52;2v")))
        radioFBFastBlank->setChecked (true);
    else if ((ba.contains ("[52;3v")))
        radioFBInvFastBlank->setChecked (true);

    /*--------------------------------
     * Blink
     *------------------------------*/
    if ((ba.contains ("[32;0v")))
        checkBlink->setChecked (false);
    else if ((ba.contains ("[32;1v")))
        checkBlink->setChecked (true);

    indx = ba.indexOf ("[33;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinBlink1->setValue (val);
    
    indx = ba.indexOf ("[34;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinBlink2->setValue (val);
    
    /*--------------------------------
     * Comm
     *------------------------------*/
    indx = ba.indexOf ("[40;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinCom5->setValue (val);

    if ((ba.contains ("[41;0v")))
        radioFlowNone->setChecked (true);
    else if ((ba.contains ("[41;1v")))
        radioFlowXONOFF->setChecked (true);
    else if ((ba.contains ("[41;2v")))
        radioFlowHardware->setChecked (true);

    indx = ba.indexOf ("[42;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    if ((val & 0x40))
	checkEchoOn->setChecked (true);

    switch (val & 0x30) {
	case 0:    radio1Stop->setChecked (true);
		   break;
	case 0x10: radio1_5Stop->setChecked (true);
		   break;
	case 0x20:
	case 0x30: radio2Stop->setChecked (true);
    }

    if ((val & 0x08))
	radio7Bit->setChecked (true);
    else
	radio8Bit->setChecked (true);

    switch (val & 0x07) {
	case 0: radioParEven->setChecked (true);
		break;
	case 1: radioParOdd->setChecked (true);
		break;
	case 2: radioParSpace->setChecked (true);
		break;
	case 3: radioParMark->setChecked (true);
		break;
	case 4: radioParNone->setChecked (true);
		break;
    }

    if ((ba.contains ("[44;0v")))
        radioSPImode0->setChecked (true);
    else if ((ba.contains ("[44;1v")))
        radioSPImode1->setChecked (true);
    else if ((ba.contains ("[44;2v")))
        radioSPImode2->setChecked (true);
    else if ((ba.contains ("[44;3v")))
        radioSPImode3->setChecked (true);
    else if ((ba.contains ("[44;4v")))
        radioSPIhardware->setChecked (true);
	
    if ((ba.contains ("[56;0v")))
        radioSP0None->setChecked (true);
    else if ((ba.contains ("[56;1v")))
        radioSP0Present->setChecked (true);

    if ((ba.contains ("[57;0v")))
        radioSP1None->setChecked (true);
    else if ((ba.contains ("[57;1v")))
        radioSP1Present->setChecked (true);

    if ((ba.contains ("[58;0v")))
        radioSP2None->setChecked (true);
    else if ((ba.contains ("[58;1v")))
        radioSP2Present->setChecked (true);
    else if ((ba.contains ("[58;2v")))
        radioSP2XE->setChecked (true);

    if ((ba.contains ("[59;0v")))
        radioSP3None->setChecked (true);
    else if ((ba.contains ("[59;1v")))
        radioSP3Present->setChecked (true);

    indx = ba.indexOf ("[60;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinCom1->setValue (val);
    
    indx = ba.indexOf ("[61;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinCom2->setValue (val);
    
    indx = ba.indexOf ("[62;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinCom3->setValue (val);
    
    indx = ba.indexOf ("[63;");
    sscanf (ba.data() + indx + 5, "%dv", &val);
    spinCom4->setValue (val);
    
    modified = false;
    buttonDownload->setEnabled (true);
}

/*------------------------------------------------------------------------------------------------
 * on_buttonSaveFile_clicked --
 *----------------------------------------------------------------------------------------------*/
void ConfigDialog::on_buttonSaveFile_clicked ()
{
    loadBA ();
    QString fileName = QFileDialog::getSaveFileName (this, tr ("Save File"), curFile,
	    tr("config files (*.b4c)"));

    if (!fileName.endsWith (".b4c"))
	fileName.append (".b4c");

    QFile file(fileName);
    if (!file.open(QFile::ReadWrite | QFile::Text)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot read file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }

    file.write (ba);
    file.close();

    modified = false;
    buttonDownload->setEnabled (false);
    //qDebug () << "[ba]" << ba;
}

/*------------------------------------------------------------------------------------------------
 * loadBA -- same order as windows concriptor for easy testing
 *----------------------------------------------------------------------------------------------*/
void ConfigDialog::loadBA ()
{
int val;
char str[32];

    if ((checkBOB4only->checkState ()))
	ba.append ("[9;1v");
    else
	ba.append ("[9;0v");
	
    if ((checkString->checkState ()))
	ba.append ("[10;1v");
    else
	ba.append ("[10;0v");
	
    if ((radioCRLF0->isChecked ()))
	ba.append ("[11;0v");
    else if ((radioCRLF1->isChecked ()))
	ba.append ("[11;1v");
    else if ((radioCRLF2->isChecked ()))
	ba.append ("[11;2v");
    else
	ba.append ("[11;3v");
	
    if ((radioNTSC->isChecked ()))
	ba.append ("[16;0v");
    else
	ba.append ("[16;1v");
	

    if ((radioProgressive->isChecked()))
	ba.append ("[17;0v");
    else
	ba.append ("[17;1v");
	
    if ((radioLocal->isChecked ()))
	ba.append ("[18;1v");
    else if ((radioExternal->isChecked ()))
	ba.append ("[18;2v");
    else
	ba.append ("[18;3v");


    if ((checkOverlay->checkState ()))
	ba.append ("[28;1v");
    else
	ba.append ("[28;0v");

    if ((checkVideoSrc->checkState ()))
	ba.append ("[19;1v");
    else
	ba.append ("[19;0v");

    if ((checkHighPixRate->checkState ()))
	ba.append ("[20;1v");
    else
	ba.append ("[20;0v");

    if ((checkRestrictOverscan->checkState ()))
	ba.append ("[21;0v");
    else
	ba.append ("[21;1v");
	
    if ((radio320x240->isChecked ()))
	ba.append ("[22;0v");
    else if ((radio384x288->isChecked ()))
	ba.append ("[22;1v");
    else if ((radio480x240->isChecked ()))
	ba.append ("[22;2v");
    else
	ba.append ("[22;3v");

    if ((radioDefault->isChecked ()))
	ba.append ("[23;0v");
    else if ((radio5MHz->isChecked ()))
	ba.append ("[23;1v");
    else if ((radio5MHz375->isChecked ()))
	ba.append ("[23;2v");
    else if ((radio5MHz769->isChecked ()))
	ba.append ("[23;3v");
    else if ((radio6MHz25->isChecked ()))
	ba.append ("[23;4v");
    else if ((radio6MHz818->isChecked ()))
	ba.append ("[23;5v");
    else if ((radio7MHz5->isChecked ()))
	ba.append ("[23;6v");
    else if ((radio8MHz33->isChecked ()))
	ba.append ("[23;7v");
    else
	ba.append ("[23;8v");

    if ((radioNoReversal->isChecked ()))
	ba.append ("[35;0v");
    else if ((radioReverseHorz->isChecked ()))
	ba.append ("[35;1v");
    else if ((radioReverseVert->isChecked ()))
	ba.append ("[35;2v");
    else
	ba.append ("[35;3v");

    val = spinDispZ1->value();
    sprintf (str, "\x1b" "[24;%s%dv", val < 0 ? "<" : "", abs (val));
    ba.append (str);

    val = spinDispZ2->value();
    sprintf (str, "\x1b" "[25;%dv", val);
    ba.append (str);

    val = spinDispZ3->value();
    sprintf (str, "\x1b" "[26;%s%dv", val < 0 ? "<" : "", abs (val));
    ba.append (str);

    val = spinDispZ4->value();
    sprintf (str, "\x1b" "[27;%dv", val);
    ba.append (str);

    val = spinDispZ5->value();
    sprintf (str, "\x1b" "[36;%dv", val);
    ba.append (str);

    val = spinDisp1->value();
    sprintf (str, "\x1b" "[29;%dv", val);
    ba.append (str);

    val = spinDisp2->value();
    sprintf (str, "\x1b" "[30;%dv", val * 1000);
    ba.append (str);

    val = spinDisp3->value();
    sprintf (str, "\x1b" "[31;%dv", val * 1000);
    ba.append (str);

    if ((checkBlink->checkState ()))
	ba.append ("[32;1v");
    else
	ba.append ("[32;0v");
	
    val = spinBlink1->value();
    sprintf (str, "\x1b" "[33;%dv", val);
    ba.append (str);

    val = spinBlink2->value();
    sprintf (str, "\x1b" "[34;%dv", val);
    ba.append (str);

    if ((radioL0Low->isChecked ()))
	ba.append ("[48;0v");
    else if ((radioL0High->isChecked ()))
	ba.append ("[48;1v");
    else if ((radioL0CompSync->isChecked ()))
	ba.append ("[48;2v");
    else
	ba.append ("[48;3v");

    if ((radioL1Low->isChecked ()))
	ba.append ("[49;0v");
    else if ((radioL1High->isChecked ()))
	ba.append ("[49;1v");
    else if ((radioL1HorzSync->isChecked ()))
	ba.append ("[49;2v");
    else
	ba.append ("[49;3v");

    if ((radioL2Low->isChecked ()))
	ba.append ("[50;0v");
    else if ((radioL2High->isChecked ()))
	ba.append ("[50;1v");
    else if ((radioL2HorzSync->isChecked ()))
	ba.append ("[50;2v");
    else
	ba.append ("[50;3v");

    if ((radioL3Low->isChecked ()))
	ba.append ("[51;0v");
    else if ((radioL3High->isChecked ()))
	ba.append ("[51;1v");
    else if ((radioL3CompSync->isChecked ()))
	ba.append ("[51;2v");
    else
	ba.append ("[51;3v");

    if ((radioFBLow->isChecked ()))
	ba.append ("[52;0v");
    else if ((radioFBHigh->isChecked ()))
	ba.append ("[52;1v");
    else if ((radioFBFastBlank->isChecked ()))
	ba.append ("[52;2v");
    else
	ba.append ("[52;3v");

    val = spinCom5->value();
    sprintf (str, "[40;%dv", val);
    ba.append (str);

    if ((radioFlowNone->isChecked ()))
	ba.append ("[41;0v");
    else if ((radioFlowXONOFF->isChecked ()))
	ba.append ("[41;1v");
    else
	ba.append ("[41;2v");

    val = 0;
    if (checkEchoOn->isChecked ())
	val |= 0x40;

    if (radio1_5Stop->isChecked ())
	val |= 0x10;
    else if (radio2Stop->isChecked ())
	val |= 0x20;

    if (radio7Bit->isChecked ())
	val |= 0x08;

    if (radioParOdd->isChecked ())
	val |= 0x01;
    else if (radioParSpace->isChecked ())
	val |= 0x02;
    else if (radioParMark->isChecked ())
	val |= 0x03;
    else if (radioParNone->isChecked ())
	val |= 0x04;

    sprintf (str, "\x1b" "[42;%dv", val);
    ba.append (str);

    if ((radioSPImode0->isChecked ()))
	ba.append ("[44;0v");
    else if ((radioSPImode1->isChecked ()))
	ba.append ("[44;1v");
    else if ((radioSPImode2->isChecked ()))
	ba.append ("[44;2v");
    else if ((radioSPImode3->isChecked ()))
	ba.append ("[44;3v");
    else
	ba.append ("[44;4v");

    if ((radioSP0None->isChecked ()))
	ba.append ("[56;0v");
    else
	ba.append ("[56;1v");

    if ((radioSP1None->isChecked ()))
	ba.append ("[57;0v");
    else
	ba.append ("[57;1v");

    if ((radioSP2None->isChecked ()))
	ba.append ("[58;0v");
    else if ((radioSP2Present->isChecked ()))
	ba.append ("[58;1v");
    else
	ba.append ("[58;2v");

    if ((radioSP3None->isChecked ()))
	ba.append ("[59;0v");
    else
	ba.append ("[59;1v");

    val = spinCom1->value();
    sprintf (str, "\x1b" "[60;%dv", val);
    ba.append (str);

    val = spinCom2->value();
    sprintf (str, "\x1b" "[61;%dv", val);
    ba.append (str);

    val = spinCom3->value();
    sprintf (str, "\x1b" "[62;%dv", val);
    ba.append (str);

    val = spinCom4->value();
    sprintf (str, "\x1b" "[63;%dv", val);
    ba.append (str);
}

/*------------------------------------------------------------------------------------------------
 * on_buttonDownload_clicked --
 *----------------------------------------------------------------------------------------------*/
void ConfigDialog::on_buttonDownload_clicked ()
{
    conscriptor->statusBar()->showMessage(tr("config file download"), 6000);
    loadBA ();
    if ((conscriptor->configDownload (ba)))
	buttonDownload->setEnabled (false);
    conscriptor->statusBar()->showMessage(tr("Download complete"), 6000);
}

/*-------------------------------------------------------------------------------------------------
 * setModified --
 *-----------------------------------------------------------------------------------------------*/
void ConfigDialog::setModified ()
{
    modified = true;
    if (conscriptor->commPort != Conscriptor::NONE)
	buttonDownload->setEnabled (true);
}

/*-------------------------------------------------------------------------------------------------
 * exitCE --
 *-----------------------------------------------------------------------------------------------*/
void ConfigDialog::exitCE ()
{
    if ((modified)) {
        int r = QMessageBox::warning(this,
		    tr("Bootscript Editor"),
		    tr("Bootscript has been modified.\n"
		       "Do you want to save it?"),
		    QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes)
	    on_buttonSaveFile_clicked ();
    }
    accept ();
}
