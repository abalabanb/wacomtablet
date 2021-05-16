/*

************************************************************
**
** Created by: CodeBench 0.23 (17.09.2011)
**
** Project: WacomTablet.usbfd
**
** File: Tablet handler functions
**
** Date: 16-05-2021 19:31:01
**
** Copyright 2012-2021 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
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

#include "WacomTablet.h"

/// Other useful macros
#define LE_INT16(a) ((int16)((((uint16)a)>>8)|(((uint16)a)<<8)))
#define BE_INT16(a) (a)
///

static void wacom_report_numbered_buttons(/*struct input_dev *input_dev,*/
                int button_count, int mask, uint32 *pButtons);
static int WacomCreateSlots(struct usbtablet *um);
static inline BOOL delay_pen_events(struct usbtablet *wacom);
static inline BOOL report_touch_events(struct usbtablet *wacom);
static void WacomHandler_intuos(struct usbtablet *wacom);

static int input_abs_get_res(struct usbtablet* wacom, int param)
{
    switch(param)
    {
        default:
            DebugLog(0, wacom, "%s: unknown parameter %d\n", __func__, param);
            return -1;
        case ABS_X:
            return wacom->maxX-wacom->minX;
            break;
        case ABS_Y:
            return wacom->maxY-wacom->minY;
            break;
    }
}

static inline uint16 get_unaligned_be16(const uint8 *p)
{
    return p[0] << 8 | p[1];
}

static inline unsigned long __fls(unsigned long x)
{
    int lz;

    asm("cntlzw %0,%1" : "=r" (lz) : "r" (x));
    return sizeof(unsigned long) - 1 - lz;
}

static unsigned long int_sqrt(unsigned long x)
{
    unsigned long b, m, y = 0;

    if(x <= 1)
        return x;

    m = 1UL << (__fls(x) & ~1UL);
    while(m != 0) {
        b = y +m;
        y >>= 1;

        if(x >= b) {
            x -= b;
            y += m;
        }
        m >>= 2;
    }

    return y;
}

/* device quirks */
#define WACOM_QUIRK_BBTOUCH_LOWRES  0x0001
#define WACOM_QUIRK_NO_INPUT        0x0002
#define WACOM_QUIRK_MONITOR         0x0004
#define WACOM_QUIRK_BATTERY         0x0008

/*
 * Scale factor relating reported contact size to logical contact area.
 * 2^14/pi is a good approximation on Intuos5 and 3rd-gen Bamboo
 */
#define WACOM_CONTACT_AREA_SCALE 2607

static void WacomSetupBasicProPen(struct usbtablet *um)
{
    //input_set_capability(input_dev, EV_MSC, MSC_SERIAL);

    SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
    SETBITS(um->toolCapabilities, BTN_STYLUS, 1);
    SETBITS(um->toolCapabilities, BTN_STYLUS2, 1);

    SetAbsParams(um, ABS_DISTANCE,
                 0, um->features->distance_max, um->features->distance_fuzz, 0);
}


static void WacomSetupCintiq(struct usbtablet *um)
{
    WacomSetupBasicProPen(um);

    SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
    SETBITS(um->toolCapabilities, BTN_TOOL_BRUSH, 1);
    SETBITS(um->toolCapabilities, BTN_TOOL_PENCIL, 1);
    SETBITS(um->toolCapabilities, BTN_TOOL_AIRBRUSH, 1);

    SetAbsParams(um, ABS_WHEEL, 0, 1023, 0, 0);
    SetAbsParams(um, ABS_TILT_X, 0, 127, um->features->tilt_fuzz, 0);
    SetAbsParams(um, ABS_TILT_Y, 0, 127, um->features->tilt_fuzz, 0);
}

static void WacomSetupIntuos(struct usbtablet *um)
{
    //input_set_capability(input_dev, EV_REL, REL_WHEEL);

    WacomSetupCintiq(um);

    SETBITS(um->buttonCapabilities, BTN_LEFT, 1);
    SETBITS(um->buttonCapabilities, BTN_RIGHT, 1);
    SETBITS(um->buttonCapabilities, BTN_MIDDLE, 1);
    SETBITS(um->buttonCapabilities, BTN_SIDE, 1);
    SETBITS(um->buttonCapabilities, BTN_EXTRA, 1);
    SETBITS(um->toolCapabilities, BTN_TOOL_MOUSE, 1);
    SETBITS(um->toolCapabilities, BTN_TOOL_LENS, 1);

    SetAbsParams(um, ABS_RZ, -900, 899, 0, 0);
    um->minThrottle = -1023;
    um->maxThrottle = 1023;
}

/// Setup device quirks
void WacomSetupDeviceQuirks(struct usbtablet *um)
{
    struct wacom_features *features = um->features;

    /* touch device found but size is not defined. use default */
    if (features->device_type == BTN_TOOL_FINGER && !features->x_max) {
        features->x_max = 1023;
        features->y_max = 1023;
    }

    /*
     * Intuos5/Pro and Bamboo 3rd gen have no useful data about its
     * touch interface in its HID descriptor. If this is the touch
     * interface (wMaxPacketSize of WACOM_PKGLEN_BBTOUCH3), override
     * the tablet values.
     */
    if ((features->type >= INTUOS5S && features->type <= INTUOSPL) ||
        (features->type >= INTUOSHT && features->type <= BAMBOO_PT)) {
        if ((LE_WORD(um->UsbEndPointDscrIn->ed_MaxPacketSize) && USBEP_SIZEM_MAXPACKETSIZE) == WACOM_PKGLEN_BBTOUCH3) {
            features->device_type = BTN_TOOL_FINGER;
            features->pktlen = WACOM_PKGLEN_BBTOUCH3;

            if (features->type == INTUOSHT2) {
                features->x_max = features->x_max / 10;
                features->y_max = features->y_max / 10;
            }
            else {
                features->x_max = 4096;
                features->y_max = 4096;
            }
        } else {
            features->device_type = BTN_TOOL_PEN;
        }
    }

    /* quirk for bamboo touch with 2 low res touches */
    if (features->type == BAMBOO_PT &&
        features->pktlen == WACOM_PKGLEN_BBTOUCH) {
        features->x_max <<= 5;
        features->y_max <<= 5;
        features->x_fuzz <<= 5;
        features->y_fuzz <<= 5;
        features->quirks |= WACOM_QUIRK_BBTOUCH_LOWRES;
    }

    if (features->type == WIRELESS) {

        /* monitor never has input and pen/touch have delayed create */
        features->quirks |= WACOM_QUIRK_NO_INPUT;

        /* must be monitor interface if no device_type set */
        if (!features->device_type)
            features->quirks |= WACOM_QUIRK_MONITOR;
            features->quirks |= WACOM_QUIRK_BATTERY;
    }

    if (features->type == REMOTE)
        features->quirks |= WACOM_QUIRK_MONITOR;
}
////

