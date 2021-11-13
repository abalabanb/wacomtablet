/*
 * Comodities support GUI
 *
 * Copyright 2012-2021 Alexandre Balaban <amiga(-@-)balaban(-.-)fr>
 * Copyright 2011 Andy Broad <andy@broad.ology.org.uk>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   - Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   - Neither the names of Alexandre Balaban or Andy Broad nor the names
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

#include "WacomTablet.h"

#define PREFS_DICT_RANGE "Range"
    #define PREFS_KEY_TOP_X "TopX"
    #define PREFS_KEY_TOP_Y "TopY"
    #define PREFS_KEY_RANGE_X "RangeX"
    #define PREFS_KEY_RANGE_Y "RangeY"
    #define PREFS_KEY_RANGE_P "RangeP"
#define PREFS_DICT_EVENTS "Events"
    #define PREFS_KEY_EVENT_TYPE "EventType"
    #define PREFS_KEY_SWITCH_BUTS "SwitchButtons"
    #define PREFS_KEY_RAWMOUSE "SendRawMouse"
#define PREFS_DICT_CURVES "Curves"
#define PREFS_ARRAY_ACTION_MAPPING "ActionMapping"
    #define PREFS_KEY_ACTION "Action"
    #define PREFS_KEY_PARAMETER "Parameter"

const LONG tabLabelIDs[] = { MSG_TAB_MISC_TITLE, MSG_TAB_PEN_TITLE, MSG_TAB_MOUSE_TITLE, MSG_TAB_TABLET_TITLE, 0 };
CONST_STRPTR tabLabels[sizeof(tabLabelIDs)] = {NULL};
const LONG buttonActionLabelIDs[] = {
    MSG_ACTION_NONE,
    MSG_ACTION_CLICK,
    MSG_ACTION_MIDDLE_CLICK,
    MSG_ACTION_RIGHT_CLICK,
    MSG_ACTION_4TH_CLICK,
    MSG_ACTION_5TH_CLICK,
    MSG_ACTION_DOUBLE_CLICK,
    MSG_ACTION_HOLD_CLICK,
    MSG_ACTION_SHOWKEY,
//    MSG_ACTION_SWITCH_MODE,
//    MSG_ACTION_QUALIFIER,
//    MSG_ACTION_KEY,
//    MSG_ACTION_RUN,
//    MSG_ACTION_DEFAULT,
    0
 };
CONST_STRPTR buttonActionLabels[sizeof(buttonActionLabelIDs)] = {NULL};


BOOL OpenClasses(struct usbtablet *um)
{
    int i = 0;
    while(tabLabelIDs[i])
    {
        DebugLog(40, um, "translating, i=%d tabLabels[i]=%d", i, tabLabelIDs[i]);
        tabLabels[i] = GetString(&um->localeInfo, (LONG)tabLabelIDs[i]);
        i++;
    }
    i = 0;
    while(buttonActionLabelIDs[i])
    {
        buttonActionLabels[i] = GetString(&um->localeInfo, (LONG)buttonActionLabelIDs[i]);
        i++;
    }

    if(!(um->WindowClassLib = um->IIntuition->OpenClass("window.class",0,&um->WindowClassPtr)))
    {
        return FALSE;
    }

    if(!(um->LayoutClassLib = um->IIntuition->OpenClass("gadgets/layout.gadget",0,&um->LayoutClassPtr)))
    {
        return FALSE;
    }

    if(!(um->IntegerClassLib = um->IIntuition->OpenClass("gadgets/integer.gadget",0,&um->IntegerClassPtr)))
    {
        return FALSE;
    }


    if(!(um->ButtonClassLib = um->IIntuition->OpenClass("gadgets/button.gadget",0,&um->ButtonClassPtr)))
    {
        return FALSE;
    }

    if(!(um->LabelClassLib = um->IIntuition->OpenClass("images/label.image",0,&um->LabelClassPtr)))
    {
        return FALSE;
    }

    if(!(um->SpaceClassLib = um->IIntuition->OpenClass("gadgets/space.gadget",0,&um->SpaceClassPtr)))
    {
        return FALSE;
    }


    if(!(um->CheckboxClassLib = um->IIntuition->OpenClass("gadgets/checkbox.gadget",0,&um->CheckboxClassPtr)))
    {
        return FALSE;
    }
    
    if(!(um->SliderClassLib = um->IIntuition->OpenClass("gadgets/slider.gadget",0,&um->SliderClassPtr)))
    {
        return FALSE;
    }    

    if(!(um->StringClassLib = um->IIntuition->OpenClass("gadgets/string.gadget",0,&um->StringClassPtr)))
    {
        return FALSE;
    }

    if(!(um->ClickTabClassLib = um->IIntuition->OpenClass("gadgets/clicktab.gadget",0,&um->ClickTabClassPtr)))
    {
        return FALSE;
    }

    if(!(um->ChooserClassLib = um->IIntuition->OpenClass("gadgets/chooser.gadget",0,&um->ChooserClassPtr)))
    {
        return FALSE;
    }

    return TRUE;
}

VOID CloseClasses(struct usbtablet *um)
{
    um->IIntuition->CloseClass(um->WindowClassLib);
    um->IIntuition->CloseClass(um->LayoutClassLib);
    um->IIntuition->CloseClass(um->IntegerClassLib);
    um->IIntuition->CloseClass(um->ButtonClassLib);
    um->IIntuition->CloseClass(um->LabelClassLib);
    um->IIntuition->CloseClass(um->SpaceClassLib);
    um->IIntuition->CloseClass(um->CheckboxClassLib);
    um->IIntuition->CloseClass(um->SliderClassLib);
    um->IIntuition->CloseClass(um->StringClassLib);
    um->IIntuition->CloseClass(um->ClickTabClassLib);
    um->IIntuition->CloseClass(um->ChooserClassLib);
}

BOOL OpenWindow(struct usbtablet *um)
{

    if(um->FirstOpen)
    {
        if(OpenClasses(um))
        {
            CreateWindow(um);
            um->FirstOpen = FALSE;
        }
        else
        {
            CloseClasses(um);
        }
    }

    if(um->Window && (um->win == NULL))
    {
        if(um->CON)
        {
            um->IDOS->FPrintf(um->CON,"Opening Window\n");
        }

        um->win = (struct Window *)um->IIntuition->IDoMethod(um->Window,WM_OPEN,NULL);
        if(um->win)
        {
            um->IIntuition->GetAttr(WINDOW_SigMask,um->Window,&(um->winsigflag));
            SetGadgets(um);

            return TRUE;
        }
    }

    return FALSE;

}

VOID CloseWindow(struct usbtablet * um)
{
    if(um->CON)
    {
        um->IDOS->FPrintf(um->CON,"Closing Window\n");
    }

    if(um->win)
    {
         um->IIntuition->IDoMethod(um->Window, WM_CLOSE, NULL);
         um->win = NULL;
    }

}

VOID DisposeWindow(struct usbtablet *um)
{
    CloseWindow(um);
    um->IIntuition->DisposeObject(um->Window);
    um->Window = NULL;
}

Object *CreateWindow(struct usbtablet *um)
{
    if (um->Window)
    {
        DisposeWindow(um);
    }

    DebugLog(35, um, "buttonCapabilities is %016llx\n", um->buttonCapabilities);

    um->windowLayout = (struct Gadget *)um->IIntuition->NewObject(um->LayoutClassPtr,NULL,
        LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
        LAYOUT_SpaceOuter,  TRUE,
        LAYOUT_DeferLayout, TRUE,

        LAYOUT_AddChild, um->gadgets[GID_CLICKTAB] = (struct Gadget *)um->IIntuition->NewObject(um->ClickTabClassPtr, NULL,
            GA_ID, GID_CLICKTAB,
            GA_RelVerify, TRUE,
            GA_Text, tabLabels,

            CLICKTAB_PageGroup, um->IIntuition->NewObject(NULL, "page.gadget",
                /* We will defer layout/render changing pages! */
                LAYOUT_DeferLayout, TRUE,

                PAGE_Add, um->IIntuition->NewObject(um->LayoutClassPtr,NULL,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_SpaceOuter,  TRUE,
                    LAYOUT_DeferLayout, TRUE,

                    LAYOUT_AddChild, um->IIntuition->NewObject(um->StringClassPtr, NULL,
                        GA_ReadOnly,TRUE,
                        STRINGA_TextVal, um->features->name,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_DETECTED),
                        TAG_END),
                        
                    LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                        LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                        LAYOUT_SpaceOuter,  TRUE,
                        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,

                            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                            LAYOUT_SpaceOuter,  TRUE,
                            LAYOUT_BevelStyle,BVS_GROUP,
                            LAYOUT_Label, GetString(&um->localeInfo, MSG_TAB_MISC_REPORT_VALUES),
            
                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddImage, um->IIntuition->NewObject(um->LabelClassPtr, NULL,
                                    LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_CURRENT),
                                    TAG_END),

                                LAYOUT_AddImage, um->IIntuition->NewObject(um->LabelClassPtr, NULL,
                                    LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_MAX),
                                    TAG_END),
                                TAG_END), // title Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, "",
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddChild, um->gadgets[GID_CURRENTX] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_CURRENTX,
                                        INTEGER_Arrows,FALSE,
                                        GA_ReadOnly,TRUE,
                                    TAG_END),

                                LAYOUT_AddChild, um->gadgets[GID_MAXX] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_MAXX,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),
                                TAG_END), // X Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_X),
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddChild, um->gadgets[GID_CURRENTY] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_CURRENTY,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),
                                LAYOUT_AddChild, um->gadgets[GID_MAXY] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_MAXY,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),
                                TAG_END), // Y Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_Y),
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddChild, um->gadgets[GID_CURRENTP] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_CURRENTP,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),

                                LAYOUT_AddChild, um->gadgets[GID_MAXP] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_MAXP,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),
                                TAG_END), // Pressure Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_P),
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddChild, um->gadgets[GID_CURRENTTILTX] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_CURRENTTILTX,
                                        INTEGER_Arrows,FALSE,
                                        GA_ReadOnly,TRUE,
                                    TAG_END),
                                LAYOUT_AddChild, um->IIntuition->NewObject(um->SpaceClassPtr, NULL, TAG_END),
                                TAG_END), // TiltX Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_TILT_X),
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,

                                LAYOUT_AddChild, um->gadgets[GID_CURRENTTILTY] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                        GA_ID, GID_CURRENTTILTY,
                                        GA_ReadOnly,TRUE,
                                        INTEGER_Arrows,FALSE,
                                    TAG_END),
                                LAYOUT_AddChild, um->IIntuition->NewObject(um->SpaceClassPtr, NULL, TAG_END),
                                TAG_END), // TiltY Layout
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_TILT_Y),
                                TAG_END),

                            LAYOUT_AddChild, um->IIntuition->NewObject(um->SpaceClassPtr, NULL, TAG_END),
                            TAG_END), // Current values layout
            
                        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                            LAYOUT_SpaceOuter,  TRUE,
                            /*LAYOUT_BevelStyle,BVS_GROUP,
                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                                LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                                LAYOUT_SpaceOuter,  TRUE,
                                LAYOUT_BevelStyle,BVS_GROUP,
                                LAYOUT_Label, GetString(&um->localeInfo, MSG_TAB_MISC_PRESSURE_TEST),
                
                                LAYOUT_AddChild, um->gadgets[GID_PTEST] = (struct Gadget *)um->IIntuition->NewObject(um->SpaceClassPtr, NULL,
                                        GA_ID, GID_PTEST,
                                        GA_RelVerify,TRUE,
                                    TAG_END),
                                TAG_END),*/
                            LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                                LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                                LAYOUT_SpaceOuter,  TRUE,
                                LAYOUT_BevelStyle,BVS_GROUP,
                                LAYOUT_Label, GetString(&um->localeInfo, MSG_TAB_MISC_PRESSURE_CURVE),
                                LAYOUT_AddChild,um->gadgets[GID_PCURVE0] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[0],
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE0,
                                        TAG_END),
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE1] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[1],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE1,
                                        TAG_END),                            
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE2] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,
                                            SLIDER_Level,um->Curve[2],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE2,
                                        TAG_END),
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE3] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[3],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE3,
                                        TAG_END),
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE4] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[4],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE4,
                                        TAG_END),              
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE5] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[5],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE5,
                                        TAG_END),            
                                 LAYOUT_AddChild,um->gadgets[GID_PCURVE6] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,100,
                                            SLIDER_Invert,TRUE,                                
                                            SLIDER_Level,um->Curve[6],                                
                                            SLIDER_Orientation,SORIENT_VERT,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_PCURVE6,
                                        TAG_END),                                                                                                                                              
                                TAG_END), // layout
                            TAG_END),  // layout
                        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                            LAYOUT_SpaceOuter,  TRUE,
                            LAYOUT_BevelStyle,BVS_GROUP,
                            LAYOUT_Label, GetString(&um->localeInfo, MSG_TAB_MISC_CURRENT_SETTINGS),
                            LAYOUT_AddChild, um->gadgets[GID_TOPX] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                    GA_ID, GID_TOPX,
                                    INTEGER_Number,(ULONG)um->TopX,
                                TAG_END),
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_TOP_X),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_TOPY] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                    GA_ID, GID_TOPY,
                                    INTEGER_Number,(ULONG)um->TopY,
                                TAG_END),
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_TOP_Y),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_RANGEX] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                    GA_ID, GID_RANGEX,
                                    INTEGER_Number,(ULONG)um->RangeX,
                                TAG_END),
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_RANGE_X),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_RANGEY] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                    GA_ID, GID_RANGEY,
                                    INTEGER_Number,(ULONG)um->RangeY,
                                TAG_END),
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_RANGE_Y),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_RANGEP] = (struct Gadget *)um->IIntuition->NewObject(um->IntegerClassPtr, NULL,
                                    GA_ID, GID_RANGEP,
                                    INTEGER_Number,(ULONG)um->RangeP,
                                TAG_END),
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                                LABEL_Text, GetString(&um->localeInfo, MSG_TAB_MISC_RANGE_P),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_NEWTABLET] = (struct Gadget *)um->IIntuition->NewObject(um->CheckboxClassPtr, NULL,
                                    GA_ID, GID_NEWTABLET,
                                    GA_Text, GetString(&um->localeInfo, MSG_USE_NEWTABLET),
                                    GA_Selected,(ULONG)(um->EventType & USE_NEWTABLET),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_TABLET] = (struct Gadget *)um->IIntuition->NewObject(um->CheckboxClassPtr, NULL,
                                    GA_ID, GID_TABLET,
                                    GA_Text, GetString(&um->localeInfo, MSG_USE_TABLET),
                                    GA_Selected,(ULONG)(um->EventType & USE_TABLET),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_SWITCH] = (struct Gadget *)um->IIntuition->NewObject(um->CheckboxClassPtr, NULL,
                                    GA_ID, GID_SWITCH,
                                    GA_Text, GetString(&um->localeInfo, MSG_SWITCH_BUTTONS),
                                    GA_Selected,(ULONG)(um->SwitchButtons),
                                TAG_END),
                            LAYOUT_AddChild, um->gadgets[GID_RAWMOUSE] = (struct Gadget *)um->IIntuition->NewObject(um->CheckboxClassPtr, NULL,
                                    GA_ID, GID_RAWMOUSE,
                                    GA_Text, GetString(&um->localeInfo, MSG_SEND_RAWMOUSE),
                                    GA_Selected,(ULONG)(um->SendRawMouse),
                                TAG_END),
            
                            TAG_END), //layout
                        TAG_END), // layout

                    LAYOUT_AddChild, um->gadgets[GID_DEBUGLEVEL] = (struct Gadget *)um->IIntuition->NewObject(um->SliderClassPtr,NULL,
                                            SLIDER_Min,0,
                                            SLIDER_Max,50,
                                            SLIDER_Invert,FALSE,                                
                                            SLIDER_Level,um->debugLevel,                                
                                            SLIDER_Orientation,SORIENT_HORIZ,
                                            GA_RelVerify,TRUE,
                                            GA_ID,GID_DEBUGLEVEL,
                                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_DEBUG_LEVEL),
                        TAG_END),
                    TAG_END), // Misc Page layout

                PAGE_Add, um->IIntuition->NewObject(um->LayoutClassPtr,NULL,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_SpaceOuter,  TRUE,
                    LAYOUT_DeferLayout, TRUE,
                    GA_Disabled,        0 ==((um->toolCapabilities)&FLAG(BTN_TOOL_PEN)),

                    LAYOUT_AddChild, um->gadgets[GID_TOUCH] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_TOUCH,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_TOUCH].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_NIB_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_STYLUS] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_STYLUS,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_STYLUS].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_FIRST_STYLUS_BUTTON_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_STYLUS2] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_STYLUS2,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_STYLUS2].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_SECOND_STYLUS_BUTTON_ACTION),
                        TAG_END),
                    TAG_END), // Pen Page Layout

                PAGE_Add, um->IIntuition->NewObject(um->LayoutClassPtr,NULL,
                    LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                    LAYOUT_SpaceOuter,  TRUE,
                    LAYOUT_DeferLayout, TRUE,
                    GA_Disabled,        0 ==((um->toolCapabilities)&FLAG(BTN_TOOL_MOUSE)),

                    LAYOUT_AddChild, um->gadgets[GID_LEFT] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_LEFT,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_LEFT].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_LEFT_BUTTON_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_RIGHT] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_RIGHT,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_RIGHT].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_RIGHT_BUTTON_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_MIDDLE] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_MIDDLE,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_MIDDLE].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_MIDDLE_BUTTON_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_4TH] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_4TH,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_SIDE].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_4TH_BUTTON_ACTION),
                        TAG_END),

                    LAYOUT_AddChild, um->gadgets[GID_5TH] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL,
                        GA_ID,               GID_5TH,
                        GA_RelVerify,        TRUE,
                        CHOOSER_LabelArray,  buttonActionLabels,  // array of strings
                        CHOOSER_Selected,    um->buttonAction[BTN_EXTRA].ba_action,
                        GA_Underscore,       0,
                        TAG_END),
                    CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,
                        LABEL_Text, GetString(&um->localeInfo, MSG_5TH_BUTTON_ACTION),
                        TAG_END),
                    TAG_END), // Mouse Page Layout

                PAGE_Add, um->IIntuition->NewObject(um->LayoutClassPtr,NULL,
                    LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
                    LAYOUT_SpaceOuter,  TRUE,
                    LAYOUT_DeferLayout, TRUE,

                        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,
            
                            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                            LAYOUT_SpaceOuter,  TRUE,
                            
                            #define AddButtonAction(i) \
                            LAYOUT_AddChild, um->gadgets[GID_BTN_0+i] = (struct Gadget *)um->IIntuition->NewObject(um->ChooserClassPtr,NULL, \
                                GA_ID,               GID_BTN_0+i,                                                                            \
                                GA_RelVerify,        TRUE,                                                                                   \
                                CHOOSER_LabelArray,  buttonActionLabels,                                                                     \
                                CHOOSER_Selected,    um->buttonAction[BTN_0+i].ba_action,                                                    \
                                GA_Underscore,       0,                                                                                      \
                                GA_Disabled,         (0 ==((um->buttonCapabilities)&FLAG(BTN_0+i))?TRUE:FALSE),                              \
                                TAG_END),                                                                                                    \
                            CHILD_Label,um->IIntuition->NewObject(um->LabelClassPtr,NULL,                                                    \
                                LABEL_Text, GetString(&um->localeInfo, MSG_1ST_BUTTON_ACTION+i),                                             \
                                TAG_END)                                                                                                     \

                            AddButtonAction(0),
                            AddButtonAction(1),
                            AddButtonAction(2),
                            AddButtonAction(3),
                            AddButtonAction(4),
                            AddButtonAction(5),
                            AddButtonAction(6),
                            AddButtonAction(7),
                            AddButtonAction(8),
                            AddButtonAction(9),
                            AddButtonAction(10),
                            AddButtonAction(11),
                            TAG_END),

                        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,

                            LAYOUT_Orientation, LAYOUT_ORIENT_VERT,
                            LAYOUT_SpaceOuter,  TRUE,

                            AddButtonAction(12),
                            AddButtonAction(13),
                            AddButtonAction(14),
                            AddButtonAction(15),
                            AddButtonAction(16),
                            AddButtonAction(17),
                            AddButtonAction(18),
                            AddButtonAction(19),
                            AddButtonAction(20),
                            AddButtonAction(21),
                            AddButtonAction(22),
                            AddButtonAction(23),
                            TAG_END),
                    TAG_END), // Tablet Page Layout

                TAG_END), // page group

            TAG_END), // clicktab

        LAYOUT_AddChild, um->IIntuition->NewObject(um->LayoutClassPtr, NULL,

            LAYOUT_Orientation, LAYOUT_ORIENT_HORIZ,
            LAYOUT_SpaceOuter,  TRUE,
            LAYOUT_AddChild, um->gadgets[GID_SAVE] = (struct Gadget *)um->IIntuition->NewObject(um->ButtonClassPtr, NULL,
                    GA_ID,GID_SAVE,
                    GA_RelVerify,TRUE,
                    GA_Text, GetString(&um->localeInfo, MSG_SAVE),
                TAG_END),
            LAYOUT_AddChild, um->gadgets[GID_SETTOMAX] = (struct Gadget *)um->IIntuition->NewObject(um->ButtonClassPtr, NULL,
                    GA_ID,GID_SETTOMAX,
                    GA_RelVerify,TRUE,
                    GA_Text, GetString(&um->localeInfo, MSG_SET_MAXIMUMS),
                TAG_END),
            LAYOUT_AddChild, um->gadgets[GID_USE] = (struct Gadget *)um->IIntuition->NewObject(um->ButtonClassPtr, NULL,
                    GA_ID,GID_USE,
                    GA_RelVerify,TRUE,
                    GA_Text, GetString(&um->localeInfo, MSG_USE),
                TAG_END),
            TAG_END),
        CHILD_WeightedHeight,0,
    TAG_END);




    um->Window = um->IIntuition->NewObject(um->WindowClassPtr,NULL,
                        WA_CloseGadget,TRUE,
                        WA_DepthGadget,TRUE,
                        WA_SizeGadget,TRUE,
                        WA_DragBar,TRUE,
                        WA_InnerHeight,200,
                        WA_InnerWidth,400,
                        WA_Title,(ULONG)GetString(&um->localeInfo,MSG_MAIN_TITLE),
                        WA_IDCMP,IDCMP_RAWKEY|IDCMP_MENUPICK,
                        WINDOW_Position,WPOS_CENTERSCREEN,
                        WINDOW_Layout, um->windowLayout,
                        WA_Activate,TRUE,
                        TAG_DONE);

    return um->Window;
}

