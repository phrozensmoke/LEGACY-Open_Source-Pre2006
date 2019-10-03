/*****************************************************************************
 * gyach.h
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


#ifndef _GYACH_H_
#define _GYACH_H_

/* This OS detection code is from yahoonix, a chat client by jalapeno_tekk */
#if defined(__FreeBSD__) || defined(__bsdi__) || defined(__386BSD__) || \
    defined(__NetBSD__) || defined(__OpenBSD__)
#	define OS_BSD
#elif defined(__linux__)
#	define OS_LINUX
#elif defined(__sun__)
#	define OS_SOLARIS
#elif defined(SVR4) || defined(SYSV)
#	define OS_SYSV
#elif defined(__CYGWIN__)
#	define OS_WINDOWS
#else
#	warning Unsure about host OS type. Continuing anyway.
#endif

#include "config.h"  /* Added by PhrozenSmoke */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* *BSD systems supposedly need this for regex.h */
#ifdef OS_BSD
#	include <sys/types.h>
#endif

/* CygWin/Windows systems */
#ifdef OS_WINDOWS
#	include <w32api/windef.h>
#endif

#include <regex.h>

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "yahoochat.h"

/* Added by PhrozenSmoke */
/* Return a User-Agent similar to the one returned by libyahoo2 */
/* of form Mozilla/4.5 [en] (" PACKAGE "/" VERSION ")  */
/*  #define GYACH_USER_AGENT "Mozilla/4.5 [en] (" PACKAGE "/" VERSION ")"   */

/* changed URL referrer, pretend to be the 'official' Yahoo Messenger 
   for Linux client, PhrozenSmoke - help avoid standing out as a 'third party' program */

#define GYACH_USER_AGENT "Mozilla/5.0 (compatible; Yahoo! Messenger 1.0.4; Linux 2.4.19-4GB i686)"


#define GYACH_EMAIL		"PhrozenSmoke ['at'] yahoo.com"
#define GYACH_AUTHOR	"PhrozenSmoke"
#define GYACH_URL		"http://phpaint.sourceforge.net/pyvoicechat/"

#define GYACH_EMAIL_OLD		"cpinkham@infi.net, cpinkham@bc2va.org"
#define GYACH_URL_OLD		"http://www4.infi.net/~cpinkham/gyach/"

#define AVATAR_START	"<font ext !;"
#define AVATAR_END		";"
#define AVATAR_SEND_END	";>"

extern YMSG_SESSION ymsg_session, *ymsg_sess;

extern int debug_level;

extern GtkWidget * login_window;
extern GtkWidget * chat_window;
extern GtkWidget * chat_status;
extern GtkWidget * chat_entry;
extern GtkTreeView * chat_aliases;
extern GtkWidget * setup_menu;
extern GtkWidget * alias_menu;
extern GtkWidget * alias_window;
extern GtkWidget * setup_window;
extern GtkWidget * find_window;

extern guint st_cid;

extern time_t connect_time;

extern struct yahoo_context yahoo_ctx;

extern char *capture_filename;
extern FILE *capture_fp;

extern int  my_status;
extern time_t auto_away_time;
extern time_t child_timeout;

extern char *search_text;
extern int  search_pos;
extern int  search_case_sensitive;

extern int  logged_in;
extern int	def_user;
extern int	auto_away;
extern int	auto_login;
extern int	buffer_size;
extern int  use_bold;
extern int  use_underline;
extern int  use_italics;
extern int  font_size;
extern int  display_font_size;
extern int  force_lowercase;
extern int  ping_minutes;
extern int  chat_window_x;
extern int  chat_window_y;
extern int  chat_window_width;
extern int  chat_window_height;
extern int  chat_text_width;
extern int  chat_user_height;
extern int  remember_position;
extern int  remember_password;
extern int  limit_lfs;
extern int  chat_timestamp;
extern int  chat_timestamp_pm;
extern int  auto_reconnect;
extern int  use_proxy;
extern int  custom_color_red;
extern int  custom_color_green;
extern int  custom_color_blue;

extern char *auto_reply_msg;
extern char *use_color;
extern char *custom_away_message;
extern char *browser_command;
extern char *filter_command;
extern char *font_family;
extern char *font_name;
extern char *display_font_family;
extern char *display_font_name;
extern char *password;
extern char *def_room;

extern char username[];
extern char roomname[];

extern char cfg_dir[];
extern char GYACH_CFG_DIR[];
extern char GYACH_CFG_COMMON_DIR[];

extern char *proxy_host;
extern int  proxy_port;

extern int connected;
extern int valid_user;

extern GList *favroom_list;
extern GList *login_list;
extern GList *tab_user;

/* some support function prototypes */
void gyach_init();
void process_gtk_events();
void set_config();
void read_config();
void write_config();
void build_tab_complete_list();
void tab_complete();
void io_callback(gpointer data, gint source, GdkInputCondition condition);
int login_to_yahoo_chat( void );
void set_status_room_counts( void );
int check_clicked_text( char *text );
int search_chat_text();
#ifdef USE_GTK2
gchar *get_word_at_pos( GtkTextView *text_view, gint x, gint y );
#else
char *get_word_at_cursor( GtkText *text );
#endif
gint chat_motion_notify(GtkWidget * widget, GdkEventMotion * event);

/* wrappers for ymsg_* functions, these send packet & display locally also */
void gyach_comment( YMSG_SESSION *session, char *text );
void gyach_emote( YMSG_SESSION *session, char *text );
void gyach_think( YMSG_SESSION *session, char *text );
void gyach_away( YMSG_SESSION *session, char *text );
void gyach_back( YMSG_SESSION *session );


/*
 * DBG_LEVEL is defined at configuration time with the following:
 *    ./configure --enable-debug=DBG_LEVEL
 *
 * Turn ON debugging, levels are as follows:
 *   0 - none
 *   1 - main function calls
 *   2 - results of main functions
 * still working on
 *  11 - most function calls
 *  12 - results of most functions
 * and eventually
 *  21 - more function calls
 *  22 - results of all functions
 * and even later
 *  31 - all function calls
 *  32 - results of all functions
 * 201 - line numbers in lots of functions
 * 255 - ALL debug messages (including calls to process_gtk_events() function)
 */


/* debug macro */
#if DBG_LEVEL
/* DBG_LEVEL is defined so define the macro as normal */
#define DBG( level, msg, args... ) \
	do { \
		if ( DBG_LEVEL >= ( level )) { \
			fprintf( stderr, msg, ##args ); \
			fflush( stderr ); \
		} \
	} while( 0 )
#else
/* DBG_LEVEL is NOT defined so define the macro empty */
#define DBG( level, msg, args... )
#endif

#endif /* #ifndef _GYACH_H_ */
