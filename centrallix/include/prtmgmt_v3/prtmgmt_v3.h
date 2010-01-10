#ifndef _PRTMGMT_V3_H
#define _PRTMGMT_V3_H

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 2001 LightSys Technology Services, Inc.		*/
/* 									*/
/* This program is free software; you can redistribute it and/or modify	*/
/* it under the terms of the GNU General Public License as published by	*/
/* the Free Software Foundation; either version 2 of the License, or	*/
/* (at your option) any later version.					*/
/* 									*/
/* This program is distributed in the hope that it will be useful,	*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	*/
/* GNU General Public License for more details.				*/
/* 									*/
/* You should have received a copy of the GNU General Public License	*/
/* along with this program; if not, write to the Free Software		*/
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  		*/
/* 02111-1307  USA							*/
/*									*/
/* A copy of the GNU General Public License has been included in this	*/
/* distribution in the file "COPYING".					*/
/* 									*/
/* Module:	prtmgmt.c,prtmgmt.h                                     */
/* Author:	Greg Beeley (GRB)                                       */
/* Date:	December 12, 2001                                       */
/*									*/
/* Description:	This module provides the Version-3 printmanagement	*/
/*		subsystem functionality.				*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: prtmgmt_v3.h,v 1.7 2010/01/10 07:20:18 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/include/prtmgmt_v3/prtmgmt_v3.h,v $

    $Log: prtmgmt_v3.h,v $
    Revision 1.7  2010/01/10 07:20:18  gbeeley
    - (feature) adding CSV file output from report writer.  Simply outputs
      only tabular data (report/table data) into a CSV file format.
    - (change) API addition to prtmgmt -- report writer can specify data type
      of a piece of printed data; used as "hints" by the CSV file output to
      output a cell as a quoted string vs. an integer or currency value

    Revision 1.6  2009/06/26 16:18:59  gbeeley
    - (change) GetCharacterMetric now returns both height and width
    - (performance) change from bubble sort to merge sort for page generation
      and output sequencing (this made a BIG difference)
    - (bugfix) attempted fix of text output overlapping problems, but there
      are still trouble points here.

    Revision 1.5  2007/02/17 04:34:51  gbeeley
    - (bugfix) test_obj should open destination objects with O_TRUNC
    - (bugfix) prtmgmt should remember 'configured' line height, so it can
      auto-adjust height only if the line height is not explicitly set.
    - (change) report writer should assume some default margin settings on
      tables/table cells, so that tables aren't by default ugly :)
    - (bugfix) various floating point comparison fixes
    - (feature) allow top/bottom/left/right border options on the entire table
      itself in a report.
    - (feature) allow setting of text line height with "lineheight" attribute
    - (change) allow table to auto-scale columns should the total of column
      widths and separations exceed the available inner width of the table.
    - (feature) full justification of text.

    Revision 1.4  2005/02/26 06:42:38  gbeeley
    - Massive change: centrallix-lib include files moved.  Affected nearly
      every source file in the tree.
    - Moved all config files (except centrallix.conf) to a subdir in /etc.
    - Moved centrallix modules to a subdir in /usr/lib.

    Revision 1.3  2005/02/24 05:44:32  gbeeley
    - Adding PostScript and PDF report output formats.  (pdf is via ps2pdf).
    - Special Thanks to Tim Irwin who participated in the Apex NC CODN
      Code-a-Thon on Feb 5, 2005, for much of the initial research on the
      PostScript support!!  See http://www.codn.net/
    - More formats (maybe PNG?) should be easy to add.
    - TODO: read the *real* font metric files to get font geometries!
    - TODO: compress the images written into the .ps file!

    Revision 1.2  2003/07/09 18:10:02  gbeeley
    Further fixes and enhancements to prtmgmt layer, particularly regarding
    visual layout of graphical borders around objects; border/shadow
    thickness is now automatically computed into the total margin between
    exterior edges and interior edges of an object.

    Revision 1.1  2003/04/21 21:00:51  gbeeley
    HTML formatter additions including image, table, rectangle, multi-col,
    fonts and sizes, now supported.  Rearranged header files for the
    subsystem so that LMData (layout manager specific info) can be
    shared with HTML formatter subcomponents.

    Revision 1.15  2003/03/18 04:06:24  gbeeley
    Added basic image (picture/bitmap) support; only PNG images supported
    at present.  Moved image and border (rectangles) functionality into a
    new file prtmgmt_v3_graphics.c.  Graphics are only monochrome at the
    present and work only under PCL (not plain text!!!).  PNG support is
    via libpng, so libpng was added to configure/autoconf.

    Revision 1.14  2003/03/15 04:46:00  gbeeley
    Added borders to tables.  Not fully tested yet.  Added a new component
    of the "PrtBorder" object: "Pad", which is the padding 'outside' of
    the border.  The reporting objdriver is going to have to really
    simplify the margins/borders stuff on tables because there are so many
    params that can be set - it can be confusing and hard to get right.

    Revision 1.13  2003/03/12 20:51:34  gbeeley
    Tables now working, but borders on tables not implemented yet.
    Completed the prt_internal_Duplicate routine and reworked the
    API interface to InitContainer on the layout managers.  Not all
    features/combinations on tables have been tested.  Footers on
    tables not working but (repeating) headers are.  Added a new
    prt obj stream field called "ContentSize" which provides the
    allocated memory size of the "Content" field.

    Revision 1.12  2003/03/07 06:16:11  gbeeley
    Added border-drawing functionality, and converted the multi-column
    layout manager to use that for column separators.  Added border
    capability to textareas.  Reworked the deinit/init kludge in the
    Reflow logic.

    Revision 1.11  2003/03/06 02:52:30  gbeeley
    Added basic rectangular-area support (example - border lines for tables
    and separator lines for multicolumn areas).  Works on both PCL and
    textonly.  Palette-based coloring of rectangles (via PCL) not seeming
    to work consistently on my system, however.  Warning: using large
    dimensions for the 'rectangle' command in test_prt may consume much
    printer ink!!  Now it's time to go watch the thunderstorms....

    Revision 1.10  2003/03/01 07:24:01  gbeeley
    Ok.  Balanced columns now working pretty well.  Algorithm is currently
    somewhat O(N^2) however, and is thus a bit expensive, but still not
    bad.  Some algorithmic improvements still possible with both word-
    wrapping and column balancing, but this is 'good enough' for the time
    being, I think ;)

    Revision 1.9  2003/02/27 22:02:18  gbeeley
    Some improvements in the balanced multi-column output.  A lot of fixes
    in the multi-column output and in the text layout manager.  Added a
    facility to "schedule" reflows rather than having them take place
    immediately.

    Revision 1.8  2003/02/27 05:21:19  gbeeley
    Added multi-column layout manager functionality to support multi-column
    sections (this is newspaper-style multicolumn formatting).  Tested in
    test_prt "columns" command with various numbers of columns.  Balanced
    mode not yet working.

    Revision 1.7  2003/02/25 03:57:48  gbeeley
    Added incremental reflow capability and test in test_prt.  Added stub
    multi-column layout manager.  Reflow is horribly inefficient, but not
    worried about that at this point.

    Revision 1.6  2003/02/19 22:53:53  gbeeley
    Page break now somewhat operational, both with hard breaks (form feeds)
    and with soft breaks (page wrapping).  Some bugs in how my printer (870c)
    places the text on pages after a soft break (but the PCL seems to look
    correct), and in how word wrapping is done just after a page break has
    occurred.  Use "printfile" command in test_prt to test this.

    Revision 1.5  2002/11/22 19:29:37  gbeeley
    Fixed some integer return value checking so that it checks for failure
    as "< 0" and success as ">= 0" instead of "== -1" and "!= -1".  This
    will allow us to pass error codes in the return value, such as something
    like "return -ENOMEM;" or "return -EACCESS;".

    Revision 1.4  2002/10/21 22:55:11  gbeeley
    Added font/size test in test_prt to test the alignment of different fonts
    and sizes on one line or on separate lines.  Fixed lots of bugs in the
    font baseline alignment logic.  Added prt_internal_Dump() to debug the
    document's structure.  Fixed a YSort bug where it was not sorting the
    YPrev/YNext pointers but the Prev/Next ones instead, and had a loop
    condition problem causing infinite looping as well.  Fixed some problems
    when adding an empty obj to a stream of objects and then modifying
    attributes which would change the object's geometry.

    There are still some glitches in the line spacing when different font
    sizes are used, however.

    Revision 1.3  2002/10/18 22:01:38  gbeeley
    Printing of text into an area embedded within a page now works.  Two
    testing options added to test_prt: text and printfile.  Use the "output"
    option to redirect output to a file or device instead of to the screen.
    Word wrapping has also been tested/debugged and is functional.  Added
    font baseline logic to the design.

    Revision 1.2  2002/04/25 04:30:13  gbeeley
    More work on the v3 print formatting subsystem.  Subsystem compiles,
    but report and uxprint have not been converted yet, thus problems.

    Revision 1.1  2002/01/27 22:50:05  gbeeley
    Untested and incomplete print formatter version 3 files.
    Initial checkin.


 **END-CVSDATA***********************************************************/

