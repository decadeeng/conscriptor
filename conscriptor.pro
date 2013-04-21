TEMPLATE = subdirs

CONFIG += ordered

SUBDIRS = serial conscriptor

win32:DEFINES += QT_DLL
win32:DEFINES += _TTY_WIN_
