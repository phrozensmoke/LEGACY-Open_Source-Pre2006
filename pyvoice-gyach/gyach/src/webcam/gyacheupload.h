/* Header for the Socket-reading and image
   conversion module for the GyachE-Webcam viewer program */

/* This program borrows alot of code from both Ayttm and 
    Gyach-E itself, however, this program is a completely 
    restructured webcam viewer that handles sockets and threads
    in the Gyach-E way, not the Ayttm way of many threads, many 
    structs, etc.  It is designed for simplicity, speed, 
    memory-friendliness, and stability: It runs as an EXTERNAL 
    program to Gyach Enhanced, so that if it DOES crash, it 
    crashes ALONE, rather than taking down an entire chat program
    with it. It is a clean, efficient SINGLE-THREADED application */

/*****************************************************************************
 * gyachewebcam.h
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
 * Copyright (C) 2003-2006 Erica Andrews (Phrozensmoke ['a'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <malloc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <signal.h>
#include<time.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlib.h>
#include <gdk-pixbuf-xlib/gdk-pixbuf-xlibrgb.h>



/* Todo: Make these configurable in GYachE, is this needed? */
#define YAHOO_WEBCAM_HOST "webcam.yahoo.com"
#define YAHOO_WEBCAM_PORT 5100

/* Todo: Make these configurable in GYachE, if the need arises */
/* connection type */
#define YCAM_DIALUP 0
#define YCAM_DSL 1
#define YCAM_T1 2

extern char *CAM_VERSION;
extern char webcam_description[41];
extern char *local_address;
extern char *whoami;
extern char *webcam_to_view;
extern char *webcam_key ;

extern char *second_server;
extern int cam_app_quit;
extern char packet_data[2050];
extern unsigned int packet_type;
extern unsigned long packet_size;
extern unsigned int packet_reason;
extern unsigned int packet_timestamp;
extern int camsocket;
extern int webcam_connected;
extern int image_need_update;
extern int video_paused;
extern GdkPixbuf *current_pixbuf;
extern GtkWidget *current_image;
extern GtkWidget *dialogwin;
extern int cam_app_quit;
extern int exit_on_error;
extern int cam_is_open;
extern int hue, contrast, brightness, colour;
extern int fix_color;
extern char *friends_list;

extern void show_webcam_packet();
extern void show_ok_dialog(char *mymsg);
extern void show_error_dialog(char *mymsg);
extern void set_local_addresses();
extern int create_webcam_socket( char *webcamhost, int webcamport) ;
extern u_char *webcam_header(u_char *buf, int packtype );
extern int webcam_recv_data( );
extern int webcam_send_packet(  u_char *bufp, int size, int headlen);
extern void show_webcam_packet();
extern void set_dummy_image_start();
extern void set_dummy_image();
extern void gyache_image_window_new(int width, int height, const char *title);
extern char *_(char *some);  
extern void mem_cleanup();
extern void start_connection() ;
extern int webcam_sock_send( u_char *buf, int size );
extern void  start_cam();
extern void update_cam();
extern void  set_video_device(char *myvdev);
extern unsigned char * image_2_jpg(char *in_img, int size);
extern void set_vid_properties(int thue, int tcontrast, int tcolor, int tbrightness, int tfc);
extern void lower_str( char *str );
extern int find_viewer_row( char *user );
extern void add_viewer( char *user );
extern void remove_viewer( char *user );
extern void remove_all_viewers();
extern GtkWidget *create_viewer_panel();
extern void  update_viewer_list();
extern void cleanup_v4l();
extern void webcam_viewer_connect(char *cwho, int connect);
extern void yahoo_webcam_accept_viewer(char* who, int accept);
extern void webcam_send_image(unsigned char *image, unsigned int length);


