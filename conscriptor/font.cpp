#include <QtGui>

#include "font.h"
#include "glyph.h"
#include "codeval.h"
#include "importbdf.h"
#include "importimage.h"
#include "conscriptor.h"

extern int debug;

/*-------------------------------------------------------------------------------------------------
 * header -- is kept up to date after file read or new
 *	name
 *	    fontname
 *	pixWidth
 *	    insert char
 *	    insert column
 *	    delete char
 *	    delete column
 *	    1<->2BPP
 *	    paste
 *	chCount
 *	    insert char
 *	    delete char
 *	chHeight
 *	    insert row
 *	    delete row
 *	chDepth
 *	    1BPPto2BPP
 *	    2BPPto1BPP
 *
 * pixmapCount
 *	set with pixWidth
 *
 * metricCount
*	insert char
*	delete char
*	setCodePoint
		when chCount is changed ??
 *-----------------------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------------------------
 * constructor --
 *-----------------------------------------------------------------------------------------------*/
B4Font::B4Font(QWidget *parent)
    : QWidget(parent)
{

    if ((debug))
	qDebug ("[font]        b4font constructor-----");

    conscriptor = qobject_cast<Conscriptor*>(parent);
    nGlyphs = 0;
    firstGlyph = 0;
    lineLength = 0;
    macroMagnification = 3;
    amBase = 0;
    colorFlag = opaqueCol;
    thePicture = new QImage ();
}

/*-------------------------------------------------------------------------------------------------
 * cleanup --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::cleanup()
{

    if ((debug))
	qDebug ("[font]        cleanup-----------");

    if ((debug & TRACEE))
	qDebug ("       free %d glyphs", nGlyphs);

    for (int i = 0; i < nGlyphs; i++)
	delete allglyphs[i];
    //delete allglyphs[];
    nGlyphs = 0;
}

/*-------------------------------------------------------------------------------------------------
 * testG --
 *	print a char ch from am where am is amBase[char] + pixWidth offsets
 *-----------------------------------------------------------------------------------------------*/
void B4Font::testG (int ch)
{
char str[32];

    if ((debug))
	qDebug ("[font]        testG-------------");

    int cnt = conscriptor->metrixBase[ch].width;
    for (int r = 0; r < conscriptor->header.chHeight; r++) {
	int indx = conscriptor->metrixBase[ch].x + r * conscriptor->header.pixWidth * 8;
	memcpy (str, (char *)&amBase [indx], cnt);
	str[cnt + 1] = 0;
	//qDebug ("       %4d %s", indx, str);
    }
}

/*-------------------------------------------------------------------------------------------------
 * makePixmap --
 *	asciiMap is up to date
 *	header is corect
 *	pixmap is strange format: see doco on pixmap format
 *-----------------------------------------------------------------------------------------------*/
void B4Font::makePixmap ()
{
char bitmask[]    = {0x01, 0x02, 0x04 , 0x08, 0x10, 0x20, 0x40, 0x80};
char opaquemask[] = {0x02, 0, 0x08 , 0, 0x20, 0, 0x80, 0};
char cthrumask[]  = {0x01, 0, 0x04 , 0, 0x10, 0, 0x40, 0};
char pmdata;
int pixmapIndx = 0;
int bitCnt = 0;
int amIndx;
int set;
int height = conscriptor->header.chHeight;
int pixWidth = conscriptor->header.pixWidth * 8;
int depth = conscriptor->header.chDepth;

    if ((debug))
	qDebug ("[font]        makePixmap--------pixmapWidth %d pixmap-> is %lx",
					    conscriptor->pixmapCount, (long)conscriptor->pixmap);

    /*---------------------------------------------------------------------------------------------
     * Create data areas.
     * old areas are freed rather than tracking size and reusing
     * CALLOC then FREE to avoid thread timing issues
     *-------------------------------------------------------------------------------------------*/
    setPixmapCount ();
    char *cp = (char *)conscriptor->pixmap;
    conscriptor->pixmap = (char *)calloc (conscriptor->pixmapCount, 1);
    if (debug & TRACEE)
	qDebug ("              calloc     pixmap 0x%lx (%d)", (long)conscriptor->pixmap,
							    conscriptor->pixmapCount);
    if ((cp)) {
	if (debug & TRACEE)
	    qDebug ("              free       pixmap 0x%lx", (long)cp);
	free (cp);
    }

    /*---------------------------------------------------------------------------------------------
     * Loop for all rows of all chars
     *	    if ever BPP is not 1 or 2 here be dragons
     *-------------------------------------------------------------------------------------------*/
    for (pmdata = set = 0; set < (pixWidth / 32); set++) {
	if (debug & TRACE2)
	    qDebug ("\t-------- slice #%d", set);

	/*-----------------------------------------------------------------------------------------
	 * Every slice of 32 bits of data for 1 or more chars for every row
	 *---------------------------------------------------------------------------------------*/
	for (int row = 0; row < height; row++) {
	    amIndx = set * (((conscriptor->header.chDepth)) ? 16 : 32) + row * pixWidth;
	    if (debug & TRACE2)
		qDebug ("\t-------- row #%d", row);

	    /*-------------------------------------------------------------------------------------
	     * each of 4 bytes in this slice
	     *-----------------------------------------------------------------------------------*/
	    for (int byteCnt = 0; byteCnt < 4; byteCnt++) {

		/*---------------------------------------------------------------------------------
		 * bit 0 .. 31 of the slice
		 * each bit of the byte
		 *-------------------------------------------------------------------------------*/
		for (bitCnt = 0; bitCnt < 8; bitCnt++, amIndx++) {
		    if (debug & TRACE2)
			qDebug ("\tamBase[%5d] %c  bit#%d", amIndx, amBase[amIndx], bitCnt);

		    if (depth == 0) {				// 1 BPP
			if (amBase[amIndx] == '#') {
			    if (debug & TRACE2)
				qDebug ("pmdata:%02x |= bitmask[%d] %02x", pmdata & 0xff, bitCnt,
									bitmask[bitCnt] & 0xff);
			    pmdata |= bitmask[bitCnt];
			}
		    }
		    else {					// 2 BPP
			if (amBase[amIndx] == '#') {		// all this fiddling to skip 2 bits
			    if (debug & TRACE2)
				qDebug ("pmdata:%02x |= opaquemask[%d] %02x", pmdata & 0xff, bitCnt,
									opaquemask[bitCnt] & 0xff);
			    pmdata |= opaquemask[bitCnt++];
			}
			else if (amBase[amIndx] == '-') {
			    if (debug & TRACE2)
				qDebug ("pmdata:%02x |= cthrumask[%d] %02x", pmdata & 0xff, bitCnt,
									cthrumask[bitCnt] & 0xff);
			    pmdata |= cthrumask[bitCnt++];
			}
			else {
			    bitCnt++;
			}
		    }
		}
		if (debug & TRACE2)
		    qDebug ("\tpixmap[%d] 0x%02x (byte #%d)", pixmapIndx, pmdata & 0xff, byteCnt);

		conscriptor->pixmap[pixmapIndx++] = pmdata;
		bitCnt = 0;
		pmdata = 0;
	    }
	}
    }
    if ((debug))
	qDebug ("              /makePixmap--------");
}

/*-------------------------------------------------------------------------------------------------
 * makeAsciiMap --
 *	header is up to date
 *		rebuild metrics 
 *		Save ascii version of the glyphs
 *		'real' chars have a width
 *		It is dumb but you can have firstChar (and subsequent chars) as not real chars
 *		the last char is a glyph hence must be real
 *		TRACEE, TRACE3, TRACE4 (detail)
 *-----------------------------------------------------------------------------------------------*/
