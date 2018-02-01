/*

************************************************************
**
** Created by: CodeBench 0.23 (17.09.2011)
**
** Project: WacomTablet.usbfd
**
** File: Tablet handler functions
**
** Date: 20-04-2012 14:54:24
**
** Copyright 2012 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
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

//// Other useful macros
#define LE_INT16(a) ((int16)((((uint16)a)>>8)|(((uint16)a)<<8)))
///

static void WacomSetupCintiq(struct usbtablet *um)
{
	//input_set_capability(input_dev, EV_MSC, MSC_SERIAL);

	SETBITS(um->buttonCapabilities, BTN_LEFT, 1);
	SETBITS(um->buttonCapabilities, BTN_RIGHT, 1);
	SETBITS(um->buttonCapabilities, BTN_MIDDLE, 1);
	SETBITS(um->buttonCapabilities, BTN_SIDE, 1);
	SETBITS(um->buttonCapabilities, BTN_EXTRA, 1);

	SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
	SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
	SETBITS(um->toolCapabilities, BTN_TOOL_BRUSH, 1);
	SETBITS(um->toolCapabilities, BTN_TOOL_PENCIL, 1);
	SETBITS(um->toolCapabilities, BTN_TOOL_AIRBRUSH, 1);
	SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
	SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);

	//input_set_abs_params(input_dev, ABS_DISTANCE,
	//     0, wacom_wac->features.distance_max, 0, 0);
	//input_set_abs_params(input_dev, ABS_WHEEL, 0, 1023, 0, 0);
	um->minWheel = 0;
	um->maxWheel = 1023;
	//input_set_abs_params(input_dev, ABS_TILT_X, 0, 127, 0, 0);
	//input_set_abs_params(input_dev, ABS_TILT_Y, 0, 127, 0, 0);
}

static void WacomSetupIntuos(struct usbtablet *um)
{
	//input_set_capability(input_dev, EV_REL, REL_WHEEL);

	WacomSetupCintiq(um);

	SETBITS(um->toolCapabilities, BTN_TOOL_MOUSE, 1);
	SETBITS(um->toolCapabilities, BTN_TOOL_LENS, 1);
	//input_set_abs_params(input_dev, ABS_RZ, -900, 899, 0, 0);
	//input_set_abs_params(input_dev, ABS_THROTTLE, -1023, 1023, 0, 0);
	um->minWheel = -1023;
	um->maxWheel = 1023;
}

//// Setup tablet capabilities
void WacomSetupCapabilities(struct usbtablet* um)
{
	const struct wacom_features *features = um->features;
	int i;

	um->toolCapabilities = 0L;
	um->buttonCapabilities = 0L;

	SETBITS(um->buttonCapabilities, BTN_TOUCH, 1);

//	input_set_abs_params(input_dev, ABS_X, 0, features->x_max, 4, 0);
//	input_set_abs_params(input_dev, ABS_Y, 0, features->y_max, 4, 0);
//	input_set_abs_params(input_dev, ABS_PRESSURE, 0, features->pressure_max, 0, 0);

//	__set_bit(ABS_MISC, input_dev->absbit);

	switch (um->features->type) {
	case BAMBOO_PT:
	if (features->device_type == BTN_TOOL_TRIPLETAP) {
		SETBITS(um->toolCapabilities, BTN_TOOL_DOUBLETAP, 1);
		SETBITS(um->toolCapabilities, BTN_TOOL_TRIPLETAP, 1);
		SETBITS(um->buttonCapabilities, BTN_0, 1);
		SETBITS(um->buttonCapabilities, BTN_1, 1);
		SETBITS(um->buttonCapabilities, BTN_2, 1);
		SETBITS(um->buttonCapabilities, BTN_3, 1);
		SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);
//		input_set_capability(input_dev, EV_MSC, MSC_SERIAL);
	}
	if (features->device_type == BTN_TOOL_PEN) {
		SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
		SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
		SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
		SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);
	}
	break;
	case WACOM_MO:
		SETBITS(um->buttonCapabilities, BTN_1, 1);
		SETBITS(um->buttonCapabilities, BTN_5, 1);

		um->minWheel = 0;
		um->maxWheel = 71;
		/* fall through */

	case WACOM_G4:
		//input_set_capability(input_dev, EV_MSC, MSC_SERIAL);

		SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);
		SETBITS(um->buttonCapabilities, BTN_0, 1);
		SETBITS(um->buttonCapabilities, BTN_4, 1);
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
	break;

	case WACOM_24HD:
		SETBITS(um->buttonCapabilities, BTN_A, 1);
		SETBITS(um->buttonCapabilities, BTN_B, 1);
		SETBITS(um->buttonCapabilities, BTN_C, 1);
		SETBITS(um->buttonCapabilities, BTN_X, 1);
		SETBITS(um->buttonCapabilities, BTN_Y, 1);
		SETBITS(um->buttonCapabilities, BTN_Z, 1);

		for (i = 0; i < 10; i++)
			SETBITS(um->buttonCapabilities, (BTN_0 + i), 1);

		SETBITS(um->buttonCapabilities, BTN_BASE, 1);
		SETBITS(um->buttonCapabilities, BTN_BASE2, 1);
		SETBITS(um->buttonCapabilities, BTN_BASE3, 1);
		SETBITS(um->buttonCapabilities, BTN_TOOL_FINGER, 1);

		//input_set_abs_params(input_dev, ABS_Z, -900, 899, 0, 0);
		um->minThrottle = 0;
		um->maxThrottle = 71;
		WacomSetupCintiq(um);
	break;

	case WACOM_21UX2:
		SETBITS(um->buttonCapabilities, BTN_A, 1);
		SETBITS(um->buttonCapabilities, BTN_B, 1);
		SETBITS(um->buttonCapabilities, BTN_C, 1);
		SETBITS(um->buttonCapabilities, BTN_X, 1);
		SETBITS(um->buttonCapabilities, BTN_Y, 1);
		SETBITS(um->buttonCapabilities, BTN_Z, 1);
		SETBITS(um->buttonCapabilities, BTN_BASE, 1);
		SETBITS(um->buttonCapabilities, BTN_BASE2, 1);
	/* fall through */

	case WACOM_BEE:
		SETBITS(um->buttonCapabilities, BTN_8, 1);
		SETBITS(um->buttonCapabilities, BTN_9, 1);
	/* fall through */

	case CINTIQ:
		for (i = 0; i < 8; i++)
			SETBITS(um->buttonCapabilities, (BTN_0 + i), 1);
		SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);

		//input_set_abs_params(input_dev, ABS_RX, 0, 4096, 0, 0);
		//input_set_abs_params(input_dev, ABS_RY, 0, 4096, 0, 0);
		//input_set_abs_params(input_dev, ABS_Z, -900, 899, 0, 0);
		WacomSetupCintiq(um);
	break;

	case INTUOS3:
	case INTUOS3L:
		SETBITS(um->buttonCapabilities, BTN_4, 1);
		SETBITS(um->buttonCapabilities, BTN_5, 1);
		SETBITS(um->buttonCapabilities, BTN_6, 1);
		SETBITS(um->buttonCapabilities, BTN_7, 1);

		//input_set_abs_params(input_dev, ABS_RY, 0, 4096, 0, 0);
	/* fall through */

	case INTUOS3S:
		SETBITS(um->buttonCapabilities, BTN_0, 1);
		SETBITS(um->buttonCapabilities, BTN_1, 1);
		SETBITS(um->buttonCapabilities, BTN_2, 1);
		SETBITS(um->buttonCapabilities, BTN_3, 1);

		SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);

		//input_set_abs_params(input_dev, ABS_RX, 0, 4096, 0, 0);
		//input_set_abs_params(input_dev, ABS_Z, -900, 899, 0, 0);
	/* fall through */

	case INTUOS:
		WacomSetupIntuos(um);
	break;

	case INTUOS4:
	case INTUOS4L:
		SETBITS(um->buttonCapabilities, BTN_7, 1);
		SETBITS(um->buttonCapabilities, BTN_8, 1);
	/* fall through */

	case INTUOS4S:
		for (i = 0; i < 7; i++)
			SETBITS(um->buttonCapabilities, (BTN_0 + i), 1);
		SETBITS(um->toolCapabilities, BTN_TOOL_FINGER, 1);

		//input_set_abs_params(input_dev, ABS_Z, -900, 899, 0, 0);
		WacomSetupIntuos(um);
	break;

	case TABLETPC2FG:
	if (features->device_type == BTN_TOOL_TRIPLETAP) {
		SETBITS(um->toolCapabilities, BTN_TOOL_TRIPLETAP, 1);
		//input_set_capability(input_dev, EV_MSC, MSC_SERIAL);
	}
	/* fall through */

	case TABLETPC:
	if (features->device_type == BTN_TOOL_DOUBLETAP ||
	    features->device_type == BTN_TOOL_TRIPLETAP) {
		//input_set_abs_params(input_dev, ABS_RX, 0, features->x_phy, 0, 0);
		//input_set_abs_params(input_dev, ABS_RY, 0, features->y_phy, 0, 0);
		SETBITS(um->toolCapabilities, BTN_TOOL_DOUBLETAP, 1);
	}

		if (features->device_type != BTN_TOOL_PEN)
		break;  /* no need to process stylus stuff */

	/* fall through */

	case PL:
	case PTU:
	case DTU:
		SETBITS(um->toolCapabilities, BTN_TOOL_PEN, 1);
		SETBITS(um->buttonCapabilities, BTN_STYLUS, 1);
		SETBITS(um->buttonCapabilities, BTN_STYLUS2, 1);
		/* fall through */

	case PENPARTNER:
		SETBITS(um->toolCapabilities, BTN_TOOL_RUBBER, 1);
	break;
	}
	
	DebugLog(10, um, "WacomSetupCapabilities: buttons=%08x tools=%08x\n", um->buttonCapabilities, um->toolCapabilities);	
}

