/*****************************************************************************
 * gyach.c
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
 *****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib.h>

#include "config.h"

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include "gyach.h"
#include "aliases.h"
#include "commands.h"
#include "callbacks.h"
#include "friends.h"
#include "gyach_int.h"
#include "history.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "main.h"
#include "parsecfg.h"
#include "roomlist.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "trayicon.h"   /* added, PhrozenSmoke */
#include "plugins.h"   /* added, PhrozenSmoke */
#include "webconnect.h"
#include "profname.h"
#include "gytreeview.h"
#include "ycht.h"

YMSG_SESSION ymsg_session, *ymsg_sess = &ymsg_session;

#define URL_REGEX "\\w{1,}\\.\\w{1,}/|\\w{1,}\\.\\w{1,}\\.\\w{1,}"

extern void show_ok_dialog(char *mymsg);  /* added: PhrozenSmoke */
extern void append_timestamp(GtkWidget *somewidget, GtkWidget *somewidget2);
extern void handle_dead_connection (int broken_pipe, int disable_reconn, int disable_dialog, char *reason);
extern int force_busy_icon;
extern int force_idle_icon;
extern gchar *_utf(char *some);
extern gchar *_b2loc(char *some);
extern int is_conference;
extern int login_invisible;
extern int login_noroom;
extern char *last_loginname;
extern char *webcam_viewer_app;
extern char *webcam_device;
extern int support_scentral_smileys;
extern int show_quick_access_toolbar;
extern char *custom_scale_text;
extern int enable_addressbook;
extern int buddy_image_upload_method;
extern void map_smileys();  /* added: PhrozenSmoke */

int debug_packets = 0;
int use_buddy_list_font=1;

GtkWidget * login_window;
GtkWidget * chat_window = NULL;
GtkWidget * chat_status;
GtkWidget * chat_entry;
GtkWidget * setup_menu;
GtkWidget * chat_username;
GtkWidget * chat_password;
GtkWidget * chat_roomname;
GtkWidget * setup_window = NULL;
GtkWidget * find_window = NULL;

guint st_cid;	/* statusbar context_id */

time_t connect_time = 0;

char *capture_filename = NULL;
FILE *capture_fp = NULL;

int my_status = 0;
time_t auto_away_time;
time_t child_timeout;

char *search_text = NULL;
int  search_pos = -1;
int  search_case_sensitive = 0;

int  logged_in = 0;
int  def_user = -1;
int  auto_away = 20;
int  auto_login = 0;
int  buffer_size = 10;
int  use_bold = 0;
int  use_underline = 0;
int  use_italics = 0;
int  font_size = 14;
int  display_font_size = 14;
int  force_lowercase = 0;
int  ping_minutes = 5;
int  chat_window_x = 0;
int  chat_window_y = 0;
int  chat_window_width = 800;
int  chat_window_height = 600;
int  chat_text_width = 580;
int  chat_user_height = 200;
int  remember_position = 1;
int  remember_password = 1;
int  limit_lfs = 8;
int  chat_timestamp = 0;
int  chat_timestamp_pm = 0;
int  auto_reconnect = 0;
int  use_proxy = 0;
int  custom_color_red = 0;
int  custom_color_green = 0;
int  custom_color_blue = 0;
int  proxy_port = 80;

char *auto_reply_msg = NULL;
char *browser_command = NULL;
char *filter_command = NULL;
char *font_name = NULL;
char *font_family = NULL;
char *display_font_name = NULL;
char *display_font_family = NULL;
char *use_color = NULL;
char *custom_away_message = NULL;
char *password = NULL;
char *def_room  = NULL;
char *proxy_host = NULL;

char *last_chatroom = NULL;
char *last_chatserver = NULL;

char *fader_text_end_str=NULL;
char *fader_text_start_str=NULL;
int use_chat_fader_val=1;

int popup_buzz=1;
int popup_buddy_on=1;
int popup_buddy_off=1;
int popup_new_mail=1;
int popup_news_bulletin=1;

int show_my_status_in_chat=1;
int pm_nick_names=0;

int emote_no_send=0;

int enable_pm_searches=1;

char cfg_filename[256] = "";
char GYACH_CFG_DIR[256] = "";
char GYACH_CFG_COMMON_DIR[256] = "";

char username[41] = "";
static char gycomment_buf[1664];

/* char roomname[129] = ""; */  /* unused - PhrozenSmoke */

int connected = 0;
int valid_user = 0;

GList *favroom_list = NULL;
GList *login_list = NULL;
GList *tab_user = NULL;

cfgList *username_list = NULL;
cfgList *fav_room_list = NULL;



/* This is XBM data for a 'hand' cursor I 
    use on my own system: The default 
    GDK hand cursors available just 
    SUCK!- PhrozenSmoke */

#define handcursor1_width 16
#define handcursor1_height 18
static unsigned char handcur1_bits[] ={
  0x30, 0x00, 0x48, 0x00, 0x48, 0x00, 0x48, 0x00, 0x48, 0x00, 0xc8, 0x6d, 
  0x48, 0x92, 0x4e, 0x92, 0x49, 0x92, 0x49, 0x92, 0x09, 0x80, 0x01, 0x80, 
  0x01, 0x80, 0x02, 0x80, 0x04, 0x40, 0xfc, 0x7f, 0xfc, 0x7f, 0xfc, 0x7f, 
  }; 
static unsigned  char handcur1mask_bits[] =  {
   0x30, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0x78, 0x00, 0xf8, 0x6d,
   0xf8, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x55, 0xd5,
   0xab, 0xaa, 0x56, 0xd5, 0xac, 0x6a, 0xfc, 0x7f, 0xfc, 0x7f, 0xfc, 0x7f };