#include "cxlib/xarray.h"
#include "cxlib/xhash.h"


#define PRT_XY_CORRECTION_FACTOR	(72.0/120.0)
#define PRT_FP_FUDGE			(0.000001)


/*** Layout Manager Structure ***/
typedef struct _PLM
    {
    int			Magic;
    char		Name[32];
    int			(*AddObject)();		/* adds a child object to a parent */
    int			(*ChildResizeReq)();	/* can we resize the child object in this container? */
    int			(*ChildResized)();	/* child object in this container *was* resized. */
    int			(*InitContainer)();	/* init a newly created container */
    int			(*DeinitContainer)();	/* de-init a container (before deallocation) */
    int			(*Break)();		/* request a break on this container (e.g., pagebreak) */
    int			(*ChildBreakReq)();	/* child object is requesting this container to break */
    int			(*Resize)();		/* request a resize on this container */
    int			(*SetValue)();		/* set a layout manager specific setting on the object */
    int			(*Reflow)();		/* called by parent object's layout mgr to indicate that
						   child geom or overlays have changed, and child should
						   reflow the layout, including LinkNext'd layout areas */
    int			(*Finalize)();		/* this is the layout manager's chance to make any last-
						   milllisecond changes to the container before the page
						   is generated.  The changes may NOT affect the container's
						   geometry */
    }
    PrtLayoutMgr, *pPrtLayoutMgr;