void SetValues(struct usbtablet *um)
{
    ULONG attr;

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_TOPX],&attr))
    {
        um->TopX = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_TOPY],&attr))
    {
        um->TopY = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_RANGEX],&attr))
    {
        um->RangeX = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_RANGEY],&attr))
    {
        um->RangeY = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_RANGEP],&attr))
    {
        um->RangeP = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(GA_Selected,(Object *)um->gadgets[GID_NEWTABLET],&attr))
    {
        if((BOOL)attr)
        {
        um->EventType  |= USE_NEWTABLET;
        }
        else
        {
            um->EventType  &= ~USE_NEWTABLET;
        }

    }
    if (um->IIntuition->GetAttr(GA_Selected,(Object *)um->gadgets[GID_TABLET],&attr))
    {
        if((BOOL)attr)
        {
        um->EventType  |= USE_TABLET;
        }
        else
        {
            um->EventType  &= ~USE_TABLET;
        }

    }
    if (um->IIntuition->GetAttr(GA_Selected,(Object *)um->gadgets[GID_SWITCH],&attr))
    {
        if((BOOL)attr)
        {
        um->SwitchButtons  = TRUE;
        }
        else
        {
        um->SwitchButtons  = FALSE;

        }

    }
    if (um->IIntuition->GetAttr(GA_Selected,(Object *)um->gadgets[GID_RAWMOUSE],&attr))
    {
        if((BOOL)attr)
        {
        um->SendRawMouse  = TRUE;
        }
        else
        {
        um->SendRawMouse  = FALSE;

        }

    }
    
    int nGID = GID_TOUCH;
    // handle stylus buttons
    for(; nGID <= GID_STYLUS2; nGID++)
    { 
        if (um->IIntuition->GetAttr(CHOOSER_Selected,(Object *)um->gadgets[nGID],&attr))
        {
            DebugLog(10,um,"Setting stylus %ld to %ld\n", BTN_TOUCH-(nGID-GID_TOUCH), attr);
            um->buttonAction[BTN_TOUCH-(nGID-GID_TOUCH)].ba_action = attr;
        }
    }
    // handle mouse buttons
    for(nGID = GID_LEFT; nGID <= GID_5TH; nGID++)
    { 
        if (um->IIntuition->GetAttr(CHOOSER_Selected,(Object *)um->gadgets[nGID],&attr))
        {
            DebugLog(10,um,"Setting mouse %ld to %ld\n", BTN_LEFT-(nGID-GID_LEFT), attr);
            um->buttonAction[BTN_LEFT-(nGID-GID_LEFT)].ba_action = attr;
        }
    }
    // handle tablet buttons
    for(nGID = GID_BTN_0; nGID <= GID_BTN_FORWARD; nGID++)
    {
        if (um->IIntuition->GetAttr(CHOOSER_Selected,(Object *)um->gadgets[nGID],&attr))
        {
            DebugLog(10,um,"Setting tablet %ld to %ld\n", nGID-GID_BTN_0, attr);
            um->buttonAction[nGID-GID_BTN_0].ba_action = attr;
        }
    }
    // handle pressure curve settings
    for(nGID = GID_PCURVE0; nGID <= GID_PCURVE6; nGID++)
    {
        if (um->IIntuition->GetAttr(SLIDER_Level,(Object *)um->gadgets[nGID],&attr))
        {
            um->Curve[nGID-GID_PCURVE0] = attr;
        }
    }

    if (um->IIntuition->GetAttr(SLIDER_Level,(Object *)um->gadgets[GID_DEBUGLEVEL],&attr))
    {
        um->debugLevel = attr;
    }
}

