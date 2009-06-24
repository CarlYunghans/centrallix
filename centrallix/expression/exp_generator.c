#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include "obj.h"
#include "cxlib/mtask.h"
#include "cxlib/xarray.h"
#include "cxlib/xhash.h"
#include "cxlib/mtlexer.h"
#include "expression.h"
#include "cxlib/mtsession.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1998-2001 LightSys Technology Services, Inc.		*/
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
/* Module:	expression.h, exp_generator.c                           */
/* Author:	Greg Beeley (GRB)                                       */
/* Date:	October 1, 2001                                         */
/*									*/
/* Description:	This is a new expression subsystem module which allows	*/
/*		an expression to be converted from an expression tree	*/
/*		back to its textual representation.			*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: exp_generator.c,v 1.14 2009/06/24 17:33:19 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/expression/exp_generator.c,v $

    $Log: exp_generator.c,v $
    Revision 1.14  2009/06/24 17:33:19  gbeeley
    - (change) adding domain param to expGenerateText, so it can be used to
      generate an expression string with lower domains converted to constants
    - (bugfix) better handling of runserver() embedded within runclient(), etc
    - (feature) allow subtracting strings, e.g., "abcde" - "de" == "abc"
    - (bugfix) after a property has been set using reverse evaluation, tag it
      as modified so it shows up as changed in other expressions using that
      same object param list
    - (change) condition() function now uses short-circuit evaluation
      semantics, so parameters are only evaluated as they are needed... e.g.
      condition(a,b,c) if a is true, b is returned and c is never evaluated,
      and vice versa.
    - (feature) add structure for reverse-evaluation of functions.  The
      isnull() function now supports this feature.
    - (bugfix) save/restore the coverage mask before/after evaluation, so that
      a nested subexpression (eval or subquery) using the same object list
      will not cause an inconsistency.  Basically a reentrancy bug.
    - (bugfix) some functions were erroneously depending on the data type of
      a NULL value to be correct.
    - (feature) adding truncate() function which is similar to round().
    - (feature) adding constrain() function which limits a value to be
      between a given minimum and maximum value.
    - (bugfix) first() and last() functions were not properly resetting the
      value to NULL between GROUP BY groups
    - (bugfix) some expression-to-JS fixes

    Revision 1.13  2008/09/14 05:17:27  gbeeley
    - (bugfix) subquery evaluator was leaking query handles if subquery did
      not return any rows.
    - (change) add ability to generate expression text based on the domain of
      evaluation (client, server, etc.)

    Revision 1.12  2008/06/25 01:04:58  gbeeley
    - (bugfix) switch to cxjs_plus instead of just the + operator when adding
      values in cxsql expressions deployed into javascript.  The + operator in
      cxsql has slightly different semantics than the + operator in javascript.

    Revision 1.11  2007/03/21 04:48:08  gbeeley
    - (feature) component multi-instantiation.
    - (feature) component Destroy now works correctly, and "should" free the
      component up for the garbage collector in the browser to clean it up.
    - (feature) application, component, and report parameters now work and
      are normalized across those three.  Adding "widget/parameter".
    - (feature) adding "Submit" action on the form widget - causes the form
      to be submitted as parameters to a component, or when loading a new
      application or report.
    - (change) allow the label widget to receive obscure/reveal events.
    - (bugfix) prevent osrc Sync from causing an infinite loop of sync's.
    - (bugfix) use HAVING clause in an osrc if the WHERE clause is already
      spoken for.  This is not a good long-term solution as it will be
      inefficient in many cases.  The AML should address this issue.
    - (feature) add "Please Wait..." indication when there are things going
      on in the background.  Not very polished yet, but it basically works.
    - (change) recognize both null and NULL as a null value in the SQL parsing.
    - (feature) adding objSetEvalContext() functionality to permit automatic
      handling of runserver() expressions within the OSML API.  Facilitates
      app and component parameters.
    - (feature) allow sql= value in queries inside a report to be runserver()
      and thus dynamically built.

    Revision 1.10  2007/03/12 19:18:32  gbeeley
    - (change) use a function to fetch a prop value in a JS expression, so we
      can catch undefined properties and objects.

    Revision 1.9  2007/03/06 16:16:55  gbeeley
    - (security) Implementing recursion depth / stack usage checks in
      certain critical areas.
    - (feature) Adding ExecMethod capability to sysinfo driver.

    Revision 1.8  2006/10/16 18:34:33  gbeeley
    - (feature) ported all widgets to use widget-tree (wgtr) alone to resolve
      references on client side.  removed all named globals for widgets on
      client.  This is in preparation for component widget (static and dynamic)
      features.
    - (bugfix) changed many snprintf(%s) and strncpy(), and some sprintf(%.<n>s)
      to use strtcpy().  Also converted memccpy() to strtcpy().  A few,
      especially strncpy(), could have caused crashes before.
    - (change) eliminated need for 'parentobj' and 'parentname' parameters to
      Render functions.
    - (change) wgtr port allowed for cleanup of some code, especially the
      ScriptInit calls.
    - (feature) ported scrollbar widget to Mozilla.
    - (bugfix) fixed a couple of memory leaks in allocated data in widget
      drivers.
    - (change) modified deployment of widget tree to client to be more
      declarative (the build_wgtr function).
    - (bugfix) removed wgtdrv_templatefile.c from the build.  It is a template,
      not an actual module.

    Revision 1.7  2005/02/26 06:42:36  gbeeley
    - Massive change: centrallix-lib include files moved.  Affected nearly
      every source file in the tree.
    - Moved all config files (except centrallix.conf) to a subdir in /etc.
    - Moved centrallix modules to a subdir in /usr/lib.

    Revision 1.6  2004/08/30 03:22:52  gbeeley
    - use cxjs_xxxyyy() for all javascript funcs in runclient() expressions now.

    Revision 1.5  2004/02/24 20:02:26  gbeeley
    - adding proper support for external references in an expression, so
      that they get re-evaluated each time.  Example - getdate().
    - adding eval() function but no implementation at this time - it is
      however supported for runclient() expressions (in javascript).
    - fixing some quoting issues

    Revision 1.4  2003/05/30 17:39:48  gbeeley
    - stubbed out inheritance code
    - bugfixes
    - maintained dynamic runclient() expressions
    - querytoggle on form
    - two additional formstatus widget image sets, 'large' and 'largeflat'
    - insert support
    - fix for startup() not always completing because of queries
    - multiquery module double objClose fix
    - limited osml api debug tracing

    Revision 1.3  2003/04/24 02:13:22  gbeeley
    Added functionality to handle "domain of execution" to the expression
    module, allowing the developer to specify the nature of an expression
    (run on client, server, or static on server).

    Revision 1.2  2002/06/19 23:29:33  gbeeley
    Misc bugfixes, corrections, and 'workarounds' to keep the compiler
    from complaining about local variable initialization, among other
    things.

    Revision 1.1  2001/10/02 16:23:09  gbeeley
    Added exp_generator expressiontree-to-text generation module.  Also fixed
    a precedence problem with EXPR_N_FUNCTION nodes; not sure why that wasn't
    causing trouble previously.


 **END-CVSDATA***********************************************************/


