#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ht_render.h"
#include "obj.h"
#include "mtask.h"
#include "xarray.h"
#include "xhash.h"
#include "mtsession.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1998-2003 LightSys Technology Services, Inc.		*/
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
/* Module: 	htdrv_scrollbar.c					*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	July 14, 2003    					*/
/* Description:	HTML Widget driver for a scrollbar - either horizontal	*/
/*		or vertical.						*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: htdrv_scrollbar.c,v 1.2 2003/07/15 01:59:50 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/htdrv_scrollbar.c,v $

    $Log: htdrv_scrollbar.c,v $
    Revision 1.2  2003/07/15 01:59:50  gbeeley
    Fixing bug with the low-value limiter on dragging the thumb around.

    Revision 1.1  2003/07/15 01:57:51  gbeeley
    Adding an independent DHTML scrollbar widget that will be used to
    control scrolling/etc on other widgets.

 **END-CVSDATA***********************************************************/

/** globals **/
static struct 
    {
    int		idcnt;
    }
    HTSB;


/*** htsbVerify - not written yet.
 ***/
int
htsbVerify()
    {
    return 0;
    }


/*** htsbRender - generate the HTML code for the page.
 ***/
int
htsbRender(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj)
    {
    char* ptr;
    char name[64];
    char sbuf[160];
    pObject sub_w_obj;
    pObjQuery qy;
    int x,y,w,h,r;
    int id;
    int visible = 1;
    char* nptr;
    char bcolor[64] = "";
    char bimage[64] = "";
    int is_horizontal = 0;
    pExpression code;

	if(!s->Capabilities.Dom0NS)
	    {
	    mssError(1,"HTSB","Netscape 4.x DOM support required");
	    return -1;
	    }

    	/** Get an id for this. **/
	id = (HTSB.idcnt++);

	/** Which direction? **/
	if (objGetAttrValue(w_obj,"direction",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    if (!strcasecmp(ptr,"vertical")) is_horizontal = 0;
	    else if (!strcasecmp(ptr,"horizontal")) is_horizontal = 1;
	    }

    	/** Get x,y,w,h of this object **/
	if (objGetAttrValue(w_obj,"x",DATA_T_INTEGER,POD(&x)) != 0) 
	    {
	    mssError(1,"HTSB","Scrollbar widget must have an 'x' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"y",DATA_T_INTEGER,POD(&y)) != 0)
	    {
	    mssError(1,"HTSB","Scrollbar widget must have a 'y' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"width",DATA_T_INTEGER,POD(&w)) != 0)
	    {
	    if (is_horizontal)
		{
		mssError(1,"HTSB","Horizontal scrollbar widgets must have a 'width' property");
		return -1;
		}
	    else
		{
		w = 18;
		}
	    }
	if (is_horizontal && w <= 18*3)
	    {
	    mssError(1,"HTSB","Horizontal scrollbar width must be greater than %d", 18*3);
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"height",DATA_T_INTEGER,POD(&h)) != 0)
	    {
	    if (!is_horizontal)
		{
		mssError(1,"HTSB","Vertical scrollbar widgets must have a 'height' property");
		return -1;
		}
	    else
		{
		h = 18;
		}
	    }
	if (!is_horizontal && h <= 18*3)
	    {
	    mssError(1,"HTSB","Vertical scrollbar height must be greater than %d", 18*3);
	    return -1;
	    }

	/** Get name **/
	if (objGetAttrValue(w_obj,"name",DATA_T_STRING,POD(&ptr)) != 0) return -1;
	memccpy(name,ptr,'\0',63);
	name[63]=0;

	/** Range of scrollbar (static or dynamic property) **/
	if (objGetAttrType(w_obj,"range") == DATA_T_INTEGER && objGetAttrValue(w_obj,"range",DATA_T_INTEGER,POD(&r)) != 0)
	    {
	    if (is_horizontal) r = w;
	    else r = h;
	    }
	if (objGetAttrType(w_obj,"range") == DATA_T_CODE)
	    {
	    objGetAttrValue(w_obj,"range", DATA_T_CODE, POD(&code));
	    htrAddExpression(s, name, "range", code);
	    }

	/** Check background color **/
	if (objGetAttrValue(w_obj,"bgcolor",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    memccpy(bcolor,ptr,'\0',63);
	    bcolor[63]=0;
	    }
	if (objGetAttrValue(w_obj,"background",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    memccpy(bimage,ptr,'\0',63);
	    bimage[63]=0;
	    }

	/** Marked not visible? **/
	if (objGetAttrValue(w_obj,"visible",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    if (!strcmp(ptr,"false")) visible = 0;
	    }

	/** Ok, write the style header items. **/
	htrAddStylesheetItem_va(s,"\t#sb%dpane { POSITION:absolute; VISIBILITY:%s; LEFT:%d; TOP:%d; WIDTH:%d; HEIGHT:%d; clip:rect(%d,%d); Z-INDEX:%d; }\n",id,visible?"inherit":"hidden",x,y,w,h,w,h, z);
	if (is_horizontal)
	    htrAddStylesheetItem_va(s,"\t#sb%dthum { POSITION:absolute; VISIBILITY:inherit; LEFT:18; TOP:0; WIDTH:18; Z-INDEX:%d; }\n",id,z+1);
	else
	    htrAddStylesheetItem_va(s,"\t#sb%dthum { POSITION:absolute; VISIBILITY:inherit; LEFT:0; TOP:18; WIDTH:18; Z-INDEX:%d; }\n",id,z+1);

	/** Write globals for internal use **/
	htrAddScriptGlobal(s, "sb_target_img", "null", 0);
	htrAddScriptGlobal(s, "sb_click_x","0",0);
	htrAddScriptGlobal(s, "sb_click_y","0",0);
	htrAddScriptGlobal(s, "sb_thum_x","0",0);
	htrAddScriptGlobal(s, "sb_thum_y","0",0);
	htrAddScriptGlobal(s, "sb_mv_timeout","null",0);
	htrAddScriptGlobal(s, "sb_mv_incr","0",0);
	htrAddScriptGlobal(s, "sb_cur_mainlayer","null",0);

	/** Write named global **/
	nptr = (char*)nmMalloc(strlen(name)+1);
	strcpy(nptr,name);
	htrAddScriptGlobal(s, nptr, "null", HTR_F_NAMEALLOC);

	htrAddScriptInclude(s, "/sys/js/htdrv_scrollbar.js", 0);
	htrAddScriptInclude(s, "/sys/js/ht_utils_string.js", 0);

	/** Script initialization call. **/
	htrAddScriptInit_va(s,"    %s=sb_init(%s.layers.sb%dpane,\"sb%dthum\",%s,%d,%d);\n", name, parentname,id,id,parentobj,is_horizontal,r);

	/** HTML body <DIV> elements for the layers. **/
	htrAddBodyItem_va(s,"<DIV ID=\"sb%dpane\"><TABLE %s%s %s%s border=0 cellspacing=0 cellpadding=0 width=%d>",id,(*bcolor)?"bgcolor=":"",bcolor, (*bimage)?"background=":"",bimage, w);
	if (is_horizontal)
	    {
	    htrAddBodyItem_va(s,"<TR><TD align=right><IMG SRC=/sys/images/ico19b.gif width=18 height=18 NAME=u></TD><TD align=right>");
	    htrAddBodyItem_va(s,"<IMG SRC=/sys/images/trans_1.gif height=18 width=%d name='b'>",w-36);
	    htrAddBodyItem_va(s,"</TD><TD align=right><IMG SRC=/sys/images/ico18b.gif width=18 height=18 NAME=d></TD></TR></TABLE>\n");
	    }
	else
	    {
	    htrAddBodyItem_va(s,"<TR><TD align=right><IMG SRC=/sys/images/ico13b.gif width=18 height=18 NAME=u></TD></TR><TR><TD align=right>");
	    htrAddBodyItem_va(s,"<IMG SRC=/sys/images/trans_1.gif height=%d width=18 name='b'>",h-36);
	    htrAddBodyItem_va(s,"</TD></TR><TR><TD align=right><IMG SRC=/sys/images/ico12b.gif width=18 height=18 NAME=d></TD></TR></TABLE>\n");
	    }
	htrAddBodyItem_va(s,"<DIV ID=\"sb%dthum\"><IMG SRC=/sys/images/ico14b.gif NAME=t></DIV>",id);

	/** Add the event handling scripts **/

	htrAddEventHandler(s, "document","MOUSEDOWN","sb",
		"    sb_target_img=e.target;\n"
		"    if (sb_target_img != null && sb_target_img.kind=='sb' && (sb_target_img.name=='u' || sb_target_img.name=='d'))\n"
		"        {\n"
		"        if (sb_target_img.name=='u') sb_mv_incr=-10; else sb_mv_incr=+10;\n"
		"        sb_target_img.src = htutil_subst_last(sb_target_img.src,\"c.gif\");\n"
		"        sb_do_mv();\n"
		"        sb_mv_timeout = setTimeout(sb_tm_mv,300);\n"
		"        }\n"
		"    else if (sb_target_img != null && sb_target_img.kind=='sb' && sb_target_img.name=='t')\n"
		"        {\n"
		"        sb_click_x = e.pageX;\n"
		"        sb_click_y = e.pageY;\n"
		"        sb_thum_x = sb_target_img.thum.pageX;\n"
		"        sb_thum_y = sb_target_img.thum.pageY;\n"
		"        }\n"
		"    else if (sb_target_img != null && sb_target_img.kind=='sb' && sb_target_img.name=='b')\n"
		"        {\n"
		"        sb_mv_incr=sb_target_img.mainlayer.controlsize + (18*3);\n"
		"        if (!sb_target_img.mainlayer.is_horizontal && e.pageY < sb_target_img.thum.pageY+9) sb_mv_incr = -sb_mv_incr;\n"
		"        if (sb_target_img.mainlayer.is_horizontal && e.pageX < sb_target_img.thum.pageX+9) sb_mv_incr = -sb_mv_incr;\n"
		"        sb_do_mv();\n"
		"        sb_mv_timeout = setTimeout(sb_tm_mv,300);\n"
		"        }\n"
		"    else sb_target_img = null;\n"
		"    if (ly.kind == 'sb') cn_activate(ly.mainlayer, 'MouseDown');");
	htrAddEventHandler(s, "document","MOUSEMOVE","sb",
		"    var ti=sb_target_img;\n"
		"    if (ti != null && ti.kind=='sb' && ti.name=='t')\n"
		"        {\n"
		"        if (ti.mainlayer.is_horizontal)\n"
		"            {\n"
		"            var new_x=sb_thum_x + (e.pageX-sb_click_x);\n"
		"            if (new_x > ti.pane.pageX+18+ti.mainlayer.controlsize) new_x=ti.pane.pageX+18+ti.mainlayer.controlsize;\n"
		"            if (new_x < ti.pane.pageX+18) new_x=ti.pane.pageX+18;\n"
		"            ti.thum.pageX=new_x;\n"
		"            ti.mainlayer.value = Math.round((new_x - (ti.pane.pageX+18))*ti.mainlayer.range/ti.mainlayer.controlsize);\n"
		"            }\n"
		"        else\n"
		"            {\n"
		"            var new_y=sb_thum_y + (e.pageY-sb_click_y);\n"
		"            if (new_y > ti.pane.pageY+18+ti.mainlayer.controlsize) new_y=ti.pane.pageY+18+ti.mainlayer.controlsize;\n"
		"            if (new_y < ti.pane.pageY+18) new_y=ti.pane.pageY+18;\n"
		"            ti.thum.pageY=new_y;\n"
		"            ti.mainlayer.value = Math.round((new_y - (ti.pane.pageY+18))*ti.mainlayer.range/ti.mainlayer.controlsize);\n"
		"            }\n"
		"        return false;\n"
		"        }\n"
		"    if (ly.kind == 'sb') cn_activate(ly.mainlayer, 'MouseMove');\n"
		"    if (sb_cur_mainlayer && ly.kind != 'sb')\n"
		"        {\n"
		"        cn_activate(sb_cur_mainlayer, 'MouseOut');\n"
		"        sb_cur_mainlayer = null;\n"
		"        }\n");
	htrAddEventHandler(s, "document","MOUSEUP","sb",
		"    if (sb_mv_timeout != null)\n"
		"        {\n"
		"        clearTimeout(sb_mv_timeout);\n"
		"        sb_mv_timeout = null;\n"
		"        sb_mv_incr = 0;\n"
		"        }\n"
		"    if (sb_target_img != null)\n"
		"        {\n"
		"        if (sb_target_img.name != 'b')\n"
		"            sb_target_img.src = htutil_subst_last(sb_target_img.src,\"b.gif\");\n"
		"        sb_target_img = null;\n"
		"        }\n"
		"    if (ly.kind == 'sb') cn_activate(ly.mainlayer, 'MouseUp');\n");
	htrAddEventHandler(s, "document", "MOUSEOVER", "sb",
		"    if (ly.kind == 'sb')\n"
		"        {\n"
		"        if (!sb_cur_mainlayer)\n"
		"            {\n"
		"            cn_activate(ly.mainlayer, 'MouseOver');\n"
		"            sb_cur_mainlayer = ly.mainlayer;\n"
		"            }\n"
		"        }\n");

	/** Check for more sub-widgets within the scrollbar (visual ones not allowed). **/
	qy = objOpenQuery(w_obj,"",NULL,NULL,NULL);
	snprintf(sbuf,160,"%s.document",name);
	if (qy)
	    {
	    while((sub_w_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		htrRenderWidget(s, sub_w_obj, z+2, sbuf, name);
		objClose(sub_w_obj);
		}
	    objQueryClose(qy);
	    }

	/** Finish off the last <DIV> **/
	htrAddBodyItem(s,"</DIV>\n");

    return 0;
    }


/*** htsbInitialize - register with the ht_render module.
 ***/
int
htsbInitialize()
    {
    pHtDriver drv;

    	/** Allocate the driver **/
	drv = htrAllocDriver();
	if (!drv) return -1;

	/** Fill in the structure. **/
	strcpy(drv->Name,"DHTML Scrollbar Widget Driver");
	strcpy(drv->WidgetName,"scrollbar");
	drv->Render = htsbRender;
	drv->Verify = htsbVerify;

	/** Events **/ 
	htrAddEvent(drv,"Click");
	htrAddEvent(drv,"MouseUp");
	htrAddEvent(drv,"MouseDown");
	htrAddEvent(drv,"MouseOver");
	htrAddEvent(drv,"MouseOut");
	htrAddEvent(drv,"MouseMove");

	htrAddAction(drv,"MoveTo");
	htrAddParam(drv,"MoveTo","Value",DATA_T_INTEGER);

	/** Register. **/
	htrRegisterDriver(drv);

	htrAddSupport(drv, "dhtml");
	HTSB.idcnt = 0;

    return 0;
    }