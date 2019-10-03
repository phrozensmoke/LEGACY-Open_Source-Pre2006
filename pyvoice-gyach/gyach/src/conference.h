/*****************************************************************************
 * conference.h
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
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 * Phrozensmoke ['at'] yahoo.com
 *****************************************************************************/
/*
	Module added by Erica Andrews (PhrozenSmoke), support for private conferences 
	on Yahoo! - still considered experimental, but gets the job done: 
	start, join, invite people to, and send messages to conferences...also, 
	preliminary voice chat support.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>






extern char *last_conference;
extern GHashTable *conference_members;
extern GHashTable *conference_invites;
extern GHashTable *buddy_status;


extern int voice_enabled;
extern void voice_launch_conference(int with_warning, char *yuser, char *yroom);

extern GtkWidget *show_confirm_dialog_config(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel, int with_callback);
extern char *_(char *some) ;

extern int is_conference;  /* conference.c */
extern char *ymsg_field( char *key );  /* main.c */


extern char *get_conference_who();
extern char *get_conference_msg();
extern char *get_conference_room();
extern char *get_conference_error();
extern char *get_conference_members();
extern char *get_conference_host();
extern void populate_conference_list(char *plist);
extern void clear_conference_list();
extern void start_conf_voice_chat();
extern void boot_attempt(char *who, char *method);
extern void conference_user_decline();
extern void accept_invite();
extern void decline_invite(char *reason);
extern void handle_conference_packet(char *pdata, int packet_type, int packet_size);

extern void clear_conference_hash();
extern int add_conference_member( char *user ) ;
extern void remove_conference_member(char *user);

extern int ymsg_leave_chat( YMSG_SESSION *session );

extern GtkWidget* create_make_conference (void);

extern void open_make_conference_window();
extern void open_make_conference_window_with_name(char *myname);
extern char  *get_conference_member_list(char *separator1, char *separator2);
extern char  *get_conference_invite_list(char *separator1, char *separator2);
extern int ymsg_conference_addinvite( YMSG_SESSION *session, char *who, char *msg);
extern int ymsg_leave_chat( YMSG_SESSION *session );
extern void show_ok_dialog(char *mymsg);











