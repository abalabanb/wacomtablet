/*
 * Copyright 2012-2021 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
 * Copyright 2011 Andy Broad <andy@broad.ology.org.uk>
 * Copyright 2005 Rene W. Olsen <ac@rebels.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the names of Andy Broad or Rene W. Olsen nor the names
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

/*
 *  WacomTablet.usbfd by Alexandre Balaban
 * based on UCLogicTablet.usbfd by Andy Broad
 * based in turn on rMouse.usbfd by Rene W. Olsen
 * based in turn on the bootmouse driver by Thomas Graff
 *
 *   1.2    202x-xx-xx  - Added: support for IntuosHT2 range of tablets (i.e.
 *                               Intuos S 2, Intuos PT S 2, Intuos P M 2 and
 *                               Intuos PT M 2)
 *                      - Added: explicit log for unsupported multitouch
 *                               tablets and mentionned it in the readme
 *                      - Updated: makefile updated to conform new repo layout
 *                      - Fixed: crash when trying to initialize unsupported
 *                               params (yet), many models were affected, mainly
 *                               Intuos and Cintiq (reported by Remo Constantin)
 *                      - Fixed: BTN_0 in functions wacom_report_numbered_buttons
 *                               and wacom_numbered_button_to_key was ineffective
 *                      - Fixed: wheel events for Intuos3 was not working
 *                      - Fixed: add correct support for tilt (value ranges, TiltX
 *                               and TiltY were exchanged)
 *                      - Added: tilt display in commodity window
 *                      - Updated: reworked layout of current/max values
 *                      - Updated: lowered CPU load by filtering out events with
 *                                 too close values
 *                      - Added: support of actions double-click and hold-click
 *                      - Fixed: debug level was instated after setting-up tablet
 *                               capabilities, meaning no debug was ever emitted
 *                               during this phase
 *   1.1    2019-11-03  - Updated: code updated to input-wacom 0.44
 *                                 (PenPartner, DTU, DTUS, DTH1152, PL, PTU,
 *                                  Bamboo pen & touch)
 *   Intern 2015-02-xx  - Added: support for Intuos Pro tablets
 *                      - Added: support for Bamboo One tablets
 *                      - Added: support for Cintiq 13HD tablets
 *                      - Updated: code updated to input-wacom 0.21.0
 *                      - Fixed: random crash in prefs program after multiple
 *                               attach/detach cycles
 *                      - Fixed: mouse button would never get released
 *                      - Added: error message in case of commodity creation
 *                               failure
 *                      - Fixed: used prefsobjects interface version 1
 *                      - Added: Initial action handler
 *   Intern 2013-03-xx  - Added: Icons by Davebraco
 *                      - Fixed: Some compilation warnings
 *                      - Fixed: Incorrect button count displayed in prefs GUI
 *                      - Fixed: Warning about bad application.library version
 *                               tag used when saving prefs
 *                      - Updated: code updated to input-wacom 0.16.0
 *                      - Fixed: Potential crash in prefs program due to
 *                               unterminated label list
 *                      - Fixed: Replaced deprecated functions by new ones
 *                      - Added: Proper version handling mechanism
 *   1.0    2012-08-01  - Added: tablet buttons configurability
 *                      - Added: debuglevel env variable load
 *                      - Added: tablet capabilities autodetection
 *                      - Added: XML preferences file 
 *   0.6    2012-04-26  - Fixed: scroll events were only allowing forward scroll
 *                      - Fixed: wrong Project for prefs program icon
 *                      - Added: mouse and stylus button configurability
 *   0.5    2012-04-24  - Added: extracted tablet handlers to a dedicated file
 *                      - Added: extracted supported tablet tables to own file
 *                      - Added: a prefs program that shows the GUI (if running)
 *   0.4    2012-04-19  - Added: support for Intuos style tablets
 *                      - Fixed: some warnings
 *                      - Added: support for horizontal wheel
 *                      - Added: support for 4th and 5th button in mouse event
 *                      - Added: support for tilt around X, Y and Z in tablet
 *   0.3    2012-04-15  - Fixed: expunge crash
 *                      - Fixed: absolute wheel handling
 *                      - Added: support for Bamboo Pen & Touch
 *                      - Added: support for tool identifying tag
 *   0.2    2012-04-14  - Added: support for more tablets
 *                      - Added: configurable debuglevel.
 *                      - Fixed: pressure handling.
 *                      - Fixed: Commodity now uses hotkey "ctrl alt w"
 *                      - Fixed: Commodity uses tablet's name as broker name
 *   0.1    2012-04-11  - First working version.
 */


#include "WacomTablet.h"

#define DebugLevel  1

/// Version variables
extern uint16 version;
extern uint16 revision;
extern CONST_STRPTR idString;
extern CONST_STRPTR vstring;
///

//// Custom tags for tablet event
#define TABLETA_Tool (TABLETA_Dummy + 0x10B)
// This tag is used to notify recepient which tool was used on the tablet. See enum WacomToolType
////

/// Structs
static APTR Manager_Vectors[] = {
    _manager_Obtain,
    _manager_Release,
    NULL,
    NULL,
    _manager_Open,
    _manager_Close,
    _manager_Expunge,
    NULL,
    (APTR)-1,
};

static struct TagItem Manager_Tags[] = {
    { MIT_Name,         (uint32)"__library"     },
    { MIT_VectorTable,  (uint32)Manager_Vectors },
    { MIT_Version,      (uint32)1               },
    { TAG_DONE,         (uint32)0               }
};

static APTR Main_Vectors[] = {
    _main_Obtain,
    _main_Release,
    NULL,
    NULL,
    _main_GetAttrsA,
    _main_GetAttrs,
    _main_RunFunction,
    _main_RunInterface,
    (APTR)-1
};

static struct TagItem Main_Tags[] = {
    { MIT_Name,         (uint32)"main"          },
    { MIT_VectorTable,  (uint32)Main_Vectors    },
    { MIT_Version,      (uint32)1               },
    { TAG_DONE,         (uint32)0               }
};

static APTR libInterfaces[] = {
    Manager_Tags,
    Main_Tags,
    NULL
};

struct TagItem libCreateTags[] = {
    { CLT_DataSize,     (uint32)sizeof(struct WacomTabletBase)  },
    { CLT_InitFunc,     (uint32)_manager_Init                   },
    { CLT_Interfaces,   (uint32)libInterfaces                  },
    { CLT_NoLegacyIFace,(uint32)TRUE                            },
    { TAG_DONE,         (uint32)0                               }
};

///

/// Forward declarations
static int WacomSwitchToTabletMode(struct usbtablet *um);
///


/* -- Library -- */

/// _start

/*
 * The system (and compiler) rely on a symbol named _start which marks
 * the beginning of execution of an ELF file. To prevent others from
 * executing this library, and to keep the compiler/linker happy, we
 * define an empty _start symbol here.
 *
 * On the classic system (pre-AmigaOS4) this was usually done by
 * moveq #0,d0
 * rts
 *
 */

int32 _start( STRPTR argstring, int32 arglen, APTR SysBase )
{

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _start()\n" );
#endif

    /* If you feel like it, open DOS and print something to the user */

    return( RETURN_OK );
}

///

/* -- Manager Interface -- */

/// _manager_Init

/* The ROMTAG Init Function */
struct Library *_manager_Init(
    struct WacomTabletBase *libBase,
    APTR seglist,
    struct ExecIFace *IExec )
{
    uint32 err;

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Init( LibBase = %lx, seglist = %lx, IExec = %lx )\n", libBase, seglist, IExec );
#endif

    libBase->rmb_Type       = NT_LIBRARY;
    libBase->rmb_Pri        = LIBPRI;
    libBase->rmb_Name       = LIBNAME;
    libBase->rmb_Flags      = LIBF_SUMUSED|LIBF_CHANGED;
    libBase->rmb_Version    = version;
    libBase->rmb_Revision   = revision;
    libBase->rmb_IdString   = idString;

    libBase->rmb_SegmentList= seglist;
    libBase->rmb_IExec      = IExec;

    libBase->rmb_IExec->Obtain();

//---------------------------------------------------------------

    libBase->rmb_USBResourceBase = IExec->OpenLibrary( "usbresource.library", 1 );

