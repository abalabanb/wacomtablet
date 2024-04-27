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
#include <proto/locale.h>
#include <proto/application.h>
#include <proto/clicktab.h>

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

#define CATCOMP_NUMBERS
#define CATCOMP_STRINGS
#define CATCOMP_BLOCK
#include "WacomTablet_locale.h"

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
#define SETBITS(var,bit,value) if (value) var|=(((typeof(var))1)<<bit); else var&=~(((typeof(var))1)<<bit);
#define HAS_FLAG(var,bit) (0 != (var & (((typeof(var))1)<<bit)))
////

#define LIBPRI          -46
#define LIBNAME         "WacomTablet.usbfd"

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
    CONST_STRPTR                rmb_Name;        // ID string, null terminated
    uint8                       rmb_Flags;       // see below
    uint8                       rmb_ABIVersion;  // ABI exported by library
    uint16                      rmb_NegSize;     // number of bytes before LIB
    uint16                      rmb_PosSize;     // number of bytes after LIB
    uint16                      rmb_Version;     // major
    uint16                      rmb_Revision;    // minor
    CONST_STRPTR                rmb_IdString;    // ASCII identification
    uint32                      rmb_Sum;         // the system-calculated checksum
    uint16                      rmb_OpenCnt;     // number of current opens
    APTR                        rmb_SegmentList;

    struct ExecIFace *          rmb_IExec;

    struct Library *            rmb_USBResourceBase;
    struct USBResourceIFace *   rmb_IUSBResource;

    struct Library *            rmb_NewlibBase;
    struct NewlibIFace *        rmb_INewlib;

    APTR                        rmb_fdkey;
};

struct LocaleInfo
{
#ifndef __amigaos4__
    struct Library     *li_LocaleBase;
#else
    struct LocaleIFace *li_ILocale;
#endif
    struct Catalog     *li_Catalog;
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
    CONST_APTR ba_parameter;
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
    DTUS,
    DTUS2,
    DTUSX,
    DTH1152,
    DTK2451,
    INTUOS,
    INTUOS3S,
    INTUOS3,
    INTUOS3L,
    INTUOS4S,
    INTUOS4,
    INTUOS4L,
    INTUOS5S,
    INTUOS5,
    INTUOS5L,
    INTUOSPS,
    INTUOSPM,
    INTUOSPL,
    WACOM_21UX2,
    WACOM_22HD,
    DTK,
    WACOM_24HD,
    WACOM_27QHD,
    CINTIQ_HYBRID,
    CINTIQ_COMPANION_2,
    WACOM_MSPRO,
    CINTIQ_16,
    WACOM_PRO2022,
    WACOM_ONE,
    CINTIQ,
    WACOM_BEE,
    WACOM_13HD,
    WACOM_MO,
    INTUOSHT,
    INTUOSHT2,
    BAMBOO_PT,
    WACOM_24HDT,
    WACOM_27QHDT,
    WACOM_MSPROT,
    DTH1152T,
    INTUOSP2,
    INTUOSP2S,
    INTUOSHT3,
    WIRELESS,
    REMOTE,
    TABLETPC,   /* add new TPC below */
    TABLETPCE,
    TABLETPC2FG,
    DTH2452T,
    MTSCREEN,
    MTTPC,
    MTTPC_B,
    MTTPC_C,
    MAX_TYPE
};

/* maximum packet length for USB devices */
#define WACOM_PKGLEN_MAX    192

/* packet length for individual models */
#define WACOM_PKGLEN_PENPRTN     7
#define WACOM_PKGLEN_GRAPHIRE    8
#define WACOM_PKGLEN_BBFUN       9
#define WACOM_PKGLEN_INTUOS     10
#define WACOM_PKGLEN_TPC1FG      5
#define WACOM_PKGLEN_TPC1FG_B   10
#define WACOM_PKGLEN_TPC2FG     14
#define WACOM_PKGLEN_BBTOUCH    20
#define WACOM_PKGLEN_BBTOUCH3   64
#define WACOM_PKGLEN_BBPEN      10
#define WACOM_PKGLEN_WIRELESS   32
#define WACOM_PKGLEN_MTOUCH     62
#define WACOM_PKGLEN_MTTPC      40
#define WACOM_PKGLEN_DTUS       68
#define WACOM_PKGLEN_PENABLED    8
#define WACOM_PKGLEN_27QHDT     64
#define WACOM_PKGLEN_MSPRO      64
#define WACOM_PKGLEN_MSPROT     50
#define WACOM_PKGLEN_INTUOSP2   64
#define WACOM_PKGLEN_INTUOSP2T  44
#define WACOM_PKGLEN_DTH1152    12

