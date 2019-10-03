/* V4L module for the GyachE-Broadcaster program:
   This application is used to send webcam streams to
    Yahoo. Right now this program has only been tested
    with a single Video4Linux device: An OV511 DLink
    C100 USB webcam.  The program uses Video4Linux-1
    for image capture and the libJasper library for Jpeg-2000
    image conversions.  */

/* This program borrows alot of code from both Ayttm and 
    Gyach-E itself, as well as the old 'videodog' program
    for a few decent V4L usage examples.

    It is designed for simplicity, speed, 
    memory-friendliness, and stability: It runs as an EXTERNAL 
    program to Gyach Enhanced, so that if it DOES crash, it 
    crashes ALONE, rather than taking down an entire chat program
    with it. It is a clean, efficient SINGLE-THREADED application 
*/

/*****************************************************************************
 * gyachewebcam.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 *
 * Copyright (C) 2003-2006 Erica Andrews (Phrozensmoke ['at'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *****************************************************************************/
/* 
	Videodog is Copyright (C) 2000, 
	Gleicon S. Moraes - gleicon@uol.com.br rde@linuxbr.com 

	Ayttm is  Copyright (C) 2003, the Ayttm team
	and Copyright (C) 1999-2002, Torrey Searle <tsearle@uci.edu>
*/

# include "gyacheupload.h"

/* ******************************* */
/*      PATCHES BELOW for webcams with OV519 sensors and 
	Logitech Quickcam for Notebooks Pro devices.  NOTE, These patches 
	could not be tested personally by the author */

/* CCVT patchs contributed by: 
	Natanael Copa  (OV519 patch)
	Date: 17 Mar 2005
	Email: n ['at'] tanael.org

	AND

	Mark Alexander  (CCVT/Logitech Quickcam for Notebooks Pro patch)
	Date: 7 Dec 2004
	Email: marka ['at'] pobox.com

*/
/* ******************************* */	

/* Uncomment the line below to enable contributed but UNTESTED patch to support
	cameras with OV519 chips - includes double-buffering support */

 /* #define USE_VIDEO_OV519 1  */


/* Uncomment the following definition to enable the use of the
 * ccvt library to convert YUV420P to RGB24, instead of the routines
 * in this file that were taken from ov511.c in the Linux kernsl.  The
 * ccvt ibrary can be found in the camstream package.  If enabled, 
 * modify the definitions of INCLUDE and gyach_LDADD in the Makefile
 * to refer to the ccvt source and built libraries.
 */
/* #define USE_CCVT 1 */

#ifdef USE_CCVT
#include "ccvt.h"
#endif

int exit_on_error=0;
int cam_is_open=0;
int x=320, y=240, w=3;
unsigned char *v_device; /* device */
 struct video_picture grab_pic;
 struct video_capability grab_cap;
 struct video_mmap grab_buf;

#ifdef USE_VIDEO_OV519
static struct video_mbuf mbuf;
#endif

 int grab_fd, grab_size;
 unsigned char *grab_data;
 char *pnm_buf=NULL;
 int pnm_size;
 unsigned char *rgb_buf;

extern int app_debugger;

/* Hard-coding my own favorite cam settings into here...
    everybody else can update theirs from the config window */

int fix_color=0;


#ifdef USE_VIDEO_OV519
int hue=32767, contrast=32767, brightness=32767, colour=32767;
#else
int hue=47104, contrast=65280, brightness=65280, colour=17152;
#endif

unsigned char* grab_one(int *, int *);
int grab_init();

void cleanup_v4l() {
	cam_is_open=0;
	if (v_device) {free (v_device); v_device=NULL;}
	if (pnm_buf) {free(pnm_buf); pnm_buf=NULL;}
	if (grab_fd>-1) {
		close (grab_fd);
		grab_fd=-1;
		munmap(grab_data, grab_size);
	}
}

void fix_colour(char *image, int x, int y)
{
   int i;
   char tmp;
   i = x * y;
   while(--i) {
      tmp = image[0];
      image[0] = image[2];
      image[2] = tmp;
      image += 3;
   }
}

void init_pnm_buf()
{
	char hdr_buf[50];
	int hdr_size;

	if (pnm_buf) {
		return;
	}

	/* The PNM header never changes, so create it only once. */
	hdr_size=sprintf(hdr_buf, "P6\n%d %d\n255\n", x, y);

	/* Allocate space for the header + the image. */
	pnm_size=grab_size+hdr_size;
	pnm_buf=malloc(pnm_size);
	if (!pnm_buf) {
		show_error_dialog("Not enough memory available.");
		return;
	}

	/* Copy the header into the buffer, and set rgb_buf to point
	 * just past the header.
	 */
	memcpy(pnm_buf, hdr_buf, hdr_size);
	rgb_buf = pnm_buf + hdr_size;
}

