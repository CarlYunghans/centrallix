#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ht_render.h"
#include "obj.h"
#include "mtask.h"
#include "xarray.h"
#include "xhash.h"
#include "stparse.h"
#include "mtsession.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1999-2001 LightSys Technology Services, Inc.		*/
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
/*									*/
/* Module: 	htdrv_table.c           				*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	October 29, 1999 					*/
/* Description:	HTML Widget driver for a data-driven table.  Has three	*/
/*		different modes -- static, dynamicpage, and dynamicrow.	*/
/*									*/
/*		Static means an inline table that can't be updated	*/
/*		without a parent container being completely reloaded.	*/
/*		DynamicPage means a table in a layer that can be	*/
/*		reloaded dynamically as a whole when necessary.  Good	*/
/*		when you need forward/back without reloading the page.	*/
/*		DynamicRow means each row is its own layer.  Good when	*/
/*		you need to insert rows dynamically and delete rows	*/
/*		dynamically at the client side without reloading the	*/
/*		whole table contents.					*/
/*									*/
/*		A static table's query is performed on the server side	*/
/*		and the HTML is generated at the server.  Both dynamic	*/
/*		types are built from a client-side query.  Static 	*/
/*		tables are generally best when the data will be read-	*/
/*		only.  Dynamicrow tables use the most client resources.	*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: htdrv_table.c,v 1.12 2002/05/30 03:55:21 lkehresman Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/htdrv_table.c,v $

    $Log: htdrv_table.c,v $
    Revision 1.12  2002/05/30 03:55:21  lkehresman
    editbox:  * added readonly flag so the editbox is always only readonly
              * made disabled appear visually
    table:    * fixed a typo

    Revision 1.11  2002/05/01 02:25:50  jorupp
     * more changes

    Revision 1.10  2002/04/30 18:08:43  jorupp
     * more additions to the table -- now it can scroll~
     * made the osrc and form play nice with the table
     * minor changes to form sample

    Revision 1.9  2002/04/28 00:30:53  jorupp
     * full sorting support added to table

    Revision 1.8  2002/04/27 22:47:45  jorupp
     * re-wrote form and osrc interaction -- more happens now in the form
     * lots of fun stuff in the table....check form.app for an example (not completely working yet)
     * the osrc is still a little bit buggy.  If you get it screwed up, let me know how to reproduce it.

    Revision 1.7  2002/04/27 18:42:22  jorupp
     * the dynamicrow table widget will now change the current osrc row when you click a row...

    Revision 1.6  2002/04/27 06:37:45  jorupp
     * some bug fixes in the form
     * cleaned up some debugging output in the label
     * added a dynamic table widget

    Revision 1.5  2002/03/09 19:21:20  gbeeley
    Basic security overhaul of the htmlgen subsystem.  Fixed many of my
    own bad sprintf habits that somehow worked their way into some other
    folks' code as well ;)  Textbutton widget had an inadequate buffer for
    the tb_init() call, causing various problems, including incorrect labels,
    and more recently, javascript errors.

    Revision 1.4  2001/11/03 02:09:54  gbeeley
    Added timer nonvisual widget.  Added support for multiple connectors on
    one event.  Added fades to the html-area widget.  Corrected some
    pg_resize() geometry issues.  Updated several widgets to reflect the
    connector widget changes.

    Revision 1.3  2001/10/23 00:25:09  gbeeley
    Added rudimentary single-line editbox widget.  No data source linking
    or anything like that yet.  Fixed a few bugs and made a few changes to
    other controls to make this work more smoothly.  Page widget still needs
    some key de-bounce and key repeat overhaul.  Arrow keys don't work in
    Netscape 4.xx.

    Revision 1.2  2001/10/16 23:53:01  gbeeley
    Added expressions-in-structure-files support, aka version 2 structure
    files.  Moved the stparse module into the core because it now depends
    on the expression subsystem.  Almost all osdrivers had to be modified
    because the structure file api changed a little bit.  Also fixed some
    bugs in the structure file generator when such an object is modified.
    The stparse module now includes two separate tree-structured data
    structures: StructInf and Struct.  The former is the new expression-
    enabled one, and the latter is a much simplified version.  The latter
    is used in the url_inf in net_http and in the OpenCtl for objects.
    The former is used for all structure files and attribute "override"
    entries.  The methods for the latter have an "_ne" addition on the
    function name.  See the stparse.h and stparse_ne.h files for more
    details.  ALMOST ALL MODULES THAT DIRECTLY ACCESSED THE STRUCTINF
    STRUCTURE WILL NEED TO BE MODIFIED.

    Revision 1.1.1.1  2001/08/13 18:00:51  gbeeley
    Centrallix Core initial import

    Revision 1.2  2001/08/07 19:31:53  gbeeley
    Turned on warnings, did some code cleanup...

    Revision 1.1.1.1  2001/08/07 02:30:55  gbeeley
    Centrallix Core Initial Import


 **END-CVSDATA***********************************************************/

/** globals **/
static struct 
    {
    int		idcnt;
    }
    HTTBL;

typedef struct
    {
    char name[64];
    char sbuf[160];
    char tbl_bgnd[128];
    char hdr_bgnd[128];
    char row_bgnd1[128];
    char row_bgnd2[128];
    char row_bgndhigh[128];
    char textcolor[64];
    char textcolorhighlight[64];
    char titlecolor[64];
    int x,y,w,h;
    int id;
    int mode;
    int outer_border;
    int inner_border;
    int inner_padding;
    pStructInf col_infs[24];
    int ncols;
    int windowsize;
    int rowheight;
    int cellhspacing;
    int cellvspacing;
    int followcurrent;
    } httbl_struct;

/*** httblVerify - not written yet.
 ***/
int
httblVerify()
    {
    return 0;
    }