void B4Font::makeAsciiMap ()
{
Conscriptor::metrixtag *mp;
Glyph *gp;
char *cp;
int offset;

char str[128];

    if ((debug & (TRACE3 | TRACE4)))
	qDebug ("[font]        makeAsciiMap------");

    /*---------------------------------------------------------------------------------------------
     * Create data areas.
     * old areas are freed rather than tracking size and reusing
     * CALLOC then FREE to avoid thread timing issues
     *-------------------------------------------------------------------------------------------*/
    cp = amBase;
    amBase = (char *)calloc (conscriptor->header.pixWidth * 8, conscriptor->header.chHeight);
    if (debug & TRACEE)
	qDebug ("         calloc     amBase 0x%lx (%d)", (long)amBase,
				conscriptor->header.pixWidth * 8 * conscriptor->header.chHeight);
    if ((cp)) {
	if (debug & TRACEE)
	    qDebug ("         free       amBase 0x%lx", (long)cp);
	free (cp);
    }

    setMetricCount ();			// we need this size next
    cp = (char *)conscriptor->metrixBase;
    conscriptor->metrixBase = (Conscriptor::metrixtag *)calloc (conscriptor->metricCount, 1);
    if (debug & TRACEE)
	qDebug ("         calloc metrixBase 0x%lx (%d)", (long)conscriptor->metrixBase,
							    conscriptor->metricCount);
    if ((cp)) {
	if (debug & TRACEE)
	    qDebug ("              free   metrixBase 0x%lx", (long)cp);
	free (cp);
    }

    QRgb opaquePix = conscriptor->opaqueColor.rgb ();
    QRgb cthruPix = conscriptor->cthruColor.rgb ();

    mp = (Conscriptor::metrixtag *)conscriptor->metrixBase;

    /*---------------------------------------------------------------------------------------------
     * calloc zeros so all metrics for not-real chars are zeroed ie chars of zero width hence 
     * ithey have no glyph
     * all -1 chars are sorted to the end
     *-------------------------------------------------------------------------------------------*/
    int inc = conscriptor->header.pixWidth * 8;
    int x = 0;
    int chno;
    int chmax = 0;
    int m1c = 0;
    for (int i = 0; i < nGlyphs; i++) {
	gp = allglyphs[i];
	chno = gp->getCharNo ();
	chmax = qMax (chno, chmax);
	if (chno < 0)
	    chno = chmax + ++m1c;

	mp[chno].width = gp->getPixWidth ();
	mp[chno].x = x;

	if (debug & TRACE4)
	    qDebug ("              metrics+pixmap Glyph #%d Ch:%d W:%d X:%d", i, chno,
									    mp[chno].width, x);

	offset = x;
	for (int row = 0; row < conscriptor->header.chHeight; row++, offset += inc) {

	    if (debug & TRACE4)
		qDebug ("---- row #%2d", row);

	    for (int col = 0; col < gp->getPixWidth(); col++) {
		QRgb result = allglyphs[i]->getPixel (col, row);
		if (result == opaquePix) {

		    if (debug & TRACE4)
			qDebug ("     col #%2d amBase[%d] = #", col, col + offset);

		    amBase[offset + col] = '#';
		}
		else if (result == cthruPix) {

		    if (debug & TRACE4)
			qDebug ("     col #%2d amBase[%d] = -", col, col + offset);

		    amBase[offset + col] = '-';
		}
		else
		    amBase[offset + col] = '.';
	    }
	}
	x += mp[chno].width;
    }
   
    /*---------------------------------------------------------------------------------------------
     * Last metric
     *-------------------------------------------------------------------------------------------*/
     //qDebug ("([x]%d [lastChar]%d",  x, allglyphs[nGlyphs-1]->getCharNo());
     mp[allglyphs[nGlyphs-1]->getCharNo() +1].x = x;

    if (debug & TRACE3) {
	int wif = inc >  80 ? 80 : inc;
	qDebug ("wif:%d inc:%d", wif, inc);
	for (int row = 0; row < conscriptor->header.chHeight; row++) {
	    memcpy (str, (char *)&amBase[row * inc], wif);
	    str[wif] = 0;
	    qDebug ("[%2d] [%s]", row, str);
	}
    }

    /*---------------------------------------------------------------------------------------------
     * Zero width chars (seem to) need an a offset
     *-------------------------------------------------------------------------------------------*/
    mp = (Conscriptor::metrixtag *)conscriptor->metrixBase;
    offset = 0;
    int last = allglyphs[nGlyphs -1]->getCharNo();
    for (int i = 0; i < last; i++) {			// spec: last metric is ALL 0 so < NOT <=
	if (mp[i].width == 0)
	    mp[i].x = offset;
	else 
	    offset = mp[i].x + mp[i].width;
    }

    if ((debug & (TRACE3 | TRACE4)))
	qDebug ("              /makeAsciiMap------");
}

/*-------------------------------------------------------------------------------------------------
 * resetColor --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::resetColor (QRgb was, QRgb is)
{
    if ((debug))
	qDebug ("[font]        resetColor--------");

    for (int glyphNo = 0; glyphNo < nGlyphs; glyphNo++)
	allglyphs[glyphNo]->changeColor (was, is);
}

/*-------------------------------------------------------------------------------------------------
 * resetXY -- set every glyph's x,y in screen co-ordinates
 *           lineLength is already set
 *-----------------------------------------------------------------------------------------------*/
void B4Font::resetXY ()
{
int glyphNo;
int r;
int p;
int x;

    if ((debug))
	qDebug ("[font]        resetXY [lineLength]%d", lineLength);

    if (nGlyphs == 0)
	return;

    /*---------------------------------------------------------------------------------------------
     * setup macro glyph
     *-------------------------------------------------------------------------------------------*/
    macroHeight = (allglyphs[macroGlyph]->getPixHeight() * viewportWidth * macroMagnification)
									/ lineLength;
    macroWidth = (allglyphs[macroGlyph]->getPixWidth() * viewportWidth * macroMagnification)
									/ lineLength;
    /*---------------------------------------------------------------------------------------------
     * Set the position of every glyph
     *      lineLength is now the greatest width
     *-------------------------------------------------------------------------------------------*/
    int rowHeight = ((conscriptor->header.chHeight + 2) * viewportWidth) / lineLength;
    for (x = 10, r = p = glyphNo = 0; glyphNo < nGlyphs; glyphNo++) {
	int y = qMax (230, MACROS + macroHeight) + r * rowHeight;

	//qDebug ("[font] resetXYrowHeight %d this row at %d", rowHeight, y);
	if (glyphNo < firstGlyph) {
	    if (debug & TRACE8)
		qDebug ("              don't show glyph %d < firstGlyph %d", glyphNo,
										    firstGlyph);
	    allglyphs[glyphNo]->visible = false;
	    continue;
	}
	if (debug & TRACE8)
	    qDebug ("              show glyph %d @%dx%d", glyphNo, x, y);

	allglyphs[glyphNo]->visible = true;
	allglyphs[glyphNo]->setXY (x, y, lineLength, viewportWidth);

	if (debug & TRACED)
	    qDebug ("        Glyph [%3d] set XY x:%-3d y:%-3d", glyphNo, x, y);

	if (++p == 16) {
	    p = 0;
	    x = 10;
	    ++r;
	}
	else {
	    x += ((allglyphs[glyphNo]->getPixWidth () + 2) * viewportWidth) / lineLength;
	}
    }
    conscriptor->setScrollSize();
    if ((debug))
	qDebug ("              /resetXY");

}

/*-------------------------------------------------------------------------------------------------
 * setNewGlyphs --
 *	called after new 
 *	CharNo is 0..n
 *	CodePoint is first..n+first
 *-----------------------------------------------------------------------------------------------*/
void B4Font::setNewGlyphs(int w, int h, int n)
{
    if ((debug))
	qDebug ("[font]        setNewGlyphs w:%d h:%d n:%d", w, h, n);

    for (int i = 0; i < n; i++) {
	allglyphs[i] = new Glyph (conscriptor);
	allglyphs[i]->newGlyph (w, h, i);
    }
    nGlyphs = n;
    macroGlyph = 0;
    findWidth ();
    
    if ((debug))
	qDebug ("              /setNewGlyphs");
}

/*-------------------------------------------------------------------------------------------------
 * setupGlyphs --
 *	called after file-read allocate glyphs
 *-----------------------------------------------------------------------------------------------*/
void B4Font::setupGlyphs()
{
int glyphNo;
int chno;
int ww;

    if ((debug))
	qDebug ("[font]        setupGlyphs");
    /*---------------------------------------------------------------------------------------------
     * Make a glyph for every char in the metrics
     *-------------------------------------------------------------------------------------------*/
    for (glyphNo = chno = 0; chno < conscriptor->header.chCount; chno++) {
	
	if ((ww = conscriptor->metrixBase[chno].width) == 0)
	    continue;

	allglyphs[glyphNo] = new Glyph (conscriptor);
	allglyphs[glyphNo]->newGlyph ((int)ww,
				     (int)conscriptor->header.chHeight,
				     (char *)&amBase[conscriptor->metrixBase[chno].x],
				     conscriptor->header.pixWidth * 8,
				     chno);
	if (debug & TRACE0)
	    qDebug ("              Glyph [%3d] w:%d h:%d chno:%d", glyphNo, ww,
						(int)conscriptor->header.chHeight, chno);
	if (++glyphNo > MAXGLYPHS) {
	    QMessageBox::warning(this,
		    tr("Conscriptor"),
		    tr("Maximum number of glyphs limited to %1").arg (MAXGLYPHS),
		    QMessageBox::Ok);
	    break;
	}
    }
    nGlyphs = glyphNo;			// nGlyphs 0..glyphNo-1
    firstGlyph = 0;
    macroGlyph = 0;

    findWidth ();
    
    if ((debug))
	qDebug ("              /setupGlyphs [nGlyphs]%d {lineWidth]%d", nGlyphs, lineLength);
}