void update_cam()
{
   unsigned char *my_jasper=NULL;
   unsigned char *grabit=grab_one(&x, &y);

   if (!grabit) {
	show_error_dialog("There was an error reading the webcam image.");
	return;
	} 


#ifdef USE_VIDEO_OV519
 current_pixbuf=gdk_pixbuf_new_from_data(grabit,GDK_COLORSPACE_RGB,FALSE,8,x,y,x*grab_pic.depth/8,NULL,NULL);
#else
 current_pixbuf=gdk_pixbuf_new_from_data(grabit,GDK_COLORSPACE_RGB,FALSE,8,320,240,320*grab_pic.depth/8,NULL,NULL);
#endif


	gtk_image_set_from_pixbuf(GTK_IMAGE(current_image), current_pixbuf);
	gdk_pixbuf_unref(current_pixbuf);
	current_pixbuf=NULL;

		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

	/* create a PNM for Jasper, convert to jpeg-2000 and send */
	 if (webcam_connected && image_need_update) { 

	init_pnm_buf();
	if (rgb_buf != grabit) {
		memcpy(rgb_buf, grabit, grab_size);
	}
	if (app_debugger) {printf("Jasper-conversion-1\n"); fflush(stdout);}
	my_jasper= image_2_jpg(pnm_buf, pnm_size);

		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

	if (my_jasper) {
		if (app_debugger) {
			printf("Jasper-conversion-2..sendiing.\n"); 
			/* printf("Jasper-conversion-2..sending: %d\n", packet_size);  */ 
			fflush(stdout); }
	webcam_send_image(my_jasper, packet_size);
		if (app_debugger) {printf("Jasper-conversion-3..sent\n"); fflush(stdout); }
	free(my_jasper); 
	my_jasper=NULL;
						}
										 								} 	
}

/* signal handler */
void _sighandler (int sig) {
	switch (sig){			
		case SIGINT: /* ctrl+x */
			cleanup_v4l();
			show_error_dialog("Application interrupted: Shutting down.");
			break;
			}
}

void set_picture() {
	if (hue > -1) grab_pic.hue=hue;
	if (contrast > -1) grab_pic.contrast=contrast;
	if (brightness > -1) grab_pic.brightness=brightness;
	if (colour > -1) grab_pic.colour=colour;

#ifdef USE_VIDEO_OV519
	/* hardcoded... */
	grab_pic.depth = 24;
	grab_pic.palette = VIDEO_PALETTE_RGB24;
#endif

	if (ioctl(grab_fd, VIDIOCSPICT, &grab_pic) == -1) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCSPICT'.\nCould not set camera properties.");
		return; 
		}
}	
	
void set_video_device(char *myvdev) {
	v_device=strdup(myvdev);
}

void init_cam () {
	signal (SIGINT, _sighandler);
	if (!v_device) {
			show_error_dialog("No Video4Linux device was specified.");
			return;
			}
	if (strlen(v_device)<1) {
			show_error_dialog("No Video4Linux device was specified.");
			return;
			}
	grab_init();
	set_picture();
}


#ifdef USE_VIDEO_OV519
int grab_init() {
        if ((grab_fd = open(v_device,O_RDWR)) == -1 ) {
		show_error_dialog("Could not open Video4Linux device.\nThe device may already be in use.");
		return 0; 
		}
	if (ioctl(grab_fd,VIDIOCGCAP,&grab_cap) == -1) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCGCAP'.\nWrong device.");
		return 0; 
		}
	memset (&grab_pic, 0, sizeof(struct video_picture));
	snprintf(webcam_description, 3, "%s",  "");
	strncpy(webcam_description, grab_cap.name ,28);
	strcat(webcam_description, " V4L1");
	if (ioctl (grab_fd, VIDIOCGPICT, &grab_pic) == -1) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCGPICT'.");
		return 0; 
		}
	

	/* The line below cannot work: we must force use of RGB24 for 
	    PNM image creation */
	/* grab_buf.format=grab_pic.palette; */ 

	/* A V4L device supporting the VIDEO_PALETTE_RGB24
	   ( 24bit RGB) color palette is REQUIRED, Gdk-Pixbuf 
	   can only handle RGB Color spaces, so no support for 
	   greyscale palettes or cams using non-standard, weirdo palettes 
	   at this time */

	grab_buf.format = VIDEO_PALETTE_RGB24;
	grab_buf.frame  = 0;
	grab_buf.width  = x;
	grab_buf.height = y;
	grab_size = x * y * w;
	set_picture();

	if (ioctl (grab_fd, VIDIOCGMBUF, &mbuf) < 0) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCGMBUF'.");
		return 0; 
		}
	printf("frames per capture: %i\n", mbuf.frames);
	if ((grab_data = mmap(0, mbuf.size, PROT_READ|PROT_WRITE, MAP_SHARED,
						  grab_fd,0)) == MAP_FAILED) {
		perror("mmap");
		show_error_dialog("An error occurred at 'mmap'.");
		return 0;
	}

	/* grab first frame */
	if (-1 == ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf)) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCMCAPTURE'.");
		return 0; 
	}
	if (mbuf.frames > 1) grab_buf.frame = 1 - grab_buf.frame;

	return(1);
	}

