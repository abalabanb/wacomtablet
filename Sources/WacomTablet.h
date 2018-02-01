/*
 * Copyright 2012 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
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

#include <devices/input.h>
#include <devices/inputevent.h>
#include <dos/dos.h>
#include <exec/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/usbresource.h>
#include <proto/commodities.h>
#include <proto/usbsys.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <proto/slider.h>
#include <proto/application.h>

#include <intuition/intuition.h>
#include <stdarg.h>
#include <string.h>
#include <usb/devclasses.h>
#include <usb/system.h>
#include <usb/usb.h>
#include <utility/tagitem.h>

#include <classes/window.h>
#include <gadgets/layout.h>
#include <gadgets/integer.h>
#include <gadgets/checkbox.h>
#include <gadgets/slider.h>
#include <gadgets/string.h>
#include <gadgets/clicktab.h>
#include <gadgets/chooser.h>
#include <images/label.h>

#include <libraries/commodities.h>

#include <clib/alib_protos.h>
#include <strings.h>
#include <stdio.h>

/*------------------------------------------------------------------------*/

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack(2)
    #endif
#elif defined(__VBCC__)
    #pragma amiga-align
#endif

/*------------------------------------------------------------------------*/

#define USBHID_SUBCLASS_BOOTINTERFACE   1
#define USBHID_PROTOCOL_MOUSE           2

//// Bit handling macros
#define SETBITS(var,bit,value) if (value) var|=(1<<bit); else var&=~(1<<bit);
#define FLAG(bit) (1L<<(bit)) 
////
#define STRING(a) #a

/* Version Tag */
#define LIBVERSION      0
#define LIBREVISION     7
#define LIBPRI          -46
#define LIBNAME         "WacomTablet.usbfd"
#define VSTRING         "WacomTablet.usbfd " STRING(LIBVERSION) "." STRING(LIBREVISION) " (10-05-2012)\r\n"
#define VERSTAG         "\0$VER: WacomTablet.usbfd " STRING(LIBVERSION) "." STRING(LIBREVISION) " (10-05-2012)"

/* Structs */
struct WacomTabletIFace {
    struct InterfaceData Data;
    uint32              APICALL (*Obtain)       (struct WacomTabletIFace *Self);
    uint32              APICALL (*Release)      (struct WacomTabletIFace *Self);
    void                APICALL (*Expunge)      (struct WacomTabletIFace *Self);
    struct Interface *  APICALL (*Clone)        (struct WacomTabletIFace *Self);
    uint32              APICALL (*GetAttrsA)    (struct WacomTabletIFace *Self, struct TagItem *taglist );
    uint32              APICALL (*GetAttrs)     (struct WacomTabletIFace *Self, ... );
    int32               APICALL (*RunFunction)  (struct WacomTabletIFace *Self, struct USBFDStartupMsg *msg );
    int32               APICALL (*RunInterface) (struct WacomTabletIFace *Self, struct USBFDStartupMsg *msg );
};

struct WacomTabletBase {
    APTR                        rmb_Succ;        // Pointer to next (successor)
    APTR                        rmb_Pred;        // Pointer to previous (predecessor)
    uint8                       rmb_Type;
    int8                        rmb_Pri;         // Priority, for sorting
    STRPTR                      rmb_Name;        // ID string, null terminated
    uint8                       rmb_Flags;       // see below
    uint8                       rmb_ABIVersion;  // ABI exported by library
    uint16                      rmb_NegSize;     // number of bytes before LIB
    uint16                      rmb_PosSize;     // number of bytes after LIB
    uint16                      rmb_Version;     // major
    uint16                      rmb_Revision;    // minor
    STRPTR                      rmb_IdString;    // ASCII identification
    uint32                      rmb_Sum;         // the system-calculated checksum
    uint16                      rmb_OpenCnt;     // number of current opens
    APTR                        rmb_SegmentList;

    struct ExecIFace *          rmb_IExec;

    struct Library *            rmb_USBResourceBase;
    struct USBResourceIFace *   rmb_IUSBResource;

    APTR                        rmb_fdkey;
};

#define USE_NEWTABLET    0x0001
#define USE_TABLET       0x0002

#define DEFAULT_TOP_X  0
#define DEFAULT_TOP_Y  0
#define DEFAULT_RANGE_X 32768
#define DEFAULT_RANGE_Y 32768
#define DEFAULT_RANGE_P 1023
#define DEFAULT_CURVE_0 0 
#define DEFAULT_CURVE_1 17 
#define DEFAULT_CURVE_2 33 
#define DEFAULT_CURVE_3 50 
#define DEFAULT_CURVE_4 66 
#define DEFAULT_CURVE_5 83 
#define DEFAULT_CURVE_6 100 