cfgStruct cfg[] = {
	/* parameter  type address of variable */
	{ "username",			CFG_STRING_LIST,	&username_list },
	{ "fav_room",			CFG_STRING_LIST,	&fav_room_list },
	{ "show_blended_colors",	CFG_INT,		&show_blended_colors },
	{ "show_colors",			CFG_INT,		&show_colors },
	{ "show_fonts",				CFG_INT,		&show_fonts },
	{ "debug_packets",			CFG_INT,		&debug_packets },
	{ "def_user",				CFG_INT,		&def_user },
	{ "auto_away",				CFG_INT,		&auto_away },
	{ "auto_login",				CFG_INT,		&auto_login },
	{ "pm_brings_back",			CFG_INT,		&pm_brings_back },
	{ "buffer_size",			CFG_INT,		&buffer_size },
	{ "show_html",				CFG_INT,		&show_html },
	{ "show_statuses",			CFG_INT,		&show_statuses },
	{ "show_enters",			CFG_INT,		&show_enters },
	{ "profile_viewer",			CFG_INT,		&profile_viewer },
	{ "ignore_on_mults",		CFG_INT,		&ignore_on_mults },
	{ "mute_on_mults",			CFG_INT,		&mute_on_mults },
	{ "use_bold",				CFG_INT,		&use_bold },
	{ "use_italics",			CFG_INT,		&use_italics },
	{ "use_underline",			CFG_INT,		&use_underline },
	{ "force_lowercase",		CFG_INT,		&force_lowercase },
	{ "suppress_mult",			CFG_INT,		&suppress_mult },
	{ "ping_minutes",			CFG_INT,		&ping_minutes },
	{ "chat_window_x",			CFG_INT,		&chat_window_x },
	{ "chat_window_y",			CFG_INT,		&chat_window_y },
	{ "chat_window_width",		CFG_INT,		&chat_window_width },
	{ "chat_window_height",		CFG_INT,		&chat_window_height },
	{ "chat_text_width",		CFG_INT,		&chat_text_width },
	{ "chat_user_height",		CFG_INT,		&chat_user_height },
	{ "disp_auto_ignored",		CFG_INT,		&disp_auto_ignored },
	{ "remember_position",		CFG_INT,		&remember_position },
	{ "remember_password",		CFG_INT,		&remember_password },
	{ "limit_lfs",				CFG_INT,		&limit_lfs },
	{ "auto_reply_when_away",	CFG_INT,		&auto_reply_when_away },
	{ "pm_in_sep_windows",		CFG_INT,		&pm_in_sep_windows },
	{ "pm_from_friends",		CFG_INT,		&pm_from_friends },
	{ "pm_from_users",			CFG_INT,		&pm_from_users },
	{ "pm_from_all",			CFG_INT,		&pm_from_all },
	{ "enable_encryption",			CFG_INT,		&enable_encryption},
	{ "auto_raise_pm",			CFG_INT,		&auto_raise_pm },
	{ "auto_close_roomlist",	CFG_INT,		&auto_close_roomlist },
	{ "cache_room_list",		CFG_INT,		&cache_room_list },
	{ "chat_timestamp",			CFG_INT,		&chat_timestamp },
	{ "chat_timestamp_pm",		CFG_INT,		&chat_timestamp_pm },
	{ "ignore_guests",			CFG_INT,		&ignore_guests },
	{ "auto_reconnect",			CFG_INT,		&auto_reconnect },
	{ "history_limit",			CFG_INT,		&history_limit },
	{ "custom_color_red",		CFG_INT,		&custom_color_red },
	{ "custom_color_blue",		CFG_INT,		&custom_color_blue },
	{ "custom_color_green",		CFG_INT,		&custom_color_green },
	{ "auto_reply_msg",			CFG_STRING,		&auto_reply_msg },
	{ "use_color",				CFG_STRING,		&use_color },
	{ "custom_away_message",	CFG_STRING,		&custom_away_message },
	{ "browser_command",		CFG_STRING,		&browser_command },
	{ "filter_command",			CFG_STRING,		&filter_command },
	{ "font_name",				CFG_STRING,		&font_name },
	{ "font_family",			CFG_STRING,		&font_family },
	{ "font_size",				CFG_INT,		&font_size },
	{ "display_font_name",				CFG_STRING,		&display_font_name },
	{ "display_font_family",			CFG_STRING,		&display_font_family },
	{ "display_font_size",				CFG_INT,		&display_font_size },
	{ "indent_wrapped",			CFG_INT,		&indent_wrapped },
	{ "last_chatserver",				CFG_STRING,		&last_chatserver },
	{ "last_chatroom",				CFG_STRING,		&last_chatroom  },
	{ "last_loginname",				CFG_STRING,		&last_loginname  },
	{ "login_invisible",			CFG_INT,		&login_invisible },
	{ "login_noroom",			CFG_INT,		&login_noroom },

	{ "fader_text_end_str",				CFG_STRING,		&fader_text_end_str  },
	{ "fader_text_start_str",				CFG_STRING,		&fader_text_start_str  },
	{ "use_chat_fader_val",			CFG_INT,		&use_chat_fader_val },

	{ "password",				CFG_STRING,		&password },
	{ "def_room",				CFG_STRING,		&def_room },
	{ "show_adult",				CFG_INT,		&show_adult },
	{ "webcam_viewer_app",				CFG_STRING,		&webcam_viewer_app },
	{ "webcam_device",				CFG_STRING,		&webcam_device },
	{ "contact_first",				CFG_STRING,		&contact_first },
	{ "contact_last",				CFG_STRING,		&contact_last },
	{ "contact_email",				CFG_STRING,		&contact_email },
	{ "contact_home",				CFG_STRING,		&contact_home },
	{ "contact_work",				CFG_STRING,		&contact_work },
	{ "contact_mobile",				CFG_STRING,		&contact_mobile },
	{ "locale_status",			CFG_INT,		&locale_status },
	{ "highlight_friends_in_chat",				CFG_INT,		&highlight_friends_in_chat },
	{ "highlight_me_in_chat",				CFG_INT,		&highlight_me_in_chat },
	{ "enable_sound_events",				CFG_INT,		&enable_sound_events },
	{ "enable_trayicon",				CFG_INT,		&enable_trayicon},
	{ "enable_webcam_features",				CFG_INT,		&enable_webcam_features},
	{ "enable_tuxvironments",				CFG_INT,		&enable_tuxvironments},
	{ "enter_leave_timestamp",				CFG_INT,		&enter_leave_timestamp },
	{ "enable_chat_spam_filter",				CFG_INT,		&enable_chat_spam_filter},
	{ "mute_noage",				CFG_INT,		&mute_noage },
	{ "mute_minors",				CFG_INT,		&mute_minors },
	{ "mute_nogender",				CFG_INT,		&mute_nogender },
	{ "mute_males",				CFG_INT,		&mute_males },
	{ "mute_females",				CFG_INT,		&mute_females },
	{ "enable_sound_events_pm",				CFG_INT,		&enable_sound_events_pm },
	{ "allow_py_voice_helper",				CFG_INT,		&allow_py_voice_helper },
	{ "max_sim_animations",				CFG_INT,		&max_sim_animations},
	{ "enable_animations",				CFG_INT,		&enable_animations},
	{ "enable_preemptive_strike",				CFG_INT,		&enable_preemptive_strike },
	{ "enable_basic_protection",				CFG_INT,		&enable_basic_protection},
	{ "disallow_random_friend_add",				CFG_INT,		&disallow_random_friend_add},
	{ "never_ignore_a_friend",				CFG_INT,		&never_ignore_a_friend},
	{ "allow_no_sent_files",				CFG_INT,		&allow_no_sent_files},
	{ "show_avatars",			CFG_INT,		&show_avatars },
	{ "send_avatar",			CFG_STRING,		&send_avatar },
	{ "show_emoticons",			CFG_INT,		&show_emoticons },
	{ "support_scentral_smileys",			CFG_INT,		&support_scentral_smileys },
	{ "use_proxy",				CFG_INT,		&use_proxy },
	{ "proxy_host",				CFG_STRING,		&proxy_host },
	{ "custom_scale_text",				CFG_STRING,		&custom_scale_text },
	{ "proxy_port",				CFG_INT,		&proxy_port },
	{ "first_post_is_url",		CFG_INT,		&first_post_is_url },
	{ "first_post_is_pm",		CFG_INT,		&first_post_is_pm },
	{ "url_from_nonroom_user",	CFG_INT,		&url_from_nonroom_user },
	{ "highlight_friends",		CFG_INT,		&highlight_friends },
	{ "popup_buzz",		CFG_INT,		&popup_buzz },
	{ "popup_buddy_on",		CFG_INT,		&popup_buddy_on },
	{ "popup_buddy_off",		CFG_INT,		&popup_buddy_off },
	{ "popup_new_mail",		CFG_INT,		&popup_new_mail },
	{ "popup_news_bulletin",		CFG_INT,		&popup_news_bulletin },
	{ "transparent_bg",			CFG_INT,		&transparent_bg },
	{ "shading_r",				CFG_INT,		&shading_r },
	{ "shading_g",				CFG_INT,		&shading_g },
	{ "shading_b",				CFG_INT,		&shading_b },
	{ "use_buddy_list_font",				CFG_INT,		&use_buddy_list_font },
	{ "show_quick_access_toolbar",				CFG_INT,		&show_quick_access_toolbar },
	{ "pm_nick_names",				CFG_INT,		&pm_nick_names },
	{ "show_my_status_in_chat",				CFG_INT,		&show_my_status_in_chat },

	{ "bimage_share",     CFG_INT,  &bimage_share }, 
	{ "share_blist_avatar",     CFG_INT,  &share_blist_avatar }, 
	{ "bimage_size",     CFG_INT,  &bimage_size }, 
	{ "auto_reject_invitations",     CFG_INT,  &auto_reject_invitations }, 
	{ "pmb_audibles",     CFG_INT,  &pmb_audibles }, 
	{ "pmb_smileys",     CFG_INT,  &pmb_smileys }, 
	{ "pmb_send_erase",     CFG_INT,  &pmb_send_erase }, 
	{ "bimage_friend_only",     CFG_INT,  &bimage_friend_only }, 
	{ "show_yavatars",     CFG_INT,  &show_yavatars }, 
	{ "enable_audibles",     CFG_INT,  &enable_audibles }, 
	{ "activate_profnames",     CFG_INT,  &activate_profnames}, 
	{ "show_bimages",     CFG_INT,  &show_bimages }, 
	{ "pm_window_style",     CFG_INT,  &pm_window_style }, 
	{ "bimage_file",     CFG_STRING,  &bimage_file}, 
	{ "flash_player_cmd",     CFG_STRING,  &flash_player_cmd}, 
	{ "mp3_player",     CFG_STRING,  &mp3_player }, 
	{ "emulate_ymsg6",     CFG_INT,  &emulate_ymsg6 }, 
	{ "pmb_toolbar1",     CFG_INT,  &pmb_toolbar1 }, 
	{ "pmb_toolbar2",     CFG_INT,  &pmb_toolbar2 }, 
	{ "chatwin_smileys",     CFG_INT,  &chatwin_smileys }, 
	{ "chatwin_send_erase",     CFG_INT,  &chatwin_send_erase }, 
	{ "show_tabs_at_bottom",     CFG_INT,  &show_tabs_at_bottom }, 

	{ "file_transfer_server",     CFG_STRING,  &file_transfer_server }, 
	{ "allow_pm_buzzes",     CFG_INT,  &allow_pm_buzzes }, 

	{ "enable_addressbook",     CFG_INT,  &enable_addressbook }, 
	{ "enable_pm_searches",     CFG_INT,  &enable_pm_searches }, 
	{ "buddy_image_upload_method",     CFG_INT,  &buddy_image_upload_method }, 

	{ NULL,						CFG_END,		NULL }
};

/*
 * this function serves the purpose of upgrading Gyach user's config files
 * to the new *nix client chat standard at:
 * http://www.edge-op.org/files/chatstandard
 *
 * this standard should be supported by curphoo and hopefully curfloo in
 * due time
 */