/*** Print Object Type structure ***/
typedef struct _POT
    {
    int			Magic;
    int			TypeID;			/* PRT_OBJ_T_xxx */
    char		TypeName[32];
    pPrtLayoutMgr	PrefLayoutMgr;
    }
    PrtObjType, *pPrtObjType;


/*** Text style definition ***/
typedef struct _PTS
    {
    int			FontID;			/* ID from PRTMGMT.FontList mapping */
    int			FontSize;		/* point size of font */
    int			Attr;			/* PRT_OBJ_A_xxx */
    int			Color;			/* 0x00RRGGBB */
    }
    PrtTextStyle, *pPrtTextStyle;


/*** Print Object Stream structure ***/
typedef struct _POS
    {
    int			Magic;
    struct _POS*	Next;			/* next sequentially in stream */
    struct _POS*	Prev;			/* previous sequentially in stream */
    struct _POS*	ContentHead;		/* Contents of this container */
    struct _POS*	ContentTail;
    struct _POS*	Parent;			/* Container of this object */
    struct _POS*	LinkNext;		/* Pointer to object where content should continue */
    struct _POS*	LinkPrev;		/* Pointer to object where content continued from */
    struct _POS*	YNext;
    struct _POS*	YPrev;
    pPrtObjType		ObjType;
    pPrtLayoutMgr	LayoutMgr;
    void*		Session;		/* pPrtObjSession */
    void*		LMData;			/* layout manager instance-specific data */
    int			nOpens;			/* total open subcontainers + container */
    int			Flags;			/* PRT_OBJ_F_xxx */
    int			BGColor;		/* 0x00RRGGBB */
    int			FGColor;		/* 0x00RRGGBB */
    int			ObjID;			/* such as page number for PAGE objects */
    PrtTextStyle	TextStyle;
    int			Justification;		/* PRT_JUST_T_xxx */
    double		PageX;
    double		PageY;
    double		X;			/* Relative X position to container origin */
    double		Y;			/* Relative Y position to container origin */
    double		YBase;			/* Relative Y baseline to the object's Y position */
    double		Width;			/* Width of object */
    double		Height;			/* Height of object */
    double		ConfigWidth;		/* initially configured width of object */
    double		ConfigHeight;		/* initially configured height of object */
    double		MarginLeft;		/* user-configured margins starting at inner border edge */
    double		MarginRight;
    double		MarginTop;
    double		MarginBottom;
    double		BorderLeft;		/* not a spec for borders, just the width */
    double		BorderRight;
    double		BorderTop;
    double		BorderBottom;
    double		LineHeight;		/* Height of lines... */
    double		ConfigLineHeight;	/* Configured height of lines, negative if unset. */
    unsigned char*	Content;		/* Text content or image bitmap */
    int			ContentSize;		/* total memory allocated for the content */
    char		DataType;		/* type of data displayed in this object (Hints) */
    }
    PrtObjStream, *pPrtObjStream;