/*** Structure for handling the expression generation. ***/
typedef struct _EG
    {
    pParamObjects   Objlist;
    int		    (*WriteFn)();
    void*	    WriteArg;
    char	    EscChar;
    int		    Domain;
    char	    TmpBuf[256];
    }
    ExpGen, *pExpGen;


/*** exp_internal_WriteText - writes text through the write_fn, but checks
 *** for escaping of strings and such, as needed.
 ***/
int
exp_internal_WriteText(pExpGen eg, char* text)
    {
    int n = strlen(text);
    char* buf;
    char* ptr;

	/** No escaping?  Then just write it. **/
	if (!eg->EscChar)
	    {
	    eg->WriteFn(eg->WriteArg, text, n, 0, 0);
	    return 0;
	    }

	/** Allocate a xfer buffer large enough **/
	buf = (char*)nmSysMalloc(2*n + 1);
	ptr = buf;

	/** Move the contents of 'text' to 'buf', escaping as needed **/
	while(*text)
	    {
	    if (*text == eg->EscChar || *text == '\\')
	        {
		*(ptr++) = '\\';
		}
	    if (eg->EscChar == '\\' && (*text == '\r' || *text == '\n' || *text == '\t'))
		{
		*(ptr++) = '\\';
		switch(*text)
		    {
		    case '\r': *(ptr++) = 'r'; break;
		    case '\n': *(ptr++) = 'n'; break;
		    case '\t': *(ptr++) = 't'; break;
		    }
		text++;
		continue;
		}
	    *(ptr++) = *(text++);
	    }
	*ptr = '\0';

	/** Write it out. **/
	eg->WriteFn(eg->WriteArg, buf, ptr-buf, 0, 0);

	/** Free the tmp buffer **/
	nmSysFree(buf);

    return 0;
    }


