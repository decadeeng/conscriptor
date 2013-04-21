#include <QApplication>
#include <QTranslator>
#include <QTextCodec>
#include "conscriptor.h"
#include "conf.h"
#include <QtDebug>
#include <QFile>
#include <QTextStream>
     
int debug = 0;

#ifdef Q_WS_MAC
extern void qt_set_sequence_auto_mnemonic(bool b);
#endif

/*-------------------------------------------------------------------------------------------------
 * Qt message handler
 *	the log file is opened every time it is called, so Append is the only option
 *-----------------------------------------------------------------------------------------------*/
void myMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    switch (type) {
	case QtDebugMsg:
	txt = QString("Debug: %1").arg(msg);
	break;
    case QtWarningMsg:
	txt = QString("Warning: %1").arg(msg);
	break;
    case QtCriticalMsg:
	txt = QString("Critical: %1").arg(msg);
	break;
    case QtFatalMsg:
	txt = QString("Fatal: %1").arg(msg);
	abort();
    }
    QFile outFile("log");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}

/*-------------------------------------------------------------------------------------------------
 * Main --
 *-----------------------------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    debug = 0;
    for (int argpos = 1; argpos < argc; ++argpos) {
        QString arg(argv[argpos]);
        if (arg == "-v") {
	    //unlink ("log");
	    QFile::remove("log"); 
            QString whatarg(argv[argpos+1]);
            if (whatarg.contains("0", Qt::CaseInsensitive))
                debug |= TRACE0;
            if (whatarg.contains("1", Qt::CaseInsensitive))
                debug |= TRACE1;
            if (whatarg.contains("2", Qt::CaseInsensitive))
                debug |= TRACE2;
            if (whatarg.contains("3", Qt::CaseInsensitive))
                debug |= TRACE3;
            if (whatarg.contains("4", Qt::CaseInsensitive))
                debug |= TRACE4;
            if (whatarg.contains("5", Qt::CaseInsensitive))
                debug |= TRACE5;
            if (whatarg.contains("6", Qt::CaseInsensitive))
                debug |= TRACE6;
            if (whatarg.contains("7", Qt::CaseInsensitive))
                debug |= TRACE7;
            if (whatarg.contains("8", Qt::CaseInsensitive))
                debug |= TRACE8;
            if (whatarg.contains("9", Qt::CaseInsensitive))
                debug |= TRACE9;
            if (whatarg.contains("a", Qt::CaseInsensitive))
                debug |= TRACEA;
            if (whatarg.contains("b", Qt::CaseInsensitive))
                debug |= TRACEB;
            if (whatarg.contains("c", Qt::CaseInsensitive))
                debug |= TRACEC;
            if (whatarg.contains("d", Qt::CaseInsensitive))
                debug |= TRACED;
            if (whatarg.contains("e", Qt::CaseInsensitive))
                debug |= TRACEE;
            if (whatarg.contains("f", Qt::CaseInsensitive))
                debug |= TRACEF;
            if (whatarg.contains("g", Qt::CaseInsensitive))
                debug |= TRACEG;
        }
        else if ((arg == "-h") || (arg == "--help")) {
            qDebug ("Usage: [-v [0..f [-h] [--help]");
	    qDebug ("0 -");
	    qDebug ("1 -");
	    qDebug ("2 -");
	    qDebug ("3 -");
	    qDebug ("4 -");
	    qDebug ("5 -");
	    qDebug ("6 -");
	    qDebug ("7 -");
	    qDebug ("8 -");
	    qDebug ("9 -");
	    qDebug ("a -");
	    qDebug ("b - import BDF parse detail");
	    qDebug ("c - pix2asc detail");
	    qDebug ("d - resetXY detail; show screen co-ordinates of each glyph");
	    qDebug ("e - show memory activity");
	    qDebug ("f - log to file \"log\"");
	    qDebug ("g - image import");
            exit (0);
        }
    }

#  ifdef Q_WS_MAC
    qt_set_sequence_auto_mnemonic(true);
#  endif

    QString locale = QLocale::system().name();
    if ((debug & TRACE1))
	qDebug () << "[locale]" << locale;

    QApplication app(argc, argv);
    if (debug & TRACEF)
	qInstallMsgHandler(myMessageHandler);
    Conscriptor window;
    QTranslator translator;
    translator.load("/usr/local/share/conscriptor_" + locale);
    app.installTranslator(&translator);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    window.show();
    return app.exec();
}
