#include <QtGui>
#include <QDebug>
#include <QAction>
#include <QFile>

#include "conscriptor.h"
#include "font.h"
#include "bootscript.h"
#include "config.h"
#include "setup.h"
#include "setmem.h"
#include "help.h"
#include "newfile.h"
#include "importbdf.h"
#include "fontname.h"
#include "baudselect.h"
#include "portselect.h"
#include "glyph.h"
#include "print.h"
#include "terminal.h"
#include "version.h"

#ifndef _WIN32
#  define DBGFIX			// Baud rate problems debug port and bootloader ??
					// in any event THIS does work, else does not
#endif
extern int debug;

/*-------------------------------------------------------------------------------------------------
 * constructer --
 *-----------------------------------------------------------------------------------------------*/
Conscriptor::Conscriptor (QWidget *parent)
        : QMainWindow(parent)
{
    if ((debug))
	qDebug ("[conscriptor] constructor");
    b4font = new B4Font (this);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(b4font);

    setCentralWidget(scrollArea);
    Conscriptor::resize(600, 800);		// trying this
  
    bootscriptDialogShowing = false;
    configDialogShowing = false;
    setupDialogShowing = false;

    metricCount = 0;
    pixmapCount = 0;
    metrixBase = 0;
    pixmap = 0;
    modified = false;

    createActions();
    createMenus();
    createToolbars();

    setCurrentFile("");
    readSettings ();

    //qDebug () << "scrollArea size" << scrollArea->size();
    //qDebug () << "b4font size" << b4font->size();
    //qDebug () << "conscriptor size" << this->size();

    b4font->setMinimumSize(this->size().width() * 0.95, this->size().height() * 0.87);

    ttyPort = 0;
    on_actionTtyQuery ();
    //commPort = NONE;

    clearImage  = new QImage (32, 32,QImage::Format_RGB32);
    opaqueImage = new QImage (32, 32,QImage::Format_RGB32);
    cthruImage  = new QImage (32, 32,QImage::Format_RGB32);

    /*---------------------------------------------------------------------------------------------
     * Image must exist, it gets filled with the relevant colour
     *-------------------------------------------------------------------------------------------*/
    setClearIcon  ();
    setOpaqueIcon ();
    setCthruIcon  ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * isBOB --
 *	checks if bob is where commPort says it is
 *	if not tries to open ttyMain then ttyDEBUG
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::isBOB ()
{
    char cmd[8];

    switch (commPort) {
	case XBOBP:
	    sprintf (cmd, "\x1b" "[1}");
	    ttyPort->write (cmd, 4);
#	  ifdef _WIN32
	    Sleep (100);
#	  endif
	    ba = ttyPort->readAll();
	    if ((debug & TRACE5))
		qDebug () << "[read] port" << ba;
	    if ((ba.contains ("v4"))) {
		commPort = XBOBP;
		actionTtyQuery->setIcon(QIcon(":icons/on.png"));
		return true;
	    }
	    else {
		delete ttyPort;
		ttyPort = 0;
		commPort = NONE;
		actionTtyQuery->setIcon(QIcon(":icons/off.png"));
		return false;
	    }
	case DEBUGP:
	    ttyPort->putChar ('\n');
#	  ifdef _WIN32
	    Sleep (100);
#	  endif
	    ba = ttyPort->readAll();
	    if ((debug & TRACE5))
		qDebug () << "[read] debug" << ba;
	    if ((ba.contains ('>'))) {
		commPort = DEBUGP;
		actionTtyQuery->setIcon(QIcon(":icons/bug.png"));
		return true;
	    }
	    else {
		delete ttyPort;
		ttyPort = 0;
		commPort = NONE;
		actionTtyQuery->setIcon(QIcon(":icons/off.png"));
		return false;
	    }
	case NONE:
	    setTtyPort ();
	    if (commPort == Conscriptor::NONE)
		setTtyDebug ();
	    if (commPort == Conscriptor::NONE)
		return false;
	    else
		return true;
    }
}

/*-------------------------------------------------------------------------------------------------
 * setClearIcon --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setClearIcon ()
{
    if ((debug))
	qDebug () << "[conscriptor] setClearIcon  color" << clearColor;
    clearImage->fill (clearColor.rgb());

    actionClearColor->setIcon(QIcon(QPixmap::fromImage((*clearImage))));
    actionSetClearColor->setIcon(QIcon(QPixmap::fromImage((*clearImage))));
}

/*-------------------------------------------------------------------------------------------------
 * setOpaqueIcon --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setOpaqueIcon ()
{
    if ((debug))
	qDebug () << "[conscriptor] setOpaqueIcon color" << opaqueColor;
    opaqueImage->fill (opaqueColor.rgb());

    actionOpaqueColor->setIcon(QIcon(QPixmap::fromImage((*opaqueImage))));
    actionSetOpaqueColor->setIcon(QIcon(QPixmap::fromImage((*opaqueImage))));
}

/*-------------------------------------------------------------------------------------------------
 * setCthruIcon --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setCthruIcon ()
{
    if ((debug))
	qDebug () << "[conscriptor] setCthruIcon  color" << cthruColor;
    cthruImage->fill (cthruColor.rgb());

    actionCthruColor->setIcon(QIcon(QPixmap::fromImage((*cthruImage))));
    actionSetCthruColor->setIcon(QIcon(QPixmap::fromImage((*cthruImage))));
}

/*-------------------------------------------------------------------------------------------------
 * createActions --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::createActions ()
{
    if ((debug))
	qDebug ("[conscriptor] createActions");
    //------ FILE -----------------------------------------------------------------
    actionNew = new QAction (tr("&New"), this);
    actionNew->setIcon(QIcon(":icons/new.png"));
    actionNew->setShortcut(QKeySequence::New);
    connect(actionNew, SIGNAL (triggered()), this, SLOT(on_actionNew()));

    actionOpen = new QAction (tr("&Open..."), this);
    actionOpen->setIcon(QIcon(":icons/open.png"));
    actionOpen->setShortcut(QKeySequence::Open);
    connect(actionOpen, SIGNAL (triggered()), this, SLOT(on_actionOpen()));

    actionImport_BDF_Font = new QAction (tr("&Import BDF font"), this);
    connect(actionImport_BDF_Font, SIGNAL (triggered()), this, SLOT(on_actionImport_BDF_Font()));

    actionImportImage = new QAction (tr("I&mport Image"), this);
    connect(actionImportImage, SIGNAL (triggered()), b4font, SLOT(on_actionImportImage()));

    actionConcatenateFiles = new QAction (tr("&Concatenate Files"), this);
    actionConcatenateFiles->setStatusTip (tr("Concatenate b4F files"));
    connect(actionConcatenateFiles, SIGNAL (triggered()), this, SLOT(on_actionConcatenateFiles()));

    actionSave = new QAction (tr("&Save"), this);
    actionSave->setIcon(QIcon(":icons/save.png"));
    actionSave->setShortcut(QKeySequence::Save);
    connect(actionSave, SIGNAL (triggered()), this, SLOT(on_actionSave()));

    actionSaveAs = new QAction (tr("Save &As..."), this);
    connect(actionSaveAs, SIGNAL (triggered()), this, SLOT(on_actionSaveAs()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActions[i] = new QAction(this);
        recentFileActions[i]->setVisible(false);
        connect(recentFileActions[i], SIGNAL (triggered()), this, SLOT(openRecentFile()));
    }

    actionPrint = new QAction (tr("&Print"), this);
    actionPrint->setIcon(QIcon(":icons/print.png"));
    actionPrint->setShortcut(QKeySequence::Print);
    connect (actionPrint, SIGNAL (triggered()), this, SLOT(on_actionPrint()));

    actionExit = new QAction (tr("E&xit"), this);
    actionExit->setShortcut (tr("Ctrl+Q"));
    connect(actionExit, SIGNAL (triggered()), this, SLOT(on_actionExit()));

    //------ EDIT -----------------------------------------------------------------
    actionCut = new QAction (tr("Cu&t"), this);
    actionCut->setIcon(QIcon(":icons/cut.png"));
    actionCut->setShortcut(QKeySequence::Cut);
    actionCut->setStatusTip (tr("Cut the current glyph's contents to paste buffer"));
    connect(actionCut, SIGNAL (triggered()), b4font, SLOT(on_actionCut()));

    actionCopy = new QAction (tr("&Copy"), this);
    actionCopy->setIcon(QIcon(":icons/copy.png"));
    actionCopy->setShortcut(QKeySequence::Copy);
    actionCopy->setStatusTip (tr("Copy the current glyph's contents to paste buffer"));
    connect(actionCopy, SIGNAL (triggered()), b4font, SLOT(on_actionCopy()));

    actionPaste = new QAction (tr("&Paste"), this);
    actionPaste->setIcon(QIcon(":icons/paste.png"));
    actionPaste->setShortcut(QKeySequence::Paste);
    actionPaste->setStatusTip (tr("Copy the paste buffer to the current glyph"));
    connect(actionPaste, SIGNAL (triggered()), b4font, SLOT(on_actionPaste()));

    actionClear = new QAction (tr("C&lear"), this);
    actionClear->setShortcut(QKeySequence::Delete);
    actionClear->setStatusTip (tr("Clear the current glyph"));
    connect(actionClear, SIGNAL (triggered()), b4font, SLOT(on_actionClear()));

    actionInsertRow = new QAction (tr("&Row"), this);
    //actionInsertRow->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionInsertRow, SIGNAL (triggered()), b4font, SLOT(on_actionInsertRow()));

    actionInsertColumn = new QAction (tr("&Column"), this);
    //actionInsertColumn->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionInsertColumn, SIGNAL (triggered()), b4font, SLOT(on_actionInsertColumn()));

# ifdef Q_WS_MAC
    actionInsertChar = new QAction (tr("c&Haracter"), this);
# else
    actionInsertChar = new QAction (tr("C&haracter"), this);
# endif
    //actionInsertChar->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionInsertChar, SIGNAL (triggered()), b4font, SLOT(on_actionInsertChar()));

    actionInsertOutline = new QAction (tr("Insert &Outline"), this);
    //actionInsertOutline->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionInsertOutline, SIGNAL (triggered()), b4font, SLOT(on_actionInsertOutline()));

    actionDeleteRow = new QAction (tr("&Row"), this);
    //actionDeleteRow->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionDeleteRow, SIGNAL (triggered()), b4font, SLOT(on_actionDeleteRow()));

    actionDeleteColumn = new QAction (tr("&Column"), this);
    //actionDeleteColumn->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionDeleteColumn, SIGNAL (triggered()), b4font, SLOT(on_actionDeleteColumn()));

# ifdef Q_WS_MAC
    actionDeleteChar = new QAction (tr("c&Haracter"), this);
# else
    actionDeleteChar = new QAction (tr("C&haracter"), this);
# endif
    //actionDeleteChar->setStatusTip (tr("Select all the cells in the current row"));
    connect(actionDeleteChar, SIGNAL (triggered()), b4font, SLOT(on_actionDeleteChar()));

    actionMoveUp = new QAction (tr("&Up"), this);
    actionMoveUp->setIcon(QIcon(":icons/up.png"));
    actionMoveUp->setStatusTip (tr("Move the character up in it's box"));
    connect(actionMoveUp, SIGNAL (triggered()), b4font, SLOT(on_actionMoveUp()));

    actionMoveDown = new QAction (tr("&Down"), this);
    actionMoveDown->setIcon(QIcon(":icons/down.png"));
    actionMoveDown->setStatusTip (tr("Move the character down in it's box"));
    connect(actionMoveDown, SIGNAL (triggered()), b4font, SLOT(on_actionMoveDown()));

    actionMoveLeft = new QAction (tr("&Left"), this);
    actionMoveLeft->setIcon(QIcon(":icons/left.png"));
    actionMoveLeft->setStatusTip (tr("Move the character left in it's box"));
    connect(actionMoveLeft, SIGNAL (triggered()), b4font, SLOT(on_actionMoveLeft()));

    actionMoveRight = new QAction (tr("&Right"), this);
    actionMoveRight->setIcon(QIcon(":icons/right.png"));
    actionMoveRight->setStatusTip (tr("Move the character right in it's box"));
    connect(actionMoveRight, SIGNAL (triggered()), b4font, SLOT(on_actionMoveRight()));

    action1to2BPP = new QAction (tr("&1BPP to 2BPP"), this);
    connect(action1to2BPP, SIGNAL (triggered()), this, SLOT(on_action1to2BPP()));

    action2to1BPP = new QAction (tr("&2BPP to 1BPP"), this);
    connect(action2to1BPP, SIGNAL (triggered()), this, SLOT(on_action2to1BPP()));

    actionFontName = new QAction (tr("&Font Name"), this);
    connect(actionFontName, SIGNAL (triggered()), this, SLOT(on_actionFontName()));

    //------ VIEW -----------------------------------------------------------------
    actionToolbar = new QAction (tr("&Toolbar"), this);
    actionToolbar->setCheckable (true);
    actionToolbar->setChecked (true);
    connect(actionToolbar, SIGNAL (triggered()), this, SLOT(on_actionToolbar()));

    actionStatusbar = new QAction (tr("&Statusbar"), this);
    actionStatusbar->setCheckable (true);
    actionStatusbar->setChecked (true);
    connect(actionStatusbar, SIGNAL (triggered()), this, SLOT(on_actionStatusbar()));
    statusBar()->setVisible (true);

    actionZoomIn = new QAction (tr("Zoom &In"), this);
    actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    connect(actionZoomIn, SIGNAL (triggered()), this, SLOT(on_actionZoomIn()));

    actionZoomOut = new QAction (tr("Zoom &Out"), this);
    actionZoomOut->setShortcut(QKeySequence::ZoomOut);
    connect(actionZoomOut, SIGNAL (triggered()), this, SLOT(on_actionZoomOut()));

    actionFontParameters = new QAction (tr("&Font Parameters"), this);
    connect(actionFontParameters, SIGNAL (triggered()), this, SLOT(on_actionFontParameters()));

    //------ COLOR ----------------------------------------------------------------
    actionClearColor = new QAction (tr("&Clear Pixel"), this);
    //actionClearColor->setStatusTip (tr("Cut the current selection's contents to the clipboard"));
    connect(actionClearColor, SIGNAL (triggered()), this, SLOT(on_actionClearColor()));

    actionOpaqueColor = new QAction(tr("&Opaque Pixel"), this);
    //actionOpaqueColor->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(actionOpaqueColor, SIGNAL(triggered()), this, SLOT(on_actionOpaqueColor()));

    actionCthruColor = new QAction(tr("&Seethru Pixel"), this);
    //actionCthruColor->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(actionCthruColor, SIGNAL(triggered()), this, SLOT(on_actionCthruColor()));

    actionOutline = new QAction (tr("Outline"), this);
    actionOutline->setStatusTip (tr("Set character outline"));
    actionOutline->setIcon(QIcon(":icons/fonts.png"));
    connect(actionOutline, SIGNAL(triggered()), b4font, SLOT(on_actionOutline()));

    actionSetClearColor = new QAction (tr("Set C&lear Color"), this);
    //actionClearColor->setStatusTip (tr("Cut the current selection's contents to the clipboard"));
    connect(actionSetClearColor, SIGNAL (triggered()), this, SLOT(on_actionSetClearColor()));

    actionSetOpaqueColor = new QAction(tr("Set O&paque Color"), this);
    //actionOpaqueColor->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(actionSetOpaqueColor, SIGNAL(triggered()), this, SLOT(on_actionSetOpaqueColor()));

    actionSetCthruColor = new QAction(tr("Set S&eeThru Color"), this);
    //actionCthruColor->setStatusTip(tr("Cut the current selection's contents to the clipboard"));
    connect(actionSetCthruColor, SIGNAL(triggered()), this, SLOT(on_actionSetCthruColor()));

    actionDefaultColor = new QAction (tr("&Reset Default Colors"), this);
    connect(actionDefaultColor, SIGNAL (triggered()), this, SLOT(on_actionDefaultColor()));

    // SERIAL ------------------------------------------------------------------------------
    actionTerminal = new QAction (tr("Terminal"), this);
    connect(actionTerminal, SIGNAL (triggered()), this, SLOT(on_actionTerminal()));
    actionBaudRate = new QAction (tr("Baud Rate"), this);
    connect(actionBaudRate, SIGNAL (triggered()), this, SLOT(on_actionBaudRate()));
    actionPort = new QAction (tr("Port"), this);
    connect(actionPort, SIGNAL (triggered()), this, SLOT(on_actionPort()));

    actionFont            = new QAction (tr("Font download"), this);
    connect(actionFont, SIGNAL (triggered()), this, SLOT(on_actionFont()));
    actionFirmware        = new QAction (tr("Firmware download"), this);
    connect(actionFirmware, SIGNAL (triggered()), this, SLOT(on_actionFirmware()));
    actionFirmware_repair = new QAction (tr("Firmware repair"), this);
    connect(actionFirmware_repair, SIGNAL (triggered()), this, SLOT(on_actionFirmware_repair()));

// Toolbar
    actionRollUp = new QAction(tr("Roll Up"), this);
    actionRollUp->setIcon(QIcon(":icons/rollup.png"));
    actionRollUp->setStatusTip (tr("Scroll the glyphs up"));
    connect(actionRollUp, SIGNAL(triggered()), b4font, SLOT(on_actionRollUp()));

    actionRollDown = new QAction(tr("Roll Down"), this);
    actionRollDown->setIcon(QIcon(":icons/rolldown.png"));
    actionRollDown->setStatusTip (tr("Scroll the glyphs down"));
    connect(actionRollDown, SIGNAL(triggered()), b4font, SLOT(on_actionRollDown()));

    actionCodePoint = new QAction(tr("Set Code Point"), this);
    actionCodePoint->setIcon(QIcon(":icons/codeval.png"));
    actionCodePoint->setStatusTip (tr("Change Character Code Point"));
    connect(actionCodePoint, SIGNAL(triggered()), b4font, SLOT(on_actionCodePoint()));

    actionTtyQuery = new QAction(tr("Find BOB"), this);
    actionTtyQuery->setIcon(QIcon(":icons/off.png"));
    actionTtyQuery->setStatusTip (tr("Find an XBOB"));
    connect(actionTtyQuery, SIGNAL(triggered()), this, SLOT(on_actionTtyQuery()));

// Dialogs
    actionBSE = new QAction (tr("Bootscript Editor"), this);
    actionCE  = new QAction (tr("Configuration Editor"), this);
    actionSE  = new QAction (tr("Setup Editor"), this);

    actionHelpHelp  = new QAction (tr("&Help"), this);
    actionHelpAbout = new QAction (tr("&About"), this);
    actionAboutQt   = new QAction (tr("About &Qt"), this);
    connect(actionAboutQt, SIGNAL (triggered()), qApp, SLOT(aboutQt()));
}

/*-------------------------------------------------------------------------------------------------
 * createMenus --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::createMenus()
{
    if ((debug))
	qDebug ("[conscriptor] createMenus");

# ifdef Q_WS_MAC
    menuBar()->setNativeMenuBar (false);
# endif

    // ---------- FILE ----------------------------
    menuFile = new QMenu (tr("&File"), this);
    menuFile->addAction(actionNew);
    menuFile->addAction(actionOpen);
    menuFile->addAction(actionImport_BDF_Font);
    menuFile->addAction(actionImportImage);
    menuFile->addAction(actionConcatenateFiles);
    menuFile->addAction(actionSave);
    menuFile->addAction(actionSaveAs);
    actionSeparator = menuFile->addSeparator();

    for (int i = 0; i < MaxRecentFiles; ++i)
	menuFile->addAction (recentFileActions[i]);
    menuFile->addSeparator();

    menuFile->addAction(actionPrint);

    menuFile->addSeparator();
    menuFile->addAction(actionExit);

    // ---------- EDIT ----------------------------
# ifdef Q_WS_MAC
    menuEdit = new QMenu (tr("e&Dit"), this);
# else
    menuEdit = new QMenu (tr("&Edit"), this);
# endif
    menuEdit->addAction(actionCut);
    menuEdit->addAction(actionCopy);
    menuEdit->addAction(actionPaste);
    menuEdit->addAction(actionClear);

    menuInsert = new QMenu (tr("&Insert"), this);
    menuDelete = new QMenu (tr("&Delete"), this);
    menuMove   = new QMenu (tr("&Move"), this);
# ifdef Q_WS_MAC
    menuPixelDepth = new QMenu (tr("pi&Xel Depth"), this);
# else
    menuPixelDepth = new QMenu (tr("Pi&xel Depth"), this);
# endif

    menuEdit->addMenu(menuInsert);
    menuEdit->addMenu(menuDelete);
    menuEdit->addMenu(menuMove);
    menuEdit->addMenu(menuPixelDepth);
    menuEdit->addAction(actionFontName);

    menuInsert->addAction (actionInsertRow);
    menuInsert->addAction (actionInsertColumn);
    menuInsert->addAction (actionInsertChar);
    menuInsert->addAction (actionInsertOutline);

    menuDelete->addAction (actionDeleteRow);
    menuDelete->addAction (actionDeleteColumn);
    menuDelete->addAction (actionDeleteChar);

    menuMove->addAction (actionMoveUp);
    menuMove->addAction (actionMoveDown);
    menuMove->addAction (actionMoveLeft);
    menuMove->addAction (actionMoveRight);

    menuPixelDepth->addAction (action1to2BPP);
    menuPixelDepth->addAction (action2to1BPP);

    // ---------- VIEW -------------------------
    menuView = new QMenu (tr("&View"), this);
    menuView->addAction(actionToolbar);
    menuView->addAction(actionStatusbar);
    menuView->addAction(actionZoomIn);
    menuView->addAction(actionZoomOut);
    menuView->addAction(actionFontParameters);

    // ---------- COLOR ------------------------
    menuColour = new QMenu (tr("&Colour"), this);
    menuColour->addAction(actionClearColor);
    menuColour->addAction(actionOpaqueColor);
    menuColour->addAction(actionCthruColor);
    menuColour->addAction(actionSetClearColor);
    menuColour->addAction(actionSetOpaqueColor);
    menuColour->addAction(actionSetCthruColor);
    menuColour->addAction(actionDefaultColor);

    // ---------- SERIAL -----------------------
    menuSerial   = new QMenu (tr("&Serial"), this);
    menuPort     = new QMenu (tr("&Port"), this);
    menuBaudRate = new QMenu (tr("&Baud Rate"), this);
    menuDownload = new QMenu (tr("&Download"), this);

    menuSerial->addAction(actionPort);
    menuSerial->addAction(actionBaudRate);
    menuSerial->addMenu(menuDownload);
    menuSerial->addAction(actionTerminal);

    menuDownload->addAction (actionFont);
    menuDownload->addAction (actionFirmware);
    menuDownload->addAction (actionFirmware_repair);

    // ---------- HELP -------------------------
    menuHelp = new QMenu (tr("&Help"), this);

    // ---------- DEBUG ------------------------
    //menuDebug = new QMenu (tr("&Debug"), this);
    //menuDebug->setEnabled (false);
    
    menuBar()->addAction(menuHelp->menuAction());
    //menuBar()->addAction(menuDebug->menuAction());

    menuHelp->addAction(actionHelpHelp);
    menuHelp->addAction(actionHelpAbout);
    menuHelp->addAction(actionAboutQt);

    menuBar()->addMenu(menuFile);
    menuBar()->addMenu(menuEdit);
    menuBar()->addMenu(menuView);
    menuBar()->addMenu(menuColour);
    menuBar()->addMenu(menuSerial);
    menuBar()->addAction(actionBSE);
    menuBar()->addAction(actionCE);
    menuBar()->addAction(actionSE);
    menuBar()->addMenu(menuHelp);
    menuBar()->addAction(actionTtyQuery);

    connect(actionBSE,       SIGNAL(triggered()), SLOT(On_actionBSE()));
    connect(actionCE,        SIGNAL(triggered()), SLOT(On_actionCE()));
    connect(actionSE,        SIGNAL(triggered()), SLOT(On_actionSE()));
    connect(actionHelpAbout, SIGNAL(triggered()), SLOT(On_actionHelpAbout()));
    connect(actionHelpHelp,  SIGNAL(triggered()), SLOT(On_actionHelpHelp()));
}

/*-------------------------------------------------------------------------------------------------
 * createToolbars --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::createToolbars ()
{
    if ((debug))
	qDebug ("[conscriptor] createToolbars");

    toolbarMyApp = addToolBar(tr("&File"));
    toolbarMyApp->setIconSize (QSize (24, 24));

    toolbarMyApp->addAction(actionNew);
    toolbarMyApp->addAction(actionOpen);
    toolbarMyApp->addAction(actionSave);
    toolbarMyApp->addSeparator();

    toolbarMyApp->addAction(actionCodePoint);
    toolbarMyApp->addSeparator();

    toolbarMyApp->addAction(actionCopy);
    toolbarMyApp->addAction(actionCut);
    toolbarMyApp->addAction(actionPaste);
    toolbarMyApp->addSeparator();

    toolbarMyApp->addAction(actionPrint);
    toolbarMyApp->addSeparator();

    toolbarMyApp->addAction(actionClearColor);
    toolbarMyApp->addAction(actionOpaqueColor);
    toolbarMyApp->addAction(actionCthruColor);
    toolbarMyApp->addAction(actionOutline);
    toolbarMyApp->addSeparator();

    toolbarMyApp->addAction(actionMoveLeft);
    toolbarMyApp->addAction(actionMoveRight);
    toolbarMyApp->addAction(actionMoveUp);
    toolbarMyApp->addAction(actionMoveDown);
    toolbarMyApp->addSeparator();
    toolbarMyApp->addAction(actionRollUp);
    toolbarMyApp->addAction(actionRollDown);
    toolbarMyApp->addSeparator();
    //toolbarMyApp->addAction(actionTtyQuery);
}

/*-------------------------------------------------------------------------------------------------
 * setModified -- this SLOT is called when widgets change things eg spinBox
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setModified ()
{
    if ((debug & TRACE1))
        qDebug ("[conscriptor] setModified");
    modified = true;
}

/*-------------------------------------------------------------------------------------------------
 * save --
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::save()
{
    if ((debug))
	qDebug ("[conscriptor] save");

    if (curFile.isEmpty()) {
        on_actionSaveAs();
        return true;
    } else {
        on_actionSave();
        return true;
    }
}

/*-------------------------------------------------------------------------------------------------
 * setCurrentFile
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setCurrentFile(const QString &fileName)
{
    if ((debug))
	qDebug () << "[conscriptor] setCurrentFile" << fileName;

    curFile = fileName;
    modified = false;

    QString shownName = tr("Untitled");
    if (!curFile.isEmpty()) {
        shownName = strippedName(curFile);
        recentFiles.removeAll(curFile);
        recentFiles.prepend(curFile);
        updateRecentFileActions();
    }
    setWindowTitle(tr("%1[*] - %2").arg(shownName).arg(tr("BOB4-Conscriptor")));
}

/*-------------------------------------------------------------------------------------------------
 * okToContinue --
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::okToContinue()
{
    if (((modified)) && (!(curFile.isEmpty()))) {
        int r = QMessageBox::warning(this,
		    tr("Conscriptor"),
		    tr("The font has been modified.\n"
		       "Do you want to save your changes?"),
		    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
        if (r == QMessageBox::Yes)
            return save();
        if (r == QMessageBox::Cancel)
            return false;
    }
    return true;
}

/*-------------------------------------------------------------------------------------------------
 * openRecent --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::openRecentFile()
{
    if (okToContinue()) {
        QAction *action = qobject_cast<QAction *>(sender());
        if (action)
            loadFile(action->data().toString());
    }
}

/*-------------------------------------------------------------------------------------------------
 * updateRecentFileActions --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::updateRecentFileActions()
{
    QMutableStringListIterator i(recentFiles);
    while (i.hasNext()) {
        if (!QFile::exists(i.next()))
            i.remove();
    }

    for (int j = 0; j < MaxRecentFiles; ++j) {
        if (j < recentFiles.count()) {
            QString text = tr("&%1 %2")
                           .arg(j + 1)
                           .arg(strippedName(recentFiles[j]));
            recentFileActions[j]->setText(text);
            recentFileActions[j]->setData(recentFiles[j]);
            recentFileActions[j]->setVisible(true);
        } else {
            recentFileActions[j]->setVisible(false);
        }
    }
    actionSeparator->setVisible(!recentFiles.isEmpty());
}

/*-------------------------------------------------------------------------------------------------
 * readSettings --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::readSettings()
{
    QSettings settings("Decade", "Conscriptor");

    restoreGeometry(settings.value("geometry").toByteArray());
    recentFiles = settings.value("recentFiles").toStringList();
    updateRecentFileActions();

    portName  = settings.value("portName", "none").toString();
    baudRate  = (BaudRateType)settings.value("baudRate", (int)BAUD9600).toInt();
    dataBits  = (DataBitsType)settings.value("dataBits", (int)DATA_8).toInt();
    parity    = (ParityType)  settings.value("parity",   (int)PAR_NONE).toInt();
    stopBits  = (StopBitsType)settings.value("stopBits", (int)STOP_1).toInt();
    flowctrl  = (FlowType)    settings.value("flowctrl", (int)FLOW_XONXOFF).toInt();

    clearColor  = settings.value("clearColor",  QColor (0, 255, 0, 127)).value<QColor>();
    opaqueColor = settings.value("opaqueColor", QColor (255, 128, 128, 127)).value<QColor>();
    cthruColor  = settings.value("cthruColor",  QColor (128, 128, 128, 127)).value<QColor>();
}

/*-------------------------------------------------------------------------------------------------
 * writeSettings --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::writeSettings()
{
    QSettings settings("Decade", "Conscriptor");

    settings.setValue("geometry", saveGeometry());
    settings.setValue("recentFiles", recentFiles);
    settings.setValue("portName", portName);
    settings.setValue("baudRate", baudRate);

    settings.setValue("clearColor",  clearColor);
    settings.setValue("opaqueColor", opaqueColor);
    settings.setValue("cthruColor",  cthruColor);
}

/*-------------------------------------------------------------------------------------------------
 * loadFile --
 *	delete any previous b4font and all it's glyphs and QImages
 *	free the metrixBase data set
 *	free the pixmap data set
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::loadFile(const QString &fileName)
{
char *p;
char *q;

    if ((debug))
	qDebug ("[conscriptor] loadFile---");
    p = (char *)metrixBase;
    q = (char *)pixmap;
    if ((b4font->nGlyphs))
	b4font->cleanup();

    QFile file;
    setCurrentFile (fileName);

    file.setFileName(curFile);
    if (!file.open(QIODevice::ReadWrite)) {
	QMessageBox::warning(this, tr ("conscriptor"),
		tr ("Cannot read file %1:%2.")
		.arg(curFile)
		.arg(file.errorString()));
	return false;
    }

    file.read((char *) &header, 32);

    /*---------------------------------------------------------------------------------------------
     * set metricCount based on header info read
     *-------------------------------------------------------------------------------------------*/
    int byteCount = (header.chCount +1) *6;
    if (((byteCount % 32) == 0) && (byteCount > 0))
        metricCount = byteCount;
    else
        metricCount = ((byteCount / 32) + 1) *32;
    metrixBase = (Conscriptor::metrixtag *)calloc (metricCount, 1);
    if (debug & TRACEE)
	qDebug ("         calloc metrixBase 0x%lx (%d)", (long)metrixBase, metricCount);
    file.read((char *)metrixBase, metricCount);

    /*---------------------------------------------------------------------------------------------
     * pixWidth is 32 bit aligned
     *-------------------------------------------------------------------------------------------*/
    if ((header.pixWidth % 4))
	qCritical ("[error] pixWith read from file is not 32bit aligned:%d", header.pixWidth);

    /*---------------------------------------------------------------------------------------------
     * pixmapCount is 32 byte aligned
     *-------------------------------------------------------------------------------------------*/
    b4font->setPixmapCount();
    pixmap = (char *)calloc (pixmapCount, 1);
    if (debug & TRACEE)
	qDebug ("         calloc     pixmap 0x%lx (%d)", (long)pixmap, pixmapCount);
    file.read((char *)pixmap, pixmapCount);
    file.close();

    if (header.chDepth == 0) {
	actionInsertOutline->setEnabled (false);
	actionOutline->setEnabled (false);
	actionCthruColor->setEnabled (false);
	action1to2BPP->setEnabled (true);
	action2to1BPP->setEnabled (false);
	if (b4font->colorFlag == B4Font::cthruCol)
	    b4font->colorFlag = B4Font::opaqueCol;
   }
    else {
	actionInsertOutline->setEnabled (true);
	actionOutline->setEnabled (true);
	actionCthruColor->setEnabled (true);
	action1to2BPP->setEnabled (false);
	action2to1BPP->setEnabled (true);
    }

    b4font->pix2asc();  
    b4font->lineLength = 0;
    b4font->setupGlyphs();  
    b4font->resetXY();  
    
    //setScrollSize ();

    /*---------------------------------------------------------------------------------------------
     * Free pixmap and metrics if they were already assigned
     * calloc before free saves thread conflicts
     *-------------------------------------------------------------------------------------------*/
    if ((p)) {
	if (debug & TRACEE)
	    qDebug ("         free   metrixBase 0x%lx (previous)", (long)p);
	free (p);
	if ((q)) {
	    if (debug & TRACEE)
		qDebug ("         free       pixmap 0x%lx (previous)", (long)q);
	    free (q);
	}
	else
	    qDebug ("[BUG] pixmapCount[%d] metricCount[%d]", pixmapCount, metricCount);
    }
    else if ((q)) {
	qDebug ("[BUG] pixmapCount[%d] metricCount[%d]", pixmapCount, metricCount);
	if (debug & TRACEE)
	    qDebug ("         free       pixmap 0x%lx", (long)q);
	free (q);
    }
    b4font->update();
    if ((debug))
	qDebug ("             /loadFile--- metricCount[%d] pixmapCount[%d]",
									metricCount, pixmapCount);
    return true;
}

