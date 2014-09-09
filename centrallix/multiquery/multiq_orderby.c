#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include "obj.h"
#include "cxlib/mtlexer.h"
#include "expression.h"
#include "cxlib/xstring.h"
#include "multiquery.h"
#include "cxlib/mtsession.h"
#include "mergesort.h"


/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1999-2010 LightSys Technology Services, Inc.		*/
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
/* Module: 	multiq_orderby.c 	 				*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	June 24, 2010    					*/
/* Description:	Provides support for high-level ORDER BY.		*/
/************************************************************************/



/*** High-level ORDER BY
 ***
 *** This is needed when processing queries whose order by clauses cannot
 *** be satisfied by a simple request for ordering from one of the data
 *** sources.  Here are some examples:
 ***
 *** - Ordering by an aggregate field (generated by multiq_tablegen)
 *** - Ordering by a multi-source computed field (generated by a join)
 *** - Ordering by a field from a source that can't be first in join order
 *** - Ordering by a field from a multi-object source (such as a SUBTREE
 ***   or WILDCARD data source)
 ***/

struct
    {
    pQueryDriver	BeforeGroupDriver;
    pQueryDriver	AfterGroupDriver;
    }
    MQOBINF;


/** keeping track of an object to order **/
typedef struct
    {
    XString		OrderBuf;
    pParamObjects	ObjList;
    }
    MqobOrderable, *pMqobOrderable;


/** context structure **/
typedef struct
    {
    XArray		Objects;
    int			IterCnt;
    int			nOrderBy;
    }
    MQOData, *pMQOData;


/*** mqobAnalyze - take a given query syntax structure, look for any group by
 *** or order by items that need ordering at a higher level than a single 
 *** simple data sources, but not *after* grouping is performed.
 ***/
int
mqobAnalyzeBeforeGroup(pQueryStatement stmt)
    {
    pQueryStructure qs, item;
    pQueryElement qe, search_qe;
    int i,j,k;
    int src_idx;
    int mask;
    int n_orderby = 0;
    int total_mask = 0;
    int n_sources;
    int n_sources_total;
    int non_simple;
    int clauses[2] = {MQ_T_GROUPBYCLAUSE, MQ_T_ORDERBYCLAUSE};

	/** Allocate a new query-element **/
	qe = mq_internal_AllocQE();
	qe->Driver = MQOBINF.BeforeGroupDriver;

    	/** Look for the GROUP BY and ORDER BY clause... **/
	for(k=0; k<2; k++)
	    {
	    if ((qs = mq_internal_FindItem(stmt->QTree, clauses[k], NULL)) != NULL)
		{
		for(i=0;i<qs->Children.nItems;i++)
		    {
		    item = (pQueryStructure)(qs->Children.Items[i]);
		    if (item->Expr && item->Expr->AggLevel == 0)
			{
			mask = item->Expr->ObjCoverageMask;
			total_mask |= mask;
			}
		    }
		}
	    }
	mask = total_mask;
	for(n_sources_total = 0; mask; mask >>= 1)
	    n_sources_total += (mask & 0x01);
	for(k=0; k<2; k++)
	    {
	    if ((qs = mq_internal_FindItem(stmt->QTree, clauses[k], NULL)) != NULL)
		{
		for(i=0;i<qs->Children.nItems;i++)
		    {
		    item = (pQueryStructure)(qs->Children.Items[i]);
		    if (item->Expr && item->Expr->AggLevel == 0)
			{
			mask = item->Expr->ObjCoverageMask;
			for(n_sources = 0; mask; mask >>= 1)
			    n_sources += (mask & 0x01);
			non_simple = 0;
			if (n_sources == 1)
			    {
			    /** One source.  However, we'll still grab this one if it 
			     ** uses WILDCARD or SUBTREE (i.e., a "non-simple" source)
			     **/
			    mask = item->Expr->ObjCoverageMask;
			    src_idx = 0;
			    while((mask & 1) == 0)
				{
				mask >>= 1;
				src_idx++;
				}
			    for(j=0;j<stmt->Trees.nItems;j++)
				{
				search_qe = (pQueryElement)(stmt->Trees.Items[j]);
				if (search_qe->QSLinkage != NULL && search_qe->SrcIndex == src_idx)
				    {
				    if (search_qe->Flags & (MQ_EF_WILDCARD | MQ_EF_FROMSUBTREE))
					{
					non_simple = 1;
					break;
					}
				    }
				}
			    }
			if ((n_sources > 1 || non_simple || n_sources_total > 1) && n_orderby < 24)
			    {
			    /** Grab this one **/
			    qe->OrderBy[n_orderby++] = exp_internal_CopyTree(item->Expr);
			    }
			}
		    }
		}
	    }

	/** Did we find any orderby items? **/
	if (n_orderby)
	    {
	    printf("using MQ orderby module.\n");

	    /** Link the qe into the multiquery **/
	    qe->OrderBy[n_orderby] = NULL;
	    xaAddItem(&stmt->Trees, qe);
	    xaAddItem(&qe->Children, stmt->Tree);
	    stmt->Tree->Parent = qe;
	    stmt->Tree = qe;
	    }
	else
	    {
	    mq_internal_FreeQE(qe);
	    }

    return 0;
    }