/*-------------------------------------------------------------------------------------------------
 * Find the widest row in pixels
 *-----------------------------------------------------------------------------------------------*/
void B4Font::findWidth ()
{
int glyphNo;
int r;
int p;
int rowWidth;

    if ((debug))
	qDebug ("[font]        findWidth");

    for (lineLength = r = p = glyphNo = 0, rowWidth = 2; glyphNo < nGlyphs; glyphNo++) {
	rowWidth += (allglyphs[glyphNo]->getPixWidth () + 2);
	if (++p == 16) {
	     p = 0;
	     if (rowWidth > lineLength)
		lineLength = rowWidth;
	    rowWidth = 2;
	    ++r;
	}
    }
    if (lineLength == 0)			// less than a whole row of chars
	lineLength = (rowWidth * 16) / nGlyphs;

    if ((debug))
	qDebug ("              /findWidth [lineLength]%d", lineLength);
}

/*-------------------------------------------------------------------------------------------------
 * pix2asc --
 *	read header
 *	read metrics
 *	read pixmap
 *-----------------------------------------------------------------------------------------------*/
void B4Font::pix2asc()
{
char bitmask[] = {1, 2, 4 , 8, 16, 32, 64, 128};
int  from = 0;
char data;
int indx;
int row;
int fontBit;
int startChar;
int startBit;
int curChar;
int byteBit;                            // The bitno in the incoming data
int byteCnt;                            // Number of chars read in a set of 4

    if ((debug))
	qDebug ("[font]-pix2asc-----------");

    amBase = (char *)calloc (conscriptor->header.pixWidth * 8, conscriptor->header.chHeight);
    if (debug & TRACEE)
	qDebug ("         calloc     amBase 0x%lx (%d)", (long)amBase,
				conscriptor->header.pixWidth * 8 * conscriptor->header.chHeight);

    /*---------------------------------------------------------------------------------------------
     * Loop for all rows of all chars
     *-------------------------------------------------------------------------------------------*/
    for (curChar = fontBit = row = startChar = startBit = 0; ; ) {

        /*-----------------------------------------------------------------------------------------
         * Read 32 byte set
         * No Exit from this loop
         * Each set is one row of part of 1 char to many chars (32bits / character_width)
         * reset the start char for each row
         * fontBit is 0..width for each char 
         * If we reach no-more-rows here then reset the startChar;
         *---------------------------------------------------------------------------------------*/
        for (byteCnt = 0; byteCnt < 4; byteCnt++) {

            data = conscriptor->pixmap[from++];
	    if (debug & TRACE9)
		qDebug ("        read[%d] 0x%02x", byteCnt, data & 0xff);

            /*-------------------------------------------------------------------------------------
             * Every bit of the incoming byte
             * If fontBit reaches width in this loop then bump char
             *-----------------------------------------------------------------------------------*/
	    if (debug & TRACEC)
		qDebug ("        bit loop ------------------ Start");

            for (byteBit = 0; byteBit < 8; byteBit++, fontBit++) {

                /*---------------------------------------------------------------------------------
                 * fontBit at end
                 * start with next char
                 * start fontBit at beginning
                 *-------------------------------------------------------------------------------*/
                if (fontBit == conscriptor->metrixBase[curChar].width) {
                    fontBit = 0;

                    for (++curChar; ;++curChar) {                       // get next char
                        if (curChar == conscriptor->header.chCount)     // past the last so proceed
                            break;                                      // normally but DONT write
                        if ((conscriptor->metrixBase[curChar].width))   // if this char has width
                            break;                                      // then we found one
                    }
                }

                /*---------------------------------------------------------------------------------
                 * for valid font chars only; not past the last
                 *-------------------------------------------------------------------------------*/
                if (curChar < conscriptor->header.chCount) {
                    indx = conscriptor->metrixBase[curChar].x + fontBit +
							(row * conscriptor->header.pixWidth * 8);

                    if (conscriptor->header.chDepth == 0)                // 1 BPP
                        amBase[indx] = data & bitmask[byteBit] ? '-' : '.';

                    else {                                              // 2 BPP
                        if ((data & bitmask[byteBit++]))
                            amBase[indx] = '#';
                        else
                            amBase[indx] = '.';

                        if ((data & bitmask[byteBit]))
                            amBase[indx] = '-';
                    }

	    if (debug & TRACEC)
		qDebug ("        metrix #%5d [%02x] %c c:%-3d r:%-2d fb:%2d bb:%2d\n",
			indx,
			data & 0xff,
			amBase[indx],
			curChar,
			row,
			fontBit,
			byteBit);

                }
            }

	    if (debug & TRACEC)
		qDebug ("        bit loop ------------------ end");
        }

	if (debug & TRACEC)
	    qDebug ("        byte loop ----------------- end");
        //testG (14);

        /*-----------------------------------------------------------------------------------------
         * if startChar is past lastChar then exit
         *---------------------------------------------------------------------------------------*/
        if (startChar >= conscriptor->header.chCount)
            return;

        /*-----------------------------------------------------------------------------------------
         * The current ROW is not the last row
         * start the CHAR sequence again
         * from the BIT set at the end of the last 4byte row
         *---------------------------------------------------------------------------------------*/
        if (row < (conscriptor->header.chHeight -1)) {
            curChar = startChar;
            fontBit = startBit;
            row++;
            if (fontBit == conscriptor->metrixBase[curChar].width)
                fontBit = 0;

	    if (debug & TRACEC)
		qDebug ("        -------- Not Last Row: set curChar=%d "
                        "fontBit=%d set row=%d\n", curChar, fontBit, row);
            continue;
            }

        /*-----------------------------------------------------------------------------------------
         * beyond this everything is LAST ROW
         *-----------------------------------------------------------------------------------------
         * fontBit has NOT reached the char width
         * start the CHAR sequence again for the remaining bits on all these characters
         *---------------------------------------------------------------------------------------*/
        if (fontBit < conscriptor->metrixBase[curChar].width) {
                startChar = curChar;
                startBit = fontBit;
                row = 0; continue;

		if (debug & TRACEC)
		    qDebug ("        ------------ Last Row fontBit stays %d set "
                            "startChar=%d set row=%d\n", fontBit, curChar, row);
            }

        /*-----------------------------------------------------------------------------------------
         * We have reached the last BIT of the last ROW
         * start with next char
         * at row 0
         * and fontBit from 0
         *---------------------------------------------------------------------------------------*/
        for (++curChar; ;++curChar) {                   // Next non zero width char
            if (curChar == conscriptor->header.chCount)
                break;
            if ((conscriptor->metrixBase[curChar].width))
                break;
        }

        if ((startChar = curChar) >= conscriptor->header.chCount)
            return;

        row = 0;
        fontBit = 0;
        startBit = 0;

	if (debug & TRACEC)
	    qDebug ("        -------- Last Row set startChar=%d set row=0\n",
											curChar);
    }           // For Ever
}

/*-------------------------------------------------------------------------------------------------
 * paintEvent --
 *	header:name
 *-----------------------------------------------------------------------------------------------*/