/// Setup tablet capabilities
void WacomSetupCapabilities(struct usbtablet* um)
{
    const struct wacom_features *features = um->features;
    int err;

    //if (features->type == REMOTE && input_dev == wacom_wac->input)
    //    return -ENODEV;

    //input_dev->evbit[0] |= BIT_MASK(EV_KEY) | BIT_MASK(EV_ABS);

    um->toolCapabilities = 0L;
    um->buttonCapabilities = 0L;

    SETBITS(um->buttonCapabilities, BTN_TOUCH, 1);
//  __set_bit(ABS_MISC, input_dev->absbit);

//    wacom_abs_set_axis(input_dev, wacom_wac);

    switch (features->type) {
    case REMOTE:
        /* kept for making legacy xf86-input-wacom accepting the pad */
        SetAbsParams(um, ABS_X, 0, 1, 0, 0);
        SetAbsParams(um, ABS_Y, 0, 1, 0, 0);

        /* kept for making udev and libwacom accepting the pad */
        SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
        SETBITS(um->buttonCapabilities, BTN_TOUCH, 0);
        //input_set_capability(input_dev, EV_MSC, MSC_SERIAL);
        SetAbsParams(um, ABS_WHEEL, 0, 71, 0, 0);
        break;

    case WACOM_MO:
        SetAbsParams(um, ABS_WHEEL, 0, 71, 0, 0);
        /* fall through */

    case WACOM_G4:
        //input_set_capability(input_dev, EV_MSC, MSC_SERIAL);
        SetAbsParams(um, ABS_DISTANCE, 0,
            features->distance_max, features->distance_fuzz, 0);

        SETBITS(um->buttonCapabilities, BTN_BACK, 1);
        SETBITS(um->buttonCapabilities, BTN_FORWARD, 1);
        /* fall through */

    case GRAPHIRE:
        //input_set_capability(input_dev, EV_REL, REL_WHEEL);

        SETBITS(um->buttonCapabilities, BTN_LEFT, 1);
        SETBITS(um->buttonCapabilities, BTN_RIGHT, 1);
        SETBITS(um->buttonCapabilities, BTN_MIDDLE, 1);

        SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
        SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
        SETBITS(um->toolCapabilities, BTN_TOOL_MOUSE, 1);
        SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
        SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);

        // __set_bit(INPUT_PROP_POINTER, input_dev->propbit);
    break;

    case WACOM_MSPRO:
    case CINTIQ_16:
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);
        SETBITS(um->buttonCapabilities, BTN_STYLUS3, 1);
        um->inputPropDirect = TRUE;

        if (features->type == WACOM_MSPRO &&
            features->numbered_buttons == 0) { /* Cintiq Pro */
            SETBITS(um->buttonCapabilities, KEY_CONTROLPANEL, 1);
            SETBITS(um->buttonCapabilities, KEY_ONSCREEN_KEYBOARD, 1);
            SETBITS(um->buttonCapabilities, KEY_BUTTONCONFIG, 1);

            //wacom_wac->previous_ring = WACOM_INTUOSP2_RING_UNTOUCHED;
        }

        WacomSetupCintiq(um);
        break;

    case WACOM_24HD:
        SETBITS(um->buttonCapabilities, KEY_PROG1, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG2, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG3, 1);

        SETBITS(um->buttonCapabilities, KEY_ONSCREEN_KEYBOARD, 1);
        SETBITS(um->buttonCapabilities, KEY_INFO, 1);

        if (!features->oPid)
        {    SETBITS(um->buttonCapabilities, KEY_BUTTONCONFIG, 1); }

        SetAbsParams(um, ABS_THROTTLE, 0, 71, 0, 0);
        /* fall through */

    case WACOM_13HD:
    case CINTIQ_HYBRID:
    case CINTIQ_COMPANION_2:
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);
        /* fall through */

    case DTK:
        um->inputPropDirect = TRUE;

        WacomSetupCintiq(um);
        break;

    case WACOM_27QHD:
        SETBITS(um->buttonCapabilities, KEY_PROG1, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG2, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG3, 1);

        SETBITS(um->buttonCapabilities, KEY_ONSCREEN_KEYBOARD, 1);
        SETBITS(um->buttonCapabilities, KEY_BUTTONCONFIG, 1);

        if (!features->oPid)
        {    SETBITS(um->buttonCapabilities, KEY_CONTROLPANEL, 1); }

        um->inputPropDirect = TRUE;
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);

        WacomSetupCintiq(um);
        break;

    case WACOM_22HD:
        SETBITS(um->buttonCapabilities, KEY_PROG1, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG2, 1);
        SETBITS(um->buttonCapabilities, KEY_PROG3, 1);

        SETBITS(um->buttonCapabilities, KEY_BUTTONCONFIG, 1);
        SETBITS(um->buttonCapabilities, KEY_INFO, 1);
        /* fall through */

    case WACOM_21UX2:
    case WACOM_BEE:
    case CINTIQ:
        SetAbsParams(um, ABS_RX, 0, 4096, 0, 0);
        SetAbsParams(um, ABS_RY, 0, 4096, 0, 0);
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);
        um->inputPropDirect = TRUE;

        WacomSetupCintiq(um);
        break;

    case INTUOS3:
    case INTUOS3L:
        SetAbsParams(um, ABS_RY, 0, 4096, 0, 0);
        /* fall through */

    case INTUOS3S:
        SetAbsParams(um, ABS_RX, 0, 4096, 0, 0);
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);
        /* fall through */

    case INTUOS:
        //__set_bit(INPUT_PROP_POINTER, input_dev->propbit);

        WacomSetupIntuos(um);
        break;

    case INTUOSP2:
    case INTUOSP2S:
        if (features->device_type == BTN_TOOL_PEN) {
            SETBITS(um->buttonCapabilities, BTN_STYLUS3, 1);
            //wacom_wac->previous_ring = WACOM_INTUOSP2_RING_UNTOUCHED;
        }
        else {
            //input_dev->evbit[0] |= BIT_MASK(EV_SW);
            SETBITS(um->buttonCapabilities, SW_MUTE_DEVICE, 1);
            //wacom_wac->shared->has_mute_touch_switch = true;
        }
        err = WacomCreateSlots(um);
        if (err)
        {
            DebugLog(20, um, "WacomSetupCapabilities: Cannot create wacom slots\n");
            return;
        }
        /* fall through */

    case INTUOS5:
    case INTUOS5L:
    case INTUOSPM:
    case INTUOSPL:
    case INTUOS5S:
    case INTUOSPS:
        //__set_bit(INPUT_PROP_POINTER, input_dev->propbit);

        if (features->device_type == BTN_TOOL_PEN) {
            SetAbsParams(um, ABS_DISTANCE, 0,
                          features->distance_max,
                          features->distance_fuzz, 0);

            SetAbsParams(um, ABS_Z, -900, 899, 0, 0);

            WacomSetupIntuos(um);
        } else if (features->device_type == BTN_TOOL_FINGER) {
            //__clear_bit(ABS_MISC, input_dev->absbit);

            /* pad is on pen interface */
            //numbered_buttons = 0;
            SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);
            SETBITS(um->toolCapabilities, BTN_TOOL_DOUBLETAP, 1);
            SETBITS(um->toolCapabilities, BTN_TOOL_TRIPLETAP, 1);
            SETBITS(um->toolCapabilities, BTN_TOOL_QUADTAP, 1);

            //input_mt_init_slots(input_dev, features->touch_max);

            //input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR,
            //                     0, features->x_max, 0, 0);
            //input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
            //                     0, features->y_max, 0, 0);

            //input_set_abs_params(input_dev, ABS_MT_POSITION_X,
            //             0, features->x_max,
            //             features->x_fuzz, 0);
            //input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
            //             0, features->y_max,
            //             features->y_fuzz, 0);
        }
        break;

    case INTUOS4:
    case INTUOS4L:
    case INTUOS4S:
        SetAbsParams(um, ABS_Z, -900, 899, 0, 0);
        WacomSetupIntuos(um);

        //__set_bit(INPUT_PROP_POINTER, input_dev->propbit);
        break;

    case WACOM_24HDT:
        if (features->device_type == BTN_TOOL_FINGER) {
            //input_set_abs_params(input_dev, ABS_MT_WIDTH_MAJOR, 0, features->x_max, 0, 0);
            //input_set_abs_params(input_dev, ABS_MT_WIDTH_MINOR, 0, features->y_max, 0, 0);
        }
        /* fall through */

    case DTH1152T:
    case DTH2452T:
    case WACOM_MSPROT:
        if (features->device_type == BTN_TOOL_FINGER) {
            //input_set_abs_params(input_dev, ABS_MT_TOUCH_MAJOR, 0, features->x_max, 0, 0);
            //if (features->type != WACOM_24HDT)
                //input_set_abs_params(input_dev, ABS_MT_TOUCH_MINOR,
                //             0, features->y_max, 0, 0);
            //input_set_abs_params(input_dev, ABS_MT_ORIENTATION, 0, 1, 0, 0);
        }
        /* fall through */

    case WACOM_27QHDT:
        //if (wacom_wac->shared->touch_input->id.product == 0x32C ||
        //    wacom_wac->shared->touch_input->id.product == 0xF6) {
        //    input_dev->evbit[0] |= BIT_MASK(EV_SW);
        //    SETBITS(um->buttonCapabilities, SW_MUTE_DEVICE, 1);
        //    wacom_wac->shared->has_mute_touch_switch = true;
        //}
        /* fall through */

    case MTSCREEN:
    case MTTPC:
    case MTTPC_B:
    case MTTPC_C:
        err = WacomCreateSlots(um);
        if (err)
            return;
        /* fall through */

    case TABLETPC2FG:
        if (features->device_type == BTN_TOOL_FINGER) {

            //input_mt_init_slots(input_dev, features->touch_max);
            //input_set_abs_params(input_dev, ABS_MT_POSITION_X,
            //        0, features->x_max, 0, 0);
            //input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
            //        0, features->y_max, 0, 0);
        }
        /* fall through */

    case TABLETPC:
    case TABLETPCE:
        //__clear_bit(ABS_MISC, input_dev->absbit);

        um->inputPropDirect = TRUE;

        //if ((features->device_type == BTN_TOOL_FINGER) &&
        //    (input_dev->id.product >= 0x353 && input_dev->id.product <= 0x356)) {
            //input_dev->evbit[0] |= BIT_MASK(EV_SW);
            //SETBITS(um->buttonCapabilities, SW_MUTE_DEVICE, 1);
            //wacom_wac->shared->has_mute_touch_switch = true;
            //wacom_wac->shared->is_touch_on = true;
        //}

        if (features->device_type != BTN_TOOL_PEN)
            break;  /* no need to process stylus stuff */

        /* fall through */

    case DTUS:
    case DTUS2:
    case DTK2451:
        //input_set_capability(input_dev, EV_MSC, MSC_SERIAL);
        /* fall through */

    case DTUSX:
    case PL:
    case DTU:
        if (features->type != DTUS2) {
            SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
            SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);
        }
        /* fall through */

    case DTH1152:
        SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
        SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);

        um->inputPropDirect = TRUE;
        break;

    case PTU:
        SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);
        /* fall through */

    case PENPARTNER:
        SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
        SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
        SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);

        // __set_bit(INPUT_PROP_POINTER, input_dev->propbit);
        break;

    case INTUOSHT:
    case INTUOSHT2:
        if (features->touch_max &&
            features->device_type == BTN_TOOL_FINGER) {
            //input_dev->evbit[0] |= BIT_MASK(EV_SW);
            SETBITS(um->buttonCapabilities, SW_MUTE_DEVICE, 1);
            //wacom_wac->shared->has_mute_touch_switch = true;
        }
        /* fall through */

    case INTUOSHT3:
    case BAMBOO_PT:
        //__clear_bit(ABS_MISC, input_dev->absbit);

        //__set_bit(INPUT_PROP_POINTER, input_dev->propbit);

        if (features->device_type == BTN_TOOL_FINGER) {
            SETBITS(um->buttonCapabilities, BTN_LEFT, 1);
            SETBITS(um->buttonCapabilities, BTN_FORWARD, 1);
            SETBITS(um->buttonCapabilities, BTN_BACK, 1);
            SETBITS(um->buttonCapabilities, BTN_RIGHT, 1);

            if (features->touch_max) {
                if (features->pktlen == WACOM_PKGLEN_BBTOUCH3) {
                    SETBITS(um->toolCapabilities, BTN_TOOL_TRIPLETAP,
                          1);
                    SETBITS(um->toolCapabilities, BTN_TOOL_QUADTAP,
                          1);

                    //input_set_abs_params(input_dev,
                    //         ABS_MT_TOUCH_MAJOR,
                    //         0, features->x_max, 0, 0);
                    //input_set_abs_params(input_dev,
                    //         ABS_MT_TOUCH_MINOR,
                    //         0, features->y_max, 0, 0);
                }

                //input_set_abs_params(input_dev, ABS_MT_POSITION_X,
                //             0, features->x_max,
                //             features->x_fuzz, 0);
                //input_set_abs_params(input_dev, ABS_MT_POSITION_Y,
                //             0, features->y_max,
                //             features->y_fuzz, 0);
                SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);
                SETBITS(um->toolCapabilities, BTN_TOOL_DOUBLETAP, 1);
                //input_mt_init_slots(input_dev, features->touch_max);
            } else {
                /* buttons/keys only interface */
                //__clear_bit(ABS_X, input_dev->absbit);
                //__clear_bit(ABS_Y, input_dev->absbit);
                SETBITS(um->buttonCapabilities,BTN_TOUCH, 0);

                /* For Bamboo, buttons are supported only when touch is supported */
                if (features->type == BAMBOO_PT) {
                    SETBITS(um->toolCapabilities, BTN_LEFT, 0);
                    SETBITS(um->toolCapabilities, BTN_FORWARD, 0);
                    SETBITS(um->toolCapabilities, BTN_BACK, 0);
                    SETBITS(um->toolCapabilities, BTN_RIGHT, 0);
                }
            }
        } else if (features->device_type == BTN_TOOL_PEN) {
            if (features->type == INTUOSHT2 || features->type == INTUOSHT3) {
                //__set_bit(ABS_MISC, input_dev->absbit);
                WacomSetupBasicProPen(um);
            } else {
                SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
                SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
                SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
                SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);
                SetAbsParams(um, ABS_DISTANCE, 0,
                          features->distance_max,
                          features->distance_fuzz, 0);
            }
        }
        break;
    }
    
    DebugLog(10, um, "WacomSetupCapabilities: buttons=%08x tools=%08x\n", um->buttonCapabilities, um->toolCapabilities);    
}