#else

int grab_init() {
        if ((grab_fd = open(v_device,O_RDWR)) == -1 ) {
		show_error_dialog("Could not open Video4Linux device.\nThe device may already be in use.");
		return 0; 
		}
	if (ioctl(grab_fd,VIDIOCGCAP,&grab_cap) == -1) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCGCAP'.\nWrong device.");
		return 0; 
		}
	memset (&grab_pic, 0, sizeof(struct video_picture));
	snprintf(webcam_description, 3, "%s",  "");
	strncpy(webcam_description, grab_cap.name ,28);
	strcat(webcam_description, " V4L1");
	if (ioctl (grab_fd, VIDIOCGPICT, &grab_pic) == -1) {
		show_error_dialog("An error occurred at 'ioctl VIDIOCGPICT'.");
		return 0; 
		}
	

	/* The line below cannot work: we must force use of RGB24 for 
	    PNM image creation */
	/* grab_buf.format=grab_pic.palette; */ 

	/* A V4L device supporting the VIDEO_PALETTE_RGB24
	   ( 24bit RGB) color palette is REQUIRED, Gdk-Pixbuf 
	   can only handle RGB Color spaces, so no support for 
	   greyscale palettes or cams using non-standard, weirdo palettes 
	   at this time */

	grab_buf.format = VIDEO_PALETTE_RGB24;
	grab_buf.frame  = 0;
	grab_buf.width  = x;
	grab_buf.height = y;
	grab_size = x * y * w;
	grab_data = mmap(0,grab_size,PROT_READ|PROT_WRITE,MAP_SHARED,grab_fd,0);

	init_pnm_buf();
	return(1);
}

#endif
	

void set_vid_properties(int thue, int tcontrast, int tcolor, int tbrightness, int tfc) {
	hue=thue;
	contrast=tcontrast;
	colour=tcolor;
	brightness=tbrightness;
	fix_color=tfc;
	set_picture();
}

#ifndef USE_CCVT

/* The following conversion routines were taken from drivers/usb/ov511.c
 * in the Mandrake Linux 2.4.21 kernel.
 */

static int force_rgb = 1;

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void
move_420_block(int yTL, int yTR, int yBL, int yBR, int u, int v,
	       int rowPixels, unsigned char * rgb, int bits)
{
	const int rvScale = 91881;
	const int guScale = -22553;
	const int gvScale = -46801;
	const int buScale = 116129;
	const int yScale  = 65536;
	int r, g, b;

	g = guScale * u + gvScale * v;
	if (force_rgb) {
		r = buScale * u;
		b = rvScale * v;
	} else {
		r = rvScale * v;
		b = buScale * u;
	}

	yTL *= yScale; yTR *= yScale;
	yBL *= yScale; yBR *= yScale;

	if (bits == 24) {
		/* Write out top two pixels */
		rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
		rgb[2] = LIMIT(r+yTL);

		rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
		rgb[5] = LIMIT(r+yTR);

		/* Skip down to next line to write out bottom two pixels */
		rgb += 3 * rowPixels;
		rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
		rgb[2] = LIMIT(r+yBL);

		rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
		rgb[5] = LIMIT(r+yBR);
	} else if (bits == 16) {
		/* Write out top two pixels */
		rgb[0] = ((LIMIT(b+yTL) >> 3) & 0x1F)
			| ((LIMIT(g+yTL) << 3) & 0xE0);
		rgb[1] = ((LIMIT(g+yTL) >> 5) & 0x07)
			| (LIMIT(r+yTL) & 0xF8);

		rgb[2] = ((LIMIT(b+yTR) >> 3) & 0x1F)
			| ((LIMIT(g+yTR) << 3) & 0xE0);
		rgb[3] = ((LIMIT(g+yTR) >> 5) & 0x07)
			| (LIMIT(r+yTR) & 0xF8);

		/* Skip down to next line to write out bottom two pixels */
		rgb += 2 * rowPixels;

		rgb[0] = ((LIMIT(b+yBL) >> 3) & 0x1F)
			| ((LIMIT(g+yBL) << 3) & 0xE0);
		rgb[1] = ((LIMIT(g+yBL) >> 5) & 0x07)
			| (LIMIT(r+yBL) & 0xF8);

		rgb[2] = ((LIMIT(b+yBR) >> 3) & 0x1F)
			| ((LIMIT(g+yBR) << 3) & 0xE0);
		rgb[3] = ((LIMIT(g+yBR) >> 5) & 0x07)
			| (LIMIT(r+yBR) & 0xF8);
	}
}