void B4Font::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QRect dirtyRect = event->rect();
    QRect rect;

    if ((debug & TRACE6))
	qDebug () << "[font]-PE--------------- nGlyphs" << nGlyphs << "window" << painter.window();

    if (debug & TRACE8) {
	qDebug () << "        painter.drawImage() dirtyRect " << dirtyRect;
	if ((nGlyphs))
	    qDebug () << "        draw glyphs window" << painter.window();
    }
    if (debug & TRACE6)
	qDebug ("       1");
    painter.drawImage(dirtyRect, image, dirtyRect);

    for (int i=0; i < nGlyphs; i++) {
	if (allglyphs[i]->visible == false)
	    continue;
	rect.setRect (allglyphs[i]->getX (), allglyphs[i]->getY (),
		      allglyphs[i]->getWidth (), allglyphs[i]->getHeight ());
	painter.drawImage (rect, allglyphs[i]->getImage ());
    }

    if (debug & TRACE6)
	qDebug ("       2 skip to end if nGlyphs:%d == 0", nGlyphs);
    /*---------------------------------------------------------------------------------------------
     * Draw the macro glyph for editing
     * draw the highlight lines
     *-------------------------------------------------------------------------------------------*/
    if (nGlyphs > 0) {
	painter.setPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        rect.setRect (MACROX, MACROY, macroWidth, macroHeight);
        //painter.drawImage (rect, (*allglyphs[macroGlyph]->getImage ()));
        painter.drawImage (rect, allglyphs[macroGlyph]->getImage ());

	int i;
	int delta;
	int x = MACROX;
	int y = MACROY;
	int xx = x + macroWidth;
	int yy = y + macroHeight;
	int ascent = conscriptor->header.chAscent;
	
    if(lineLength == 0) {		// Sig FPE
	qDebug ("PE called with LL 0 - [why]");
    }

	for (i = delta = 0; delta < yy; i++) { 
	    delta = MACROY + (i * macroMagnification * viewportWidth) / lineLength;
	    if (i == ascent) {
		//qDebug ("[ascent %d] %3d,%3d %3d,%3d", i, x, delta, xx, delta);
		painter.setPen(QPen(Qt::red, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter.drawLine (x, delta, xx, delta);
	    }
	    else if (i == (ascent +1)) {
		//qDebug ("[ascent %d] %3d,%3d %3d,%3d", i, x, delta, xx, delta);
		painter.drawLine (x, delta, xx, delta);
		painter.setPen(QPen(Qt::black, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	    }
	    else {
		painter.drawLine (x, delta, xx, delta);
		//qDebug ("[ploty %d] %3d,%3d %3d,%3d", i, x, delta, xx, delta);
	    }

	}

	for (i = delta = 0; delta < xx; i++) {
	    delta = MACROX + (i * macroMagnification * viewportWidth) / lineLength;
	    //qDebug ("[plotx] %3d,%3d %3d,%3d", delta, y, delta, yy);
	    painter.drawLine (delta, y, delta, yy);
	}

	/*-----------------------------------------------------------------------------------------
	 * And macroGlyph lines
	 *---------------------------------------------------------------------------------------*/
	if ((allglyphs[macroGlyph]->visible)) {
	    painter.setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	    painter.drawLine   (allglyphs[macroGlyph]->getX(),
				allglyphs[macroGlyph]->getY(),
				MACROX,
				MACROY + macroHeight);
	    painter.drawLine   (allglyphs[macroGlyph]->getX() + allglyphs[macroGlyph]->getWidth(),
				allglyphs[macroGlyph]->getY(),
				MACROX + macroWidth,
				MACROY + macroHeight);
	}
	/*-----------------------------------------------------------------------------------------
	 * And text
	 *---------------------------------------------------------------------------------------*/
	if (debug & TRACE6)
	    qDebug ("       3");
	painter.setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	char str[80];
#   if QT_VERSION > 0x040700
	QStaticText msg;
#   endif

	sprintf (str, "FontName: %s", conscriptor->header.name);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+15, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+15, str);
#   endif

	//sprintf (str, "Character #%d", allglyphs[macroGlyph]->getCharNo());
	sprintf (str, "Glyph #%d", macroGlyph);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+35, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+35, str);
#   endif

	sprintf (str, "Code Point: %d [0x%02x]", allglyphs[macroGlyph]->getCodePoint(),
					        allglyphs[macroGlyph]->getCodePoint());
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+55, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+55, str);
#   endif

        cp2utf (allglyphs[macroGlyph]->getCodePoint());
	sprintf (str, "UTF-8: %s", utf);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+75, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+75, str);
#   endif

	sprintf (str, "Pixel Depth: %s", ((conscriptor->header.chDepth)) ? "2BPP" : "1BPP");
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+95, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+95, str);
#   endif

	sprintf (str, "Width:%d", allglyphs[macroGlyph]->getPixWidth());
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+115, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+115, str);
#   endif

	sprintf (str, "Height:%d", conscriptor->header.chHeight);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+135, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+135, str);
#   endif

	sprintf (str, "Ascent:%d", conscriptor->header.chAscent);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+155, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+155, str);
#   endif

	sprintf (str, "File Size:%d", 32+ conscriptor->metricCount+ conscriptor->pixmapCount);
	if (debug & TRACE6)
	    qDebug ("[font] -PE- header:32 metrix:%d pixmap:%d",
					    conscriptor->metricCount, conscriptor->pixmapCount);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (MACROX * 2 + macroWidth, MACROY+175, msg);
#   else
        painter.drawText (MACROX * 2 + macroWidth, MACROY+175, str);
#   endif
    }
    if ((debug & TRACE6))
	qDebug ("      /PE");
}

/*-------------------------------------------------------------------------------------------------
 * resizeEvent --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::resizeEvent(QResizeEvent *event)
{
    if ((debug & TRACE7))
	qDebug ("[font]-RE--------------- set viewport w:%d h:%d", width(), height());

    viewportWidth = width();
    viewportHeight = height();

    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
    }

    if ((nGlyphs)) {
	resetXY ();
    }
    update();
    if ((debug & TRACE7))
	qDebug ("      /RE");
}

/*-------------------------------------------------------------------------------------------------
 * mousePressEvent --
 *	do nothing if no file or new loaded
 *-----------------------------------------------------------------------------------------------*/
void B4Font::mousePressEvent (QMouseEvent *event)
{
int n;

    if (event->button() == Qt::LeftButton) {
	int x = event->x();
	int y = event->y();

	if (nGlyphs == 0)
	    return;

	if ((editGlyph (x, y)))
	    return;
	if ((n = selectGlyph (x, y)) >= 0) {
	    update ();
	    macroGlyph = n;
	}
    }
}

/*-------------------------------------------------------------------------------------------------
 * mouseWheelEvent --
 * int x = event->delta()/120;
  //this->setText("Total Steps: "+QString::number(x));
 *-----------------------------------------------------------------------------------------------*/
void B4Font::wheelEvent (QWheelEvent * event)
{
    if (nGlyphs == 0)
	return;

    if (event->delta() > 0) {		// positive
	if ((firstGlyph + 16) < nGlyphs)
	    firstGlyph += 16;
    }
    else {				// negative
	if ((firstGlyph))		// must be 16 or more
	   firstGlyph -= 16;
    }

    resetXY ();
    update ();
} 

/*-------------------------------------------------------------------------------------------------
 * mouseMoveEvent --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::mouseMoveEvent (QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton)) {
	int x = event->x();
	int y = event->y();

	if (nGlyphs == 0)
	    return;

	if ((editGlyph (x, y)))
	    return;
    }
}

/*-------------------------------------------------------------------------------------------------
 * on_actionRollUp --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionRollUp ()
{
    if (nGlyphs == 0)
	return;

    if ((debug))
	qDebug ("[font]-rollUP nGlyphs:%d firstGlyph:%d", nGlyphs, firstGlyph);

    if ((firstGlyph + 16) < nGlyphs)
	firstGlyph += 16;
    resetXY ();
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionRollDown --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionRollDown ()
{
    if ((debug))
	qDebug ("[font]-rollDOWN nGlyphs:%d firstGlyph:%d", nGlyphs, firstGlyph);

    if ((firstGlyph))		// must be 16 or more
       firstGlyph -= 16;
    resetXY ();
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * editGlyph --
 *	TRACE5
 *-----------------------------------------------------------------------------------------------*/
bool B4Font::editGlyph (int x, int y)
{
    if ((debug & TRACE5))
	qDebug ("[font]-editGlyph---------");

    if ((x > MACROX) && (x < (MACROX+ macroWidth)) &&
	(y > MACROY) && (y < (MACROY + macroHeight))) {
	conscriptor->setModified();
	int c = ((x - MACROX) * allglyphs[macroGlyph]->getPixWidth())  / macroWidth;
	int r = ((y - MACROY) * allglyphs[macroGlyph]->getPixHeight()) / macroHeight;
	switch (colorFlag) {
	    case clearCol:
		allglyphs[macroGlyph]->setPixel (c, r, conscriptor->clearColor.rgb());
		break;
	    case opaqueCol:
		allglyphs[macroGlyph]->setPixel (c, r, conscriptor->opaqueColor.rgb());
		break;
	    case cthruCol:
		allglyphs[macroGlyph]->setPixel (c, r, conscriptor->cthruColor.rgb());
		break;
	}
	update ();
	return true;
    }
    return false;
}