    if ( libBase->rmb_USBResourceBase )
    {
        libBase->rmb_IUSBResource = (struct USBResourceIFace *)IExec->GetInterface( libBase->rmb_USBResourceBase, "main", 1, NULL );

        if ( libBase->rmb_IUSBResource )
        {

//---------------------------------------------------------------

            libBase->rmb_fdkey = libBase->rmb_IUSBResource->USBResRegisterFD(
                USBA_FD_Name,             LIBNAME,
                USBA_FD_Title,            vstring,
                USBA_FD_InterfaceDriver,  TRUE,
                USBA_ErrorCode,           &err,
                USBA_VendorID,            (ULONG)USB_VENDOR_ID_WACOM,
                USBA_Priority,             1,
                TAG_DONE
            );

            if (( libBase->rmb_fdkey != NULL ) || ( libBase->rmb_fdkey == NULL && err == USBERR_ISPRESENT ))
            {

                return( (struct Library *)libBase );

            }

//---------------------------------------------------------------

            IExec->DropInterface( (struct Interface *)libBase->rmb_IUSBResource );
        }
        IExec->CloseLibrary( libBase->rmb_USBResourceBase );
    }

//---------------------------------------------------------------

    return( NULL );
}

///
/// _manager_Obtain

uint32 _manager_Obtain(
    struct LibraryManagerInterface *Self )
{

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Obtain( Self = %lx )\n", Self );
#endif

    Self->Data.RefCount++;

    return( Self->Data.RefCount );
}

///
/// _manager_Release

uint32 _manager_Release(
    struct LibraryManagerInterface *Self )
{
#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Release( Self = %lx )\n", Self );
#endif

    Self->Data.RefCount--;

    return( Self->Data.RefCount );
}

///
/// _manager_Open

/* Open the library */
struct WacomTabletBase *_manager_Open(
    struct LibraryManagerInterface *Self,
    uint32 version )
{
    struct WacomTabletBase *libBase;

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Open( Self = %lx, Version = %ld )\n", Self, version );
#endif

    libBase = (struct WacomTabletBase *)Self->Data.LibBase;

    /* Add any specific open code here
       Return 0 before incrementing OpenCnt to fail opening */


    /* Add up the open count */
    libBase->rmb_OpenCnt++;
    libBase->rmb_Flags &= ~LIBF_DELEXP;

    return( libBase );
}

///
/// _manager_Close

/* Close the library */
APTR _manager_Close(
    struct LibraryManagerInterface *Self )
{
    struct WacomTabletBase *libBase;

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Close( Self = %lx )\n", Self );
#endif

    libBase = (struct WacomTabletBase *)Self->Data.LibBase;
    /* Make sure to undo what open did */

    /* Make the close count */
    libBase->rmb_OpenCnt--;

    if ( libBase->rmb_OpenCnt == 0 )
    {
        if ( libBase->rmb_Flags & LIBF_DELEXP )
        {

#if DebugLevel > 0
            ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
            "--> _manager_Close calling Self->Expunge %08lx\n",Self->Expunge );
#endif

            // Self->Expunge();
            _manager_Expunge(Self);

        }
    }

    return( NULL );
}

///
/// _manager_Expunge

/* Expunge the library */
APTR _manager_Expunge(
    struct LibraryManagerInterface *Self )
{
    struct WacomTabletBase *libBase;
    APTR result;

    libBase = (struct WacomTabletBase *)Self->Data.LibBase;

    if ( libBase->rmb_OpenCnt == 0 )
    {
#if DebugLevel > 0
        ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
        "--> _manager_Expunge really expunging ( Self = %lx )\n", Self );
#endif

        /* Undo what the init code did */

        if ( libBase->rmb_fdkey ) {
            libBase->rmb_IUSBResource->USBResUnregisterFD( libBase->rmb_fdkey );
        }

        if ( libBase->rmb_IUSBResource ) {
            libBase->rmb_IExec->DropInterface( (struct Interface *)libBase->rmb_IUSBResource );
        }

        if ( libBase->rmb_USBResourceBase ) {
            libBase->rmb_IExec->CloseLibrary( libBase->rmb_USBResourceBase );
        }

        /* Remove Library from Public */

        libBase->rmb_IExec->Remove( (struct Node *)libBase );
        
        result = libBase->rmb_SegmentList;
        struct Interface * IExec = (struct Interface*)libBase->rmb_IExec;
        
        libBase->rmb_IExec->DeleteLibrary( (struct Library *)libBase );

        IExec->Release();
    }
    else
    {
        libBase->rmb_Flags |= LIBF_DELEXP;
        result = NULL;
    }
#if DebugLevel > 1
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _manager_Expunge( Self = %lx Result %lx)\n", Self,result );
#endif

    return( result );
}

///

/* -- Main Interface -- */

/// _main_Obtain

uint32 _main_Obtain(
    struct WacomTabletIFace *Self )
{
#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_Obtain( Self = %lx )\n", Self );
#endif

    Self->Data.RefCount++;

    return( Self->Data.RefCount );
}

///
/// _main_Release

uint32 _main_Release(
    struct WacomTabletIFace *Self )
{
#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_Release( Self = %lx )\n", Self );
#endif

    Self->Data.RefCount--;

    return( Self->Data.RefCount );
}

///
/// _main_GetAttrsA

uint32 _main_GetAttrsA(
    struct WacomTabletIFace *Self,
    struct TagItem *taglist )
{
#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_GetAttrsA( Self = %lx, taglist = %lx )\n", Self, taglist );
#endif

    /* We don't support any tags - so we simply return */

    return( 0 );
}

///
/// _main_GetAttrs

uint32 VARARGS68K _main_GetAttrs(
    struct WacomTabletIFace *Self,
    ... )
{
    struct TagItem *tags;
    va_list ap;

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_GetAttrs( Self = %lx )\n", Self );
#endif

    va_startlinear( ap, Self );
    tags = va_getlinearva( ap, struct TagItem * );

    return( Self->GetAttrsA( tags ));
}

///
/// _main_RunFunction

int32 _main_RunFunction(
    struct WacomTabletIFace *Self,
    struct USBFDStartupMsg *msg )
{
#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_RunFunction( Self = %lx, msg = %lx )\n", Self, msg );
#endif

    return( USBERR_UNSUPPORTED );
}

///
/// _main_RunInterface

int32 _main_RunInterface(
    struct WacomTabletIFace *Self,
    struct USBFDStartupMsg *msg )
{
    struct USBBusIntDsc *dsc;
    int32 retval;

    dsc = (struct USBBusIntDsc *)msg->Descriptor;

#if DebugLevel > 0
    ((struct ExecIFace *)((*(struct ExecBase **)4)->MainInterface))->DebugPrintF(
    "--> _main_RunInterface( Self = %lx, msg = %lx )\nClass %ld, SubClass %ld Protocol %ld", Self, msg, dsc->id_Class, dsc->id_Subclass, dsc->id_Protocol );
#endif

    retval = USBERR_UNSUPPORTED;

    if ( dsc->id_Class == USBCLASS_HID )
    {
        // note some Wacom tablet present themselves without subclass
        //if ( dsc->id_Subclass == USBHID_SUBCLASS_BOOTINTERFACE )
        {
            // note some Wacom tablet present themselves without protocol
            //if ( dsc->id_Protocol == USBHID_PROTOCOL_MOUSE )
            {
                retval = WacomInterface( Self, msg );
            }
        }
    }

    return( retval );
}

///

/* -- Driver -- */

/// Wacom Interface

int32 WacomInterface( struct WacomTabletIFace *Self, struct USBFDStartupMsg *msg )
{
    struct WacomTabletBase *libBase;
    struct usbtablet *um;
    uint32 mask;
    int32 retval;

    libBase = (struct WacomTabletBase *)Self->Data.LibBase;

    retval = USBERR_NOMEM;

    /* Allocate an internal structure, so we can be called many times and still use
     * same code without interferrence from other mice that maybe be connected */

    um = libBase->rmb_IExec->AllocVecTags( sizeof(struct usbtablet), 
                                AVT_Type,           MEMF_SHARED, 
                                AVT_ClearWithValue, 0,
                                TAG_DONE );

    if ( um )
    {
        um->FirstOpen = TRUE;

        um->debugLevel = DebugLevel;
        um->IExec = libBase->rmb_IExec;
        um->USBReq = msg->USBReq;
        um->IntDsc = (struct USBBusIntDsc *)msg->Descriptor;
        um->StartMsg = msg;

        if ( WacomStartup( um ))
        {
            /* Main loop */

            while( um->TheEnd == FALSE )
            {
                DebugLog(40, um, "WacomInterface: about to wait\n");
                mask = um->IExec->Wait( um->InterfaceBit | um->UsbBit | um->CXsigflag | um->winsigflag );

                if ( mask & um->InterfaceBit )
                {
                    DebugLog(40, um, "WacomInterface: Interface message detected\n");
                    InterfaceHandler( um );
                }

                if ( mask & um->UsbBit )
                {
                    DebugLog(40, um, "WacomInterface: USB message detected\n");
                    WacomHandler( um );
                }

                if (mask & um->CXsigflag)
                {
                    DebugLog(40, um, "WacomInterface: CX message detected\n");

                    HandleCXmsg(um);
                }
                if (mask & um->winsigflag)
                {
                    DebugLog(40, um, "WacomInterface: window message detected\n");

                    HandleWindowInput(um);
                }

            }

            /* Abort any hanging IORequest there may be */

            um->IExec->AbortIO( (struct IORequest *)um->UsbIOReq );
            um->IExec->WaitIO( (struct IORequest *)um->UsbIOReq );

            retval = USBERR_NOERROR;
        }

        WacomShutdown( um );
    }

    return( retval );
}

