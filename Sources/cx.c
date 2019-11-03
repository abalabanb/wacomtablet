/*
 * Comodities support
 *
 * Copyright 2012-2015 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
 * Copyright 2011 Andy Broad <andy@broad.ology.org.uk>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the names of Alexandre Balaban or Andy Broad nor the names
 *     of contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
*/

#include <libraries/commodities.h>

#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/commodities.h>


#include <strings.h>
#include <stdio.h>

#include "WacomTablet.h"


#define POPKEY_ID 20


struct NewBroker mynb = {
   NB_VERSION,                                  /* Library needs to know version */
   (STRPTR)"Wacom Tablet",                      /* broker internal name          */
   (STRPTR)"USB Wacom Tablet Driver",           /* commodity title               */
   (STRPTR)"USB Wacom Graphics Tablet Config",  /* description                   */
   NBU_NOTIFY | NBU_UNIQUE,                     /* We want to be the only broker */
                                                /* with this name and we want to */
                                                /* be notified of any attempts   */
                                                /* to add a commodity with the   */
                                                /* same name                     */
   COF_CONSTANT,                                /* flags                         */
   0,                                           /* default priority              */
   NULL,                                        /* port, will fill in            */
   0                                            /* channel (reserved)            */
};



CxObj *
myHotKey(struct usbtablet *um, CONST_STRPTR descr,struct MsgPort * port,LONG ID)
{
        CxObj * result = NULL;
        CxObj * filter = NULL;
        struct CommoditiesIFace * ICommodities = um->ICommodities; // makes the macros work

        filter = CxFilter(descr);
        if(filter == NULL)
        goto out;

        um->ICommodities->AttachCxObj(filter,CxSender(port,ID));
        um->ICommodities->AttachCxObj(filter,CxTranslate(NULL));

        if(um->ICommodities->CxObjError(filter))
        goto out;

        result = filter;

 out:

        if(result == NULL && filter != NULL)
        um->ICommodities->DeleteCxObjAll(filter);

        return(result);
}



BOOL SetupCX(struct usbtablet *um)
{
    LONG error;

    ShutdownCX(um);   /* shutdown previous and restart from scratch */

    if(um->features)
    {
        mynb.nb_Name = (STRPTR)um->features->name;
    }

    if(!(um->CXPort = (struct MsgPort*)um->IExec->AllocSysObject(ASOT_PORT, NULL)))
    {
        um->IUSBSys->USBLogPuts( -1, NULL, "Cannot allocate CXPort" );
        return FALSE;
    }
    um->CXsigflag = 1L << um->CXPort->mp_SigBit;

    mynb.nb_Pri = 1;
    mynb.nb_Port = um->CXPort;

    mynb.nb_Flags |= COF_SHOW_HIDE;

    if(!(um->CXBroker = um->ICommodities->CxBroker(&mynb, &error)))
    {
        um->IUSBSys->USBLogVPrintf( -1, NULL, "CxBroker returned error %ld", (ULONG*)&error );
        ShutdownCX(um);
        return FALSE;
    }

    um->ICommodities->AttachCxObj(um->CXBroker,
                        myHotKey(um,"ctrl alt w",
                        um->CXPort,
                        POPKEY_ID));

   if((error = um->ICommodities->CxObjError(um->CXBroker)))
   {
        um->IUSBSys->USBLogVPrintf( -1, NULL, "CxObjError returned error %ld", (ULONG*)&error );
        ShutdownCX(um);
        return (FALSE);
   }
   um->ICommodities->ActivateCxObj(um->CXBroker,1);
   return TRUE;
}

void ShutdownCX(struct usbtablet *um)
{
    struct Message *msg;
    if (um->CXPort)
    {
         um->ICommodities->DeleteCxObjAll(um->CXBroker);      /* safe, even if NULL   */

         /* now that messages are shut off, clear port   */
         while((msg=um->IExec->GetMsg(um->CXPort))) um->IExec->ReplyMsg(msg);
         um->IExec->FreeSysObject(ASOT_PORT, um->CXPort);


      um->CXPort    = NULL;
      um->CXsigflag = 0;
      um->CXBroker  = NULL;

    }
}




BOOL HandleCXmsg(struct usbtablet* um)
{
    ULONG   msgid;
    ULONG   msgtype;

    BOOL    result = TRUE;
    struct Message *msg = um->IExec->GetMsg(um->CXPort);
    if(msg)
    {
        msgid   = um->ICommodities->CxMsgID((CxMsg *)msg);
        msgtype = um->ICommodities->CxMsgType((CxMsg *)msg);

        um->IExec->ReplyMsg(msg);

        switch(msgtype)
        {
            case CXM_IEVENT:
                switch(msgid)
                {
                    case POPKEY_ID:
                        OpenWindow(um);
                        break;

                    default:
                        break;
                }
                break;
            case CXM_COMMAND:
                switch(msgid)
                {
                    case CXCMD_DISABLE:
                        um->ICommodities->ActivateCxObj(um->CXBroker,0L);
                        break;
                    case CXCMD_ENABLE:
                       um->ICommodities->ActivateCxObj(um->CXBroker,1L);
                        break;
                    case CXCMD_APPEAR:  /* Time to pop up the window         */
                    case CXCMD_UNIQUE:   /* Someone has tried to run us again */

                        OpenWindow(um);
                        break;            /* the window                        */
                    case CXCMD_DISAPPEAR:
                        CloseWindow(um);
                        break;
                    case CXCMD_KILL:
                        result = FALSE;
                        break;
                    default:
                        break;
                 }
                 break;
        }
    }
    return result;
}