void upgrade_config_to_standard() {
	char filename[256];
	char filename2[256];
	char filename3[256];
	struct stat sbuf;
	struct stat sbuf2;
	struct stat sbuf3;

	snprintf( filename, 254, "%s/.gyach/gyachrc", getenv( "HOME" ));
	snprintf( filename2, 254, "%s/.yahoorc/gyach/gyachrc", getenv( "HOME" ));

	if (( ! stat( filename, &sbuf )) &&
		( stat( filename2, &sbuf2 ))) {
		/* Convert from ~/.gyach directory to ~/.yahoorc format */
		/* make ~/.yahoorc dir if doesn't exist */
		snprintf( filename3,254,  "%s/.yahoorc", getenv( "HOME" ));
		if ( stat( filename3, &sbuf3 )) {
			mkdir( filename3, 0700 );
		}

		/* remove ~/.yahoorc/gyach dir if exists */
		/* only do this since we know no ~/.yahoorc/gyach/gyachrc file exists */
		snprintf( filename3,254,  "%s/.yahoorc/gyach", getenv( "HOME" ));
		if ( ! stat( filename3, &sbuf3 )) {
			rmdir( filename3 );
		}

		/* move gyachrc file */
		rename( filename, filename2 );

		/* move aliases/commands file */
		snprintf( filename, 254, "%s/.gyach/aliases", getenv( "HOME" ));
		snprintf( filename2, 254, "%s/.gyach/commands", getenv( "HOME" ));
		/* waiting on this for fan to implement standard
		snprintf( filename2, 254, "%s/.yahoorc/gyach/commands", getenv( "HOME" ));
		*/
		if ( stat( filename2, &sbuf2 )) {
			/* if commands doesn't exist then move aliases file */
			rename( filename, filename2 );
		}

		/* move ignore.list file */
		snprintf( filename, 254, "%s/.gyach/ignore.list", getenv( "HOME" ));
		snprintf( filename2,254,  "%s/.gyach/ignore", getenv( "HOME" ));
		/* waiting on this for fan to implement standard
		snprintf( filename2, 254, "%s/.yahoorc/ignore", getenv( "HOME" ));
		*/
		if ( stat( filename2, &sbuf2 )) {
			/* if ignore list doesn't exist them move ignore.list file */
			rename( filename, filename2 );
		}

		/* finally move the gyach directory itself */
		snprintf( filename,254,  "%s/.gyach", getenv( "HOME" ));
		snprintf( filename2,254,  "%s/.yahoorc/gyach", getenv( "HOME" ));
		rename( filename, filename2 );
	}

	/* convert from 0.9.1-pre3 */
	snprintf( filename, 254, "%s/.yahoorc/ignore", getenv( "HOME" ));
	snprintf( filename2, 254, "%s/.yahoorc/gyach/ignore", getenv( "HOME" ));
	if (( ! stat( filename, &sbuf )) &&
		( stat( filename2, &sbuf2 ))) {
		rename( filename, filename2 );
	}

	snprintf( filename, 254, "%s/.yahoorc/commands", getenv( "HOME" ));
	snprintf( filename2, 254, "%s/.yahoorc/gyach/commands", getenv( "HOME" ));
	if (( ! stat( filename, &sbuf )) &&
		( stat( filename2, &sbuf2 ))) {
		rename( filename, filename2 );
	}
}

void gyach_copy( char *src, char *dest ) {
	FILE *in, *out;
	int ch;
	char fn_src[256];
	char fn_dest[256];

	snprintf( fn_src, 254, "%s/.yahoorc/%s", getenv( "HOME" ), src );
	snprintf( fn_dest, 254, "%s/.yahoorc/%s", getenv( "HOME" ), dest );

	in = fopen( fn_src, "r" );
	if ( in ) {
		out = fopen( fn_dest, "w" );
		if ( out ) {
			while(( ch = fgetc( in )) != EOF ) {
				fputc( ch, out );
			}
			fclose( out );
		}
		fclose( in );
	}
}

void gyach_backup() {
	char filename[256];
	char filename2[256];
	char backup_dir[256];
	struct stat sbuf;
	int i;

	snprintf( backup_dir, 254, "%s/backups", GYACH_CFG_DIR );
	if ( stat( backup_dir, &sbuf )) {
		mkdir( backup_dir, 0700 );
	}

	for( i = 8; i >= 0; i-- ) {
		snprintf( filename, 254, "%s/gyachrc.%d", backup_dir, i );
		snprintf( filename2, 254, "%s/gyachrc.%d", backup_dir, i + 1 );
		rename( filename, filename2 );

		snprintf( filename, 254, "%s/ignore.%d", backup_dir, i );
		snprintf( filename2, 254, "%s/ignore.%d", backup_dir, i + 1 );
		rename( filename, filename2 );

		snprintf( filename, 254, "%s/commands.%d", backup_dir, i );
		snprintf( filename2, 254, "%s/commands.%d", backup_dir, i + 1 );
		rename( filename, filename2 );
	}

	/* now copy originals to *.0 in backups dir */
	gyach_copy( "gyach/gyachrc", "gyach/backups/gyachrc.0" );
	gyach_copy( "gyach/ignore", "gyach/backups/ignore.0" );
	gyach_copy( "gyach/commands", "gyach/backups/commands.0" );
}


void gyach_init() {
	struct stat sbuf;

	// DBG( 1, "gyach_init()\n" );

	upgrade_config_to_standard();

	snprintf( GYACH_CFG_COMMON_DIR, 254, "%s/.yahoorc", getenv( "HOME" ));
	if ( stat( GYACH_CFG_COMMON_DIR, &sbuf ))
		mkdir( GYACH_CFG_COMMON_DIR, 0700 );

	snprintf( GYACH_CFG_DIR, 254, "%s/.yahoorc/gyach", getenv( "HOME" ));
	if ( stat( GYACH_CFG_DIR, &sbuf ))
		mkdir( GYACH_CFG_DIR, 0700 );

	/* overrides until we get the standard in place */
	snprintf( GYACH_CFG_COMMON_DIR, 254, "%s/.yahoorc/gyach", getenv( "HOME" ));

	gyach_backup();
	read_config();
	ignore_load();
	history_load();
	smileys_load();
	map_smileys(); /* added: PhrozenSmoke */

	if ( ! history_list ) {	history_add( "" );	}

	auto_away_time = time(NULL) + ( auto_away * 60 );

	if ( ! font_name ) {
		font_name = strdup( "times 12" );
	} else {
		if ( strstr( font_name, "-*-" )) {
			free( font_name );
			font_name = strdup( "times 12" );
			if ( font_family)
				free( font_family );
			font_family = strdup( "times" );
			font_size = 12;
			use_bold = 0;
			use_italics = 0;
		}
	}

	if ( ! font_family ) {font_family = strdup( "times" );}
	if ( font_size < 2 ) {font_size = 14;}


	if ( ! display_font_name ) {
		display_font_name = strdup( "fixed 12" );
	} else {
		if ( strstr( display_font_name, "-*-" )) {
			free( display_font_name );
			display_font_name = strdup( "fixed 12" );
			if ( display_font_family)
				free( display_font_family );
			display_font_family = strdup( "fixed" );
			display_font_size = 12;
		}
	}

	if ( ! display_font_family ) {display_font_family = strdup( "fixed" );}
	if ( display_font_size < 2 ) {display_font_size = 14;}


	if ( ! browser_command ) {
		browser_command =
			strdup( "opera -remote 'openURL\\(%s\\)' &" );
	}
}

void process_gtk_events() {
	/* // DBG( 251, "process_gtk_events()\n" ); */

	return;     /* currently disable to help with thread safety, PhrozenSmoke */

	while( gtk_events_pending())
		gtk_main_iteration();
}


/* BEGIN Pass encoding/decoding - PhrozenSmoke
   get ride of plain text passwords saved to file!  */


/*
 * Converts a Base64 encoded letter to a (6 bit) number between 0 and 63, or 64
 * if it is a pad character, or -1 if it is anything else.
  * Taken from base64.c, public domain code 
*/
static int b64toi(char c)
{
	/* Decode A-B */
	if((c >= 'A') && (c <= 'Z')) return(c - 'A');
	/* Decode a-b */
	else if((c >= 'a') && (c <= 'z')) return(c - 'a' + 26);
	/* Decode 0-9 */
	else if((c >= '0') && (c <= '9')) return(c - '0' + 52);
	/* Decode the weird ones */
	else if(c == '+') return 62;
	else if(c == '/') return 63;
	else if(c == '=') return 64;
	/* Anything else is not part of Base64 */
	else return -1;
}

/*
 * Converts a 6 bit number to a Base64 encoded letter.
  * Taken from base64.c, public domain code 
*/

static char itob64(int n)
{
	/* Encode A-B */
	if(n <= 25) return('A' + n);
	/* Encode a-b */
	else if(n <= 51) return('a' + n - 26);
	/* Encode 0-9 */
	else if(n <= 61) return('0' + n - 52);
	/* Encode the weird ones */
	else if(n == 62) return '+';
	else if(n == 63) return '/';
	/* Not a 6 bit number */
	else return 'a';
}

/* implement basic password decoding, PhrozenSmoke */
char *decode_pass(char *some_p) {
	int stopit=0;
	char inbuf[355]="";
	char obuf[102]="";
	char tbuf[6]="";
	char nbuf[6]="";
	char *inb=inbuf;
	char *findit=NULL;
	int iletto=12;
	int letwh=0;
	char *retc=obuf;
	snprintf(retc, 3, "%s","");
	if (some_p==NULL) {  return strdup("");}
	if (strlen(some_p)==0) {  return strdup("");}

	stopit=snprintf(inbuf,353,"%s",some_p);
	inbuf[stopit]='\0';

	findit=strstr(inbuf,"1p7127143319");
	if (!findit) {  return strdup("");}

	sprintf(obuf,"%s","");
	while (findit != NULL)  {
		*findit='\0';
		snprintf(nbuf,5, "%s", inb);
		snprintf(tbuf,5,"%c", itob64( atoi( nbuf )-89) );
		strncat(obuf,tbuf,5);
		letwh=0;
		while (letwh<iletto){ letwh++; findit++;}
		inb=findit;
		findit=strstr(inb,"1p7127143319");
		if (findit==NULL ) {break;}
		if (strlen(obuf)>80) {break;}		
	}
return strdup(retc);
}


/* implement basic password encoding, PhrozenSmoke */
char *encode_pass(char *some_p) {
	int stopit=0;
	char inbuf[85]="";
	char obuf[355]="";
	char tbuf[8]="";
	char *retc=obuf;
	int ij=0;
	if (some_p==NULL) {  return strdup("");}
	if (strlen(some_p)==0) {  return strdup("");}
	stopit=snprintf(inbuf,83,"%s",some_p);
	inbuf[stopit]='\0';
	sprintf(obuf,"%s","");
	for (ij=0; ij<stopit; ij++)  {
		snprintf(tbuf,7,"%d", b64toi(inbuf[ij])+89 );
		strncat(obuf,tbuf,7);
		strcat(obuf,"1p7127143319");
		if (strlen(obuf)>330) {break;}
	}
return strdup(retc);
}