#define PRTSESSION(x) ((pPrtSession)((x)->Session))


/*** Units-of-measure conversion data ***/
typedef struct _PU
    {
    char		Name[32];
    double		AdjX;
    double		AdjY;
    }
    PrtUnits, *pPrtUnits;


/*** Font ID descriptor ***/
typedef struct _PFN
    {
    int			FontID;
    char		FontName[40];
    }
    PrtFontDesc, *pPrtFontDesc;


/*** Resolution definition structure ***/
typedef struct _PRE
    {
    int			Xres;
    int			Yres;
    int			Colors;		/* PRT_COLOR_T_xxx */
    }
    PrtResolution, *pPrtResolution;


/*** Formatter structure ***/
typedef struct _PFM
    {
    int			Magic;
    char		Name[32];
    int			Priority;
    void*		(*Probe)();
    int			(*Generate)();
    int			(*GetNearestFontSize)();
    void		(*GetCharacterMetric)();
    double		(*GetCharacterBaseline)();
    int			(*Close)();
    }
    PrtFormatter, *pPrtFormatter;


/*** Output Driver structure ***/
typedef struct _PD
    {
    int			Magic;
    char		Name[32];
    char		ContentType[64];
    void*		(*Open)();
    int			(*Close)();
    pXArray		(*GetResolutions)();
    int			(*SetResolution)();
    int			(*SetPageGeom)();
    int			(*SetTextStyle)();
    int			(*GetNearestFontSize)();
    void		(*GetCharacterMetric)();
    double		(*GetCharacterBaseline)();
    int			(*SetHPos)();
    int			(*SetVPos)();
    int			(*WriteText)();
    double		(*WriteRasterData)();
    int			(*WriteFF)();
    double		(*WriteRect)();
    }
    PrtOutputDriver, *pPrtOutputDriver;


/*** Scheduled event structure ***/
typedef struct _PE
    {
    struct _PE*		Next;
    int			EventType;
    pPrtObjStream	TargetObject;
    void*		Parameter;
    }
    PrtEvent, *pPrtEvent;