////

/// Handler PenPartner
static void WacomHandler_penpartner(struct usbtablet *um)
{
    unsigned char *data = um->UsbData;
    struct WacomState *state = &um->currentState;
    uint64 buttons = 0;

    switch (data[0]) {
    case 1:
        if (data[5] & 0x80) {
            um->tool[0] = (data[5] & 0x20) ? BTN_TOOL_RUBBER : BTN_TOOL_PEN;
            um->id[0] = (data[5] & 0x20) ? ERASER_DEVICE_ID : STYLUS_DEVICE_ID;
            state->proximity[0] = 1;
            state->X = LE_INT16(*((int16 *)&data[1]));
            state->Y = LE_INT16(*((int16 *)&data[3]));
            state->Pressure = (signed char)data[6] + 127;
            SETBITS(buttons, BTN_TOUCH, ((signed char)data[6] > -127));
            SETBITS(buttons, BTN_STYLUS, (data[5] & 0x40));
        } else {
            state->proximity[0] = 0;
            state->Pressure = -1;
            SETBITS(buttons, BTN_TOUCH, 0);
        }
        break;

    case 2:
        um->tool[0] = BTN_TOOL_PEN;
        um->id[0] = STYLUS_DEVICE_ID;
        state->proximity[0] = 1;
        state->X = LE_INT16(*((int16 *)&data[1]));
        state->Y = LE_INT16(*((int16 *)&data[3]));
        state->Pressure = (signed char)data[6] + 127;
        SETBITS(buttons, BTN_TOUCH, ((signed char)data[6] > -80) && !(data[5] & 0x20));
        SETBITS(buttons, BTN_STYLUS, (data[5] & 0x40));
        break;

    default:
        DebugLog(0, um, "WacomHandler_penpartner_irq: received report #%d\n", data[0]);
        return;
    }

    SendMouseEvent(um, buttons);
    SendTabletEvent(0, um, buttons);    
}
////

/// Handler DTU
static void WacomHandler_dtu(struct usbtablet *um)
{
    UBYTE *data = um->UsbData;
    struct WacomState *state = &um->currentState;
    int prox = data[1] & 0x20;
    uint64 buttons = 0;

    DebugLog(20, um, "WacomHandler_dtu_irq: received report #%d\n", data[0]);

    if (prox) {
        /* Going into proximity select tool */
        um->tool[0] = (data[1] & 0x0c) ? BTN_TOOL_RUBBER : BTN_TOOL_PEN;
        if (um->tool[0] == BTN_TOOL_PEN)
            um->id[0] = STYLUS_DEVICE_ID;
        else
            um->id[0] = ERASER_DEVICE_ID;
    }
    SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
    SETBITS(buttons, BTN_STYLUS2, data[1] & 0x10);
    state->X = LE_INT16(*((int16 *)&data[2]));
    state->Y = LE_INT16(*((int16 *)&data[4]));
    state->Pressure = ((data[7] & 0x01) << 8) | data[6];
    SETBITS(buttons, BTN_TOUCH, data[1] & 0x05);
    if (!prox) { /* out-prox */
        um->id[0] = 0;
    }
    state->proximity[0] = prox;
    //input_report_abs(input, ABS_MISC, wacom->id[0]);
    SendMouseEvent(um, buttons);
    SendTabletEvent(0, um, buttons);
}
////

/// Handler DTUS
static int WacomHandler_dtus(struct usbtablet *um)
{
    UBYTE *data = um->UsbData;
    struct wacom_features *features = um->features;
    struct WacomState *state = &um->currentState;
    unsigned short prox, pressure = 0;
    uint64 buttons = 0;

    if (data[0] != WACOM_REPORT_DTUS && data[0] != WACOM_REPORT_DTUSPAD
            && data[0] != WACOM_REPORT_DTK2451PAD) {
        DebugLog(40, um,
            "%s: received unknown report #%d", __func__, data[0]);
        return 0;
    } else if (data[0] == WACOM_REPORT_DTUSPAD
            || data[0] == WACOM_REPORT_DTK2451PAD) {
        SETBITS(buttons, BTN_0, (data[1] & 0x01));
        SETBITS(buttons, BTN_1, (data[1] & 0x02));
        SETBITS(buttons, BTN_2, (data[1] & 0x04));
        SETBITS(buttons, BTN_3, (data[1] & 0x08));
        //input_report_abs(input, ABS_MISC,
        //         data[1] & 0x0f ? PAD_DEVICE_ID : 0);
        /*
         * Serial number is required when expresskeys are
         * reported through pen interface.
         */
        //input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
        SendMouseEvent(um, buttons);
        SendTabletEvent(0, um, buttons);
        return 1;
    } else {
        prox = data[1] & 0x80;
        if (prox) {
            switch ((data[1] >> 3) & 3) {
            case 1: /* Rubber */
                um->tool[0] = BTN_TOOL_RUBBER;
                um->id[0] = ERASER_DEVICE_ID;
                break;

            case 2: /* Pen */
                um->tool[0] = BTN_TOOL_PEN;
                um->id[0] = STYLUS_DEVICE_ID;
                break;
            }
        }

        SETBITS(buttons, BTN_STYLUS, data[1] & 0x20);
        SETBITS(buttons, BTN_STYLUS2, data[1] & 0x40);
        if (features->type  == DTK2451) {
            pressure = LE_INT16(*((int16 *)&data[2]));
            state->X = LE_INT16(*((int16 *)&data[4]));
            state->Y = LE_INT16(*((int16 *)&data[6]));
        } else {
            pressure = ((data[1] & 0x03) << 8) | (data[2] & 0xff);
            state->X = BE_INT16(*((int16 *)&data[3]));
            state->Y = BE_INT16(*((int16 *)&data[5]));
        }
        state->Pressure = pressure;
        SETBITS(buttons, BTN_TOUCH, pressure > 10);

        if (!prox) /* out-prox */
            um->id[0] = 0;
        um->tool[0]= prox;
        //input_report_abs(input, ABS_MISC, wacom->id[0]);
        //input_event(input, EV_MSC, MSC_SERIAL, 1);
        SendMouseEvent(um, buttons);
        SendTabletEvent(0, um, buttons);
        return 1;
    }
}
///