/*=================================================================================================
 * FILE
 * on_actionNew -- all calloc'd entries set to 0
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionNew ()
{
QString newName;

    if ((debug))
	qDebug ("[conscriptor] on_actionNew");

    if (((modified)) && (!(curFile.isEmpty()))) {
        int r = QMessageBox::warning(this,
                    tr("New File"),
                    tr("The existing font has been modified.\n"
                       "Do you want to save it?"),
                    QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes)
            save();
    }

    /*---------------------------------------------------------------------------------------------
     * dialog sets header info, font metrics and sets the blank pixmap
     *-------------------------------------------------------------------------------------------*/
    b4font->lineLength = 200;
    NewfileDialog *dialog = new NewfileDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    if (dialog->exec() == QDialog::Rejected) {
	return;
    }
    newName = (char *)header.name;
    if (!(newName.endsWith(".b4f", Qt::CaseSensitive)))
        newName.append (".b4f");

    setCurrentFile (newName);
    modified = false;

    if (debug & TRACE1) {
	qDebug () << "              [name]      " << (char *)header.name;
	qDebug () << "              [count]     " << header.chCount;
	qDebug () << "              [height]    " << header.chHeight;
	qDebug () << "              [depth]     " << (header.chDepth ? 2 : 1) << "BPP";
	qDebug () << "              [pixWidth]  " << header.pixWidth;
	qDebug () << "               metricCount" << metricCount;
	qDebug () << "               pixmapCount" << pixmapCount;
    }

    if (header.chDepth == 0) {
	actionInsertOutline->setEnabled (false);
	actionOutline->setEnabled (false);
	actionCthruColor->setEnabled (false);
	action1to2BPP->setEnabled (true);
	action2to1BPP->setEnabled (false);
	if (b4font->colorFlag == B4Font::cthruCol)
	    b4font->colorFlag = B4Font::opaqueCol;
   }
    else {
	actionInsertOutline->setEnabled (true);
	actionOutline->setEnabled (true);
	actionCthruColor->setEnabled (true);
	action1to2BPP->setEnabled (false);
	action2to1BPP->setEnabled (true);
    }

    // WHY b4font->setPixmapWidth ();		// Now Glyphs are allocated, set size of pixmap
    // WHY b4font->setMetricCount ();
    b4font->resetXY();  
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * setScrollSize --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setScrollSize ()
{
    int w = qMax ((int)(this->size().width() * 0.95), b4font->lineLength + 20);
        w = qMax (w, b4font->macroWidth + 100);
    int h = b4font->allglyphs[b4font->nGlyphs -1]->getY()
				    + b4font->allglyphs[b4font->nGlyphs -1]->getHeight() +100;

    //qDebug ("[conscriptor] setScrollSize last glyph at %d it's height %d",
    //					    b4font->allglyphs[b4font->nGlyphs -1]->getY(),
    //					    b4font->allglyphs[b4font->nGlyphs -1]->getHeight());

    //qDebug ("[conscriptor] setScrollSize ll:%d mG:%d wind:%d w:%d h:%d",
    //					    b4font->lineLength + 20,
    //					    b4font->macroWidth + 100,
    //					    (int)(this->size().width() * 0.95),
    //					    w,
    //					    h);
    b4font->setMinimumSize(w, h);
}