/*-------------------------------------------------------------------------------------------------
 * selectGlyph --
 *	recalculate width for THIS glyph
 *-----------------------------------------------------------------------------------------------*/
int B4Font::selectGlyph (int x, int y)
{
int theGlyph;

    if ((debug))
	qDebug ("[font]-selectGlyph--------");

    if (y < allglyphs[firstGlyph]->getY ())			// Before top
	return -1;

    for (theGlyph = firstGlyph; theGlyph < nGlyphs; theGlyph += 16) {
	if (y > (allglyphs[theGlyph]->getY() + allglyphs[theGlyph]->getHeight()))
	    continue;
								// Found the row
	for (int i = 0; (theGlyph < nGlyphs) && (1 < 16); theGlyph++, i++) {
	    if (((x > allglyphs[theGlyph]->getX())) &&
			    (x < (allglyphs[theGlyph]->getX() + allglyphs[theGlyph]->getWidth()))) {
		macroWidth = (allglyphs[theGlyph]->getPixWidth() *
					    viewportWidth * macroMagnification) / lineLength;
		return theGlyph;
	    }
	}
    }
    return -1;
}

/*-------------------------------------------------------------------------------------------------
 * resizeImage --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::resizeImage(QImage *image, const QSize &newSize)
{
    if ((debug))
	qDebug ("[font]        resizeImage");

    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonImportBDF --
 *	I make an effort to avoid parsing keywords in comments
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_buttonImportBDF ()
{
int maxGlyphs;
int indx;
int boundingBoxWidth;			// Font width from BOUBINGBOX
int boundingBoxHeight;			// Font width from BOUBINGBOX
int fromOriginX;			// Font x from BOUBINGBOX
int fromOriginY;			// Font y from BOUBINGBOX
int ascent;

    bdfFile = conscriptor->ba;

    /*---------------------------------------------------------------------------------------------
     * Discover BOUNDINGBOX sizes
     *-------------------------------------------------------------------------------------------*/
    indx = bdfFile.indexOf ("FONTBOUNDINGBOX", 0);
    sscanf (conscriptor->ba.data() + indx, "FONTBOUNDINGBOX %d %d %d %d",
			&boundingBoxWidth, &boundingBoxHeight, &fromOriginX, &fromOriginY);
    fromOriginX = abs (fromOriginX);			// safe for LtoR fonts only
    fromOriginY += (boundingBoxHeight -1);
    ascent = fromOriginY;

    /*---------------------------------------------------------------------------------------------
     * set header info
     *-------------------------------------------------------------------------------------------*/
    memset (conscriptor->header.name, 0, 16);			// spec

    QString b4fName = conscriptor->bdfDialog->editFontName->text();
    if (!(b4fName.endsWith(".b4f", Qt::CaseSensitive))) {
	strncpy ((char *)conscriptor->header.name, b4fName.toAscii().constData(), 12);
        b4fName.append (".b4f");
    }
    else {
	QFileInfo fName (b4fName);
	strncpy ((char *)conscriptor->header.name, fName.baseName().toAscii().constData(), 12);
    }
    conscriptor->setCurrentFile (b4fName);
    conscriptor->modified - false;

    conscriptor->header.chHeight = boundingBoxHeight;
    conscriptor->header.chAscent = ascent;
    conscriptor->header.chFirst  = conscriptor->bdfDialog->spinFirst->value();
    conscriptor->header.chCount  = conscriptor->bdfDialog->spinCount->value();
    conscriptor->header.fontVersion  = VERSIONNO;
    conscriptor->header.reserved[0]  = 0;
    conscriptor->header.reserved[1]  = 0;
    conscriptor->header.reserved[2]  = 0;
    conscriptor->header.reserved[3]  = 0;

    if ((conscriptor->bdfDialog->radio1BPP->isChecked()))
	conscriptor->header.chDepth = 0;
    else
	conscriptor->header.chDepth = 1;

    conscriptor->header.skip = 0;

    if (debug & TRACEA) {
	qDebug () << "[name]   " << (char *)conscriptor->header.name;
	qDebug () << "[count]  " << conscriptor->header.chCount;
	qDebug () << "[height] " << conscriptor->header.chHeight;
	qDebug () << "[depth]  " << (conscriptor->header.chDepth ? 2 : 1) << "BPP";
	qDebug () << "[first]  " << conscriptor->header.chFirst;
	qDebug () << "[count]  " << conscriptor->header.chCount;
    }

    if (conscriptor->header.chDepth == 0) {
	conscriptor->actionInsertOutline->setEnabled (false);
	conscriptor->actionOutline->setEnabled (false);
	conscriptor->actionCthruColor->setEnabled (false);
	conscriptor->action1to2BPP->setEnabled (true);
	conscriptor->action2to1BPP->setEnabled (false);
	if (colorFlag == cthruCol)
	    colorFlag = opaqueCol;
   }
    else {
	conscriptor->actionInsertOutline->setEnabled (true);
	conscriptor->actionOutline->setEnabled (true);
	conscriptor->actionCthruColor->setEnabled (true);
	conscriptor->action1to2BPP->setEnabled (false);
	conscriptor->action2to1BPP->setEnabled (true);
    }

    if ((maxGlyphs = conscriptor->bdfDialog->spinCount->value()) > MAXGLYPHS) {
	maxGlyphs = MAXGLYPHS;
	QMessageBox::warning(this,
		    tr("Conscriptor"),
		    tr("Maximum number of glyphs limited %1").arg (MAXGLYPHS),
		    QMessageBox::Ok);
    }
    setNewGlyphs (boundingBoxWidth, boundingBoxHeight, maxGlyphs);

    /*---------------------------------------------------------------------------------------------
     * Fill the glyphs from bitmap field
     *	indx -> STARTCHAR all-glyph-data ENDCHAR
     *-------------------------------------------------------------------------------------------*/
    int val;
    int maxval = 0;;
    int tmpx;
    int BBXwidth;
    int BBXheight;
    int BBXx;
    int BBXy;
    int ng;

    indx = bdfFile.indexOf (QRegExp ("[\\r\\n]STARTCHAR"));
    for (ng = 0; ng < maxGlyphs; ng++) {
	if ((indx = bdfFile.indexOf (QRegExp("[\\r\\n]STARTCHAR"), indx)) == -1)
	    break;
	tmpx = bdfFile.indexOf (QRegExp("[\\r\\n]ENCODING"), indx);
	sscanf (conscriptor->ba.data () + tmpx +1, "ENCODING %d", &val);
	if ((debug & TRACEB))
	    qDebug ("[search] index:%d tmpx:%d encoding:%d glyph:%d", indx, tmpx, val, ng);
	if (val == 0) {
	    indx = tmpx;
	    continue;			// This because Erics code comments
	}
	else
	    allglyphs[ng]->setCodePoint (val);
	maxval = qMax (val, maxval);

	tmpx = bdfFile.indexOf (QRegExp ("[\\r\\n]BBX"), indx);
	sscanf (conscriptor->ba.data () + tmpx +1, "BBX %d %d %d %d",
							&BBXwidth, &BBXheight, &BBXx, &BBXy);
	tmpIndx = bdfFile.indexOf (QRegExp ("[\\r\\n]BITMAP"), indx);

	int myx;
	int myy;
	QRgb opaquePix = conscriptor->opaqueColor.rgb ();

	if ((debug & TRACEA))
	    qDebug () << bdfFile.data();
	for (int row = 0; row < BBXheight; row++) {
	    myy = ascent - BBXy - BBXheight + row +1;
	    if ((debug & TRACEA))
		printf ("%2d> ", myy);
	    for (int col = 0; col < BBXwidth; col++) {
		myx = fromOriginX + BBXx + col;
		if (getbit (col))
		    allglyphs[ng]->setPixel (myx, myy, opaquePix);
	    if ((debug & TRACEA))
		printf (" %d", myx);
	    }

	if ((debug & TRACEA))
	    printf ("\n");
	}

	indx = bdfFile.indexOf (QRegExp ("[\\r\\n]ENDCHAR"), indx);
    }

    for (int i = 0; i < maxGlyphs; ++i)
	if (allglyphs[i]->getCharNo () < 0)
	    allglyphs[i]->setCodePoint (++maxval);
    setMetricCount();
    setPixmapWidth();

    sortGlyphs ();

    resetXY ();
    update ();
}