/// Wacom definitions

enum ButtonActionType {
    ACTION_NONE = 0,
    ACTION_CLIC,
    ACTION_MIDDLE_CLIC,
    ACTION_RIGHT_CLIC,
    ACTION_4TH_CLIC,
    ACTION_5TH_CLIC,
    ACTION_DOUBLE_CLIC,
    ACTION_HOLD_CLIC,
    ACTION_SHOWKEY,
    ACTION_SWITCH_MODE,
    ACTION_QUALIFIER,
    ACTION_KEY,
    ACTION_RUN,
    ACTION_DEFAULT
} ;

struct ButtonAction
{
    enum ButtonActionType ba_action;
    APTR ba_parameter;
    LONG ba_paramSize;
};

#define USB_VENDOR_ID_WACOM                0x056a

enum {
    PENPARTNER = 0,
    GRAPHIRE,
    WACOM_G4,
    PTU,
    PL,
    DTU,
    BAMBOO_PT,
    INTUOS,
    INTUOS3S,
    INTUOS3,
    INTUOS3L,
    INTUOS4S,
    INTUOS4,
    INTUOS4L,
    WACOM_24HD,
    WACOM_21UX2,
    CINTIQ,
    WACOM_BEE,
    WACOM_MO,
    TABLETPC,
    TABLETPC2FG,
    MAX_TYPE
};

/* maximum packet length for USB devices */
#define WACOM_PKGLEN_MAX    32

/* packet length for individual models */
#define WACOM_PKGLEN_PENPRTN     7
#define WACOM_PKGLEN_GRAPHIRE    8
#define WACOM_PKGLEN_BBFUN       9
#define WACOM_PKGLEN_INTUOS     10
#define WACOM_PKGLEN_TPC1FG      5
#define WACOM_PKGLEN_TPC2FG     14
#define WACOM_PKGLEN_BBTOUCH    20

/* wacom data packet report IDs */
enum {
    WACOM_REPORT_PENABLED       = 2,
    WACOM_REPORT_INTUOSREAD     = 5, 
    WACOM_REPORT_INTUOSWRITE    = 6, 
    WACOM_REPORT_INTUOSPAD      = 12,
    WACOM_REPORT_TPC1FG         = 6,     
    WACOM_REPORT_TPC2FG         = 13,    

} WacomReportID;

/* Wacom tool type */
enum {
    BTN_TOOL_PEN     = 0,
    BTN_TOOL_RUBBER,
    BTN_TOOL_BRUSH,
    BTN_TOOL_PENCIL,
    BTN_TOOL_AIRBRUSH,
    BTN_TOOL_FINGER,
    BTN_TOOL_MOUSE,
    BTN_TOOL_LENS, 
    BTN_TOOL_DOUBLETAP,
    BTN_TOOL_TRIPLETAP,
//    BTN_TOOL_QUADTAP,
//    BTN_TOOL_QUINTTAP
} WacomToolType;

/* Wacom Device IDs */
enum {
    STYLUS_DEVICE_ID    = 0x02,
    TOUCH_DEVICE_ID     = 0x03,
    CURSOR_DEVICE_ID    = 0x06,
    ERASER_DEVICE_ID    = 0x0A,
    PAD_DEVICE_ID       = 0x0F,

} WacomDeviceID;

/* Wacom BTN type */
enum {
    BTN_LEFT    = 31,
    BTN_MIDDLE  = 30,
    BTN_RIGHT   = 29,
    BTN_SIDE    = 28,
    BTN_EXTRA   = 27,
    BTN_TOUCH   = 26,
    BTN_STYLUS  = 25,
    BTN_STYLUS2 = 24,
    
    BTN_BASE3   = 18,
    BTN_BASE2   = 17,
    BTN_BASE    = 16,
    BTN_Z       = 15,
    BTN_Y       = 14,
    BTN_X       = 13,
    BTN_C       = 12,
    BTN_B       = 11,
    BTN_A       = 10,
    BTN_9       = 9,
    BTN_8       = 8,
    BTN_7       = 7,
    BTN_6       = 6,
    BTN_5       = 5,
    BTN_4       = 4,
    BTN_3       = 3,
    BTN_2       = 2,
    BTN_1       = 1,
    BTN_0       = 0,
} WacomButtons;

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


