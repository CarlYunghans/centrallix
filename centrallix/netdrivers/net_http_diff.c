/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1998-2011 LightSys Technology Services, Inc.		*/
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
/* Module: 	net_http.h, net_http.c, net_http_conn.c, net_http_sess.c, net_http_osml.c, net_http_app.c, net_http_diff.c*/
/* Author:	Micah Shennum					*/
/* Creation:	July 28, 2011  					*/
/* Description:	Network handler providing updates to clients.			*/
/************************************************************************/

#include "obj.h"
#include "net_http.h"

pXArray nht_internal_DecomposeSQL(pObjSession session, char *sql){
    int i;
    pObject obj;
    pXArray cache;
    pXArray names;
    pObjQuery query;
    
    query = objMultiQuery(session, sql, NULL, 0);
    if(!query)return NULL;
    cache = objMultiQueryObjects(query);
    names = nmMalloc(sizeof(XArray));
    xaInit(names,xaCount(cache));
    for(i=0;i<xaCount(cache);i++){
        obj = xaGetItem(cache,i);
        xaAddItem(names, objGetPathname(obj));
    }
    objQueryClose(query);
    return names;
}

pNhtUpdate nht_internal_CreateUpdates(){
    pNhtUpdate update = nmMalloc(sizeof(NhtUpdate));
    update->Saved = nmMalloc(sizeof(XHashTable));
    xhInit(update->Saved,16,sizeof(XTree));
    update->Notifications = nmMalloc(sizeof(XHashTable));
    xhInit(update->Notifications,8,0);
    update->NotificationNames = nmMalloc(sizeof(XArray));
    xaInit(update->NotificationNames,8);
    return update;
}//end nht_internal_createUpdates

void nht_internal_FreeUpdates(pNhtUpdate update){
    xaDeInit(update->NotificationNames);
    nmFree(update->NotificationNames,sizeof(XArray));
    xhDeInit(update->Notifications);
    nmFree(update->Notifications,sizeof(XHashTable));
    xhDeInit(update->Saved);
    nmFree(update->Saved,sizeof(XHashTable));
    return;
}//end nht_internal_freeUpdates

void nht_internal_DiffArrays(pXTree prevous, pXTree results, pFile fd){

}//end nht_internal_diffArrays

///@brief fetches the result of a SQL statment as would be seen by the http client
pXTree nht_internal_FetchSQL(char *sql, pObjSession session, pNhtConn conn){
    int rowid;
    char *buff;
    pObject obj;
    pXTree results;
    pObjQuery query;
    pFile savedConn, sendCon, recvCon;
    
    //save the real fd
    savedConn = conn->ConnFD;
    //open pipe
    fdPipe(&sendCon,&recvCon);
    //and redirect
    conn->ConnFD = sendCon;

    //open the query
    query = objMultiQuery(session, sql, NULL, 0);
    results = nmMalloc(sizeof(pXTree));
    xtInit(results,0);

    //now get results
    buff = nmSysMalloc(1024);
    rowid = 0;
    while((obj=objQueryFetch(query,O_RDONLY))){
        nht_internal_WriteAttrs(obj,conn,0,1);
        objClose(obj);
        fdRead(recvCon,buff,1024,0,0);
        //essentially random value, since we never need it again
        xtAdd(results,nmSysStrdup(buff),(void *)0xdeafcab9);
    }
    objQueryClose(query);
    //return to regularly scheduled broadcast
    conn->ConnFD = savedConn;
    //clean up!
    nmSysFree(buff);
    fdClose(sendCon,FD_U_IMMEDIATE);
    fdClose(recvCon,FD_U_IMMEDIATE);
    return results;
}//end nht_internal_FetchSQL

static int freeRow(char *data, void *hints){
    nmSysFree(data);
    return 0;
}

//frees array of strings, as from above
void nht_internal_FreeResults(pXTree results){
    xtClear(results,freeRow,NULL);
    xtDeInit(results);
    nmFree(results,sizeof(XTree));
}//end nht_internal_FreeResults

int nht_internal_writeUpdate(pNhtConn conn, pNhtUpdate updates, pObjSession session, char *path){
    char *sql;
    pXTree results;
    pXTree prevous;

    //open the query
    sql = xtLookupBeginning(updates->Querys,path);
    results = nht_internal_FetchSQL(sql, session, conn);

    //now diff these things
    prevous = (pXTree)xhLookup(updates->Saved,sql);
    nht_internal_DiffArrays(prevous, results, conn->ConnFD);

    //drop last results and save these
    if(prevous)xhRemove(updates->Saved,sql);
    xhAdd(updates->Saved,sql,(char *)results);

    if(prevous)
        nht_internal_FreeResults(prevous);
    return 0;
}