/*** Print Session structure ***/
typedef struct _PS
    {
    int			Magic;
    pPrtObjStream	StreamHead;
    double		PageWidth;
    double		PageHeight;
    pPrtUnits		Units;
    int			(*WriteFn)();
    void*		WriteArg;
    pPrtFormatter	Formatter;
    int			ResolutionX;	/* in DPI, always */
    int			ResolutionY;	/* in DPI, always */
    void*		FormatterData;
    int			FocusHandle;
    pPrtEvent		PendingEvents;
    char		ImageExtDir[256];
    char		ImageSysDir[256];
    void*		ImageContext;
    void*		(*ImageOpenFn)();
    void*		(*ImageWriteFn)();
    void*		(*ImageCloseFn)();
    }
    PrtSession, *pPrtSession;


/*** Print Object Handle structure ***/
typedef struct _PHD
    {
    int			Magic;
    int			HandleID;
    union
        {
	pPrtObjStream	Object;
	pPrtSession	Session;
	void*		Generic;
	}
	Ptr;
    }
    PrtHandle, *pPrtHandle;


#define PRT_MAXBDR	2

/*** border line data, such as for tables, etc. ***/
typedef struct _PBD
    {
    double		Width[PRT_MAXBDR]; /* width/thickness of border lines in X units (0.1in / 7.2pt) */
    double		Sep;		   /* separation between border lines, if two */
    double		Pad;		   /* padding before lines start */
    double		TotalWidth;	   /* total width of border */
    int			nLines;		   /* number of border lines */
    int			Color[PRT_MAXBDR]; /* color of border lines */
    }
    PrtBorder, *pPrtBorder;


/*** image data header structure ***/
typedef struct _PIH
    {
    int			Width;		    /* raster data width in pixels */
    int			Height;		    /* raster data height in pixels */
    int			ColorMode;	    /* PRT_COLOR_T_xxx; mono=1bpp, grey=8bpp, color=32bpp */
    int			DataLength;	    /* length of raster data in bytes */
    double		YOffset;	    /* offset into image next printed area is. Used by output drv. */
    }
    PrtImageHdr, *pPrtImageHdr;

typedef struct _PIM
    {
    PrtImageHdr		Hdr;
    union
	{
	unsigned char	Byte[1];	    /* content in byte-addressing */
	unsigned int	Word[1];	    /* content in word (32bit) addressing */
	}		
	Data;
    }
    PrtImage, *pPrtImage;


/*** Print management global structure ***/
typedef struct _PG
    {
    XArray		UnitsList;
    XArray		TypeList;
    XArray		LayoutMgrList;
    XArray		FormatterList;
    XArray		FormatDriverList;
    XArray		FontList;
    XHashTable		HandleTable;
    XHashTable		HandleTableByPtr;
    int			NextHandleID;
    }
    PrtGlobals, *pPrtGlobals;

extern PrtGlobals PRTMGMT;


#define PRT_OBJ_F_FLOWAROUND	    1		/* multiple lines can flow around this */
#define PRT_OBJ_F_NOLINESEQ	    2		/* do not honor sequence within the line */
#define PRT_OBJ_F_OPEN     	    4		/* container is open for writing */
#define PRT_OBJ_F_NEWLINE  	    8		/* object starts a new line (for textflow) */
#define PRT_OBJ_F_SOFTNEWLINE	    16		/* object starts a soft new line (from wordwrapping) */
#define PRT_OBJ_F_ALLOWHARDBREAK    32		/* allow object to be broken for pagination via WriteFF */
#define PRT_OBJ_F_REPEAT	    64		/* object repeats *verbatim* on subsequent pages */
#define PRT_OBJ_F_CALLBACK	    128		/* do an API callback to get the object's content */
#define PRT_OBJ_F_FIXEDSIZE	    256		/* object's size cannot be resized */
#define PRT_OBJ_F_REQCOMPLETE	    512		/* require completion of content before emitting the page */
#define PRT_OBJ_F_XSET		    1024	/* X location is pre-set. */
#define PRT_OBJ_F_YSET		    2048	/* Y location is pre-set. */
#define PRT_OBJ_F_ALLOWSOFTBREAK    4096	/* allow break for pagination via wrapping or newlines */
#define PRT_OBJ_F_ALLOWBREAK	    (PRT_OBJ_F_ALLOWSOFTBREAK | PRT_OBJ_F_ALLOWHARDBREAK)
#define PRT_OBJ_F_SYNCBREAK	    8192	/* container will break when another on the page does */
#define PRT_OBJ_F_LMFLAG1	    16384	/* layout-manager specific flag */
#define PRT_OBJ_F_LMFLAG2	    32768	/* layout-manager specific flag */
#define PRT_OBJ_F_LMFLAG3	    65536	/* layout-manager specific flag */
#define PRT_OBJ_F_PERMANENT	    131072	/* object is permanent - don't remove it to reflow */
#define PRT_OBJ_F_MARGINRELEASE	    262144	/* object is not subject to container's margins */

