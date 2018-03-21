/*
 * Copyright (c) 2017-2018 Ioannis Nompelis
 * See "LICENSE" for terms of use
 */

#ifndef _XROOT_STREAM_H_
#define _XROOT_STREAM_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <X11/Xmu/WinUtil.h>
#include <X11/XWDFile.h>


struct my_xvars {
   Display *xdisplay;
   Window xroot;
   int iscreen;
   int iplanes;
   unsigned int width, height;
   unsigned int iseq,jseq, imax;
   XImage *ximage;
   XImage **ximage_array;
   XImage **ximage_array2;
// char **ichange;
};


int attach_root( struct my_xvars *xvars,
                 char *display_name,
                 unsigned int imax );

int refresh_root( struct my_xvars * );

int dettach_root( struct my_xvars * );

int compare_ximage( XImage *ximg1, XImage *ximg2 );

int refresh_root_using( struct my_xvars *,
                        void func( unsigned int, void *arg ), void *arg );

#endif