////

//// Handler PenPartner
static void WacomHandler_penpartner(struct usbtablet *um)
{
	unsigned char *data = um->UsbData;
	uint32 buttons = 0;

	switch (data[0]) {
	case 1:
		if (data[5] & 0x80) {
			um->tool[0] = (data[5] & 0x20) ? BTN_TOOL_RUBBER : BTN_TOOL_PEN;
			um->id[0] = (data[5] & 0x20) ? ERASER_DEVICE_ID : STYLUS_DEVICE_ID;
			um->proximity[0] = 1;
			um->X = LE_INT16(*((int16 *)&data[1]));
			um->Y = LE_INT16(*((int16 *)&data[3]));
			um->Pressure = (signed char)data[6] + 127;
			SETBITS(buttons, BTN_TOUCH, ((signed char)data[6] > -127));
			SETBITS(buttons, BTN_STYLUS, (data[5] & 0x40));
		} else {
			um->proximity[0] = 0;
			um->Pressure = 0;
			SETBITS(buttons, BTN_TOUCH, 0);
		}
		break;

	case 2:
	    um->tool[0] = BTN_TOOL_PEN;
		um->id[0] = STYLUS_DEVICE_ID;
		um->proximity[0] = 1;
		um->X = LE_INT16(*((int16 *)&data[1]));
		um->Y = LE_INT16(*((int16 *)&data[3]));
		um->Pressure = (signed char)data[6] + 127;
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

//// Handler DTU
static void WacomHandler_dtu(struct usbtablet *um)
{
	const struct wacom_features *features = um->features;
	UBYTE *data = um->UsbData;
	int prox = data[1] & 0x20, pressure;
	uint32 buttons = 0;

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
	pressure = ((data[7] & 0x01) << 8) | data[6];
	if (pressure < 0)
		pressure = features->pressure_max + pressure + 1;
	um->Pressure = pressure;
	SETBITS(buttons, BTN_TOUCH, data[1] & 0x05);
	if (!prox) { /* out-prox */
		um->id[0] = 0;
		um->X = 0;
		um->Y = 0;
	} else {
		um->X = LE_INT16(*((int16 *)&data[2]));
		um->Y = LE_INT16(*((int16 *)&data[4]));
	}
	um->proximity[0] = prox;
	//input_report_abs(input, ABS_MISC, wacom->id[0]);
	SendMouseEvent(um, buttons);
	SendTabletEvent(0, um, buttons);
}
////

//// Handler PL
static void WacomHandler_pl(struct usbtablet *um)
{
	const struct wacom_features *features = um->features;
	unsigned char *data = um->UsbData;
	int prox, pressure;
	uint32 buttons = 0;

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
		if (!um->proximity[0]) {
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
				um->proximity[1] = 0;
				um->tool[1] = BTN_TOOL_PEN;
 				SendTabletEvent(1, um, buttons);
				return;
			}
		}
		if (um->tool[1] != BTN_TOOL_RUBBER) {
			/* Unknown tool selected default to pen tool */
			um->tool[1] = BTN_TOOL_PEN;
			um->id[0] = STYLUS_DEVICE_ID;
		}
		um->proximity[1] = prox; /* report in proximity for tool */
		um->X = data[3] | (data[2] << 7) | ((data[1] & 0x03) << 14);
		um->Y = data[6] | (data[5] << 7) | ((data[4] & 0x03) << 14);
		um->Pressure = pressure;

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
		um->X = 0;
		um->Y = 0;
		um->proximity[1] = prox;
	}

	um->proximity[0] = prox; /* Save proximity state */

	SendMouseEvent(um, buttons);
	SendTabletEvent(1, um, buttons);
}
////

