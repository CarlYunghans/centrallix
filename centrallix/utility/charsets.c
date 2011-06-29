#include "charsets.h"
#include "centrallix.h"
#include <langinfo.h>

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
/* Module: 	charsets.c, charsets.h                                  */
/* Author:	Daniel Rothfus                                          */
/* Creation:	June 29, 2011                                           */
/* Description: This module provides utilities for working with the     */
/*              variable character set support built into Centrallix.   */
/************************************************************************/

char* chrGetEquivalentName(char* name)
    {
    pStructInf highestCharsetNode;
    char * charsetValue;
    
        // Try to find the requested attribute
        highestCharsetNode = stLookup(CxGlobals.CharsetMap, name);
        if(highestCharsetNode)
            {
            stAttrValue(highestCharsetNode, NULL, &charsetValue, 0);
            return charsetValue;
            }
        else
            {
            // If not, return the system's name for the charset.
            return nl_langinfo(CODESET);
            }
    }