///

/// DumpBuf
void DumpBuf( struct usbtablet *um, UBYTE *buf, ULONG len )
{
    switch( buf[1] )
    {
        case USBDESC_DEVICE :
        {
            struct USBBusDevDsc * pDesc = (struct USBBusDevDsc *) buf;
            DebugLog( 40, um, "Device descriptor :\n\tUSB version : %04x\n\tClass : %d\n\tSubClass : %d\n\tProtocol : %d\n\t",
                    LE_WORD(pDesc->dd_USBVer), pDesc->dd_Class, pDesc->dd_Subclass, pDesc->dd_Protocol );
            DebugLog( 40, um, "Endpoint 0 max packet size : %d\n\tVendorID : %04x\n\tProduct ID : %04x\n\tDevice version : %04x\n\t",
                    pDesc->dd_MaxPacketSize0, LE_WORD(pDesc->dd_VendorID), LE_WORD(pDesc->dd_Product), LE_WORD(pDesc->dd_DevVer) );
            DebugLog( 40, um, "Manufacturer String Index : %d\n\tProduct String Index : %d\n\tSerial String Index : %d\n\t",
                    pDesc->dd_ManufacturerStr, pDesc->dd_ProductStr, pDesc->dd_SerialStr );
            DebugLog( 40, um, "Configuration possible : %d\n", pDesc->dd_NumConfigs );
        }
        break;
        case USBDESC_INTERFACE :
        {
            struct USBBusIntDsc * pDesc = (struct USBBusIntDsc *) buf;
            DebugLog( 40, um, "Interface descriptor :\n\tInterface ID : %d\n\tAlternate Setting : %d\n\tNumber of Endpoints :%d\n\t",
                    pDesc->id_InterfaceID, pDesc->id_AltSetting, pDesc->id_NumEndPoints );
            DebugLog( 40, um, "Class : %d\n\tSubClass : %d\n\tProtocol : %d\n\tInterface string Index : %d\n",
                    pDesc->id_Class, pDesc->id_Subclass, pDesc->id_Protocol, pDesc->id_InterfaceStr );
        }
        break;
        case USBDESC_ENDPOINT :
        {
            struct USBBusEPDsc * pDesc = (struct USBBusEPDsc *) buf;
            DebugLog( 40, um, "EndPoint descriptor :\n\tAddress : %02x (n°%d, %s)\n\tAttributs : %02x ->", 
                pDesc->ed_Address, pDesc->ed_Address&USBEPADRM_EPNUMBER, ((pDesc->ed_Address&USBEPADRM_DIRECTION)==USBEPADR_DIR_IN)?"IN":"OUT", pDesc->ed_Attributes );
            switch( pDesc->ed_Attributes & USBEPATRM_TRANSFERTYPE )
            {
                case USBEPTT_CONTROL :      DebugLog( 40, um, "Control");break;
                case USBEPTT_ISOCHRONOUS :  DebugLog( 40, um, "Isochronous");break;
                case USBEPTT_BULK :         DebugLog( 40, um, "Bulk");break;
                case USBEPTT_INTERRUPT :    DebugLog( 40, um, "Interrupt");break;
                default: DebugLog( 40, um, "unknown Transfert type");
            }
            DebugLog( 40, um, " %s %s\n\tPacket size :%ld\n\tPolling Interval : %d ms\n",pDesc->ed_Attributes&USBCFGATRF_REMOTEWAKEUP?"Remote WakeUp":"", pDesc->ed_Attributes&USBCFGATRF_SELFPOWERED?"Self Powered":"", LE_WORD(pDesc->ed_MaxPacketSize), pDesc->ed_Interval );
        }
        break;
        case USBDESC_CONFIGURATION :
        {
            struct USBBusCfgDsc * pDesc = (struct USBBusCfgDsc *) buf;
            DebugLog( 40, um, "Configuration Description :\n\tLength : %d\n\tNumber of Interface : %d\n\tConfigurationID :%d\n\t",
                    LE_WORD(pDesc->cd_TotalLength), pDesc->cd_NumInterfaces, pDesc->cd_ConfigID );
            DebugLog( 40, um, "Configuration String Index : %d\n\tAttributes : %s %s\n\tMax Power : %d mA\n", 
                    pDesc->cd_ConfigStr, pDesc->cd_Attributes&USBCFGATRF_REMOTEWAKEUP?"Remote WakeUp":"", pDesc->cd_Attributes&USBCFGATRF_SELFPOWERED?"Self Powered":"",
                    pDesc->cd_MaxPower*2 );
        }
        break;
        case USBDESC_STRING :
            DebugLog( 40, um, " String descriptor" );
            break;
        default:
            DebugLog( 40, um, "Unknown Descriptor type %d\n", buf[1] );
            break;

    }

    DebugLog( 40, um, "Hex Dump:" );
    while (len--) {

        DebugLog( 40, um, " 0x%02x", (*buf) );
        buf++;
    }
    DebugLog( 40, um, "\n");
}
///

/// Wacom Startup