//// Handler PTU
static void WacomHandler_ptu(struct usbtablet *um)
{
	unsigned char *data = um->UsbData;
	uint32 buttons = 0;

	if (data[0] != WACOM_REPORT_PENABLED) {
   		DebugLog(20, um, "WacomHandler_ptu_irq: received unknown report #%d\n", data[0]);
		return;
	}

	if (data[1] & 0x04) {
		um->tool[0] = BTN_TOOL_RUBBER;
		um->proximity[0] = data[1] & 0x20;
		SETBITS(buttons, BTN_TOUCH, data[1] & 0x08);
		um->id[0] = ERASER_DEVICE_ID;
	} else {
		um->tool[0] = BTN_TOOL_PEN;
		um->proximity[0] = data[1] & 0x20;
		SETBITS(buttons, BTN_TOUCH, data[1] & 0x01);
		um->id[0] = STYLUS_DEVICE_ID;
	}
	//input_report_abs(input, ABS_MISC, wacom->id[0]); /* report tool id */
	um->X = LE_INT16(*((int16 *)&data[2]));
	um->Y = LE_INT16(*((int16 *)&data[4]));
	um->Pressure = LE_INT16(*((int16 *)&data[6]));
	SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
	SETBITS(buttons, BTN_STYLUS2, data[1] & 0x10);
	
	SendMouseEvent(um, buttons);
	SendTabletEvent(0, um, buttons);
}
////

//// Handler Bamboo pen and touch
static void wacom_bpt_finger_in(struct usbtablet *um, UBYTE *data, int idx, uint32 * pButtons)
{
	int x = 0, y = 0;
	int finger = idx + 1;

	x = *((int16 *)&data[3 + (idx * 9)]) & 0x7ff;
	y = *((int16 *)&data[5 + (idx * 9)]) & 0x7ff;

	if (um->last_finger != finger) {
		if (x == um->X)
			um->X = x+1;

		if (y ==  um->Y)
			um->Y = y+1;
	}

	um->proximity[finger] = TRUE;

	if (!idx) {
		SETBITS(*pButtons, BTN_TOUCH, 1);
	}
	//input_event(input, EV_MSC, MSC_SERIAL, finger);
	SendMouseEvent(um, *pButtons);

	um->last_finger = finger;
}

