/*

************************************************************
**
** Created by: CodeBench 0.23 (17.09.2011)
**
** Project: WacomTablet.usbfd
**
** File: Pseudo prefs program, shows GUI of any running
** commodities from one of the supported tablet.
**
** Date: 21-04-2012 00:00:02
**
** Copyright 2012-2013 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
**
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License  as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
**
** This program is distributed in the hope that it will be useful, but WITHOUT
** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
** more details.
**
** You should have received a copy of the GNU General Public License along with
** this program; if not, write to the Free Software Foundation, Inc., 51
** Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
************************************************************

*/

#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/commodities.h>

#include <exec/lists.h>

struct CommoditiesIFace * ICommodities = NULL;

//// WACOM tablet stuffs
struct wacom_features {
    const char *name;
    int pktlen;
    int x_max;
    int y_max;
    int pressure_max;
    int distance_max;
    int type;
    int device_type;
    int x_phy;
    int y_phy;
    unsigned char unit;
    unsigned char unitExpo;
};

struct wacom_device {
    uint16    idVendor;
    uint16    idDevice;
    struct wacom_features const * features;
};

extern const struct wacom_device wacom_devices[];
////

struct NewBroker myNB = {
   NB_VERSION,                        /* Library needs to know version */
   (STRPTR)"Wacom Tablet",            /* broker internal name          */
   (STRPTR)"USB Wacom Tablet Driver", /* commodity title               */
   NULL,                              /* description                   */
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

int main(void) {
   struct List iList;
   
   IExec->DebugPrintF("I'm in\n");
   IExec->NewList(&iList);

   struct Library * pLib = IExec->OpenLibrary( "commodities.library", 0L) ;
   if(!pLib)
   {
       return -10;
   }
   ICommodities = (struct CommoditiesIFace*) IExec->GetInterface(pLib, "main", 1, 0L);
   if(!ICommodities)
   {
       return -15;
   }
   
   uint32 nBrokerCount = ICommodities->CopyBrokerList(&iList);
   
   IExec->DebugPrintF("nb broker %ld\n", nBrokerCount);

    int32 nProductIndex = 0;
    int8  nFoundBroker = 0;
    int32 nError = CBERR_OK;
    CxObj * cxBroker = NULL;
    while( wacom_devices[nProductIndex].idVendor != 0 )
    {
        myNB.nb_Name = (STRPTR)wacom_devices[nProductIndex].features->name;
        if( NULL != IExec->FindName(&iList, myNB.nb_Name) )
        {
            IExec->DebugPrintF("Trying to show commodity '%s'\n", myNB.nb_Name);

            cxBroker = ICommodities->CxBroker(&myNB, &nError);
            if( NULL != cxBroker )
            {
                ICommodities->DeleteCxObjAll(cxBroker);
            }
            else
            {
                if( CBERR_DUP != nError )
                {
                    IExec->DebugPrintF("Error %ld while trying to show GUI for '%s'\n", nError, myNB.nb_Name);
                }
                else
                    nFoundBroker++;
            }
        }
        nProductIndex++;
    }

    if( !nFoundBroker )
    {
        IDOS->TimedDosRequesterTags(
            TDR_FormatString,   "Wacom driver does not seem to be loaded.\nEither no Wacom tablet is connected to the computer,\neither it is no compatible with the current driver.",
            TDR_TitleString,    "Wacom Prefs",
            TDR_GadgetString,   "Ok",
            TDR_ImageType,      TDRIMAGE_ERROR,
            TAG_DONE, TAG_END
        );
    }
   ICommodities->FreeBrokerList( &iList );
   IExec->DropInterface((struct Interface*)ICommodities);
    IExec->CloseLibrary(pLib);

   return nFoundBroker?RETURN_OK:RETURN_FAIL;
}
