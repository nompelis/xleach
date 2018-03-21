/*
 * Copyright (c) 2017-2018 Ioannis Nompelis
 * See "LICENSE" for terms of use
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "xroot_stream.h"


void dump_tecplot_ximage( XImage *ximage )
{
   FILE *fp;
   int i,j;

   fp=fopen("grab.dat","w");
   fprintf(fp,"variables = x y R G B mag \n");
   fprintf(fp,"zone T=\"image\", i=%d, j=%d, f=point \n",
       ximage->width, ximage->height);

   for(j=0; j < ximage->height; ++j) {
      for(i=0; i < ximage->width; ++i) {
         unsigned long pixel = XGetPixel( ximage, i, j );
         unsigned char uc[3];
            uc[0] = (unsigned char) (pixel >> 16);
            uc[1] = (unsigned char) ((pixel & 0x00ff00) >> 8);
            uc[2] = (unsigned char) (pixel & 0x0000ff);
         fprintf(fp," %d %d    %d %d %d  %lf \n",
            i, ximage->height - j,
            uc[0],uc[1],uc[2],
            sqrt(
                 ((double) uc[0]) * ((double) uc[0]) +
                 ((double) uc[1]) * ((double) uc[1]) +
                 ((double) uc[2]) * ((double) uc[2])
                )
         );
      }
   }

   fclose(fp);
}


int attach_root( struct my_xvars *xvars, char *display_name, unsigned int imax )
{
   XWindowAttributes win_attr;
// int num_color;
// XColor xcolors;
   unsigned int irem,jrem, i,j, nsub;
   char iok = 0;

//----- connect to server and get window attributes
   if( display_name == NULL ) {
       xvars->xdisplay = XOpenDisplay( ":0" );
   } else {
       xvars->xdisplay = XOpenDisplay( display_name );
   }
   if( xvars->xdisplay == NULL ){
      fprintf(stderr, "could not connect to display %s \n", display_name );
      return(1);
   }

   xvars->iscreen = DefaultScreen( xvars->xdisplay );
   xvars->iplanes = DisplayPlanes( xvars->xdisplay, xvars->iscreen );
   xvars->xroot = DefaultRootWindow( xvars->xdisplay );

   printf(" Display ptr: %p, root: %ld screen: %d planes: %d \n",
          xvars->xdisplay, xvars->xroot, xvars->iscreen, xvars->iplanes);

   XGetWindowAttributes( xvars->xdisplay, xvars->xroot, &win_attr );
   xvars->width = win_attr.width;
   xvars->height= win_attr.height;

   printf(" Root window size: %d %d position: %d %d \n",
           win_attr.width, win_attr.height, win_attr.y, win_attr.y);

// num_color = Get_XColors( xvars->xdisplay, &win_attr, &xcolors );

//----- get the snapshot of the root window and create an image
   xvars->ximage = XGetImage( xvars->xdisplay,
                              xvars->xroot,     // Drawable
                              0, 0, win_attr.width, win_attr.height,
                              AllPlanes, ZPixmap );
   if(xvars->ximage == NULL) {
      fprintf(stderr, "Could not obtain an image of the root window \n");
      return(2);
   }

   printf(" Image width: %d height: %d depth: %d byte_order %d bits/pixel %d\n",
            xvars->ximage->width,
            xvars->ximage->height,
            xvars->ximage->depth,
            xvars->ximage->byte_order,
            xvars->ximage->bits_per_pixel );


//----- create subimages from the snapshot
   if( imax == 0 ) imax = 100;
   xvars->imax = imax;

   xvars->iseq = xvars->width / imax;
   xvars->jseq = xvars->height / imax;
   irem = xvars->width - xvars->iseq*imax;
   jrem = xvars->height - xvars->jseq*imax;
   if( irem > 0) ++(xvars->iseq);
   if( jrem > 0) ++(xvars->jseq);
printf("Sequencing: %d %d \n", xvars->iseq, xvars->jseq);//HACK

   nsub = xvars->iseq*xvars->jseq;
   xvars->ximage_array = (XImage **) malloc((size_t) nsub*2*sizeof(XImage *));
   if( xvars->ximage_array == NULL ) {
      dettach_root( xvars );
      return(3);
   }

   jrem = xvars->height;
   for(j=0; j < xvars->jseq; ++j) {
      unsigned int iheight = imax;

      if( j == xvars->jseq-1 ) iheight = jrem;

      irem = xvars->width;
      for(i=0; i < xvars->iseq; ++i) {
         unsigned int iwidth = imax;
         unsigned int n = j * xvars->iseq + i;
         XImage *xip;

         if( i == xvars->iseq-1 ) iwidth = irem;

         xip = XSubImage( xvars->ximage,
                  (int) (i*imax), (int) (j*imax), iwidth, iheight );
         xvars->ximage_array[n] = xip;
         if( xip == NULL ) iok = 1;

         xip = XSubImage( xvars->ximage,
                  (int) (i*imax), (int) (j*imax), iwidth, iheight );
         xvars->ximage_array[n + nsub] = xip;
         if( xip == NULL ) iok = 1;

         irem -= imax;
      }
      jrem -= imax;
   }

   if( iok != 0) {
       dettach_root( xvars );
       return(4);
   }

   return(0);
}



int dettach_root( struct my_xvars *xvars )
{
   if( xvars->ximage_array != NULL ) {
      unsigned int i,j,n,nsub;

      nsub = xvars->jseq * xvars->iseq;
      for(j=0; j < xvars->jseq; ++j) {
      for(i=0; i < xvars->iseq; ++i) {
         n = j * xvars->iseq + i;

         if( xvars->ximage_array[n] != NULL ) {
            XDestroyImage( xvars->ximage_array[n] );
         }

         if( xvars->ximage_array[n + nsub] != NULL ) {
            XDestroyImage( xvars->ximage_array[n + nsub] );
         }
      }}

      free( xvars->ximage_array );
      xvars->ximage_array = NULL;
   }

   if( xvars->ximage != NULL) XDestroyImage( xvars->ximage );
   xvars->ximage = NULL;

   xvars->width = -1;
   xvars->height = -1;
   xvars->iscreen = 0;
   xvars->iplanes = 0;

   XCloseDisplay( xvars->xdisplay );
   xvars->xdisplay = NULL;

   return(0);
}



int refresh_root( struct my_xvars *xvars )
{
   unsigned int irem,jrem, i,j, nsub, imax;

printf("Refreshing...\n");fflush(stdout);//HACK
   imax = xvars->imax;
   nsub = xvars->jseq * xvars->iseq;

   jrem = xvars->height;
   for(j=0; j < xvars->jseq; ++j) {
      unsigned int iheight = imax;

      if( j == xvars->jseq-1 ) iheight = jrem;

      irem = xvars->width;
      for(i=0; i < xvars->iseq; ++i) {
         unsigned int iwidth = imax;
         unsigned int n = j * xvars->iseq + i;
         XImage *xip;
         int iret;

         if( i == xvars->iseq-1 ) iwidth = irem;

      // xip = xvars->ximage_array[n];
         xip = xvars->ximage_array[n + nsub];
         XGetSubImage( xvars->xdisplay, xvars->xroot,
                       (int) (i*imax), (int) (j*imax), iwidth, iheight,
                       AllPlanes, ZPixmap, xip, 0, 0 );

         iret = compare_ximage( xvars->ximage_array[n],
                                xvars->ximage_array[n + nsub] );
         if( iret != 0) {
            XImage *tmp = xvars->ximage_array[n];
            xvars->ximage_array[n] = xip;
            xvars->ximage_array[n + nsub] = tmp;

            // code can be injected here to do something with the changed image

            printf("Changes: (%d, %d)   %d \n", i,j, iret );
         }

         irem -= imax;
      }
      jrem -= imax;
   }
printf("Done\n");fflush(stdout);//HACK

   return(0);
}



int compare_ximage( XImage *ximg1, XImage *ximg2 )
{
   int i,j;
   int icnt = 0;

   for(j=0; j < ximg1->height; ++j) {
      for(i=0; i < ximg1->width; ++i) {
         unsigned long pixel1 = XGetPixel( ximg1, i, j );
         unsigned long pixel2 = XGetPixel( ximg2, i, j );
/*       unsigned char uc[3];
         uc[0] = (unsigned char) (pixel2 >> 16);
         uc[1] = (unsigned char) ((pixel2 & 0x00ff00) >> 8);
         uc[2] = (unsigned char) (pixel2 & 0x0000ff);
*/
         if( pixel1 != pixel2 ) ++icnt;
      }
   }

   return( icnt );
}