/// Handler DTH1152
static int WacomHandler_dth1152(struct usbtablet *wacom)
{
    unsigned char *data = wacom->UsbData;
    struct WacomState *state = &wacom->currentState;
    unsigned short prox, pressure = 0;
    uint64 buttons = 0;

    if (data[0] != WACOM_REPORT_DTUS) {
        if (data[0] == WACOM_REPORT_DTUSPAD) {
            SETBITS(buttons, BTN_0, (data[1] & 0x01));
            SETBITS(buttons, BTN_1, (data[1] & 0x02));
            SETBITS(buttons, BTN_2, (data[1] & 0x04));
            SETBITS(buttons, BTN_3, (data[1] & 0x08));
            //input_report_abs(input, ABS_MISC,
            //     data[1] & 0x0f ? PAD_DEVICE_ID : 0);
            /*
             * Serial number is required when expresskeys are
             * reported through pen interface.
             */
            //input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
            SendMouseEvent(wacom, buttons);
            SendTabletEvent(0, wacom, buttons);
            return 1;
        }
        DebugLog(40, wacom,
            "%s: received unknown report #%d", __func__, data[0]);
        return 0;
    } else {
        prox = data[1] & 0x80;
        if (prox) {
            wacom->tool[0] = BTN_TOOL_PEN;
            wacom->id[0] = STYLUS_DEVICE_ID;
        }
        SETBITS(buttons, BTN_STYLUS, data[1] & 0x20);
        state->X = LE_INT16(*((int16 *)&data[4]));
        state->Y = LE_INT16(*((int16 *)&data[6]));
        pressure = data[2] | (data[3] << 8);
        state->Pressure = pressure;
        SETBITS(buttons, BTN_TOUCH, data[1] & 0x10);

        if (!prox)
            wacom->id[0] = 0;
        wacom->tool[0] = prox;
        //input_report_abs(input, ABS_MISC, wacom->id[0]);
        SendMouseEvent(wacom, buttons);
        SendTabletEvent(0, wacom, buttons);
        return 1;
    }
}

///

/// Handler PL
static void WacomHandler_pl(struct usbtablet *um)
{
    const struct wacom_features *features = um->features;
    unsigned char *data = um->UsbData;
    int prox, pressure;
    uint64 buttons = 0;
    struct WacomState *state = &um->currentState;

    if (data[0] != WACOM_REPORT_PENABLED) {
        DebugLog(0, um, "WacomHandler_pl_irq: received unknown report #%d", data[0]);
        return;
    }

    prox = data[1] & 0x40;

    if (prox) {
        um->id[0] = ERASER_DEVICE_ID;
        pressure = (signed char)((data[7] << 1) | ((data[4] >> 2) & 1));
        if (features->pressure_max > 255)
            pressure = (pressure << 1) | ((data[4] >> 6) & 1);
        pressure += (features->pressure_max + 1) / 2;

        /*
         * if going from out of proximity into proximity select between the eraser
         * and the pen based on the state of the stylus2 button, choose eraser if
         * pressed else choose pen. if not a proximity change from out to in, send
         * an out of proximity for previous tool then a in for new tool.
         */
        if (!state->proximity[0]) {
            /* Eraser bit set for DTF */
            if (data[1] & 0x10)
                um->tool[1] = BTN_TOOL_RUBBER;
            else
                /* Going into proximity select tool */
                um->tool[1] = (data[4] & 0x20) ? BTN_TOOL_RUBBER : BTN_TOOL_PEN;
        } else {
            /* was entered with stylus2 pressed */
            if (um->tool[1] == BTN_TOOL_RUBBER && !(data[4] & 0x20)) {
                /* report out proximity for previous tool */
                state->proximity[1] = 0;
                SendTabletEvent(1, um, buttons);
                um->tool[1] = BTN_TOOL_PEN;
                return;
            }
        }
        if (um->tool[1] != BTN_TOOL_RUBBER) {
            /* Unknown tool selected default to pen tool */
            um->tool[1] = BTN_TOOL_PEN;
            um->id[0] = STYLUS_DEVICE_ID;
        }
        state->proximity[1] = prox; /* report in proximity for tool */
        //input_report_abs(input, ABS_MISC, wacom->id[0]); /* report tool id */
        state->X = data[3] | (data[2] << 7) | ((data[1] & 0x03) << 14);
        state->Y = data[6] | (data[5] << 7) | ((data[4] & 0x03) << 14);
        state->Pressure = pressure;

        SETBITS(buttons, BTN_TOUCH, data[4] & 0x08);
        SETBITS(buttons, BTN_STYLUS, data[4] & 0x10);
        /* Only allow the stylus2 button to be reported for the pen tool. */
        SETBITS(buttons, BTN_STYLUS2, (um->tool[1] == BTN_TOOL_PEN) && (data[4] & 0x20));
    } else {
        /* report proximity-out of a (valid) tool */
        if (um->tool[1] != BTN_TOOL_RUBBER) {
            /* Unknown tool selected default to pen tool */
            um->tool[1] = BTN_TOOL_PEN;
        }
        state->proximity[1] = prox;
    }

    state->proximity[0] = prox; /* Save proximity state */

    SendMouseEvent(um, buttons);
    SendTabletEvent(1, um, buttons);
}
////

/// Handler PTU
static void WacomHandler_ptu(struct usbtablet *um)
{
    unsigned char *data = um->UsbData;
    uint64 buttons = 0;
    struct WacomState *state = &um->currentState;

    if (data[0] != WACOM_REPORT_PENABLED) {
        DebugLog(0, um, "WacomHandler_ptu_irq: received unknown report #%d\n", data[0]);
        return;
    }

    if (data[1] & 0x04) {
        um->tool[0] = BTN_TOOL_RUBBER;
        state->proximity[0] = data[1] & 0x20;
        SETBITS(buttons, BTN_TOUCH, data[1] & 0x08);
        um->id[0] = ERASER_DEVICE_ID;
    } else {
        um->tool[0] = BTN_TOOL_PEN;
        state->proximity[0] = data[1] & 0x20;
        SETBITS(buttons, BTN_TOUCH, data[1] & 0x01);
        um->id[0] = STYLUS_DEVICE_ID;
    }
    //input_report_abs(input, ABS_MISC, wacom->id[0]); /* report tool id */
    state->X = LE_INT16(*((int16 *)&data[2]));
    state->Y = LE_INT16(*((int16 *)&data[4]));
    state->Pressure = LE_INT16(*((int16 *)&data[6]));
    SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
    SETBITS(buttons, BTN_STYLUS2, data[1] & 0x10);
    
    SendMouseEvent(um, buttons);
    SendTabletEvent(0, um, buttons);
}
////

/// Handler Bamboo pen and touch
static void WacomHandler_bpt_touch(struct usbtablet *um)
{
    uint64 buttons = 0;
    const struct wacom_features *features = um->features;
    UBYTE *data = um->UsbData;
    int i = 0;
    struct WacomState *state = &um->currentState;

    if (data[0] != 0x02)
        return;

    for (i = 0; i < 2; i++) {
        int offset = (data[1] & 0x80) ? (8 * i) : (9 * i);
        BOOL touch = report_touch_events(um)
               && (data[offset + 3] & 0x80);

        //ABA: to be investigated input_mt_slot(input, i);
        um->tool[0] = touch?BTN_TOOL_FINGER:0;
        if (touch) {
            int x = BE_INT16(*((int16 *)(&data[offset + 3]))) & 0x7ff;
            int y = BE_INT16(*((int16 *)(&data[offset + 5]))) & 0x7ff;
            if (features->quirks & WACOM_QUIRK_BBTOUCH_LOWRES) {
                x <<= 5;
                y <<= 5;
            }
            state->X = x;
            state->Y = y;
        }
    }

    // input_mt_report_pointer_emulation(input, true);

    SETBITS(buttons, BTN_LEFT, (data[1] & 0x08) != 0);
    SETBITS(buttons, BTN_FORWARD, (data[1] & 0x04) != 0);
    SETBITS(buttons, BTN_BACK, (data[1] & 0x02) != 0);
    SETBITS(buttons, BTN_RIGHT, (data[1] & 0x01) != 0);
    //wacom->shared->touch_down = wacom_wac_finger_count_touches(wacom);

    SendTabletEvent(0, um, buttons);
    SendMouseEvent(um, buttons);
}

static void WacomHandler_bpt3_touch_msg(struct usbtablet *um, unsigned char *data, uint64 * pButtons)
{
    const struct wacom_features *features = um->features;
    //struct input_dev *input = wacom->input;
    BOOL touch = data[1] & 0x80;
    //int slot = data[0] - 2;  /* data[0] is between 2 and 17 */
    uint64 buttons = *pButtons;
    struct WacomState *state = &um->currentState;

    touch = touch && report_touch_events(um);

    // NOTE ABA: AmigaOS does not support MT as of yet, as a
    // consequence, we will only report a finger tool with one
    // touch point

    //input_mt_slot(input, slot);
    //input_mt_report_slot_state(input, MT_TOOL_FINGER, touch);
    um->tool[0] = touch?BTN_TOOL_FINGER:0;

    if (touch) {
        int x = (data[2] << 4) | (data[4] >> 4);
        int y = (data[3] << 4) | (data[4] & 0x0f);
        int width, height;

        if (features->type >= INTUOSPS && features->type <= INTUOSPL) {
            width  = data[5] * 100;
            height = data[6] * 100;
        } else {
            /*
             * "a" is a scaled-down area which we assume is
             * roughly circular and which can be described as:
             * a=(pi*r^2)/C.
             */
            int a = data[5];
            int x_res  = input_abs_get_res(um, ABS_X);
            int y_res  = input_abs_get_res(um, ABS_Y);
            width  = 2 * int_sqrt(a * WACOM_CONTACT_AREA_SCALE);
            height = width * y_res / x_res;
        }

        state->X = x;
        state->Y = y;
        //input_report_abs(input, ABS_MT_TOUCH_MAJOR, width);
        //input_report_abs(input, ABS_MT_TOUCH_MINOR, height);
    }
    *pButtons = buttons;
}