/*-------------------------------------------------------------------------------------------------
 * getbit -- read the data stream for the next bit
 *	tmpIndx is set to BITMAP on first entry
 *-----------------------------------------------------------------------------------------------*/
bool B4Font::getbit (int col)
{
    int bitNo = col % 4;
    int mask = 8 >> bitNo;

    /*---------------------------------------------------------------------------------------------
     * new row of data for each line
     *-------------------------------------------------------------------------------------------*/
    if (col == 0)				
	tmpIndx = bdfFile.indexOf (QRegExp ("[\\r\\n][0-9A-Fa-f]+"), tmpIndx+1); // next row data

    /*---------------------------------------------------------------------------------------------
     * new char every 4 bits
     *-------------------------------------------------------------------------------------------*/
    if (bitNo == 0) {
	bitmapVal = conscriptor->ba.data() [(col / 4) + tmpIndx +1];

	//printf (" [%d]%02x ", (col / 4) + tmpIndx +1, bitmapVal);
	if ((bitmapVal >= '0') && (bitmapVal <= '9'))
	    bitmapVal -= '0';
	else if ((bitmapVal >= 'A') && (bitmapVal <= 'F'))
	    bitmapVal -= ('A' - 10);
	else if ((bitmapVal >= 'a') && (bitmapVal <= 'f'))
	    bitmapVal -= ('a' - 10);
    }

    //printf (" %02x:%02x", bitmapVal, mask);
    if ((bitmapVal & mask))
	return true;

   return false;
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonPrint[Screen | All] --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_buttonPrintScreen ()
{
int drawW;
int drawH;
float delta;
int pageW;
int pageH;

    if (nGlyphs == 0) {
	conscriptor->statusBar()->showMessage(tr("Nothing to print"), 5000);
	return;
    }

    QPrintDialog printDialog (&printer, this);
    printDialog.setWindowTitle(tr("Print Visible Glyphs"));
    if (printDialog.exec()) {
	QPainter painter (&printer);
	// Achieved nothing
	//printer.setResolution (300);
	//painter.begin(&printer);
	QRect rect = painter.viewport();
	//qDebug () << "[paper]" << rect;

        int glyphH = allglyphs[macroGlyph]->getPixHeight ();
        int glyphW = allglyphs[macroGlyph]->getPixWidth ();

	pageW = rect.width() - 20;
	pageH = rect.height();

	/*-----------------------------------------------------------------------------------------
	 * make the viewport bigger to get 1 pix lines thinner
	 *---------------------------------------------------------------------------------------*/
	//painter.setViewport (0,0, pageW, pageH);

	if ((glyphH > 128) || (glyphW > 128))
	    drawW = 7 * pageW / 10;
	else
	    drawW = 4 * pageW / 10;


	if ((drawH = drawW * glyphH / glyphW) > pageH) {
	    drawH = pageH;
	    drawW = pageH * glyphW / glyphH;
	}

	/*-----------------------------------------------------------------------------------------
	 * the rect to draw macroGlyph in
	 *---------------------------------------------------------------------------------------*/
	QRect pRect (0, 0, drawW, drawH);

        painter.drawImage (pRect, allglyphs[macroGlyph]->getImage ());
	painter.setPen(QPen(Qt::black, 0, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin));

	// Horiz Lines
	for (int i = 0; i <= glyphH; i++) { 
	    delta = i * drawH / glyphH;
	    painter.drawLine (0, delta, drawW, delta);
	}

	// Vert Lines
	for (int i = 0; i <= glyphW; i++) {
	    delta = i * drawW / glyphW;
	    painter.drawLine (delta, 0, delta, drawH);
	}

	// Text
	char str[80];
#   if QT_VERSION > 0x040700
	QStaticText msg;
#   endif

	sprintf (str, "FontName: %s", conscriptor->header.name);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 15, msg);
#   else
        painter.drawText (drawW + 10, 15, str);
#   endif

	//sprintf (str, "Character #%d", allglyphs[macroGlyph]->getCharNo());
	sprintf (str, "Glyph #%d", macroGlyph);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 35, msg);
#   else
        painter.drawText (drawW + 10, 35, str);
#   endif

	sprintf (str, "Code Point: %d [0x%02x]", allglyphs[macroGlyph]->getCodePoint(),
					        allglyphs[macroGlyph]->getCodePoint());
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 55, msg);
#   else
        painter.drawText (drawW + 10, 55, str);
#   endif

        cp2utf (allglyphs[macroGlyph]->getCodePoint());
	sprintf (str, "UTF-8: %s", utf);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 75, msg);
#   else
        painter.drawText (drawW + 10, 75, str);
#   endif

	sprintf (str, "Pixel Depth: %s", ((conscriptor->header.chDepth)) ? "2BPP" : "1BPP");
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 95, msg);
#   else
        painter.drawText (drawW + 10, 95, str);
#   endif

	sprintf (str, "Width:%d", allglyphs[macroGlyph]->getPixWidth());
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 115, msg);
#   else
        painter.drawText (drawW + 10, 115, str);
#   endif

	sprintf (str, "Height:%d", conscriptor->header.chHeight);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 135, msg);
#   else
        painter.drawText (drawW + 10, 135, str);
#   endif

	sprintf (str, "Ascent:%d", conscriptor->header.chAscent);
#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 155, msg);
#   else
        painter.drawText (drawW + 10, 155, str);
#   endif

	sprintf (str, "File Size:%d", 32+ conscriptor->metricCount+ conscriptor->pixmapCount);

#   if QT_VERSION > 0x040700
	msg.setText (str);
	painter.drawStaticText (drawW + 10, 175, msg);
#   else
        painter.drawText (drawW + 10, 175, str);
#   endif

	// Visible Glyphs
	int imageX;
	int imageY = drawH + 20;
	int imageXZ = pageW - 51;				// 3 pix spacing
	int imageRowPixW;
	int imageH;
	int i;							// ISO scoping
	int j;							// ISO scoping

	for (i = 0; i < nGlyphs; i += 16) {
	    if (allglyphs[i]->visible == false)
		continue;

	    imageRowPixW = 0;
	    for (j = i; (j < (i + 16)) && (j < nGlyphs); j++)
		imageRowPixW += allglyphs[j]->getPixWidth();
	    //qDebug ("[imageRowPixW] %d", imageRowPixW);

            int delta = j - i;
	    if (delta < 16) {
		imageRowPixW = imageRowPixW * 16 / delta;
		//qDebug ("last row %d gliphs ->16 so width is %d", delta, imageRowPixW);
	    }

	    imageH = imageXZ * conscriptor->header.chHeight / imageRowPixW;
	    imageX = 3;
	    for (j = i; (j < (i + 16)) && (j < nGlyphs); j++) {
		QRect box
		    (imageX, imageY, allglyphs[j]->getPixWidth() * imageXZ / imageRowPixW, imageH);
		//qDebug () << "[glyph]" << box;
		painter.drawImage (box, allglyphs[j]->getImage ());
		imageX += (3 + (allglyphs[j]->getPixWidth()) * imageXZ / imageRowPixW);
	    }
	    imageY += (imageH + 3);
	    if ((imageY + imageH) > pageH)
		break;
	}
    }
}