/*** exp_internal_CheckConstants() - from the domain declaration and target
 *** for the expression generation, figure out if the node should be treated
 *** as a constant.
 ***/
int
exp_internal_CheckConstants(pExpression exp, pExpGen eg)
    {
    int nodetype = exp->NodeType;
    int n;

	if ((eg->Domain == EXPR_F_RUNCLIENT && ((exp->Flags & EXPR_F_RUNSERVER) || (exp->Flags & EXPR_F_RUNSTATIC) || (exp->Flags & EXPR_F_DOMAINMASK) == 0)) ||
	    (eg->Domain == EXPR_F_RUNSERVER && ((exp->Flags & EXPR_F_RUNSTATIC) || (exp->Flags & EXPR_F_DOMAINMASK) == 0)) ||
	    (exp->Flags & EXPR_F_FREEZEEVAL))
	    {
	    if (exp->Flags & EXPR_F_NULL)
		nodetype = 0;
	    else if ((n = expDataTypeToNodeType(exp->DataType)) >= 0)
		nodetype = n;
	    }

    return nodetype;
    }


/*** exp_internal_GenerateText_cxsql - internal recursive version to do the
 *** work needed by the below function.
 ***/
int
exp_internal_GenerateText_cxsql(pExpression exp, pExpGen eg)
    {
    int i;
    int nodetype;

	/** Check recursion **/
	if (thExcessiveRecursion())
	    {
	    mssError(1,"EXP","Failed to generate CXSQL expression: resource exhaustion occurred");
	    return -1;
	    }

	/** Do we have a domain declaration?  Add the pseudo-function for it if so **/
	if (exp->Flags & EXPR_F_DOMAINMASK)
	    {
	    if (exp->Flags & EXPR_F_RUNSTATIC)
		exp_internal_WriteText(eg, "runstatic(");
	    else if (exp->Flags & EXPR_F_RUNSERVER)
		exp_internal_WriteText(eg, "runserver(");
	    else
		exp_internal_WriteText(eg, "runclient(");
	    }

	/** Treat some expressions as constants **/
	nodetype = exp_internal_CheckConstants(exp, eg);

	/** Select an expression type **/
	switch(nodetype)
	    {
	    case EXPR_N_FUNCTION:
	        /** Function node - write function call, param list, end paren. **/
	        sprintf(eg->TmpBuf,"%.250s(",exp->Name);
		exp_internal_WriteText(eg, eg->TmpBuf);
		for(i=0;i<exp->Children.nItems;i++)
		    {
		    if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[i]), eg) < 0) return -1;
		    if (i != exp->Children.nItems-1)
		        {
		        exp_internal_WriteText(eg, ",");
			}
		    }
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_LIST:
	        /** List node - write paren, list items, end paren **/
		exp_internal_WriteText(eg, "(");
		for(i=0;i<exp->Children.nItems;i++)
		    {
		    if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[i]), eg) < 0) return -1;
		    if (i != exp->Children.nItems-1)
		        {
		        exp_internal_WriteText(eg, ",");
			}
		    }
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_MULTIPLY:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " * ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_DIVIDE:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " / ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_PLUS:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " + ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_MINUS:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " - ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_COMPARE:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		switch(exp->CompareType)
		    {
		    case (MLX_CMP_EQUALS): exp_internal_WriteText(eg, " == "); break;
		    case (MLX_CMP_GREATER): exp_internal_WriteText(eg, " > "); break;
		    case (MLX_CMP_LESS): exp_internal_WriteText(eg, " < "); break;
		    case (MLX_CMP_LESS | MLX_CMP_EQUALS): exp_internal_WriteText(eg, " <= "); break;
		    case (MLX_CMP_GREATER | MLX_CMP_EQUALS): exp_internal_WriteText(eg, " >= "); break;
		    case (MLX_CMP_GREATER | MLX_CMP_LESS): exp_internal_WriteText(eg, " != "); break;
		    default:
		        mssError(1,"EXP","Generator - invalid compare type in expression.");
			return -1;
		    }
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_IN:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " IN ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_CONTAINS:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " CONTAINS ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_LIKE:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " LIKE ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_ISNULL:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " IS NULL");
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_NOT:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
		exp_internal_WriteText(eg, "NOT ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_AND:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " AND ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_OR:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " OR ");
	        if (exp_internal_GenerateText_cxsql((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_TRUE:
		exp_internal_WriteText(eg, "1");
		break;

	    case EXPR_N_FALSE:
		exp_internal_WriteText(eg, "0");
		break;

	    case EXPR_N_INTEGER:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_INTEGER, &(exp->Integer), 0));
		break;

	    case EXPR_N_STRING:
	        if (eg->EscChar == '"')
		    exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_STRING, exp->String, DATA_F_QUOTED | DATA_F_SINGLE));
		else
		    exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_STRING, exp->String, DATA_F_QUOTED));
		break;

	    case EXPR_N_DOUBLE:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_DOUBLE, &(exp->Types.Double), 0));
		break;

	    case EXPR_N_DATETIME:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_DATETIME, &(exp->Types.Date), DATA_F_QUOTED));
		break;

	    case EXPR_N_MONEY:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_MONEY, &(exp->Types.Money), 0));
		break;
	    
	    case EXPR_N_OBJECT:
	        if (exp->ObjID == -1 && exp->Name) exp_internal_WriteText(eg, exp->Name);
		break;

	    case EXPR_N_PROPERTY:
	        switch(exp->ObjID)
		    {
		    case -1: break;
		    case EXPR_OBJID_CURRENT: break;
		    case EXPR_OBJID_PARENT: exp_internal_WriteText(eg, ":"); break;
		    default: 
		        if (exp->ObjID >= 0) 
			    {
			    exp_internal_WriteText(eg, ":");
		            exp_internal_WriteText(eg, eg->Objlist->Names[expObjID(exp,eg->Objlist)]);
			    }
			break;
		    }
		exp_internal_WriteText(eg,":");
		exp_internal_WriteText(eg,exp->Name);
		break;

	    case 0:  /** NULL **/
		exp_internal_WriteText(eg, " NULL ");
		break;

	    default:
	        mssError(1,"EXP","Bark!  Generator - Unknown expression node type %d", exp->NodeType);
		return -1;
	    }

	/** If a domain declaration, add the closing parenthesis **/
	if (exp->Flags & EXPR_F_DOMAINMASK)
	    exp_internal_WriteText(eg,")");

    return 0;
    }