/* Converts from planar YUV420 to RGB24. */
static void
yuv420p_to_rgb(unsigned char *pIn0, unsigned char *pOut0, int bits)
{
	const int numpix = grab_buf.width * grab_buf.height;
	const int bytes = bits >> 3;
	int i, j, y00, y01, y10, y11, u, v;
	unsigned char *pY = pIn0;
	unsigned char *pU = pY + numpix;
	unsigned char *pV = pU + numpix / 4;
	unsigned char *pOut = pOut0;

	for (j = 0; j <= grab_buf.height - 2; j += 2) {
		for (i = 0; i <= grab_buf.width - 2; i += 2) {
			y00 = *pY;
			y01 = *(pY + 1);
			y10 = *(pY + grab_buf.width);
			y11 = *(pY + grab_buf.width + 1);
			u = (*pU++) - 128;
			v = (*pV++) - 128;

			move_420_block(y00, y01, y10, y11, u, v,
				       grab_buf.width, pOut, bits);

			pY += 2;
			pOut += 2 * bytes;
		}
		pY += grab_buf.width;
		pOut += grab_buf.width * bytes;
	}
}

#endif	/* !USE_CCVT */



#ifdef USE_VIDEO_OV519

unsigned char* grab_one(int *width, int *height) {
	set_picture();

		if (-1 == ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf)) {
			show_error_dialog("An error occurred at 'ioctl VIDIOCMCAPTURE'.");
			return NULL; 
	}

	if (mbuf.frames > 1) grab_buf.frame = 1 - grab_buf.frame;
				if (-1 == ioctl(grab_fd,VIDIOCSYNC,&grab_buf)) {
					show_error_dialog("An error occurred at 'ioctl VIDIOCSYNC'.");
					return NULL; 
	}
	if (fix_color) {fix_colour(grab_data + mbuf.offsets[grab_buf.frame], x, y); }
				 	*width  = grab_buf.width;
				 	*height = grab_buf.height;
	return grab_data + mbuf.offsets[grab_buf.frame];
}

#else

unsigned char* grab_one(int *width, int *height) {
	char buf[128];
	int ret;

	set_picture();

	for (;;) {
		ret = ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf);
		if (ret == -1) {
			/* Some drivers (such as pwc) don't support capturing
			 * an RGB24 image, so try YUV420Pp.  If that works,
			 * we'll have to convert the image to RGB24 below.
			 */
			grab_buf.format = VIDEO_PALETTE_YUV420P;
			ret = ioctl(grab_fd,VIDIOCMCAPTURE,&grab_buf);
		}
		if (ret == -1) {
			sprintf(buf, "An error (%d) (%s) occurred at 'ioctl VIDIOCMCAPTURE'.",
				errno, strerror(errno));
			show_error_dialog(buf);
			return NULL; 
		} else {
			if (-1 == ioctl(grab_fd,VIDIOCSYNC,&grab_buf)) {
				show_error_dialog("An error occurred at 'ioctl VIDIOCSYNC'.");
				return NULL; 
			} else {
				if (fix_color) {fix_colour(grab_data, x, y); }
				*width  = grab_buf.width;
				*height = grab_buf.height;
				if (grab_buf.format == VIDEO_PALETTE_YUV420P) {
					/* Convert from YUV420P to RGB24. */
#ifdef USE_CCVT
					ccvt_420p_rgb24(grab_buf.width,
							grab_buf.height,
							grab_data,
							rgb_buf);
#else
					yuv420p_to_rgb(grab_data, rgb_buf, 24);
#endif
					return rgb_buf;
				}
				return grab_data;
			}
		}
		
	}
}

#endif



void  start_cam()
{
   current_image = gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG);
   init_cam();
   gtk_widget_set_usize(current_image,x,y);
}
