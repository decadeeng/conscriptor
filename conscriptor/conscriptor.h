#ifndef CONSCRIPTOR_H
#define CONSCRIPTOR_H

#include <QMainWindow>
#include <QColor>
#include <QScrollArea>
#include <QComboBox>
#include "conf.h"
#include "qextserialport.h"

class QPixmap;
class QAction;
class QMenu;
class QFile;
class ImportDialog;
class TerminalDialog;
class BaudSelectDialog;
class PortSelectDialog;

class B4Font;

class Conscriptor : public QMainWindow
{
    Q_OBJECT

//-----------------------------------------------------------------------------------------------
public:
    enum comtag {NONE, XBOBP, DEBUGP};
    comtag       commPort;
    QString	 portName;
    BaudRateType baudRate;

    QScrollArea *scrollArea;

    Conscriptor (QWidget *parent = 0);
    B4Font *b4font;
    QextSerialPort *ttyPort;
    ImportDialog *bdfDialog;
    TerminalDialog *terminalDialog;
    BaudSelectDialog *baudselectDialog;
    PortSelectDialog *portselectDialog;

    QString curFile;
    QColor clearColor;
    QColor opaqueColor;
    QColor cthruColor;

    QByteArray ba;

    bool bootscriptDialogShowing;
    bool configDialogShowing;
    bool setupDialogShowing;
    bool font_main_port (QString fileName);
    bool font_debug_port (QString fileName);
    bool configDownload (QByteArray &ba);
    bool bootscriptDownload (QByteArray &ba);
    bool isBOB ();

    void createToolbars ();
    void setTtyPort ();
    void setTtyDebug ();
    void setScrollSize ();

    struct fonttag {
	unsigned char  name[12];        // name unused bytes to null
	int skip;                       // offset to next font struct (mod 32)
	int pixWidth;                   // # bytes to make 1 line of the font pixmap (mod 4)
	unsigned short chCount;         // Number of chars in this font
	unsigned short chFirst;         // first char eg 0x20 <ascii space>
	unsigned char  chHeight;        // number of scan lines in pixmap
	unsigned char  chAscent;        // Top to baseline count
	unsigned char  chDepth;         // ln(BPP) ie 0=1BPP, 1=2BPP etc
	unsigned char  fontVersion;     // currently 1
	unsigned char  reserved[4];     // Header to size mod 32; set bytes to 0
    } header;

    struct metrixtag {
	unsigned short x;               // dist LHS pixmap to LHS image (mod 8)
	char           left;            // cursor to LHS image; fixed width = 0
	unsigned char  width;           // char width (fixed for FIXED fonts) (mod 8)
	unsigned char  top;             // first non zero line
	unsigned char  bottom;          // last non zero line +1 eg 8x13 top = 0 bottom = 13
    } *metrixBase;

    bool modified;			// If anything has changed
    char *pixmap;
    int metricCount;			// Bytes in metric array 32 byte aligned
    int pixmapCount;			// Bytes in pixmap array 32 byte aligned
    int flashDevice;			// BOB4 Flash Device
    int indent;				// debug indent

    bool save();

    void setCurrentFile(const QString &fileName);
    QAction *actionInsertOutline;
    QAction *actionOutline;
    QAction *action1to2BPP;
    QAction *action2to1BPP;
    QAction *actionCthruColor;

public slots:
    void setModified ();
    void on_actionTtyQuery ();

//-----------------------------------------------------------------------------------------------
protected:

//-----------------------------------------------------------------------------------------------
private slots:
    void openRecentFile();
    bool okToContinue();

    void on_actionNew ();
    void on_actionOpen ();
    void on_actionImport_BDF_Font ();
    void on_actionConcatenateFiles ();
    void on_actionSave ();
    void on_actionSaveAs ();
    void on_actionPrint ();
    void on_actionExit ();

    void on_actionFontName ();

    void on_actionToolbar ();
    void on_actionStatusbar ();
    void on_actionZoomIn ();
    void on_actionZoomOut ();
    void on_actionFontParameters ();

    void on_actionCthruColor ();
    void on_actionClearColor ();
    void on_actionOpaqueColor ();
    void on_actionSetClearColor ();
    void on_actionSetOpaqueColor ();
    void on_actionSetCthruColor ();