/*** expGenerateText_js - converts an expression tree into JavaScript format
 *** for eventual embedding in a DHTML page.
 ***
 *** Note that the following support functions are *required* for these expressions
 *** to work properly:
 ***
 ***         cxjs_indexof(array,element) - finds an element in an array
 ***         cxjs_likematch(string,patternstring) - do a "LIKE" match comparison
 ***/
int
exp_internal_GenerateText_js(pExpression exp, pExpGen eg)
    {
    int i;
    int prop_func;
    int nodetype;

	/** Check recursion **/
	if (thExcessiveRecursion())
	    {
	    mssError(1,"EXP","Failed to generate JS expression: resource exhaustion occurred");
	    return -1;
	    }

	/** Treat some expressions as constants **/
	nodetype = exp_internal_CheckConstants(exp, eg);

	if ((exp->Flags & EXPR_F_PERMNULL) || nodetype == 0)
	    {
	    exp_internal_WriteText(eg, " null ");
	    return 0;
	    }

	/** Select an expression type **/
	switch(nodetype)
	    {
	    case EXPR_N_FUNCTION:
	        /** Function node - write function call, param list, end paren. **/
		if ((!strcmp(exp->Name,"runclient") || !strcmp(exp->Name,"runserver") || !strcmp(exp->Name,"runstatic")) && exp->Children.nItems == 1)
		    return exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg);
	        snprintf(eg->TmpBuf,sizeof(eg->TmpBuf),"cxjs_%.250s(",exp->Name);
		exp_internal_WriteText(eg, eg->TmpBuf);
		for(i=0;i<exp->Children.nItems;i++)
		    {
		    if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[i]), eg) < 0) return -1;
		    if (i != exp->Children.nItems-1)
		        {
		        exp_internal_WriteText(eg, ",");
			}
		    }
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_LIST:
	        /** List node - write paren, list items, end paren.  In javascript, this
		 ** amounts to an array, so write it as an array.
		 **/
		exp_internal_WriteText(eg, "(new Array(");
		for(i=0;i<exp->Children.nItems;i++)
		    {
		    if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[i]), eg) < 0) return -1;
		    if (i != exp->Children.nItems-1)
		        {
		        exp_internal_WriteText(eg, ",");
			}
		    }
		exp_internal_WriteText(eg, "))");
		break;

	    case EXPR_N_MULTIPLY:
		/** FIXME: precedence for standard cxsql expressions is not necessarily
		 ** the same as precedence of ops for JavaScript expressions.
		 **/
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " * ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_DIVIDE:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " / ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_PLUS:
		/** rules for string concat are different in javascript and cxsql **/
		exp_internal_WriteText(eg, "cxjs_plus(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ", ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_MINUS:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " - ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_COMPARE:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		switch(exp->CompareType)
		    {
		    case (MLX_CMP_EQUALS): exp_internal_WriteText(eg, " == "); break;
		    case (MLX_CMP_GREATER): exp_internal_WriteText(eg, " > "); break;
		    case (MLX_CMP_LESS): exp_internal_WriteText(eg, " < "); break;
		    case (MLX_CMP_LESS | MLX_CMP_EQUALS): exp_internal_WriteText(eg, " <= "); break;
		    case (MLX_CMP_GREATER | MLX_CMP_EQUALS): exp_internal_WriteText(eg, " >= "); break;
		    case (MLX_CMP_GREATER | MLX_CMP_LESS): exp_internal_WriteText(eg, " != "); break;
		    default:
		        mssError(1,"EXP","Generator - invalid compare type in expression.");
			return -1;
		    }
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_IN:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
		exp_internal_WriteText(eg, " cxjs_indexof(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ",");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ") >= 0");
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_CONTAINS:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ".indexOf(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ") >= 0");
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_LIKE:
		exp_internal_WriteText(eg, " cxjs_likematch(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ",");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_ISNULL:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " == null");
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_NOT:
		exp_internal_WriteText(eg, "!");
		exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_AND:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " && ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_OR:
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, "(");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		exp_internal_WriteText(eg, " || ");
	        if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[1]), eg) < 0) return -1;
		if (exp->Parent && EXP.Precedence[exp->Parent->NodeType] < EXP.Precedence[exp->NodeType])
		    exp_internal_WriteText(eg, ")");
		break;

	    case EXPR_N_TRUE:
		exp_internal_WriteText(eg, "true");
		break;

	    case EXPR_N_FALSE:
		exp_internal_WriteText(eg, "false");
		break;

	    case EXPR_N_INTEGER:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_INTEGER, &(exp->Integer), 0));
		break;

	    case EXPR_N_STRING:
	        if (eg->EscChar == '"')
		    exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_STRING, exp->String, DATA_F_QUOTED | DATA_F_SINGLE | DATA_F_CONVSPECIAL));
		else
		    exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_STRING, exp->String, DATA_F_QUOTED | DATA_F_CONVSPECIAL));
		break;

	    case EXPR_N_DOUBLE:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_DOUBLE, &(exp->Types.Double), 0));
		break;

	    case EXPR_N_DATETIME:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_DATETIME, &(exp->Types.Date), DATA_F_QUOTED));
		break;

	    case EXPR_N_MONEY:
	        exp_internal_WriteText(eg, objDataToStringTmp(DATA_T_MONEY, &(exp->Types.Money), 0));
		break;
	    
	    case EXPR_N_OBJECT:
		prop_func = 0;
	        if (exp->Children.nItems != 1) return -1;
	        if (exp->ObjID == -1 && exp->Name) 
		    {
		    if (((pExpression)(exp->Children.Items[0]))->NodeType == EXPR_N_PROPERTY)
			prop_func = 1;
		    if (prop_func)
			exp_internal_WriteText(eg, "wgtrGetProperty(");
		    exp_internal_WriteText(eg, "wgtrGetNode(_context,\"");
		    exp_internal_WriteText(eg, exp->Name);
		    exp_internal_WriteText(eg, "\")");
		    if (prop_func)
			exp_internal_WriteText(eg, ",\"");
		    else
			exp_internal_WriteText(eg, ".");
		    }
		if (exp_internal_GenerateText_js((pExpression)(exp->Children.Items[0]), eg) < 0) return -1;
		if (prop_func)
		    exp_internal_WriteText(eg, "\")");
		break;

	    case EXPR_N_PROPERTY:
	        switch(exp->ObjID)
		    {
		    case -1: break;
		    case EXPR_OBJID_CURRENT: break;
		    case EXPR_OBJID_PARENT: exp_internal_WriteText(eg, "this."); break;
		    default: 
		        if (exp->ObjID >= 0)
			    {
			    if (eg->Objlist) 
				exp_internal_WriteText(eg, eg->Objlist->Names[expObjID(exp,eg->Objlist)]);
			    else
				exp_internal_WriteText(eg, exp->Name);
			    }
			break;
		    }
		if (!exp->Parent || exp->Parent->NodeType != EXPR_N_OBJECT)
		    {
		    exp_internal_WriteText(eg,"((typeof _this.");
		    exp_internal_WriteText(eg,exp->Name);
		    exp_internal_WriteText(eg," != 'undefined')?(_this.");
		    exp_internal_WriteText(eg,exp->Name);
		    exp_internal_WriteText(eg,"):null)");
		    }
		else
		    exp_internal_WriteText(eg,exp->Name);
		break;

	    default:
	        mssError(1,"EXP","Bark!  Generator - Unknown expression node type %d", exp->NodeType);
		return -1;
	    }

    return 0;
    }



