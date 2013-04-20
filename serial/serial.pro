TEMPLATE		= lib
CONFIG			= warn_on qt thread staticlib
TARGET			= qextserialport

unix:OBJECTS_DIR	= unix_obj
unix:MOC_DIR		= unix_moc
unix:DESTDIR		= lib

win32:MOC_DIR		= win_moc
win32-borland: {
    win32:OBJECTS_DIR   = bc_obj
    win32:LIBS	+= Vcl50.lib
    win32:DESTDIR	= bclib
}

win32-msvc: {
    win32:OBJECTS_DIR   = vc_obj
    win32:LIBS	+= Netapi32.lib
    win32:DESTDIR	= vclib
}

win32-g++: {
    win32:OBJECTS_DIR   = mingw_obj
    win32:LIBS	+= Netapi32.lib
    win32:DESTDIR	= mingw
}

HEADERS                 = qextserialbase.h \
                          qextserialport.h 
SOURCES                 = qextserialbase.cpp \
                          qextserialport.cpp 

unix:HEADERS           += posix_qextserialport.h
unix:SOURCES           += posix_qextserialport.cpp
unix:DEFINES            = _TTY_POSIX_
 
win32:HEADERS          += win_qextserialport.h
win32:SOURCES          += win_qextserialport.cpp

win32:DEFINES += QT_DLL
win32:DEFINES += _TTY_WIN_