/** these flags can be used by the api caller **/
#define PRT_OBJ_U_FLOWAROUND	    PRT_OBJ_F_FLOWAROUND
#define PRT_OBJ_U_NOLINESEQ	    PRT_OBJ_F_NOLINESEQ
#define PRT_OBJ_U_ALLOWBREAK	    PRT_OBJ_F_ALLOWBREAK
#define PRT_OBJ_U_ALLOWHARDBREAK    PRT_OBJ_F_ALLOWHARDBREAK
#define PRT_OBJ_U_ALLOWSOFTBREAK    PRT_OBJ_F_ALLOWSOFTBREAK
#define PRT_OBJ_U_REPEAT	    PRT_OBJ_F_REPEAT
#define PRT_OBJ_U_FIXEDSIZE	    PRT_OBJ_F_FIXEDSIZE
#define PRT_OBJ_U_REQCOMPLETE	    PRT_OBJ_F_REQCOMPLETE
#define PRT_OBJ_U_XSET		    PRT_OBJ_F_XSET
#define PRT_OBJ_U_YSET		    PRT_OBJ_F_YSET
#define PRT_OBJ_U_SYNCBREAK	    PRT_OBJ_F_SYNCBREAK

#define PRT_OBJ_UFLAGMASK	    (PRT_OBJ_U_FLOWAROUND | PRT_OBJ_U_NOLINESEQ | PRT_OBJ_U_ALLOWBREAK | PRT_OBJ_U_REPEAT | PRT_OBJ_U_FIXEDSIZE | PRT_OBJ_U_REQCOMPLETE | PRT_OBJ_U_XSET | PRT_OBJ_U_YSET | PRT_OBJ_U_SYNCBREAK)

#define PRT_OBJ_A_BOLD		    1
#define PRT_OBJ_A_ITALIC	    2
#define PRT_OBJ_A_UNDERLINE	    4

#define PRT_OBJ_T_DOCUMENT	    1		/* whole document */
#define PRT_OBJ_T_PAGE		    2		/* one page of the document */
#define PRT_OBJ_T_AREA		    3		/* a textflow-managed area */
#define PRT_OBJ_T_STRING	    4		/* a string of text content */
#define PRT_OBJ_T_IMAGE		    5		/* an image/picture */
#define PRT_OBJ_T_RECT		    6		/* rectangle, solid color */
#define PRT_OBJ_T_TABLE		    7		/* tabular-data */
#define PRT_OBJ_T_TABLEROW	    8		/* table row */
#define PRT_OBJ_T_TABLECELL	    9		/* table cell */
#define PRT_OBJ_T_SECTION	    10		/* columnar section */
#define PRT_OBJ_T_SECTCOL	    11		/* one column in a section. */

#define PRT_COLOR_T_MONO	    1		/* black/white only image, 1 bit per pixel */
#define PRT_COLOR_T_GREY	    2		/* greyscale image, 1 byte per pixel */
#define PRT_COLOR_T_FULL	    3		/* RGB color image, 1 int (32bit) per pixel */

#define PRT_FONT_T_MONOSPACE	    1		/* typically courier */
#define PRT_FONT_T_SANSSERIF	    2		/* typically helvetica */
#define PRT_FONT_T_SERIF	    3		/* typically times */
#define PRT_FONT_T_USBARCODE	    4		/* u.s.a. postal barcode */