/* END Pass encoding/decoding - PhrozenSmoke
   get ride of plain text passwords saved to file!  */


void read_config() {
	struct stat sbuf;
	cfgList *this_item;

	// DBG( 1, "read_config()\n" );

	if ( ! getenv( "HOME" )) {
		return;
	}

	snprintf( cfg_filename, 254, "%s/gyachrc", GYACH_CFG_DIR );

	if ( stat( cfg_filename, &sbuf ))
		write_config();

	cfgParse( cfg_filename, cfg, CFG_SIMPLE );

	if (last_chatserver) {last_chatserver=strdup(last_chatserver);}
	if (last_chatroom) {last_chatroom=strdup(last_chatroom);}
	if (!flash_player_cmd) {flash_player_cmd=strdup("gflashplayer");}
	if (! mp3_player) {mp3_player=strdup("mplayer -ao esd -quiet");}

		if (password)  {
			/* This comes to us strdup'ed, as we need it */
			password=decode_pass(password);
			/* printf("\ngot pass: %s\n",password); fflush(stdout);  */ /* testing */
								}

	if ( ! use_color )
		use_color = strdup( "black" );

	ymsg_sess->debug_packets = debug_packets;

	this_item = username_list;
	while( this_item ) {
		login_list = g_list_append( login_list, strdup( this_item->str ));
		this_item = this_item->next;
	}

	this_item = fav_room_list;
	while( this_item ) {
		favroom_list = g_list_append( favroom_list, strdup( this_item->str ));
		this_item = this_item->next;
	}
}

void write_config() {
	GtkWidget *tmp_widget;
	GList *this_item = NULL;
	FILE *gyachrc;

	// DBG( 1, "write_config()\n" );

	snprintf( cfg_filename, 254, "%s/gyachrc", GYACH_CFG_DIR );

	gyachrc = fopen( cfg_filename, "wb" );

	if ( gyachrc ) {
		if ( chat_window ) {
			tmp_widget = chat_window;

			if ( remember_position ) {
				gdk_window_get_position( tmp_widget->window,
					&chat_window_x, &chat_window_y );
			}

			/* get current sizes */
			chat_window_width = tmp_widget->allocation.width;
			chat_window_height = tmp_widget->allocation.height;

			tmp_widget = lookup_widget( chat_window, "hbox2" );
			chat_text_width = tmp_widget->allocation.width;

			tmp_widget = lookup_widget( chat_window, "scrolledwindow3" );
			chat_user_height = tmp_widget->allocation.height;
		}

		/* FIXME, put while loop here to save "username" */
		this_item = login_list;
		while( this_item ) {
			fprintf( gyachrc, "username = \"%s\"\n", (char *)this_item->data );
			this_item = g_list_next( this_item );
		}

		if (password)  {
			char *encpass=encode_pass(password);
			fprintf( gyachrc, "password = \"%s\"\n", encpass);
			free(encpass);
								}

		this_item = favroom_list;
		while( this_item ) {
			fprintf( gyachrc, "fav_room = \"%s\"\n", (char *)this_item->data );
			this_item = g_list_next( this_item );
		}

		if (auto_reply_msg) fprintf( gyachrc,
			"auto_reply_msg = \"%s\"\n", auto_reply_msg );
		if (custom_away_message) fprintf( gyachrc,
			"custom_away_message = \"%s\"\n", custom_away_message );
		if (use_color) fprintf( gyachrc,
			"use_color = \"%s\"\n", use_color );

		if (font_name) fprintf( gyachrc,
			"font_name = \"%s\"\n", font_name );
		if (font_family) fprintf( gyachrc,
			"font_family = \"%s\"\n", font_family );
		fprintf( gyachrc, "font_size = %d\n", font_size );

		if (display_font_name) fprintf( gyachrc,
			"display_font_name = \"%s\"\n", display_font_name );
		if (display_font_family) fprintf( gyachrc,
			"display_font_family = \"%s\"\n", display_font_family );
		fprintf( gyachrc, "display_font_size = %d\n", display_font_size );

		if (browser_command) fprintf( gyachrc,
			"browser_command = \"%s\"\n", browser_command );
		if (filter_command) fprintf( gyachrc,
			"filter_command = \"%s\"\n", filter_command );

		if (custom_scale_text) fprintf( gyachrc,
			"custom_scale_text = \"%s\"\n", custom_scale_text );

		if (webcam_viewer_app) fprintf( gyachrc,
			"webcam_viewer_app = \"%s\"\n", webcam_viewer_app );
		if (webcam_device) fprintf( gyachrc,
			"webcam_device = \"%s\"\n", webcam_device);

		if (contact_first) fprintf( gyachrc,
			"contact_first = \"%s\"\n", contact_first );
		if (contact_last) fprintf( gyachrc,
			"contact_last = \"%s\"\n", contact_last );
		if (contact_email) fprintf( gyachrc,
			"contact_email = \"%s\"\n", contact_email );
		if (contact_home) fprintf( gyachrc,
			"contact_home = \"%s\"\n", contact_home );
		if (contact_work) fprintf( gyachrc,
			"contact_work = \"%s\"\n", contact_work );
		if (contact_mobile) fprintf( gyachrc,
			"contact_mobile = \"%s\"\n", contact_mobile );

		if (last_chatroom) fprintf( gyachrc,
			"last_chatroom = \"%s\"\n", last_chatroom );

		if (last_chatserver) fprintf( gyachrc,
			"last_chatserver = \"%s\"\n", last_chatserver );

		if (last_loginname) fprintf( gyachrc,
			"last_loginname = \"%s\"\n", last_loginname );
		fprintf( gyachrc, "login_invisible = %d\n", login_invisible );
		fprintf( gyachrc, "login_noroom = %d\n", login_noroom );

		if (fader_text_start) {
			fprintf( gyachrc,"fader_text_start_str = \"%s\"\n", gtk_entry_get_text(GTK_ENTRY(fader_text_start)) );
		}
		if (fader_text_end) {
			fprintf( gyachrc,"fader_text_end_str = \"%s\"\n", gtk_entry_get_text(GTK_ENTRY(fader_text_end)) );
		}

		if (fader_text_start && use_chat_fader) { 
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) {
				fprintf( gyachrc, "use_chat_fader_val = %d\n", 1 );
			} else {fprintf( gyachrc, "use_chat_fader_val = %d\n", 0);}
		}

		fprintf( gyachrc, "show_blended_colors = %d\n", show_blended_colors );
		fprintf( gyachrc, "show_colors = %d\n", show_colors );
		fprintf( gyachrc, "show_fonts = %d\n", show_fonts );
		fprintf( gyachrc, "debug_packets = %d\n", ymsg_sess->debug_packets );
		fprintf( gyachrc, "auto_away = %d\n", auto_away );
		fprintf( gyachrc, "auto_login = %d\n", auto_login );
		fprintf( gyachrc, "pm_brings_back = %d\n", pm_brings_back );
		fprintf( gyachrc, "buffer_size = %d\n", buffer_size );
		fprintf( gyachrc, "show_html = %d\n", show_html );
		fprintf( gyachrc, "show_statuses = %d\n", show_statuses );
		fprintf( gyachrc, "show_enters = %d\n", show_enters );
		fprintf( gyachrc, "profile_viewer = %d\n", profile_viewer );
		fprintf( gyachrc, "ignore_on_mults = %d\n", ignore_on_mults );
		fprintf( gyachrc, "mute_on_mults = %d\n", mute_on_mults );
		fprintf( gyachrc, "use_bold = %d\n", use_bold );
		fprintf( gyachrc, "use_italics = %d\n", use_italics );
		fprintf( gyachrc, "use_underline = %d\n", use_underline );
		fprintf( gyachrc, "force_lowercase = %d\n", force_lowercase );
		fprintf( gyachrc, "suppress_mult = %d\n", suppress_mult );
		fprintf( gyachrc, "ping_minutes = %d\n", ping_minutes );
		fprintf( gyachrc, "indent_wrapped = %d\n", indent_wrapped );
		fprintf( gyachrc, "popup_buzz = %d\n", popup_buzz );
		fprintf( gyachrc, "popup_buddy_on = %d\n", popup_buddy_on );
		fprintf( gyachrc, "popup_buddy_off = %d\n", popup_buddy_off );
		fprintf( gyachrc, "popup_new_mail = %d\n", popup_new_mail );
		fprintf( gyachrc, "popup_news_bulletin = %d\n", popup_news_bulletin);
		fprintf( gyachrc, "transparent_bg = %d\n", transparent_bg );
		fprintf( gyachrc, "shading_r = %d\n", shading_r );
		fprintf( gyachrc, "shading_g = %d\n", shading_g );
		fprintf( gyachrc, "shading_b = %d\n", shading_b );
		fprintf( gyachrc, "use_buddy_list_font = %d\n", use_buddy_list_font );

	fprintf( gyachrc, "bimage_share = %d\n",bimage_share );
	fprintf( gyachrc, "share_blist_avatar = %d\n", share_blist_avatar );
	fprintf( gyachrc, "bimage_size = %d\n", bimage_size );
	fprintf( gyachrc, "auto_reject_invitations = %d\n", auto_reject_invitations );
	fprintf( gyachrc, "pmb_audibles = %d\n", pmb_audibles );
	fprintf( gyachrc, "pmb_smileys = %d\n", pmb_smileys );
	fprintf( gyachrc, "pmb_send_erase = %d\n", pmb_send_erase );
	fprintf( gyachrc, "bimage_friend_only = %d\n", bimage_friend_only );
	fprintf( gyachrc, "show_yavatars = %d\n", show_yavatars );
	fprintf( gyachrc, "enable_audibles = %d\n", enable_audibles );
	fprintf( gyachrc, "activate_profnames = %d\n", activate_profnames);
	fprintf( gyachrc, "show_bimages = %d\n", show_bimages );
	fprintf( gyachrc, "pm_window_style = %d\n", pm_window_style );
	fprintf( gyachrc, "emulate_ymsg6 = %d\n", emulate_ymsg6 );
	fprintf( gyachrc, "pmb_toolbar1 = %d\n", pmb_toolbar1 );
	fprintf( gyachrc, "pmb_toolbar2 = %d\n", pmb_toolbar2);

	fprintf( gyachrc, "enable_addressbook = %d\n", enable_addressbook);
	fprintf( gyachrc, "enable_pm_searches = %d\n", enable_pm_searches);

	fprintf( gyachrc, "show_tabs_at_bottom = %d\n", show_tabs_at_bottom);
	fprintf( gyachrc, "chatwin_send_erase = %d\n", chatwin_send_erase);
	fprintf( gyachrc, "chatwin_smileys = %d\n", chatwin_smileys);
		if (bimage_file) fprintf( gyachrc,
			"bimage_file = \"%s\"\n", bimage_file );
		if (flash_player_cmd) fprintf( gyachrc,
			"flash_player_cmd = \"%s\"\n", flash_player_cmd );
		if (mp3_player) fprintf( gyachrc,
			"mp3_player = \"%s\"\n", mp3_player );

		if ( remember_position ) {
			fprintf( gyachrc, "chat_window_x = %d\n", chat_window_x );
			fprintf( gyachrc, "chat_window_y = %d\n", chat_window_y );
		}

		fprintf( gyachrc, "chat_window_width = %d\n", chat_window_width );
		fprintf( gyachrc, "chat_window_height = %d\n", chat_window_height );
		fprintf( gyachrc, "chat_text_width = %d\n", chat_text_width );
		fprintf( gyachrc, "chat_user_height = %d\n", chat_user_height );
		fprintf( gyachrc, "disp_auto_ignored = %d\n", disp_auto_ignored );
		fprintf( gyachrc, "remember_position = %d\n", remember_position );
		fprintf( gyachrc, "remember_password = %d\n", remember_password );
		fprintf( gyachrc, "limit_lfs = %d\n", limit_lfs );
		fprintf( gyachrc, "auto_reply_when_away = %d\n", auto_reply_when_away );
		fprintf( gyachrc, "pm_in_sep_windows = %d\n", pm_in_sep_windows );
		fprintf( gyachrc, "pm_from_friends = %d\n", pm_from_friends );
		fprintf( gyachrc, "pm_from_users = %d\n", pm_from_users );
		fprintf( gyachrc, "pm_from_all = %d\n", pm_from_all );
		fprintf( gyachrc, "enable_encryption = %d\n", enable_encryption);
		fprintf( gyachrc, "auto_raise_pm = %d\n", auto_raise_pm );
		fprintf( gyachrc, "auto_close_roomlist = %d\n", auto_close_roomlist );
		fprintf( gyachrc, "cache_room_list = %d\n", cache_room_list );
		fprintf( gyachrc, "chat_timestamp = %d\n", chat_timestamp );
		fprintf( gyachrc, "chat_timestamp_pm = %d\n", chat_timestamp_pm );
		fprintf( gyachrc, "ignore_guests = %d\n", ignore_guests );
		fprintf( gyachrc, "auto_reconnect = %d\n", auto_reconnect );
		fprintf( gyachrc, "history_limit = %d\n", history_limit );
		fprintf( gyachrc, "show_adult = %d\n", show_adult );
		fprintf( gyachrc, "highlight_friends_in_chat = %d\n",  highlight_friends_in_chat);
		fprintf( gyachrc, "highlight_me_in_chat = %d\n",  highlight_me_in_chat);
		fprintf( gyachrc, "locale_status = %d\n",  locale_status);
		fprintf( gyachrc, "enable_trayicon = %d\n",  enable_trayicon);
		fprintf( gyachrc, "enable_sound_events = %d\n",  enable_sound_events);
		fprintf( gyachrc, "enable_sound_events_pm = %d\n",  enable_sound_events_pm);
		fprintf( gyachrc, "enable_webcam_features = %d\n",  enable_webcam_features);
		fprintf( gyachrc, "enter_leave_timestamp = %d\n", enter_leave_timestamp);
		fprintf( gyachrc, "enable_tuxvironments = %d\n", enable_tuxvironments);
		fprintf( gyachrc, "enable_chat_spam_filter = %d\n", enable_chat_spam_filter);
		fprintf( gyachrc, "mute_noage = %d\n",  mute_noage);
		fprintf( gyachrc, "mute_minors = %d\n",  mute_minors);
		fprintf( gyachrc, "mute_nogender = %d\n",  mute_nogender);
		fprintf( gyachrc, "mute_males = %d\n",  mute_males);
		fprintf( gyachrc, "mute_females = %d\n",  mute_females);
		fprintf( gyachrc, "allow_py_voice_helper = %d\n",  allow_py_voice_helper);
		fprintf( gyachrc, "max_sim_animations = %d\n",  max_sim_animations);
		fprintf( gyachrc, "enable_animations = %d\n",  enable_animations);
		fprintf( gyachrc, "enable_preemptive_strike = %d\n",  enable_preemptive_strike);
		fprintf( gyachrc, "enable_basic_protection = %d\n",  enable_basic_protection);
		fprintf( gyachrc, "disallow_random_friend_add = %d\n",  disallow_random_friend_add);
		fprintf( gyachrc, "never_ignore_a_friend = %d\n",  never_ignore_a_friend);
		fprintf( gyachrc, "allow_no_sent_files = %d\n",  allow_no_sent_files);
		fprintf( gyachrc, "use_proxy = %d\n", use_proxy );
		fprintf( gyachrc, "show_avatars = %d\n", show_avatars );
		fprintf( gyachrc, "show_emoticons = %d\n", show_emoticons );
		fprintf( gyachrc, "support_scentral_smileys = %d\n", support_scentral_smileys );
		fprintf( gyachrc, "custom_color_red = %d\n", custom_color_red );
		fprintf( gyachrc, "custom_color_blue = %d\n", custom_color_blue );
		fprintf( gyachrc, "custom_color_green = %d\n", custom_color_green );

		if ( proxy_host ) {
			fprintf( gyachrc, "proxy_host = \"%s\"\n", proxy_host );
		}

		fprintf( gyachrc, "proxy_port = %d\n", proxy_port );

		fprintf( gyachrc, "show_quick_access_toolbar = %d\n", show_quick_access_toolbar );
		fprintf( gyachrc, "show_my_status_in_chat = %d\n", show_my_status_in_chat );
		fprintf( gyachrc, "pm_nick_names = %d\n", pm_nick_names );

		if ( send_avatar ) {
			fprintf( gyachrc, "send_avatar = \"%s\"\n", send_avatar );
		}

		fprintf( gyachrc, "first_post_is_url = %d\n", first_post_is_url );
		fprintf( gyachrc, "first_post_is_pm = %d\n", first_post_is_pm );
		fprintf( gyachrc, "url_from_nonroom_user = %d\n", url_from_nonroom_user );
		fprintf( gyachrc, "highlight_friends = %d\n", highlight_friends );

		fprintf( gyachrc, "allow_pm_buzzes = %d\n", allow_pm_buzzes );
		if (file_transfer_server) {
			fprintf( gyachrc, "file_transfer_server = %s\n", file_transfer_server );
		}
		fprintf( gyachrc, "buddy_image_upload_method = %d\n", buddy_image_upload_method );

		fclose( gyachrc );
	}
}