/* wacom data packet report IDs */
enum {
    WACOM_REPORT_PENABLED           = 2,
    WACOM_REPORT_INTUOS_ID1         = 5,
    WACOM_REPORT_INTUOS_ID2         = 6,
    WACOM_REPORT_INTUOSPAD          = 12,
    WACOM_REPORT_INTUOS5PAD         = 3,
    WACOM_REPORT_DTUSPAD            = 21,
    WACOM_REPORT_TPC1FG             = 6,
    WACOM_REPORT_TPC2FG             = 13,
/*
    WACOM_REPORT_TPCMT              = 13,
    WACOM_REPORT_TPCMT2             = 3,
    WACOM_REPORT_TPCHID             = 15,
    WACOM_REPORT_TPCST              = 16,
*/
    WACOM_REPORT_CINTIQ             = 16,
    WACOM_REPORT_MSPRO              = 16,
    WACOM_REPORT_INTUOS_PEN         = 16,
    WACOM_REPORT_CINTIQPAD          = 17,
    WACOM_REPORT_DTUS               = 17,
    WACOM_REPORT_MSPROPAD           = 17,
//    WACOM_REPORT_TPC1FGE            = 18,
    WACOM_REPORT_MSPRODEVICE        = 19,
    WACOM_REPORT_DTK2451PAD         = 21,
/*
    WACOM_REPORT_24HDT              = 1,
    WACOM_REPORT_WL                 = 128,
*/
    WACOM_REPORT_USB                = 192,
//    WACOM_REPORT_DEVICE_LIST        = 16,
//    WACOM_REPORT_REMOTE             = 17,
//    WACOM_REPORT_VENDOR_DEF_TOUCH   = 33,
//    WAC_CMD_LED_CONTROL_GENERIC     = 50,

} WacomReportID;

/* device quirks */
#define WACOM_QUIRK_BBTOUCH_LOWRES  0x0001
#define WACOM_QUIRK_NO_INPUT        0x0002
#define WACOM_QUIRK_MONITOR     0x0004
#define WACOM_QUIRK_BATTERY     0x0008


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
    BTN_TOOL_QUADTAP,
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

/* Absolute axis names */
enum {
    ABS_WHEEL,
    ABS_TILT_X,
    ABS_TILT_Y,
    ABS_X,
    ABS_Y,
    ABS_Z,
    ABS_RX,
    ABS_RY,
    ABS_RZ,
    ABS_DISTANCE,
    ABS_THROTTLE,
    ABS_PRESSURE,
    ABS_MT_POSITION_X,
    ABS_MT_POSITION_Y,
    ABS_MT_TOUCH_MAJOR,
    ABS_MT_TOUCH_MINOR,
    ABS_MT_WIDTH_MAJOR,
    ABS_MT_WIDTH_MINOR,
    ABS_MT_ORIENTATION,

    ABS_MAX,
    ABS_CNT = (ABS_MAX + 1)
} Absolute_Axis;

struct absinfo {
    int32 value;
    int32 minimum;
    int32 maximum;
    int32 fuzz;
    int32 flat;
    int32 resolution;
};

/* Wacom BTN type */
enum {
    BUTTON_ACTION_SIZE      = 64,
    // add below, any new value cannot exceed BUTTON_ACTION_SIZE

    BTN_STYLUS3             = 37,
    SW_MUTE_DEVICE          = 36,
    KEY_INFO                = 35,
    KEY_BUTTONCONFIG        = 34,
    KEY_ONSCREEN_KEYBOARD   = 33,
    KEY_CONTROLPANEL        = 32,
    BTN_LEFT                = 31,
    BTN_MIDDLE              = 30,
    BTN_RIGHT               = 29,
    BTN_SIDE                = 28,
    BTN_EXTRA               = 27,
    BTN_TOUCH               = 26,
    BTN_STYLUS              = 25,
    BTN_STYLUS2             = 24,
    BTN_FORWARD             = 23,
    BTN_BACK                = 22,
    KEY_PROG3               = 21,
    KEY_PROG2               = 20,
    KEY_PROG1               = 19,
    BTN_BASE3               = 18,
    BTN_BASE2               = 17,
    BTN_BASE                = 16,
    BTN_Z                   = 15,
    BTN_Y                   = 14,
    BTN_X                   = 13,
    BTN_C                   = 12,
    BTN_B                   = 11,
    BTN_A                   = 10,
    BTN_9                   = 9,
    BTN_8                   = 8,
    BTN_7                   = 7,
    BTN_6                   = 6,
    BTN_5                   = 5,
    BTN_4                   = 4,
    BTN_3                   = 3,
    BTN_2                   = 2,
    BTN_1                   = 1,
    BTN_0                   = 0,
} WacomButtons;

