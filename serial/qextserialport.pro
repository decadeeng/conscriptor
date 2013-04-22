TEMPLATE		= lib
CONFIG			= warn_on qt thread
TARGET                  = qextserialport

unix:OBJECTS_DIR	= unix_obj
unix:MOC_DIR		= unix_moc
unix:DESTDIR		= lib

win32:MOC_DIR		= win_moc

win32-g++: {
    win32:OBJECTS_DIR   = mingw_obj
    win32:LIBS    	+= "C:\\Program Files\\mingw\\lib\\libnetapi32.a"
    win32:DESTDIR	= mingw
    win32:CONFIG        += dll
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
win32:DEFINES            = _TTY_WIN_

