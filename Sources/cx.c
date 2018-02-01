/*
Comodities support
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
   NB_VERSION,                        /* Library needs to know version */
   "Wacom Tablet",                    /* broker internal name          */
   "USB Wacom Tablet Driver",         /* commodity title               */
   "USB Wacom Graphics Tablet Config",/* description                   */
   NBU_NOTIFY | NBU_UNIQUE,           /* We want to be the only broker */
                                      /* with this name and we want to */
                                      /* be notified of any attempts   */
                                      /* to add a commodity with the   */
                                      /* same name                     */
   COF_CONSTANT,                      /* flags                         */
   0,                                 /* default priority              */
   NULL,                              /* port, will fill in            */
   0                                  /* channel (reserved)            */
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
	    mynb.nb_Name = um->features->name;
	}

    if(!(um->CXPort=um->IExec->CreateMsgPort()))
    {
        return FALSE;
    }
    um->CXsigflag = 1L << um->CXPort->mp_SigBit;

    mynb.nb_Pri = 1;
    mynb.nb_Port = um->CXPort;

    mynb.nb_Flags |= COF_SHOW_HIDE;

    if(!(um->CXBroker = um->ICommodities->CxBroker(&mynb, &error)))
    {
        ShutdownCX(um);
        return FALSE;
    }

    um->ICommodities->AttachCxObj(um->CXBroker,
               myHotKey(um,"ctrl alt w",
                        um->CXPort,
                        POPKEY_ID));

   if((error = um->ICommodities->CxObjError(um->CXBroker)))
   {
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
         um->IExec->DeleteMsgPort(um->CXPort);


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
        msgid   = um->ICommodities->CxMsgID((struct CxMsg *)msg);
        msgtype = um->ICommodities->CxMsgType((struct CxMsg *)msg);

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



