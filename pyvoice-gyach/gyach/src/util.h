/*****************************************************************************
 * util.h
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


#ifndef _UTIL_H_
#define _UTIL_H_

typedef struct smiley {
	char *sm_text;
	char *sm_file;
} SMILEY;

extern char mail_user[];
extern char *recv_sound;
extern char *send_sound;
extern int  profile_viewer;
extern SMILEY *smileys;

/* added: PhrozenSmoke */
extern char *smiley_tags[90];
extern char *smiley_files[90];

extern char *mp3_player;
extern int show_yavatars;
extern int enable_audibles;
extern int activate_profnames;
extern int show_bimages;
extern int pm_window_style;
extern int auto_reject_invitations;
extern int pmb_audibles;
extern int pmb_smileys;
extern int pmb_send_erase;
extern int pmb_toolbar1;
extern int pmb_toolbar2;
extern char *flash_player_cmd;
extern char *bimage_file;
extern char *bimage_url;
extern int bimage_timestamp;
extern char *bimage_hash;
extern int bimage_friend_only;
extern int bimage_share;
extern int share_blist_avatar;
extern int bimage_size;

extern int chatwin_smileys;
extern int chatwin_send_erase;
extern int show_tabs_at_bottom;

extern char *avatar_filetype;
void delete_yavatars(char *ava_key);
void delete_all_bimages();
void delete_all_yavatars();
int bimage_exists(char *ava_key);
void clean_bimage_name(char *bkey);

void strip_html_tags( char *str );
int my_system( char *command );
void display_profile();
int fetch_url( char *image_url, char *buf, char *cookie );
void set_max_url_fetch_size(int mysize);
void lower_str( char *str );
void smileys_load();
int check_smiley( char *str );
void convert_smileys( char *str );
void convert_tags( char *str );
void strip_junk( char *str );
void capture_text_to_file( char *filename );
void subst_escs( char *str );
char *filter_text( char *txt );
char *replace_args( char *str, char *args );
void *fetch_avatar( void *arg );
void display_avatar( char *user, char *avatar );
int download_yavatar(char *avatar, int ava_size, char *burl);
void *play_sound( void *arg );
gint gstrcmp( gpointer a, gpointer b );
GList *gyach_g_list_free( GList *list );
GList *gyach_g_list_copy( GList *list );
GList *gyach_g_list_merge( GList *main, GList *sub );
GtkWidget *lookup_widget (GtkWidget  *widget, const gchar *widget_name);
extern char *download_image( char *image_url );
extern char *_(char *some);  /* added PhrozenSmoke: locale support */
extern gchar *_utf(char *some);
extern gchar *_b2loc(char *some);
extern void show_ok_dialog(char *mymsg);  /* added: PhrozenSmoke */


extern void set_tooltip(GtkWidget *somewid, char *somechar);
extern GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id);

extern int set_quick_profile( char *user, char *prof_info );
extern int is_chat_spam(char *tmpstr, char *tmpstr2);
extern int is_spam_name(char *who);
extern int is_valid_system_message(char *who);
extern int is_valid_game_sys_msg_content(char *who, char *content);
extern void convert_buddy_image();
extern int immunity_check( char *user );
extern void immunity_add( char *user );

void unset_url_post_data() ;
void set_url_post_data(char *pp);
void clear_fetched_url_data();

extern void set_socket_timer(int sockfd, int mytime);
extern void show_popup_dialog(char *mytitle, char *mymsg, int myicon);
extern void show_mail_stat(char *mailcount);


#endif /* #ifndef _UTIL_H_ */