uint32 WacomStartup( struct usbtablet *um )
{
    um->InputStat = 1;

    um->buttonAction[BTN_LEFT].ba_action = ACTION_CLIC;
    um->buttonAction[BTN_MIDDLE].ba_action = ACTION_MIDDLE_CLIC;
    um->buttonAction[BTN_RIGHT].ba_action = ACTION_RIGHT_CLIC;
    um->buttonAction[BTN_SIDE].ba_action = ACTION_4TH_CLIC;
    um->buttonAction[BTN_EXTRA].ba_action = ACTION_5TH_CLIC;
    um->buttonAction[BTN_TOUCH].ba_action = ACTION_CLIC;
    um->buttonAction[BTN_STYLUS].ba_action = ACTION_MIDDLE_CLIC;
    um->buttonAction[BTN_STYLUS2].ba_action = ACTION_RIGHT_CLIC;
    

    um->EventType = USE_NEWTABLET;  // Set the default event type.

    um->TopX = DEFAULT_TOP_X;
    um->TopY = DEFAULT_TOP_Y;
    um->RangeX = DEFAULT_RANGE_X;
    um->RangeY = DEFAULT_RANGE_Y;
    um->RangeP = DEFAULT_RANGE_P;

    um->Curve[0] = DEFAULT_CURVE_0;
    um->Curve[1] = DEFAULT_CURVE_1;
    um->Curve[2] = DEFAULT_CURVE_2;
    um->Curve[3] = DEFAULT_CURVE_3; 
    um->Curve[4] = DEFAULT_CURVE_4;
    um->Curve[5] = DEFAULT_CURVE_5;
    um->Curve[6] = DEFAULT_CURVE_6;     
    
    /* Set Task Pri. to 15 for smooth mouse movements */

    um->TaskAddr = um->IExec->FindTask( NULL );
    um->TaskOldPri = um->IExec->SetTaskPri( um->TaskAddr, 15 );

    /* Get USBSys Infterface */
    um->DOSBase = um->IExec->OpenLibrary("dos.library",0);
    um->IDOS = (struct DOSIFace *)um->IExec->GetInterface(um->DOSBase,"main",1,0);

    // um->CON = um->IDOS->FOpen("CON:/",MODE_OLDFILE,1024);

    if((um->CxBase = um->IExec->OpenLibrary("commodities.library",0L)))
    {
        if(!(um->ICommodities = (struct CommoditiesIFace *)um->IExec->GetInterface(um->CxBase,"main",1,0)))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    if((um->IntuitionBase = um->IExec->OpenLibrary("intuition.library",0L)))
    {
        if(!(um->IIntuition = (struct IntuitionIFace *) um->IExec->GetInterface(um->IntuitionBase,"main",1,0)))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
    if((um->UtilityBase = um->IExec->OpenLibrary("utility.library",0L)))
    {
        if(!(um->IUtility = (struct UtilityIFace *) um->IExec->GetInterface(um->UtilityBase,"main",1,0)))
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    struct Library * library = NULL;
    if(NULL != (library = um->IExec->OpenLibrary("locale.library",0L)))
    {
        if(!(um->localeInfo.li_ILocale = (struct LocaleIFace*) um->IExec->GetInterface(library,"main",1,0)))
        {
            return FALSE;
        }
        else
        {
            um->localeInfo.li_Catalog = um->localeInfo.li_ILocale->OpenCatalog(NULL, (STRPTR )"WacomTablet.catalog",
                                                            OC_BuiltInLanguage, "english",
                                                            OC_Version, 0,
                                                            TAG_DONE);
        }
    }
    else
    {

        return FALSE;
    }

    if(!(um->ApplicationBase = um->IExec->OpenLibrary("application.library",0L)) 
        || !(um->IPrefsObjects = (struct PrefsObjectsIFace *) um->IExec->GetInterface(um->ApplicationBase, "prefsobjects", 2, 0)))
    {
        return FALSE;
    }

    um->USBSysBase = (struct Library *)um->USBReq->io_Device;
    um->IUSBSys = (struct USBSysIFace *)um->IExec->GetInterface( um->USBSysBase, "main", 1, NULL );

    if ( um->IUSBSys == NULL ) {
        return( FALSE );
    }

    // /
    struct USBBusDscHead *dsclist = um->StartMsg->Descriptor;
    while ( dsclist ) {
        DumpBuf( um, (UBYTE *) dsclist, dsclist->dh_Length );
        dsclist = um->IUSBSys->USBNextDescriptor( dsclist ); // Get next descriptor
    }
    // /

    /* Identify the product */
    struct USBBusDevDsc *pDevDesc = NULL;
    um->IUSBSys->USBGetRawInterfaceAttrs( um->StartMsg->Object, USBA_DeviceDesc, (ULONG)&pDevDesc, TAG_END );
    if ( NULL != pDevDesc )
    {
        int nProductIndex = 0;
        BOOL bFound = FALSE;
        uint16 sVendorId    = LE_WORD(pDevDesc->dd_VendorID); 
        uint16 sProductId   = LE_WORD(pDevDesc->dd_Product);
        while( wacom_devices[nProductIndex].idVendor != 0 )
        {
            if (wacom_devices[nProductIndex].idVendor == sVendorId 
                && wacom_devices[nProductIndex].idDevice == sProductId)
            {
                bFound = TRUE;
                break;
            }
            nProductIndex++;
        }
        
        if( FALSE == bFound )
        {
            um->IUSBSys->USBLogPuts( 1, NULL, "Unable to identify device by Vendor and Product Id" );
            return( FALSE );            
        } 

        um->features = wacom_devices[nProductIndex].features;
        um->RangeX = (uint16)um->features->x_max;
        um->RangeY = (uint16)um->features->y_max;
        um->RangeP = (uint16)um->features->pressure_max;
        um->touch_arbitration = 1;
        DebugLog( 20, um, "Identified product '%s'\n", um->features->name );
    }
    else
    {
        um->IUSBSys->USBLogPuts( 1, NULL, "Unable to identify device because DeviceDesc can't be obtained" );
        return( FALSE );
    }

    TEXT debug_var[10];
    if((um->IDOS->GetVar("WacomTablet/DEBUG",debug_var,sizeof(debug_var),GVF_GLOBAL_ONLY) > 0))
    {
        LONG et = 0L;
        um->IDOS->StrToLong(debug_var,&et);
        if( et > 0 )
            um->debugLevel = et;
    }
    LoadValues(um);
    DebugLog(20, um, "--> Prefs Loaded\n" );

    WacomSetupCapabilities(um);

    /*set up commodity for GUI */

    if(!SetupCX(um))
    {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error Creating Commodity" );
        return FALSE;
    }

    /* Setup and Claim Interface */

    um->InterfaceMP = (struct MsgPort *)um->IExec->AllocSysObject(ASOT_PORT, NULL);

    if ( um->InterfaceMP == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error Creating Interface MsgPort" );
        return( FALSE );
    }

    um->InterfaceBit = 1 << um->InterfaceMP->mp_SigBit;

    um->Interface = um->IUSBSys->USBClaimInterface( um->StartMsg->Object, (APTR) -1, um->InterfaceMP );

    if ( um->Interface == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error Claiming Interface" );
        return( FALSE );
    }

    um->UsbControlEndPoint = um->IUSBSys->USBGetEndPoint( NULL, um->Interface, 0 );

    if ( um->UsbControlEndPoint == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Control EndPoint not found" );
        return( FALSE );
    }

    int32 error = WacomSwitchToTabletMode(um);
    if( 0 != error )
    {
        DebugLog(0, um, "Can't switch to Wacom mode, error %ld\n", error );
        return( FALSE );
    }

    /* Find EndPoints  */

    um->UsbEndPointDscrIn = FindEndPointDscr( um, um->StartMsg->Descriptor, USBEPTT_INTERRUPT, USBEPADR_DIR_IN );

    if ( um->UsbEndPointDscrIn == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "EndPoint Descriptor not found" );
        return( FALSE );
    }

    um->UsbStatusEndPoint = um->IUSBSys->USBGetEndPoint( NULL, um->Interface, um->UsbEndPointDscrIn->ed_Address );

    if ( um->UsbStatusEndPoint == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Status EndPoint not found" );
        return( FALSE );
    }

    /* Setup and Open input.device so we can send InputEvents to the system */

    um->InputMP = (struct MsgPort *)um->IExec->AllocSysObject(ASOT_PORT, NULL);

    if ( um->InputMP == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error Input creaing MsgPort" );
        return( FALSE );
    }

    um->InputIOReq = (struct IOStdReq *)um->IExec->AllocSysObjectTags(  ASOT_IOREQUEST,
                                                                        ASO_NoTrack,        FALSE,
                                                                        ASOIOR_ReplyPort,   um->InputMP,
                                                                        ASOIOR_Size,        sizeof( struct IOStdReq ),
                                                                        TAG_END
                                                                      );

    if ( um->InputIOReq == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error creating Input IORequest" );
        return( FALSE );
    }

    um->InputStat = um->IExec->OpenDevice( "input.device", 0, (struct IORequest *)um->InputIOReq, 0 );

    if ( um->InputStat != 0 ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Open input device failed" );
        return( FALSE );
    }

    um->UsbData = um->IExec->AllocVecTags(  um->features->pktlen, 
                                            AVT_Type, MEMF_SHARED,
                                            AVT_ClearWithValue, 0,
                                            TAG_DONE );

    if ( NULL == um->UsbData ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error allocating USB communication buffer" );
        return( FALSE );        
    }

    /* Setup USB IORequester, We'll get a signal every time the mouse move or change button status */

    um->UsbMP = (struct MsgPort *)um->IExec->AllocSysObject(ASOT_PORT, NULL);

    if ( um->UsbMP == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error USB creating MsgPort" );
        return( FALSE );
    }

    um->UsbBit = 1 << um->UsbMP->mp_SigBit;

    um->UsbIOReq = um->IUSBSys->USBAllocRequestA( um->USBReq, NULL );

    if ( um->UsbIOReq == NULL ) {
        um->IUSBSys->USBLogPuts( -1, NULL, "Error creating USB IORequest" );
        return( FALSE );
    }

    um->UsbIOReq->io_Message.mn_ReplyPort = um->UsbMP;

    /* Start the data flow */
    um->UsbIOReq->io_Command    = CMD_READ;
    um->UsbIOReq->io_Data       = um->UsbData;
    um->UsbIOReq->io_Length     = um->features->pktlen;
    um->UsbIOReq->io_EndPoint   = um->UsbStatusEndPoint;

    if ( um->UsbIOReq->io_Length > LE_WORD( um->UsbEndPointDscrIn->ed_MaxPacketSize ))
    {
        um->UsbIOReq->io_Length = LE_WORD( um->UsbEndPointDscrIn->ed_MaxPacketSize );
    }

    um->IExec->SendIO( (struct IORequest *)um->UsbIOReq );

    um->IUSBSys->USBLogPuts( 0, NULL, "WacomTablet Initialised" );

    return( TRUE );
}

///
/// Wacom Shutdown

VOID WacomShutdown( struct usbtablet *um )
{
    if ( um == NULL ) {
        return;
    }

    if ( um->localeInfo.li_Catalog ) {
        um->localeInfo.li_ILocale->CloseCatalog(um->localeInfo.li_Catalog);
        um->localeInfo.li_Catalog = NULL;
    }

    if ( um->localeInfo.li_ILocale ) {
        struct Library * pLibrary = um->localeInfo.li_ILocale->Data.LibBase;
        um->IExec->DropInterface( (struct Interface *)um->localeInfo.li_ILocale );
        um->localeInfo.li_ILocale = NULL;
        um->IExec->CloseLibrary(pLibrary);
    }

    if ( um->UsbData ) {
        um->IExec->FreeVec( um->UsbData );
        um->UsbData = NULL;
    }

    if ( um->UsbIOReq ) {
        um->IUSBSys->USBFreeRequest( um->UsbIOReq );
        um->UsbIOReq = NULL;
    }

    if ( um->UsbMP ) {
        um->IExec->FreeSysObject( ASOT_PORT, um->UsbMP );
        um->UsbMP = NULL;
    }

    if ( um->InputStat == 0 ) {
        um->IExec->CloseDevice( (struct IORequest *)um->InputIOReq );
    }

    if ( um->InputIOReq ) {
        um->IExec->FreeSysObject( ASOT_IOREQUEST, um->InputIOReq );
        um->InputIOReq = NULL;
    }

    if ( um->InputMP ) {
        um->IExec->FreeSysObject( ASOT_PORT, um->InputMP );
        um->InputMP = NULL;
    }

    if ( um->Interface ) {
        um->IUSBSys->USBDeclaimInterface( um->Interface );
        um->Interface = NULL;
    }

    if ( um->InterfaceMP ) {
        um->IExec->FreeSysObject( ASOT_PORT, um->InterfaceMP );
        um->InterfaceMP = NULL;
    }

    if ( um->IUSBSys ) {
        um->IExec->DropInterface( (struct Interface *)um->IUSBSys );
        um->IUSBSys = NULL;
    }

    um->IExec->SetTaskPri( um->TaskAddr, um->TaskOldPri );


    ShutdownCX(um);
    DisposeWindow(um);
    CloseClasses(um);

    um->IExec->DropInterface((struct Interface *)um->ICommodities);
    um->IExec->CloseLibrary(um->CxBase);
    um->IExec->DropInterface((struct Interface *)um->IIntuition);
    um->IExec->CloseLibrary(um->IntuitionBase);

    um->IExec->DropInterface((struct Interface *)um->IUtility);
    um->IExec->CloseLibrary(um->UtilityBase);


    if(um->CON)
    {
        um->IDOS->FClose(um->CON);
        um->CON = ZERO;
    }
    if(um->IDOS)
    {
        um->IExec->DropInterface((struct Interface *)um->IDOS);
        um->IDOS = NULL;
    }
    if(um->DOSBase)
    {
        um->IExec->CloseLibrary(um->DOSBase);
        um->DOSBase = NULL;
    }

    um->IExec->FreeVec( um );
    um = NULL;
}

///

/// Interface Handler

VOID InterfaceHandler( struct usbtablet *um )
{
struct USBNotifyMsg *msg;

    while(( msg = (struct USBNotifyMsg *)um->IExec->GetMsg( um->InterfaceMP )))
    {
        if ( msg->Type == USBNM_TYPE_INTERFACEDETACH )
        {
            um->TheEnd = TRUE;
        }

        um->IExec->ReplyMsg( (struct Message *)msg );
    }
}

///
static int32  MaxP = 0;
static int32  MaxX = 0;
static int32  MaxY = 0;

#define max(a,b) ((a) > (b) ? (a) : (b))

uint32 HandleExecuteActions(struct usbtablet *ut, struct ButtonAction buttonAction[], uint32 buttons)
{
    DebugLog(10, ut, "HandleExecuteActions in, buttons #%08x prevbuttons #%08x\n", buttons, ut->PrevButtons);
    // do not do anything if there was no change in buttons state
    if(buttons == ut->PrevButtons) return 0;

    uint8 bit = 0;
    for(; bit < 32; bit++)
    {
        if((buttons & FLAG(bit)) && !(ut->PrevButtons & FLAG(bit)))
        {
            switch(buttonAction[bit].ba_action)
            {
                case ACTION_SHOWKEY:
                    {
                        DebugLog(10, ut, "Keyshow action detected for bit %d\n", bit);
                        int32 err = ut->IDOS->SystemTags("appdir:keyshow",
                                                        SYS_UserShell,  TRUE,
                                                        SYS_Asynch,     TRUE,
                                                        SYS_Input,      ZERO,
                                                        SYS_Output,     ZERO,
                                                        TAG_END);
                        if(err)
                            DebugLog(10, ut, "Error launching keyshow %d\n", ut->IDOS->IoErr());
                    } break;
                case ACTION_HOLD_CLIC:
                    {
                        ut->holdClick = !ut->holdClick;
                        DebugLog(10, ut, "Hold-Click action detected for bit %d, hold-click is %s\n",
                                        bit, ut->holdClick?"engaged":"disengaged");
                    } break;
                case ACTION_DOUBLE_CLIC:
                    {
                        // record state
                        uint64 prevButtons = ut->PrevButtons;
                        uint64 currButtons = ut->Buttons;

                        uint32 buttons = 0;
                        // send first click
                        SETBITS(buttons, BTN_LEFT, 1);
                        SendMouseEvent(ut, buttons);
                        SETBITS(buttons, BTN_LEFT, 0);
                        SendMouseEvent(ut, buttons);

                        // send second click
                        SETBITS(buttons, BTN_LEFT, 1);
                        SendMouseEvent(ut, buttons);
                        SETBITS(buttons, BTN_LEFT, 0);
                        SendMouseEvent(ut, buttons);

                        // restore state
                        ut->PrevButtons = prevButtons;
                        ut->Buttons = currButtons;
                    } break;
                default:
                    break;
            }
        }
    }

    ut->PrevButtons = buttons;

    DebugLog(10, ut, "HandleExecuteActions out");
    return 0;
}

uint32 SendWheelEvent(struct usbtablet *um, int32 horizWheelData, int32 vertWheelData, BOOL relative)
{
    int32 vWheel = um->currentState.wheel[1], hWheel = um->currentState.wheel[0];
    if(!relative)
    {
        DebugLog(10, um, "SendWheelEvent: old vWheel %ld, vertWheelData %ld,"
                        " old hWheel %ld, horizWheelData %ld\n",
                        vWheel, vertWheelData, hWheel, horizWheelData);
        if(!vertWheelData)
        {
            vWheel = 0;   
        }
        else
        {    
            vWheel = vertWheelData - vWheel;
            if(abs(vWheel) > 20)
            {
                vWheel = (vWheel > 0) ? 20 : -20;
            }
        }
        um->prevState.wheel[1] = vertWheelData;
        if(!horizWheelData)
        {
            hWheel = 0;   
        }
        else
        {    
            hWheel = horizWheelData - hWheel;
            if(abs(hWheel) > 20)
            {
                hWheel = (hWheel > 0)? 20 : -20;
            }
        }
        um->prevState.wheel[0] = horizWheelData;
    }
    else
    {
        vWheel = vertWheelData;
        hWheel = horizWheelData;
    }
    
    if(hWheel | vWheel)
    {
        DebugLog(10, um, "SendWheelEvent: scroll X %ld, Y %ld\n", hWheel, vWheel );

        um->InputEvent.ie_NextEvent = NULL;
        um->InputEvent.ie_Class     = IECLASS_MOUSEWHEEL;
        um->InputEvent.ie_SubClass  = 0;
        um->InputEvent.ie_Code      = 0;
        um->InputEvent.ie_Qualifier = 0;
        um->InputEvent.ie_X         = hWheel;
        um->InputEvent.ie_Y         = vWheel;
        
        um->InputIOReq->io_Command  = IND_ADDEVENT;
        um->InputIOReq->io_Data     = &um->InputEvent;
        um->InputIOReq->io_Length   = sizeof( struct InputEvent );
        um->IExec->DoIO( (struct IORequest *)um->InputIOReq );
    }
    
    return 0;
}

uint32 GetMouseButtons(struct usbtablet *um, struct ButtonAction buttonAction[], uint32 buttons)
{
    uint32 result = 0;
    uint8 bit = 0;
    
    for(; bit < 32; bit++)
    {
        if(buttons & FLAG(bit)) {
            DebugLog(10, um, "Button set detected for bit %d\n", bit);  
            switch(buttonAction[bit].ba_action)
            {
                case ACTION_CLIC: 
                    SETBITS(result, BTN_LEFT, buttons & FLAG(bit));
                    break;
                case ACTION_MIDDLE_CLIC:
                    SETBITS(result, BTN_MIDDLE, buttons & FLAG(bit));                
                     break;
                case ACTION_RIGHT_CLIC:
                    SETBITS(result, BTN_RIGHT, buttons & FLAG(bit));                
                    break;
                case ACTION_4TH_CLIC:
                    SETBITS(result, BTN_SIDE, buttons & FLAG(bit));                
                    break;
                case ACTION_5TH_CLIC:
                    SETBITS(result, BTN_EXTRA, buttons & FLAG(bit));                
                    break;
                default:
                    break;
            }
        }
    }

    // enforce click holding, i.e. consider BTN_LEFT down while it is set
    if(um->holdClick)
    {
        SETBITS(result, BTN_LEFT, 1);
    }
    
    return result;
}

uint32 SendMouseEvent(struct usbtablet *um, uint32 buttons)
{
    uint16 code, qual;

    uint32 But1 = GetMouseButtons(um, um->buttonAction, buttons);
    uint32 tmp1, tmp2;

    DebugLog(10, um, "SendMouseEvent: buttons #%08x extracted buttons: #%08x\n", buttons, But1 );

    if ( But1 != um->Buttons )
    {
        DebugLog(15, um, "SendMouseEvent: button state change detected: #%08x (new) vs #%08x (old)\n", But1, um->Buttons );

        code = IECODE_NOBUTTON;
        qual = IEQUALIFIER_RELATIVEMOUSE;

        /* Check Left Button */

        tmp1 = But1 & FLAG(BTN_LEFT);

        if ( tmp1 )
        {
            qual |= IEQUALIFIER_LEFTBUTTON;
        }

        tmp2 = um->Buttons & FLAG(BTN_LEFT);

        if ( tmp1 != tmp2 )
        {
            if ( tmp1 )
            {
                DebugLog(20, um, "SendMouseEvent: left button pressed\n" );
                code = IECODE_LBUTTON;
                um->ButtonDown = TRUE;
            }
            else
            {
                DebugLog(20, um, "SendMouseEvent: left button released\n" );
                code = IECODE_LBUTTON|IECODE_UP_PREFIX;
                um->ButtonDown = FALSE;
            }
        }

        /* Check Right Button */
        int32 nFlag = 0;
        if(!um->SwitchButtons)
        {
            nFlag = FLAG(BTN_RIGHT);
        }
        else
        {
            nFlag = FLAG(BTN_MIDDLE);
        }

        tmp1 = But1 & nFlag;
        if ( tmp1 )
        {
            qual |= IEQUALIFIER_RBUTTON;
        }

        tmp2 = um->Buttons & nFlag;
        if ( tmp1 != tmp2 )
        {
            if ( tmp1 )
            {
                DebugLog(20, um, "SendMouseEvent: right button pressed\n" );
                code = IECODE_RBUTTON;
            }
            else
            {
                DebugLog(20, um, "SendMouseEvent: right button released\n" );
                code = IECODE_RBUTTON|IECODE_UP_PREFIX;
            }
        }

        /* Check Middle Button */
        if(!um->SwitchButtons)
        {
            nFlag = FLAG(BTN_MIDDLE);
        }
        else
        {
            nFlag = FLAG(BTN_RIGHT);
        }

        tmp1 = But1 & nFlag;
        if ( tmp1 )
        {
            qual |= IEQUALIFIER_MIDBUTTON;
        }

        tmp2 = um->Buttons & nFlag;
        if ( tmp1 != tmp2 )
        {
            if ( tmp1 )
            {
                DebugLog(20, um, "SendMouseEvent: middle button pressed\n" );
                code = IECODE_MBUTTON;
            }
            else
            {
                DebugLog(20, um, "SendMouseEvent: middle button released\n" );
                code = IECODE_MBUTTON|IECODE_UP_PREFIX;
            }
        }

        /* Check for 4th button */
        nFlag = FLAG(BTN_SIDE);

        tmp1 = But1 & nFlag;
        tmp2 = um->Buttons & nFlag;
        if ( tmp1 != tmp2 )
        {
            if ( tmp1 )
            {
                DebugLog(20, um, "SendMouseEvent: 4th button pressed\n" );
                code = IECODE_4TH_BUTTON;
            }
            else
            {
                DebugLog(20, um, "SendMouseEvent: 4th button released\n" );
                code = IECODE_4TH_BUTTON|IECODE_UP_PREFIX;
            }
        }

        /* Check for 5th button */
        nFlag = FLAG(BTN_EXTRA);

        tmp1 = But1 & nFlag;
        tmp2 = um->Buttons & nFlag;
        if ( tmp1 != tmp2 )
        {
            if ( tmp1 )
            {
                DebugLog(20, um, "SendMouseEvent: 5th button pressed\n" );
                code = IECODE_5TH_BUTTON;
            }
            else
            {
                DebugLog(20, um, "SendMouseEvent: 5th button released\n" );
                code = IECODE_5TH_BUTTON|IECODE_UP_PREFIX;
            }
        }

        um->Buttons = But1;
        um->Qual    = qual;

        DebugLog(15, um, "SendMouseEvent : code %ld qual %ld\n", code, qual );

        /* Send Input Event */
        um->InputEvent.ie_NextEvent = NULL;
        um->InputEvent.ie_Class     = IECLASS_RAWMOUSE;
        um->InputEvent.ie_SubClass  = 0;
        um->InputEvent.ie_Code      = code;
        um->InputEvent.ie_Qualifier = qual;
        um->InputEvent.ie_X         = 0;
        um->InputEvent.ie_Y         = 0;

        um->InputIOReq->io_Command  = IND_ADDEVENT;
        um->InputIOReq->io_Data     = &um->InputEvent;
        um->InputIOReq->io_Length   = sizeof( struct InputEvent );
        um->IExec->DoIO( (struct IORequest *)um->InputIOReq );
    }
    
    return 0;
}

BOOL TabletStateHasChanged(struct usbtablet *wacom)
{
    BOOL change = 0;

    if(abs(wacom->currentState.X - wacom->prevState.X) >= max(10, wacom->fuzzX)) change++;
    if(abs(wacom->currentState.Y - wacom->prevState.Y) >= max(10, wacom->fuzzY)) change++;
    if(abs(wacom->currentState.Z - wacom->prevState.Z) >= max(10, wacom->fuzzZ)) change++;
    if(abs(wacom->currentState.tiltX - wacom->prevState.tiltX) >= max(4, wacom->fuzzTiltX)) change++;
    if(abs(wacom->currentState.tiltY - wacom->prevState.tiltY) >= max(4, wacom->fuzzTiltY)) change++;
    if(abs(wacom->currentState.tiltZ - wacom->prevState.tiltZ) >= max(4, 1/*wacom->fuzzTiltZ*/)) change++;
    if(abs(wacom->currentState.Pressure - wacom->prevState.Pressure) >= max(1, wacom->features->pressure_fuzz)) change++;
    if(abs(wacom->currentState.proximity[0] - wacom->prevState.proximity[0]) >= 1) change++;
    if(abs(wacom->currentState.proximity[1] - wacom->prevState.proximity[1]) >= 1) change++;
    if(abs(wacom->currentState.wheel[0] - wacom->prevState.wheel[0]) >= max(1, wacom->fuzzWheel)) change++;
    if(abs(wacom->currentState.wheel[1] - wacom->prevState.wheel[1]) >= max(1, wacom->fuzzWheel)) change++;

    if(change)
    {
        DebugLog(40, wacom, "TabletStateHasChanged: change detected\n");
        DebugLog(40, wacom, "TabletStateHasChanged: current: {X:%ld, Y:%ld, Z:%ld, tiltX:%ld, tiltY:%ld, tiltZ:%ld, P:%ld,"
                            " dist:{%ld, %ld}, prox:{%d:%d}, wheel:{%ld,%ld}}\n",
                            wacom->currentState.X, wacom->currentState.Y, wacom->currentState.Z,
                            wacom->currentState.tiltX, wacom->currentState.tiltY, wacom->currentState.tiltZ,
                            wacom->currentState.Pressure, wacom->currentState.distance[0], wacom->currentState.distance[1],
                            wacom->currentState.proximity[0], wacom->currentState.proximity[1], wacom->currentState.wheel[0],
                            wacom->currentState.wheel[1]);
        DebugLog(40, wacom, "TabletStateHasChanged: prev: {X:%ld, Y:%ld, Z:%ld, tiltX:%ld, tiltY:%ld, tiltZ:%ld, P:%ld,"
                            " dist:{%ld, %ld}, prox:{%d:%d}, wheel:{%ld,%ld}}\n",
                            wacom->prevState.X, wacom->prevState.Y, wacom->prevState.Z,
                            wacom->prevState.tiltX, wacom->prevState.tiltY, wacom->prevState.tiltZ,
                            wacom->prevState.Pressure, wacom->prevState.distance[0], wacom->prevState.distance[1],
                            wacom->prevState.proximity[0], wacom->prevState.proximity[1], wacom->prevState.wheel[0],
                            wacom->prevState.wheel[1]);
    }

    return (change>0)?TRUE:FALSE;
}

uint32 SendTabletEvent(uint8 toolIdx, struct usbtablet *um, uint32 buttons)
{
    if (TabletStateHasChanged(um))
    {
        DebugLog(45, um, "SendTabletEvent: buttons #%08x\n", buttons );

        if(um->win)
        {
            if (um->currentState.Pressure > MaxP) MaxP = um->currentState.Pressure;
            if (um->currentState.X > MaxX) MaxX = um->currentState.X;
            if (um->currentState.Y > MaxY) MaxY = um->currentState.Y;
        }
        if(um->win)
        {
            um->IIntuition->GetWindowAttr(um->win,WA_Activate,&um->WindowActive,4);
        }
        if(um->win && (um->WindowActive || !um->ButtonDown))
        {

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_CURRENTX],um->win,NULL,
                                                    INTEGER_Number,um->currentState.X,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_CURRENTY],um->win,NULL,
                                                    INTEGER_Number,um->currentState.Y,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_CURRENTTILTX],um->win,NULL,
                                                    INTEGER_Number,um->currentState.tiltX,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_CURRENTTILTY],um->win,NULL,
                                                    INTEGER_Number,um->currentState.tiltY,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_CURRENTP],um->win,NULL,
                                                    INTEGER_Number,um->currentState.Pressure,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_MAXX],um->win,NULL,
                                                    INTEGER_Number,MaxX,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_MAXY],um->win,NULL,
                                                    INTEGER_Number,MaxY,
                                                    TAG_DONE);

            um->IIntuition->SetGadgetAttrs(um->gadgets[GID_MAXP],um->win,NULL,
                                                    INTEGER_Number,MaxP,
                                                    TAG_DONE);

        }
        if(um->EventType & USE_NEWTABLET)
        {

            DebugLog(20, um, "Sending EventType NEWTABLET\n");

            /* Send Input Event */

            um->InputEvent.ie_NextEvent = NULL;
            um->InputEvent.ie_Class     = IECLASS_NEWPOINTERPOS;
            um->InputEvent.ie_SubClass  = IESUBCLASS_NEWTABLET;
            um->InputEvent.ie_Code      = IECODE_NOBUTTON;
            um->InputEvent.ie_Qualifier = um->Qual;
            um->InputEvent.ie_EventAddress = &um->IENT;

            um->IENT.ient_RangeX = um->RangeX - um->TopX;
            um->IENT.ient_RangeY = um->RangeY - um->TopY;
            if (um->currentState.X < um->TopX)
            {
                um->IENT.ient_TabletX = 0;
            }
            else
            {
                um->IENT.ient_TabletX = um->currentState.X - um->TopX;
            }
            if (um->currentState.Y < um->TopY)
            {
                um->IENT.ient_TabletY = 0;
            }
            else
            {
                um->IENT.ient_TabletY = um->currentState.Y - um->TopY;
            }
            um->IENT_Tags[0].ti_Tag = TABLETA_Pressure;
            // normalize to fill signed long integer range
            //um->IENT_Tags[0].ti_Data = (ConvertPressure(um, um->currentState.Pressure) * 0xffffffff / um->RangeP ) - 0x80000000;
            um->IENT_Tags[0].ti_Data = (ConvertPressure(um, um->currentState.Pressure) * 0x7fffffff / um->RangeP );
            // identify tool
            um->IENT_Tags[1].ti_Tag = TABLETA_Tool;
            um->IENT_Tags[1].ti_Data = um->tool[toolIdx];
            um->IENT_Tags[2].ti_Tag = TABLETA_InProximity;
            um->IENT_Tags[2].ti_Data = um->currentState.proximity[toolIdx];
            um->IENT_Tags[3].ti_Tag = TABLETA_TabletZ;
            um->IENT_Tags[3].ti_Data = um->currentState.Z;
            um->IENT_Tags[4].ti_Tag = TABLETA_AngleX;
            um->IENT_Tags[4].ti_Data = -(um->currentState.tiltY - ((um->maxTiltY - um->minTiltY) / 2)) * (0x7fffffff / 360);
            um->IENT_Tags[5].ti_Tag = TABLETA_AngleY;
            um->IENT_Tags[5].ti_Data = (um->currentState.tiltX - ((um->maxTiltX - um->minTiltX) / 2)) * (0x7fffffff / 360);
            um->IENT_Tags[6].ti_Tag = TABLETA_AngleZ;
            um->IENT_Tags[6].ti_Data = um->currentState.tiltZ;

            DebugLog(20, um, "NewTablet: X %ld, Y %ld, Pressure %ld, Tool %ld Proximity %ld, AngleX %ld, AngleY %ld, AngleZ %ld\n",
                            um->IENT.ient_TabletX, 
                            um->IENT.ient_TabletY, 
                            um->IENT_Tags[0].ti_Data,
                            um->IENT_Tags[1].ti_Data,
                            um->IENT_Tags[2].ti_Data,
                            um->IENT_Tags[4].ti_Data,
                            um->IENT_Tags[5].ti_Data,
                            um->IENT_Tags[6].ti_Data
                            );

            um->IENT_Tags[7].ti_Tag = TAG_DONE;
            um->IENT_Tags[7].ti_Data = 0;

            um->IENT.ient_TagList = um->IENT_Tags;

            um->InputIOReq->io_Command  = IND_WRITEEVENT;
            um->InputIOReq->io_Data     = &um->InputEvent;
            um->InputIOReq->io_Length   = sizeof( struct InputEvent );
            um->IExec->DoIO( (struct IORequest *)um->InputIOReq );
        }
        if(um->EventType & USE_TABLET)
        {

            /* Send Input Event */

            um->InputEvent.ie_NextEvent = NULL;
            um->InputEvent.ie_Class     = IECLASS_NEWPOINTERPOS;
            um->InputEvent.ie_SubClass  = IESUBCLASS_TABLET;
            um->InputEvent.ie_Code      = IECODE_NOBUTTON;;
            um->InputEvent.ie_Qualifier = um->Qual;
            um->InputEvent.ie_EventAddress = &um->IEPT;

            um->IEPT.iept_Range.X = um->RangeX - um->TopX;
            um->IEPT.iept_Range.Y = um->RangeY - um->TopY;
            if( um->currentState.X < um->TopX )
            {
                um->IEPT.iept_Value.X = 0;
            }
            else
            {
                um->IEPT.iept_Value.X = um->currentState.X - um->TopX;
            }
            if( um->currentState.Y < um->TopY )
            {
                um->IEPT.iept_Value.Y = 0;
            }
            else
            {
                um->IEPT.iept_Value.Y = um->currentState.Y - um->TopY;
            }
            // normalize pressure, in old Tablet data range is -128 - 127
            um->IEPT.iept_Pressure = (um->currentState.Pressure * 0xff / um->RangeP) - 0x80;

            DebugLog(20, um, "Pressure %ld\n", um->IEPT.iept_Pressure);

            um->InputIOReq->io_Command  = IND_WRITEEVENT;
            um->InputIOReq->io_Data     = &um->InputEvent;
            um->InputIOReq->io_Length   = sizeof( struct InputEvent );
            um->IExec->DoIO( (struct IORequest *)um->InputIOReq );
        }
        if(um->SendRawMouse)
        {
            um->InputEvent.ie_NextEvent = NULL;
            um->InputEvent.ie_Class     = IECLASS_RAWMOUSE;
            um->InputEvent.ie_SubClass  = 0;
            um->InputEvent.ie_Code      = IECODE_NOBUTTON;;
            um->InputEvent.ie_Qualifier = IEQUALIFIER_RELATIVEMOUSE | um->Qual;
            um->InputEvent.ie_X         = 0;
            um->InputEvent.ie_Y         = 0;

            um->InputIOReq->io_Command  = IND_ADDEVENT;
            um->InputIOReq->io_Data     = &um->InputEvent;
            um->InputIOReq->io_Length   = sizeof( struct InputEvent );
            um->IExec->DoIO( (struct IORequest *)um->InputIOReq );

        }

    }

    um->IExec->CopyMem(&um->currentState, &um->prevState, sizeof(struct WacomState));

    return 0;
}