/*** mqobAnalyze - take a given query syntax structure, look for any order by
 *** items that need to take place *after* grouping (e.g., they contain an
 *** aggregate function)
 ***/
int
mqobAnalyzeAfterGroup(pQueryStatement stmt)
    {
    pQueryStructure qs, item;
    pQueryElement qe, child;
    int i, n_orderby = 0;

	/** Allocate a new query-element **/
	qe = mq_internal_AllocQE();
	qe->Driver = MQOBINF.AfterGroupDriver;

	/** Look for an ORDER BY clause **/
	if ((qs = mq_internal_FindItem(stmt->QTree, MQ_T_ORDERBYCLAUSE, NULL)) != NULL)
	    {
	    /** Look for ORDER BY items with an Aggregate Level of 1 **/
	    for(i=0;i<qs->Children.nItems;i++)
		{
		item = (pQueryStructure)(qs->Children.Items[i]);
		if (item->Expr && item->Expr->AggLevel == 1 && n_orderby < 24)
		    {
		    /** Found one.  Squirrel it away in our order-by list. **/
		    qe->OrderBy[n_orderby++] = exp_internal_CopyTree(item->Expr);
		    }
		}
	    }

	/** Did we find any orderby items? **/
	if (n_orderby)
	    {
	    qe->OrderBy[n_orderby] = NULL;
	    printf("using MQ after-grouping orderby module.\n");

	    /** Set up the attribute list, since we may be the top-level node
	     ** in the query execution tree.
	     **/
	    child = stmt->Tree;
	    for(i=0;i<child->AttrNames.nItems;i++)
		{
		xaAddItem(&qe->AttrNames, child->AttrNames.Items[i]);
		xaAddItem(&qe->AttrExprPtr, child->AttrExprPtr.Items[i]);
		xaAddItem(&qe->AttrCompiledExpr, child->AttrCompiledExpr.Items[i]);
		xaAddItem(&qe->AttrDeriv, child->AttrDeriv.Items[i]);
		}

	    /** Link the qe into the multiquery **/
	    xaAddItem(&stmt->Trees, qe);
	    xaAddItem(&qe->Children, child);
	    child->Parent = qe;
	    stmt->Tree = qe;
	    }
	else
	    {
	    mq_internal_FreeQE(qe);
	    }

    return 0;
    }


/*** mqobStart - just trickle down the Start operation.  We'll do the actual
 *** sorting once the higher level calls NextItem.
 ***/
int
mqobStart(pQueryElement qe, pQueryStatement stmt, pExpression additional_expr)
    {
    pQueryElement cld;
    int rval = -1;

	/** Now, 'trickle down' the Start operation to the child item(s). **/
	cld = (pQueryElement)(qe->Children.Items[0]);
	if (cld->Driver->Start(cld, stmt, NULL) < 0) 
	    {
	    mssError(0,"MQOB","Failed to start child join/projection operation");
	    goto error;
	    }

	rval = 0;

    error:
	return rval;
    }


/*** Comparison function for msMergeSort() to use ***/
int
mqob_internal_CompareItems(void* item_a, void* item_b)
    {
    pMqobOrderable a = (pMqobOrderable)item_a;
    pMqobOrderable b = (pMqobOrderable)item_b;
    int rval;
    int cmplen;

	/** Compare using memcmp() **/
	cmplen = a->OrderBuf.Length;
	if (cmplen > b->OrderBuf.Length) cmplen = b->OrderBuf.Length;
	rval = memcmp(a->OrderBuf.String, b->OrderBuf.String, cmplen);

	/** Same?  Then check length comparison **/
	if (rval == 0)
	    {
	    if (a->OrderBuf.Length > cmplen)
		rval = 1;
	    else if (b->OrderBuf.Length > cmplen)
		rval = -1;
	    }

    return rval;
    }


/*** mqobNextItem - the first time this is called, we iterate through all of the
 *** child QE's results, then sort them, then return the first one.  After that,
 *** we return one result per call (rval 1), then rval 0 when there are no more.
 ***/