    void on_actionFont ();
    void on_actionFirmware ();
    void on_actionFirmware_repair ();

    void on_actionBaudRate ();
    void on_actionPort ();
    void on_actionTerminal ();

    void on_action1to2BPP ();
    void on_action2to1BPP ();

    void On_actionBSE ();
    void On_actionCE ();
    void On_actionSE ();
    void On_actionHelpAbout ();
    void On_actionHelpHelp ();

    void onBootscript_finished ();
    void onConfig_finished ();
    void onSetup_finished ();
    void on_actionDefaultColor ();

//-----------------------------------------------------------------------------------------------
private:
    DataBitsType dataBits;
    ParityType   parity;
    StopBitsType stopBits;
    FlowType     flowctrl;

    QStringList recentFiles;
    enum { MaxRecentFiles = 5 };

    QComboBox *comboBaudRate;
    QComboBox *comboPort;
//    QAction *separatorAction;
    QAction *recentFileActions[MaxRecentFiles];
    void updateRecentFileActions();
    QString strippedName(const QString &fullFileName);
    QString baseName(const QString &fullFileName);

    bool loadFile(const QString &fileName);
    void openFile (const QString &fileName);

    void readSettings();
    void writeSettings();

    void createActions();
    void createMenus();

    void setClearIcon ();
    void setOpaqueIcon ();
    void setCthruIcon ();
    bool maybeSave();
    bool saveFile(const QByteArray &fileFormat);
    bool bob4Send85 (int flashDevice, quint32 loc, quint8 *data, int len, int debugPort);
    bool writeXModem (int sector, char *data);
    void firmware_main_port ();
    void firmware_debug_port ();
    void bootloaderLoad (QString fileName, bool flag);

    // File
    QAction *actionNew;
    QAction *actionOpen;
    QAction *actionImport_BDF_Font;
    QAction *actionImportImage;
    QAction *actionConcatenateFiles;
    QAction *actionSave;
    QAction *actionSaveAs;
    QAction *actionInfo;
    QAction *actionPrint;
    QAction *actionExit;

    // Edit
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
    QAction *actionClear;

    QAction *actionInsertRow;
    QAction *actionInsertColumn;
    QAction *actionInsertChar;

    QAction *actionDeleteRow;
    QAction *actionDeleteColumn;
    QAction *actionDeleteChar;

    QAction *actionMoveDown;
    QAction *actionMoveUp;
    QAction *actionMoveRight;
    QAction *actionMoveLeft;



    QAction *actionFontName;

    QAction *actionSeparator;

    // View
    QAction *actionToolbar;
    QAction *actionStatusbar;
    QAction *actionZoomIn;
    QAction *actionZoomOut;
    QAction *actionFontParameters;

    // Colour
    QAction *actionClearColor;
    QAction *actionOpaqueColor;
    QAction *actionSetClearColor;
    QAction *actionSetOpaqueColor;
    QAction *actionSetCthruColor;
    QAction *actionDefaultColor;

    // Serial
    QAction *actionBaudRate;
    QAction *actionPort;

    QAction *actionFont;
    QAction *actionFirmware;
    QAction *actionFirmware_repair;

    QAction *actionTerminal;

    // Bootscript Editor
    QAction *actionBSE;

    // Config Editor
    QAction *actionCE;

    // Setup Editor
    QAction *actionSE;

    // Help
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionAboutQt;
    QAction *actionHelpAbout;
    QAction *actionHelpHelp;

    // Toolbar Only
    QAction *actionRollUp;
    QAction *actionRollDown;
    QAction *actionCodePoint;
    QAction *actionTtyQuery;

    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuInsert;
    QMenu *menuDelete;
    QMenu *menuMove;
    QMenu *menuPixelDepth;
    QMenu *menuView;
    QMenu *menuColour;
    QMenu *menuSerial;
    QMenu *menuPort;
    QMenu *menuBaudRate;
    QMenu *menuDownload;
    QMenu *menuBSE;
    QMenu *menuCE;
    QMenu *menuSE;
    QMenu *menuHelp;
    QMenu *menuDebug;

    QImage *clearImage;
    QImage *opaqueImage;
    QImage *cthruImage;

    QToolBar *toolbarMyApp;

    QFile *myFile;
};

#endif