/*-------------------------------------------------------------------------------------------------
 * on_buttonPrintAll --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_buttonPrintAll ()
{
int drawH;

    if (nGlyphs == 0) {
	conscriptor->statusBar()->showMessage(tr("Nothing to print"), 5000);
	return;
    }

    QPrintDialog printDialog (&printer, this);
    printDialog.setWindowTitle(tr("Print All Glyphs"));
    if (printDialog.exec()) {
	QPainter painter (&printer);
	QRect rect = painter.viewport();

	int pageW = rect.width() - 20;
	int pageH = rect.height();

	int imageX;					// x ord of image
	int imageY = 20;					// y ord of image
	int imageXZ = pageW - 48;				// 3 pix spacing
	int imageRowPixW;					// pix width of the whole row
	int imageH;						// height of each row
	int i;							// ISO scoping
	int j;							// ISO scoping

	for (i = 0; i < nGlyphs; i += 16) {

	    imageRowPixW = 0;
	    for (j = i; (j < (i + 16)) && (j < nGlyphs); j++)
		imageRowPixW += allglyphs[j]->getPixWidth();

	    int delta = j - i;
	    if (delta < 16)
		imageRowPixW = imageRowPixW * 16 / delta;

	    imageH = imageXZ * conscriptor->header.chHeight / imageRowPixW;
	    imageX = 3;
	    for (j = i; (j < (i + 16)) && (j < nGlyphs); j++) {
		QRect box
		    (imageX, imageY, allglyphs[j]->getPixWidth() * imageXZ / imageRowPixW, imageH);
		painter.drawImage (box, allglyphs[j]->getImage ());
		imageX += (3 + (allglyphs[j]->getPixWidth()) * imageXZ / imageRowPixW);
	    }
	    imageY += (imageH + 3);
	    if ((imageY + imageH) > pageH) {
		imageY = 20;
		printer.newPage ();
	    }
	}
    }
}

/*-------------------------------------------------------------------------------------------------
 * EDIT functions
 * edit.move
 * on_actionMoveDown --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionMoveDown ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->rowDown();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.move
 * on_actionMoveUp --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionMoveUp ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->rowUp();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.move
 * on_actionMoveRight --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionMoveRight ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->columnRight();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.move
 * on_actionMoveLeft --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionMoveLeft ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->columnLeft();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionCut --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionCut ()
{
    if (nGlyphs == 0)
	return;

    pasteBuffer = allglyphs[macroGlyph]->getImage();
    allglyphs[macroGlyph]->getImage().fill (conscriptor->clearColor.rgb());
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionCopy --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionCopy ()
{
    if (nGlyphs == 0)
	return;

    pasteBuffer = allglyphs[macroGlyph]->getImage();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionPaste --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionPaste ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->setImage(pasteBuffer);
//WHY
    setPixmapWidth ();
    findWidth ();
    setPixmapWidth ();
    resetXY ();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionClear --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionClear ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->getImage().fill (conscriptor->clearColor.rgb());
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.insert
 * on_actionInsertRow --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionInsertRow ()
{
    if ((nGlyphs == 0) || (conscriptor->header.chHeight > 254))
	return;

    conscriptor->header.chHeight++;

    for (int i = 0; i < nGlyphs; i++)
	allglyphs[i]->insertRow();
    conscriptor->header.chHeight++;
    resetXY ();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.insert
 * on_actionInsertColumn --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionInsertColumn ()
{
    if (nGlyphs == 0)
	return;

    allglyphs[macroGlyph]->insertColumn();
    setPixmapWidth ();
    findWidth ();
    resetXY ();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.insert
 * on_actionInsertCharacter --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionInsertChar ()
{
int prevN;
int thisN;
int nextN;

    if (nGlyphs == 0)
	return;

    /*---------------------------------------------------------------------------------------------
     * insert before macroGlyph at the end
     *-------------------------------------------------------------------------------------------*/
    if ((nextN = macroGlyph +1) == nGlyphs) {
	prevN = macroGlyph -1;
	allglyphs[nextN] = allglyphs[macroGlyph];
	allglyphs[macroGlyph] = new Glyph (conscriptor);
	allglyphs[macroGlyph]->newGlyph (allglyphs[nextN]->getPixWidth(),
		    allglyphs[nextN]->getPixHeight(), allglyphs[prevN]->getCharNo () +1);
	if ((allglyphs[nextN]->getCharNo() - allglyphs[prevN]->getCharNo()) < 2) 
	    allglyphs[nextN]->setCodePoint (allglyphs[macroGlyph]->getCodePoint() +1);
    }

    /*---------------------------------------------------------------------------------------------
     * insert before macroGlyph from the beginning upto the end
     * we *know* that there is a next glyph
     *-------------------------------------------------------------------------------------------*/
    else {
	int r = QMessageBox::warning(this,
		tr("Conscriptor"),
		tr("Every character from here on will be incremented\n"
		   "IE \"1\" will become \"2\", \"a\" will become \"b\" etc\n"
		   "See discussion in Help->Insert->char\n"
		   "Is this what you want?"),
	    QMessageBox::Yes | QMessageBox::No);
	if (r == QMessageBox::No)
		return;

	for (thisN = nGlyphs, prevN = nGlyphs -1; thisN > macroGlyph; thisN--, prevN--) {
	    allglyphs[prevN]->setCodePoint (allglyphs[prevN]->getCodePoint() +1);
	    //qDebug ("allglyphs[%d] CodePoint %d->%d", prevN,
	    //	     allglyphs[prevN]->getCodePoint() -1, allglyphs[prevN]->getCodePoint());
	    allglyphs[thisN] = allglyphs[prevN];
	    //qDebug ("         [%d] -> [%d]", prevN, thisN);
	}

	allglyphs[macroGlyph] = new Glyph (conscriptor);
	//qDebug ("new[%d] set w:%d h:%d from [%d]",
	//	macroGlyph,
	//	allglyphs[nextN]->getPixWidth(),
	//	allglyphs[nextN]->getPixHeight(),
	//	nextN);
	if (macroGlyph == 0)
	    allglyphs[macroGlyph]->newGlyph (allglyphs[nextN]->getPixWidth(),
			    allglyphs[nextN]->getPixHeight(), conscriptor->header.chFirst);
	else
	    allglyphs[macroGlyph]->newGlyph (allglyphs[nextN]->getPixWidth(),
			    allglyphs[nextN]->getPixHeight(), allglyphs[nextN]->getCharNo() -1);
    }
    nGlyphs++;
    // chCount is lastCodePoint - firstCodePoint + 1;
    conscriptor->header.chCount = allglyphs[nGlyphs-1]->getCharNo() + 1;
    setPixmapWidth ();
    findWidth ();
    resetXY ();
    conscriptor->setModified ();
    update();
}

/*-------------------------------------------------------------------------------------------------
 * toolbar - outline
 * on_actionInsertOutline --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionOutline ()
{
    if ((debug))
	qDebug () << "Single Char Outline";
    for (int i = 0; i < nGlyphs; i++)
	allglyphs[i]->outline (conscriptor->clearColor.rgb (),
			       conscriptor->opaqueColor.rgb (),
			       conscriptor->cthruColor.rgb ());
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.insert
 * on_actionInsertOutline --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionInsertOutline ()
{
    if ((debug))
	qDebug () << "Whole font Outline";
    allglyphs[macroGlyph]->outline (conscriptor->clearColor.rgb (),
				    conscriptor->opaqueColor.rgb (),
				    conscriptor->cthruColor.rgb ());
    update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.delete
 * on_actionDeleteRow
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionDeleteRow ()
{
    if ((nGlyphs == 0) || (conscriptor->header.chHeight == 0))
	return;

    conscriptor->header.chHeight--;

    for (int i = 0; i < nGlyphs; i++)
	allglyphs[i]->deleteRow();
    conscriptor->header.chHeight--;
    resetXY ();
    conscriptor->setModified ();
    update();
    //conscriptor->scrollArea->viewport()->update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.delete
 * on_actionDeleteColumn --
 *	if the glyph has width them pixmap *must* have width
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionDeleteColumn ()
{
    if ((nGlyphs == 0) || (allglyphs[macroGlyph]->getPixWidth() == 0))
	return;

    allglyphs[macroGlyph]->deleteColumn();
    setPixmapWidth ();
    findWidth ();
    resetXY ();
    conscriptor->setModified ();
    update();
    //conscriptor->scrollArea->viewport()->update();
}

/*-------------------------------------------------------------------------------------------------
 * edit.delete
 * on_actionDeleteChar --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionDeleteChar ()
{
    if (nGlyphs == 0)
	return;

    delete allglyphs[macroGlyph];

    if ((macroGlyph + 1) == nGlyphs)			// delete last glyph
	macroGlyph--;
    else {						// anywhere else
	int me = macroGlyph;
	for (int next = me + 1; next < nGlyphs; me++, next++)
	    allglyphs[me] = allglyphs[next];
    }
    conscriptor->header.chCount = allglyphs[--nGlyphs -1]->getCharNo() + 1;
    //qDebug ("header.chCount %d Glyph[%d]char %d", conscriptor->header.chCount, nGlyphs -1,
    //						    allglyphs[nGlyphs-1]->getCodePoint());
    setPixmapWidth ();
    findWidth ();
    resetXY ();
    conscriptor->setModified ();
    update();
    //conscriptor->scrollArea->viewport()->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionCodePoint --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionCodePoint ()
{
    if (nGlyphs == 0)
	return;

    CodePointDialog *dialog =  new CodePointDialog (this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    //dialog->setWindowTitle (strippedName (curFile));
    if (dialog->exec() == QDialog::Rejected)
        return;

    for (int i = 0; i < nGlyphs; i++)			// check if value is used
	if (allglyphs[i]->getCodePoint () == (unsigned int)value) {
	    QMessageBox::warning(this,
			tr("Conscriptor"),
			tr("This code point is already used"),
		    QMessageBox::Ok);
	    return;
	}

    allglyphs[macroGlyph]->setCodePoint (value);		// OK to use it
    sortGlyphs ();
}

/*-------------------------------------------------------------------------------------------------
 * insertion sort the glyphs
 *-----------------------------------------------------------------------------------------------*/
