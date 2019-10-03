/*****************************************************************************
 * users.h
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


#ifndef _USERS_H_
#define _USERS_H_

typedef struct pm_session {
	GtkWidget *pm_window;
	GtkWidget *pm_text;
	char *pm_user;
	int typing;
	int encryption_type;  /* added, PhrozeSmoke */
	int encrypted_myway;  /* added, PhrozeSmoke */
	int encrypted_theirway;  /* added, PhrozeSmoke */
	int my_gpg_passphrase;
	int their_gpg_passphrase;

	int sms;

	int buddy_image_size;
	int buddy_image_share;
	int buddy_image_visible;
	int window_style;
	char *buddy_image_file;
	GtkWidget *bimage_panel;
	GtkWidget *bimage_me;
	GtkWidget *bimage_them;
} PM_SESSION;

extern int emulate_ymsg6;
extern char *version_emulation;

extern GList *user_list;
extern GList *left_list;
extern GList *full_list;

extern GtkTreeView *chat_users;
extern GtkTreeIter chat_user_iter;
extern GtkTreeModel *chat_user_model;
extern int chat_user_count;
extern int igg_count_in_room;
extern int mutey_count_in_room;

extern GtkWidget *chat_user_menu;
extern char *user_selected;
extern char *follow_user;

extern int  show_html;
extern int  show_statuses;
extern int  show_enters;
extern int  show_blended_colors;
extern int  show_colors;
extern int  show_fonts;
extern int  show_avatars;
extern char *send_avatar;
extern int  show_emoticons;

extern GtkWidget * pm_window;
extern GtkWidget * pm_entry;
extern GtkWidget * pm_user;
extern GList *pm_list;
extern int  pm_in_sep_windows;
extern int  pm_from_friends;
extern int  pm_from_users;
extern int  pm_from_all;
extern int  auto_raise_pm;
extern int  auto_reply_when_away;
extern int  pm_brings_back;
extern int allow_pm_buzzes;
extern char *file_transfer_server;

/* added, PhrozenSmoke, support for ESound sound events and other features */
extern int  enable_sound_events;
extern int  enable_sound_events_pm;
extern int  allow_py_voice_helper;
extern int max_sim_animations; //
extern int enable_animations;
extern int enable_preemptive_strike;
extern int enable_basic_protection;
extern int disallow_random_friend_add;
extern int never_ignore_a_friend;
extern int allow_no_sent_files;
extern int highlight_friends_in_chat;
extern int highlight_me_in_chat;

extern int locale_status;
extern char *contact_first;
extern char *contact_last;
extern char *contact_email;
extern char *contact_home;
extern char *contact_work;
extern char *contact_mobile;

extern int mute_noage;
extern  int  mute_minors;
extern  int  mute_nogender;
extern  int mute_males;
extern int mute_females;

extern int enter_leave_timestamp;
extern int enable_webcam_features;
extern int enable_tuxvironments;
extern int enable_chat_spam_filter;

extern GtkWidget * profile_window;
extern char *prof_text;
extern char *prof_pic;

/* generic user/userlist routines */
int find_user_row( char *user );
void user_add( char *user );
void user_remove( char *user );
void chatter_list_populate( char *list, int clear_first );
void chatter_list_add( char *user );
void chatter_list_remove( char *user );
void chatter_list_status( char *user, gchar **status_image, char *status );
void *fetch_profile( void *arg );
void *display_url( void *arg );
int set_last_comment( char *user, char *comment );
char *get_last_comment( char *user );
void show_last_comments();

/* pm sessions */
GList *find_pm_session( char *user );
GList *find_pm_session_from_widget( GtkWidget *widget, char *name );

/* added: PhrozenSmoke */
int set_screenname_alias( char *user, char *useralias );
char *get_screenname_alias(char *user);
int update_screenname_alias( char *user, char *useralias ) ;


#endif /* #ifndef _USERS_H_ */