//// GIDS
enum {
    GID_CURRENTX,
    GID_CURRENTY,
    GID_CURRENTP,
    GID_MAXX,
    GID_MAXY,
    GID_MAXP,
    GID_TOPX,
    GID_TOPY,
    GID_RANGEX,
    GID_RANGEY,
    GID_RANGEP,
    GID_SETTOMAX,
    GID_SAVE,
    GID_USE,
    GID_NEWTABLET,
    GID_TABLET,
    GID_PTEST,
    GID_SWITCH,
    GID_RAWMOUSE,
    GID_PCURVE0,
    GID_PCURVE1,
    GID_PCURVE2,
    GID_PCURVE3,
    GID_PCURVE4,        
    GID_PCURVE5,            
    GID_PCURVE6,
    GID_DEBUGLEVEL,
    GID_CLICKTAB,
    GID_TOUCH,
    GID_STYLUS,
    GID_STYLUS2,
    GID_LEFT,
    GID_MIDDLE,
    GID_RIGHT,
    GID_4TH,
    GID_5TH,
    GID_BTN_0,
    GID_BTN_1,
    GID_BTN_2,
    GID_BTN_3,
    GID_BTN_4,
    GID_BTN_5,
    GID_BTN_6,
    GID_BTN_7,
    GID_BTN_8,
    GID_BTN_9,
    GID_BTN_A,
    GID_BTN_B,
    GID_BTN_C,
    GID_BTN_X,
    GID_BTN_Y,
    GID_BTN_Z,
    GID_BTN_BASE,
    GID_BTN_BASE2,
    GID_BTN_BASE3,
            
    GID_LAST
} gids;
////

struct usbtablet {
    uint32                  TopX;
    uint32                  TopY;
    uint32                  RangeX;
    uint32                  RangeY;
    uint32                  RangeP;

    BOOL                    SwitchButtons;
    BOOL                    SendRawMouse;
    BOOL                    ButtonDown;
    uint32                  WindowActive;
    struct ExecIFace *      IExec;

    struct Library *        DOSBase;
    struct DOSIFace *       IDOS;
    BPTR                    CON;

    struct Library *        USBSysBase;
    struct USBSysIFace *    IUSBSys;


    struct Task *           TaskAddr;
    uint32                  TaskOldPri;

    struct UsbInterface *   Interface;
    struct MsgPort *        InterfaceMP;
    uint32                  InterfaceBit;

    struct MsgPort *        InputMP;
    uint32                  InputStat;
    struct IOStdReq *       InputIOReq;
    uint16                  EventType;
    struct InputEvent       InputEvent;
    struct IENewTablet      IENT;
    struct TagItem          IENT_Tags[8];
    struct IEPointerTablet  IEPT;
    struct MsgPort *        UsbMP;
    uint32                  UsbBit;
    struct USBIOReq *       UsbIOReq;
    UBYTE *                 UsbData;
    struct USBBusEPDsc *    UsbEndPointDscrIn;

    struct UsbEndPoint *    UsbStatusEndPoint;
    struct UsbEndPoint *    UsbControlEndPoint;

    struct USBFDStartupMsg *StartMsg;
    struct IORequest       *USBReq;
    struct USBBusIntDsc *   IntDsc;

    uint32                  TheEnd;
    uint32                  Buttons;
    uint16                  Qual;

    struct Library *        CxBase;
    struct CommoditiesIFace* ICommodities;


    CxObj*                  CXBroker;
    struct MsgPort*         CXPort;
    uint32                  CXsigflag;

    struct Library*         IntuitionBase;
    struct IntuitionIFace*  IIntuition;

    struct Library*         UtilityBase;
    struct UtilityIFace*    IUtility;

	struct Library *		ApplicationBase;
	struct ApplicationIFace* IApplication;
	struct PrefsObjectsIFace* IPrefsObjects;


    struct  ClassLibrary *  WindowClassLib;
    Class *                 WindowClassPtr;

    struct ClassLibrary *   LayoutClassLib;;
    Class *                 LayoutClassPtr;

    struct ClassLibrary *   IntegerClassLib;
    Class *                 IntegerClassPtr;

    struct ClassLibrary *   ButtonClassLib;
    Class *                 ButtonClassPtr;

    struct ClassLibrary *   LabelClassLib;
    Class *                 LabelClassPtr;

    struct ClassLibrary *   CheckboxClassLib;
    Class *                 CheckboxClassPtr;

    struct ClassLibrary *   SpaceClassLib;
    Class *                 SpaceClassPtr;

    struct ClassLibrary *   SliderClassLib;
    Class *                 SliderClassPtr;

    struct ClassLibrary *   StringClassLib;
    Class *                 StringClassPtr;