/*-------------------------------------------------------------------------------------------------
 * on_actionOpen --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionOpen ()
{
    QFile file;
    if ((debug))
	qDebug ("[conscriptor] on_actionOpen");

    if (modified == true) {
	QMessageBox msgBox;
	msgBox.setWindowTitle (strippedName (curFile));
	msgBox.setText(tr ("\"%1\" not saved.").arg(curFile));
        msgBox.setIcon (QMessageBox::Question);
	msgBox.setInformativeText("Proceed anyway?");
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::No);
	msgBox.setDefaultButton(QMessageBox::No);
        if (msgBox.exec() == QMessageBox::No)
	    return;
	}

    QString fileName = QFileDialog::getOpenFileName(this, "read b4F fonts", "", "fonts (*.b4f)");
    if (fileName.isNull())
	return;

    setCurrentFile(fileName);
    loadFile (fileName);

    if (debug & TRACE1) {
	qDebug () << "        [name]       " << (char *)header.name;
	qDebug () << "        [count]      " << header.chCount;
	qDebug () << "        [height]     " << header.chHeight;
	qDebug () << "        [depth]      " << (header.chDepth ? 2 : 1)
											<< "BPP";
	qDebug () << "        [pixWidth]   " << header.pixWidth;
	qDebug () << "          metricCount" << metricCount;
	qDebug () << "          pixmapCount" << pixmapCount;
    }
}

/*-------------------------------------------------------------------------------------------------
 * on_actionImport_BDF_Font --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionImport_BDF_Font ()
{
    if ((debug))
	qDebug ("[conscriptor] on_actionImportBDF_Font");

    if (((modified)) && (!(curFile.isEmpty()))) {
        int r = QMessageBox::warning(this,
                    tr("Import BDF Font"),
                    tr("The existing font has been modified.\n"
                       "Do you want to save it?"),
                    QMessageBox::Yes | QMessageBox::No);
        if (r == QMessageBox::Yes)
            save();
    }

    curFile = QFileDialog::getOpenFileName(this, "read BDF fonts", "", "fonts (*.bdf)");
    if ((curFile.isNull()))
	return;

    QFile file;
    file.setFileName (curFile);
    if (!file.open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this, tr ("import BDF"),
                tr ("Cannot read file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }

    ba = file.readAll();
    file.close ();

    bdfDialog = new ImportDialog (this);
    bdfDialog->setAttribute(Qt::WA_DeleteOnClose);

    bdfDialog->editFontName->setText (baseName(curFile));

    /*---------------------------------------------------------------------------------------------
     * Number of characters advertised
     *-------------------------------------------------------------------------------------------*/
    int indx = ba.indexOf ("CHARS ");
    int val;
    char str[16];
    sscanf (ba.data()+indx, "CHARS %d", &val);
    sprintf (str, "%d", val);
    bdfDialog->editCount->setText (str);
    bdfDialog->spinCount->setMaximum (65535);
    bdfDialog->spinCount->setValue (val);

    /*---------------------------------------------------------------------------------------------
     * Adobe's spec does not require characters to be sorted. The lowest codepoint may be anywhere
     *-------------------------------------------------------------------------------------------*/
    int encoding;
    for (encoding = 100000; ;indx += 10) {
	if ((indx = ba.indexOf ("ENCODING", indx)) == -1)
	    break;
	if ((sscanf (ba.data()+indx, "ENCODING %d", &val)) != 1)
	    continue;
	if (val < 1)			// do something See Adobe BDF spec
	    continue;
	encoding = qMin (encoding, val);
    } 
    sprintf (str, "%d   0x%02x", encoding, encoding);
    bdfDialog->editFirst->setText (str);
    bdfDialog->spinFirst->setMaximum (65535);
    bdfDialog->spinFirst->setValue (encoding);
    //bdfDialog->checkBox->setCheckState (Qt::Checked);
    bdfDialog->radio1BPP->setChecked (true);
    bdfDialog->exec();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionConcatenateFiles --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionConcatenateFiles ()
{
QStringList files;
QString fileName;
QString eachName;

QByteArray ba;
QByteArray tmp;

QFile file;

int mtricCount;
int pxmapCount;
int next;
int totalSize;
int oldSize;
int fileCount;

struct fonttag headerOne;

    if ((debug))
	qDebug ("[conscriptor] on_actionConcatenateFiles");

    ba.clear ();

    files = QFileDialog::getOpenFileNames(this, "select two or more files" , "", "fonts (*.b4f)");
    fileName = QFileDialog::getSaveFileName(this, "select save file name", "", "fonts (*.b4f)");

    if (!(fileName.endsWith(".b4f", Qt::CaseSensitive)))
	fileName.append (".b4f");

    for (fileCount = 0; fileCount < files.size(); fileCount++) {
	file.setFileName (files.at(fileCount).toLocal8Bit().constData());
	if (!file.open(QIODevice::ReadWrite)) {
	    QMessageBox::warning(this, tr ("import BDF"),
		    tr ("Cannot read file %1:%2.")
		    .arg(curFile)
		    .arg(file.errorString()));
	    return;
	}

	tmp = file.readAll();
	file.close ();
	ba.append (tmp);
	tmp.clear();
    }

    for (oldSize = totalSize = 0; ;oldSize = totalSize) {
	memcpy (&headerOne, ba.data() + totalSize, sizeof (headerOne));

	int byteCount = (headerOne.chCount +1) *6;
	if (((byteCount % 32) == 0) && (byteCount > 0))
	    mtricCount = byteCount;
	else
	    mtricCount = ((byteCount / 32) + 1) *32;

	pxmapCount = headerOne.pixWidth * headerOne.chHeight;
	if ((pxmapCount % 32))
	    pxmapCount = ((pxmapCount / 32) +1) * 32;

	next = 32 + pxmapCount + mtricCount;
	totalSize += next;
	//qDebug () << "totalSize" << totalSize << "ba size" << ba.size();
	if (totalSize >= ba.size ())
	    // Not explicitly setting skip to 0 is probably OK, not nice
	    break;
	headerOne.skip = next;
	memcpy (ba.data() + oldSize, &headerOne, sizeof (headerOne));
	//qDebug ("skip = %d", next);
    }
  
    file.setFileName (fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot create file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }
    file.write (ba);
    file.close ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSave --
 *	do nothing if a file is not opened
 *	header is correct
 *	metrics are correct
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionSave ()
{
    if ((debug))
	qDebug ("[conscriptor] on_actionSave");

    if (b4font->nGlyphs == 0) {
        QMessageBox::information(this, tr ("conscriptor"),
                tr ("No glyphs (loaded or new). No file will be saved"));
	return;
    }

# if _WIN32
    QString theName = curFile;
    theName.prepend ("C:\\Documents and Settings\\All Users\\Desktop\\");
    QFile file(theName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot create file %1:%2.")
                .arg(theName)
                .arg(file.errorString()));
        return;
    }
# else
    QFile file(curFile);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot create file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }
# endif

    b4font->setPixmapWidth ();		// needed by makeAsciiMap ()
    b4font->makeAsciiMap ();		// make an ascii map from glyphs
    b4font->makePixmap ();		// make a pixmap from ascii map

    int n = file.write ((char *) &header, 32);
    //qDebug ("    write header:32");
    n = file.write ((char *) metrixBase, metricCount);
    //qDebug ("    write metrics:%d", n);
    n = file.write (pixmap,  pixmapCount);
    //qDebug ("    write pixmap:%d", n);
    file.close();
    modified = false;
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSaveAs --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionSaveAs ()
{
    if ((debug))
	qDebug ("[conscriptor] on_actionSaveAs");

    if (b4font->nGlyphs == 0) {
        QMessageBox::information(this, tr ("conscriptor"),
                tr ("No File loaded so no file will be saved"));
	return;
    }
#  if _WIN32
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
	    "C:\\Documents and Settings\\All Users\\Desktop", tr("BOB4 Font Files (*.b4f)"));
#  else
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), ".",
	    tr("BOB4 Font Files (*.b4f)"));
#endif

    if (!(fileName.endsWith(".b4f", Qt::CaseSensitive)))
	fileName.append (".b4f");

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot read file %1:%2.")
                .arg(fileName)
                .arg(file.errorString()));
        return;
    }

    b4font->setPixmapWidth ();		// needed by makeAsciiMap ()
    b4font->makeAsciiMap ();		// make an ascii map from glyphs and metrics
    b4font->makePixmap ();		// make a pixmap from ascii map

    int n = file.write ((char *) &header, 32);
    n = file.write ((char *) metrixBase, metricCount);
    n = file.write (pixmap,  pixmapCount);
    file.close();
    modified = false;
}