static void WacomHandler_bpt3_button_msg(struct usbtablet *um, unsigned char *data, uint64 * pButtons)
{
    const struct wacom_features *features = um->features;
    uint32 buttons = *pButtons;

    if (features->type == INTUOSHT || features->type == INTUOSHT2) {
        SETBITS(buttons, BTN_LEFT, (data[1] & 0x02) != 0);
        SETBITS(buttons, BTN_BACK, (data[1] & 0x08) != 0);
    } else {
        SETBITS(buttons, BTN_BACK, (data[1] & 0x02) != 0);
        SETBITS(buttons, BTN_LEFT, (data[1] & 0x08) != 0);
    }
    SETBITS(buttons, BTN_FORWARD, (data[1] & 0x04) != 0);
    SETBITS(buttons, BTN_RIGHT, (data[1] & 0x01) != 0);

    *pButtons = buttons;
}

static int WacomHandler_bpt3_touch(struct usbtablet *um)
{
    UBYTE *data = um->UsbData;
    uint64 buttons = 0;
    int count = data[1] & 0x07;
    int touch_changed = 0, i;

    if (data[0] != 0x02)
        return 0;

    /* data has up to 7 fixed sized 8-byte messages starting at data[2] */
    for (i = 0; i < count; i++) {
        int offset = (8 * i) + 2;
        int msg_id = data[offset];

        if (msg_id >= 2 && msg_id <= 17) {
            WacomHandler_bpt3_touch_msg(um, data + offset, &buttons);
            touch_changed++;
        } else if (msg_id == 128)
            WacomHandler_bpt3_button_msg(um, data + offset, &buttons);

    }

    //if (touch_changed) {
        //input_mt_report_pointer_emulation(input, TRUE);
        //wacom->shared->touch_down = wacom_wac_finger_count_touches(wacom);
    //}

    SendTabletEvent(0, um, buttons);
    SendMouseEvent(um, buttons);

    return 0;
}

static void WacomHandler_bpt_pen(struct usbtablet *um)
{
    const struct wacom_features *features = um->features;
    UBYTE *data = um->UsbData;
    int x = 0, y = 0, p = 0, d = 0;
    BOOL pen = FALSE, btn1 = FALSE, btn2 = FALSE;
    BOOL range, prox, rdy;
    uint64 buttons = 0;
    struct WacomState *state = &um->currentState;

    if (data[0] != WACOM_REPORT_PENABLED)
        return;

    range = (data[1] & 0x80) == 0x80;
    prox = (data[1] & 0x40) == 0x40;
    rdy = (data[1] & 0x20) == 0x20;

    um->stylus_in_proximity = range;
    if (delay_pen_events(um))
        return;

    if (rdy) {
        p = LE_INT16(*((int16 *)&data[6]));
        pen = data[1] & 0x01;
        btn1 = data[1] & 0x02;
        btn2 = data[1] & 0x04;
    }
    if (prox) {
        x = LE_INT16(*((int16 *)&data[2]));
        y = LE_INT16(*((int16 *)&data[4]));

        if (data[1] & 0x08) {
            um->tool[0] = BTN_TOOL_RUBBER;
            um->id[0] = ERASER_DEVICE_ID;
        } else {
            um->tool[0] = BTN_TOOL_PEN;
            um->id[0] = STYLUS_DEVICE_ID;
        }
        um->reporting_data = TRUE;
    }
    if (range) {
        /*
         * Convert distance from out prox to distance from tablet.
         * distance will be greater than distance_max once
         * touching and applying pressure; do not report negative
         * distance.
         */
        if (data[8] <= features->distance_max)
            d = features->distance_max - data[8];
    } else {
        um->id[0] = 0;
    }

    if (um->reporting_data) {
        SETBITS(buttons, BTN_TOUCH, pen);
        SETBITS(buttons, BTN_STYLUS, btn1);
        SETBITS(buttons, BTN_STYLUS2, btn2);

        if (prox || !range) {
            state->X = x;
            state->Y = y;
        }
        state->Pressure = p;
        state->distance[0] = d;

        //input_report_key(input, wacom->tool[0], range); /* PEN or RUBBER */
        //input_report_abs(input, ABS_MISC, wacom->id[0]); /* TOOL ID */
    }

    if (!range) {
        um->reporting_data = FALSE;
    }

    SendTabletEvent(0, um, buttons);
    SendMouseEvent(um, buttons);

    return;
}

static void WacomHandler_bpt(struct usbtablet *um, size_t len)
{
    struct wacom_features *features = um->features;

    if ((features->type == INTUOSHT2) &&
        (features->device_type == BTN_TOOL_PEN))
        WacomHandler_intuos(um);
    else if (len == WACOM_PKGLEN_BBTOUCH)
        WacomHandler_bpt_touch(um);
    else if (len == WACOM_PKGLEN_BBTOUCH3)
        WacomHandler_bpt3_touch(um);
    else if (len == WACOM_PKGLEN_BBFUN || len == WACOM_PKGLEN_BBPEN)
        WacomHandler_bpt_pen(um);

    // return 0;
}
////

//// Handler Graphire
static void WacomHandler_graphire(struct usbtablet *um)
{
    const struct wacom_features *features = um->features;
    UBYTE *data = um->UsbData;
    int prox;
    int rw = 0;
    uint64 buttons = 0;
    struct WacomState *state = &um->currentState;

    DebugLog(45, um, "WacomHandler_graphire: in\n");

    if (data[0] != WACOM_REPORT_PENABLED) {
        DebugLog(0, um, "WacomHandler_graphire : received unknown report #%d\n", data[0] );
        return;
    }
    
    if( um->debugLevel >= 30)
    {
        uint16 sCurIndex = 0;
        DebugLog(30, um, "Data received:\n");
        for( ;sCurIndex < um->features->pktlen; sCurIndex++)
        {
            DebugLog(30, um,  "%02x ", data[sCurIndex] );
        }
        DebugLog(30, um, "\n");
    }

    prox = data[1] & 0x80;
    if (prox || um->id[0]) {
        if (prox) {
            switch ((data[1] >> 5) & 3) {

            case 0: /* Pen */
                um->tool[0] = BTN_TOOL_PEN;
                um->id[0] = STYLUS_DEVICE_ID;
                break;

            case 1: /* Rubber */
                um->tool[0] = BTN_TOOL_RUBBER;
                um->id[0] = ERASER_DEVICE_ID;
                break;

            case 2: /* Mouse with wheel */
                SETBITS(buttons, BTN_MIDDLE, data[1] & 0x04);
                /* fall through */

            case 3: /* Mouse without wheel */
                um->tool[0] = BTN_TOOL_MOUSE;
                um->id[0] = CURSOR_DEVICE_ID;
                break;
            }
        }
        state->X = LE_INT16(*((int16 *)&data[2]));
        state->Y = LE_INT16(*((int16 *)&data[4]));
        if (um->tool[0] != BTN_TOOL_MOUSE) {
            state->Pressure = (data[6] | ((data[7] & 0x03) << 8));
            SETBITS(buttons, BTN_TOUCH, data[1] & 0x01);
            SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
            SETBITS(buttons, BTN_STYLUS2, data[1] & 0x04);
        } else {
            SETBITS(buttons, BTN_LEFT, data[1] & 0x01);
            SETBITS(buttons, BTN_RIGHT, data[1] & 0x02);
            if (features->type == WACOM_G4 ||
                    features->type == WACOM_MO) {
                state->distance[0] = data[6] & 0x3f;
                rw = (data[7] & 0x04) - (data[7] & 0x03);
            } else {
                state->distance[0] = data[7] & 0x3f;
                rw = -(signed char)data[6];
            }
            //input_report_rel(input, REL_WHEEL, rw);
        }

        if (!prox) {
            um->id[0] = 0;
        }
        //input_report_abs(input, ABS_MISC, wacom->id[0]); /* report tool id */
        state->proximity[0] = prox;
        //input_event(input, EV_MSC, MSC_SERIAL, 1);
        if (0 != rw ) {
            SendWheelEvent(um, 0, rw, TRUE);
        }
        SendMouseEvent(um, buttons);
    }

    rw = 0;
    BOOL relative = FALSE;
    /* send pad data */
    switch (features->type) {
    case WACOM_G4:
        prox = data[7] & 0xf8;
        if (prox || um->id[1]) {
            um->id[1] = PAD_DEVICE_ID;
            SETBITS(buttons, BTN_BACK, (data[7] & 0x40));
            SETBITS(buttons, BTN_FORWARD, (data[7] & 0x80));
            rw = ((data[7] & 0x18) >> 3) - ((data[7] & 0x20) >> 3);
            state->proximity[1] = prox;
            if (!prox)
                um->id[1] = 0;
            relative = TRUE;
            //input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
        }
        break;

    case WACOM_MO:
        prox = (data[7] & 0x78) || (data[8] & 0x7f);
        if (prox || um->id[1]) {
            um->id[1] = PAD_DEVICE_ID;
            SETBITS(buttons, BTN_BACK,    (data[7] & 0x08));
            SETBITS(buttons, BTN_LEFT,    (data[7] & 0x20));
            SETBITS(buttons, BTN_FORWARD, (data[7] & 0x10));
            SETBITS(buttons, BTN_RIGHT,   (data[7] & 0x40));
            rw =  (data[8] & 0x7f);
            state->proximity[1] = prox;
            if (!prox)
                um->id[1] = 0;
            relative = FALSE;
            //input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
        };
        break;
    }
    if ( rw ) {
        SendWheelEvent(um, 0, rw, relative);
    }
    SendTabletEvent(0, um, buttons);

    HandleExecuteActions(um, um->buttonAction, buttons);
}
////

