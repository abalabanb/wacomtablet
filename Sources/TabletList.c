/*

************************************************************
**
** Created by: CodeBench 0.23 (17.09.2011)
**
** Project: WacomTablet.usbfd
**
** File: Supported tablet device lists and tables
**
** Date: 02-11-2019 23:08:10
**
** Copyright 2012-2019 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
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

/* resolution for penabled devices */
#define WACOM_PL_RES        20
#define WACOM_PENPRTN_RES   40
#define WACOM_VOLITO_RES    50
#define WACOM_GRAPHIRE_RES  80
#define WACOM_INTUOS_RES    100
#define WACOM_INTUOS3_RES   200

/* Newer Cintiq and DTU have an offset between tablet and screen areas */
#define WACOM_DTU_OFFSET    200
#define WACOM_CINTIQ_OFFSET 400

/// tablet features definition
static const struct wacom_features wacom_features_0x00 =
    { "Wacom Penpartner",     WACOM_PKGLEN_PENPRTN,    5040, 3780, 255, 0,
      PENPARTNER, WACOM_PENPRTN_RES, WACOM_PENPRTN_RES };
static const struct wacom_features wacom_features_0x10 =
    { "Wacom Graphire",       WACOM_PKGLEN_GRAPHIRE,  10206, 7422, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x11 =
    { "Wacom Graphire2 4x5",  WACOM_PKGLEN_GRAPHIRE,  10206, 7422, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x12 =
    { "Wacom Graphire2 5x7",  WACOM_PKGLEN_GRAPHIRE,  13918, 10206, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x13 =
    { "Wacom Graphire3",      WACOM_PKGLEN_GRAPHIRE,  10208, 7424, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x14 =
    { "Wacom Graphire3 6x8",  WACOM_PKGLEN_GRAPHIRE,  16704, 12064, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x15 =
    { "Wacom Graphire4 4x5",  WACOM_PKGLEN_GRAPHIRE,  10208, 7424, 511, 63,
      WACOM_G4, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x16 =
    { "Wacom Graphire4 6x8",  WACOM_PKGLEN_GRAPHIRE,  16704, 12064, 511, 63,
      WACOM_G4, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x17 =
    { "Wacom BambooFun 4x5",  WACOM_PKGLEN_BBFUN,     14760, 9225, 511, 63,
      WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x18 =
    { "Wacom BambooFun 6x8",  WACOM_PKGLEN_BBFUN,     21648, 13530, 511, 63,
      WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x19 =
    { "Wacom Bamboo1 Medium", WACOM_PKGLEN_GRAPHIRE,  16704, 12064, 511, 63,
      GRAPHIRE, WACOM_GRAPHIRE_RES, WACOM_GRAPHIRE_RES };
static const struct wacom_features wacom_features_0x60 =
    { "Wacom Volito",         WACOM_PKGLEN_GRAPHIRE,   5104, 3712, 511, 63,
      GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x61 =
    { "Wacom PenStation2",    WACOM_PKGLEN_GRAPHIRE,   3250, 2320, 255, 63,
      GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x62 =
    { "Wacom Volito2 4x5",    WACOM_PKGLEN_GRAPHIRE,   5104, 3712, 511, 63,
      GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x63 =
    { "Wacom Volito2 2x3",    WACOM_PKGLEN_GRAPHIRE,   3248, 2320, 511, 63,
      GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x64 =
    { "Wacom PenPartner2",    WACOM_PKGLEN_GRAPHIRE,   3250, 2320, 511, 63,
      GRAPHIRE, WACOM_VOLITO_RES, WACOM_VOLITO_RES };
static const struct wacom_features wacom_features_0x65 =
    { "Wacom Bamboo",         WACOM_PKGLEN_BBFUN,     14760, 9225, 511, 63,
      WACOM_MO, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x69 =
    { "Wacom Bamboo1",        WACOM_PKGLEN_GRAPHIRE,   5104, 3712, 511, 63,
      GRAPHIRE, WACOM_PENPRTN_RES, WACOM_PENPRTN_RES };
static const struct wacom_features wacom_features_0x6A =
    { "Wacom Bamboo1 4x6",    WACOM_PKGLEN_GRAPHIRE,  14760, 9225, 1023, 63,
      GRAPHIRE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x6B =
    { "Wacom Bamboo1 5x8",    WACOM_PKGLEN_GRAPHIRE,  21648, 13530, 1023, 63,
      GRAPHIRE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x20 =
    { "Wacom Intuos 4x5",     WACOM_PKGLEN_INTUOS,    12700, 10600, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x21 =
    { "Wacom Intuos 6x8",     WACOM_PKGLEN_INTUOS,    20320, 16240, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x22 =
    { "Wacom Intuos 9x12",    WACOM_PKGLEN_INTUOS,    30480, 24060, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x23 =
    { "Wacom Intuos 12x12",   WACOM_PKGLEN_INTUOS,    30480, 31680, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x24 =
    { "Wacom Intuos 12x18",   WACOM_PKGLEN_INTUOS,    45720, 31680, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x30 =
    { "Wacom PL400",          WACOM_PKGLEN_GRAPHIRE,   5408, 4056, 255, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x31 =
    { "Wacom PL500",          WACOM_PKGLEN_GRAPHIRE,   6144, 4608, 255, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x32 =
    { "Wacom PL600",          WACOM_PKGLEN_GRAPHIRE,   6126, 4604, 255, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x33 =
    { "Wacom PL600SX",        WACOM_PKGLEN_GRAPHIRE,   6260, 5016, 255, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x34 =
    { "Wacom PL550",          WACOM_PKGLEN_GRAPHIRE,   6144, 4608, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x35 =
    { "Wacom PL800",          WACOM_PKGLEN_GRAPHIRE,   7220, 5780, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x37 =
    { "Wacom PL700",          WACOM_PKGLEN_GRAPHIRE,   6758, 5406, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x38 =
    { "Wacom PL510",          WACOM_PKGLEN_GRAPHIRE,   6282, 4762, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x39 =
    { "Wacom DTU710",         WACOM_PKGLEN_GRAPHIRE,  34080, 27660, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC4 =
    { "Wacom DTF521",         WACOM_PKGLEN_GRAPHIRE,   6282, 4762, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC0 =
    { "Wacom DTF720",         WACOM_PKGLEN_GRAPHIRE,   6858, 5506, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0xC2 =
    { "Wacom DTF720a",        WACOM_PKGLEN_GRAPHIRE,   6858, 5506, 511, 0,
      PL, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x03 =
    { "Wacom Cintiq Partner", WACOM_PKGLEN_GRAPHIRE,  20480, 15360, 511, 0,
      PTU, WACOM_PL_RES, WACOM_PL_RES };
static const struct wacom_features wacom_features_0x41 =
    { "Wacom Intuos2 4x5",    WACOM_PKGLEN_INTUOS,    12700, 10600, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x42 =
    { "Wacom Intuos2 6x8",    WACOM_PKGLEN_INTUOS,    20320, 16240, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x43 =
    { "Wacom Intuos2 9x12",   WACOM_PKGLEN_INTUOS,    30480, 24060, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x44 =
    { "Wacom Intuos2 12x12",  WACOM_PKGLEN_INTUOS,    30480, 31680, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x45 =
    { "Wacom Intuos2 12x18",  WACOM_PKGLEN_INTUOS,    45720, 31680, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xB0 =
    { "Wacom Intuos3 4x5",    WACOM_PKGLEN_INTUOS,    25400, 20320, 1023, 63,
      INTUOS3S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 4 };
static const struct wacom_features wacom_features_0xB1 =
    { "Wacom Intuos3 6x8",    WACOM_PKGLEN_INTUOS,    40640, 30480, 1023, 63,
      INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xB2 =
    { "Wacom Intuos3 9x12",   WACOM_PKGLEN_INTUOS,    60960, 45720, 1023, 63,
      INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xB3 =
    { "Wacom Intuos3 12x12",  WACOM_PKGLEN_INTUOS,    60960, 60960, 1023, 63,
      INTUOS3L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xB4 =
    { "Wacom Intuos3 12x19",  WACOM_PKGLEN_INTUOS,    97536, 60960, 1023, 63,
      INTUOS3L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xB5 =
    { "Wacom Intuos3 6x11",   WACOM_PKGLEN_INTUOS,    54204, 31750, 1023, 63,
      INTUOS3, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xB7 =
    { "Wacom Intuos3 4x6",    WACOM_PKGLEN_INTUOS,    31496, 19685, 1023, 63,
      INTUOS3S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 4 };
static const struct wacom_features wacom_features_0xB8 =
    { "Wacom Intuos4 4x6",    WACOM_PKGLEN_INTUOS,    31496, 19685, 2047, 63,
      INTUOS4S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 7 };
static const struct wacom_features wacom_features_0xB9 =
    { "Wacom Intuos4 6x9",    WACOM_PKGLEN_INTUOS,    44704, 27940, 2047, 63,
      INTUOS4, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9 };
static const struct wacom_features wacom_features_0xBA =
    { "Wacom Intuos4 8x13",   WACOM_PKGLEN_INTUOS,    65024, 40640, 2047, 63,
      INTUOS4L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9 };
static const struct wacom_features wacom_features_0xBB =
    { "Wacom Intuos4 12x19",  WACOM_PKGLEN_INTUOS,    97536, 60960, 2047, 63,
      INTUOS4L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9 };
static const struct wacom_features wacom_features_0xBC =
    { "Wacom Intuos4 WL",     WACOM_PKGLEN_INTUOS,    40640, 25400, 2047, 63,
      INTUOS4, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9 };
static const struct wacom_features wacom_features_0x26 =
    { "Wacom Intuos5 touch S", WACOM_PKGLEN_INTUOS,  31496, 19685, 2047, 63,
      INTUOS5S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 7, .touch_max = 16 };
static const struct wacom_features wacom_features_0x27 =
    { "Wacom Intuos5 touch M", WACOM_PKGLEN_INTUOS,  44704, 27940, 2047, 63,
      INTUOS5, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 16 };
static const struct wacom_features wacom_features_0x28 =
    { "Wacom Intuos5 touch L", WACOM_PKGLEN_INTUOS, 65024, 40640, 2047, 63,
      INTUOS5L, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 16 };
static const struct wacom_features wacom_features_0x29 =
    { "Wacom Intuos5 S", WACOM_PKGLEN_INTUOS,  31496, 19685, 2047, 63,
      INTUOS5S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 7 };
static const struct wacom_features wacom_features_0x2A =
    { "Wacom Intuos5 M", WACOM_PKGLEN_INTUOS,  44704, 27940, 2047, 63,
      INTUOS5, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9 };
static const struct wacom_features wacom_features_0x314 =
    { "Wacom Intuos Pro S", WACOM_PKGLEN_INTUOS,  31496, 19685, 2047, 63,
      INTUOSPS, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 7, .touch_max = 16 };
static const struct wacom_features wacom_features_0x315 =
    { "Wacom Intuos Pro M", WACOM_PKGLEN_INTUOS,  44704, 27940, 2047, 63,
      INTUOSPM, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 16 };
static const struct wacom_features wacom_features_0x317 =
    { "Wacom Intuos Pro L", WACOM_PKGLEN_INTUOS,  65024, 40640, 2047, 63,
      INTUOSPL, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 16 };
static const struct wacom_features wacom_features_0xF4 =
    { "Wacom Cintiq 24HD",       WACOM_PKGLEN_INTUOS,   104480, 65600, 2047, 63,
      WACOM_24HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 16,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0xF8 =
    { "Wacom Cintiq 24HD touch", WACOM_PKGLEN_INTUOS,   104480, 65600, 2047, 63, /* Pen */
      WACOM_24HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 16,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0xf6 };
static const struct wacom_features wacom_features_0xF6 =
    { "Wacom Cintiq 24HD touch", .type = WACOM_24HDT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0xf8, .touch_max = 10 };
static const struct wacom_features wacom_features_0x32A =
    { "Wacom Cintiq 27QHD", WACOM_PKGLEN_INTUOS, 120140, 67920, 2047, 63,
      WACOM_27QHD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x32B =
    { "Wacom Cintiq 27QHD touch", WACOM_PKGLEN_INTUOS, 120140, 67920, 2047, 63,
      WACOM_27QHD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x32C };
static const struct wacom_features wacom_features_0x32C =
    { "Wacom Cintiq 27QHD touch", .type = WACOM_27QHDT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x32B, .touch_max = 10 };
static const struct wacom_features wacom_features_0x3F =
    { "Wacom Cintiq 21UX",    WACOM_PKGLEN_INTUOS,    87200, 65600, 1023, 63,
      CINTIQ, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 8 };
static const struct wacom_features wacom_features_0xC5 =
    { "Wacom Cintiq 20WSX",   WACOM_PKGLEN_INTUOS,    86680, 54180, 1023, 63,
      WACOM_BEE, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 10 };
static const struct wacom_features wacom_features_0xC6 =
    { "Wacom Cintiq 12WX",    WACOM_PKGLEN_INTUOS,    53020, 33440, 1023, 63,
      WACOM_BEE, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 10 };
static const struct wacom_features wacom_features_0x304 =
    { "Wacom Cintiq 13HD",    WACOM_PKGLEN_INTUOS,    59552, 33848, 1023, 63,
      WACOM_13HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x333 =
    { "Wacom Cintiq 13HD touch", WACOM_PKGLEN_INTUOS, 59552, 33848, 2047, 63,
      WACOM_13HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x335 };
static const struct wacom_features wacom_features_0x335 =
    { "Wacom Cintiq 13HD touch", .type = WACOM_24HDT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x333, .touch_max = 10 };
static const struct wacom_features wacom_features_0xC7 =
    { "Wacom DTU1931",        WACOM_PKGLEN_GRAPHIRE,  37832, 30305, 511, 0,
      PL, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xCE =
    { "Wacom DTU2231",        WACOM_PKGLEN_GRAPHIRE,  47864, 27011, 511, 0,
      DTU, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xF0 =
    { "Wacom DTU1631",        WACOM_PKGLEN_GRAPHIRE,  34623, 19553, 511, 0,
      DTU, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xFB =
    { "Wacom DTU1031",        WACOM_PKGLEN_DTUS,      22096, 13960, 511, 0,
      DTUS, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET };
static const struct wacom_features wacom_features_0x32F =
    { "Wacom DTU1031X",       WACOM_PKGLEN_DTUS,      22672, 12928, 511, 0,
      DTUSX, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 0,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET };
static const struct wacom_features wacom_features_0x336 =
    { "Wacom DTU1141",        WACOM_PKGLEN_DTUS,      23672, 13403, 1023, 0,
      DTUS, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET };
static const struct wacom_features wacom_features_0x57 =
    { "Wacom DTK2241",        WACOM_PKGLEN_INTUOS,    95840, 54260, 2047, 63,
      DTK, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 6,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x59 = /* Pen */
    { "Wacom DTH2242",        WACOM_PKGLEN_INTUOS,    95840, 54260, 2047, 63,
      DTK, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 6,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x5D };
static const struct wacom_features wacom_features_0x5D = /* Touch */
    { "Wacom DTH2242",       .type = WACOM_24HDT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x59, .touch_max = 10 };
static const struct wacom_features wacom_features_0xCC =
    { "Wacom Cintiq 21UX2",   WACOM_PKGLEN_INTUOS,    87200, 65600, 2047, 63,
      WACOM_21UX2, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 18,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0xFA =
    { "Wacom Cintiq 22HD",    WACOM_PKGLEN_INTUOS,    95840, 54260, 2047, 63,
      WACOM_22HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 18,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x5B =
    { "Wacom Cintiq 22HDT", WACOM_PKGLEN_INTUOS,      95840, 54260, 2047, 63,
      WACOM_22HD, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 18,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x5e };
static const struct wacom_features wacom_features_0x5E =
    { "Wacom Cintiq 22HDT", .type = WACOM_24HDT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x5b, .touch_max = 10 };
static const struct wacom_features wacom_features_0x90 =
    { "Wacom ISDv4 90",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x93 =
    { "Wacom ISDv4 93",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x97 =
    { "Wacom ISDv4 97",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 511, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x9A =
    { "Wacom ISDv4 9A",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x9F =
    { "Wacom ISDv4 9F",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xE2 =
    { "Wacom ISDv4 E2",       WACOM_PKGLEN_TPC2FG,    26202, 16325, 255, 0,
      TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xE3 =
    { "Wacom ISDv4 E3",       WACOM_PKGLEN_TPC2FG,    26202, 16325, 255, 0,
      TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xE5 =
    { "Wacom ISDv4 E5",       WACOM_PKGLEN_MTOUCH,    26202, 16325, 255, 0,
      MTSCREEN, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xE6 =
    { "Wacom ISDv4 E6",       WACOM_PKGLEN_TPC2FG,    27760, 15694, 255, 0,
      TABLETPC2FG, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xEC =
    { "Wacom ISDv4 EC",       WACOM_PKGLEN_GRAPHIRE,  25710, 14500, 255, 0,
      TABLETPC,    WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xED =
    { "Wacom ISDv4 ED",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPCE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xEF =
    { "Wacom ISDv4 EF",       WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x100 =
    { "Wacom ISDv4 100",      WACOM_PKGLEN_MTTPC,     26202, 16325, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x101 =
    { "Wacom ISDv4 101",      WACOM_PKGLEN_MTTPC,     26202, 16325, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x10D =
    { "Wacom ISDv4 10D",      WACOM_PKGLEN_MTTPC,     26202, 16325, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x10E =
    { "Wacom ISDv4 10E",      WACOM_PKGLEN_MTTPC,     27760, 15694, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x10F =
    { "Wacom ISDv4 10F",      WACOM_PKGLEN_MTTPC,     27760, 15694, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x116 =
    { "Wacom ISDv4 116",      WACOM_PKGLEN_GRAPHIRE,  26202, 16325, 255, 0,
      TABLETPCE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x12C =
    { "Wacom ISDv4 12C",      WACOM_PKGLEN_GRAPHIRE,  27848, 15752, 2047, 0,
      TABLETPCE, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x4001 =
    { "Wacom ISDv4 4001",      WACOM_PKGLEN_MTTPC,     26202, 16325, 255, 0,
      MTTPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x4004 =
    { "Wacom ISDv4 4004",      WACOM_PKGLEN_MTTPC,     11060, 6220, 255, 0,
      MTTPC_B, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5000 =
    { "Wacom ISDv4 5000",      WACOM_PKGLEN_MTTPC,     27848, 15752, 1023, 0,
      MTTPC_B, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5002 =
    { "Wacom ISDv4 5002",      WACOM_PKGLEN_MTTPC,     29576, 16724,  1023, 0,
      MTTPC_B, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5010 =
    { "Wacom ISDv4 5010",      WACOM_PKGLEN_MTTPC,     13756, 7736,  1023, 0,
      MTTPC_B, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5013 =
    { "Wacom ISDv4 5013",      WACOM_PKGLEN_MTTPC,     11752, 6612,  1023, 0,
      MTTPC_B, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5044 =
    { "Wacom ISDv4 5044",      WACOM_PKGLEN_MTTPC,     27648, 15552, 2047, 0,
      MTTPC_C, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5048 =
    { "Wacom ISDv4 5048",      WACOM_PKGLEN_MTTPC,     27648, 15552, 2047, 0,
      MTTPC_C, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x5090 =
    { "Wacom ISDv4 5090",      WACOM_PKGLEN_MTTPC,     27648, 15552, 2047, 0,
      MTTPC_C, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x47 =
    { "Wacom Intuos2 6x8",    WACOM_PKGLEN_INTUOS,    20320, 16240, 1023, 31,
      INTUOS, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x84 =
    { "Wacom Wireless Receiver", WACOM_PKGLEN_WIRELESS, .type = WIRELESS, .touch_max = 16 };
static const struct wacom_features wacom_features_0xD0 =
    { "Wacom Bamboo 2FG",     WACOM_PKGLEN_BBFUN,     14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD1 =
    { "Wacom Bamboo 2FG 4x5", WACOM_PKGLEN_BBFUN,     14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD2 =
    { "Wacom Bamboo Craft",   WACOM_PKGLEN_BBFUN,     14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD3 =
    { "Wacom Bamboo 2FG 6x8", WACOM_PKGLEN_BBFUN,     21648, 13700, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD4 =
    { "Wacom Bamboo Pen",     WACOM_PKGLEN_BBFUN,     14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xD5 =
    { "Wacom Bamboo Pen 6x8",     WACOM_PKGLEN_BBFUN, 21648, 13700, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xD6 =
    { "Wacom BambooPT 2FG 4x5", WACOM_PKGLEN_BBFUN,   14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD7 =
    { "Wacom BambooPT 2FG Small", WACOM_PKGLEN_BBFUN, 14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xD8 =
    { "Wacom Bamboo Comic 2FG", WACOM_PKGLEN_BBFUN,   21648, 13700, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xDA =
    { "Wacom Bamboo 2FG 4x5 SE", WACOM_PKGLEN_BBFUN,  14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xDB =
    { "Wacom Bamboo 2FG 6x8 SE", WACOM_PKGLEN_BBFUN,  21648, 13700, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 2 };
static const struct wacom_features wacom_features_0xDD =
        { "Wacom Bamboo Connect", WACOM_PKGLEN_BBPEN,     14720, 9200, 1023, 31,
          BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0xDE =
        { "Wacom Bamboo 16FG 4x5", WACOM_PKGLEN_BBPEN,    14720, 9200, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0xDF =
        { "Wacom Bamboo 16FG 6x8", WACOM_PKGLEN_BBPEN,    21648, 13700, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0x300 =
    { "Wacom Bamboo One S",    WACOM_PKGLEN_BBPEN,    14720, 9225, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x301 =
    { "Wacom Bamboo One M",    WACOM_PKGLEN_BBPEN,    21648, 13530, 1023, 31,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x302 =
    { "Wacom Intuos PT S",     WACOM_PKGLEN_BBPEN,    15200, 9500, 1023, 31,
      INTUOSHT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0x303 =
    { "Wacom Intuos PT M",     WACOM_PKGLEN_BBPEN,    21600, 13500, 1023, 31,
      INTUOSHT, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0x30E =
    { "Wacom Intuos S",        WACOM_PKGLEN_BBPEN,    15200, 9500, 1023, 31,
      INTUOSHT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x6004 =
    { "ISD-V4",               WACOM_PKGLEN_GRAPHIRE,  12800, 8000, 255, 0,
      TABLETPC, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x307 =
    { "Wacom ISDv5 307", WACOM_PKGLEN_INTUOS,  59552, 33848, 2047, 63,
      CINTIQ_HYBRID, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x309 };
static const struct wacom_features wacom_features_0x309 =
    { "Wacom ISDv5 309", .type = WACOM_24HDT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x0307, .touch_max = 10 };
static const struct wacom_features wacom_features_0x30A =
    { "Wacom ISDv5 30A", WACOM_PKGLEN_INTUOS, 59552, 33848, 2047, 63,
      CINTIQ_HYBRID, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x30C };
static const struct wacom_features wacom_features_0x30C =
    { "Wacom ISDv5 30C", .type = WACOM_24HDT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x30A, .touch_max = 10 };
static const struct wacom_features wacom_features_0x323 =
    { "Wacom Intuos P M", WACOM_PKGLEN_BBPEN,    21600, 13500, 1023, 31,
      INTUOSHT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x325 =
    { "Wacom ISDv5 325", WACOM_PKGLEN_INTUOS,    59552, 33848, 2047, 63,
      CINTIQ_COMPANION_2, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 11,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x326 };
static const struct wacom_features wacom_features_0x326 = /* Touch */
    { "Wacom ISDv5 326", .type = WACOM_24HDT, .oVid = USB_VENDOR_ID_WACOM,
      .oPid = 0x325, .touch_max = 10 };
static const struct wacom_features wacom_features_0x331 =
    { "Wacom Express Key Remote", WACOM_PKGLEN_WIRELESS, .type = REMOTE,
      .numbered_buttons = 18 };
static const struct wacom_features wacom_features_0x33B =
    { "Wacom Intuos S 2", WACOM_PKGLEN_INTUOS, 15200, 9500, 2047, 63,
      INTUOSHT2, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x33C =
    { "Wacom Intuos PT S 2", WACOM_PKGLEN_INTUOS, 15200, 9500, 2047, 63,
      INTUOSHT2, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0x33D =
    { "Wacom Intuos P M 2",  WACOM_PKGLEN_INTUOS, 21600, 13500, 2047, 63,
      INTUOSHT2, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x33E =
    { "Wacom Intuos PT M 2", WACOM_PKGLEN_INTUOS, 21600, 13500, 2047, 63,
      INTUOSHT2, WACOM_INTUOS_RES, WACOM_INTUOS_RES, .touch_max = 16 };
static const struct wacom_features wacom_features_0x343 =
    { "Wacom DTK1651", WACOM_PKGLEN_DTUS, 34816, 19759, 1023, 0,
      DTUS, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET };
static const struct wacom_features wacom_features_0x34A =
    { "Wacom MobileStudio Pro 13 Touch", WACOM_PKGLEN_MSPROT, .type = WACOM_MSPROT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x34D };
static const struct wacom_features wacom_features_0x34B =
    { "Wacom MobileStudio Pro 16 Touch", WACOM_PKGLEN_MSPROT, .type = WACOM_MSPROT, /* Touch */
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x34E };
static const struct wacom_features wacom_features_0x34D =
    { "Wacom MobileStudio Pro 13", WACOM_PKGLEN_MSPRO, 59552, 33848, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 11,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x34A };
static const struct wacom_features wacom_features_0x34E =
    { "Wacom MobileStudio Pro 16", WACOM_PKGLEN_MSPRO, 69920, 39680, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 13,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x34B };
static const struct wacom_features wacom_features_0x34F =
    { "Wacom Cintiq Pro 13 FHD", WACOM_PKGLEN_MSPRO, 59552, 33848, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x353 };
static const struct wacom_features wacom_features_0x350 =
    { "Wacom Cintiq Pro 16UHD", WACOM_PKGLEN_MSPRO, 69920, 39680, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x354 };
static const struct wacom_features wacom_features_0x351 =
    { "Wacom Cintiq Pro 24", WACOM_PKGLEN_MSPRO, 105286, 59574, 8191, 63, /* Pen & Touch */
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x355 };
static const struct wacom_features wacom_features_0x352 =
    { "Wacom Cintiq Pro 32", WACOM_PKGLEN_MSPRO, 140384, 79316, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x356 };
static const struct wacom_features wacom_features_0x353 =
    { "Wacom Cintiq Pro 13FHD Touch", WACOM_PKGLEN_MSPROT, .type = WACOM_MSPROT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x34f }; /* Touch */
static const struct wacom_features wacom_features_0x354 =
    { "Wacom Cintiq Pro 16UHD Touch", WACOM_PKGLEN_MSPROT, .type = WACOM_MSPROT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x350 }; /* Touch */
static const struct wacom_features wacom_features_0x355 =
    { "Wacom Cintiq Pro 24 Touch", WACOM_PKGLEN_27QHDT, .type = WACOM_27QHDT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x351, .touch_max = 10 }; /* Touch */
static const struct wacom_features wacom_features_0x356 =
    { "Wacom Cintiq Pro 32 Touch", WACOM_PKGLEN_27QHDT, .type = WACOM_27QHDT,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x352, .touch_max = 10 }; /* Touch */
static const struct wacom_features wacom_features_0x357 =
    { "Wacom Intuos Pro M", WACOM_PKGLEN_INTUOSP2, 44800, 29600, 8191, 63,
      INTUOSP2, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 10 };
static const struct wacom_features wacom_features_0x358 =
    { "Wacom Intuos Pro L", WACOM_PKGLEN_INTUOSP2, 62200, 43200, 8191, 63,
      INTUOSP2, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 9, .touch_max = 10 };
static const struct wacom_features wacom_features_0x359 =
    { "Wacom DTU-1141B", WACOM_PKGLEN_DTH1152, 22320, 12555, 1023, 0,
      DTUS2, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4 };
static const struct wacom_features wacom_features_0x35A =
    { "Wacom DTH-1152", WACOM_PKGLEN_DTH1152, 22320, 12555, 1023, 0,
      DTH1152, WACOM_INTUOS_RES, WACOM_INTUOS_RES,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x368 };
static const struct wacom_features wacom_features_0x368 =
    { "Wacom DTH-1152 Touch", WACOM_PKGLEN_27QHDT,
      .type = DTH1152T, .touch_max = 10, .oVid = USB_VENDOR_ID_WACOM,
      .oPid = 0x35A }; /* Touch */
static const struct wacom_features wacom_features_0x374 =
    { "Intuos S", WACOM_PKGLEN_INTUOSP2, 15200, 9500, 4095,
      63, INTUOSHT3, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4 };
static const struct wacom_features wacom_features_0x375 =
    { "Intuos M", WACOM_PKGLEN_INTUOSP2, 21600, 13500, 4095,
      63, INTUOSHT3, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4 };
static const struct wacom_features wacom_features_0x376 =
    { "Intuos BT S", WACOM_PKGLEN_INTUOSP2, 15200, 9500, 4095,
      63, INTUOSHT3, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4 };
static const struct wacom_features wacom_features_0x378 =
    { "Intuos BT M", WACOM_PKGLEN_INTUOSP2, 21600, 13500, 4095,
      63, INTUOSHT3, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4 };
static const struct wacom_features wacom_features_0x37A =
    { "Wacom One by Wacom S", WACOM_PKGLEN_BBPEN, 15200, 9500, 2047, 63,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x37B =
    { "Wacom One by Wacom M", WACOM_PKGLEN_BBPEN, 21600, 13500, 2047, 63,
      BAMBOO_PT, WACOM_INTUOS_RES, WACOM_INTUOS_RES };
static const struct wacom_features wacom_features_0x37C =
    { "Wacom Cintiq Pro 24", WACOM_PKGLEN_MSPRO, 105286, 59574, 8191, 63, /* Pen-only */
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x37D =
    { "Wacom DTH-2452", WACOM_PKGLEN_DTH1152, 53104, 30046, 2047,
      0, DTK2451, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x37E };
static const struct wacom_features wacom_features_0x37E =
    { "Wacom DTH-2452 Touch", WACOM_PKGLEN_MSPROT,
      .type = DTH2452T, .touch_max = 10, .oVid = USB_VENDOR_ID_WACOM,
      .oPid = 0x37D }; /* Touch */
static const struct wacom_features wacom_features_0x382 =
    { "Wacom DTK-2451", WACOM_PKGLEN_DTH1152, 53104, 30046, 2047,
      0, DTK2451, WACOM_INTUOS_RES, WACOM_INTUOS_RES, 4,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET,
      WACOM_DTU_OFFSET, WACOM_DTU_OFFSET };
static const struct wacom_features wacom_features_0x390 =
    { "Wacom Cintiq 16", WACOM_PKGLEN_MSPRO, 69632, 39518, 8191, 63,
      CINTIQ_16, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x391 =
    { "Wacom Cintiq 22", WACOM_PKGLEN_MSPRO, 96012, 54358, 8191, 63,
      CINTIQ_16, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x392 =
    { "Wacom Intuos Pro S", WACOM_PKGLEN_INTUOSP2, 31920, 19950, 8191, 63,
      INTUOSP2S, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 7, .touch_max = 10 };
static const struct wacom_features wacom_features_0x396 =
    { "Wacom DTK-1660E", WACOM_PKGLEN_MSPRO, 69632, 39518, 8191, 63,
      CINTIQ_16, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 0,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET };
static const struct wacom_features wacom_features_0x398 =
    { "Wacom MobileStudio Pro 13", WACOM_PKGLEN_MSPRO, 59552, 33848, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 11,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x39A };
static const struct wacom_features wacom_features_0x399 =
    { "Wacom MobileStudio Pro 16", WACOM_PKGLEN_MSPRO, 69920, 39680, 8191, 63,
      WACOM_MSPRO, WACOM_INTUOS3_RES, WACOM_INTUOS3_RES, 13,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      WACOM_CINTIQ_OFFSET, WACOM_CINTIQ_OFFSET,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x39B };
static const struct wacom_features wacom_features_0x39A =
    { "Wacom MobileStudio Pro 13 Touch", WACOM_PKGLEN_MSPROT, /* Touch */
      .type = WACOM_MSPROT, .touch_max = 10,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x398 };
static const struct wacom_features wacom_features_0x39B =
    { "Wacom MobileStudio Pro 16 Touch", WACOM_PKGLEN_MSPROT, /* Touch */
      .type = WACOM_MSPROT, .touch_max = 10,
      .oVid = USB_VENDOR_ID_WACOM, .oPid = 0x399 };

///

/// Supported devices definition list
#define USB_DEVICE_WACOM(prod)  \
    (uint16)USB_VENDOR_ID_WACOM,(uint16) (prod), &wacom_features_##prod
#define USB_DEVICE_DETAILED(prod, class, subclass, protocol) USB_DEVICE_WACOM(prod)

const struct wacom_device wacom_devices[] = {
    { USB_DEVICE_WACOM(0x00) },
    { USB_DEVICE_WACOM(0x03) },
    { USB_DEVICE_WACOM(0x10) },
    { USB_DEVICE_WACOM(0x11) },
    { USB_DEVICE_WACOM(0x12) },
    { USB_DEVICE_WACOM(0x13) },
    { USB_DEVICE_WACOM(0x14) },
    { USB_DEVICE_WACOM(0x15) },
    { USB_DEVICE_WACOM(0x16) },
    { USB_DEVICE_WACOM(0x17) },
    { USB_DEVICE_WACOM(0x18) },
    { USB_DEVICE_WACOM(0x19) },
    { USB_DEVICE_WACOM(0x20) },
    { USB_DEVICE_WACOM(0x21) },
    { USB_DEVICE_WACOM(0x22) },
    { USB_DEVICE_WACOM(0x23) },
    { USB_DEVICE_WACOM(0x24) },
    { USB_DEVICE_WACOM(0x26) },
    { USB_DEVICE_WACOM(0x27) },
    { USB_DEVICE_WACOM(0x28) },
    { USB_DEVICE_WACOM(0x29) },
    { USB_DEVICE_WACOM(0x2A) },
    { USB_DEVICE_WACOM(0x30) },
    { USB_DEVICE_WACOM(0x31) },
    { USB_DEVICE_WACOM(0x32) },
    { USB_DEVICE_WACOM(0x33) },
    { USB_DEVICE_WACOM(0x34) },
    { USB_DEVICE_WACOM(0x35) },
    { USB_DEVICE_WACOM(0x37) },
    { USB_DEVICE_WACOM(0x38) },
    { USB_DEVICE_WACOM(0x39) },
    { USB_DEVICE_WACOM(0x3F) },
    { USB_DEVICE_WACOM(0x41) },
    { USB_DEVICE_WACOM(0x42) },
    { USB_DEVICE_WACOM(0x43) },
    { USB_DEVICE_WACOM(0x44) },
    { USB_DEVICE_WACOM(0x45) },
    { USB_DEVICE_WACOM(0x47) },
    { USB_DEVICE_WACOM(0x57) },
    { USB_DEVICE_WACOM(0x59) },
    { USB_DEVICE_WACOM(0x5B) },
    { USB_DEVICE_DETAILED(0x5D, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x5E, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x60) },
    { USB_DEVICE_WACOM(0x61) },
    { USB_DEVICE_WACOM(0x62) },
    { USB_DEVICE_WACOM(0x63) },
    { USB_DEVICE_WACOM(0x64) },
    { USB_DEVICE_WACOM(0x65) },
    { USB_DEVICE_WACOM(0x69) },
    { USB_DEVICE_WACOM(0x6A) },
    { USB_DEVICE_WACOM(0x6B) },
    { USB_DEVICE_WACOM(0x84) },
    { USB_DEVICE_WACOM(0x90) },
    { USB_DEVICE_WACOM(0x93) },
    { USB_DEVICE_WACOM(0x97) },
    { USB_DEVICE_WACOM(0x9A) },
    { USB_DEVICE_WACOM(0x9F) },
    { USB_DEVICE_WACOM(0xB0) },
    { USB_DEVICE_WACOM(0xB1) },
    { USB_DEVICE_WACOM(0xB2) },
    { USB_DEVICE_WACOM(0xB3) },
    { USB_DEVICE_WACOM(0xB4) },
    { USB_DEVICE_WACOM(0xB5) },
    { USB_DEVICE_WACOM(0xB7) },
    { USB_DEVICE_WACOM(0xB8) },
    { USB_DEVICE_WACOM(0xB9) },
    { USB_DEVICE_WACOM(0xBA) },
    { USB_DEVICE_WACOM(0xBB) },
    { USB_DEVICE_WACOM(0xBC) },
    { USB_DEVICE_WACOM(0xC0) },
    { USB_DEVICE_WACOM(0xC2) },
    { USB_DEVICE_WACOM(0xC4) },
    { USB_DEVICE_WACOM(0xC5) },
    { USB_DEVICE_WACOM(0xC6) },
    { USB_DEVICE_WACOM(0xC7) },
    { USB_DEVICE_WACOM(0xCC) },
    /*
     * DTU-2231 has two interfaces on the same configuration,
     * only one is used.
     */
    { USB_DEVICE_DETAILED(0xCE, USB_CLASS_HID,
                  USB_INTERFACE_SUBCLASS_BOOT,
                  USB_INTERFACE_PROTOCOL_MOUSE) },
    { USB_DEVICE_WACOM(0xD0) },
    { USB_DEVICE_WACOM(0xD1) },
    { USB_DEVICE_WACOM(0xD2) },
    { USB_DEVICE_WACOM(0xD3) },
    { USB_DEVICE_WACOM(0xD4) },
    { USB_DEVICE_WACOM(0xD5) },
    { USB_DEVICE_WACOM(0xD6) },
    { USB_DEVICE_WACOM(0xD7) },
    { USB_DEVICE_WACOM(0xD8) },
    { USB_DEVICE_WACOM(0xDA) },
    { USB_DEVICE_WACOM(0xDB) },
    { USB_DEVICE_WACOM(0xDD) },
    { USB_DEVICE_WACOM(0xDE) },
    { USB_DEVICE_WACOM(0xDF) },
    { USB_DEVICE_WACOM(0xE2) },
    { USB_DEVICE_WACOM(0xE3) },
    { USB_DEVICE_WACOM(0xE5) },
    { USB_DEVICE_WACOM(0xE6) },
    { USB_DEVICE_WACOM(0xEC) },
    { USB_DEVICE_WACOM(0xED) },
    { USB_DEVICE_WACOM(0xEF) },
    { USB_DEVICE_WACOM(0xF0) },
    { USB_DEVICE_WACOM(0xF4) },
    { USB_DEVICE_DETAILED(0xF6, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0xF8) },
    { USB_DEVICE_WACOM(0xFA) },
    { USB_DEVICE_WACOM(0xFB) },
    { USB_DEVICE_WACOM(0x100) },
    { USB_DEVICE_WACOM(0x101) },
    { USB_DEVICE_WACOM(0x10D) },
    { USB_DEVICE_WACOM(0x10E) },
    { USB_DEVICE_WACOM(0x10F) },
    { USB_DEVICE_WACOM(0x116) },
    { USB_DEVICE_WACOM(0x12C) },
    { USB_DEVICE_WACOM(0x300) },
    { USB_DEVICE_WACOM(0x301) },
    { USB_DEVICE_DETAILED(0x302, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x303, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x304) },
    { USB_DEVICE_WACOM(0x307) },
    { USB_DEVICE_DETAILED(0x309, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x30A) },
    { USB_DEVICE_WACOM(0x30C) },
    { USB_DEVICE_DETAILED(0x30E, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x314, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x315, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x317, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x323, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x325) },
    { USB_DEVICE_WACOM(0x326) },
    { USB_DEVICE_WACOM(0x32A) },
    { USB_DEVICE_WACOM(0x32B) },
    { USB_DEVICE_WACOM(0x32C) },
    { USB_DEVICE_WACOM(0x32F) },
    { USB_DEVICE_DETAILED(0x331, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x333) },
    { USB_DEVICE_WACOM(0x335) },
    { USB_DEVICE_WACOM(0x336) },
    { USB_DEVICE_DETAILED(0x33B, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x33C, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x33D, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x33E, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x343) },
    { USB_DEVICE_WACOM(0x34A) },
    { USB_DEVICE_WACOM(0x34B) },
    { USB_DEVICE_WACOM(0x34D) },
    { USB_DEVICE_WACOM(0x34E) },
    { USB_DEVICE_WACOM(0x34F) },
    { USB_DEVICE_WACOM(0x350) },
    { USB_DEVICE_WACOM(0x351) },
    { USB_DEVICE_WACOM(0x352) },
    { USB_DEVICE_WACOM(0x353) },
    { USB_DEVICE_WACOM(0x354) },
    { USB_DEVICE_WACOM(0x355) },
    { USB_DEVICE_WACOM(0x356) },
    { USB_DEVICE_DETAILED(0x357, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x358, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x359) },
    { USB_DEVICE_WACOM(0x35A) },
    { USB_DEVICE_WACOM(0x368) },
    { USB_DEVICE_WACOM(0x374) },
    { USB_DEVICE_WACOM(0x375) },
    { USB_DEVICE_WACOM(0x376) },
    { USB_DEVICE_WACOM(0x378) },
    { USB_DEVICE_WACOM(0x37A) },
    { USB_DEVICE_WACOM(0x37B) },
    { USB_DEVICE_WACOM(0x37C) },
    { USB_DEVICE_WACOM(0x37D) },
    { USB_DEVICE_WACOM(0x37E) },
    { USB_DEVICE_WACOM(0x382) },
    { USB_DEVICE_DETAILED(0x390, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x391, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x392, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_DETAILED(0x396, USB_CLASS_HID, 0, 0) },
    { USB_DEVICE_WACOM(0x398) },
    { USB_DEVICE_WACOM(0x399) },
    { USB_DEVICE_WACOM(0x39A) },
    { USB_DEVICE_WACOM(0x39B) },
    { USB_DEVICE_WACOM(0x4001) },
    { USB_DEVICE_WACOM(0x4004) },
    { USB_DEVICE_WACOM(0x5000) },
    { USB_DEVICE_WACOM(0x5002) },
    { USB_DEVICE_WACOM(0x5010) },
    { USB_DEVICE_WACOM(0x5013) },
    { USB_DEVICE_WACOM(0x5044) },
    { USB_DEVICE_WACOM(0x5048) },
    { USB_DEVICE_WACOM(0x5090) },
    { 0, 0, NULL }
};
///