int chars_match( char *str1, char *str2 ) {
	int i = 0;

	// DBG( 21, "chars_match( '%s', '%s' )\n", str1, str2 );

	/* if the 2nd string is longer then it's can't be the same word */
	if ( strlen( str2 ) > strlen( str1 ) )
		return( 0 );

	while(( *str1 ) && ( *str2 ) && ( *str1 == *str2 )) {
		i++;
		str1++;
		str2++;
	}

	// DBG( 22, "RETURN chars_match() == %d\n", i );

	return( i );
}

char *find_file_best_match( char *partial ) {
	static char best_match[513] = "";
	int matches = 0;
	char dir[513] = "";
	char *ptr;
	char *lptr;
	char *dptr;
	char lcp[513] = "";  /* longest common part */
	DIR *d;
	struct dirent *dp;
	struct stat sbuf;


	strncpy( dir, partial, 511 );
	ptr = dir + strlen( dir ) - 1;
	while(( ptr >= dir ) &&
		( *ptr != '/' )) {
		ptr--;
	}

	if ( ptr < dir ) {
		/* didn't find any / chars, so search current directory */
		d = opendir( "./" );
		ptr = dir;
	} else {
		/* found a / so set to zero and search that directory */
		*ptr = '\0';
		ptr++;
		if ( dir[0] ) {
			d = opendir( dir );
		} else {
			d = opendir( "/" );
		}
	}

	/* return if couldn't open directory */
	if ( d == NULL ) {
		strncpy( best_match, partial , 511);
		return( best_match );
	}

	/* do the search */
	while( 1 ) {
		dp = readdir( d );

		if ( ! dp ) {
			break;
		}
		if ( ! strncmp( dp->d_name, ptr, strlen( ptr ))) {
			/* this file matches */
			matches++;
			if ( lcp[0] ) {
				/* this isn't our first match */
				lptr = lcp;
				dptr = dp->d_name;
				while(( *lptr ) && ( *dptr ) &&
					( *lptr == *dptr )) {
					lptr++;
					dptr++;
				}
				if ( *lptr ) {
					/* truncate longest at this point */
					*lptr = '\0';
				}
			} else {
				/* this is our first match so it's our longest */
				strncpy( lcp, dp->d_name, 511 );
			}
		}
	}
	closedir( d );

	if ( strchr( partial, '/' )) {
		if ( dir[0] ) {
			strncpy( best_match, dir , 511);
		} else {
			strncpy( best_match, "/", 4 );
		}
	} else {
		strncpy( best_match, "./" , 4);
	}
	if ( best_match[ strlen( best_match ) - 1 ] != '/' ) {
		strcat( best_match, "/" );
	}
	strcat( best_match, lcp );

	/* check if this is a dir and if so and matches == 1, append a / */
	if ( ! stat( best_match, &sbuf )) {
		if ( S_ISDIR(sbuf.st_mode)) {
			strcat( best_match, "/" );
		}
	}

	return( best_match );
}