//// Handler Intuos

static int wacom_intuos_pad(struct usbtablet *wacom)
{
    const struct wacom_features *features = wacom->features;
    unsigned char *data = wacom->UsbData;
    int i;
    int buttons = 0, nbuttons = features->numbered_buttons;
    int keys = 0, nkeys = 0;
    int ring1 = 0, ring2 = 0;
    int strip1 = 0, strip2 = 0;
    uint32 buttonsSet = 0;
    BOOL prox = FALSE;
    struct WacomState *state = &wacom->currentState;

    /* pad packets. Works as a second tool and is always in prox */
    if (!(data[0] == WACOM_REPORT_INTUOSPAD || data[0] == WACOM_REPORT_INTUOS5PAD ||
          data[0] == WACOM_REPORT_CINTIQPAD))
        return 0;

    if (features->type >= INTUOS4S && features->type <= INTUOS4L) {
        buttons = (data[3] << 1) | (data[2] & 0x01);
        ring1 = data[1];
    } else if (features->type == DTK) {
        buttons = data[6];
    } else if (features->type == WACOM_13HD) {
        buttons = (data[4] << 1) | (data[3] & 0x01);
    } else if (features->type == WACOM_24HD) {
        buttons = (data[8] << 8) | data[6];
        ring1 = data[1];
        ring2 = data[2];

        /*
         * Three "buttons" are available on the 24HD which are
         * physically implemented as a touchstrip. Each button
         * is approximately 3 bits wide with a 2 bit spacing.
         * The raw touchstrip bits are stored at:
         *    ((data[3] & 0x1f) << 8) | data[4])
         */
        nkeys = 3;
        keys = ((data[3] & 0x1C) ? 1<<2 : 0) |
               ((data[4] & 0xE0) ? 1<<1 : 0) |
               ((data[4] & 0x07) ? 1<<0 : 0);
    } else if (features->type == WACOM_27QHD) {
        nkeys = 3;
        keys = data[2] & 0x07;
    } else if (features->type == CINTIQ_HYBRID) {
        /*
         * Do not send hardware buttons under Android. They
         * are already sent to the system through GPIO (and
         * have different meaning).
         *
         * d-pad right  -> data[4] & 0x10
         * d-pad up     -> data[4] & 0x20
         * d-pad left   -> data[4] & 0x40
         * d-pad down   -> data[4] & 0x80
         * d-pad center -> data[3] & 0x01
         */
        buttons = (data[4] << 1) | (data[3] & 0x01);
    } else if (features->type == CINTIQ_COMPANION_2) {
        /* d-pad right  -> data[4] & 0x10
         * d-pad up     -> data[4] & 0x20
         * d-pad left   -> data[4] & 0x40
         * d-pad down   -> data[4] & 0x80
         * d-pad center -> data[3] & 0x01
         */
        buttons = ((data[2] >> 4) << 7) |
                  ((data[1] & 0x04) << 6) |
                  ((data[2] & 0x0F) << 2) |
                  (data[1] & 0x03);
    } else if (features->type >= INTUOS5S && features->type <= INTUOSPL) {
        /*
         * ExpressKeys on Intuos5/Intuos Pro have a capacitive sensor in
         * addition to the mechanical switch. Switch data is
         * stored in data[4], capacitive data in data[5].
         *
         * Touch ring mode switch (data[3]) has no capacitive sensor
         */
        buttons = (data[4] << 1) | (data[3] & 0x01);
        ring1 = data[2];
    } else {
        if (features->type == WACOM_21UX2 || features->type == WACOM_22HD) {
            buttons = (data[8] << 10) | ((data[7] & 0x01) << 9) |
                      (data[6] << 1) | (data[5] & 0x01);

            if (features->type == WACOM_22HD) {
                nkeys = 3;
                keys = data[9] & 0x07;
            }
        } else {
            buttons = ((data[6] & 0x10) << 5)  |
                      ((data[5] & 0x10) << 4)  |
                      ((data[6] & 0x0F) << 4)  |
                      (data[5] & 0x0F);
        }
        strip1 = ((data[1] & 0x1f) << 8) | data[2];
        strip2 = ((data[3] & 0x1f) << 8) | data[4];
    }

    prox = (buttons & ~(~0 << nbuttons)) | (keys & ~(~0 << nkeys)) |
           (ring1 & 0x80) | (ring2 & 0x80) | strip1 | strip2;

    wacom_report_numbered_buttons(nbuttons, buttons, &buttonsSet);

    for (i = 0; i < nkeys; i++)
        SETBITS(buttonsSet, (KEY_PROG1 + i), keys & (1 << i));

    //input_report_abs(input, ABS_RX, strip1);
    //input_report_abs(input, ABS_RY, strip2);
    if( strip1 || strip2 ) {
        DebugLog(20, wacom, "wacom_intuos_pad: strip (%ld, %ld)\n", strip2, strip1);
        SendWheelEvent(wacom, strip2, strip1, FALSE);
    }
    else {
        const int32 vWheel = (ring1 & 0x80) ? (ring1 & 0x7f) : 0;
        const int32 hWheel = (ring2 & 0x80) ? (ring2 & 0x7f) : 0;
        DebugLog(20, wacom, "wacom_intuos_pad: wheel values (%ld, %ld)\n", hWheel, vWheel);
        SendWheelEvent(wacom, hWheel, vWheel, FALSE);
    }

    state->proximity[1] = prox ? TRUE : FALSE;
    wacom->tool[1] = PAD_DEVICE_ID;

    //input_event(input, EV_MSC, MSC_SERIAL, 0xffffffff);

    SendTabletEvent(1, wacom, buttonsSet);
    HandleExecuteActions(wacom, wacom->buttonAction, buttonsSet);

    return 1;
}

//static int wacom_intuos_id_mangle(int tool_id)
//{
//    return (tool_id & ~0xFFF) << 4 | (tool_id & 0xFFF);
//}

static int wacom_intuos_get_tool_type(int tool_id)
{
    int tool_type;

    switch (tool_id) {
    case 0x812: /* Inking pen */
    case 0x801: /* Intuos3 Inking pen */
    case 0x12802: /* Intuos4/5 Inking Pen */
    case 0x012:
        tool_type = BTN_TOOL_PENCIL;
        break;

    case 0x822: /* Pen */
    case 0x842:
    case 0x852:
    case 0x823: /* Intuos3 Grip Pen */
    case 0x813: /* Intuos3 Classic Pen */
    case 0x885: /* Intuos3 Marker Pen */
    case 0x802: /* Intuos4/5 13HD/24HD General Pen */
    case 0x804: /* Intuos4/5 13HD/24HD Marker Pen */
    case 0x8e2: /* IntuosHT2 pen */
    case 0x022:
    case 0x10804: /* Intuos4/5 13HD/24HD Art Pen */
    case 0x14802: /* Intuos4/5 13HD/24HD Classic Pen */
    case 0x16802: /* Cintiq 13HD Pro Pen */
    case 0x18802: /* DTH2242 Pen */
    case 0x10802: /* Intuos4/5 13HD/24HD General Pen */
        tool_type = BTN_TOOL_PEN;
        break;

    case 0x832: /* Stroke pen */
    case 0x032:
        tool_type = BTN_TOOL_BRUSH;
        break;

    case 0x007: /* Mouse 4D and 2D */
    case 0x09c:
    case 0x094:
    case 0x017: /* Intuos3 2D Mouse */
    case 0x806: /* Intuos4 Mouse */
        tool_type = BTN_TOOL_MOUSE;
        break;

    case 0x096: /* Lens cursor */
    case 0x097: /* Intuos3 Lens cursor */
    case 0x006: /* Intuos4 Lens cursor */
        tool_type = BTN_TOOL_LENS;
        break;

    case 0x82a: /* Eraser */
    case 0x84a:
    case 0x85a:
    case 0x91a:
    case 0xd1a:
    case 0x0fa:
    case 0x82b: /* Intuos3 Grip Pen Eraser */
    case 0x81b: /* Intuos3 Classic Pen Eraser */
    case 0x91b: /* Intuos3 Airbrush Eraser */
    case 0x80c: /* Intuos4/5 13HD/24HD Marker Pen Eraser */
    case 0x80a: /* Intuos4/5 13HD/24HD General Pen Eraser */
    case 0x90a: /* Intuos4/5 13HD/24HD Airbrush Eraser */
    case 0x1480a: /* Intuos4/5 13HD/24HD Classic Pen Eraser */
    case 0x1090a: /* Intuos4/5 13HD/24HD Airbrush Eraser */
    case 0x1080c: /* Intuos4/5 13HD/24HD Art Pen Eraser */
    case 0x1680a: /* Cintiq 13HD Pro Pen Eraser */
    case 0x1880a: /* DTH2242 Eraser */
    case 0x1080a: /* Intuos4/5 13HD/24HD General Pen Eraser */
        tool_type = BTN_TOOL_RUBBER;
        break;

    case 0xd12:
    case 0x912:
    case 0x112:
    case 0x913: /* Intuos3 Airbrush */
    case 0x902: /* Intuos4/5 13HD/24HD Airbrush */
    case 0x10902: /* Intuos4/5 13HD/24HD Airbrush */
        tool_type = BTN_TOOL_AIRBRUSH;
        break;

    default: /* Unknown tool */
        tool_type = BTN_TOOL_PEN;
        break;
    }
    return tool_type;
}