void LoadValues(struct usbtablet *um)
{
  PrefsObject *pPrefs = NULL;
  ULONG nErr;
  LONG nVal;
  
  DebugLog(0, um, "Loading prefs");

  if(NULL != (pPrefs = um->IPrefsObjects->PrefsDictionary(NULL, NULL, ALPO_Alloc, 0, TAG_DONE)))
  {
    if(!(nErr = um->IPrefsObjects->ReadPrefs(pPrefs,
        READPREFS_FileName, "ENVARC:WacomDriver.xml",
        READPREFS_ReadENV, TRUE,
        READPREFS_ReadENVARC, TRUE, 
        TAG_DONE)))
    {
        PrefsObject *pDict = NULL;
        // Events
        if(NULL != (pDict = um->IPrefsObjects->DictGetObjectForKey(pPrefs, PREFS_DICT_EVENTS)))
        {
            um->SwitchButtons = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_SWITCH_BUTS, 0);
            um->SendRawMouse = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_RAWMOUSE, 0);
            nVal = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_EVENT_TYPE, -1);
            if(-1 != nVal)
            {
                um->EventType = nVal;
            }
        }
       // Range
        if(NULL != (pDict = um->IPrefsObjects->DictGetObjectForKey(pPrefs, PREFS_DICT_RANGE)))
        {
            um->TopX = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_TOP_X, 0);
            um->TopY = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_TOP_Y, 0);
            nVal = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_RANGE_X, -1);
            if(-1 != nVal)
            {
                um->RangeX = nVal;
            }
            nVal = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_RANGE_Y, -1);
            if(-1 != nVal)
            {
                um->RangeY = nVal;
            }
            nVal = um->IPrefsObjects->DictGetIntegerForKey(pDict, PREFS_KEY_RANGE_P, -1);
            if(-1 != nVal)
            {
                um->RangeP = nVal;
            }
        }
        PrefsObject *pArray = NULL;
        // Curves
        if(NULL != (pArray = um->IPrefsObjects->DictGetObjectForKey(pPrefs, PREFS_DICT_CURVES)))
        {
          LONG nCount;
          struct ALPOObjIndex iIdx;

          um->IPrefsObjects->PrefsArray(pArray, NULL, ALPOARR_GetCount, &nCount, TAG_DONE);
          for(iIdx.index = 0; iIdx.index < nCount; iIdx.index++)
          {
            um->IPrefsObjects->PrefsArray(pArray, NULL, ALPOARR_GetObjAtIndex, &iIdx, TAG_DONE);
            if(iIdx.obj)
            {
              um->IPrefsObjects->PrefsNumber(iIdx.obj, &nErr, ALPONUM_GetLong, &nVal, TAG_DONE);
              if(!nErr)
              {
                  um->Curve[iIdx.index] = nVal;
              }
              else     um->IDOS->Printf("??? ");
            }
          }
        }
        else DebugLog(0, um, "Unable to find Curves configuration");
        // ActionMapping
        if(NULL != (pArray = um->IPrefsObjects->DictGetObjectForKey(pPrefs, PREFS_ARRAY_ACTION_MAPPING)))
        {
          LONG nCount;
          struct ALPOObjIndex iIdx;

          um->IPrefsObjects->PrefsArray(pArray, NULL, ALPOARR_GetCount, &nCount, TAG_DONE);
          for(iIdx.index = 0; iIdx.index < nCount; iIdx.index++)
          {
            um->IPrefsObjects->PrefsArray(pArray, NULL, ALPOARR_GetObjAtIndex, &iIdx, TAG_DONE);
            if(iIdx.obj)
            {
                nVal = um->IPrefsObjects->DictGetIntegerForKey(iIdx.obj, PREFS_KEY_ACTION, -1);
                if( -1 != nVal )
                {
                    um->buttonAction[iIdx.index].ba_action = nVal;
                    DebugLog(40, um, "action for button %d=%d", iIdx.index, nVal);
                }
                um->buttonAction[iIdx.index].ba_parameter = um->IPrefsObjects->DictGetStringForKey(iIdx.obj, PREFS_KEY_PARAMETER, NULL);
            }
          }
        }
        else DebugLog(0, um, "Unable to find Action mapping configuration");
      }
      else DebugLog(0, um, "Unable to read conf file (err $%lx)", nErr);

      um->IPrefsObjects->PrefsDictionary(pPrefs, NULL, ALPO_Release, 0, TAG_DONE);
    }
}

