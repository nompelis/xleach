/*
 * Copyright (c) 2017-2018 Ioannis Nompelis
 * See "LICENSE" for terms of use
 */

#include <stdio.h>
#include <stdlib.h>

#include "xwin.h"


int xwindow_setup( struct my_xwin_vars *xvars,
                   int width, int height, int xpos, int ypos )
{
   int ierr;


   //
   // Initialize Xlib in multi-thread mode
   // (this seems to be necessary if mutliple threads will use Xlib)
   //
   ierr = XInitThreads();
   if(ierr == 0) {
      fprintf(stderr," e  Could not get Xlib to work in multi-thread mode\n");
      return(1);
   }

   //
   // make a connection to the X server
   // (we use the user's default display from the environment variable)
   //
   xvars->xdisplay = XOpenDisplay( getenv("DISPLAY") );
   if(xvars->xdisplay == NULL) {
      fprintf(stderr," e  Failed to open display.\n");
      return(2);
   }

   //
   // echo the number of screens on this X display
   //
   fprintf(stderr," i  The display has %d available screen(s)\n",
           ScreenCount( xvars->xdisplay ));

   //
   // get the default screen number (we will open our window in it) and get
   // the root window
   //
   xvars->xscreen = DefaultScreen( xvars->xdisplay );
   xvars->xroot   = RootWindow( xvars->xdisplay, xvars->xscreen );

   //
   // create a window
   //
   xvars->xwindow = XCreateSimpleWindow( xvars->xdisplay, xvars->xroot,
                                    (unsigned int) xpos, (unsigned int) ypos, 
                                    (unsigned int) width, (unsigned int) height,
                                    (unsigned int) 1,    // border width
                                    (unsigned long) 0x00000000,    // border
                                    (unsigned long) 0x00006600 );  // background
   if( xvars->xwindow <= 0 ) {
      fprintf(stderr," e  Could not create window\n");
      XCloseDisplay( xvars->xdisplay );
      return(3);
   }

   XStoreName( xvars->xdisplay, xvars->xwindow, (char *) "Xleach" );

   //
   // Specify events to be sent to the program from the window
   //
   XSelectInput( xvars->xdisplay, xvars->xwindow,
                ExposureMask |
                StructureNotifyMask |
                PointerMotionMask |
                ButtonPressMask |
                ButtonReleaseMask |
                KeyPressMask |
                KeyReleaseMask);

   //
   // Create the window and bring it up
   // (An X window can be created and not be displayed until needed.)
   //
   XMapWindow( xvars->xdisplay, xvars->xwindow );
   XMapRaised( xvars->xdisplay, xvars->xwindow );
   XFlush( xvars->xdisplay );


   return(0);
}