/// Find EndPoint Dscr

struct USBBusEPDsc *FindEndPointDscr( struct usbtablet *um, struct USBBusDscHead *dsclist, uint8 ttype, uint8 dir )
{
// This function locates an EndPoint based on EndPoint requirements.
//
// Inputs
//  dsclist     Pointer to first USB Descriptor to check for matching EndPoint
//  ttype       TransferType to search for (a USBEPTT_xxx constant)
//  dir         Direction of EndPoint (a USBEPADR_DIR_xxx constant)

struct USBBusDscHead *dsc;
struct USBBusEPDsc *epd;

    dsc = dsclist;

    while ( dsc )
    {
        if ( dsc->dh_Type == USBDESC_ENDPOINT )
        {
            epd = (struct USBBusEPDsc *)dsc;

            if ( (( epd->ed_Address & USBEPADRM_DIRECTION ) == dir ) && (( epd->ed_Attributes & USBEPATRM_TRANSFERTYPE ) == ttype ) )
            {
                break; // We've got a matching EndPoint
            }
        }
        dsc = um->IUSBSys->USBNextDescriptor( dsc );
    }

    return( (struct USBBusEPDsc *)dsc );
}

///

/// Convert Pressure Against Curve

uint32 ConvertPressure(struct usbtablet *um, uint32 pressure)
{
    /* we have 5 sliders we makes 6 ranges */
    /* 01 12 23 34 45 56 */
    float r[7] = {0.0,16.0,33.0,50.0,67.0, 83.0, 100.0};
    float c[7] = {(float)um->Curve[0],(float)um->Curve[1],(float)um->Curve[3],(float)um->Curve[3],(float)um->Curve[4],(float)um->Curve[5],(float)um->Curve[6]};
    
    
    int range = 0;
    int i;
    
    float inpc =  (100.0 * (float)pressure ) / (float)um->RangeP;
    float outpc;
    
    for(i = 0; i < 6;i++)
    {
        if(inpc >= r[i]) range ++;
    }
    outpc = c[range -1] + (c[range] - c[range -1]) * (inpc -r[range-1]) /( r[range] - r[range -1]);
    
    return (uint32) ((outpc * (float)um->RangeP)/100.0);
}
///


