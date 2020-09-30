/*
 * Copyright (c) 2017-2018 Ioannis Nompelis
 * See "LICENSE" for terms of use
 */

#include <stdio.h>
#include <stdlib.h>

#include "xroot_stream.h"
#include "xwin.h"

// a dummy structure to use for passing arguments to the "using" function
struct dum_s {
   void *v1;
   void *v2;
};

// a function to use in the "using" function call
void nothing_func( unsigned int image_index,
                   int iloc,
                   int jloc,
                   unsigned int isize,
                   unsigned int jsize,
                   void *arg )
{
   struct dum_s *s;
   struct my_xvars *xvars;
   struct my_xwin_vars *xwinvars;
   XImage *xip;

   // play type-casting tricks to get to the X11 storage structures
   s = (struct dum_s *) arg;
   xvars = (struct my_xvars *) s->v1;           // X11 being monitored
   xwinvars = (struct my_xwin_vars *) s->v2;    // X11 displaying the window

   // address the sub-image by its index
   xip = ( xvars->ximage_array[ image_index ] );
   // display the image in the window
   XPutImage( xwinvars->xdisplay, xwinvars->xwindow,
               DefaultGC( xwinvars->xdisplay, xwinvars->xscreen ), xip,
               0, 0, iloc, jloc,  isize, jsize );
}

int main( int argc, char** argv ) {
   static struct my_xvars xvars;
   static struct my_xwin_vars xwinvars;
   static struct dum_s s;        // will use for bridging
   void *data_arg_ptr = NULL;    // will use to pass the argument to "using"

   // assign pointers for the "using" function callback
   s.v1 = (void *) &xvars;
   s.v2 = (void *) &xwinvars;
   data_arg_ptr = (void *) &s;


   (void) attach_root( &xvars, (char *) NULL, 100 );
   //dump_tecplot_ximage( xvars.ximage );

   // For now default to leaching everything and displaying a small section
   // but allow for the whole X server to be displayed. (If the default is to
   // display teh whole thing, this will freak people out if they launch the
   // code on the local X server!)
   if( argc > 1 ) {
      (void) xwindow_setup( &xwinvars, 
                            (int) xvars.width, 
                            (int) xvars.height, 10, 10 );
   } else {
      (void) xwindow_setup( &xwinvars, 800, 500, 10, 10 );
   }

while(1==1) {
// (void) refresh_root( &xvars );
   (void) refresh_root_using( &xvars, nothing_func, data_arg_ptr );
   //dump_tecplot_ximage( xvars.ximage_array[0] );
}
   (void) dettach_root( &xvars );

   return(0);
}


