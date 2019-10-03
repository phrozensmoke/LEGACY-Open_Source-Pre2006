/* Main module for the GyachE-Broadcaster program:
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
 * main.c
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

# include "gyacheupload.h"

#include <signal.h>
#include <time.h>
#  include <libintl.h>  /* added PhrozenSmoke: locale support */
#include <locale.h>  /* added PhrozenSmoke: locale support */

extern GtkWidget *vstatuslabel;
int cam_app_quit=0;
unsigned long sock_sleep=250;

/* FOR TESTING */ /* 
int jpfswitch=0; 
 */

char *_(char *some)   /* added PhrozenSmoke: locale support */
{
	return gettext(some);
}

void SIGPIPE_Handler (int signo) {
   if (signo == SIGPIPE) {
	char gbuf[255]="";
	snprintf(gbuf, 254, "%s\n%s",_("You have been disconnected from Yahoo!: Broken pipe."), _("You have been disconnected from the Yahoo webcam server."));
	show_ok_dialog(gbuf);
	mem_cleanup();
   							 }
}


/* FOR TESTING */ /*  
void jasp_im_test()  {
	FILE *jpf;
	int jpflen=0;
	char myjpf[255]="";

	if (jpfswitch==0) {
		snprintf(myjpf, 252, "%s",  "/tmp/john_last65.jp2");
						  } else {
		snprintf(myjpf, 252, "%s",  "/tmp/sjs329.jp2");
									}

    jpf=fopen(myjpf, "rb");
    if (!jpf) {return; }
    fseek(jpf, 0, SEEK_END);
    jpflen=ftell(jpf);
	packet_size=jpflen;
    fseek(jpf, 0, SEEK_SET);
    fread(packet_data, 1, jpflen, jpf);
    fclose(jpf);

	if (!jpfswitch) {jpfswitch=1;} else {jpfswitch=0;}
	image_need_update=1;
}
 */  
/* End: FOR TESTING */





int main( int argc, char **argv ) {
	struct timeval tv;
	fd_set set;
	int ret;
	char *vid_device=NULL;
	char titlebuf[81]="";

	/* Takes 3-4 command-line arguments right now:
		[your_screename], string
		[web_cam_key_from_gyachE], string
		[v4l_device] - ignored for now, int
		[optional_comma_separated_friends_list]- string
	*/
		
	if (argc<4) {
		printf("\n   Gyach Enhanced External Webcam Broadcaster\n   Version:  %s\n   License:  GNU General Public License\n   Copyright (c) 2004 Erica Andrews\n   PhrozenSmoke ['at'] yahoo.com\n\n   Usage:\n   gyache-upload [your_screename] [web_cam_key_from_gyachE] [v4l_device] [optional_comma_separated_friends_list]\n\n   You should be running this\n   program in Gyach Enhanced,\n   NOT on the command-line!\n\n", CAM_VERSION);
		fflush(stdout);
		exit(0);
				  }

	whoami=strdup(argv[1]);
	webcam_to_view=whoami;
	webcam_key=strdup(argv[2]);
	vid_device=strdup(argv[3]);

	if (argc>4) {
		friends_list=strdup(argv[4]);
		/* printf("\nFriends List: %s\n", friends_list); fflush(stdout);  */
				}

	setlocale(LC_ALL, "");
	setlocale (LC_NUMERIC, "C");
	bindtextdomain (PACKAGE, LOCALEDIR);  
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain (PACKAGE);
	gtk_set_locale();

	gtk_init( &argc, &argv );

	set_video_device(vid_device);

	/* init GUI */
	snprintf(titlebuf, 80, "%s: %s", webcam_to_view, _("Webcam"));
	gyache_image_window_new(320, 240, titlebuf);
	gtk_label_set_text(GTK_LABEL(vstatuslabel), _("Not Broadcasting"));

	/* Trap possible broken pipe crap with msg dialog instead of 
	    abrupt exiting of the app */

  if (signal(SIGPIPE, SIGPIPE_Handler) == SIG_ERR) {  }

	gdk_threads_enter();	

		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

	 /*  disabled: sometimes you want to adjust the cam first */	
	/* start_connection() ;  */

	while( ! cam_app_quit ) {
		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

		/* jasp_im_test();   */   /* for TESTING */

			if (! video_paused) {
				if (cam_is_open) 	{
					update_cam();
					gtk_widget_show_all(current_image);
											   }
											}


		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

		/* now check for input on our socket */
		FD_ZERO( &set );
		if (webcam_connected) {
		if ( camsocket != -1 ) {
			FD_SET( camsocket, &set );

			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			ret = select( camsocket + 1, &set, NULL, NULL, &tv );

			if ( ret ) {
				show_webcam_packet();
				usleep(sock_sleep);   /* a little flow control */
						}
										} /* end if */
											} /* end if */
												else {usleep(sock_sleep); } 
													}  /* end while */

	mem_cleanup();
	if (vid_device) {free(vid_device); vid_device=NULL;}
	if (friends_list) {free(friends_list); friends_list=NULL;}
	return( 0 );
}