///

#define USB_REQ_GET_REPORT  0x01
#define USB_REQ_SET_REPORT  0x09
#define WAC_HID_FEATURE_REPORT  0x03

int usb_get_report(struct usbtablet *um, unsigned char type,
                unsigned char id, void *buf, int size)
{
    DebugLog(45, um, "usb_get_report: type %c id %c buffer %08x buffer size %ld\n", type, id, buf, size );

    int32 rc = um->IUSBSys->USBEPControlXferA( um->USBReq, um->UsbControlEndPoint, 
        USB_REQ_GET_REPORT, USBSDT_TYP_CLASS | USBSDT_REC_INTERFACE | USBSDT_DIR_DEVTOHOST,
        (type << 8) + id, um->IntDsc->id_InterfaceID,
        buf, size, NULL );
    DebugLog(45, um, "usb_get_report: rc %ld\n", rc );

    return  rc;
}

int usb_set_report(struct usbtablet *um, unsigned char type,
                unsigned char id, void *buf, int size)
{
    DebugLog(45, um, "usb_set_report: type %c id %c buffer %08x buffer size %ld\n", type, id, buf, size );

    int32 rc = um->IUSBSys->USBEPControlXferA( um->USBReq, um->UsbControlEndPoint,
        USB_REQ_SET_REPORT, USBSDT_TYP_CLASS | USBSDT_REC_INTERFACE,
        (type << 8) + id, um->IntDsc->id_InterfaceID,
        buf, size, NULL);
    DebugLog(45, um, "usb_set_report: rc %ld\n", rc );

    return  USBERR_STALL == rc ? 0 : rc;
}
///