/* We get:
     * ls__sid
     * ls__sql#
     * ls__rowcount#
     * ls__notify#
     * ls__req -- multiquery?
     * ls__autofetch
     * ls__autoclose_sr
     */
int nht_internal_GetUpdates(pNhtConn conn,pStruct url_inf){
    int i;
    int reqc;
    char *sid;
    char *sql;
    pXArray fetch;
    pXTree results;
    char name[0xff];
    pXArray waitfor;
    pXArray sqlobjects;
    pNhtUpdate updates;
    pObjSession session;
    pObjObserver observer;
    handle_t session_handle;
    ObjObserverEventType event_t;

    /** Get the session data **///stolen from net_http_osml.c:514, but now unreconizable
    stAttrValue_ne(stLookup_ne(url_inf,"ls__sid"),&sid);
    if (!sid || !strcmp(sid,"XDEFAULT")){
        mssError(1,"NHT","Session ID required for update request");
        nht_internal_ErrorExit(conn,405,"Update request without session ID.");
        return -1;
    }
    session_handle = xhnStringToHandle(sid+1,NULL,16);
    session = (pObjSession)xhnHandlePtr(&(conn->NhtSession->Hctx), session_handle);

    ///@todo store this somewhere, since the xhnHandlePtr didn't work out
    if(!updates || updates){
        mssError(0,"NHT","Couldn't fetch list of update request!");
        updates = nht_internal_CreateUpdates();
    }

    //load saved observers
    waitfor = nmMalloc(sizeof(XArray));
    xaInit(waitfor,xaCount(updates->NotificationNames));
    for(i=0;i<xaCount(updates->NotificationNames);i++)
        xaAddItem(waitfor,
                xhLookup(updates->Notifications,
                xaGetItem(updates->NotificationNames,i)));

    //Get requested sql's
    reqc = strtoi(stLookup_ne(url_inf,"cx__numObjs")->StrVal,NULL,16);
    if(errno == ERANGE){
        mssError(0,"NHT","Imposable number of sql request: %s",
                stLookup_ne(url_inf,"cx__numObjs")->StrVal);
        reqc=0;
    }
    fetch = nmMalloc(sizeof(XArray));
    xaInit(fetch,16);
    //get all the requested queries
    for(i=0;i<reqc;i++){
        char *obj;
        int j,save;
        snprintf(name,0xff,"ls__notify%x",i);
        save=(stLookup_ne(url_inf,name)->StrVal[1]!='0');
        snprintf(name,0xff,"ls__sql%x",i);
        sql = stLookup_ne(url_inf,name)->StrVal;
        if(!save){
            xaAddItem(fetch,sql);
            continue;
        }
        sqlobjects = nht_internal_DecomposeSQL(session,sql);
        if(!sqlobjects){
            mssError(0,"NHT","Could not decompose sql statement %s",
                    stLookup_ne(url_inf,name)->StrVal);
            continue;
        }//end if can't decompose
        for(j=0;j<xaCount(sqlobjects);j++){
            observer = NULL;
            obj=xaGetItem(sqlobjects,j);
            //if we don't already have it, open a observer for it
            if(!xhLookup(updates->Notifications,obj)){
                observer=objOpenObserver(session,obj);
                if(!xaFindItem(fetch,sql))
                    xaAddItem(fetch,sql);
                xaAddItem(waitfor,observer);
                xhAdd(updates->Notifications,obj,(void *)observer);
                xaAddItem(updates->NotificationNames,obj);
                xtAdd(updates->Querys,obj,sql);
            }//end if saveable new observer
        }//end for sqlobjects
    }//end for reqc

    //send non-persistent request
    for(i=0;i<xaCount(fetch);i++){
        results = nht_internal_FetchSQL((char *)xaGetItem(fetch,i), session, conn);
        nht_internal_DiffArrays(NULL,results,conn->ConnFD);
        nht_internal_FreeResults(results);
    }

    //now that that's over, get some updates!
    event_t = objPollObservers(waitfor,xaCount(fetch),&observer,&sid);
    //write header
    //fdPrintf(conn->ConnFD,"Content-Type: text/html\r\nTransfer-Encoding: chunked\r\n\r\n");
    while(event_t != OBJ_OBSERVER_EVENT_NONE){
        nht_internal_writeUpdate(conn,updates,session,sid);
        event_t=objPollObservers(waitfor,0,&observer,&sid);
    }

    xaDeInit(waitfor);
    nmFree(waitfor,sizeof(XArray));
    return 0;
}//end of nht_internal_GetUpdates
