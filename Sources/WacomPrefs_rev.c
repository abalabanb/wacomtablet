/*

************************************************************
**
** Created by: CodeBench 0.23 (17.09.2011)
**
** Project: WacomTablet.usbfd
**
** File: Pseudo preferences program version.
**
** Date: 12-03-2013 22:04:02
**
** Copyright 2013 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
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

#include "WacomPrefs_rev.h"
#include <exec/types.h>

#if defined(__amigaos4__)
# define OS "OS4"
#elif defined(__MORPHOS__)
# define OS "MOS"
#else
# define OS "68k"
#endif

#define EDITION ""

#ifndef USED
# define USED  __attribute__((used))
#endif

USED CONST_STRPTR vstring = VERS " " OS EDITION;
USED CONST_STRPTR verstag = NEW_VERSTAG " Compiled for " OS;


