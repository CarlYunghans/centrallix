#ifndef _WIDGET_TREE_H
#define _WIDGET_TREE_H

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
/* 									*/
/* Module: 	widget tree     					*/
/* Author:	Matt McGill (MJM)					*/
/* Creation:	July 15, 2004   					*/
/* Description:	Provides an interface for creating and manipulating  	*/
/*		widget trees, primarily used in the process of rendering*/
/*		a DHTML page from an application.                      	*/
/************************************************************************/

/**CVSDATA***************************************************************

 **END-CVSDATA***********************************************************/

#include "obj.h"
#include "expression.h"

typedef struct
    {
    int		Magic;
    void*	reserved;
    int		Flags;
    char	Type[64];			/** widget type - editbox, etc. **/
    char	Name[64];			/** widget name **/
    int		r_x, r_y, r_width, r_height;	/** Requested geometry **/
    int		fl_x, fl_y, fl_width, fl_height;/** Flexibility **/
    double  	fx, fy, fw, fh;			/** internal flexibility calculations **/
    int		x, y, width, height;		/** actual geometry **/
    int		top, bottom, left, right;	/** container offsets **/
    XArray	Properties;			/** Array of widget properties **/
    XArray	Children;			/** Array of child widgets **/
    int		CurrProperty;			/** Property to return on next call to wgtNextProperty **/
    int		CurrChild;			/** Child to return on next call to wgtrNextChild **/
    int		extra;
    } 
    WgtrNode, *pWgtrNode;

typedef struct
    {
    int		Magic;		    /** Magic number **/
    WgtrNode	Tree;		    /** Widget tree for this iterator **/
    int		TraversMethod;	    /** WGTR_TM_XXX **/
    WgtrNode	CurrNode;	    /** Next node to be returned **/
    XArray	BacktrackStack;	    /** Keep track of parents for backtracking **/
    XArray	VistQueue;	    /** For keeping track of nodes to visit **/
    } 
    WgtrIterator, *pWgtrIterator;

#define WGTR_TM_LEVELORDER	1
#define	WGTR_TM_PREORDER	2
#define WGTR_TM_POSTORDER	3

pWgtrNode wgtrParseObject(pObjSession s, char* path, int mode, int permission_mask, char* type);  /** parse osml object **/
pWgtrNode wgtrParseOpenObject(pObject obj);	/** parses an open OSML object into a widget tree **/
void wgtrFree(pWgtrNode tree);	/** frees memory associated with a widget tree **/
pWgtrIterator wgtrGetIterator(pWgtrNode tree, int traversal_type);	/** returns an iterator for the tree **/
pWgtrNode wgtrNext(pWgtrIterator itr);	/** return next node in tree **/
void wgtrFreeIterator(pWgtrIterator itr);	/** frees memory associated with a widget tree **/
int wgtrGetPropertyType(pWgtrNode widget, char* name);	/** get the type of the given property **/
int wgtrGetPropertyValue(pWgtrNode widget, char* name, int datatype, pObjData val); /** get property value **/
char* wgtrFirstPropertyName(pWgtrNode widget);	/** returns name of first property in property array **/
char* wgtrNextPropertyName(pWgtrNode widget);	/** returns next name in property array **/
int wgtrAddProperty(pWgtrNode widget, char* name, int datatype, pObjData val);	/** add a property to the widget **/
int wgtrDeleteProperty(pWgtrNode widget, char* name);	/** deletes the property from the widget **/
int wgtrSetProperty(pWgtrNode widget, char* name, int datatype, pObjData val);	/** sets the widget property val **/
pWgtrNode wgtrNewNode(	char* name, char* type, 
			int rx, int ry, int rwidth, int rheight,
			int flx, int fly, int flwidth, int flheight);   /** create a new widget node **/
int wgtrDeleteChild(pWgtrNode widget, char* child_name);    /** deletes child widget from tree, along w/ sub-widgets **/
int wgtrAddChild(pWgtrNode widget, pWgtrNode child);	    /** graft one tree onto another **/
pWgtrNode wgtrNextChild(pWgtrNode tree);	/** return the next child **/
pWgtrNode wgtrFirstChild(pWgtrNode tree);	/** return the first child **/
pObjPresentationHints wgtrWgtToHints(pWgtrNode widget);	/** mimick objObjToHints **/
pExpression wgtrGetExpr(pWgtrNode widget, char* attrname);	/** Get an expression from a widget node **/

void wgtrPrint(pWgtrNode tree, int indent);	/** for debug purposes **/

#endif