int
mqobNextItem(pQueryElement qe, pQueryStatement stmt)
    {
    pQueryElement cld;
    pMQOData context;
    pMqobOrderable item;
    pParamObjects objlist;
    int n;
    int rval = -1;

	cld = (pQueryElement)(qe->Children.Items[0]);

	/** Have we gotten the sorted list yet? **/
	if (qe->PrivateData == NULL)
	    {
	    /** Init our private data result list **/
	    context = (pMQOData)nmMalloc(sizeof(MQOData));
	    qe->PrivateData = context;
	    xaInit(&context->Objects, 16);
	    for(n=0;n<25;n++)
		{
		if (!qe->OrderBy[n])
		    {
		    context->nOrderBy = n;
		    break;
		    }
		}
	    context->IterCnt = 0;

	    /** Loop through the child QE's results and build our unsorted list **/
	    while ((rval = cld->Driver->NextItem(cld, stmt)) == 1)
		{
		item = (pMqobOrderable)nmMalloc(sizeof(MqobOrderable));
		objlist = expCreateParamList();
		if (!objlist || !item) goto error;
		expCopyList(stmt->Query->ObjList, objlist, -1);
		item->ObjList = objlist;
		expLinkParams(objlist, stmt->Query->nProvidedObjects, -1);
		xsInit(&item->OrderBuf);
		xaAddItem(&context->Objects, item);
		if (objBuildBinaryImageXString(&item->OrderBuf, qe->OrderBy, context->nOrderBy, item->ObjList) < 0)
		    goto error;
		}
	    if (rval < 0) goto error;

	    /** Sort the results **/
	    msMergeSort(context->Objects.Items, context->Objects.nItems, mqob_internal_CompareItems);
	    }
	else
	    {
	    context = qe->PrivateData;
	    }

	/** Ok, got the sorted list.  Do we have items remaining? **/
	if (context->IterCnt >= context->Objects.nItems)
	    return 0;

	/** Close the previous item **/
	expUnlinkParams(stmt->Query->ObjList, stmt->Query->nProvidedObjects, -1);

	/** Copy in the next item **/
	item = context->Objects.Items[context->IterCnt];
	objlist = item->ObjList;
	expCopyList(objlist, stmt->Query->ObjList, -1);
	expFreeParamList(objlist);
	item->ObjList = NULL;
	rval = 1;
	context->IterCnt++;

    error:

    return rval;
    }


/*** mqobFinish - clean up.
 ***/
int
mqobFinish(pQueryElement qe, pQueryStatement stmt)
    {
    pMQOData context = (pMQOData)(qe->PrivateData);
    pMqobOrderable item;
    pParamObjects objlist;
    pQueryElement cld;
    int i;

	cld = (pQueryElement)(qe->Children.Items[0]);

	/** Close the previous item **/
	expUnlinkParams(stmt->Query->ObjList, stmt->Query->nProvidedObjects, -1);

	if (context)
	    {
	    /** Did we get interrupted in the middle of the list?  Free 'em **/
	    for(i=0; i<context->Objects.nItems; i++)
		{
		item = (pMqobOrderable)(context->Objects.Items[i]);
		if (item)
		    {
		    if (item->ObjList)
			{
			objlist = item->ObjList;
			expUnlinkParams(objlist, stmt->Query->nProvidedObjects, -1);
			expFreeParamList(objlist);
			}
		    xsDeInit(&item->OrderBuf);
		    nmFree(item, sizeof(MqobOrderable));
		    }
		}
	    xaDeInit(&context->Objects);
	    nmFree(context, sizeof(MQOData));
	    qe->PrivateData = NULL;
	    }

	cld->Driver->Finish(cld, stmt);

    return 0;
    }


/*** mqobRelease - clean up after mqobAnalyze()
 ***/
int
mqobRelease(pQueryElement qe, pQueryStatement stmt)
    {
    return 0;
    }


/*** mqobInitialize - initialize this module and register with the multi-
 *** query system.  We register two drivers here, the first one does
 *** ordering *before* GROUP BY, the second one does ordering *after*
 *** grouping/aggregate is performed.
 ***/
int
mqobInitialize()
    {
    pQueryDriver drv;

    	/** Allocate the driver descriptor structure **/
	drv = (pQueryDriver)nmMalloc(sizeof(QueryDriver));
	if (!drv) return -1;
	memset(drv,0,sizeof(QueryDriver));

	/** Fill in the structure elements **/
	strcpy(drv->Name, "MQOB - MultiQuery ORDER BY Module - Before Grouping");
	drv->Precedence = 3500;
	drv->Flags = 0;
	drv->Analyze = mqobAnalyzeBeforeGroup;
	drv->Start = mqobStart;
	drv->NextItem = mqobNextItem;
	drv->Finish = mqobFinish;
	drv->Release = mqobRelease;

	/** Register with the multiquery system. **/
	if (mqRegisterQueryDriver(drv) < 0) return -1;
	MQOBINF.BeforeGroupDriver = drv;

    	/** Allocate the driver descriptor structure **/
	drv = (pQueryDriver)nmMalloc(sizeof(QueryDriver));
	if (!drv) return -1;
	memset(drv,0,sizeof(QueryDriver));

	/** Fill in the structure elements **/
	strcpy(drv->Name, "MQOB - MultiQuery ORDER BY Module - After Grouping");
	drv->Precedence = 4500;
	drv->Flags = 0;
	drv->Analyze = mqobAnalyzeAfterGroup;
	drv->Start = mqobStart;
	drv->NextItem = mqobNextItem;
	drv->Finish = mqobFinish;
	drv->Release = mqobRelease;

	/** Register with the multiquery system. **/
	if (mqRegisterQueryDriver(drv) < 0) return -1;
	MQOBINF.AfterGroupDriver = drv;

    return 0;
    }