void B4Font::sortGlyphs ()
{
    Glyph *item;
    int hole;
    for (int i = 1; i < nGlyphs; i++) { 
	item = allglyphs[i];
	hole = i;
	while ((hole > 0) && (allglyphs[hole-1]->getCodePoint() > item->getCodePoint())) {
	    allglyphs[hole] = allglyphs[hole-1];	// move hole to next smaller index
	    hole--;
	}
	allglyphs[hole] = item;				// put item in hole
    }
    conscriptor->header.chCount = allglyphs[nGlyphs -1]->getCharNo() + 1;	// 1stchar[0]
    setMetricCount ();    
    //qDebug ("Set header.chCount to %d", conscriptor->header.chCount);
    resetXY ();
    conscriptor->setModified ();
    update();
    //conscriptor->scrollArea->viewport()->update();
}

/*-------------------------------------------------------------------------------------------------
 * on_actionImportImage --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::on_actionImportImage ()
{
    if ((debug))
	qDebug ("[font] ImportImage");

    if (((conscriptor->modified)) && (!(conscriptor->curFile.isEmpty()))) {
	int r = QMessageBox::warning(this,
					tr("Import Image"),
					tr("The existing font has been modified.\n"
					   "Do you want to save it?"),
					QMessageBox::Yes | QMessageBox::No);
	if (r == QMessageBox::Yes)
	    conscriptor->save();
    }

    ImportImageDialog importImageDialog (this);
    //importImageDialog.setAttribute(Qt::WA_DeleteOnClose);
    if (importImageDialog.exec() == QDialog::Rejected)
        return;

    memset (conscriptor->header.name, 0, 16);			// spec
    strcpy ((char *)conscriptor->header.name, "picture");
    conscriptor->setCurrentFile ("picture.b4f");
    conscriptor->header.chCount = 1;
    conscriptor->header.chFirst = 32;
    conscriptor->header.chHeight = thePicture->height ();
    conscriptor->header.chAscent = 0;
    conscriptor->header.fontVersion = VERSIONNO;

    setNewGlyphs (thePicture->width(), thePicture->height(), 1);
    allglyphs[0]->setImage ((*thePicture));

    conscriptor->header.chDepth = 1;		// Always imported as 2BPP

    if (debug & TRACEA) {
	qDebug () << "[name]   " << (char *)conscriptor->header.name;
	qDebug () << "[count]  " << conscriptor->header.chCount;
	qDebug () << "[height] " << conscriptor->header.chHeight;
	qDebug () << "[depth]  " << (conscriptor->header.chDepth ? 2 : 1) << "BPP";
	qDebug () << "[first]  " << conscriptor->header.chFirst;
	qDebug () << "[count]  " << conscriptor->header.chCount;
    }

    setMetricCount();
    setPixmapWidth();
    setPixmapCount();

    resetXY ();
    update ();

    if ((debug))
	qDebug ("      /ImportImage");
}

/*-------------------------------------------------------------------------------------------------
 * cp2utf --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::cp2utf (int n)
{
int o;
int p;
int q;
int r;

    if (n < 0x80) {
	sprintf (utf, "%02x", n);
    }
    else if (n < 0x800) {
	o = (n & 0x3f) | 0x80;
	p = ((n >> 6) & 0x1f) | 0xc0;
	sprintf (utf, "%02x %02x", p, o);
    }
    else if (n < 0x10000) {
	o = (n & 0x3f) | 0x80;
	p = ((n >> 6) & 0x3f) | 0x80;
	q = ((n >> 12) & 0x0f) | 0xe0;
	sprintf (utf, "%02x %02x %02x", q, p, o);
    }
    else if (n < 0x110000) {
	o = (n & 0x3f) | 0x80;
	p = ((n >> 6) & 0x3f) | 0x80;
	q = ((n >> 12) & 0x3f) | 0x80;
	r = ((n >> 18) & 0x0f) | 0xf0;
	sprintf (utf, "%02x %02x %02x %02x", r, q, p, o);
    }
    else {
	sprintf (utf, "[error] unhandled code point\n");
    }
}

/*-------------------------------------------------------------------------------------------------
 * setPixmapWidth --
 *	here be dragons 1BPP 2BPP (more?)
 *-----------------------------------------------------------------------------------------------*/
void B4Font::setPixmapWidth ()
{
int totalPixWidth = 0;
int byteCount;

    if ((debug))
	qDebug ("[font]        setPixmapWidth");

    for (int i = 0; i < nGlyphs; i++)
	totalPixWidth += allglyphs[i]->getPixWidth ();
    if ((conscriptor->header.chDepth))
	totalPixWidth *= 2;
    byteCount = 4 * (totalPixWidth % 32 ? totalPixWidth / 32 + 1: totalPixWidth / 32);
    //qDebug ("              [totalPixWidth] %d [[pixWidth] pixmap 1 line byteCount] %d",
    //								    totalPixWidth, byteCount);
    conscriptor->header.pixWidth = byteCount;
    if ((debug))
	qDebug ("             /setPixmapWidth %d", byteCount);
}

/*-------------------------------------------------------------------------------------------------
 * setPixmapCount --
 *-----------------------------------------------------------------------------------------------*/
void B4Font::setPixmapCount ()
{
    if ((debug))
	qDebug ("[font]        setPixmapCount");
 
    conscriptor->pixmapCount =
		     ((conscriptor->header.pixWidth * conscriptor->header.chHeight) % 32) ?
		    (((conscriptor->header.pixWidth * conscriptor->header.chHeight) / 32) +1) * 32 :
		       conscriptor->header.pixWidth * conscriptor->header.chHeight; 
    if ((debug))
	qDebug ("              /setPixmapCount [pixmapCount]%d pixMap %dx%d",
	    conscriptor->pixmapCount, conscriptor->header.pixWidth, conscriptor->header.chHeight);
}

/*-------------------------------------------------------------------------------------------------
 * setMetricCount --
 *	header, metricData and pixmap must each be 32 byte aligned in the file
 * what if top n chars have zero width
 *	called when we still may have -1 chars in the glyph set
 *-----------------------------------------------------------------------------------------------*/
void B4Font::setMetricCount ()
{
    int m1c;
    int gCount;

    if ((debug))
	qDebug ("[font]        setMetricCount");

    if (conscriptor->header.chCount == 0)
	return;

    for (m1c = 0, gCount = nGlyphs -1; gCount > -1; gCount--) {
	if (allglyphs[gCount]->getCharNo() != -1)
	    continue;;
	m1c++;
    }

    if ((gCount == 0) && (allglyphs[0]->getCharNo() == -1)) {
	// conted down from nGlyphs to 0 and it's still -1
	conscriptor->header.chCount = m1c;
	//if ((debug))
	    qDebug ("              [chCount]%d [nGlyphs]%d. Every entry is -1)", m1c, nGlyphs);
    }
    else {
	conscriptor->header.chCount = m1c + allglyphs[nGlyphs -1]->getCharNo() + 1;
	if ((debug))
	    qDebug ("              [charCount:%d == lastCharNo:%d + 1 + (-1s):%d",
					    conscriptor->header.chCount,
					    allglyphs[nGlyphs - 1]->getCharNo(),
					    m1c);
    }
   
    // last (say) 36 first (say) 32 there are 5 chars 6 spaces-in-metrixTable 
    int byteCount = (conscriptor->header.chCount +1) *6;
    if (((byteCount % 32) == 0) && (byteCount > 0))
        conscriptor->metricCount = byteCount;
    else
        conscriptor->metricCount = ((byteCount / 32) + 1) *32;

    if ((debug))
	qDebug ("              /setMetricCount %d", conscriptor->metricCount);
}

	

