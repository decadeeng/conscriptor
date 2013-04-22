#ifndef CONF_H
#define CONF_H

#define TTY 1
#define MAXGLYPHS 5000		// max you may have
#define NEWGLYPHS 2000		// max created with new

#define MACROX 30		// MacroGlyph offsets
#define MACROY 5
#define MACROS 75

#define VERSIONNO 1		// Conscriptor file version no

// ------- These probably don't need to be changed
#define SOH 1
#define EOT 4
#define ACK 6
#define NAK 15

#ifdef _WIN32
#define HELPPATH "doc\\index.html"
#else
#define HELPPATH "/usr/share/conscriptor/doc/index.html"
#endif

#define TRACE0  0x00000001
#define TRACE1  0x00000002
#define TRACE2  0x00000004
#define TRACE3  0x00000008
#define TRACE4  0x00000010
#define TRACE5  0x00000020
#define TRACE6  0x00000040
#define TRACE7  0x00000080
#define TRACE8  0x00000100
#define TRACE9  0x00000200
#define TRACEA  0x00000400
#define TRACEB  0x00000800
#define TRACEC  0x00001000
#define TRACED  0x00002000
#define TRACEE  0x00004000
#define TRACEF  0x00008000
#define TRACEG  0x00010000
#endif