/*-------------------------------------------------------------------------------------------------
 * on_actionPrint --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionPrint ()
{
    if (b4font->nGlyphs == 0) {
	statusBar()->showMessage(tr("Nothing to print"), 5000);
	return;
    }

    PrintDialog *dialog = new PrintDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionExit --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionExit ()
{
    if (okToContinue()) {
        writeSettings();
	qApp->quit ();
	return;
    }
}

/*=================================================================================================
 * EDIT
 *	most edit functions are font related and they are handled by SLOTS in the font class
 *-------------------------------------------------------------------------------------------------
 * edit.pixel depth
 * on_action1to2BPP --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_action1to2BPP ()
{
    if ((debug))
	qDebug () << "1BPP to 2BPP";
    actionInsertOutline->setEnabled (true);
    actionOutline->setEnabled (true);
    actionCthruColor->setEnabled (true);
    action1to2BPP->setEnabled (false);
    action2to1BPP->setEnabled (true);
    modified = true;
    header.chDepth = 1;
    b4font->setPixmapWidth ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.pixel depth
 * on_action2to1BPP --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_action2to1BPP ()
{
    if ((debug))
	qDebug () << "2BPP to 1BPP";
    actionInsertOutline->setEnabled (false);
    actionOutline->setEnabled (false);
    actionCthruColor->setEnabled (false);
    action1to2BPP->setEnabled (true);
    action2to1BPP->setEnabled (false);
    modified = true;
    header.chDepth = 0;
    b4font->setPixmapWidth ();
    b4font->resetColor (cthruColor.rgb(), clearColor.rgb());
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionFontName --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionFontName ()
{
    FontNameDialog *dialog =  new FontNameDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle (strippedName (curFile));
    dialog->exec();
    b4font->update();
}

/*=================================================================================================
 * VIEW
 * on_actionToolbar --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionToolbar ()
{
    if (actionToolbar->isChecked())
	toolbarMyApp->setVisible (true);
    else
	toolbarMyApp->setVisible (false);
}


/*-------------------------------------------------------------------------------------------------
 * on_actionStatusbar --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionStatusbar ()
{
    if (actionStatusbar->isChecked())
	statusBar()->setVisible (true);
    else
	statusBar()->setVisible (false);
}

/*-------------------------------------------------------------------------------------------------
 * on_actionZoomIn --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionZoomIn ()
{
    switch (b4font->macroMagnification) {
	case 1: b4font->macroMagnification = 2;
		break;
	case 2: b4font->macroMagnification = 3;
		break;
	case 3: b4font->macroMagnification = 4;
		break;
	case 4: b4font->macroMagnification = 5;
		break;
	case 5: b4font->macroMagnification = 6;
		break;
	case 6: b4font->macroMagnification = 7;
		break;
	case 7: b4font->macroMagnification = 8;
		break;
	case 8: b4font->macroMagnification = 9;
		break;
	case 9: b4font->macroMagnification = 10;
		break;
	case 10: b4font->macroMagnification = 10;
		break;
	default: b4font->macroMagnification = 4;
    }
    b4font->resetXY ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionZoomOut --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionZoomOut ()
{
    switch (b4font->macroMagnification) {
	case 1: b4font->macroMagnification = 1;
		break;
	case 2: b4font->macroMagnification = 1;
		break;
	case 3: b4font->macroMagnification = 2;
		break;
	case 4: b4font->macroMagnification = 3;
		break;
	case 5: b4font->macroMagnification = 4;
		break;
	case 6: b4font->macroMagnification = 5;
		break;
	case 7: b4font->macroMagnification = 6;
		break;
	case 8: b4font->macroMagnification = 7;
		break;
	case 9: b4font->macroMagnification = 8;
		break;
	case 10: b4font->macroMagnification = 9;
		break;
	default: b4font->macroMagnification = 4;
    }
    b4font->resetXY ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionFontParameters --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionFontParameters ()
{
char line1[160];

    if (b4font->nGlyphs == 0)
	return;

    sprintf (line1, "Name: %s\n"
		    "First:\t%d\n"
		    "Skip:\t%d\n"
		    "Height:\t%d\n"
		    "PixWidth:\t%d bytes\n"
		    "Ascent:\t%d\n"
		    "Chars:\t%d\n"
		    "LDepth:\t%d %s\n",
		    header.name,
		    (int)header.chFirst,
		    header.skip,
		    (int)header.chHeight,
		    header.pixWidth,
		    (int)header.chAscent,
		    (int)header.chCount,
		    (int)header.chDepth,
		    header.chDepth ? "(2BPP)" : "(1BPP)"
		    );

    QMessageBox msgBox;
    msgBox.setWindowTitle ("Font Parameters");
    msgBox.setText(line1);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}


/*=================================================================================================
 * COLOR
 * on_actionClearColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionClearColor ()
{
    b4font->colorFlag = B4Font::clearCol;
}

/*-------------------------------------------------------------------------------------------------
 * on_actionOpaqueColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionOpaqueColor ()
{
    b4font->colorFlag = B4Font::opaqueCol;
}

/*-------------------------------------------------------------------------------------------------
 * on_actionCthruColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionCthruColor ()
{
    b4font->colorFlag = B4Font::cthruCol;
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSetClearColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionSetClearColor ()
{
    QColor color = QColorDialog::getColor (clearColor, this,
			    "Select Color for Clear Pixels", QColorDialog::DontUseNativeDialog);
    b4font->resetColor (clearColor.rgb(), color.rgb());
    clearColor = color;
    setClearIcon ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSetOpaqueColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionSetOpaqueColor ()
{
    QColor color = QColorDialog::getColor (opaqueColor, this,
			    "Select Color for Opaque Pixels", QColorDialog::DontUseNativeDialog);
    b4font->resetColor (opaqueColor.rgb(), color.rgb());
    opaqueColor = color;
    setOpaqueIcon ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSetCthruColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionSetCthruColor ()
{
    QColor color = QColorDialog::getColor (cthruColor, this,
			    "Select Color for See thru Pixels", QColorDialog::DontUseNativeDialog);
    b4font->resetColor (cthruColor.rgb(), color.rgb());
    cthruColor = color;
    setCthruIcon ();
    b4font->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionDefaultColor --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionDefaultColor ()
{
QColor color;

    color.setRgb  (0, 255, 0);
    b4font->resetColor (clearColor.rgb(), color.rgb());
    clearColor = color;
    setClearIcon  ();

    color.setRgb (250, 250, 250);
    b4font->resetColor (opaqueColor.rgb(), color.rgb());
    opaqueColor = color;
    setOpaqueIcon ();

    color.setRgb  (128, 128, 128);
    b4font->resetColor (cthruColor.rgb(), color.rgb());
    cthruColor = color;
    setCthruIcon  ();

    b4font->update();
}

/*=================================================================================================
 * SERIAL
 * on_actionTtyS0 --
 *-----------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------
 * on_actionFont --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionFont ()
{
    setTtyPort ();
    QString fileName = QFileDialog::getOpenFileName(this, "read b4F fonts", "", "fonts (*.b4f)");
    if (fileName.isNull())
	return;

    if (commPort == XBOBP) {
	font_main_port (fileName);
    }
    else {
	setTtyDebug ();
	if (commPort == DEBUGP)
	    font_debug_port (fileName);
	else
	    QMessageBox::warning(this, tr("Conscriptor"), tr("No Conscriptor detected"));
    }
    //qDebug ("on_actionFont");
}

/*-------------------------------------------------------------------------------------------------
 * on_actionFirmware --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionFirmware ()
{
    //qDebug ("on_actionFirmware");
    setTtyPort ();

    if (commPort == XBOBP)
	firmware_main_port ();
    else {
	setTtyDebug ();
	if (commPort == DEBUGP)
	    firmware_debug_port ();
	else
	    QMessageBox::warning(this, tr("Conscriptor"), tr("No Conscriptor detected"));
    }
}

/*-------------------------------------------------------------------------------------------------
 * on_action1BaudRate --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionBaudRate ()
{
    baudselectDialog = new BaudSelectDialog (this);
    baudselectDialog->setAttribute(Qt::WA_DeleteOnClose);
    switch (baudRate) {
	case BAUD1200:
	    baudselectDialog->radio1200->setChecked (true);
	    break;
	case BAUD2400:
	    baudselectDialog->radio2400->setChecked (true);
	    break;
	case BAUD4800:
	    baudselectDialog->radio4800->setChecked (true);
	    break;
	case BAUD9600:
	    baudselectDialog->radio9600->setChecked (true);
	    break;
	case BAUD19200:
	    baudselectDialog->radio19200->setChecked (true);
	    break;
	case BAUD38400:
	    baudselectDialog->radio38400->setChecked (true);
	    break;
	case BAUD115200:
	    baudselectDialog->radio115200->setChecked (true);
	    break;
	case BAUD50:
	case BAUD75:
	case BAUD110:
	case BAUD134:
	case BAUD150:
	case BAUD200:
	case BAUD300:
	case BAUD600:
	case BAUD1800:
	case BAUD14400:
	case BAUD56000:
	case BAUD57600:
	case BAUD76800:
	case BAUD128000:
	case BAUD256000:
	    break;
    }
    baudselectDialog->exec();
}

/*-------------------------------------------------------------------------------------------------
 * on_action1Port --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionPort ()
{
    portselectDialog = new PortSelectDialog (this);
    portselectDialog->setAttribute(Qt::WA_DeleteOnClose);

    statusBar()->showMessage
		(tr("Be Careful: you are allowed to select a device that does not exist"));
# ifndef _WIN32
    if ((portName.contains ("ttyS0")))
	portselectDialog->radioTty1->setChecked (true);
    else if ((portName.contains ("ttyS1")))
	portselectDialog->radioTty2->setChecked (true);
    else if ((portName.contains ("ttyS2")))
	portselectDialog->radioTty3->setChecked (true);
    else if ((portName.contains ("ttyS3")))
	portselectDialog->radioTty4->setChecked (true);
    else if ((portName.contains ("ttyUSB0")))
	portselectDialog->radioTty5->setChecked (true);
    else if ((portName.contains ("ttyUSB1")))
	portselectDialog->radioTty6->setChecked (true);
    else if ((portName.contains ("ttyUSB2")))
	portselectDialog->radioTty7->setChecked (true);
    else if ((portName.contains ("ttyUSB3")))
	portselectDialog->radioTty8->setChecked (true);
    else if ((portName.contains ("ttys000")))
	portselectDialog->radioTty9->setChecked (true);
#  else
    if ((portName.contains ("COM1")))
	portselectDialog->radioTty1->setChecked (true);
    else if ((portName.contains ("COM2")))
	portselectDialog->radioTty2->setChecked (true);
    else if ((portName.contains ("COM3")))
	portselectDialog->radioTty3->setChecked (true);
    else if ((portName.contains ("COM4")))
	portselectDialog->radioTty4->setChecked (true);
    else if ((portName.contains ("COM5")))
	portselectDialog->radioTty5->setChecked (true);
    else if ((portName.contains ("COM6")))
	portselectDialog->radioTty6->setChecked (true);
    else if ((portName.contains ("COM7")))
	portselectDialog->radioTty7->setChecked (true);
    else if ((portName.contains ("COM8")))
	portselectDialog->radioTty8->setChecked (true);
    else if ((portName.contains ("COM9")))
	portselectDialog->radioTty9->setChecked (true);
#  endif
    portselectDialog->exec();
}

/*-------------------------------------------------------------------------------------------------
 * serial.download
 * on_actionTerminal --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionTerminal ()
{
    terminalDialog = new TerminalDialog (this);
    terminalDialog->setAttribute(Qt::WA_DeleteOnClose);
    terminalDialog->exec();
}

/*-------------------------------------------------------------------------------------------------
 * font_main_port --
 *	serial port is OPEN
 *	
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::font_main_port (QString fileName)
{
    char cmd[32];
    int addr;
    int x;
    QByteArray ba;

    SetmemDialog *dialog = new SetmemDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();

    sprintf (cmd, "\x1b" "[93;%d;0|", flashDevice);	// undocumented.
							// flashDevice=0..4:dialog, 0..7:Eric's code
    ttyPort->write (cmd, strlen (cmd));
    statusBar()->showMessage(tr("Erase Font"), 2000);
# ifdef _WIN32
    Sleep (100);
# endif
    ba = ttyPort->readAll();
    if (!(ba.contains (ACK))) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("No ACK from erase"));
	return false;
    }

    QFile file;
    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadWrite)) {
	QMessageBox::warning(this, tr ("conscriptor"),
		tr ("Cannot read file %1:%2.")
		.arg(curFile)
		.arg(file.errorString()));
	return false;
    }

    QProgressDialog progress("Downloading font...", "Abort", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    int end = file.size() / 256;
    progress.setMaximum(end);
    progress.setMinimumDuration (0);
    
    ttyPort->setTimeout (0, 50);
    for (int i = addr = 0; ;i++, addr += 256) {
        quint8 buf[256];
	if ((x = file.read((char *)buf, 256)) <= 0)
	    break;

	progress.setValue(i);
	QApplication::processEvents();
	statusBar()->showMessage(tr("Download Font sector %1").arg (i), 1000);
	//qDebug ("[on_actionFont_main_port] write sector %d", addr);
        if (!bob4Send85(flashDevice, addr, &buf[0], x, 0)) {
	    QMessageBox::warning(this, tr("Conscriptor"), tr("Font download failed (BOB:no ACK)"));
            return false;
        }
    }
    progress.setValue(end);
    ttyPort->setTimeout (0, 500);
    sprintf (cmd, "\x1b" "[92;%d;1|", flashDevice);	// undocumented.
    ttyPort->write (cmd, strlen (cmd));
    statusBar()->showMessage(tr("Flush Font"), 2000);
# ifdef _WIN32
    Sleep (100);
# endif
    ba = ttyPort->readAll();
    if (!(ba.contains (ACK))) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("No ACK from flush"));
	return false;
    }
    ttyPort->setTimeout (0, 500);
    return true;
}

/*-------------------------------------------------------------------------------------------------
 * bootscriptDownload --
 *	serial port OPEN
 *	
 * XBOB-4 boots with a big "BOB-4" splash screen display by default. This display is cleared when
 * the first incoming character is detected. To eliminate the splash screen entirely, just clear
 * the default boot script. This can be accomplished by sending <ESC>X<ESC>\ (empty string)
 * <CSI>8v (capture boot script) <CSI>1v (store new configuration).
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::bootscriptDownload (QByteArray &ba)
{
    char str[16];
    if (isBOB () == false) {
	QMessageBox::warning(this, tr("Conscriptor"),
				   tr("No Conscriptor detected"));
	return false;
    }

    switch (commPort) {
	case DEBUGP:
            QMessageBox::warning(this, tr("Conscriptor"),
                                   tr("<p><i>bootscript</i> data can be loaded on the debug port "
                                      "but it is tricky to do, so we load to <font color='#f00000'>"
                                      "main port</font> only\n"
                                      "XBOB was detected on debug port\n"
                                      "You can use <font color='#0000f0'>serial->terminal </font>"
                                      "to do this task manually. See Application Guide</p>"));
	    return false;
	case XBOBP:
	    statusBar()->showMessage(tr("Download bootscript"), 3000);
	    sprintf (str, "\x1b" "X");
	    ttyPort->write (str, strlen (str));
	    ttyPort->write (ba);
	    sprintf (str, "\x1b" "\\" "\x1b" "[8v" "\x1b" "[1v");
	    ttyPort->write (str, strlen (str));
	    statusBar()->showMessage(tr("Download complete"), 6000);
	    return true;
    }
}

/*-------------------------------------------------------------------------------------------------
 * configDownload --
 *	serial port OPEN
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::configDownload (QByteArray &ba)
{
    if (isBOB () == false) {
	QMessageBox::warning(this, tr("Conscriptor"),
				   tr("No Conscriptor detected"));
	return false;
    }

    switch (commPort) {
	case DEBUGP:
	    ba.replace ("[", "config ");
	    ba.replace (';', '=');
	    ba.replace ('v', '\n');
qDebug () << ba;
	    return true;
	case XBOBP:
	    statusBar()->showMessage(tr("Download config"), 3000);
	    ttyPort->write (ba);
	    statusBar()->showMessage(tr("Download complete"), 6000);
	    return true;
    }
}

/*-------------------------------------------------------------------------------------------------
 * font_debug_port --
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::font_debug_port (QString fileName)
{
    char cmd[32];
    int addr;
    int x;
    QByteArray ba;

    SetmemDialog *dialog = new SetmemDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();

    sprintf (cmd, "memory erase %d 0\n", flashDevice);		// undocumented
    ttyPort->write (cmd, strlen (cmd));
# ifdef _WIN32
    Sleep (100);
# endif
    ba = ttyPort->readAll();
    if (!(ba.contains ('>'))) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("No prompt after erase"));
	return false;
    }

    QFile file;
    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadWrite)) {
	QMessageBox::warning(this, tr ("conscriptor"),
		tr ("Cannot read file %1:%2.")
		.arg(curFile)
		.arg(file.errorString()));
	return false;
    }

    QProgressDialog progress("Downloading font...", "Abort", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMaximum(file.size() / 256);
    progress.setMinimumDuration (0);
    
    ttyPort->setTimeout (0, 50);
    for (int i = addr = 0; ;i++, addr += 256) {
        quint8 buf[256];
	if ((x = file.read((char *) &buf, 256)) <= 0)
	    break;

	progress.setValue(i);
	QApplication::processEvents();
	statusBar()->showMessage(tr("Download Font sector %1").arg (i), 1000);

        if (!bob4Send85(flashDevice, addr, &buf[0], x, 1)) {
	    QMessageBox::warning(this, tr("Conscriptor"), tr("Font download failed (BOB:no ACK)"));
            return false;
        }
    }

    statusBar()->showMessage(tr("Flush Font"), 2000);
    ttyPort->setTimeout (0, 500);
    sprintf (cmd, "memory flush %d 1\n", flashDevice);	// undocumented.
    ttyPort->write (cmd, strlen (cmd));
# ifdef _WIN32
    Sleep (100);
# endif
    ba = ttyPort->readAll();
    if (!(ba.contains ('>'))) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("No prompt after flush"));
	return false;
    }
    statusBar()->showMessage(tr("Download Complete"), 3000);
    return true;
}

/*-------------------------------------------------------------------------------------------------
 * bootloaderLoad --
 *      firmware main port
 *      firmware debug port
 *      firmware repair
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::bootloaderLoad (QString fileName, bool normal)
{
    int sectorAddr;
    int x;
    QByteArray ba;
    QFile file;

    if ((debug & TRACE5))
	qDebug () << "[bootloaderLoad]" << fileName;

    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadWrite)) {
        QMessageBox::warning(this, tr ("conscriptor"),
                tr ("Cannot read file %1:%2.")
                .arg(curFile)
                .arg(file.errorString()));
        return;
    }

    QProgressDialog progress("Downloading firmware...", "Abort", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    int theEnd = file.size () / 128;
    progress.setMaximum(theEnd);
    progress.setMinimumDuration (0);

    /*---------------------------------------------------------------------------------------------
     * The firmware file is taken to ALWAYS be nn * 128 (128 is flash sector size) Else fails here
     *-------------------------------------------------------------------------------------------*/
    int i;
    for (i = 0; i < 50; ++i) {
	char a;
	ttyPort->write ("S104000,#", 9);
	if ((debug & TRACE5))
	    qDebug () << "[wrote] S104000,# [read]" << ttyPort->bytesAvailable ()
						    << ttyPort->errorString();

	if ((normal)) {
	    for (int tries = 0; tries < 10; tries++) {
#	      ifdef _WIN32
		Sleep (40);
#	      else
		usleep (40000);
#	      endif

		ba = ttyPort->readAll();
		if ((debug & TRACE5))
		    qDebug () << "[" << tries << "ba]" << ba;
		if (ba.contains ('C')) {
		    //qDebug ("[ACK] n break");
		    break;
		}
	    }
	    if (ba.contains ('C'))
		break;
	}
	else {
#	      ifdef _WIN32
		Sleep (40);
#	      else
		usleep (40000);
#	      endif

	    ttyPort->getChar (&a);
	    ttyPort->getChar (&a);
	    ttyPort->getChar (&a);

	    if (a == 'C')
		break;
	}
	if (a == 'C')
	    break;
    }

    if (i == 50) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("Failed to start download"
				      " (no C response to S104000,#"));
        return;
    }

    if ((debug & TRACE5))
	qDebug () << "[sending sectors]";

    for (int i = sectorAddr = 0; ;i++) {
	quint8 buf[128];
	if ((x = file.read((char *) &buf, 128)) < (qint64)(sizeof (buf)))
	    break;
	progress.setValue(i);
	QApplication::processEvents();
	statusBar()->showMessage(tr("Download Firmware sector %1").arg (i), 1000);

	if (!((writeXModem (++sectorAddr & 0xff, (char *)buf)))) {
	    QMessageBox::warning(this, tr("Conscriptor"),
				       tr("Error uploading packet %1").arg (sectorAddr));
	    return;
	}
    }
    progress.setValue(theEnd);
    writeXModem (0, 0);
    statusBar()->showMessage(tr("Download complete"), 3000);
    ttyPort->write ("g#", 2);
    setTtyPort ();
}