/*
 * fixme, this function needs cleaning up probably
 */
char *find_tab_best_match( char *partial ) {
	static char best_match[480] = "";
	static char last_part[480] = "";
	char part[480] = "";
	GList *last_user = tab_user;
	gchar *tmp_str;
	int row;

	// DBG( 11, "find_tab_best_match( '%s' )\n", partial );

	if ( ! partial ) {
		return( "" );
	}

	/* bounds checking */
	if ( ! partial[0] ) {
		best_match[0] = '\0';
		last_part[0] = '\0';
		return( best_match );
	}

	lower_str( best_match );
	strncpy( part, partial, 478 );

	/* IF we haven't been in here before */
	/* OR this part isn't equal to either the last part or the best_match */
	if (( ! last_part[0] ) ||
		(( strcasecmp( part, last_part )) &&
		 ( strcasecmp( part, best_match )))) {
		best_match[0] = '\0';
		strncpy( last_part, part, 478);
		tab_user = full_list;
		last_user = full_list;
	} else {
		/* we are still trying to find a match for the last part so revert */
		strncpy( part, last_part, 478 );
		if (( tab_user ) &&
			( ! strcasecmp( tab_user->data, best_match ))) {
			tab_user = g_list_next( tab_user );
		}
	}

	/* IF we're past the end start over */
	if ( ! tab_user )
		tab_user = full_list;

	/* go through the chatter list looking for the next match */
	while(( tab_user ) &&
		  ( strncasecmp( tab_user->data, part, strlen( part )))) {
		tab_user = g_list_next( tab_user );
	}

	/* did we hit the end of the list without finding any more matches */
	/* after starting at the beginning.  if so, return */
	if (( ! tab_user ) &&
		( last_user == full_list )) {
		strncpy( best_match, part, 478 );
		tab_user = full_list;
		return( best_match );
	} 

	if ( ! tab_user ) {
		/* if we didn't find a match, then start from the beginning again */
		tab_user = full_list;

		while(( tab_user ) &&
			  ( tab_user != last_user ) &&
			  ( strncasecmp( tab_user->data, part, strlen( part )))) {
			tab_user = g_list_next( tab_user );
		}
	}

	if ((( tab_user ) &&
		 ( strncasecmp( tab_user->data, part, strlen( part )))) ||
		( ! tab_user )) {
		strncpy( best_match, part, 478 );
		tab_user = full_list;
	} else {
		strncpy( best_match, tab_user->data, 478 );
	}

	if (( row = find_user_row( best_match )) > -1 ) {
		char *stringgy=NULL;
		gtk_tree_model_get(gtk_tree_view_get_model(chat_users), &chat_user_iter, GYQUAD_COL2,&tmp_str , -1);
		stringgy=strdup(tmp_str);
		strncpy( best_match, stringgy, 478);
		free(stringgy);
		g_free(tmp_str);
	}
	
	if ( ! best_match[0] ) {
		strncpy( best_match, partial, 478 );
	}

	return( best_match );
}

void build_tab_complete_list() {

	/* free the list before we rebuild */
	if ( full_list ) {
		full_list = gyach_g_list_free( full_list );
	}

	/* add the room users, people that just left, and friends to the list */
	full_list = gyach_g_list_copy( user_list );
		/* Force creation of list if one was not created: PhrozenSmoke */ 
		if (!full_list) {full_list=g_list_append( full_list, strdup("gyach-e"));}
	full_list = gyach_g_list_merge( full_list, gyach_g_list_copy( left_list ));
	full_list = gyach_g_list_merge( full_list, gyach_g_list_copy( friend_list));

	/* add things like aliases, built-in commands, etc. */

	/* Disabled by PhrozenSmoke, i dont like emotes in the tab completion list */ 
	/* append_aliases_to_list( full_list ); */ 

	append_commands_to_list( full_list );

	/* sort the list before we're done */
	full_list = g_list_sort( full_list, (GCompareFunc)gstrcmp );
	full_list = g_list_first( full_list );
	tab_user = NULL;
}

void tab_complete() {
	const char *text = gtk_entry_get_text( GTK_ENTRY(chat_entry));
	char new[512];
	char *ptr;
	char *dest;

	// DBG( 11, "tab_complete()\n" );

	if ( ! strlen( text ))
		return;

	strncpy( new, text, 510 );
	ptr = new + strlen( new ) - 1;

	while(( ptr > new ) && ( ! isspace( *ptr )))
		ptr--;

	if ( isspace( *ptr ))
		ptr++;


	if ( *ptr == '!' ) {
		/* tab completion of filename */
		ptr++;
		if ( *ptr == '!' ) {
			ptr++;
		}
		dest = ptr;
		strncpy( dest, find_file_best_match( ptr ), 511);
	/* } else if (( ptr != new ) &&
		( *ptr == '/' )) {
		dest = ptr;
		strncpy( dest, find_file_best_match( ptr ), 511); */
	} else {
		/* tab completion of username/alias/command/etc. */
		if (! full_list) {build_tab_complete_list();}
		dest = ptr;
		lower_str( ptr );
		strncpy( dest, find_tab_best_match( ptr ), 511);
	}

	gtk_entry_set_text( GTK_ENTRY(chat_entry), new );
	gtk_editable_set_position(GTK_EDITABLE(chat_entry), strlen(new));
}


void io_callback(gpointer data, gint source, GdkInputCondition condition)
{
	/* Tell the library to retrieve any newly available data */
	if (condition == GDK_INPUT_READ)
	{
		if ( ymsg_recv_data( ymsg_sess )) {
			show_yahoo_packet(); 
		}

	} else {
		/* invalid/unknown condition, so disconnect */
		on_disconnect_activate( NULL, NULL );
	}
}

int ping_callback( guint data ) {
	if ( logged_in )
		ymsg_ping( ymsg_sess );

	return( 1 );
}


int login_to_yahoo_chat( void ) {
	int result;

	logged_in=0;
	if (login_invisible) {cmd_away("12");} else {cmd_away("0");}

	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
		"Opening Connection to chat server." );

	snprintf( ymsg_sess->error_msg, 5, "%s","" );
	using_web_login=0;
	ycht_only=0;

	lower_str(ymsg_sess->user);

	if (! strcasecmp(ymsg_sess->host, "wcs2.msg.dcn.yahoo.com")) {using_web_login=1;}
	if (! strcasecmp(ymsg_sess->host, "wcs1.msg.dcn.yahoo.com")) {using_web_login=1;}

	if (! strncasecmp(ymsg_sess->host, "WEB LOGIN", 9)) {		
		if (strstr(ymsg_sess->host, "WEB LOGIN 2")) {
			strncpy(ymsg_sess->host, "wcs2.msg.dcn.yahoo.com", 60); 
		} else {
			strncpy(ymsg_sess->host, "wcs1.msg.dcn.yahoo.com", 60); 
		}
		using_web_login=1;
	}

	if (! strncasecmp(ymsg_sess->host, "YCHT", 4))  {
		using_web_login=1;
		ycht_only=1;
		if (strstr(ymsg_sess->host, "YCHT 2")) {
			strncpy(ymsg_sess->host, "jcs1.chat.dcn.yahoo.com", 60); 
		}
		else if (strstr(ymsg_sess->host, "YCHT 3")) {
			strncpy(ymsg_sess->host, "jcs2.chat.dcn.yahoo.com", 60); 
		}
		else if (strstr(ymsg_sess->host, "YCHT 4")) {
			strncpy(ymsg_sess->host, "jcs3.chat.dcn.yahoo.com", 60); 
		} else {
			strncpy(ymsg_sess->host, "jcs.chat.dcn.yahoo.com", 60); 
		}
	}

	connect_time = time(NULL);

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nSTARTING CONNECTION - CONNECTION SETTINGS:\nUsing Web Login Method: %s\nUser: %s\nHost: %s\nPort: %d\nInitial Chat Room: %s\nPacket Debugging: %s\nInitially Invisible: %s\n\n", ctime(&time_llnow), using_web_login?"Yes":"No", ymsg_sess->user, ymsg_sess->host, ymsg_sess->port?ymsg_sess->port:5050, ymsg_sess->req_room, ymsg_sess->debug_packets?"Yes":"No", login_invisible?"Yes":"No");
				fflush( capture_fp );
			}

	if (using_web_login) { 
		ycht_set_host("jcs.chat.dcn.yahoo.com");
		if (strstr(ymsg_sess->host, "wcs2")) {
			ycht_set_host("jcs2.chat.dcn.yahoo.com");
		}

		if (ycht_only) {
			ycht_set_host(ymsg_sess->host);
				if (strstr(ymsg_sess->host, "jcs1") || strstr(ymsg_sess->host, "jcs3")) {
					strncpy(ymsg_sess->host, "wcs2.msg.dcn.yahoo.com", 60);
				} else { strncpy(ymsg_sess->host, "wcs1.msg.dcn.yahoo.com", 60); }
			}
		result=yahoo_web_login( ymsg_sess );
		if (! result) {
				ymsg_sess->sock=-1;
				handle_dead_connection (0,1,0,_("Could not connect to Yahoo!"));
				return( 0 );
						 }
								}

		/* a little sanity checking for the port number */ 
	if (ymsg_sess->port<20) {
		ymsg_sess->port = YMSG_CHAT_PORT;
	}
	if (ymsg_sess->port>8002) {
		ymsg_sess->port = YMSG_CHAT_PORT;
	}


	if ( use_proxy ) {
		strncpy( ymsg_sess->proxy_host, proxy_host, 127 );
		ymsg_sess->proxy_port = proxy_port;
	} else {
		/* make sure this is empty */
		ymsg_sess->proxy_host[0] = '\0';
		ymsg_sess->proxy_port = 0;
	}

	if (ycht_only) {result=1;}
	else { result = ymsg_open_socket( ymsg_sess ); }

	/* if we connected */
	if ( result ) {		

		gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
		gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
			"Requesting Login Key." );