struct wacom_features {
    const char *name;
    int pktlen;
    int x_max;
    int y_max;
    int pressure_max;
    int distance_max;
    int type;
    int x_resolution;
    int y_resolution;
    int numbered_buttons;
    int offset_left;
    int offset_right;
    int offset_top;
    int offset_bottom;
    int device_type;
    int x_phy;
    int y_phy;
    unsigned char unit;
    unsigned char unitExpo;
    int x_fuzz;
    int y_fuzz;
    int pressure_fuzz;
    int distance_fuzz;
    int tilt_fuzz;
    unsigned quirks;
    unsigned touch_max;
    int oVid;
    int oPid;
};

struct wacom_battery {
    //struct power_supply battery;
    //struct power_supply ac;
    //char bat_name[WACOM_NAME_MAX];
    //char ac_name[WACOM_NAME_MAX];
    int bat_status;
    int battery_capacity;
    int bat_charging;
    int bat_connected;
    int ps_connected;
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
    GID_CURRENTTILTX,
    GID_CURRENTTILTY,
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
    GID_BTN_PROG1,
    GID_BTN_PROG2,
    GID_BTN_PROG3,
    GID_BTN_BACK,
    GID_BTN_FORWARD,
            
    GID_LAST
} gids;
////

struct WacomState {
    uint32                  X;
    uint32                  Y;
    uint32                  Z;
    uint32                  tiltX;
    uint32                  tiltY;
    uint32                  tiltZ;
    uint32                  Pressure;
    uint32                  distance[2];
    BOOL                    proximity[2];
    int32                   wheel[2];
};

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

    struct LocaleInfo       localeInfo;

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
    uint64                  Buttons;        // Latest tablet buttons states as reported by last tablet message
    uint64                  PrevButtons;    // Previous buttons states (usefull for execute actionshandling)n
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

    struct Library *        ApplicationBase;
    struct ApplicationIFace* IApplication;
    struct PrefsObjectsIFace* IPrefsObjects;


    struct ClassLibrary *   WindowClassLib;
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
    struct ClickTabIFace *  IClickTab;
    Class *                 ClickTabClassPtr;

    struct ClassLibrary *   ChooserClassLib;
    Class *                 ChooserClassPtr;

    BOOL                    FirstOpen;

    Object *                Window;
    struct Window*          win;
    uint32                  winsigflag;

    struct Gadget*          windowLayout;

    struct Gadget*          gadgets[GID_LAST];
    
    // features & capabilities
    struct wacom_features   features;
    uint64                  buttonCapabilities;
    uint64                  toolCapabilities;
    uint8                   touch_arbitration;
    struct absinfo          absinfo[ABS_CNT];

    struct ButtonAction     buttonAction[BUTTON_ACTION_SIZE];
    UWORD                   Curve[7];

    uint8                   debugLevel;

    struct WacomState       currentState;
    struct WacomState       prevState;

    // From wacom_wac structure
    UBYTE                   tool[2];
    uint32                  id[2];
    BOOL                    reporting_data;
    int*                    slots;
    int                     previous_buttons;
    int                     previous_ring;
    int                     previous_keys;
    // From wacom_shared
    BOOL                    touch_down;
    BOOL                    stylus_in_proximity;
    BOOL                    is_touch_on;
    // From wacom structure
    struct wacom_battery    battery;
    // Temporary
    BOOL                    inputPropDirect;
    BOOL                    holdClick; // switched by ACTION_HOLD_CLIC
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
uint32                  SendRawKeyEvent(struct usbtablet *um, uint32 buttons);
uint32                  HandleExecuteActions(struct usbtablet *ut, struct ButtonAction buttonAction[], uint64 buttons);

void                    WacomSetupCapabilities(struct usbtablet *um);
void                    wacom_setup_device_quirks(struct usbtablet *wacom);

CONST_STRPTR            GetString(struct LocaleInfo *li, LONG stringNum);

void                    input_set_abs_params(struct usbtablet *um, uint32 axis, int32 min, int32 max, int32 fuzz, int32 flat);
#define INPUT_DECLARE_ABS_ACCESSORS(_suffix) \
int32                   input_abs_get_##_suffix(struct usbtablet *dev, uint32 axis); \
void                    input_abs_set_##_suffix(struct usbtablet *dev, uint32 axis, int32 val);

INPUT_DECLARE_ABS_ACCESSORS(val)
INPUT_DECLARE_ABS_ACCESSORS(min)
INPUT_DECLARE_ABS_ACCESSORS(max)
INPUT_DECLARE_ABS_ACCESSORS(fuzz)
INPUT_DECLARE_ABS_ACCESSORS(flat)
INPUT_DECLARE_ABS_ACCESSORS(res)

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

