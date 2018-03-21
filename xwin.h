/*
 * Copyright (c) 2017-2018 Ioannis Nompelis
 * See "LICENSE" for terms of use
 */

#ifndef _XWIN_H_
#define _XWIN_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Xmu/WinUtil.h>
#include <X11/XWDFile.h>


struct my_xwin_vars {
   Display *xdisplay;
   int xscreen;
   Window xroot;
   Window xwindow;
};



int xwindow_setup( struct my_xwin_vars *xvars,
                   int width, int height, int xpos, int ypos );

#endif