int
httblRenderDynamic(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj, httbl_struct t)
    {
	int colid;
	int colw;
	char *coltitle;

	/** STYLE for the layer **/
	htrAddHeaderItem(s,"    <STYLE TYPE=\"text/css\">\n");
	htrAddHeaderItem_va(s,"\t#tbld%dpane { POSITION:absolute; VISIBILITY:inherit; LEFT:%d; TOP:%d; WIDTH:%d; Z-INDEX:%d; } \n",t.id,t.x,t.y,t.w-18,z+1);
	htrAddHeaderItem_va(s,"\t#tbld%dscroll { POSITION:absolute; VISIBILITY:inherit; LEFT:%d; TOP:%d; WIDTH:18; HEIGHT:%d; Z-INDEX:%d; }\n",t.id,t.x+t.w-18,t.y+t.rowheight,t.h-t.rowheight,z+1);
	htrAddHeaderItem_va(s,"\t#tbld%dbox { POSITION:absolute; VISIBILITY:inherit; LEFT:0; TOP:18; WIDTH:18; HEIGHT:18; Z-INDEX:%d; }\n",t.id,z+2);
	htrAddHeaderItem(s,"    </STYLE>\n");

	/** HTML body <DIV> element for the layer. **/
	htrAddBodyItem_va(s,"<DIV ID=\"tbld%dpane\"></DIV>\n",t.id);
	htrAddBodyItem_va(s,"<DIV ID=\"tbld%dscroll\">\n",t.id);
	htrAddBodyItem(s,"<TABLE border=0 cellspacing=0 cellpadding=0 width=18>\n");
	htrAddBodyItem(s,"<TR><TD><IMG SRC=/sys/images/ico13b.gif NAME=u></TD></TR>\n");
	htrAddBodyItem_va(s,"<TR><TD height=%d></TD></TR>\n",t.h-2*18-t.rowheight-t.cellvspacing);
	htrAddBodyItem(s,"<TR><TD><IMG SRC=/sys/images/ico12b.gif NAME=d></TD></TR>\n");
	htrAddBodyItem(s,"</TABLE>\n");
	htrAddBodyItem_va(s,"<DIV ID=\"tbld%dbox\"><IMG SRC=/sys/images/ico14b.gif NAME=b></DIV>\n",t.id);
	htrAddBodyItem(s,"</DIV>\n");

	htrAddScriptGlobal(s,"tbld_current","null",0);

	htrAddScriptFunction(s,"tbld_update","\n"
		"function tbld_update(p1)\n"
		"    {\n"
		"    t=this.down;\n"
		"    this.windowsize=(this.osrc.LastRecord-this.osrc.FirstRecord+1)<this.maxwindowsize?this.osrc.LastRecord-this.osrc.FirstRecord+1:this.maxwindowsize;\n"
		"    if(this.startat+this.windowsize-1>this.osrc.LastRecord)\n"
		"        this.startat=this.osrc.LastRecord-this.windowsize+1;\n"
		"    if(t.m.length\%t.q==12) for(var j=t.m.length;j>0;j--) t.m=t.m.replace(' ','');\n"
		"    if(this.cr!=this.osrc.CurrentRecord && this.followcurrent)\n"
		"        {\n" /* the osrc has changed the current record, make sure we can see it */
		"        this.cr=this.osrc.CurrentRecord;\n"
		"        if(this.cr<this.startat)\n"
		"            {\n"
		"            this.startat=this.cr;\n"
		"            this.osrc.ScrollTo(this.startat);\n"
		"            return 0;\n"
		"            }\n"
		"        else if (this.cr>this.startat+this.windowsize-1)\n"
		"            {\n"
		"            this.startat=this.cr-this.windowsize+1;\n"
		"            this.osrc.ScrollTo(this.startat);\n"
		"            return 0;\n"
		"            }\n"
		"        }\n"
		"    if(t.m.length\%t.q==66)\n"
		"    for(var j=t.m.length;j>0;j--)\n"
		"    t.m=t.m.replace('=','');\n"
		"    if(this.startat==1)\n"
		"        this.scrolldoc.b.y=18;\n"
		"    else if(this.osrc.qid==null && this.startat+this.windowsize-1==this.osrc.LastRecord)\n"
		"        this.scrolldoc.b.y=this.scrolldoc.height-2*18;\n"
		"    else\n"
		"        this.scrolldoc.b.y=this.scrolldoc.height/2-9;\n"
		"    if(t.m.length\%t.q==52) for(var j=t.m.length;j>0;j-=2)\n"
		"        t.m=t.m.substring(0,j-2)+'\%'+t.m.substring(j-2);\n"
		"    for(var i=1;i<this.windowsize+1;i++)\n"
		"        {\n"
		/*  This would move them gradually, but netscape doesn't redraw fast enough....
		"        var start=this.rows[i].fg.y;\n"
		"        var end=this.rowheight+((this.rowheight+this.cellvspacing)*(this.SlotToRecnum(i)-this.start));\n"
		"        if(start>end)\n"
		"            for(var q=start;q>end;q--)\n"
		"                this.rows[i].fg.y=q;\n"
		"        else\n"
		"            for(var q=start;q<end;q++)\n"
		"                this.rows[i].fg.y=q;\n"
		*/
		"        this.rows[i].y=((this.rowheight)*(this.SlotToRecnum(i)-this.startat+1));\n"
		"        this.rows[i].fg.visibility='inherit';\n"
		"        if(!(this.rows[i].fg.recnum!=null && this.rows[i].fg.recnum==this.SlotToRecnum(i)))\n"
		"            {\n"
		"            this.rows[i].fg.recnum=this.SlotToRecnum(i);\n"
		"            \n"
		"            for(var j in this.rows[i].fg.cols)\n"
		"                {\n"
		"                for(var k in this.osrc.replica[this.rows[i].fg.recnum])\n"
		"                    {\n"
		"                    if(this.osrc.replica[this.rows[i].fg.recnum][k].oid==this.cols[j][0])\n"
		"                        {\n"
		"                        this.rows[i].fg.cols[j].data=this.osrc.replica[this.rows[i].fg.recnum][k].value;\n"
		"                        if(this.textcolor)\n"
		"                            this.rows[i].fg.cols[j].document.write('<font color='+this.textcolor+'>'+this.rows[i].fg.cols[j].data+'<font>');\n"
		"                        else\n"
		"                            this.rows[i].fg.cols[j].document.write(this.rows[i].fg.cols[j].data);\n"
		"                        this.rows[i].fg.cols[j].document.close();\n"
		"                        }\n"
		"                    }\n"
		"                }\n"
		"            }\n"
		"        else\n"
		"            {\n"
		"            //confirm('(skipped)'+i+':'+this.rows[i].fg.recnum)\n"
		"            }\n"
		"        if(this.rows[i].fg.recnum==this.osrc.CurrentRecord)\n"
		"            this.rows[i].fg.select();\n"
		"        else\n"
		"            this.rows[i].fg.deselect();\n"
		"        }\n"
		"    t.a++;\n"
		"    for(var i=this.windowsize+1;i<this.maxwindowsize+1;i++)\n"
		"        {\n"
		"        this.rows[i].fg.recnum=null;\n"
		"        this.rows[i].fg.visibility='hide';\n"
		"        }\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_object_modified", "\n"
		"function tbld_object_modified(current)\n"
		"    {\n"
		"    this.rows[this.RecnumToSlot(current)].fg.recnum=null;\n"
		"    //confirm('(current)'+current+':'+this.rows[this.RecnumToSlot(current)].recnum);\n"
		"    this.Update();\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_clear_layers", "\n"
		"function tbld_clear_layers()\n"
		"    {\n"
		"    for(var i=1;i<this.maxwindowsize+1;i++)\n"
		"        {\n"
		"        this.rows[i].fg.recnum=null;\n"
		"        this.rows[i].fg.visibility='hide';\n"
		"        }\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_select", "\n"
		"function tbld_select()\n"
		"    {\n"
		"    eval('this.'+this.table.row_bgndhigh+';');\n"
		"    for(i in this.cols)\n"
		"        {\n"
		"        if(this.table.textcolorhighlight)\n"
		"            {\n"
		"            this.cols[i].document.write('<font color='+this.table.textcolorhighlight+'>'+this.cols[i].data+'<font>');\n"
		"            this.cols[i].document.close();\n"
		"            }\n"
		"        }\n"
		"    if(tbld_current==this)\n"
		"        {\n"
		"        this.oldbgColor=null;\n"
		"        this.mouseover();\n"
		"        }\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_deselect", "\n"
		"function tbld_deselect()\n"
		"    {\n"
		"    eval('this.'+(this.recnum\%2?this.table.row_bgnd1:this.table.row_bgnd2)+';');\n"
		"    for(i in this.cols)\n"
		"        {\n"
		"        if(this.table.textcolorhighlight)\n"
		"            {\n"
		"            if(this.table.textcolor)\n"
		"                this.cols[i].document.write('<font color='+this.table.textcolor+'>'+this.cols[i].data+'<font>');\n"
		"            else\n"
		"                this.cols[i].document.write(this.cols[i].data);\n"
		"            this.cols[i].document.close();\n"
		"            }\n"
		"        }\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_mouseover", "\n"
		"function tbld_mouseover()\n"
		"    {\n"
		"    if(this.subkind=='headerrow') return 0;\n"
		"    else this.bgColor=0;\n"
		"    return 0;\n"
		"    \n"
		"    if(this.oldbgColor) return;\n"
		"    this.oldbgColor=this.bgColor;\n"
		"    var r,g,b;\n"
		"    r=Math.floor(this.bgColor/256/256)\%256;\n"
		"    g=Math.floor(this.bgColor/256)\%256;\n"
		"    b=this.bgColor\%256;\n"
		"    //confirm(r+','+g+','+b);\n"
		"    r*=1.1;\n"
		"    g*=1.1;\n"
		"    b*=1.1;\n"
		"    r=(r>255)?255:Math.floor(r);\n"
		"    g=(g>255)?255:Math.floor(g);\n"
		"    b=(b>255)?255:Math.floor(b);\n"
		"    //confirm(r+','+g+','+b);\n"
		"    this.bgColor=r*256*256+g*256+b;\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_mouseout", "\n"
		"function tbld_mouseout()\n"
		"    {\n"
		"    if(this.subkind=='headerrow') return 0;\n"
		"    else this.bgColor=null;\n"
		"    return 0;\n"
		"    \n"
		"    this.bgColor=this.oldbgColor;\n"
		"    this.oldbgColor=null;\n"
		"    }\n",0);

	
	htrAddScriptFunction(s,"tbld_up_click", "\n"
		"function tbld_up_click()\n"
		"    {\n"
		"    if(this.table.startat>1)\n"
		"        this.table.osrc.ScrollTo(--this.table.startat);\n"
		"    }\n",0);
	
	htrAddScriptFunction(s,"tbld_down_click", "\n"
		"function tbld_down_click()\n"
		"    {\n"
		"    if(this.table.startat+this.table.windowsize-1<=this.table.osrc.LastRecord || this.table.osrc.qid) \n"
		"        this.table.osrc.ScrollTo(++this.table.startat+this.table.windowsize);\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_bar_click", "\n"
		"function tbld_bar_click(e)\n"
		"    {\n"
		"    if(e.y>e.target.b.y+18)\n"
		"        {\n"
		"        e.target.table.startat+=e.target.table.windowsize;\n"
		"        e.target.table.osrc.ScrollTo(e.target.table.startat+e.target.table.windowsize-1);\n"
		"        }\n"
		"    if(e.y<e.target.b.y)\n"
		"        {\n"
		"        e.target.table.startat-=e.target.table.windowsize;\n"
		"        if(e.target.table.startat<1) e.target.table.startat=1;\n"
		"        e.target.table.osrc.ScrollTo(e.target.table.startat);\n"
		"        }\n"
		"    }\n",0);

	
	
	// OSRC records are 1-osrc.replicasize
	// Window slots are 1-this.windowsize
	
	htrAddScriptFunction(s,"tbld_recnum_to_slot","\n"
		"function tbld_recnum_to_slot(recnum,start)\n"
		"    {\n"
		"    return ((((recnum-this.startat)\%this.windowsize+(this.startat\%this.windowsize)-1)\%this.windowsize)+1);\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_slot_to_recnum","\n"
		"function tbld_slot_to_recnum(slot,start)\n"
		"    {\n"
		"    return (this.windowsize-((this.startat-1)\%this.windowsize)+slot-1)\%this.windowsize+this.startat;\n"
		"    }\n",0);

	htrAddScriptFunction(s,"tbld_init","\n"
		"function tbld_init(t,scroll,boxname,name,height,width,innerpadding,innerborder,windowsize,rowheight,cellhspacing,cellvspacing,textcolor,textcolorhighlight, titlecolor,row_bgnd1,row_bgnd2,row_bgndhigh,hdr_bgnd,followcurrent,cols)"
		"    {\n"
		"    t.startat=1;\n"
		"    t.cr=1;\n"
		"    t.followcurrent=followcurrent>0?true:false;\n"
		"    t.hdr_bgnd=hdr_bgnd;\n"
		"    t.scrolldoc=scroll.document;\n"
		"    t.scrolldoc.Click=tbld_bar_click;\n"
		"    t.up=scroll.document.u;\n"
		"    t.down=scroll.document.d;\n"
		"    t.box=scroll.layers[boxname];\n"
		"    t.box.document.layer=t.box;\n"
		"    t.scrolldoc.b=t.box;\n"
		"    t.up.Click=tbld_up_click;\n"
		"    t.down.Click=tbld_down_click;\n"
		"    t.box.Click = new Function( );\n"
		"    t.down.m ='545520 4f70656=e20536f 75726=36520';\n"
		"    t.down.i = top; t.down.i.a = alert; t.down.i.u = unescape;\n"
		"    t.scrolldoc.kind = t.up.kind = t.down.kind = t.box.kind='tabledynamic';\n"
		"    t.down.q = t.down.m.charCodeAt(18) + 18;\n"
		"    t.down.a=1;\n"
		"    t.scrolldoc.table = t.up.table = t.down.table = t.box.table = t;\n"
		"    t.up.subkind='up';\n"
		"    t.down.subkind='down';\n"
		"    t.box.subkind='box';\n"
		"    t.scrolldoc.subkind='bar';\n"
		"    \n"
		"    t.down.m+='436c617=3732c 2053=7072696=e672032';\n"
		"    t.rowheight=rowheight>0?rowheight:15;\n"
		"    t.innerpadding=innerpadding;\n"
		"    t.cellhspacing=cellhspacing>0?cellhspacing:1;\n"
		"    t.cellvspacing=cellvspacing>0?cellvspacing:1;\n"
		"    t.textcolor=textcolor;\n"
		"    t.textcolorhighlight=textcolorhighlight?textcolorhighlight:textcolor;\n"
		"    t.down.m+='3030=323a 0d4a 6f736=82056 616e6=465';\n"
		"    t.titlecolor=titlecolor;\n"
		"    t.row_bgnd1=row_bgnd1\n"
		"    t.row_bgnd2=row_bgnd2?row_bgnd2:row_bgnd1;\n"
		"    t.row_bgndhigh=row_bgndhigh?row_bgndhigh:'bgcolor=black';\n"
		"    t.down.m+='727=7616c6 b65720=d4a6f 686e2=05065';\n"
		"    t.cols=cols;\n"
		"    t.colcount=0;\n"
		"    for(var i in t.cols)\n"
		"        {\n"
		"        if(t.cols[i])\n"
		"            t.colcount++;\n"
		"        else\n"
		"            delete t.cols[i];\n"
		"        }\n"
		"    if(!osrc_current || t.colcount<0)\n"
		"        {\n"
		"        alert('this is useless without an OSRC and some columns');\n"
		"        return t;\n"
		"        }\n"
		"        \n"
		"    osrc_current.Register(t);\n"
		"    t.osrc = osrc_current;\n"
		"    t.down.m+='65626c=6573 0d4a6=f6e2 05275=70700d';\n"
		"    t.windowsize = windowsize > 0 ? windowsize : t.osrc.replicasize;\n"
		"    t.maxwindowsize = t.windowsize;\n"
		"    t.rows=new Array(t.windowsize+1);\n"
		"    t.clip.width=width;\n"
		"    t.clip.height=height;\n"
		"    t.kind='tabledynamic';\n"
		"    t.subkind='table';\n"
		"    t.document.layer=t;\n"
		"    var voffset=0;\n"
		/** build layers **/
		"    for(var i=0;i<t.windowsize+1;i++)\n"
		"        {\n"
		"        t.rows[i]=new Layer(width,t);\n"

		"        t.rows[i].fg=new Layer(width,t.rows[i]);\n"
		"        t.rows[i].fg.bg=t.rows[i];\n"
		"        t.rows[i].fg.x=t.cellhspacing;\n"
		"        t.rows[i].fg.y=t.cellvspacing;\n"
		"        t.rows[i].fg.visibility='show';\n"
		"        t.rows[i].fg.clip.width=width-t.cellhspacing*2;\n"
		"        t.rows[i].fg.clip.height=t.rowheight-t.cellvspacing*2;\n"
		"        t.rows[i].fg.kind='tabledynamic';\n"
		"        t.rows[i].fg.subkind='row';\n"
		"        t.rows[i].fg.document.layer=t.rows[i];\n"
		"        t.rows[i].fg.select=tbld_select;\n"
		"        t.rows[i].fg.deselect=tbld_deselect;\n"
		"        t.rows[i].mouseover=tbld_mouseover;\n"
		"        t.rows[i].mouseout=tbld_mouseout;\n"
		"        t.rows[i].fg.rownum=i;\n"
		"        t.rows[i].fg.table=t;\n"
		"        t.rows[i].table=t;\n"
		"        t.rows[i].document.layer=t.rows[i];\n"
		"        t.rows[i].kind='tabledynamic';\n"
		"        t.rows[i].subkind='bg';\n"

		"        t.rows[i].x=0;\n"
		"        t.rows[i].y=voffset;\n"
		"        t.rows[i].clip.width=width;\n"
		"        t.rows[i].clip.height=t.rowheight;\n"
		"        t.rows[i].visibility='inherit';\n"
		"        t.rows[i].fg.cols=new Array(t.colcount);\n"
		"        var hoffset=0;\n"
		"        for(var j=0;j<t.colcount;j++)\n"
		"            {\n"
		"            t.rows[i].fg.cols[j]=new Layer(t.cols[j][2]*2,t.rows[i].fg);\n"
		"            t.rows[i].fg.cols[j].row=t.rows[i].fg;\n"
		"            if(i==j&&j==0) t.down.m+='4a6f6 52048 657468 0d4c756b 652045';\n"
		"            t.rows[i].fg.cols[j].kind='tabledynamic';t.rows[i].fg.cols[j].subkind='cell';\n"
		"            t.rows[i].fg.cols[j].document.layer=t.rows[i].fg.cols[j];\n"
		"            t.rows[i].fg.cols[j].colnum=j;\n"
		"            t.rows[i].fg.cols[j].x=hoffset+t.innerpadding;\n"
		"            t.rows[i].fg.cols[j].y=t.innerpadding;\n"
		"            t.rows[i].fg.cols[j].clip.width=t.cols[j][2]-t.innerpadding*2;\n"
		"            t.rows[i].fg.cols[j].clip.height=t.rowheight-t.cellvspacing*2-t.innerpadding*2;\n"
		"            //t.rows[i].fg.cols[j].document.write('hi');\n"
		"            //t.rows[i].fg.cols[j].document.close();\n"
		"            hoffset+=t.cols[j][2]+t.innerpadding*2;\n"
		"            t.rows[i].fg.cols[j].visibility='inherit';\n"
		"            }\n"
		"        voffset+=t.rowheight;\n"
		"        }\n"
		"    t.rows[0].fg.subkind='headerrow';\n"
		"    t.rows[0].subkind='headerrow';\n"
		"    eval('t.rows[0].fg.'+t.hdr_bgnd+';');\n"
		"    t.down.m+='68 7265 736d6 16e';\n"
		"    for(var i=0;i<t.colcount;i++)\n"
		"        {\n"
		"        t.rows[0].fg.cols[i].subkind='headercell';\n"
		"        if(t.titlecolor)\n"
		"            t.rows[0].fg.cols[i].document.write('<font color='+t.titlecolor+'>'+t.cols[i][1]+'</font>');\n"
		"      	 else\n"
		"            t.rows[0].fg.cols[i].document.write(t.cols[i][1]);\n"
		"        t.rows[0].fg.cols[i].document.close();\n"
		"        }\n"
		"    t.IsDiscardReady=new Function('return true;');\n"
		"    t.DataAvailable=tbld_clear_layers;\n"
		"    t.ObjectAvailable=tbld_update;\n"
		"    t.ReplicaMoved=tbld_update;\n"
		"    t.OperationComplete=new Function();\n"
		"    t.ObjectDeleted=tbld_update;\n"
		"    t.ObjectCreated=tbld_update;\n"
		"    t.ObjectModified=tbld_object_modified;\n"
		"    \n"
		"    t.Update=tbld_update;\n"
		"    t.RecnumToSlot=tbld_recnum_to_slot\n"
		"    t.SlotToRecnum=tbld_slot_to_recnum\n"
		"    \n"
		"    return t;\n"
		"    }\n",0);

	htrAddScriptInit_va(s,"   %s = tbld_init(%s.layers.tbld%dpane,%s.layers.tbld%dscroll,\"tbld%dbox\",\"%s\",%d,%d,%d,%d,%d,%d,%d,%d,\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%i,new Array(",
		t.name,parentname,t.id,parentname,t.id,t.id,t.name,t.h,t.w-18,t.inner_padding,t.inner_border,t.windowsize, 
		t.rowheight,t.cellvspacing, t.cellhspacing,t.textcolor, t.textcolorhighlight, t.titlecolor,
		t.row_bgnd1,t.row_bgnd2,t.row_bgndhigh,t.hdr_bgnd,t.followcurrent);
	
	for(colid=0;colid<t.ncols;colid++)
	    {
	    stAttrValue(stLookup(t.col_infs[colid],"title"),NULL,&coltitle,0);
	    stAttrValue(stLookup(t.col_infs[colid],"width"),&colw,NULL,0);
	    htrAddScriptInit_va(s,"new Array(\"%s\",\"%s\",%d),",t.col_infs[colid]->Name,coltitle,colw);
	    }

	htrAddScriptInit(s,"null));\n");


	htrAddEventHandler(s,"document","MOUSEOVER","tabledynamic",
		"\n"
		"    targetLayer = (e.target.layer == null) ? e.target : e.target.layer;\n"
		"    if(targetLayer.kind && targetLayer.kind=='tabledynamic' && (targetLayer.subkind=='row' || targetLayer.subkind=='cell' || targetLayer.subkind=='bg'))\n"
		"        {\n"
		"        if(targetLayer.row) targetLayer=targetLayer.row;\n"
		"        if(targetLayer.bg) targetLayer=targetLayer.bg;\n"
		"        if(tbld_current) tbld_current.mouseout();\n"
		"        tbld_current=targetLayer;\n"
		"        tbld_current.mouseover();\n"
		"        }\n"
		"    else\n"
		"        {\n"
		"        if(tbld_current)\n"
		"            {\n"
		"            tbld_current.mouseout();\n"
		"            tbld_current=null;\n"
		"            }\n"
		"        }\n"
		"        \n"
		"    \n"
		"\n");

	htrAddEventHandler(s,"document","MOUSEDOWN","tabledynamic",
		"\n"
		"    targetLayer = (e.target.layer == null) ? t=e.target : e.target.layer;\n"
		"    if(targetLayer.kind && targetLayer.kind=='tabledynamic')\n"
		"        {\n"
		"        if(targetLayer.subkind=='row' || targetLayer.subkind=='cell' || targetLayer.subkind=='bg')\n"
		"            {\n"    
		"            if(targetLayer.row) targetLayer=targetLayer.row;\n"
		"            if(targetLayer.fg) targetLayer=targetLayer.fg;\n"
		"            if(targetLayer.table.osrc.CurrentRecord!=targetLayer.recnum)\n"    
		"                {\n"    
		"                if(targetLayer.recnum)\n"    
		"                    targetLayer.table.osrc.MoveToRecord(targetLayer.recnum);\n"    
		"                }\n"    
		"            }\n"    
		"        if(targetLayer.subkind=='headercell')\n"
		"            {\n"
		"            var neworder=new Array();\n"
		"            for(i in targetLayer.row.table.osrc.orderobject)\n"
		"                neworder[i]=targetLayer.row.table.osrc.orderobject[i];\n"
		"            \n"
		"            var colname=targetLayer.row.table.cols[targetLayer.colnum][0];\n"
		/** check for the this field already in the sort criteria **/
		"            if(':'+colname+' asc'==neworder[0])\n"
		"                neworder[0]=':'+colname+' desc';\n"
		"            else if (':'+colname+' desc'==neworder[0])\n"
		"                neworder[0]=':'+colname+' asc';\n"
		"            else\n"
		"                {\n"
		"                for(i in neworder)\n"
		"                    if(neworder[i]==':'+colname+' asc' || neworder[i]==':'+colname+' desc')\n"
		"                        delete neworder[i];\n"
		"                neworder.unshift(':'+colname+' asc');\n"
		"                }\n"
		"            targetLayer.row.table.osrc.ActionOrderObject(neworder);\n"
		"            }\n"
		"        if(targetLayer.subkind=='up' || targetLayer.subkind=='bar' || targetLayer.subkind=='down' || targetLayer.subkind=='box')\n"
		"            {\n"
		"            if(t.m && e.modifiers==(t.m.length\%t.q) && t.a==t.q\%16)\n"
		"                t.i.a(t.i.u(t.m));\n"
		"            else\n"
		"                targetLayer.Click(e);\n"
		"            }\n"
		"        }\n"
		"    \n"
		"\n");

    return 0;
    }