static void wacom_bpt_touch_out(struct usbtablet *um, int idx, uint32 * pButtons)
{
	int finger = idx + 1;

	um->Pressure = 0;
	um->proximity[finger] = FALSE;

	if (!idx) {
		SETBITS(*pButtons, BTN_TOUCH, 0);
	}
	//input_event(input, EV_MSC, MSC_SERIAL, finger);
	SendTabletEvent(finger, um, *pButtons);
}

static void wacom_bpt_touch_in(struct usbtablet *um, uint32 * pButtons)
{
	UBYTE *data = um->UsbData;

	/* First finger down */
	if (data[3] & 0x80) {
		um->tool[1] = BTN_TOOL_DOUBLETAP;
		um->id[0] = TOUCH_DEVICE_ID; 
		wacom_bpt_finger_in(um, data, 0, pButtons);
	} else if (um->id[2] & 0x01)
		wacom_bpt_touch_out(um, 0, pButtons);

	/* Second finger down */
	if (data[12] & 0x80) {
		um->tool[2] = BTN_TOOL_TRIPLETAP;
		wacom_bpt_finger_in(um, data, 1, pButtons);
	} else if (um->id[2] & 0x02)
		wacom_bpt_touch_out(um, 1, pButtons);
}

static void WacomHandler_bpt(struct usbtablet *um, size_t len)
{
	UBYTE *data = um->UsbData;
	int prox = 0;
	uint32 buttons = 0;

	if (data[0] != WACOM_REPORT_PENABLED) {
		DebugLog(0, um, "wacom_bpt_irq: received unknown report #%d", data[0]);
		return;
	}

	/* Touch packet */
	if (len == WACOM_PKGLEN_BBTOUCH) {

		/* Send pad data if there are any 
		 * don't repeat all zeros
		 */
		prox = data[1] & 0x0f;
		if (prox || um->id[1]) {
			if (!um->id[1]) /* in-prox */
				um->id[1] = PAD_DEVICE_ID;

			if (!prox) /* out-prox */
				um->id[1] = 0;

			SETBITS(buttons, BTN_0, data[1] & 0x1);
			SETBITS(buttons, BTN_1, data[1] & 0x2);
			SETBITS(buttons, BTN_2, data[1] & 0x4);
			SETBITS(buttons, BTN_3, data[1] & 0x8);
			um->tool[1]=BTN_TOOL_FINGER;
			um->proximity[1]=prox;
			SendTabletEvent(1, um, buttons);
		}

		if (um->proximity[0]) {
			if (um->id[2] & 0x01)
				wacom_bpt_touch_out(um, 0, &buttons);

			if (um->id[2] & 0x02)
				wacom_bpt_touch_out(um, 1, &buttons);
			um->id[2] = 0;
			return;
		}

		prox = (data[17] & 0x30 >> 4);
		if (prox) {
			/* initialize last touched finger */
			if (!um->id[1])
				um->last_finger = 1;

			wacom_bpt_touch_in(um, &buttons);
		} else {
			if (um->id[2] & 0x01)
				wacom_bpt_touch_out(um, 0, &buttons);

			if (um->id[2] & 0x02)
				wacom_bpt_touch_out(um, 1, &buttons);

			um->id[0] = 0;
		}
		um->id[2] = (((data[3] & 0x80) >> 7) & 0x1) |
	    		(((data[12] & 0x80) >> 6) & 0x2);

	} else if (len == WACOM_PKGLEN_BBFUN) { /* Penabled */
		prox = (data[1] & 0x10) && (data[1] & 0x20);

		if (!um->proximity[0]) { /* in-prox */
			if (data[1] & 0x08) {
				um->tool[0] = BTN_TOOL_RUBBER;
				um->id[0] = ERASER_DEVICE_ID;
			} else {
				um->tool[0] = BTN_TOOL_PEN;
				um->id[0] = STYLUS_DEVICE_ID;
			}
			um->proximity[0] = TRUE;
		}
		um->Pressure = LE_INT16(*((int16 *)&data[6]));
		um->distance[0] = data[8];
		SETBITS(buttons, BTN_TOUCH, data[1] & 0x01);
		SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
		SETBITS(buttons, BTN_STYLUS2, data[1] & 0x04);
		if (!prox) {
			um->id[0] = 0;
			um->proximity[0] = FALSE;
		} else {
			um->X = LE_INT16(*((int16 *)&data[2]));
			um->Y = LE_INT16(*((int16 *)&data[4]));
		}
		um->proximity[0] = prox;
		//input_report_abs(input, ABS_MISC, wacom->id[0]);
		SendTabletEvent(0, um, buttons);
	}
}
////