void SaveValues(struct usbtablet *um)
{
    PrefsObject *pPrefsDict, *pObj;
       
    pPrefsDict = um->IPrefsObjects->PrefsDictionary(NULL, NULL, ALPO_Alloc, 0, TAG_DONE);
    
    // -- Ranges
    pObj = um->IPrefsObjects->PrefsDictionary(NULL, NULL, ALPO_Alloc, 0, TAG_DONE);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->TopX,
                            TAG_DONE),
               PREFS_KEY_TOP_X);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->TopY,
                            TAG_DONE),
               PREFS_KEY_TOP_Y);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->RangeX,
                            TAG_DONE),
               PREFS_KEY_RANGE_X);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->RangeY,
                            TAG_DONE),
               PREFS_KEY_RANGE_Y);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->RangeP,
                            TAG_DONE),
               PREFS_KEY_RANGE_P);
    um->IPrefsObjects->DictSetObjectForKey(pPrefsDict, pObj, PREFS_DICT_RANGE);
    // -- Events
    pObj = um->IPrefsObjects->PrefsDictionary(NULL, NULL, ALPO_Alloc, 0, TAG_DONE);    
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetLong, um->EventType,
                            TAG_DONE),
               PREFS_KEY_EVENT_TYPE);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetBool, um->SwitchButtons,
                            TAG_DONE),
               PREFS_KEY_SWITCH_BUTS);
    um->IPrefsObjects->DictSetObjectForKey(pObj, 
               um->IPrefsObjects->PrefsNumber( NULL, NULL,
                            ALPONUM_AllocSetBool, um->SendRawMouse,
                            TAG_DONE),
               PREFS_KEY_RAWMOUSE);
    um->IPrefsObjects->DictSetObjectForKey(pPrefsDict, pObj, PREFS_DICT_EVENTS);
    // -- Curves
    pObj = um->IPrefsObjects->PrefsArray(NULL, NULL, 
               ALPO_Alloc, 0, 
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[0],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[1],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[2],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[3],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[4],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[5],
                                           TAG_DONE),
               ALPOARR_AddObj, (uint32)um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->Curve[6],
                                           TAG_DONE),
               TAG_DONE);
    um->IPrefsObjects->DictSetObjectForKey(pPrefsDict, pObj, PREFS_DICT_CURVES);
    // -- Action Mapping
    pObj = um->IPrefsObjects->PrefsArray(NULL, NULL, ALPO_Alloc, 0, TAG_DONE);
    
    int nIndex = 0;
    for(nIndex = 0; nIndex < 32; nIndex++)
    {
       PrefsObject *pSubDict = um->IPrefsObjects->PrefsDictionary(NULL, NULL, ALPO_Alloc, 0, TAG_DONE);
       
       um->IPrefsObjects->DictSetObjectForKey(pSubDict, 
                um->IPrefsObjects->PrefsNumber(NULL, NULL,
                                           ALPONUM_AllocSetLong, um->buttonAction[nIndex].ba_action,
                                           TAG_DONE),
                PREFS_KEY_ACTION);
       um->IPrefsObjects->DictSetObjectForKey(pSubDict, 
                um->IPrefsObjects->PrefsString(NULL, NULL,
                                           ALPOSTR_AllocSetString, um->buttonAction[nIndex].ba_parameter,
                                           TAG_DONE),
                PREFS_KEY_PARAMETER);
       
       um->IPrefsObjects->PrefsArray(pObj, NULL, ALPOARR_AddObj, pSubDict, TAG_DONE);
    }
    um->IPrefsObjects->DictSetObjectForKey(pPrefsDict, pObj, PREFS_ARRAY_ACTION_MAPPING);

    um->IPrefsObjects->WritePrefs( pPrefsDict, 
       WRITEPREFS_FileName, "ENVARC:WacomDriver.xml",
       WRITEPREFS_WriteENV, TRUE,
       WRITEPREFS_WriteENVARC, TRUE,
       TAG_DONE);
    
    um->IPrefsObjects->PrefsDictionary( pPrefsDict, NULL, ALPO_Release, 0, TAG_DONE );
}