#define PRT_JUST_T_LEFT		    0
#define PRT_JUST_T_RIGHT	    1
#define PRT_JUST_T_CENTER	    2
#define PRT_JUST_T_FULL		    3

#define PRT_EVENT_T_REFLOW	    0		/* reflow the contents of a container */


/*** MakeBorder flags ***/
#define PRT_MKBDR_F_TOP		    1		/* border is 'top' */
#define PRT_MKBDR_F_BOTTOM	    2
#define PRT_MKBDR_F_LEFT	    4
#define PRT_MKBDR_F_RIGHT	    8
#define PRT_MKBDR_DIRFLAGS	    (PRT_MKBDR_F_TOP | PRT_MKBDR_F_BOTTOM | PRT_MKBDR_F_LEFT | PRT_MKBDR_F_RIGHT)
#define PRT_MKBDR_F_MARGINRELEASE   16


/*** System functions ***/
int prtInitialize();
int prtRegisterUnits(char* units_name, double x_adj, double y_adj);
pPrtObjType prtAllocType();
int prtRegisterType(pPrtObjType type);
pPrtLayoutMgr prtAllocLayoutMgr();
int prtRegisterLayoutMgr(pPrtLayoutMgr layout_mgr);
pPrtLayoutMgr prtLookupLayoutMgr(char* layout_mgr);
pPrtUnits prtLookupUnits(char* units_name);
int prtRegisterFont(char* font_name, int font_id);
int prtLookupFont(char* font_name);
char* prtLookupFontName(int font_id);
int prtAllocHandle(void* ptr);
void* prtHandlePtr(int handle_id);
int prtUpdateHandle(int handle_id, void* ptr);
int prtUpdateHandleByPtr(void* old_ptr, void* ptr);
int prtFreeHandle(int handle_id);
int prtLookupHandle(void* ptr);
pPrtFormatter prtAllocFormatter();
int prtRegisterFormatter(pPrtFormatter fmt);

/*** Session-level functions ***/
pPrtSession prtOpenSession(char* output_type, int (*write_fn)(), void* write_arg, int page_flags);
int prtCloseSession(pPrtSession s);
int prtSetPageGeometry(pPrtSession s, double width, double height);
int prtGetPageGeometry(pPrtSession s, double *width, double *height);
int prtSetUnits(pPrtSession s, char* units_name);
char* prtGetUnits(pPrtSession s);
double prtGetUnitsRatio(pPrtSession s);
int prtSetResolution(pPrtSession s, int dpi);
int prtSetImageStore(pPrtSession s, char* extdir, char* sysdir, void* open_ctx, void* (*open_fn)(), void* (*write_fn)(), void* (*close_fn)());

/** Internal management functions **/
pPrtObjStream prt_internal_AllocObj(char* type);
pPrtObjStream prt_internal_AllocObjByID(int type_id);
int prt_internal_FreeObj(pPrtObjStream obj);
int prt_internal_Add(pPrtObjStream parent, pPrtObjStream new_child);
int prt_internal_Insert(pPrtObjStream sibling, pPrtObjStream new_obj);
int prt_internal_CopyAttrs(pPrtObjStream src, pPrtObjStream dst);
int prt_internal_CopyGeom(pPrtObjStream src, pPrtObjStream dst);
double prt_internal_GetFontHeight(pPrtObjStream obj);
double prt_internal_FontToLineHeight(pPrtObjStream obj);
double prt_internal_GetFontBaseline(pPrtObjStream obj);
double prt_internal_GetStringWidth(pPrtObjStream obj, char* str, int n);
pPrtObjStream prt_internal_YSort(pPrtObjStream obj);
int prt_internal_AddYSorted(pPrtObjStream obj, pPrtObjStream newobj);
int prt_internal_FreeTree(pPrtObjStream obj);
int prt_internal_GeneratePage(pPrtSession s, pPrtObjStream page);
pPrtObjStream prt_internal_GetPage(pPrtObjStream obj);
pPrtObjStream prt_internal_AddEmptyObj(pPrtObjStream container);
pPrtObjStream prt_internal_CreateEmptyObj(pPrtObjStream container);
int prt_internal_Dump(pPrtObjStream obj);
pPrtObjStream prt_internal_Duplicate(pPrtObjStream obj, int with_content);
int prt_internal_AdjustOpenCount(pPrtObjStream obj, int adjustment);
int prt_internal_Reflow(pPrtObjStream obj);
int prt_internal_ScheduleEvent(pPrtSession s, pPrtObjStream target, int type, void* param);
int prt_internal_DispatchEvents(pPrtSession s);
int prt_internal_MakeBorder(pPrtObjStream parent, double x, double y, double len, int flags, pPrtBorder b, pPrtBorder sb, pPrtBorder eb);
int prt_internal_GetPixel(pPrtImage img, double xoffset, double yoffset);
double prtInnerWidth(pPrtObjStream obj);
double prtInnerHeight(pPrtObjStream obj);