/*-------------------------------------------------------------------------------------------------
 * firmware_main_port --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::firmware_main_port ()
{
    char cmd[32];

    QString fileName = QFileDialog::getOpenFileName(this, "Firmware", "", "firmware (*.enc)");
    if (fileName.isNull())
        return;

    /*---------------------------------------------------------------------------------------------
     * Firmare Download, start bootloader at 115200 baud
     *-------------------------------------------------------------------------------------------*/
    sprintf (cmd, "\x1b" "[3210;1|");
    ttyPort->write (cmd, strlen (cmd));


    delete ttyPort;
    ttyPort = new QextSerialPort();

    ttyPort->setPortName(portName);
    ttyPort->setParity(PAR_NONE);
    ttyPort->setBaudRate(BAUD115200);
    ttyPort->setFlowControl(FLOW_XONXOFF);
    ttyPort->setDataBits(DATA_8);
    ttyPort->setStopBits(STOP_1);
    ttyPort->setTimeout (0, 50);

    if (!ttyPort->open(QIODevice::ReadWrite)) {
	QString errorStr = "[firmware main port] ";
	errorStr.append (portName);
	errorStr.append (ttyPort->errorString ());
	delete ttyPort;
	ttyPort = 0;
	commPort = NONE;
	actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	statusBar()->showMessage(errorStr, 5000);
	return;
    }
    else
	actionTtyQuery->setIcon(QIcon(":icons/fast.png"));

    bootloaderLoad (fileName, true);
}

