
Version 1.1

This is a driver for Wacom Graphics Tablets. These tablets typically work with
the driver from 'The Linux Wacom Project' under linux.

Alexandre Balaban

alexandre(-@-)balaban(-.-)fr
http://www.balaban.fr

License
-------

This driver uses parts of code from "The Linux Wacom Project", as such it's
bound to the GPL v2. See attached file "GPL" for the complete text of this
license.


Installation
------------

copy WacomTablet.usbfd to devs:usb/fd/

copy WacomTablet.fdclass to devs:usb/fdclasses/

Reboot or try C:USBCtrl RESTART to load the driver, then insert your tablet in
a usb slot.

Usage
-----

The driver has a configuration GUI, which is hidden by default, that works as a
commodity so it can be brought up by pressing 'ctrl alt w' or via Exchange.

The GUI is separated in four tabs, the last three allow to configure actions
associated to, respectively, the Pen, the Mouse and the Tablet. On the opposite,
the first tab is dedicated to the configuration of general options and display
of current information. At the top of the tab is the name of the currently
detected tablet. Below three parts are available: the left shows the current
position and pressure (in tablet coords) and below that the maximum values that
have occured during this session. The middle part is planed for future use. On
the right hand side are the current settings, the range of tablet coords that
are mapped onto the screen and the value of the pressure for maximum.

You can move the pen arround and press on the nib to find the maximum values,
then copy them to the settings, or choose a smaller area if you prefer. You
might want to do that if your screen has a different aspect to your tablet,
otherwise generally you will want the maximums.