void SetGadgets(struct usbtablet *um)
{
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_TOPX],um->win,NULL,INTEGER_Number,(ULONG)um->TopX,TAG_DONE);
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_TOPY],um->win,NULL,INTEGER_Number,(ULONG)um->TopY,TAG_DONE);
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_RANGEX],um->win,NULL,INTEGER_Number,(ULONG)um->RangeX,TAG_DONE);
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_RANGEY],um->win,NULL,INTEGER_Number,(ULONG)um->RangeY,TAG_DONE);
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_RANGEP],um->win,NULL,INTEGER_Number,(ULONG)um->RangeP,TAG_DONE);
    um->IIntuition->SetGadgetAttrs(um->gadgets[GID_DEBUGLEVEL],um->win,NULL,SLIDER_Level,(ULONG)um->debugLevel,TAG_DONE);

}

void SetToMaxs(struct usbtablet *um)
{

    ULONG attr;

    um->TopX = 0;

    um->TopY = 0;

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_MAXX],&attr))
    {
        um->RangeX = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_MAXY],&attr))
    {
        um->RangeY = (uint16)attr;
    }

    if (um->IIntuition->GetAttr(INTEGER_Number,(Object *)um->gadgets[GID_MAXP],&attr))
    {
        um->RangeP = (uint16)attr;
    }

    SetGadgets(um);
}

BOOL HandleWindowInput(struct usbtablet *um)
{
    ULONG result;
    UWORD code;

    while((result = um->IIntuition->IDoMethod(um->Window, WM_HANDLEINPUT, &code)) != WMHI_LASTMSG )
    {
        switch(result & WMHI_CLASSMASK)
        {
            case WMHI_CLOSEWINDOW:
                CloseWindow(um);
                break;

            case WMHI_GADGETUP:

                switch(result & WMHI_GADGETMASK)
                {
                    case GID_USE:
                        /* Use the currently defined values but don't save */
                        SetValues(um);
                        CloseWindow(um);
                        break;
                    case GID_SAVE:
                        SetValues(um);
                        SaveValues(um);
                        CloseWindow(um);
                        break;
                    case GID_SETTOMAX:
                        SetToMaxs(um);
                        break;
                }
        }

    }
    return TRUE;
}