/*** expGenerateText - converts an expression tree back to its textual
 *** representation.  Generates the text to a given write_fn with a given
 *** context parameter.  Can be used with fdWrite/objWrite, or with a 
 *** custom write function which could, for instance, write the text
 *** into a buffer.  Allows a quote character to be specified; if given,
 *** any generated text matching that character will be prefixed with a
 *** backslash (\); this is useful when the output text will be quoted as
 *** a string.  In this case, any otherwise normally-occurring backslashes
 *** in the output will also be escaped with a backslash.
 ***
 *** Normal values of quote_char are \0, ', and ".
 ***
 *** Currently supported languages: 
 ***
 ***         CXSQL (Centrallix SQL style expressions)
 ***         JavaScript (JavaScript style expressions for DHTML embedding)
 ***
 *** "domain" can be 0 to generate the full expression, or EXPR_F_RUNCLIENT
 *** to constant-ize any runserver() or runstatic() subexpressions.  If the
 *** domain is EXPR_F_RUNSERVER, only any runstatic() subexpressions are
 *** converted to constants.
 ***/
int
expGenerateText(pExpression exp, pParamObjects objlist, int (*write_fn)(), void* write_arg, char quote_char, char* language, int domain)
    {
    pExpGen eg;

	/** Allocate a structure for this **/
	eg = (pExpGen)nmMalloc(sizeof(ExpGen));
	if (!eg) return -1;
	eg->Objlist = objlist;
	eg->WriteFn = write_fn;
	eg->WriteArg = write_arg;
	eg->EscChar = quote_char;
	eg->Domain = domain;

	/** Call the internal recursive version of this function **/
	if (!strcasecmp(language,"cxsql"))
	    {
	    if (exp_internal_GenerateText_cxsql(exp, eg) < 0)
		{
		nmFree(eg,sizeof(ExpGen));
		return -1;
		}
	    }
	else if (!strcmp(language,"javascript"))
	    {
	    if (exp_internal_GenerateText_js(exp, eg) < 0)
		{
		nmFree(eg,sizeof(ExpGen));
		return -1;
		}
	    }
	else
	    {
	    mssError(1,"EXP","Unknown language '%s' requested for expression generation", language);
	    nmFree(eg,sizeof(ExpGen));
	    return -1;
	    }

	/** Free the structure **/
	nmFree(eg,sizeof(ExpGen));

    return 0;
    }