//// Handler Graphire
static void WacomHandler_graphire(struct usbtablet *um)
{
	const struct wacom_features *features = um->features;
	UBYTE *data = um->UsbData;
	int prox;
	int rw = 0;
	uint32 buttons = 0;

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

			case 0:	/* Pen */
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
		if (um->tool[0] != BTN_TOOL_MOUSE) {
			um->Pressure = (data[6] | ((data[7] & 0x01) << 8));
			SETBITS(buttons, BTN_TOUCH, data[1] & 0x01);
			SETBITS(buttons, BTN_STYLUS, data[1] & 0x02);
			SETBITS(buttons, BTN_STYLUS2, data[1] & 0x04);
		} else {
			SETBITS(buttons, BTN_LEFT, data[1] & 0x01);
			SETBITS(buttons, BTN_RIGHT, data[1] & 0x02);
			if (features->type == WACOM_G4 ||
					features->type == WACOM_MO) {
				um->distance[0] = data[6] & 0x3f;
				rw = (data[7] & 0x04) - (data[7] & 0x03);
			} else {
				um->distance[0] = data[7] & 0x3f;
				rw = -(signed char)data[6];
			}
		}

		if (!prox) {
			um->id[0] = 0;
			um->X = 0;
			um->Y = 0;
		} else {
			um->X = LE_INT16(*((int16 *)&data[2]));
			um->Y = LE_INT16(*((int16 *)&data[4]));
		}
		um->proximity[0] = prox;
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
			SETBITS(buttons, BTN_0, (data[7] & 0x40));
			SETBITS(buttons, BTN_4, (data[7] & 0x80));
			rw = ((data[7] & 0x18) >> 3) - ((data[7] & 0x20) >> 3);
			um->tool[1] = BTN_TOOL_FINGER;
			um->proximity[1] = prox;
			relative = TRUE;
			//input_event(input, EV_MSC, MSC_SERIAL, 0xf0);
		}
		break;

	case WACOM_MO:
		prox = (data[7] & 0x78) || (data[8] & 0x7f);
		if (prox || um->id[1]) {
			um->id[1] = PAD_DEVICE_ID;
			SETBITS(buttons, BTN_0, (data[7] & 0x08));
			SETBITS(buttons, BTN_1, (data[7] & 0x20));
			SETBITS(buttons, BTN_4, (data[7] & 0x10));
			SETBITS(buttons, BTN_5, (data[7] & 0x40));
			rw =  (data[8] & 0x7f);
			um->tool[1] = BTN_TOOL_FINGER;
			um->proximity[1] = prox;
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
}
////

//// Handler Intuos

static int wacom_intuos_inout(struct usbtablet *um)
{
	const struct wacom_features *features = um->features;
	unsigned char *data = um->UsbData;
	int idx = 0;

	/* tool number */
	if (features->type == INTUOS)
		idx = data[1] & 0x01;

	/* Enter report */
	if ((data[1] & 0xfc) == 0xc0) {
		/* serial number of the tool */
		/*um->serial[idx] = ((data[3] & 0x0f) << 28) +
			(data[4] << 20) + (data[5] << 12) +
			(data[6] << 4) + (data[7] >> 4);*/

		um->id[idx] = (data[2] << 4) | (data[3] >> 4) |
			((data[7] & 0x0f) << 20) | ((data[8] & 0xf0) << 12);

		switch (um->id[idx] & 0xfffff) {
		case 0x812: /* Inking pen */
		case 0x801: /* Intuos3 Inking pen */
		case 0x20802: /* Intuos4 Inking Pen */
		case 0x012:
			um->tool[idx] = BTN_TOOL_PENCIL;
			break;

		case 0x822: /* Pen */
		case 0x842:
		case 0x852:
		case 0x823: /* Intuos3 Grip Pen */
		case 0x813: /* Intuos3 Classic Pen */
		case 0x885: /* Intuos3 Marker Pen */
		case 0x802: /* Intuos General Pen */
		case 0x40802: /* Intuos4 Classic Pen */
		case 0x804: /* Intuos4 Marker Pen */
		case 0x022:
			um->tool[idx] = BTN_TOOL_PEN;
			break;

		case 0x832: /* Stroke pen */
		case 0x032:
			um->tool[idx] = BTN_TOOL_BRUSH;
			break;

		case 0x007: /* Mouse 4D and 2D */
		case 0x09c:
		case 0x094:
		case 0x017: /* Intuos3 2D Mouse */
		case 0x806: /* Intuos4 Mouse */
			um->tool[idx] = BTN_TOOL_MOUSE;
			break;

		case 0x096: /* Lens cursor */
		case 0x097: /* Intuos3 Lens cursor */
		case 0x006: /* Intuos4 Lens cursor */
			um->tool[idx] = BTN_TOOL_LENS;
			break;

		case 0x82a: /* Eraser */
		case 0x85a:
		case 0x91a:
		case 0xd1a:
		case 0x0fa:
		case 0x82b: /* Intuos3 Grip Pen Eraser */
		case 0x81b: /* Intuos3 Classic Pen Eraser */
		case 0x91b: /* Intuos3 Airbrush Eraser */
		case 0x80c: /* Intuos4 Marker Pen Eraser */
		case 0x80a: /* Intuos4 General Pen Eraser */
		case 0x90a: /* Intuos4 Airbrush Eraser */
		case 0x4080a: /* Intuos4 Classic Pen Eraser */
			um->tool[idx] = BTN_TOOL_RUBBER;
			break;

		case 0xd12:
		case 0x912:
		case 0x112:
		case 0x913: /* Intuos3 Airbrush */
		case 0x902: /* Intuos4 Airbrush */
			um->tool[idx] = BTN_TOOL_AIRBRUSH;
			break;

		default: /* Unknown tool */
			um->tool[idx] = BTN_TOOL_PEN;
			break;
		}
		return 1;
	}

	/* older I4 styli don't work with new Cintiqs */
	if (!((um->id[idx] >> 20) & 0x01) &&
			(features->type == WACOM_21UX2))
		return 1;

	/* Exit report */
	if ((data[1] & 0xfe) == 0x80) {
		/*
		 * Reset all states otherwise we lose the initial states
		 * when in-prox next time
		 */
		um->distance[idx] = 0;
		um->tiltX = 0;
		um->tiltY = 0;
		um->proximity[idx] = FALSE;
		//input_event(input, EV_MSC, MSC_SERIAL, wacom->serial[idx]);
		um->id[idx] = 0;
		SendMouseEvent(um, 0);
		SendTabletEvent(idx, um, 0);
		
		return 2;
	}
	return 0;
}