    struct ClassLibrary *   ClickTabClassLib;
    Class *                 ClickTabClassPtr;

    struct ClassLibrary *   ChooserClassLib;
    Class *                 ChooserClassPtr;

    BOOL                    FirstOpen;

    Object *                Window;
    struct Window*          win;
    uint32                  winsigflag;

    struct Gadget*          windowLayout;

    struct Gadget*          gadgets[GID_LAST];
    
    // capabilities
    const struct wacom_features*  features;
    uint32                  buttonCapabilities;
    uint32                  toolCapabilities;
    uint32                  maxZ;
    int32                   minWheel;
    int32                   maxWheel;
    int32                   minThrottle;
    int32                   maxThrottle;

    struct ButtonAction     buttonAction[32];

    uint8                   debugLevel;

    uint32                  X;
    uint32                  Y;
    uint32                  Z;
    uint32                  tiltX;
    uint32                  tiltY;
    uint32                  tiltZ;

    uint32                  Pressure;
    UWORD                   Curve[7];
    UBYTE                   id[3];
    UBYTE                   tool[3];
    uint32                  distance[3];
    BOOL                    proximity[3];
    int32                   wheel[2];

    int32                   last_finger;
};

//// Debug macro
#define DebugLog(level,um,fmt,...) if((um)->debugLevel>(level)) (um)->IExec->DebugPrintF(fmt, ##__VA_ARGS__)
////

//// Protos

int32                   _start(             STRPTR argstring, int32 arglen, APTR SysBase );

struct Library *        _manager_Init(      struct WacomTabletBase *libBase, APTR seglist, struct ExecIFace *IExec );
uint32                  _manager_Obtain(    struct LibraryManagerInterface *Self );
uint32                  _manager_Release(   struct LibraryManagerInterface *Self );
struct WacomTabletBase* _manager_Open(      struct LibraryManagerInterface *Self, uint32 version );
APTR                    _manager_Close(     struct LibraryManagerInterface *Self );
APTR                    _manager_Expunge(   struct LibraryManagerInterface *Self );

uint32                  _main_Obtain(       struct WacomTabletIFace *Self );
uint32                  _main_Release(      struct WacomTabletIFace *Self );
uint32                  _main_GetAttrsA(    struct WacomTabletIFace *Self, struct TagItem *taglist );
uint32      VARARGS68K  _main_GetAttrs(     struct WacomTabletIFace *Self, ... );
int32                   _main_RunFunction(  struct WacomTabletIFace *Self, struct USBFDStartupMsg *startmsg );
int32                   _main_RunInterface( struct WacomTabletIFace *Self, struct USBFDStartupMsg *startmsg );

int32                   WacomInterface(     struct WacomTabletIFace *Self, struct USBFDStartupMsg *msg );
uint32                  WacomStartup(       struct usbtablet *um );
VOID                    WacomShutdown(      struct usbtablet *um );

VOID                    InterfaceHandler(   struct usbtablet *um );
VOID                    WacomHandler(       struct usbtablet *um );

struct USBBusEPDsc *    FindEndPointDscr(   struct usbtablet *um, struct USBBusDscHead *dsclist, uint8 ttype, uint8 dir );

BOOL                    SetupCX(struct usbtablet *um);
void                    ShutdownCX(struct usbtablet *um);
BOOL                    HandleCXmsg(struct usbtablet *um);

void                    LoadValues(struct usbtablet *um);


BOOL                    OpenClasses(struct usbtablet *um);
void                    CloseClasses(struct usbtablet *um);

VOID                    CloseWindow(struct usbtablet *um);
BOOL                    OpenWindow(struct usbtablet *um);
VOID                    DisposeWindow(struct usbtablet *um);
Object *                CreateWindow(struct usbtablet *um);
BOOL                    HandleWindowInput(struct usbtablet *um);
void                    SetGadgets(struct usbtablet *um);
uint32                  ConvertPressure(struct usbtablet *um, uint32 pressure);

uint32                  SendWheelEvent(struct usbtablet *um, int32 horizWheel, int32 vertWheelData, BOOL relative);
uint32                  SendMouseEvent(struct usbtablet *um, uint32 buttons);
uint32                  SendTabletEvent(uint8 toolIdx, struct usbtablet *um, uint32 buttons);

void                    WacomSetupCapabilities(struct usbtablet *um);
////

/*------------------------------------------------------------------------*/

#ifdef __GNUC__
    #ifdef __PPC__
        #pragma pack()
    #endif
#elif defined(__VBCC__)
    #pragma default-align
#endif

/*------------------------------------------------------------------------*/

