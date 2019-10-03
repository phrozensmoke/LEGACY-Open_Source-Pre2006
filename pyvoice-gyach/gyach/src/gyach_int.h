/*****************************************************************************
 * gyach_int.h
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
 * Copyright (C) 2003-2006, Erica Andrews
 * (Phrozensmoke ['at'] yahoo.com)
 * http://phpaint.sourceforge.net/pyvoicechat/
 * 
 * Copyright (C) 2000-2002 Chris Pinkham
 * cpinkham@corp.infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 * 
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * cpinkham@infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 *****************************************************************************/

#ifndef _GYACH_INT_H_
#define _GYACH_INT_H_

/* generic routines for things like toolbar & filling login window fields */
int build_toolbar( GtkWidget *chat_window );
GtkWidget *build_chat_window( void );
GtkWidget *build_login_window( void );
void set_config();
int set_menu_connected( int connected );
void set_setup_options();
void get_setup_options();

#define TRANS_BG
extern int trans_bg;

extern int highlight_friends;
extern int indent_wrapped;
extern GtkWidget *color_window;

extern int login_invisible;
extern int login_noroom;
extern char *last_loginname;
extern char *last_chatroom ;
extern char *last_chatserver ;
extern char *webcam_viewer_app;
extern char *webcam_device;
extern int support_scentral_smileys;
extern int popup_buzz;
extern int popup_buddy_on;
extern int popup_buddy_off;
extern int popup_new_mail;
extern int popup_news_bulletin;

extern GtkWidget *top_text;
extern GtkWidget *vpaned;
extern int transparent_bg;
extern int shading_r;
extern int shading_g;
extern int shading_b;

/* routines in gyach_int_*.c files */
int ct_build();
GtkWidget *ct_widget();
int ct_append( char *str, int len );
int ct_append_fixed( char *str, int len );
int ct_scroll_at_bottom();
int ct_scroll_to_bottom();
int ct_freeze();
int ct_thaw();
int ct_clear();
GdkColor *ct_get_color();
int ct_set_color( int r, int g, int b );

char *ct_get_font();
void ct_set_underline( int on );
int ct_set_font( char *new_font );
#	ifdef TRANS_BG
void ct_set_transparent( int trans );
void ct_set_transparent_shade( int r, int g, int b );
void ct_update_bg();
#	endif

int ct_set_max_lines( int lines );
int ct_capture_to_file( char *filename );
int ct_can_do_pixmaps( void );
int ct_append_pixmap( char *filename, GtkWidget *textbox );
void ct_set_indent( int indent_value );

#endif /* #ifndef _GYACH_INT_H_ */