/*-------------------------------------------------------------------------------------------------
 * firmware_debug_port --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::firmware_debug_port ()
{
    char cmd[32];
    QByteArray ba;

    QString fileName = QFileDialog::getOpenFileName(this, "read firmware", "", "firmware (*.enc)");
    if (fileName.isNull())
        return;

    sprintf (cmd, "reset x\n");				// undocumented Enter Bootloader
    ttyPort->write (cmd, strlen (cmd));
# ifdef _WIN32
    Sleep (20);
# endif
    ba = ttyPort->readAll();
    if (!(ba.contains ("boot loader"))) {
        QMessageBox::warning(this, tr("Conscriptor"), tr("Failed to enter boot loader"));
        return;
    }

    ttyPort->setTimeout (0, 50);
# ifdef _WIN32
    bootloaderLoad (fileName, true);
# else
    bootloaderLoad (fileName, false);
# endif
}

/*-------------------------------------------------------------------------------------------------
 * serial.download
 * on_actionFirmware_repair --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionFirmware_repair ()
{
    QByteArray ba;
    QString fileName;

    int r = QMessageBox::warning(this,
		tr("Conscriptor"),
		tr("<p>If the BOB is not responding it has probably entered bootloader mode and "
		   "can be fixed by <font color='#f00000'>Repair Firmware</font>\n"
		   "Turn the BOB off\n"
		   "Proceed and Select the firmware file "
		   "(<font color='#0000f0'>http://decadenet.com)</font></p>\n"
		   "Turn ON the BOB. If valid bootloader welcome messages are received then the "
		   "firmware will be loaded"),
		QMessageBox::Ok | QMessageBox::Cancel);
    if (r == QMessageBox::Ok) {
	fileName = QFileDialog::getOpenFileName(this, "select Firmware File", "", "fonts (*.enc)");
	if (fileName.isNull())
	    return;
    }
    else if (r == QMessageBox::Cancel)
	return;

    /*---------------------------------------------------------------------------------------------
     * BOB wont respond, as long as A port device is right we will prevail
     *-------------------------------------------------------------------------------------------*/
    if (ttyPort)
        delete ttyPort;

    if ((debug & TRACE1))
        qDebug() << "[firmware repair]" << portName;

    ttyPort = new QextSerialPort();
    ttyPort->setPortName(portName);
    ttyPort->setParity(PAR_NONE);
    ttyPort->setBaudRate(BAUD115200);
    ttyPort->setFlowControl(FLOW_XONXOFF);
    ttyPort->setDataBits(DATA_8);
    ttyPort->setStopBits(STOP_1);
    ttyPort->setTimeout (0, 100);

    if (!ttyPort->open(QIODevice::ReadWrite)) {
	QString errorStr = "[firmware repair] ";
	errorStr.append (portName);
	errorStr.append (ttyPort->errorString ());
	delete ttyPort;
	ttyPort = 0;
	commPort = NONE;
	actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	statusBar()->showMessage(errorStr, 5000);
	return;
    }
    else
	actionTtyQuery->setIcon(QIcon(":icons/fast.png"));

    /*---------------------------------------------------------------------------------------------
     * Getting here means the user powered ON the bob, pressed OK and bootloader is active
     * Wait 1.5 sec for bootloader to answer
     *-------------------------------------------------------------------------------------------*/
     ttyPort->write ("V#", 2);
     int tries;
     for (tries = 0; tries < 15; tries++) {
#     ifdef _WIN32
	Sleep (100);
#     endif  
	ba = ttyPort->readAll ();
	if ((ba.contains (">")))
	    break;
    }
    if (tries == 15) {
	statusBar()->showMessage(tr("XBOB bootloader not responding"), 10000);
	return;
    }

    bootloaderLoad (fileName, true);
}

/*=================================================================================================
 * onBootscript_finished --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::onBootscript_finished ()
{
    bootscriptDialogShowing = false;
}

/*-------------------------------------------------------------------------------------------------
 * onConfig_finished --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::onConfig_finished ()
{
    configDialogShowing = false;
}

/*-------------------------------------------------------------------------------------------------
 * onSetup_finished --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::onSetup_finished ()
{
    setupDialogShowing = false;
}

/*=================================================================================================
 * BOOTSCRIPT CONFIG SETUP
 * on_actionBSE --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::On_actionBSE ()
{
    if (bootscriptDialogShowing == true)
	return;
    bootscriptDialogShowing = true;

    BootscriptDialog *dialog = new BootscriptDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(accepted()), SLOT(onBootscript_finished()));

    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionCE --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::On_actionCE ()
{
    if (configDialogShowing == true)
	return;
    configDialogShowing = true;

    ConfigDialog *dialog = new ConfigDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(accepted()), SLOT(onConfig_finished()));

    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionSE --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::On_actionSE ()
{
    if (setupDialogShowing == true)
	return;
    setupDialogShowing = true;

    SetupDialog *dialog = new SetupDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, SIGNAL(accepted()), SLOT(onSetup_finished()));

    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionHelpAbout --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::On_actionHelpAbout ()
{
   QString ver = tr ("Conscriptor\n");
   ver.append(tr("Version: %1\nGIT: ").arg (revision));
   ver.append(gitrev);
   ver.append(tr("\nCopyright Decade Engineering"));
   QMessageBox::about(this, tr ("About Conscriptor"), ver);
}

/*-------------------------------------------------------------------------------------------------
 * on_actionHelpHelp --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::On_actionHelpHelp ()
{
    HelpDialog *dialog = new HelpDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionTtyQuery --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::on_actionTtyQuery ()
{
    setTtyPort ();
    if (commPort == NONE)
	setTtyDebug ();
}

/*-------------------------------------------------------------------------------------------------
 * openFile --
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::openFile (const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr ("Application"),
                             tr ("Cannot read file %1 %2.")
                             .arg(fileName)
                             .arg(file.errorString()));
        return;
    }

    setCurrentFile(fileName);
    //statusBar()->showMessage(tr("File loaded"), 2000);
}

/*-------------------------------------------------------------------------------------------------
 * strippedName --
 *-----------------------------------------------------------------------------------------------*/