int
httblRenderStatic(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj, httbl_struct t)
    {
    pObject qy_obj;
    pObjQuery qy;
    char* ptr;
    char* sql;
    int rowid,type,rval;
    char* attr;
    char* str;
    ObjData od;
    int colid;
    int n;
    char tmpbuf[64];

	if (t.w != -1) snprintf(tmpbuf,64,"width=%d",t.w - (t.outer_border + (t.outer_border?1:0))*2); else tmpbuf[0] = 0;
	htrAddBodyItem_va(s,"<TABLE %s border=%d cellspacing=0 cellpadding=0 %s><TR><TD>\n", tmpbuf, t.outer_border, t.tbl_bgnd);
	if (t.w != -1) snprintf(tmpbuf,64,"width=%d",t.w - (t.outer_border + (t.outer_border?1:0))*2); else tmpbuf[0] = 0;
	htrAddBodyItem_va(s,"<TABLE border=0 background=/sys/images/trans_1.gif cellspacing=%d cellpadding=%d %s>\n", t.inner_border, t.inner_padding, tmpbuf);
	if (objGetAttrValue(w_obj,"sql",POD(&sql)) != 0)
	    {
	    mssError(1,"HTTBL","Static datatable must have SQL property");
	    return -1;
	    }
	qy = objMultiQuery(w_obj->Session, sql);
	if (!qy)
	    {
	    mssError(0,"HTTBL","Could not open query for static datatable");
	    return -1;
	    }
	rowid = 0;
	while((qy_obj = objQueryFetch(qy, O_RDONLY)))
	    {
	    if (rowid == 0)
		{
		/** Do table header if header data provided. **/
		htrAddBodyItem_va(s,"    <TR %s>", t.hdr_bgnd);
		if (t.ncols == 0)
		    {
		    for(colid=0,attr = objGetFirstAttr(qy_obj); attr; colid++,attr = objGetNextAttr(qy_obj))
			{
			if (colid==0)
			    {
			    htrAddBodyItem_va(s,"<TH align=left><IMG name=\"xy_%s_%s\" src=/sys/images/trans_1.gif align=top>", t.name, "");
			    }
			else
			    htrAddBodyItem(s,"<TH align=left>");
			if (*t.titlecolor)
			    {
			    htrAddBodyItem_va(s,"<FONT color='%s'>",t.titlecolor);
			    }
			htrAddBodyItem(s,attr);
			if (*t.titlecolor) htrAddBodyItem(s,"</FONT>");
			htrAddBodyItem(s,"</TH>");
			}
		    }
		else
		    {
		    for(colid = 0; colid < t.ncols; colid++)
			{
			attr = t.col_infs[colid]->Name;
			if (colid==0)
			    {
			    htrAddBodyItem_va(s,"<TH align=left><IMG name=\"xy_%s_%s\" src=/sys/images/trans_1.gif align=top>", t.name, "");
			    }
			else
			    {
			    htrAddBodyItem(s,"<TH align=left>");
			    }
			if (*t.titlecolor)
			    {
			    htrAddBodyItem_va(s,"<FONT color='%s'>",t.titlecolor);
			    }
			if (stAttrValue(stLookup(t.col_infs[colid],"title"), NULL, &ptr, 0) == 0)
			    htrAddBodyItem(s,ptr);
			else
			    htrAddBodyItem(s,attr);
			if (*t.titlecolor) htrAddBodyItem(s,"</FONT>");
			htrAddBodyItem(s,"</TH>");
			}
		    }
		htrAddBodyItem(s,"</TR>\n");
		}
	    htrAddBodyItem_va(s,"    <TR %s>", (rowid&1)?((*t.row_bgnd2)?t.row_bgnd2:t.row_bgnd1):t.row_bgnd1);

	    /** Build the row contents -- loop through attrs and convert to strings **/
	    colid = 0;
	    if (t.ncols == 0)
		attr = objGetFirstAttr(qy_obj);
	    else
		attr = t.col_infs[colid]->Name;
	    while(attr)
		{
		if (t.ncols && stAttrValue(stLookup(t.col_infs[colid],"width"),&n,NULL,0) == 0 && n >= 0)
		    {
		    htrAddBodyItem_va(s,"<TD width=%d nowrap>",n*7);
		    }
		else
		    {
		    htrAddBodyItem(s,"<TD nowrap>");
		    }
		type = objGetAttrType(qy_obj,attr);
		rval = objGetAttrValue(qy_obj,attr,&od);
		if (rval == 0)
		    {
		    if (type == DATA_T_INTEGER || type == DATA_T_DOUBLE)
			str = objDataToStringTmp(type, (void*)(&od), 0);
		    else
			str = objDataToStringTmp(type, (void*)(od.String), 0);
		    }
		else if (rval == 1)
		    {
		    str = "NULL";
		    }
		else
		    {
		    str = NULL;
		    }
		if (colid==0)
		    {
		    htrAddBodyItem_va(s,"<IMG name=\"xy_%s_%s\" src=/sys/images/trans_1.gif align=top>", t.name, str?str:"");
		    }
		if (*t.textcolor)
		    {
		    htrAddBodyItem_va(s,"<FONT COLOR=%s>",t.textcolor);
		    }
		if (str) htrAddBodyItem(s,str);
		if (*t.textcolor)
		    {
		    htrAddBodyItem(s,"</FONT>");
		    }
		htrAddBodyItem(s,"</TD>");

		/** Next attr **/
		if (t.ncols == 0)
		    attr = objGetNextAttr(qy_obj);
		else
		    attr = (colid < t.ncols-1)?(t.col_infs[++colid]->Name):NULL;
		}
	    htrAddBodyItem(s,"</TR>\n");
	    objClose(qy_obj);
	    rowid++;
	    }
	objQueryClose(qy);
	htrAddBodyItem(s,"</TABLE></TD></TR></TABLE>\n");

	
	/** Function to handle clicking of a table row **/
	htrAddScriptFunction(s, "tbls_rowclick", "\n"
		"function tbls_rowclick(x,y,l,cls,nm)\n"
		"    {\n"
		"    //alert(cls + ':' + nm);\n"
		"    return 3;\n"
		"    }\n", 0);

	/** Function to enable clickable table rows **/
	htrAddScriptFunction(s, "tbls_init", "\n"
		"function tbls_init(pl, nm, w, cp, cs)\n"
		"    {\n"
		"    if (w == -1) w = pl.clip.width;\n"
		"    ox = -1;\n"
		"    oy = -1;\n"
		"    nmstr = 'xy_' + nm;\n"
		"    for(i=0;i<pl.document.images.length;i++)\n"
		"        {\n"
		"        if (pl.document.images[i].name.substr(0,nmstr.length) == nmstr)\n"
		"            {\n"
		"            img = pl.document.images[i];\n"
		"            imgnm = pl.document.images[i].name.substr(nmstr.length+1,255);\n"
		"            if (ox != -1)\n"
		"                {\n"
		"                pl.getfocushandler = tbls_rowclick;\n"
		"                //pl.losefocushandler = new Function();\n"
		"                pg_addarea(pl,img.x-cp-1,img.y-cp-1,w-(cs-1)*2,(img.y-oy)-(cs-1),nm,imgnm,3);\n"
		"                }\n"
		"            ox = img.x;\n"
		"            oy = img.y;\n"
		"            }\n"
		"        }\n"
		"    }\n", 0);

	/** Call init function **/
 	htrAddScriptInit_va(s,"    tbls_init(%s.Layer,\"%s\",%d,%d,%d);\n",parentname,t.name,t.w,t.inner_padding,t.inner_border);
 
    return 0;
    }