/* Below, disabled by PhrozenSmoke - too many memory leaks !, regular socket polling is used
    instead. Plus regular socket polling gives us better flow control against flooders/booters */

/* 
#ifdef OS_WINDOWS
		 ymsg_sess->io_callback_tag = gdk_input_add( ymsg_sess->sock,
			GDK_INPUT_READ, io_callback, (gpointer)0 ); 
#endif
*/

		logged_in = 1;
		if (! using_web_login) {ymsg_request_key( ymsg_sess ); }
		gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );

		if (! using_web_login) { sleep( 1 ); }
		else {yahoo_web_login_finish(ymsg_sess);}

		ymsg_sess->ping_callback_tag =
			gtk_timeout_add( ping_minutes * 60 * 1000,
				(void *)ping_callback, NULL );

		set_menu_connected( 1 );
		return( 1 );
	} else {
		ymsg_sess->sock=-1;
		handle_dead_connection (0,1,0, _("Could not connect to Yahoo!"));
		return( 0 );
	}

	return( 0 );
}

void set_status_room_counts( void ) {
	int users;
	int ig_users;
	int mu_users;
	char buf[156];
	char tmp[21];

	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	users = chat_user_count;
	if (is_conference) {
	snprintf(buf ,94, "%s: %s  (%d ",_("Conference Room"), ymsg_sess->room, users ); 
				  } else {
	snprintf(buf , 94, "%s: %s  (%d ",_("Current Room"), ymsg_sess->room, users ); 
					   }
	strncat(buf,_("users"), 18);

	ig_users = ignore_count_in_room();
	if ( ig_users ) {
		snprintf( tmp, 20, ", %d %s", ig_users,_("ignored")); 
		strcat( buf, tmp );
	}

	mu_users = mute_count_in_room();
	if ( mu_users ) {
		snprintf( tmp, 20, ", %d %s", mu_users,_("muted") ); 
		strcat( buf, tmp );
	}

	strcat( buf, ")" );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid, buf );
}

int check_clicked_text( char *text ) {
	char beginning[72];
	char url[320];
	char *ptr;
	regex_t url_regex;
	int is_command=0;
#ifdef USE_PTHREAD_CREATE
	pthread_t prof_thread;
#endif

	if ( ! text ) {	return( 0 );}

	strncpy( url, text, 317 );
	g_free( text );
	ptr = NULL;
	/* match anything with 3 strings separated by periods */
	regcomp( &url_regex, URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB );
	if (( strstr( url, "http://" )) || ( strstr( url, "https://" )) || 
		( strstr( url, "ftp://" )) || 
		( ! regexec( &url_regex, url, 0, NULL, 0 ))) {
		ptr = strdup( url );
	} else if ( ! strcasecmp( url, "gyach" )) {
		ptr = strdup( GYACH_URL ); 
	} else if  ( strstr( url, "y-mail://" ))  {
		snprintf( url, 10, "%s", "/mail" );
		ptr = strdup( url );
		is_command=1;
	} else {
		/* if ends in a : then strip it */
		if (( url[strlen(url)-1] == ':' ) ||
			( url[strlen(url)-1] == ',' )) {
			url[strlen(url)-1] = '\0';
		}

		/* check to see if highlighted text is actually a username */
		if ( find_user_row( url ) >= 0 ) {
			strncpy( beginning, url, 70 );
			lower_str(beginning);
			snprintf( url, 318, "/profile %s", beginning );
			ptr = strdup( url );
			is_command=1;
		}
	}
	regfree( &url_regex );
	if (is_command)  {
		chat_command(url);
		free(ptr);
		return( 0 );
	}

	if ( ptr ) {
#ifdef USE_PTHREAD_CREATE
		pthread_create( &prof_thread, NULL, display_url, (void *)ptr );
#else
		display_url( (void *)ptr );
#endif
	}

	return( 0 );
}


int search_chat_text( int case_sensitive ) {
	GtkTextBuffer *text_buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkWidget *tmp_widget;
	int pos;
	int len;
	int max_pos;
	char *str;
	int started_at = search_pos;

	tmp_widget = ct_widget();

	if (( search_pos == -1 ) &&
		( ! search_text )) {
		/* initial search so popup window */
	} else {
		/* search again starting at search_pos + 1 */
		pos = search_pos + 1;
		len = strlen( search_text );

		/* if they have both panes visible, then scroll the top one */
		if ( gtk_paned_get_position( GTK_PANED( vpaned ))) {
			tmp_widget = top_text;
		}
		text_buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW(tmp_widget));
		max_pos = gtk_text_buffer_get_char_count( text_buffer ) - len;
		if ( pos > max_pos ) {
			return( 0 );
		}

		gtk_text_buffer_get_iter_at_offset( text_buffer, &start, pos );
		gtk_text_buffer_get_iter_at_offset( text_buffer, &end, pos + len );
		str = gtk_text_buffer_get_text( text_buffer, &start, &end, 0 );

		if ( case_sensitive ) {
			while(( strcmp( search_text, str )) &&
				( pos < max_pos )) {
				g_free( str );
				pos++;
				gtk_text_iter_forward_char( &start );
				gtk_text_iter_forward_char( &end );
				str = gtk_text_buffer_get_text( text_buffer, &start, &end, 0 );

			}
			if ( ! strcmp( search_text, str )) {
				search_pos = pos;
				gtk_text_buffer_move_mark_by_name( text_buffer,
					"insert", &start );
				gtk_text_buffer_move_mark_by_name( text_buffer,
					"selection_bound", &end );
				gtk_text_view_scroll_to_iter( GTK_TEXT_VIEW(tmp_widget),
					&start, 0.0, 0, 0.0, 0.0 );

				return( 1 );
			} else if ( started_at != -1 ) {
				/* loop around if we didn't start at the top of the window */
				search_pos = -1;
				search_chat_text( case_sensitive );
			}
		} else {
			while(( strcasecmp( search_text, str )) &&
				( pos < max_pos )) {
				g_free( str );
				pos++;
				gtk_text_iter_forward_char( &start );
				gtk_text_iter_forward_char( &end );
				str = gtk_text_buffer_get_text( text_buffer, &start, &end, 0 );

			}
			if ( ! strcasecmp( search_text, str )) {
				search_pos = pos;
				gtk_text_buffer_move_mark_by_name( text_buffer,
					"insert", &start );
				gtk_text_buffer_move_mark_by_name( text_buffer,
					"selection_bound", &end );
				gtk_text_view_scroll_to_iter( GTK_TEXT_VIEW(tmp_widget),
					&start, 0.0, 0, 0.0, 0.0 );
				return( 1 );
			} else if ( started_at != -1 ) {
				/* loop around if we didn't start at the top of the window */
				search_pos = -1;
				search_chat_text( case_sensitive );
				
			}
		}
		g_free( str );
	}

	return( 0 );
}

#define URL_CHAR_LIST "abcdefghijklmnopqrstuvwxyzzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\\.?!/%&_:~=-#"

gchar *get_word_at_pos( GtkTextView *text_view, gint x, gint y ) {
	GtkTextBuffer *text_buffer;
	GtkTextIter start;
	GtkTextIter iter;
	GtkTextIter end;
	gchar *ch;
	gchar *result;
	int not_at_end = 1;

	text_buffer = gtk_text_view_get_buffer( text_view );

	gtk_text_view_get_iter_at_location( text_view, &start, x, y );
	gtk_text_view_get_iter_at_location( text_view, &iter, x, y );

	if ( gtk_text_iter_backward_char( &start )) {
		not_at_end = 1;
		ch = gtk_text_buffer_get_text( text_buffer, &start, &iter, 0 );
		while(( strchr( URL_CHAR_LIST, *ch )) &&
			( not_at_end )) {
			if ( gtk_text_iter_backward_char( &start )) {
				g_free( ch );
				gtk_text_iter_backward_char( &iter );
				ch = gtk_text_buffer_get_text( text_buffer, &start, &iter, 0 );
			} else {
				not_at_end = 0;
			}
		}
		g_free( ch );
		if ( not_at_end ) {
			gtk_text_iter_forward_char( &start );
		}
	}

	gtk_text_view_get_iter_at_location( text_view, &iter, x, y );
	gtk_text_view_get_iter_at_location( text_view, &end, x, y );
	if ( gtk_text_iter_forward_char( &end )) {
		not_at_end = 1;
		ch = gtk_text_buffer_get_text( text_buffer, &iter, &end, 0 );
		while(( strchr( URL_CHAR_LIST, *ch )) &&
			( not_at_end )) {
			if ( gtk_text_iter_forward_char( &end )) {
				g_free( ch );
				gtk_text_iter_forward_char( &iter );
				ch = gtk_text_buffer_get_text( text_buffer, &iter, &end, 0 );
			} else {
				not_at_end = 0;
			}
		}
		g_free( ch );
		if ( not_at_end ) {
			gtk_text_iter_backward_char( &end );
		}
	}

	result = gtk_text_buffer_get_text( text_buffer, &start, &end, 0 );

	/* strip off any CR or LF at the end of the result */
	while( result[strlen(result)-1] == '\n' ) {
		result[strlen(result)-1] = '\0';
	}

	return( result );
}