int
exp_internal_AddPropToList(pXArray objs_xa, pXArray props_xa, char* objname, char* propname)
    {
    int i;
    char* objn = NULL;
    char* propn = NULL;

	/** Check dups **/
	for(i=0;i<objs_xa->nItems;i++)
	    {
	    objn = (char*)(objs_xa->Items[i]);
	    propn = (char*)(props_xa->Items[i]);
	    if (((!objn && !objname) || (objn && objname && !strcmp(objn, objname))) &&
		((!propn && !propname) || (propn && propname && !strcmp(propn, propname))))
		return -1;
	    }

	/** add it **/
	if (objname)
	    objn = nmSysStrdup(objname);
	else
	    objn = NULL;
	if (propname)
	    propn = nmSysStrdup(propname);
	else
	    propn = NULL;
	xaAddItem(props_xa, propn);
	xaAddItem(objs_xa, objn);

    return 0;
    }


/*** expGetPropList - get a list of object/property names referenced in a given
 *** expression tree.  Returns the list in a pair of xarrays - one for the
 *** object names, the second with the property names.  The caller must init
 *** the xarrays, and must deinit them afterwards.  The strings referenced in
 *** the list MUST be freed with nmSysFree() by the caller.
 ***/
int
expGetPropList(pExpression exp, pXArray objs_xa, pXArray props_xa)
    {
    pExpression subexp;
    int i;
    char* objn = NULL;
    char* propn = NULL;

	/** Is node an object/property node? **/
	if (exp->NodeType == EXPR_N_OBJECT)
	    {
	    subexp = (pExpression)(exp->Children.Items[0]);
	    objn = exp->Name;
	    if (subexp && subexp->NodeType == EXPR_N_PROPERTY)
		propn = subexp->Name;
	    else
		propn = NULL;
	    exp_internal_AddPropToList(objs_xa, props_xa, objn, propn);
	    }
	else if (exp->NodeType == EXPR_N_PROPERTY)
	    {
	    exp_internal_AddPropToList(objs_xa, props_xa, NULL, exp->Name);
	    }
	else
	    {
	    for(i=0;i<exp->Children.nItems;i++) expGetPropList((pExpression)(exp->Children.Items[i]), objs_xa, props_xa);
	    }

    return objs_xa->nItems;
    }