//// WacomSwitchToTabletMode -- tries to switch, returns 0 on success, a negative error code otherwise
static int WacomSwitchToTabletMode(struct usbtablet *um)
{
    UBYTE *rep_data;
    int limit = 0, report_id = 2;
    int error = USBERR_NOMEM;

    rep_data = um->IExec->AllocVecTags(3, AVT_Type, MEMF_SHARED, TAG_END);
    if (!rep_data)
        return error;

    /* ask to report tablet data if it is 2FGT Tablet PC
     * OR not a Tablet PC */
    if (/*um->features->device_type == BTN_TOOL_TRIPLETAP &&*/
            (um->features->type == TABLETPC2FG)) {
        do {
            rep_data[0] = 3;
            rep_data[1] = 4;
            report_id = 3;
            error = usb_set_report(um, WAC_HID_FEATURE_REPORT,
                report_id, rep_data, 2);
            if (error >= 0)
                error = usb_get_report(um,
                    WAC_HID_FEATURE_REPORT, report_id,
                    rep_data, 3);
        } while ((error < 0 || rep_data[1] != 4) && limit++ < 5);
    } else if (um->features->type != TABLETPC && um->features->type != TABLETPC2FG) {
        do {
            rep_data[0] = 2;
            rep_data[1] = 2;
            error = usb_set_report(um, WAC_HID_FEATURE_REPORT,
                report_id, rep_data, 2);
            if (error >= 0)
                error = usb_get_report(um,
                    WAC_HID_FEATURE_REPORT, report_id,
                    rep_data, 2);
        } while ((error < 0 || rep_data[1] != 2) && limit++ < 5);
    }

    um->IExec->FreeVec(rep_data);

    return error != 0 ? error : 0;
}
///