GdkCursor *create_hand_cursor() {
	/* Taken from an example straight out the Gtk docs */
  GdkCursor *cursor;
  GdkPixmap *source, *mask;
  GdkColor fg = { 0, 0, 0, 8500 }; /* dark blue */
  GdkColor bg = { 0, 229*256, 244*256, 255*256 }; /* light light blue/white, a little diversity? */
  source = gdk_bitmap_create_from_data (NULL, handcur1_bits,
	handcursor1_width, handcursor1_height);
  mask = gdk_bitmap_create_from_data (NULL, handcur1mask_bits,
  handcursor1_width, handcursor1_height);
  cursor = gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, 8, 8);
  gdk_pixmap_unref (source);
  gdk_pixmap_unref (mask);
   return cursor;
}


gint chat_motion_notify(GtkWidget * widget, GdkEventMotion * event) {
	GdkCursor *cursor = NULL;
	char *ch;
	regex_t url_regex;
	int x, y;
	GdkWindow *win;
	gint buf_x, buf_y;
	GdkModifierType state;
	int valid_link = 0;

	if (event->is_hint) {
		gdk_window_get_pointer(event->window, &x, &y, &state);
	} else {
		x = event->x;
		y = event->y;
		state = event->state;
	}

	/* don't do anything if there is a mouse button pressed */
	if ((state & GDK_BUTTON1_MASK) ||
		(state & GDK_BUTTON3_MASK)) {
		return( FALSE );
	}

	gtk_text_view_window_to_buffer_coords( GTK_TEXT_VIEW(widget),
		GTK_TEXT_WINDOW_TEXT, x, y, &buf_x, &buf_y );
	ch = get_word_at_pos( GTK_TEXT_VIEW(widget), buf_x, buf_y );

	if (( ch ) &&
		( ch[0] )) {
		regcomp( &url_regex, URL_REGEX, REG_EXTENDED | REG_ICASE | REG_NOSUB );
		if (( strstr( ch, "http://" )) || ( strstr( ch, "https://" )) || 
			( strstr( ch, "ftp://" )) || ( strstr( ch, "y-mail://" )) || 
			( ! regexec( &url_regex, ch, 0, NULL, 0 ))) {

			valid_link = 1;
		} else if (( ! strcasecmp( ch, "gyach" )) ||
				   ( ! strcasecmp( ch, "curphoo" )) ||
				   ( ! strcasecmp( ch, "curfloo" )) ||
				   ( ! strcasecmp( ch, "mybitch" )) ||
				   ( ! strcasecmp( ch, "ncchat" )) ||
				   ( ! strcasecmp( ch, "qtchat" )) ||
				   ( ! strcasecmp( ch, "yahoonix" )) ||
				   ( ! strcasecmp( ch, "yahscrewed" )) ||
				   ( ! strcasecmp( ch, "ychat" ))) {
			valid_link = 1;
		} else {
			/* check if it's a username */
			/* if ends in a : then strip it */
			if (( ch[strlen(ch)-1] == ':' ) ||
				( ch[strlen(ch)-1] == ',' )) {
				ch[strlen(ch)-1] = '\0';
			}

			/* check to see if highlighted text is actually a username */
			if ( find_user_row( ch ) >= 0 ) {
				valid_link = 1;
			}
		}

		regfree( &url_regex );
		g_free( ch );
	}

	if ( valid_link ) {
		/* cursor = gdk_cursor_new(GDK_HAND2); */ /* old ugly cursor */
		cursor=create_hand_cursor();  /* changed, PhrozenSmoke */
		win = gtk_text_view_get_window( GTK_TEXT_VIEW(widget),
			GTK_TEXT_WINDOW_TEXT );
		gdk_window_set_cursor( win, cursor );
		gdk_cursor_destroy(cursor);
	} else {
		win = gtk_text_view_get_window( GTK_TEXT_VIEW(widget),
			GTK_TEXT_WINDOW_TEXT );
		gdk_window_set_cursor( win, NULL );
	}

	return( TRUE );
}

void gyach_comment( YMSG_SESSION *session, char *text ) {
	char *alias=NULL;

		if (( my_status ) &&
			( my_status != 12 ) &&
			( pm_brings_back )) {
			cmd_away( "0" );
											  }

	ymsg_comment( session, _b2loc(text) );
	alias=get_screenname_alias(get_chat_profile_name());
	set_last_comment(get_chat_profile_name(), text);
	if ( chat_timestamp ) {append_timestamp(chat_window, NULL);	}
	/* now print it to our window */
	snprintf( gycomment_buf, 1662, "%s%s%s%s%s: %s%s %s\n",
		YAHOO_STYLE_BOLDON, "\033[#2AA6B1m", 				
		alias,
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF,
		YAHOO_COLOR_BLACK, text, YAHOO_COLOR_BLACK );
	append_to_textbox_color( chat_window, NULL, gycomment_buf );
	g_free(alias);

	auto_away_time = time(NULL) + ( auto_away * 60 );
}

void gyach_emote( YMSG_SESSION *session, char *text ) {
	char *alias=NULL;

		if (( my_status ) &&
			( my_status != 12 ) &&
			( pm_brings_back )) {
			cmd_away( "0" );
											  }

	if (!emote_no_send) {ymsg_emote( session, _b2loc(text) );}
	alias=get_screenname_alias(get_chat_profile_name());
	set_last_comment(get_chat_profile_name(), text);

	if ( chat_timestamp ) {append_timestamp(chat_window, NULL);	}
	snprintf( gycomment_buf, 1662, "%s%s%s%s%s %s%s %s\n",
		YAHOO_STYLE_BOLDON, "\033[#2AA6B1m", alias,
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF,
		"\033[#8D71BEm", text, YAHOO_COLOR_BLACK );
	append_to_textbox( chat_window, NULL, gycomment_buf );
	g_free(alias);
	auto_away_time = time(NULL) + ( auto_away * 60 );
}

void gyach_think( YMSG_SESSION *session, char *text ) {
	char *alias=NULL;

		if (( my_status ) &&
			( my_status != 12 ) &&
			( pm_brings_back )) {
			cmd_away( "0" );
											  }

	ymsg_think( session, _b2loc(text) );
	alias=get_screenname_alias(get_chat_profile_name());
	set_last_comment(get_chat_profile_name(), text);

	if ( chat_timestamp ) {append_timestamp(chat_window, NULL);	}

	snprintf( gycomment_buf, 1662,  "%s%s%s%s%s %s. o O ( %s )%s\n",
		YAHOO_STYLE_BOLDON, "\033[#2AA6B1m", alias,
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF,
		"\033[#BD7981m" , text, YAHOO_COLOR_BLACK );
	append_to_textbox( chat_window, NULL, gycomment_buf );
	g_free(alias);
	auto_away_time = time(NULL) + ( auto_away * 60 );
}

void gyach_away( YMSG_SESSION *session, char *text ) {
	char buf[280];
	char *mptr;
	int skipit=0;

	if (!logged_in) {return;}
	if ( (! force_busy_icon) && (! force_idle_icon) ) {skipit=1;} 
	ymsg_away( session, _b2loc(text) );

	if (!show_my_status_in_chat) {emote_no_send=1;}

	/* now tell people we're away */
	mptr = strchr( text, ':' );
	if ( ! mptr )
		return;

    mptr++;

	/* fixed, dont sent emotes to chat/conference when we go INVISIBLE */
	if ( atoi( text ) == 12) { return; }
	if ( atoi( text ) == 10 ) {
		/* a custom '10' status that is neither 'busy' or 'idle', so 
		dont broadcast in the room */
		if (skipit) {emote_no_send=1;}
	}

	if ( atoi( text ) == 10 ) {
		snprintf( buf,274, "%s ( %s",  locale_status?_("is away"):"is away",  mptr ); 
		strncat(buf," ).", 4);
	} else if ( atoi( text ) == 999 ) {
		snprintf( buf,128, "%s ( %s ).",  locale_status?_("is away"):"is away",  _("Idle") );
	} else {
		snprintf( buf,156, "%s ( %s ).",  locale_status?_("is away"):"is away",  _(away_msgs[ atoi( text ) ]) );
	}
	gyach_emote( session, buf );
	emote_no_send=0;
}

void gyach_back( YMSG_SESSION *session ) {

	if (!logged_in) {return;}
	ymsg_back( session );
	if (!show_my_status_in_chat) {emote_no_send=1;}
	if (locale_status) { gyach_emote( session, _("is back") ); }
	else {gyach_emote( session, "is back" ); }
	emote_no_send=0;
}