QString Conscriptor::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

/*-------------------------------------------------------------------------------------------------
 * baseName --
 *-----------------------------------------------------------------------------------------------*/
QString Conscriptor::baseName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).baseName();
}

/*-------------------------------------------------------------------------------------------------
 * setTtyDebug -- setup the debug serial port
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setTtyDebug()
{
    if (ttyPort)
        delete ttyPort;

    if ((debug & TRACE1))
        qDebug() << "[setTtyDebug]" << portName;

    ttyPort = new QextSerialPort();

    ttyPort->setPortName(portName);
    ttyPort->setParity(PAR_NONE);
    ttyPort->setBaudRate(BAUD115200);
    ttyPort->setFlowControl(FLOW_XONXOFF);
    ttyPort->setDataBits(DATA_8);
    ttyPort->setStopBits(STOP_1);
    ttyPort->setTimeout (0, 100);

    if (!ttyPort->open(QIODevice::ReadWrite)) {
	QString errorStr = "[setTtyPort] ";
	errorStr.append (portName);
	errorStr.append (ttyPort->errorString ());
	delete ttyPort;
	ttyPort = 0;
	commPort = NONE;
	actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	statusBar()->showMessage(errorStr, 3000);
	QApplication::processEvents();
//#     ifdef _WIN32
//	Sleep (3000);
//#     else
//	usleep (3000000);
//#     endif
    }
    else {
	ttyPort->putChar ('\n');
#     ifdef _WIN32
	Sleep (100);
#     endif
	QByteArray ba = ttyPort->readAll();
	if ((debug & TRACE5))
	    qDebug () << "[read] debug" << ba;
	if ((ba.contains ('>'))) {
	    commPort = DEBUGP;
	    actionTtyQuery->setIcon(QIcon(":icons/bug.png"));
	}
	else {
	    delete ttyPort;
	    ttyPort = 0;
	    commPort = NONE;
	    actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	}
    }
    //ttyPort->setTimeout (0, 500);
}

/*-------------------------------------------------------------------------------------------------
 * setTtyPort -- setup the serial port
 *-----------------------------------------------------------------------------------------------*/
void Conscriptor::setTtyPort()
{
char cmd[16];

    if (ttyPort)
        delete ttyPort;

    if ((debug & TRACE1))
        qDebug() << "[setTtyPort]" << portName;

    ttyPort = new QextSerialPort();

    ttyPort->setPortName(portName);
    ttyPort->setParity(parity);
    ttyPort->setBaudRate(baudRate);
    ttyPort->setFlowControl(flowctrl);
    ttyPort->setDataBits(dataBits);
    ttyPort->setStopBits(stopBits);
    ttyPort->setTimeout (0, 100);

    if (!ttyPort->open(QIODevice::ReadWrite)) {
	QString errorStr = "[setTtyPort] ";
	errorStr.append (portName);
	errorStr.append (ttyPort->errorString ());
	delete ttyPort;
	ttyPort = 0;
	commPort = NONE;
	actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	statusBar()->showMessage(errorStr, 3000);
	QApplication::processEvents();
//#     ifdef _WIN32
//	Sleep (3000);
//#     else
//	usleep (3000000);
//#     endif
    }
    else {
	sprintf (cmd, "\x1b" "[1}");
	ttyPort->write (cmd, 4);
#     ifdef _WIN32
	Sleep (100);
#     endif
	QByteArray ba = ttyPort->readAll();
	if ((debug & TRACE5))
	    qDebug () << "[read] port" << ba;
	if ((ba.contains ("v4"))) {
	    commPort = XBOBP;
	    actionTtyQuery->setIcon(QIcon(":icons/on.png"));
	}
	else {
	    delete ttyPort;
	    ttyPort = 0;
	    commPort = NONE;
	    actionTtyQuery->setIcon(QIcon(":icons/off.png"));
	}
    }
    //ttyPort->setTimeout (0, 500);
}

/*-------------------------------------------------------------------------------------------------
 * bob4Send85 --
 *	embedded strings are <esc>Q...<esc>R so download strings are <esc>X ... <esc>\
 *-----------------------------------------------------------------------------------------------*/
bool Conscriptor::bob4Send85 (int flashDevice, quint32 destinationAddr, quint8 *data, int len, int debugPort)
{
    QByteArray answer;
    char buf[256 * 5 / 4 + 16];	// Big enough to encode 256 bytes
    char *bp = &buf[0];
    quint32 cksum = 0;
    quint8 *dp = data;

    while (len > 0) {
	quint32 n = 0;
	if (len >= 1)
	    n = dp[0] << 24;
	if (len >= 2)
	    n |= dp[1] << 16;
	if (len >= 3)
	    n |= dp[2] << 8;
	if (len >= 4) {
	    n |= dp[3] << 0;
	    if (n == 0) {
		*bp++ = 'z';
		cksum += 0xbabeface;
		cksum <<= 1;
		goto skip;
	    }
	}

	cksum += 0xbabeface;
	cksum <<= 1;
	cksum += n;

	bp[4] = (n % 85) + '!';
	n /= 85;
	bp[3] = (n % 85) + '!';
	n /= 85;
	bp[2] = (n % 85) + '!';
	n /= 85;
	bp[1] = (n % 85) + '!';
	n /= 85;
	bp[0] = (n % 85) + '!';

	if (len > 4)
	    bp += 5;
	else
	    bp += len + 1;
skip:
	len -= 4;
	dp += 4;
    }

    *bp = '\0';
    cksum &= 0x7fffffff;

    len = bp - &buf[0];
    bp = &buf[0];

    /*---------------------------------------------------------------------------------------------
     * Main Port
     *-------------------------------------------------------------------------------------------*/
    if (!debugPort) {
	char cmd[32];

	ttyPort->write("\x1b" "X", 2);		// Start string
	ttyPort->write(bp, len);		// ASCII-85 data.
	ttyPort->write("\x1b" "\\", 2);		// End string

	sprintf(cmd, "\x1b" "[91;%d;%d;%d|", flashDevice, destinationAddr, cksum);
	ttyPort->write(cmd, strlen (cmd));

	for (int i = 0; i < 20; i++) {
	    if (ttyPort->bytesAvailable() > 0)
		break;
	    QApplication::processEvents();
#	  ifdef _WIN32
	    Sleep (40);
#	  else
	    usleep (40000);
#	  endif
        }

	answer = ttyPort->readAll ();
	if (!(answer.contains ('\x6')))		// Not ACK also isEmpty
	    return false;
    }
    /*---------------------------------------------------------------------------------------------
     * Debug port so use packets that are smaller
     *-------------------------------------------------------------------------------------------*/
    else {		
	while (len > 0) {
	    int ll = len;
	    if (ll > 56)
		ll = 56;

	    ttyPort->write("str ", 4);
	    ttyPort->write(bp, ll);
	    ttyPort->putChar('\n');

	    bp += ll;
	    len -= ll;

	    for (int tries = 0; tries < 10; tries++) {
#	      ifdef _WIN32
		Sleep (40);
#	      else
		usleep (40000);
#	      endif

		answer = ttyPort->readAll();
		//qDebug () << "[" << tries << "answer]" << answer;
		if (answer.contains ("bob4>")) {
		    //qDebug ("[ACK] n break");
		    break;
		}
	    }
	    if (!(answer.contains ("bob4>"))) {
		//qDebug ("[return] false");
		return false;
	    }
	}

	char cmd[32];
	sprintf(cmd, "memory write %d %d %d\n", flashDevice, destinationAddr, cksum);
	ttyPort->write(cmd, strlen (cmd));

	for (int tries = 0; tries < 10; tries++) {
#	   ifdef _WIN32
	    Sleep (100);
#          else
	    usleep (100000);
#	   endif
	    answer = ttyPort->readAll();
	    if (answer.contains ("bob4>"))
		break;
	}
	if (!(answer.contains ("bob4>")))
	    return false;
    }
    return true;
}

/*-------------------------------------------------------------------------------------------------
 * writeXModem --
 *-----------------------------------------------------------------------------------------------*/
unsigned short crcData[256]={
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0};

#define crc16(ch, crc)  (crcData[((crc >> 8) & 255)] ^ (crc << 8) ^ ch)

bool Conscriptor::writeXModem (int sector, char *data)
{
    int attempts;
    QByteArray ba;

    if (data == 0) {
	ttyPort->putChar (EOT);
#     ifdef _WIN32
	Sleep (100);
#     endif
	ba = ttyPort->readAll ();
	return true;
    }

    for (attempts = 0; attempts <= 20; attempts++) {
	unsigned int crc = 0;
	char *dp;
	int i;
	char buf[140];

	buf[0] = SOH;
	buf[1] = sector;
	buf[2] = ~sector;

	dp = data;
	for (i = 0; i < 128; i++) {
	    unsigned int x = *dp++ & 0xff;
	    buf[i+3] = x;
	    crc = crc16(x, crc);
	}

	crc = crc16(0, crc16(0, crc));
	buf[3+128] = crc >> 8;
	buf[4+128] = crc;

	for (int indx = 0; indx < (5+128); indx++)
	    ttyPort->putChar (buf[indx]);

#     ifdef _WIN32
	Sleep (40);
#     else
	usleep (40000);
#     endif

#     ifdef DBGFIX
	char a;
	ttyPort->getChar (&a);
	if (a == ACK)
#     else
	ba = ttyPort->readAll ();
	if (ba.contains (ACK))
#     endif
	    return true;
    }
    return false;               // Too many retries.
}