static int wacom_intuos_inout(struct usbtablet *um)
{
    const struct wacom_features *features = um->features;
    unsigned char *data = um->UsbData;
    struct WacomState *state = &um->currentState;
    int idx = (features->type == INTUOS) ? (data[1] & 0x01) : 0;

    if (!(((data[1] & 0xfc) == 0xc0) ||  /* in prox */
        ((data[1] & 0xfe) == 0x20) ||    /* in range */
        ((data[1] & 0xfe) == 0x80)))     /* out prox */
        return 0;

    /* Enter report */
    if ((data[1] & 0xfc) == 0xc0) {
        /* serial number of the tool */
        /*um->serial[idx] = ((data[3] & 0x0f) << 28) +
            (data[4] << 20) + (data[5] << 12) +
            (data[6] << 4) + (data[7] >> 4);*/
            
        um->id[idx] = (data[2] << 4) | (data[3] >> 4) |
            ((data[7] & 0x0f) << 16) | ((data[8] & 0xf0) << 8);

        um->tool[idx] = wacom_intuos_get_tool_type(um->id[idx]);

        um->stylus_in_proximity = TRUE;
        return 1;
    }


    /* in Range */
    if ((data[1] & 0xfe) == 0x20) {
        if (features->type != INTUOSHT2)
            um->stylus_in_proximity = TRUE;

        /* in Range while exiting */
        if (um->reporting_data) {
            //input_report_key(input, BTN_TOUCH, 0);
            state->Pressure = 0;
            state->distance[idx] = features->distance_max;
            return 2;
        }
        return 1;
    }

    /* Exit report */
    if ((data[1] & 0xfe) == 0x80) {
        um->stylus_in_proximity = FALSE;
        um->reporting_data = FALSE;

        /* don't report exit if we don't know the ID */
        if (!um->id[idx])
            return 1;

        /*
         * Reset all states otherwise we lose the initial states
         * when in-prox next time
         */
        state->distance[idx] = 0;
        state->tiltX = 0;
        state->tiltY = 0;
        state->proximity[idx] = FALSE;
        //input_event(input, EV_MSC, MSC_SERIAL, wacom->serial[idx]);
        um->id[idx] = 0;
        //SendMouseEvent(um, 0);
        //SendTabletEvent(idx, um, 0);
        
        return 2;
    }

    /* don't report other events if we don't know the ID */
    if (!um->id[idx])
        return 1;

    return 0;
}

static inline BOOL delay_pen_events(struct usbtablet *wacom)
{
    return (wacom->touch_down && wacom->touch_arbitration)?TRUE:FALSE;
}

static inline BOOL report_touch_events(struct usbtablet *wacom)
{
    return (wacom->touch_arbitration ? !wacom->stylus_in_proximity : 1);
}

static int wacom_intuos_general(struct usbtablet *um, uint32 *pButtons, uint32 *toolIdx)
{
    const struct wacom_features *features = um->features;
    unsigned char *data = um->UsbData;
    int idx = (features->type == INTUOS) ? (data[1] & 0x01) : 0;
    unsigned char type = (data[1] >> 1) & 0x0F;
    unsigned int x, y, distance, t;
    struct WacomState *state = &um->currentState;

    if (data[0] != WACOM_REPORT_PENABLED && data[0] != WACOM_REPORT_CINTIQ &&
        data[0] != WACOM_REPORT_INTUOS_PEN)
        return 0;

    if (delay_pen_events(um))
        return 1;

    /*
     * don't report events for invalid data
     */
    /* older I4 styli don't work with new Cintiqs */
    if ((!((um->id[idx] >> 16) & 0x01) &&
            (features->type == WACOM_21UX2)) ||
        /* Only large Intuos support Lense Cursor */
        (um->tool[idx] == BTN_TOOL_LENS &&
        (features->type == INTUOS3 ||
         features->type == INTUOS3S ||
         features->type == INTUOS4 ||
         features->type == INTUOS4S ||
         features->type == INTUOS5 ||
         features->type == INTUOS5S ||
         features->type == INTUOSPM ||
         features->type == INTUOSPS)) ||
       /* Cintiq doesn't send data when RDY bit isn't set */
       (features->type == CINTIQ && !(data[1] & 0x40)))
        return 1;

    x = (*((uint16 *)&data[2]) << 1) | ((data[9] >> 1) & 1);
    y = (*((uint16 *)&data[4]) << 1) | (data[9] & 1);
    distance = data[9] >> 2;
    if (features->type < INTUOS3S) {
        x >>= 1;
        y >>= 1;
        distance >>= 1;
    }
    state->X = x;
    state->Y = y;
    state->distance[idx] = distance;

    switch (type) {
        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
            /* general pen packet */
            t = (data[6] << 3) | ((data[7] & 0xC0) >> 5) | (data[1] & 1);
            if (features->pressure_max < 2047)
                t >>= 1;
            state->Pressure = t;
            if (features->type != INTUOSHT2) {
                state->tiltX = ((data[7] << 1) & 0x7e) | (data[8] >> 7);
                state->tiltY = data[8] & 0x7f;
            }
            SETBITS(*pButtons, BTN_STYLUS, (data[1] & 2));
            SETBITS(*pButtons, BTN_STYLUS2, (data[1] & 4));
            SETBITS(*pButtons, BTN_TOUCH, (t > 10));
            break;

        case 0x0a:
            /* airbrush second packet */
            SendWheelEvent(um, 0, (data[6] << 2) | ((data[7] >> 6) & 3), FALSE);
            state->tiltX = ((data[7] << 1) & 0x7e) | (data[8] >> 7);
            state->tiltY = data[8] & 0x7f;
            break;

        case 0x05:
            /* Rotation packet */
            if (features->type >= INTUOS3S) {
                /* I3 marker pen rotation */
                t = (data[6] << 3) | ((data[7] >> 5) & 7);
                t = (data[7] & 0x20) ? ((t > 900) ? ((t-1) / 2 - 1350) :
                    ((t-1) / 2 + 450)) : (450 - t / 2) ;
                state->Z = t;
            } else {
                /* 4D mouse 2nd packet */
                t = (data[6] << 3) | ((data[7] >> 5) & 7);
                //input_report_abs(input, ABS_RZ, (data[7] & 0x20) ?
                //    ((t - 1) / 2) : -t / 2);
            }
            break;

        case 0x04:
            /* 4D mouse 1st packet */
            SETBITS(*pButtons, BTN_LEFT,   data[8] & 0x01);
            SETBITS(*pButtons, BTN_MIDDLE, data[8] & 0x02);
            SETBITS(*pButtons, BTN_RIGHT,  data[8] & 0x04);

            SETBITS(*pButtons, BTN_SIDE,   data[8] & 0x20);
            SETBITS(*pButtons, BTN_EXTRA,  data[8] & 0x10);
            t = (data[6] << 2) | ((data[7] >> 6) & 3);
            //input_report_abs(input, ABS_THROTTLE, (data[8] & 0x08) ? -t : t);
            break;

        case 0x06:
            /* I4 mouse */
            SETBITS(*pButtons, BTN_LEFT,   data[6] & 0x01);
            SETBITS(*pButtons, BTN_MIDDLE, data[6] & 0x02);
            SETBITS(*pButtons, BTN_RIGHT,  data[6] & 0x04);
            SendWheelEvent(um, 0,  ((data[7] & 0x80) >> 7)
                     - ((data[7] & 0x40) >> 6), TRUE);
            SETBITS(*pButtons, BTN_SIDE,   data[6] & 0x08);
            SETBITS(*pButtons, BTN_EXTRA,  data[6] & 0x10);

            state->tiltX = ((data[7] << 1) & 0x7e) | (data[8] >> 7);
            state->tiltY = data[8] & 0x7f;
            break;

        case 0x08:
            if (um->tool[idx] == BTN_TOOL_MOUSE) {
                /* 2D mouse packet */
                SETBITS(*pButtons, BTN_LEFT,   data[8] & 0x04);
                SETBITS(*pButtons, BTN_MIDDLE, data[8] & 0x08);
                SETBITS(*pButtons, BTN_RIGHT,  data[8] & 0x10);
                SendWheelEvent(um, 0, (data[8] & 0x01)
                         - ((data[8] & 0x02) >> 1), TRUE);

                /* I3 2D mouse side buttons */
                if (features->type >= INTUOS3S && features->type <= INTUOS3L) {
                    SETBITS(*pButtons, BTN_SIDE,   data[8] & 0x40);
                    SETBITS(*pButtons, BTN_EXTRA,  data[8] & 0x20);
                }
            }
            else if (um->tool[idx] == BTN_TOOL_LENS) {
                /* Lens cursor packets */
                SETBITS(*pButtons, BTN_LEFT,   data[8] & 0x01);
                SETBITS(*pButtons, BTN_MIDDLE, data[8] & 0x02);
                SETBITS(*pButtons, BTN_RIGHT,  data[8] & 0x04);
                SETBITS(*pButtons, BTN_SIDE,   data[8] & 0x10);
                SETBITS(*pButtons, BTN_EXTRA,  data[8] & 0x08);
            }
            break;

        case 0x07:
        case 0x09:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
            /* unhandled */
            break;
    }

    //input_report_abs(input, ABS_MISC,
    //         wacom_intuos_id_mangle(wacom->id[idx])); /* report tool id */
    //input_report_key(input, wacom->tool[idx], 1);
    //input_event(input, EV_MSC, MSC_SERIAL, wacom->serial[idx]);
    um->reporting_data = TRUE;
    return 2;
}