int refresh_root_using( struct my_xvars *xvars,
                        void func( unsigned int, void * ), void *arg )
{
   unsigned int irem,jrem, i,j, nsub, imax;

printf("Refreshing...\n");fflush(stdout);//HACK
   imax = xvars->imax;
   nsub = xvars->jseq * xvars->iseq;

   jrem = xvars->height;
   for(j=0; j < xvars->jseq; ++j) {
      unsigned int iheight = imax;

      if( j == xvars->jseq-1 ) iheight = jrem;

      irem = xvars->width;
      for(i=0; i < xvars->iseq; ++i) {
         unsigned int iwidth = imax;
         unsigned int n = j * xvars->iseq + i;
         XImage *xip;
         int iret;

         if( i == xvars->iseq-1 ) iwidth = irem;

      // xip = xvars->ximage_array[n];
         xip = xvars->ximage_array[n + nsub];
         XGetSubImage( xvars->xdisplay, xvars->xroot,
                       (int) (i*imax), (int) (j*imax), iwidth, iheight,
                       AllPlanes, ZPixmap, xip, 0, 0 );

         iret = compare_ximage( xvars->ximage_array[n],
                                xvars->ximage_array[n + nsub] );
         if( iret != 0) {
            XImage *tmp = xvars->ximage_array[n];
            xvars->ximage_array[n] = xip;
            xvars->ximage_array[n + nsub] = tmp;

            // call a function to handle the situation
            (*func)( n, arg );

            printf("Changes: (%d, %d)   %d \n", i,j, iret );
         }

         irem -= imax;
      }
      jrem -= imax;
   }
printf("Done\n");fflush(stdout);//HACK

   return(0);
}

