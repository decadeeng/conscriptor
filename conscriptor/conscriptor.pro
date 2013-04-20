TEMPLATE = app
TARGET = conscriptor
SUBDIRS = serial

DEPENDPATH += .
INCLUDEPATH += ../serial

# CONFIG += release
CONFIG += debug

# To get tty qDebug messages
# CONFIG += console

# Input
HEADERS += conf.h \
	   bootscript.h \
           config.h \
           conscriptor.h \
           font.h \
           help.h \
           importbdf.h \
           importimage.h \
           newfile.h \
           setup.h \
           setmem.h \
           fontname.h \
	   glyph.h \
	   codeval.h \
	   print.h \
	   terminal.h \
	   baudselect.h \
	   portselect.h \
	   ../serial/qextserialport.h

FORMS += bootscript.ui \
         config.ui \
         help.ui \
         importbdf.ui \
         importimage.ui \
         newfile.ui \
         setup.ui \
         setmem.ui \
         codeval.ui \
         print.ui \
         terminal.ui \
         baudselect.ui \
         portselect.ui \
	 fontname.ui

SOURCES += bootscript.cpp \
           config.cpp \
           conscriptor.cpp \
           font.cpp \
           help.cpp \
           importbdf.cpp \
           importimage.cpp \
           main.cpp \
           newfile.cpp \
           setup.cpp \
           setmem.cpp \
           fontname.cpp \
           codeval.cpp \
           print.cpp \
           terminal.cpp \
           baudselect.cpp \
           portselect.cpp \
	   glyph.cpp

RESOURCES = conscriptor.qrc
TRANSLATIONS  = conscriptor_ru_RU.ts
CODECFORTR = UTF-8


win32:DEFINES += QT_DLL
win32:DEFINES += _TTY_WIN_

unix: {
  UI_DIR = ui
  MOC_DIR = moc
  OBJECTS_DIR = obj

  DEFINES    += _TTY_POSIX_
  LIBS        += -L../serial/lib -lqextserialport
}

macx: {
  QMAKE_EXTRA_TARGETS += vertarget
  PRE_TARGETDEPS += version.h
        
  vertarget.target = version.h
  vertarget.commands = echo \"const char *gitrev = \\\"`git log |grep commit |sed s/commit\ // |head -c5`\\\";\" > $$vertarget.target;
  vertarget.commands += echo \"const int revision = `git log |grep commit |wc -l`;\" >> $$vertarget.target;
  vertarget.depends = FORCE
}

win32-g++: {
    win32:OBJECTS_DIR   = mingw_obj
#    win32:LIBS  += serial\mingw\qextserialport.a
    win32:LIBS  += -L../serial/mingw -lqextserialport
}