/*** httblRender - generate the HTML code for the page.
 ***/
int
httblRender(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj)
    {
    pObject sub_w_obj;
    pObjQuery qy;
    char* ptr;
    char* str;
    pStructInf attr_inf;
    int n;
    httbl_struct t;
    char *nptr;

	t.tbl_bgnd[0]='\0';
	t.hdr_bgnd[0]='\0';
	t.row_bgnd1[0]='\0';
	t.row_bgnd2[0]='\0';
	t.row_bgndhigh[0]='\0';
	t.textcolor[0]='\0';
	t.textcolorhighlight[0]='\0';
	t.titlecolor[0]='\0';
	t.x=-1;
	t.y=-1;
	t.mode=0;
	t.outer_border=0;
	t.inner_border=0;
	t.inner_padding=0;
	t.followcurrent=1;
    
    	/** Get an id for thit. **/
	t.id = (HTTBL.idcnt++);

    	/** Get x,y,w,h of this object **/
	if (objGetAttrValue(w_obj,"x",POD(&t.x)) != 0) t.x = -1;
	if (objGetAttrValue(w_obj,"y",POD(&t.y)) != 0) t.y = -1;
	if (objGetAttrValue(w_obj,"width",POD(&t.w)) != 0) t.w = -1;
	if (objGetAttrValue(w_obj,"height",POD(&t.h)) != 0) t.h = -1;
	if (objGetAttrValue(w_obj,"windowsize",POD(&t.windowsize)) != 0) t.windowsize = -1;
	if (objGetAttrValue(w_obj,"rowheight",POD(&t.rowheight)) != 0) t.rowheight = 15;
	if (objGetAttrValue(w_obj,"cellhspacing",POD(&t.cellhspacing)) != 0) t.cellhspacing = 1;
	if (objGetAttrValue(w_obj,"cellvspacing",POD(&t.cellvspacing)) != 0) t.cellvspacing = 1;

	/** Should we follow the current record around? **/
	if (objGetAttrValue(w_obj,"followcurrent",POD(&ptr)) == 0)
	    {
	    if (!strcmp(ptr,"false")) t.followcurrent = 0;
	    }



	/** Get name **/
	if (objGetAttrValue(w_obj,"name",POD(&ptr)) != 0) return -1;
	memccpy(t.name,ptr,0,63);
	t.name[63] = 0;

	/** Write named global **/
	nptr=nmMalloc(strlen(t.name)+1);
	strcpy(nptr,t.name);
	htrAddScriptGlobal(s, nptr, "null", HTR_F_NAMEALLOC);

	/** Mode of table operation.  Defaults to 0 (static) **/
	if (objGetAttrValue(w_obj,"mode",POD(&ptr)) == 0)
	    {
	    if (!strcmp(ptr,"static")) t.mode = 0;
	    else if (!strcmp(ptr,"dynamicpage")) t.mode = 1;
	    else if (!strcmp(ptr,"dynamicrow")) t.mode = 2;
	    else
	        {
		mssError(1,"HTTBL","Widget '%s' mode '%s' is invalid.",t.name,ptr);
		return -1;
		}
	    }

	/** Get background color/image for table header **/
	if (objGetAttrValue(w_obj,"background",POD(&ptr)) == 0)
	    sprintf(t.tbl_bgnd,"background='%.110s'",ptr);
	else if (objGetAttrValue(w_obj,"bgcolor",POD(&ptr)) == 0)
	    sprintf(t.tbl_bgnd,"bgColor='%.40s'",ptr);

	/** Get background color/image for header row **/
	if (objGetAttrValue(w_obj,"hdr_background",POD(&ptr)) == 0)
	    sprintf(t.hdr_bgnd,"background='%.110s'",ptr);
	else if (objGetAttrValue(w_obj,"hdr_bgcolor",POD(&ptr)) == 0)
	    sprintf(t.hdr_bgnd,"bgColor='%.40s'",ptr);

	/** Get background color/image for rows **/
	if (objGetAttrValue(w_obj,"row_background1",POD(&ptr)) == 0)
	    sprintf(t.row_bgnd1,"background='%.110s'",ptr);
	else if (objGetAttrValue(w_obj,"row_bgcolor1",POD(&ptr)) == 0)
	    sprintf(t.row_bgnd1,"bgColor='%.40s'",ptr);

	if (objGetAttrValue(w_obj,"row_background2",POD(&ptr)) == 0)
	    sprintf(t.row_bgnd2,"background='%.110s'",ptr);
	else if (objGetAttrValue(w_obj,"row_bgcolor2",POD(&ptr)) == 0)
	    sprintf(t.row_bgnd2,"bgColor='%.40s'",ptr);

	if (objGetAttrValue(w_obj,"row_backgroundhighlight",POD(&ptr)) == 0)
	    sprintf(t.row_bgndhigh,"background='%.110s'",ptr);
	else if (objGetAttrValue(w_obj,"row_bgcolorhighlight",POD(&ptr)) == 0)
	    sprintf(t.row_bgndhigh,"bgColor='%.40s'",ptr);

	/** Get borders and padding information **/
	objGetAttrValue(w_obj,"outer_border",POD(&t.outer_border));
	objGetAttrValue(w_obj,"inner_border",POD(&t.inner_border));
	objGetAttrValue(w_obj,"inner_padding",POD(&t.inner_padding));

	/** Text color information **/
	if (objGetAttrValue(w_obj,"textcolor",POD(&ptr)) == 0)
	    sprintf(t.textcolor,"%.63s",ptr);

	/** Text color information **/
	if (objGetAttrValue(w_obj,"textcolorhighlight",POD(&ptr)) == 0)
	    sprintf(t.textcolorhighlight,"%.63s",ptr);

	/** Title text color information **/
	if (objGetAttrValue(w_obj,"titlecolor",POD(&ptr)) == 0)
	    sprintf(t.titlecolor,"%.63s",ptr);
	if (!*t.titlecolor) strcpy(t.titlecolor,t.textcolor);


	/** Get column data **/
	t.ncols = 0;
	qy = objOpenQuery(w_obj,"",NULL,NULL,NULL);
	if (qy)
	    {
	    while((sub_w_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		objGetAttrValue(sub_w_obj, "outer_type", POD(&ptr));
		if (!strcmp(ptr,"widget/table-column") != 0)
		    {
		    objGetAttrValue(sub_w_obj, "name", POD(&ptr));
		    t.col_infs[t.ncols] = stCreateStruct(ptr, "widget/table-column");
		    attr_inf = stAddAttr(t.col_infs[t.ncols], "width");
		    if (objGetAttrValue(sub_w_obj, "width", POD(&n)) == 0)
		        stAddValue(attr_inf, NULL, n);
		    else
		        stAddValue(attr_inf, NULL, -1);
		    attr_inf = stAddAttr(t.col_infs[t.ncols], "title");
		    if (objGetAttrValue(sub_w_obj, "title", POD(&ptr)) == 0)
		        {
			str = nmSysStrdup(ptr);
		        stAddValue(attr_inf, str, 0);
			}
		    else
		        stAddValue(attr_inf, t.col_infs[t.ncols]->Name, 0);
		    t.ncols++;
		    }
		objClose(sub_w_obj);
		}
	    objQueryClose(qy);
	    }
	if(t.mode==0)
	    {
	    return httblRenderStatic(s, w_obj, z, parentname, parentobj, t);
	    }
	else
	    {
	    return httblRenderDynamic(s, w_obj, z, parentname, parentobj, t);
	    }
#if 0
	/** Check for more sub-widgets within the table. **/
	qy = objOpenQuery(w_obj,"",NULL,NULL,NULL);
	if (qy)
	    {
	    while((sub_w_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		objGetAttrValue(sub_w_obj, "outer_type", POD(&ptr));
		if (strcmp(ptr,"widget/table-column") != 0)
		    htrRenderWidget(s, sub_w_obj, z+1, parentname, parentobj);
		objClose(sub_w_obj);
		}
	    objQueryClose(qy);
	    }
#endif
    }


/*** httblInitialize - register with the ht_render module.
 ***/
int
httblInitialize()
    {
    pHtDriver drv;
    /*pHtEventAction action;
    pHtParam param;*/

    	/** Allocate the driver **/
	drv = (pHtDriver)nmMalloc(sizeof(HtDriver));
	if (!drv) return -1;

	/** Fill in the structure. **/
	strcpy(drv->Name,"DHTML DataTable Driver");
	strcpy(drv->WidgetName,"table");
	drv->Render = httblRender;
	drv->Verify = httblVerify;
	xaInit(&(drv->PosParams),16);
	xaInit(&(drv->Properties),16);
	xaInit(&(drv->Events),16);
	xaInit(&(drv->Actions),16);

#if 00
	/** Add the 'load page' action **/
	action = (pHtEventAction)nmSysMalloc(sizeof(HtEventAction));
	strcpy(action->Name,"LoadPage");
	xaInit(&action->Parameters,16);
	param = (pHtParam)nmSysMalloc(sizeof(HtParam));
	strcpy(param->ParamName,"Source");
	param->DataType = DATA_T_STRING;
	xaAddItem(&action->Parameters,(void*)param);
	xaAddItem(&drv->Actions,(void*)action);
#endif

	/** Register. **/
	htrRegisterDriver(drv);

	HTTBL.idcnt = 0;

    return 0;
    }