On the right side, you can choose whether to send NEWTABLET (recommended) or the
old TABLET (for compatibility with old apps). (having allowed this option so far
most apps I've tried work with NEWTABLET.) Also you have the option to send
additional RAWMOUSE events, which enable some software such as popupmenu.class
(< 53.8) to see the pointer movements.

SAVE saves the settings and closes the window
USE uses them for the curent session only,
SET TO MAXIMUMS copies the max values recorded in that session for you.

The config is saved in the following preferences file: ENVARC:WacomPrefs.xml

Disclaimers
-----------

Configuration of action 'showkey' is currently considered 'beta', it may freeze
the machine if you try it.

Multitouch tablets cannot be supported right now because AmigaOS does not have
support for it. Once AmigaOS gets proper support for such feature, this driver
could be updated to support such tablets.

Supported tablet informations
-----------------------------

This driver supports both NewTablet event and old ones. Depending on the actual
capabilities of the tablet hooked up, the following information are supported.

All information from the old PointerTablet structure are suported:
 * X and Y Ranges
 * X and Y positions
 * Pressure (-128 to +127)

From the NewTablet the following information are supported:
 * X, Y and Z positions
 * X, Y and Z ranges
 * Pressure
 * X, Y and Z angle
 * Buttons bits
 * Proximity notification

Additionnaly this driver also supports sending a custom tag in order to indicate
which tool is actually being reported by the event. This is done by the tag
TABLETA_Tool whose value is currently (TABLETA_Dummy + 0x10B)
Possible values for this tag are :
    0 BTN_TOOL_PEN: the stylus using its usual nib,
    1 BTN_TOOL_RUBBER: the stylus using its opposite end,
    2 BTN_TOOL_BRUSH: a special stylus (or nib) representing a brush
    3 BTN_TOOL_PENCIL: a special stylus (or nib) representing pencil
    4 BTN_TOOL_AIRBRUSH: a special stylus (or nib) reprensting airbrush
    5 BTN_TOOL_FINGER: a single finger touch on a touchable part of the tablet 
    6 BTN_TOOL_MOUSE: a compatible mouse
    7 BTN_TOOL_LENS: use the lens
    8 BTN_TOOL_DOUBLETAP: a double finger touch on touchable parts of the tablet
    9 BTN_TOOL_TRIPLETAP: a triple finger touch on touchable parts of the tablet



Tested machines and OSes
------------------------

Any AmigaOS4 supported machines should be ok, anyway it's been formally reported
to work on:

- AmigaOne XE/G4, AmigaOS 4.1 update 4+
- AmigaOne X1000, AmigaOS 4.1 beta
- Sam 440, AmigaOS 4.1 update 4+
- Sam Flex 440, AmigaOS 4.1 update 4+
- Sam 460, AmigaOS 4.1 beta
- A1222, AmigaOS 4.1 beta

Compatible Tablets (possibly)
-----------------------------

The following Tablets are reported to work with the same driver as my tablet on
linux, that means that there is a reasonable chance they'll work with this driver
or they may not. No guarentees....

The following have been reported to work on Amiga Forums:

Bamboo MTE-450A (my tablet)
Graphire ET-405-U (Davebraco)
Volito FT-405-U (Davebraco)
Bamboo Fun4x6 CTE-450 (mr2)
Bamboo One CTF-430 (Nick Clover)
Intuos3 9x12 (mbrantley)
Intuos2 12x12 XD-1212-U (Martin J.)
Saphire CTE-430 (Davebraco)

These are reported to work on linux with the same driver as my tablet uses:

Wacom Penpartner
Wacom Graphire
Wacom Graphire2 4x5, 5x7
Wacom Graphire3
Wacom Graphire3 6x8
Wacom Graphire4 4x5, 6x8
Wacom BambooFun 4x5, 6x8
Wacom Bamboo1 Medium
Wacom Volito
Wacom PenStation2
Wacom Volito2 4x5, 2x3
Wacom PenPartner2
Wacom Bamboo
Wacom Bamboo 2FG, 4x5, Craft
Wacom Bamboo1
Wacom BambooFun 2FG 6x8, 2FG 4x5
Wacom Intuos 4x5, 6x8, 9x12, 12x12, 12x18
Wacom PL400, PL500, PL510, PL550, PL600, PL600SX, PL700, PL800
Wacom DTU710
Wacom DTF521, DTF720, DTF720a
Wacom Cintiq Partner, 21UX, 20WSX, 12WX, 21UX2
Wacom Intuos2 4x5, 6x8, 9x12, 12x12, 12x18
Wacom Intuos3 4x5, 4x6, 6x8, 6x11, 9x12, 12x12, 12x19
Wacom Intuos4 4x6, 6x9, 8x13, 12x19, WL
Wacom DTU1931
Wacom ISDv4 90, 93, 9A, 9F, E2, E3

History
-------

 1.2    2019-xx-xx  - Added: support for IntuosHT2 range of tablets (i.e.
                             Intuos S 2, Intuos PT S 2, Intuos P M 2 and
                             Intuos PT M 2)
                    - Added: explicit log for unsupported multitouch
                             tablets and mentionned it in the readme
                    - Updated: makefile updated to conform new repo layout
                    - Fixed: crash when trying to initialize unsupported
                             params (yet), many models were affected, mainly
                             Intuos and Cintiq (reported by Remo Constantin)

 1.1    2019-11-03  - Updated: code updated to input-wacom 0.44
                               (PenPartner, DTU, DTUS, DTH1152, PL, PTU,
                                Bamboo pen & touch, graphire)

 Intern 2015-02-xx  - Added: support for Intuos Pro tablets
                    - Added: support for Bamboo One tablets
                    - Added: support for Cintiq 13HD tablets
                    - Updated: code updated to input-wacom 0.21.0
                    - Fixed: random crash in prefs program after multiple
                             attach/detach cycles
                    - Fixed: mouse button would never get released
                    - Added: error message in case of commodity creation
                             failure
                    - Fixed: used prefsobjects interface version 1
                    - Added: Initial action handler

 Intern 2013-03-xx  - Added: Icons by Davebraco
                    - Fixed: Some compilation warnings
                    - Fixed: Incorrect button count displayed in prefs GUI
                    - Fixed: Warning about bad application.library version
                             tag used when saving prefs
                    - Updated: code updated to input-wacom 0.16.0
                    - Fixed: Potential crash in prefs program due to
                             unterminated label list
                    - Fixed: Replaced deprecated functions by new ones
                    - Added: Proper version handling mechanism

 v1.0 - 2012-08-01  - Added: tablet buttons configurability
 (Public release)   - Added: debuglevel env variable load
                    - Added: tablet capabilities autodetection
                    - Added: XML preferences file

 v0.6 - 08/05/2012  - Fixed: scroll events were only allowing forward scroll
                    - Fixed: wrong Project for prefs program icon
                    - Added: mouse and stylus button configurability

 v0.5 - 24/04/2012  - Added: extracted tablet handlers to a dedicated file
                    - Added: extracted supported tablet tables to own file
                    - Added: a prefs program that shows the GUI (if running)

 v0.4 - 19/04/2012  - Added: support for Intuos style tablets
                    - Fixed: some warnings
                    - Added: support for horizontal wheel
                    - Added: support for 4th and 5th button in mouse event
                    - Added: support for tilt around X, Y and Z in tablet

 v0.3 - 16/04/2012  - Fixed: expunge crash
                    - Fixed: absolute wheel handling
                    - Added: support for Bamboo Pen & Touch
                    - Added: support for tool identifying tag

 v0.2 - 14/04/2012  - Added: more tablets support
                    - Added: configurable debug level
                    - Fixed: Pressure handling now works correctly
                    - Fixed: Commodities hotkey is "ctrl alt w" now
                    - Added: Commodities uses tablet name as broker name

 v0.1 - 11/04/2012  - First working version.


Donations
---------

If this works for you, I'd like some feedback!

And if you are really pleased consider a donation to Andy as without his tablet
driver as sample I wouldn't have tried to implement this one.

See his donation button on: www.broad.ology.org.uk/amiga/