/// SetAbsParam
void SetAbsParams(struct usbtablet *um, int32 paramName, int32 min, int32 max, int32 fuzz, int32 flat)
{
    switch(paramName)
    {
        default:
            DebugLog(0, um, "WacomTablet: unsupported value for paramName '%08x'\n", paramName);
            break;
        case ABS_X:
            um->minX = min;
            um->maxX = max;
            um->fuzzX = fuzz;
            //um->flatX = flat;
            break;
        case ABS_Y:
            um->minY = min;
            um->maxY = max;
            um->fuzzY = fuzz;
            //um->flatY = flat;
            break;
        case ABS_Z:
        case ABS_DISTANCE:
            // NOTE: https://www.kernel.org/doc/Documentation/input/event-codes.txt,
            // for now ABS_Z and ABS_DISTANCE are considered as synonyms
            um->minZ = min;
            um->maxZ = max;
            um->fuzzZ = fuzz;
            //um->flatZ = flat;
            break;
        case ABS_WHEEL:
            um->minWheel = min;
            um->maxWheel = max;
            um->fuzzWheel = fuzz;
            //um->flatWheel = flat;
            break;
        case ABS_TILT_X:
            um->minTiltX = min;
            um->maxTiltX = max;
            um->fuzzTiltX = fuzz;
            //um->flatTiltX = flat;
            break;
        case ABS_TILT_Y:
            um->minTiltY = min;
            um->maxTiltY = max;
            um->fuzzTiltY = fuzz;
            //um->flatTiltY = flat;
            break;
    }
}
///

CONST_STRPTR GetString(struct LocaleInfo *li, LONG stringNum)
{
    struct LocaleIFace *ILocale    = li->li_ILocale;
    LONG         *l;
    UWORD        *w;
    CONST_STRPTR  builtIn = NULL;

    l = (LONG *)CatCompBlock;

    while (l < (LONG *)(&CatCompBlock[sizeof(CatCompBlock)]))
    {
        if (*l == stringNum)
        {
            builtIn = (CONST_STRPTR)((ULONG)l + 6);
            break;
        }
        w = (UWORD *)((ULONG)l + 4);
        l = (LONG *)((ULONG)l + (ULONG)*w + 6);
    }

    if (ILocale)
    {
#ifdef __USE_INLINE__
        return GetCatalogStr(li->li_Catalog, stringNum, builtIn);
#else
        return ILocale->GetCatalogStr(li->li_Catalog, stringNum, builtIn);
#endif
    }
    return builtIn;
}


/* -- The End -- */