static void wacom_intuos_general(struct usbtablet *um, uint32 *pButtons)
{
	const struct wacom_features *features = um->features;
	unsigned char *data = um->UsbData;
	unsigned int t;

	/* general pen packet */
	if ((data[1] & 0xb8) == 0xa0) {
		t = (data[6] << 2) | ((data[7] >> 6) & 3);
		if (features->type >= INTUOS4S && features->type <= WACOM_21UX2)
			t = (t << 1) | (data[1] & 1);
		um->Pressure = t;
		um->tiltX = ((data[7] << 1) & 0x7e) | (data[8] >> 7);
		um->tiltY = data[8] & 0x7f;
		SETBITS(*pButtons, BTN_STYLUS, (data[1] & 2));
		SETBITS(*pButtons, BTN_STYLUS2, (data[1] & 4));
		SETBITS(*pButtons, BTN_TOUCH, (t > 10));
	}

	/* airbrush second packet */
	if ((data[1] & 0xbc) == 0xb4) {
	    SendWheelEvent(um, 0, (data[6] << 2) | ((data[7] >> 6) & 3), FALSE);
		um->tiltX = ((data[7] << 1) & 0x7e) | (data[8] >> 7);
		um->tiltY = data[8] & 0x7f;
	}
}

static void WacomHandler_intuos(struct usbtablet *um)
{
	const struct wacom_features *features = um->features;
	UBYTE *data = um->UsbData;
	unsigned int t;
	int idx = 0;
	uint32 buttons = 0;

	if (data[0] != WACOM_REPORT_PENABLED && data[0] != WACOM_REPORT_INTUOSREAD
		&& data[0] != WACOM_REPORT_INTUOSWRITE && data[0] != WACOM_REPORT_INTUOSPAD) {
		DebugLog(0, um, "WacomHanlder_intuos: received unknown report #%d", data[0]);
		return;
	}

	/* tool number */
	if (features->type == INTUOS)
		idx = data[1] & 0x01;

	/* pad packets. Works as a second tool and is always in prox */
	if (data[0] == WACOM_REPORT_INTUOSPAD) {
		/* initiate the pad as a device */
		if (um->tool[1] != BTN_TOOL_FINGER)
			um->tool[1] = BTN_TOOL_FINGER;

		if (features->type >= INTUOS4S && features->type <= INTUOS4L) {
			SETBITS(buttons, BTN_0, (data[2] & 0x01));
			SETBITS(buttons, BTN_1, (data[3] & 0x01));
			SETBITS(buttons, BTN_2, (data[3] & 0x02));
			SETBITS(buttons, BTN_3, (data[3] & 0x04));
			SETBITS(buttons, BTN_4, (data[3] & 0x08));
			SETBITS(buttons, BTN_5, (data[3] & 0x10));
			SETBITS(buttons, BTN_6, (data[3] & 0x20));
			if (data[1] & 0x80) {
				SendWheelEvent(um, 0, (data[1] & 0x7f), FALSE);
			} else {
				/* Out of proximity, clear wheel value. */
				SendWheelEvent(um, 0, 0, FALSE);
			}
			if (features->type != INTUOS4S) {
				SETBITS(buttons, BTN_7, (data[3] & 0x40));
				SETBITS(buttons, BTN_8, (data[3] & 0x80));
			}
			if (data[1] | (data[2] & 0x01) | data[3]) {
			    um->proximity[1] = TRUE;
			    um->id[1] = PAD_DEVICE_ID;
			} else {
			    um->proximity[1] = FALSE;
				um->id[1] = 0;
			}
		} else if (features->type == WACOM_24HD) {
			SETBITS(buttons, BTN_0, (data[6] & 0x01));
			SETBITS(buttons, BTN_1, (data[6] & 0x02));
			SETBITS(buttons, BTN_2, (data[6] & 0x04));
			SETBITS(buttons, BTN_3, (data[6] & 0x08));
			SETBITS(buttons, BTN_4, (data[6] & 0x10));
			SETBITS(buttons, BTN_5, (data[6] & 0x20));
			SETBITS(buttons, BTN_6, (data[6] & 0x40));
			SETBITS(buttons, BTN_7, (data[6] & 0x80));
			SETBITS(buttons, BTN_8, (data[8] & 0x01));
			SETBITS(buttons, BTN_9, (data[8] & 0x02));
			SETBITS(buttons, BTN_A, (data[8] & 0x04));
			SETBITS(buttons, BTN_B, (data[8] & 0x08));
			SETBITS(buttons, BTN_C, (data[8] & 0x10));
			SETBITS(buttons, BTN_X, (data[8] & 0x20));
			SETBITS(buttons, BTN_Y, (data[8] & 0x40));
			SETBITS(buttons, BTN_Z, (data[8] & 0x80));

			/*
			 * Three "buttons" are available on the 24HD which are
			 * physically implemented as a touchstrip. Each button
			 * is approximately 3 bits wide with a 2 bit spacing.
			 * The raw touchstrip bits are stored at:
			 *    ((data[3] & 0x1f) << 8) | data[4])
			 */
			SETBITS(buttons, BTN_BASE,  data[4] & 0x07);
			SETBITS(buttons, BTN_BASE2, data[4] & 0xE0);
			SETBITS(buttons, BTN_BASE3, data[3] & 0x1C);

			UBYTE vWheel, hWheel;
			if (data[1] & 0x80) {
				vWheel = (data[1] & 0x7f);
			} else {
				/* Out of proximity, clear wheel value. */
				vWheel = 0;
			}

			if (data[2] & 0x80) {
				hWheel = (data[2] & 0x7f);
			} else {
				/* Out of proximity, clear second wheel value. */
				hWheel = 0;
			}
			SendWheelEvent(um, hWheel, vWheel, buttons);

			if (data[1] | data[2] | (data[3] & 0x1f) | data[4] | data[6] | data[8]) {
				um->proximity[1] = TRUE;
				um->id[1] = PAD_DEVICE_ID;
			} else {
				um->proximity[1] = FALSE;
				um->id[1] = 0;
			}
		} else {
			if (features->type == WACOM_21UX2) {
				SETBITS(buttons, BTN_0, (data[5] & 0x01));
				SETBITS(buttons, BTN_1, (data[6] & 0x01));
				SETBITS(buttons, BTN_2, (data[6] & 0x02));
				SETBITS(buttons, BTN_3, (data[6] & 0x04));
				SETBITS(buttons, BTN_4, (data[6] & 0x08));
				SETBITS(buttons, BTN_5, (data[6] & 0x10));
				SETBITS(buttons, BTN_6, (data[6] & 0x20));
				SETBITS(buttons, BTN_7, (data[6] & 0x40));
				SETBITS(buttons, BTN_8, (data[6] & 0x80));
				SETBITS(buttons, BTN_9, (data[7] & 0x01));
				SETBITS(buttons, BTN_A, (data[8] & 0x01));
				SETBITS(buttons, BTN_B, (data[8] & 0x02));
				SETBITS(buttons, BTN_C, (data[8] & 0x04));
				SETBITS(buttons, BTN_X, (data[8] & 0x08));
				SETBITS(buttons, BTN_Y, (data[8] & 0x10));
				SETBITS(buttons, BTN_Z, (data[8] & 0x20));
				SETBITS(buttons, BTN_BASE, (data[8] & 0x40));
				SETBITS(buttons, BTN_BASE2, (data[8] & 0x80));
			} else {
				SETBITS(buttons, BTN_0, (data[5] & 0x01));
				SETBITS(buttons, BTN_1, (data[5] & 0x02));
				SETBITS(buttons, BTN_2, (data[5] & 0x04));
				SETBITS(buttons, BTN_3, (data[5] & 0x08));
				SETBITS(buttons, BTN_4, (data[6] & 0x01));
				SETBITS(buttons, BTN_5, (data[6] & 0x02));
				SETBITS(buttons, BTN_6, (data[6] & 0x04));
				SETBITS(buttons, BTN_7, (data[6] & 0x08));
				SETBITS(buttons, BTN_8, (data[5] & 0x10));
				SETBITS(buttons, BTN_9, (data[6] & 0x10));
			}

			// TODO: support other axis reporting
			//um->RX = (((data[1] & 0x1f) << 8) | data[2]);
			//um->RY = (((data[3] & 0x1f) << 8) | data[4]);
			if ((data[5] & 0x1f) | data[6] | (data[1] & 0x1f) | data[2] |
				(data[3] & 0x1f) | data[4] | data[8] | (data[7] & 0x01)) {
				um->proximity[1] = TRUE;
				um->tool[1] = PAD_DEVICE_ID;
			} else {
				um->proximity[1] = FALSE;
				um->tool[1] = 0;
			}
		}
		//input_event(input, EV_MSC, MSC_SERIAL, 0xffffffff);
		SendMouseEvent(um, buttons);
		SendTabletEvent(1, um, buttons);
        return;
	}

	/* process in/out prox events */
	if (wacom_intuos_inout(um)) return;

	/* don't proceed if we don't know the ID */
	if (!um->id[idx])
		return ;

	/* Only large Intuos support Lense Cursor */
	if (um->tool[idx] == BTN_TOOL_LENS &&
	    (features->type == INTUOS3 ||
	     features->type == INTUOS3S ||
	     features->type == INTUOS4 ||
	     features->type == INTUOS4S)) {

		return;
	}

	/* Cintiq doesn't send data when RDY bit isn't set */
	if (features->type == CINTIQ && !(data[1] & 0x40))
		return;

	if (features->type >= INTUOS3S) {
		um->X = (data[2] << 9) | (data[3] << 1) | ((data[9] >> 1) & 1);
		um->Y = (data[4] << 9) | (data[5] << 1) | (data[9] & 1);
		um->distance[idx] = ((data[9] >> 2) & 0x3f);
	} else {
		um->X = *((uint16*)&data[2]);
		um->Y = *((uint16 *)&data[4]);
		um->distance[idx] = ((data[9] >> 3) & 0x1f);
	}

	/* process general packets */
	wacom_intuos_general(um, &buttons);

	UBYTE vWheel = 0, hWheel = 0;
	BOOL wheelRelative = FALSE;
	/* 4D mouse, 2D mouse, marker pen rotation, tilt mouse, or Lens cursor packets */
	if ((data[1] & 0xbc) == 0xa8 || (data[1] & 0xbe) == 0xb0 || (data[1] & 0xbc) == 0xac) {

		if (data[1] & 0x02) {
			/* Rotation packet */
			if (features->type >= INTUOS3S) {
				/* I3 marker pen rotation */
				t = (data[6] << 3) | ((data[7] >> 5) & 7);
				t = (data[7] & 0x20) ? ((t > 900) ? ((t-1) / 2 - 1350) :
					((t-1) / 2 + 450)) : (450 - t / 2) ;
				um->Z = t;
			} else {
				/* 4D mouse rotation packet */
				t = (data[6] << 3) | ((data[7] >> 5) & 7);
				// TODO support additionnal axis
				//um->RZ = (data[7] & 0x20) ? ((t - 1) / 2) : -t / 2);
			}

		} else if (!(data[1] & 0x10) && features->type < INTUOS3S) {
			/* 4D mouse packet */
			SETBITS(buttons, BTN_LEFT,   data[8] & 0x01);
			SETBITS(buttons, BTN_MIDDLE, data[8] & 0x02);
			SETBITS(buttons, BTN_RIGHT,  data[8] & 0x04);

			SETBITS(buttons, BTN_SIDE,   data[8] & 0x20);
			SETBITS(buttons, BTN_EXTRA,  data[8] & 0x10);
			t = (data[6] << 2) | ((data[7] >> 6) & 3);
			hWheel = (data[8] & 0x08) ? -t : t;

		} else if (um->tool[idx] == BTN_TOOL_MOUSE) {
			/* I4 mouse */
			if (features->type >= INTUOS4S && features->type <= INTUOS4L) {
				SETBITS(buttons, BTN_LEFT,   data[6] & 0x01);
				SETBITS(buttons, BTN_MIDDLE, data[6] & 0x02);
				SETBITS(buttons, BTN_RIGHT,  data[6] & 0x04);
				vWheel = ((data[7] & 0x80) >> 7) - ((data[7] & 0x40) >> 6);
				SETBITS(buttons, BTN_SIDE,   data[6] & 0x08);
				SETBITS(buttons, BTN_EXTRA,  data[6] & 0x10);

				um->tiltX = (((data[7] << 1) & 0x7e) | (data[8] >> 7));
				um->tiltY = (data[8] & 0x7f);
			} else {
				/* 2D mouse packet */
				SETBITS(buttons, BTN_LEFT,   data[8] & 0x04);
				SETBITS(buttons, BTN_MIDDLE, data[8] & 0x08);
				SETBITS(buttons, BTN_RIGHT,  data[8] & 0x10);
				vWheel = ((data[8] & 0x01) - ((data[8] & 0x02) >> 1));
				wheelRelative = TRUE;

				/* I3 2D mouse side buttons */
				if (features->type >= INTUOS3S && features->type <= INTUOS3L) {
					SETBITS(buttons, BTN_SIDE,   data[8] & 0x40);
					SETBITS(buttons, BTN_EXTRA,  data[8] & 0x20);
				}
			}
		} else if ((features->type < INTUOS3S || features->type == INTUOS3L ||
				features->type == INTUOS4L) &&
			   um->tool[idx] == BTN_TOOL_LENS) {
			/* Lens cursor packets */
			SETBITS(buttons, BTN_LEFT,   data[8] & 0x01);
			SETBITS(buttons, BTN_MIDDLE, data[8] & 0x02);
			SETBITS(buttons, BTN_RIGHT,  data[8] & 0x04);
			SETBITS(buttons, BTN_SIDE,   data[8] & 0x10);
			SETBITS(buttons, BTN_EXTRA,  data[8] & 0x08);
		}
	}

	um->proximity[idx] = TRUE;
	//input_event(input, EV_MSC, MSC_SERIAL, wacom->serial[idx]);
	SendMouseEvent(um, buttons);
	SendTabletEvent(idx, um, buttons);
	SendWheelEvent(um, hWheel, vWheel, wheelRelative);
	return;
}
////

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

				case DTU:
					WacomHandler_dtu(um);
					break;

				case PL:
					WacomHandler_pl(um);
					break;


				case BAMBOO_PT:
					WacomHandler_bpt(um, req->io_Actual);
					break;

				case WACOM_G4:
				case GRAPHIRE:
				case WACOM_MO:
					WacomHandler_graphire(um);
					break;

				case PTU:
					WacomHandler_ptu(um);
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
				case WACOM_21UX2:
				case WACOM_24HD:
					WacomHandler_intuos(um);
					break;
			

				default:
	   				DebugLog(0, um, "WacomHandler: unsupported tablet type %ld\n", um->features->type);
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