/** Strict-formatter to output-driver interfacing **/
pPrtOutputDriver prt_strictfm_AllocDriver();
int prt_strictfm_RegisterDriver(pPrtOutputDriver drv);

/** These macros are used for units conversion **/
#define prtUnitX(s,x) ((x)*(((pPrtSession)(s))->Units->AdjX))
#define prtUnitY(s,y) ((y)*(((pPrtSession)(s))->Units->AdjY))
#define prtUsrUnitX(s,x) ((x)/(((pPrtSession)(s))->Units->AdjX))
#define prtUsrUnitY(s,y) ((y)/(((pPrtSession)(s))->Units->AdjY))

/*** General Printing Functions ***/
int prtGetPageRef(pPrtSession s);
int prtSetFocus(pPrtSession s, int handle_id);
int prtSetPageNumber(int handle_id, int new_pagenum);
int prtGetPageNumber(int handle_id);

/*** Formatting functions ***/
int prtSetJustification(int handle_id, int just_mode);	/* PRT_JUST_T_xxx */
int prtGetJustification(int handle_id);
int prtSetLineHeight(int handle_id,double line_height);
double prtGetLineHeight(int handle_id);
int prtSetTextStyle(int handle_id, pPrtTextStyle style);
int prtGetTextStyle(int handle_id, pPrtTextStyle *style);
int prtSetAttr(int handle_id, int attrs);		/* PRT_OBJ_A_xxx */
int prtGetAttr(int handle_id);
int prtSetFont(int handle_id, char* fontname);
char* prtGetFont(int handle_id);
int prtSetFontSize(int handle_id, int pt_size);
int prtGetFontSize(int handle_id);
int prtSetColor(int handle_id, int font_color);
int prtGetColor(int handle_id);
int prtSetHPos(int handle_id, double x);
int prtSetVPos(int handle_id, double y);
int prtSetValue(int handle_id, char* attrname, ...);
int prtSetMargins(int handle_id, double t, double b, double l, double r);
pPrtBorder prtAllocBorder(int n_lines, double sep, double pad, ...);
int prtFreeBorder(pPrtBorder b);
pPrtImage prtCreateImageFromPNG(int (*read_fn)(), void* read_arg);
int prtWriteImageToPNG(int (*write_fn)(), void* write_arg, pPrtImage img, int w, int h);
int prtFreeImage(pPrtImage i);
int prtImageSize(pPrtImage i);
int prtSetDataHints(int handle_id, int data_type, int flags);

/*** Printing content functions ***/
int prtWriteImage(int handle_id, pPrtImage imgdata, double x, double y, double width, double height, int flags);
int prtWriteString(int handle_id, char* str);
int prtWriteNL(int handle_id);
int prtWriteFF(int handle_id);

/*** Print object creation functions ***/
int prtAddObject(int handle_id, int obj_type, double x, double y, double width, double height, int flags, ...);
int prtSetObjectCallback(int handle_id, void* (*callback_fn)(), int is_pre);
int prtEndObject(int handle_id);


#endif /* defined _PRTMGMT_V3_H */