static void WacomHandler_intuos(struct usbtablet *wacom)
{
    UBYTE *data = wacom->UsbData;
    uint32 buttons = 0, toolIdx = 0;
    int result;

    if (data[0] != WACOM_REPORT_PENABLED &&
        data[0] != WACOM_REPORT_INTUOS_ID1 &&
        data[0] != WACOM_REPORT_INTUOS_ID2 &&
        data[0] != WACOM_REPORT_INTUOSPAD &&
        data[0] != WACOM_REPORT_INTUOS_PEN &&
        data[0] != WACOM_REPORT_CINTIQ &&
        data[0] != WACOM_REPORT_CINTIQPAD &&
        data[0] != WACOM_REPORT_INTUOS5PAD) {
        DebugLog(0, wacom, "WacomHanlder_intuos: received unknown report #%d", data[0]);
        return;
    }

    /* process pad events */
    result = wacom_intuos_pad(wacom);
    if (result)
    {
        SendTabletEvent(0, wacom, buttons);
        HandleExecuteActions(wacom, wacom->buttonAction, buttons);

        DebugLog(20, wacom, "WacomHandler_intuos: return after wacom_intuos_pad\n");
        return /*result*/;
    }

    /* process in/out prox events */
    result = wacom_intuos_inout(wacom);
    if (result)
    {
        SendTabletEvent(0, wacom, buttons);
        HandleExecuteActions(wacom, wacom->buttonAction, buttons);

        DebugLog(20, wacom, "WacomHandler_intuos: return after wacom_intuos_inout\n");
        return /*result - 1*/;
    }

    /* process general packets */
    result = wacom_intuos_general(wacom, &buttons, &toolIdx);
    if (result)
    {
        SendTabletEvent(0, wacom, buttons);
        SendMouseEvent(wacom, buttons);
        HandleExecuteActions(wacom, wacom->buttonAction, buttons);

        DebugLog(20, wacom, "WacomHandler_intuos: return after wacom_intuos_general\n");
        return /*result - 1*/;
    }

    //return;
}
////

static int wacom_numbered_button_to_key(int n)
{
    if (n < 10)
        return BTN_0 + n;
    else if (n < 16)
        return BTN_A + (n-10);
    else if (n < 18)
        return BTN_BASE + (n-16);
    else
        return -1; // 0; // ABA: BTN_0 has value 0, changed error to -1 to differentiate
}

static void wacom_report_numbered_buttons(/*struct input_dev *input_dev,*/
                int button_count, int mask, uint32* pButtons)
{
    int i;

    for (i = 0; i < button_count; i++) {
        int key = wacom_numbered_button_to_key(i);

        if (key >= 0) /* (key) */ // ABA: changed due to key emulation constraints, 0 is a possible value
        {
            SETBITS(*pButtons, key, mask & (1 << i));
        }
    }
}

static int WacomCreateSlots(struct usbtablet *um)
{
    struct wacom_features *features = um->features;
    int i;

    if (features->device_type != BTN_TOOL_FINGER)
        return 0;

    um->slots = um->IExec->AllocVecTags(    features->touch_max * sizeof(int),
                                            AVT_Type, MEMF_SHARED,
                                            AVT_ClearWithValue, 0,
                                            TAG_DONE );

    if (!um->slots)
    {
        DebugLog(0, um, "WacomCreateSlots: memory error");
        return -1;//ENOMEM;
    }

    for (i = 0; i < features->touch_max; i++)
        um->slots[i] = -1;

    return 0;
}

VOID WacomHandler( struct usbtablet *um )
{
    struct USBIOReq *req;

    DebugLog(45, um, "WacomHandler: in\n");

    req = (struct USBIOReq *)um->IExec->GetMsg( um->UsbMP );

    if ( req )
    {
        if ( req->io_Error == USBERR_NOERROR )
        {
            switch(um->features->type)
            {
                case PENPARTNER:
                    WacomHandler_penpartner(um);
                    break;

                case PL:
                    WacomHandler_pl(um);
                    break;

                case WACOM_G4:
                case GRAPHIRE:
                case WACOM_MO:
                    WacomHandler_graphire(um);
                    break;

                case PTU:
                    WacomHandler_ptu(um);
                    break;

                case DTU:
                    WacomHandler_dtu(um);
                    break;

                case DTUS:
                case DTUSX:
                case DTK2451:
                    WacomHandler_dtus(um);
                    break;

                case DTUS2:
                case DTH1152:
                    WacomHandler_dth1152(um);
                    break;

                case INTUOS:
                case INTUOS3S:
                case INTUOS3:
                case INTUOS3L:
                case INTUOS4S:
                case INTUOS4:
                case INTUOS4L:
                case CINTIQ:
                case WACOM_BEE:
                case WACOM_13HD:
                case WACOM_21UX2:
                case WACOM_22HD:
                case WACOM_24HD:
                case WACOM_27QHD:
                case DTK:
                case CINTIQ_HYBRID:
                case CINTIQ_COMPANION_2:
                    WacomHandler_intuos(um);
                    break;
            
                /* Note ABA: below tablets are not supported
                 * because of their MT nature and MT isn't
                 * supported yet by AmigaOS 4
                 */
                case WACOM_MSPRO:
                case INTUOSP2:
                case INTUOSP2S:
                case CINTIQ_16:
                    if (req->io_Actual == WACOM_PKGLEN_INTUOSP2T/* &&
                        um->UsbData[0] == WACOM_REPORT_VENDOR_DEF_TOUCH*/)
                    {
                        //sync = wacom_multitouch_generic(um);
                        DebugLog(0, um, "WacomHandler: AmigaOS does not support multitouch yet, hence tablet type %ld, '%s' is not supported\n", um->features->type, um->features->name);
                    }
                    else
                    {
                        //sync = wacom_mspro_irq(um);
                        DebugLog(0, um, "WacomHandler: unsupported tablet type %ld, '%s'\n", um->features->type, um->features->name);
                    }
                    break;

                case WACOM_24HDT:
                case WACOM_27QHDT:
                case DTH1152T:
                case DTH2452T:
                case WACOM_MSPROT:
                    //sync = wacom_multitouch_generic(um);
                    DebugLog(0, um, "WacomHandler: AmigaOS does not support multitouch yet, hence tablet type %ld, '%s' is not supported\n", um->features->type, um->features->name);
                    break;

                case INTUOS5S:
                case INTUOS5:
                case INTUOS5L:
                case INTUOSPS:
                case INTUOSPM:
                case INTUOSPL:
                    if (WACOM_PKGLEN_BBTOUCH3 == req->io_Actual)
                        WacomHandler_bpt3_touch(um);
                    else if (um->UsbData[0] == WACOM_REPORT_USB)
                    {
                        //sync = wacom_status_irq(um, len);
                        DebugLog(0, um, "WacomHandler: unsupported status tablet type %ld, '%s'\n", um->features->type, um->features->name);
                    }
                    else
                        WacomHandler_intuos(um);
                    break;

                case BAMBOO_PT:
                case INTUOSHT:
                case INTUOSHT2:
                    if (um->UsbData[0] == WACOM_REPORT_USB)
                    {
                        //sync = wacom_status_irq(wacom_wac, len);
                        DebugLog(0, um, "WacomHandler: unsupported status tablet type %ld, '%s'\n", um->features->type, um->features->name);
                    }
                    else
                        WacomHandler_bpt(um, req->io_Actual);
                    break;

                default:
                    DebugLog(0, um, "WacomHandler: unsupported tablet type %ld, '%s'\n", um->features->type, um->features->name);
                    break;
            }
        }
        else if ( req->io_Error == USBERR_STALL )
        {
            ULONG lErr = um->IUSBSys->USBEPDestall( um->USBReq, um->UsbStatusEndPoint );
            switch(lErr)
            {
                case USBERR_DETACHED:
                    um->TheEnd = TRUE;
                    return;
                case USBERR_NOERROR:
                    break;
                default:
                    DebugLog(0, um, "WacomHandler: unexpected error %ld while trying to destall endpoint\n", lErr);
                    return;
            }
        }
        else
        {
            DebugLog(20, um, "WacomHandler: USB request received with error %ld\n", req->io_Error);
        }

        DebugLog(30, um, "WacomHandler: updating the USBRequest\n");

        /* Send USB Mouse Request */
        um->UsbIOReq->io_Command    = CMD_READ;
        um->UsbIOReq->io_Data       = um->UsbData;
        um->UsbIOReq->io_Length     = um->features->pktlen;
        um->UsbIOReq->io_Flags      = 0;
        um->UsbIOReq->io_Actual     = 0;
        um->UsbIOReq->io_EndPoint   = um->UsbStatusEndPoint;

        if ( um->UsbIOReq->io_Length > LE_WORD( um->UsbEndPointDscrIn->ed_MaxPacketSize ))
        {
            um->UsbIOReq->io_Length = LE_WORD( um->UsbEndPointDscrIn->ed_MaxPacketSize );
        }

        DebugLog(30, um, "WacomHandler: about to resend the USB Request\n");
        um->IExec->SendIO( (struct IORequest *)um->UsbIOReq );
    }

    DebugLog(45, um, "WacomHandler: out\n");
}
///

