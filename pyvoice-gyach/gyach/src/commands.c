/*****************************************************************************
 * commands.c
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
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#include "config.h"

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include "gyach.h"
#include "aliases.h"
#include "callbacks.h"
#include "commands.h"
#include "friends.h"
#include "gyach_int.h"
#include "history.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "bootprevent.h"
#include "conference.h"
#include "trayicon.h"   /* added, PhrozenSmoke */
#include "webcam.h"   /* added, PhrozenSmoke */
#include "profname.h"
#include "gytreeview.h"

extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 
extern gchar *_b2loc(char *some);
extern int ymsg_p2p_ask( YMSG_SESSION *session, char *yahid, int probe);
extern gchar **choose_pixmap(char *user);
extern void load_myyahoo_URL(char *turl);
extern int immed_rejoin;
extern int in_a_chat;
extern int force_busy_icon;
extern int force_idle_icon;

extern int ignore_ymsg_online;
extern int come_back_from_invisible;

char *last_status_msg=NULL;

char *find_whitespace( char *str ) {
	char *ptr = str;

	while(( *ptr ) && ( ! isspace( *ptr )))
		ptr++;

	return( ptr );
}

char *skip_whitespace( char *str ) {
	char *ptr = str;

	while(( *ptr ) && ( isspace( *ptr )))
		ptr++;

	return( ptr );
}


void cmd_contact_info( char *who ) {
	char continfo[448]="";
	snprintf(continfo, 446, "%s\t%s\t%s\t%s\t%s\t%s\t", 
		contact_first?contact_first:"", 
		contact_last?contact_last:"", 
		contact_email?contact_email:"",
		contact_home?contact_home:"", 
		contact_work?contact_work:"", 
		contact_mobile?contact_mobile:""

		);
	ymsg_contact_info(ymsg_sess, who, continfo);
}



void cmd_booter( char *who ) {
	int i=0;
	while (i<6) {
		preemptive_strike(who);
		i++;
			}
}

void cmd_leave_room( char *room ) {
	if (!is_conference) {in_a_chat=1;}
	ymsg_leave_chat( ymsg_sess );
	if (!is_conference) {in_a_chat=0;}
}

void cmd_rejoin( char *room ) {
	if (in_a_chat) {
	strncpy( ymsg_sess->req_room, ymsg_sess->room, 62 );
	set_current_chat_profile_name( get_chat_profile_name());
	ymsg_leave_chat( ymsg_sess );
	immed_rejoin=1;
			    }
}


void cmd_viewcam( char *who ) {
	yahoo_webcam_get_feed(who );
}

void cmd_flooder_remove( char *who ) {
	remove_flooder(who );
}


void cmd_add_tmp_friend( char *who ) {
	add_temporary_friend(who );
}

void cmd_rem_tmp_friend( char *who ) {
	remove_temporary_friend(who );
}


void cmd_join( char *room ) {
	ymsg_conference_leave(ymsg_sess);
	strncpy( ymsg_sess->req_room, room, 62 );
	if (in_a_chat) 		{
		if (strcasecmp(get_chat_profile_name(), get_current_chat_profile_name())) {
				/* We are in a chat room under one name, trying
				to enter another room using another name...we must
				log off chat and do the whole 'ymsg_online' thing over */
			immed_rejoin=1;
			ymsg_leave_chat( ymsg_sess );
		} else 	{ymsg_join( ymsg_sess );} /* keeping the same profile name */
			    			} else {
		ymsg_online( ymsg_sess );
				     		}
}

void cmd_revoke_friend( char *who ) {
	char buf[300]="";
	ymsg_reject_buddy(ymsg_sess, who );
		snprintf(buf,298, "%s%s** %s '%s' **%s%s\n", YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED, _("Denied buddy list privileges for"), who, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
			append_to_textbox( chat_window, NULL, buf );
}


void cmd_conf_start( char *room ) {
	use_multpm_settings=0;
	open_make_conference_window();
}

void cmd_conf_invite( char *user ) {
	if (is_conference) {
	char *msg=NULL;
	ymsg_conference_addinvite(ymsg_sess,user,msg);
				  }
}



void cmd_image( char *filename ) {
	char buf[192];
	snprintf(buf, 189, "\n%s",filename);
	strcat(buf,"\n");
	append_to_textbox( chat_window, NULL, buf );
	ct_append_pixmap( filename,NULL );
	snprintf(buf,8, "%s", "\n\n");
	append_to_textbox( chat_window, NULL, buf );
}


void cmd_goto( char *user ) {
	set_current_chat_profile_name( get_default_profile_name());
	if (in_a_chat) {
		ymsg_goto( ymsg_sess, user );
					} else {
		ignore_ymsg_online=1;
		ymsg_online( ymsg_sess);
		ymsg_goto( ymsg_sess, user );
							 }
}

void cmd_follow( char *user ) {
	char buf[129] = "";
	int r;

	if ( *user ) {
		/* check to make sure we don't want to display who we're following */
		if ( strcmp( "?", user )) {
			/* make sure they don't try to follow themselves */
			if ( !strcasecmp( user, ymsg_sess->user )) {
				snprintf( buf, 127, 
					"%s** %s **%s\n",
					YAHOO_COLOR_RED, _("You can't follow yourself."), YAHOO_COLOR_BLACK );
				append_to_textbox( chat_window, NULL, buf );
				return;
			}

			r = find_user_row( user );
			if ( r < 0 ) {
				snprintf( buf, 127, 
					"** %s %s%s%s \n",
					_("User"), YAHOO_COLOR_BLUE, user, YAHOO_COLOR_BLACK );
				strcat(buf,_("is not in this room.  User can not be followed."));
				strcat(buf," **\n");
				append_to_textbox( chat_window, NULL, buf );
				return;
			}

			if ( follow_user ) {
				if (!strcasecmp(follow_user, user)) { /* same as last followed user, turn off */
							free( follow_user );
							follow_user = NULL;
																} else {
							free( follow_user );
							follow_user = strdup( user );
																		  }
									} else {follow_user = strdup( user );}
		}
	} else {
		if ( follow_user ) {
			free( follow_user );
		}
		follow_user = NULL;
	}

	if ( follow_user ) {
		snprintf( buf, 127, "** %s: %s%s%s **\n",
			_("Following user"), YAHOO_COLOR_BLUE, follow_user, YAHOO_COLOR_BLACK );
	} else {
		snprintf( buf,127,  "** %s%s%s **\n",
			YAHOO_COLOR_RED, _("Not currently following anyone."), YAHOO_COLOR_BLACK );
	}
	append_to_textbox( chat_window, NULL, buf );
}

void cmd_invite( char *user ) {
	ymsg_invite( ymsg_sess, user, ymsg_sess->room );
}

void cmd_cls() {
	ct_clear();
}

void cmd_tell( char *args ) {
	char *ptr;

	args = skip_whitespace( args );
	ptr = args;

	if ( strlen( ptr )) {
		/* they gave a user & message so send message */
		/* format is "tell dummytext\001user\001message" */
		ptr = find_whitespace( ptr );
		*ptr = '\0';
		ptr++;

		ymsg_pm( ymsg_sess, args, _b2loc(ptr) );
	} 
}

void cmd_away( char *msg ) {
	GtkWidget *tmp_widget;
	GtkWidget *cw = chat_window;
	char buf[264];
	char last_stat[256]="";
	int len;

	if (msg==NULL) {return;}
	if (last_status_msg) {free(last_status_msg); last_status_msg=NULL;}
	snprintf(last_stat,emulate_ymsg6?254:68, "%s",msg);
	last_status_msg=strdup(last_stat);

	len = strlen( msg );

	if (my_status==12) {come_back_from_invisible=1;}

	if ( len == 0 ) {
		if (( custom_away_message ) &&
			( strlen( custom_away_message )))
			msg = custom_away_message;
		else
			msg = "Be Right Back";
		my_status = 10;
	} else if (( len == 1 ) &&
		( isdigit( msg[0] ))) {
		my_status = atoi( msg );
	} else if (( len == 2 ) &&
		( isdigit( msg[0] )) &&
		( isdigit( msg[1] ))) {
		my_status = atoi( msg );
	} else if (( len == 3 ) &&     /* 999, idle      */
		( isdigit( msg[0] )) &&
		( isdigit( msg[1] )) && 
		( isdigit( msg[2] ))) {
		my_status = atoi( msg );
	} else {
		my_status = 10;
	}

	if ( my_status  ) {
		chatter_list_status( ymsg_sess->user, pixmap_status_away, "AW" );
	} else {
		chatter_list_status( ymsg_sess->user, choose_pixmap(ymsg_sess->user), "" );
	}

	if (( my_status <= 9 ) || ( my_status == 999 ) || 
		( my_status == 11 ) ||
		( my_status == 12 )) {
		if ( my_status ) {
			if (! locale_status || (my_status == 12) || (my_status == 999)) {
				if (my_status == 999) {	snprintf( buf, emulate_ymsg6?166:77,  "%03d:", my_status ); }
				else { snprintf( buf, emulate_ymsg6?261:77,  "%02d:", my_status ); }
				gyach_away( ymsg_sess, buf );
				 
					if ((my_status == 999) || ( my_status == 11 )) {
						/* idle, and auto-away */
						trayicon_set_status(TRAY_ICON_IDLE);
																						}
					else if  (my_status == 12) {
						/* invisible */
						trayicon_set_status(TRAY_ICON_INVISIBLE);
														  } 
					else {trayicon_set_status(TRAY_ICON_BUSY); }
						 																			 } else {
					/* send locale-translated statuses */
					if (my_status==11) { /* auto-away */
						force_busy_icon=0; 
						force_idle_icon=1;
						trayicon_set_status(TRAY_ICON_IDLE);
												 }
					else  {
						force_busy_icon=1; 
						force_idle_icon=0;
						trayicon_set_status(TRAY_ICON_BUSY);
							}
				snprintf( buf, emulate_ymsg6?261:77, "10:%s", _(away_msgs[ my_status ]) );
				gyach_away( ymsg_sess, buf );
							   																				  }
		} else {
			gyach_back( ymsg_sess );
			trayicon_set_status(TRAY_ICON_ONLINE_AVAILABLE);
		}

		switch( my_status ) {
			case 0: tmp_widget = lookup_widget( cw, "status_here" );
					break;
			case 1: tmp_widget = lookup_widget( cw, "status_be_right_back" );
					break;
			case 2: tmp_widget = lookup_widget( cw, "status_busy" );
					break;
			case 3: tmp_widget = lookup_widget( cw, "status_not_at_home" );
					break;
			case 4: tmp_widget = lookup_widget( cw, "status_not_at_my_desk" );
					break;
			case 5: tmp_widget = lookup_widget( cw, "status_not_in_the_office");
					break;
			case 6: tmp_widget = lookup_widget( cw, "status_on_the_phone" );
					break;
			case 7: tmp_widget = lookup_widget( cw, "status_on_vacation" );
					break;
			case 8: tmp_widget = lookup_widget( cw, "status_out_to_lunch" );
					break;
			case 9: tmp_widget = lookup_widget( cw, "status_stepped_out" );
					break;
			case 11: tmp_widget = lookup_widget( cw, "status_autoaway" );
					break;
			case 12: tmp_widget = lookup_widget( cw, "status_invisible" );
					break;
			case 999: tmp_widget = lookup_widget( cw, "status_idleaway" );
					break;
			default: tmp_widget = lookup_widget( cw, "status_here" );
		}

		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 1 );
	} else {
		snprintf( buf, emulate_ymsg6?261:77, "10:%s", msg );
		if ((force_busy_icon==0) && (force_idle_icon==0))  
			   {
			trayicon_set_status(TRAY_ICON_ONLINE_AVAILABLE);
				}  else {
					if (force_busy_icon) {trayicon_set_status(TRAY_ICON_BUSY);}
					if (force_idle_icon) {trayicon_set_status(TRAY_ICON_IDLE);}
						   }
		gyach_away( ymsg_sess, buf );
		tmp_widget = lookup_widget( cw, "status_custom" );
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 1 );
	}
	come_back_from_invisible=0;
}

void cmd_auto_away( char *value ) {
	char buf[128];

	if ( ! value[0] )
		return;

	auto_away= atoi( value );

	if ( ! auto_away)
		auto_away = 15;

	auto_away_time = time(NULL) + ( auto_away * 60 );

	write_config();

	snprintf( buf,125,  "*** %s %d %s ***",_("Auto-Away timer set to"), auto_away, _("minute(s)") );
	strcat(buf,"\n");
	append_to_textbox( chat_window, NULL, buf );
}

void cmd_think( char *msg ) {
	char buf[2048];

	strncpy( buf, msg, 1800 );
	if( send_avatar ) {
		strcat( buf, AVATAR_START );
		strcat( buf, send_avatar );
		strcat( buf, AVATAR_SEND_END );
#ifndef OS_WINDOWS
		if ( show_avatars ) {
			display_avatar( ymsg_sess->user, send_avatar );
		}
#endif
	}

	gyach_think( ymsg_sess, buf );
}

void cmd_emote( char *msg ) {
	char buf[2048];

	strncpy( buf, msg, 1800 );
	if( send_avatar ) {
		strcat( buf, AVATAR_START );
		strcat( buf, send_avatar );
		strcat( buf, AVATAR_SEND_END );
#ifndef OS_WINDOWS
		if ( show_avatars ) {
			display_avatar( ymsg_sess->user, send_avatar );
		}
#endif
	}

	gyach_emote( ymsg_sess, buf );
}

void cmd_connect() {
	on_connect_activate( NULL, NULL );
}

void cmd_disconnect() {
	on_disconnect_activate( NULL, NULL );
}

void cmd_date_time() {
	char buf[40];
	time_t time_llnow= time(NULL);
	snprintf(buf,3, "%s","\n");
	append_to_textbox( chat_window, NULL, buf );
	append_timestamp(chat_window, NULL);
	snprintf(buf,38, "%s\n", ctime(&time_llnow ));
	append_to_textbox( chat_window, NULL, buf );
}

void cmd_reconnect() {
	on_disconnect_activate( NULL, NULL );
	login_to_yahoo_chat();
}

void cmd_help( int full ) {
	/* re-wrote this to read from external .txt file, PhrozenSmoke */
	struct stat sbuf;
	int fd;
	int bytes;
	char filebuf[513];
	char help_url[256];
	char nwline[20];
	snprintf(help_url, 254, "%s/doc/gyache-help-short.txt", PACKAGE_DATA_DIR);

	if ( stat( help_url, &sbuf )) {
		/* file doesn't exist so exit */  
		snprintf(filebuf, 500, "** Could not load Help file '%s' ",help_url);
		strcat(filebuf," **\n");
		append_to_textbox_color( chat_window, NULL, filebuf );
		return;
	}
	fd = open( help_url, O_RDONLY, 0600 );
	if ( fd == -1 ) {
		/* can't open file */ 
		snprintf(filebuf, 500, "** Could not load Help file '%s' ",help_url);
		strcat(filebuf," **\n");
		append_to_textbox_color( chat_window, NULL, filebuf );
		return;
	}
	snprintf(nwline,18, "%s\n", YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, nwline );
	bytes = read( fd, filebuf, 512 );
	while( bytes ) {
		ct_append_fixed( filebuf, bytes);
		bytes = read( fd, filebuf, 512 );
	}
	close( fd );
	append_to_textbox_color( chat_window, NULL, nwline );
	ct_scroll_to_bottom();  
}

/* fixme, fill in the rest of this function */
void cmd_friend( char *arg ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( arg );
	end = find_whitespace( ptr );

	while( *end ) {
		*end = '\0';
		if ( find_friend( ptr )) {
			/* don't do this here, we can do this on the buddy list tab,
			and this helps avoid accidental deletion - PhrozenSmoke */
			/* remove_friend( ptr ); */ 			
		} else {
			add_friend( ptr );
		}

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	if ( find_friend( ptr )) {
			/* don't do this here, we can do this on the buddy list tab,
			and this helps avoid accidental deletion - PhrozenSmoke */
			/* remove_friend( ptr ); */ 			
	} else {
		add_friend( ptr );
	}
}

void cmd_mail() {
	/* This method changed by PhrozenSmoke, 
	we no longer try to automatically login because it wasn't
	really working anymore and it's a dumb idea to send 
	any password info out using an HTTP 'GET' request 
	in a web browser...instead, we can show a 'preview'
	of what's in our mailbox */

	char prof_url[64];
	GtkWidget *tmp_widget=NULL;
	snprintf(prof_url, 62, "%s", "http://my.yahoo.com/preview/pirymail.html?.pirf=yml");
	tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
	load_myyahoo_URL(prof_url);
	if (tmp_widget) {gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);}
	gdk_window_raise(chat_window->window );
}


void cmd_profile2( char *user ) {
	char *ptr;
/* #ifdef USE_PTHREAD_CREATE
	pthread_t prof_thread;
#endif */

	if ( profile_window ) {
		gtk_widget_destroy( profile_window );
		profile_window = NULL;
	}

	ptr = strdup( user );
	if (prof_text) {g_free(prof_text); prof_text=NULL;}

/* #ifdef USE_PTHREAD_CREATE
	pthread_create( &prof_thread, NULL, fetch_profile, (void *)ptr );
#else */
	fetch_profile( (void *)ptr );
	if (prof_text) {display_profile();}
/* #endif */
}



void cmd_profile( char *user ) {
	char prof_url[128];
	snprintf(prof_url, 127, "http://profiles.yahoo.com/%s", user);

	if (profile_viewer < 1) {  /* launch in browser */
		char *ptr=NULL;
		snprintf( prof_url, 128, "http://profiles.yahoo.com/%s", user );
		ptr = strdup( prof_url );
		display_url( (void *)ptr );
		return;
	}  else  {
		if (profile_viewer==2) {
			/* Old profile viewer window */
			cmd_profile2( user );
			return;
											} else  {
			/* use My Yahoo Tab */
		GtkWidget *tmp_widget=NULL;
		tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
		load_myyahoo_URL(prof_url);
		if (tmp_widget) {gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);}
		gdk_window_raise(chat_window->window );
														}
			}
}



void cmd_online( void ) {
	char buf[201] = "";
	time_t current_time;
	int hours;
	int minutes;
	int seconds;

	if ( connect_time ) {
		current_time = time(NULL);

		hours = (int)(( current_time - connect_time ) / 3600);
		minutes = (int)(( current_time - connect_time ) % 3600 ) / 60;
		seconds = (int)(( current_time - connect_time ) % 60 );

		snprintf( buf, 200,  "*** %s %d:%02d:%02d ***\n",
			_("online for") ,hours, minutes, seconds );
		append_to_textbox_color( chat_window, NULL, buf );
	} else {
		snprintf( buf, 200, "*** %s ***\n", _("You don't appear to be online at the moment.") );
		append_to_textbox_color( chat_window, NULL, buf );
	}
}

void cmd_url( char *url ) {
	char buf[400] = "";
	time_t current_time;
	int hours;
	int minutes;
	struct utsname un;

	if ( *url ) {
		strcat( buf, YAHOO_STYLE_URLON );
		strncat( buf, url, 350 );
		strcat( buf, YAHOO_STYLE_URLOFF );
		chat_command( buf );
	} else {
		current_time = time(NULL);

		hours = (int)(( current_time - connect_time ) / 3600);
		minutes = (int)(( current_time - connect_time ) % 3600 ) / 60;

		uname( &un );

		if ( connect_time ) {
			snprintf( buf, 398, 
				"/emote is using %sGYach Enhanced %s v.%s  [%s%s%s] : Online for %d:%02d under %s  %s.  GYach Enhanced supports webcams, faders, Y! 'nicknames', and %spY! Voice Chat%s (the FIRST Y! voice chat program for Linux.)",
				YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF, VERSION,
				YAHOO_STYLE_URLON, GYACH_URL, YAHOO_STYLE_URLOFF,
				hours, minutes, un.sysname, un.release, YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF);
			chat_command( buf );
		} else {
			snprintf( buf, 398,
				"%s%sGYach Enhanced %s v.%s  [%s%s%s]  (currently OFFline) under %s %s.   GYach Enhanced supports webcams, faders, Y! 'nicknames', and %spY! Voice Chat%s (the FIRST Y! voice chat program for Linux.)\n",
				YAHOO_COLOR_BLACK,
				YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF, VERSION,
				YAHOO_STYLE_URLON, GYACH_URL,  YAHOO_STYLE_URLOFF,
				un.sysname, un.release, YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF );
			append_to_textbox( chat_window, NULL, buf );
		}

	}
}

void cmd_alias( char *args ) {
	char tmp_str[256];
	char tmp_item[24];
	char *ptr;
	char *alias;
	char *def;
	int found;

	// DBG( 11, "cmd_alias( '%s' )\n", args );

	strncpy( tmp_str, args, 254 );
	ptr = tmp_str;
	ptr = skip_whitespace( ptr );

	/* find alias string and null-terminate it */
	alias = ptr;
	ptr = find_whitespace( ptr );
	*ptr = '\0';

	/* find args string */
	ptr++;
	ptr = skip_whitespace( ptr );
	def = ptr;

	found = alias_find( alias );

	if ( found == -1 ) {
		/* new alias so add item*/
		GtkTreeModel *freezer;
		/* delete the alias */
		freezer=freeze_treeview(GTK_WIDGET(chat_aliases));
		gtk_list_store_append(GTK_LIST_STORE(freezer), &alias_find_iter);
		strncpy( tmp_item, alias, 22 );
		gtk_list_store_set(GTK_LIST_STORE(freezer), &alias_find_iter,  
			GYTRIPLE_COL1, tmp_item, GYTRIPLE_COL2, def, 
			GYTRIPLE_COL3, def, -1);
		found=1;
		unfreeze_treeview(GTK_WIDGET(chat_aliases), freezer);
	}

	/* now save list */
	save_aliases();

	// DBG( 12, "RETURN cmd_alias()\n" );
}

void cmd_unalias( char *args ) {
	char tmp_str[257];
	char *ptr;
	char *alias;
	int found;

	// DBG( 11, "cmd_unalias( '%s' )\n", args );

	strncpy( tmp_str, args, 255 );
	ptr = tmp_str;
	ptr = skip_whitespace( ptr );

	/* find alias string and null-terminate it */
	alias = ptr;
	ptr = find_whitespace( ptr );
	*ptr = '\0';

	found = alias_find( alias );

	if ( found != -1 ) {
		GtkTreeModel *freezer;
		/* delete the alias */
		freezer=freeze_treeview(GTK_WIDGET(chat_aliases));
		gtk_list_store_remove(GTK_LIST_STORE(freezer), &alias_find_iter);
		unfreeze_treeview(GTK_WIDGET(chat_aliases), freezer);
		/* now save list */
		save_aliases();
	}

	// DBG( 12, "RETURN cmd_unalias()\n" );
}

void cmd_last() {
	char buf[128];
	GList *this_user;

	snprintf( buf, 126, "--- %s ---\n", _("Last users to leave the room") );
	ct_append_fixed( buf, strlen( buf ));

	this_user = left_list;
	while( this_user ) {
		ct_append_fixed( this_user->data, strlen( this_user->data ));
		strncpy( buf, "\n" , 3);
		ct_append_fixed( buf, strlen( buf ));
		this_user = g_list_next( this_user );
	}
	strncpy( buf, "-------------------------------\n", 50 );
	ct_append_fixed( buf, strlen( buf ));

	snprintf( buf, 126, "--- %s ---\n", _("Full Tab Completion List") );
	ct_append_fixed( buf, strlen( buf ));

	if (! full_list) {build_tab_complete_list();}
	this_user = full_list;
	while( this_user ) {
		ct_append_fixed( this_user->data, strlen( this_user->data ));
		if ( tab_user == this_user ) {
			strncpy( buf, "   <-- current position", 126 );
			ct_append_fixed( buf, strlen( buf ));
		}
		strncpy( buf, "\n", 4 );
		ct_append_fixed( buf, strlen( buf ));
		this_user = g_list_next( this_user );
	}
	strncpy( buf, "-------------------------------\n", 126  );
	ct_append_fixed( buf, strlen( buf ));
}

void cmd_last_comments() {
	show_last_comments();
}

void cmd_send_file( char *args ) {
	char tmp[257];
	char msg[256];
	char *ptr;

	strncpy( tmp, args, 255 );
	ptr = strchr( tmp, ' ' );
	if ( ptr ) {
		*ptr = '\0';
		ptr++;
	} else {
		strncpy( tmp, "ERROR, usage: /file <file> <username>\n", 126  );
		append_to_textbox( chat_window, NULL, tmp );
		return;
	}

	snprintf( msg, 255, "File: %s", tmp ); 
	ymsg_send_file( ymsg_sess, ptr, msg, tmp );
}

void cmd_ignore( char *user ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( user );
	end = find_whitespace( ptr );

	while( *end ) {
		*end = '\0';
		ignore_toggle( ptr );

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	ignore_toggle( ptr );
}

void cmd_unignore( char *user ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( user );
	end = find_whitespace( ptr );

	while( *end ) {
		*end = '\0';

		if ( ignore_check( ptr ))
			ignore_toggle( ptr );

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	if ( ignore_check( ptr ))
		ignore_toggle( ptr );
}

void cmd_reload() {
	char buf[128];

	ignore_load();
	read_aliases();

	snprintf( buf, 127, "*** Loaded files: %d aliases and %d ignored users ***\n",
		alias_count, ignore_count());

	append_to_textbox( chat_window, NULL, buf );
}

void cmd_mute( char *user ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( user );
	end = find_whitespace( ptr );

	while( *end ) {
		*end = '\0';
		mute_toggle( ptr );

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	mute_toggle( ptr );
}

void cmd_unmute( char *user ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( user );
	end = find_whitespace( ptr );

	while( *end ) {
		*end = '\0';

		if ( mute_check( ptr ))
			mute_toggle( ptr );

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	if ( mute_check( ptr ))
		mute_toggle( ptr );
}

void cmd_regex( char *regex ) {
	char *ptr;
	char *end;

	ptr = skip_whitespace( regex );
	end = find_whitespace( ptr );

/* new way, only assume 1 regex per line, so allow whitespace & skip quotes */

	if ( *ptr == '"' ) {
		ptr++;
	}
	if ( ptr[strlen(ptr)-1] == '"' ) {
		ptr[strlen(ptr)-1] = '\0';
	}
	regex_toggle( ptr );


/* old way, where it was assumed to be a line of whitespace separated regex
	while( *end ) {
		*end = '\0';
		regex_toggle( ptr );

		ptr = end + 1;
		ptr = skip_whitespace( ptr );
		end = find_whitespace( ptr );
	}

	regex_toggle( ptr );
*/

}

void cmd_regex_test( char *args ) {
	char tmp[257];
	char msg[256];
	char *ptr;
	char *text;
	char *match;
	regex_t test_regex;

	strncpy( tmp, args, 255 );
	ptr = strchr( tmp, '"' );
	if ( ! ptr ) {
		strncpy( tmp, "USAGE: /test_regex \"text to test match on\" <regex>\n", 126  );
		append_to_textbox( chat_window, NULL, tmp );
		return;
	}
	text = ptr + 1;
	ptr = strchr( text, '"' );
	if ( *ptr ) {
		*ptr = '\0';
		ptr++;
	} else {
		strncpy( tmp, 
			"USAGE: /test_regex \"text to test match on\" <regex>\n" , 254);
		append_to_textbox( chat_window, NULL, tmp );
	}

	if ( ! *ptr ) {
		/* no regex specified so check against all current regex */
		if (( match = auto_ignore_check( text )) != NULL ) {
			snprintf( msg, 254, 
				"*** Gyach regex test: '%s' MATCHES using regex '%s'\n",
				text, match );
		} else {
			snprintf( msg,254, 
				"*** Gyach regex test: '%s' does NOT match any current regex\n",
				text );
		}
		append_to_textbox( chat_window, NULL, msg );
		return;
	}

	while( *ptr && *ptr == ' ' ) {
		ptr++;
	}

	regcomp( &test_regex, ptr, REG_EXTENDED | REG_ICASE | REG_NOSUB );
	if ( ! regexec( &test_regex, text, 0, NULL, 0 )) {
		/* matched */
		snprintf( msg,254,  "*** Gyach regex test: '%s' matches using regex '%s'\n",
			text, ptr );
	} else {
		/* didn't match */
		snprintf( msg,254, 
			"*** Gyach regex test: '%s' does NOT match using regex '%s'\n",
			text, ptr );
	}
	append_to_textbox( chat_window, NULL, msg );
	regfree( &test_regex );
}

void cmd_find( char *text ) {
	GtkWidget *tmp_widget;

	search_pos = -1;
	search_case_sensitive = 0;
	if ( search_text ) {
		free( search_text );
		search_text = NULL;
	}

	if ( strlen( text )) {
		search_text = strdup( text );

		search_chat_text( search_case_sensitive );
	} else {
		if ( ! find_window ) {
			find_window = create_find_window();
		} else {
			if ( search_text ) {
				tmp_widget = lookup_widget( find_window, "search_text" );
				gtk_entry_set_text(GTK_ENTRY(tmp_widget), search_text );

				tmp_widget = lookup_widget( find_window, "case_sensitive" );
				gtk_toggle_button_set_active(
					GTK_TOGGLE_BUTTON(tmp_widget) ,
					search_case_sensitive );
			}
		}
		gtk_widget_show( find_window );
	}
}

void cmd_quit() {
	ymsg_leave_chat( ymsg_sess );
	ymsg_messenger_logout(ymsg_sess);
	ymsg_sess->pkt.type = YMSG_GYE_EXIT; /*  force logout */
	show_yahoo_packet();
	ymsg_sess->sock = -1; 
	ymsg_sess->quit = 1; 

	history_save();

	gdk_threads_leave();

#ifndef OS_WINDOWS
	exit(0);
	/* gtk_main_quit(); */
#endif
}

void cmd_exec( GtkWidget *window, GtkWidget *textbox, char *cmd, int local,
		char *user, int emote ) {
	char buf[601] = "";
#ifndef OS_WINDOWS
	int p[2];
	int pid;
	int chars;
	char new_cmd[513];
	char *ptr;
	int chars_read = 0;
	int elapsed = 0;
	int timed_out = 0;
	int flags;


	pipe( p );
	strncpy( buf, cmd, 595 );
	strcat( buf, ":\n" );
	chars = strlen( buf );

	snprintf( new_cmd, 511, "%s 2>&1", cmd );

	if (( pid = fork()) == -1 ) {
		strcat( buf, "couldn't fork!\n" );
	} else if ( pid ) {
		close( p[1] );
		flags = fcntl( p[0], F_GETFL, 0 );
		fcntl( p[0], F_SETFL, flags | O_NONBLOCK );
		ptr = buf + chars;
		while( chars < 512 ) {
			if ( elapsed > ( 3 * 1000 * 1000 )) {
				timed_out = 1;
				break;
			}

			if (( chars_read = read( p[0], ptr, (512 - chars ))) < 0 ) {
				usleep( 50000 );
				elapsed += 50000;
				continue;
			} else if ( chars_read == 0 ) {
				break;
			}
			chars += chars_read;
			ptr += chars_read;
			*ptr = 0;
		}

		if ( kill( pid, 0 ) == 0 ) {
			kill( pid, SIGKILL );
		}

		if ( chars > 511 ) {
			buf[509] = '.';
			buf[510] = '.';
			buf[511] = '.';
		}

		waitpid( pid, NULL, 0 );
	} else {
		close( p[0] );
		if ( p[1] != STDOUT_FILENO ) {
			dup2( p[1], STDOUT_FILENO );
			close( p[1] );
		}
		if ( STDERR_FILENO != STDOUT_FILENO ) {
			dup2(STDOUT_FILENO, STDERR_FILENO);
		}

		execlp( "sh", "sh", "-c", new_cmd, NULL );
	}

	if ( ! textbox ) {
		/* ! or !! in chat window */
		if ( local ) {
			strcat( buf, "\n" );
			append_to_textbox( chat_window, NULL, buf );
		} else {
			if ( emote ) {
				gyach_emote( ymsg_sess, buf );
			} else {
				gyach_comment( ymsg_sess, buf );
			}
		}
	} else {
		/* ! or !! in PM window */
		if ( ! local ) {
			ymsg_pm( ymsg_sess, user, buf );
		}
		strcat( buf, "\n" );
		append_to_textbox( window, textbox, buf );
	}

#else
	snprintf( buf, 254, "Sorry, /exec (!) and /execlocal (!!) are not "
		"implemented on this platform as of this Gyach version." );
	append_to_textbox( chat_window, NULL, buf );
#endif
}

void invisible_probe ( char *args ) {
		if ( strcmp(args, ymsg_sess->user)  && (! find_profile_name(args) ) ) {
				ymsg_p2p_ask( ymsg_sess, args, 1);
	}
}

int try_command( char *input ) {
	char *action = NULL;
	char *args = NULL;
	char tmp_input[ 640 ];

	strncpy( tmp_input, input , 638);

	action = tmp_input + 1; /* skip the '/' */

	action = skip_whitespace( action ); /* find head of command */
	args = action;
	args = find_whitespace( args ); /* find whitespace after command */
	if ( *args ) {
		*args = '\0';
		args++;
		args = skip_whitespace( args ); /* find text after whitespace */
	}

	if ( action[0] == '\0' )
		return( 0 );

	if ( regex_match( "^autoaway$", action )) {
		cmd_auto_away( args );
	} else if ( regex_match( "^al(ias)?$", action )) {
		cmd_alias( args );
	} else if ( regex_match( "^aw(ay)?$", action )) {
		cmd_away( args );
	} else if ( regex_match( "^back$", action )) {
		cmd_away( "0" );
	} else if ( regex_match( "^cl(r|s|ear)?$", action )) {
		cmd_cls( args );
	} else if ( regex_match( "^connect$", action )) {
		cmd_connect();
	} else if ( regex_match( "^disconnect)$", action )) {
		cmd_disconnect();
	} else if ( regex_match( "^date$", action )) {
		cmd_date_time();
	} else if ( regex_match( "^time$", action )) {
		cmd_date_time();
	} else if (( regex_match( "^e(mote)?$", action )) ||
			   ( regex_match( "^me$", action ))) {
		cmd_emote( args );
	} else if ( regex_match( "^exec$", action )) {
		cmd_exec( chat_window, NULL, args, 0, NULL, 0 );
	} else if ( regex_match( "^execlocal$", action )) {
		cmd_exec( chat_window, NULL, args, 1, NULL, 0 );
	} else if ( regex_match( "^exec_e$", action )) {
		cmd_exec( chat_window, NULL, args, 0, NULL, 1 );
	} else if ( regex_match( "^execlocal_e$", action )) {
		cmd_exec( chat_window, NULL, args, 1, NULL, 1 );
	} else if ( regex_match( "^file$", action )) {
		cmd_send_file( args );
	} else if ( regex_match( "^f(ind)?$", action )) {
		cmd_find( args );
	} else if ( regex_match( "^follow$", action )) {
		cmd_follow( args );
	} else if (( regex_match( "^friends$", action )) ||
			   ( regex_match( "^buddies$", action ))) {
		show_friends();
	} else if (( regex_match( "^fr(iend)?$", action )) ||
			   ( regex_match( "^b(uddy)?$", action ))) {
		cmd_friend( args );
	} else if ( regex_match( "^image$", action )) {
		cmd_image( args );
	} else if ( regex_match( "^g(oto)?$", action )) {
		cmd_goto( args );
	} else if ( regex_match( "^help$", action )) {
		cmd_help( 0 );
	} else if ( regex_match( "^help_full$", action )) {
		cmd_help( 1 );
	} else if ( regex_match( "^history$", action )) {
		history_print();
	} else if ( regex_match( "^ig(nore)?$", action )) {
		cmd_ignore( args );
	} else if (( regex_match( "^call$", action )) ||
			   ( regex_match( "^inv(ite)?$", action ))) {
		cmd_invite( args );
	} else if ( regex_match( "^j(oin)?$", action )) {
		cmd_join( args );
	} else if ( regex_match( "^l(ast)?$", action )) {
		cmd_last();
	} else if ( regex_match( "^l(c|ast_comments)?$", action )) {
		cmd_last_comments();
	} else if ( regex_match( "^mail$", action )) {
		cmd_mail( args );
	} else if ( regex_match( "^hidden$", action )) {
		invisible_probe( args );
	} else if ( regex_match( "^mute$", action )) {
		cmd_mute( args );
	} else if ( regex_match( "^online$", action )) {
		cmd_online();
	} else if ( regex_match( "^pr(ofile)?$", action )) {
		cmd_profile( args );
	} else if (( regex_match( "^q(uit)?$", action )) ||
			   ( regex_match( "^exit$", action )) ||
			   ( regex_match( "^logo(ff|ut)$", action ))) {
		cmd_quit();
	} else if ( regex_match( "^reconnect$", action )) {
		cmd_reconnect();
	} else if ( regex_match( "^regex$", action )) {
		cmd_regex( args );
	} else if ( regex_match( "^regex_test$", action )) {
		cmd_regex_test( args );
	} else if ( regex_match( "^reload$", action )) {
		cmd_reload();
	} else if (( regex_match( "^tell$", action )) ||
			   ( regex_match( "^pm$", action ))) {
		cmd_tell( args );
	} else if ( regex_match( "^t(hink)?$", action )) {
		cmd_think( args );


	} else if ( regex_match( "^conf_start$", action )) {
		cmd_conf_start( args );
	} else if ( regex_match( "^conf-start$", action )) {
		cmd_conf_start( args );
	} else if ( regex_match( "^conf_invite$", action )) {
		cmd_conf_invite( args );
	} else if ( regex_match( "^conf-invite$", action )) {
		cmd_conf_invite( args );

	} else if ( regex_match( "^revoke$", action )) {
		cmd_revoke_friend( args );

	} else if ( regex_match( "^viewcam$", action )) {
		cmd_viewcam( args );

	} else if ( regex_match( "^temp-friend-add$", action )) {
		cmd_add_tmp_friend( args );

	} else if ( regex_match( "^temp-friend-remove$", action )) {
		cmd_rem_tmp_friend( args );


	} else if ( regex_match( "^flooder-remove$", action )) {
		cmd_flooder_remove( args );

	} else if ( regex_match( "^boot$", action )) {
		cmd_booter( args );

	} else if ( regex_match( "^business-card$", action )) {
		cmd_contact_info( args );

	} else if ( regex_match( "^leave$", action )) {
		cmd_leave_room( args );

	} else if ( regex_match( "^rejoin$", action )) {
		cmd_rejoin( args );

	} else if ( regex_match( "^unalias$", action )) {
		cmd_unalias( args );
	} else if ( regex_match( "^unignore$", action )) {
		cmd_unignore( args );
	} else if ( regex_match( "^unmute$", action )) {
		cmd_unmute( args );
	} else if ( regex_match( "^url$", action )) {
		cmd_url( args );
	} else if ( regex_match( "^v(er)?$", action )) {
		cmd_url( "" );
	} else if ( regex_match( "^adv$", action )) {
		cmd_url( "" );
	} else {
		return( 0 );
	}

	return( 1 );
}

#define APP_CMD( list, cmd )	g_list_append( list, strdup( cmd ))
void append_commands_to_list( GList *list ) {
	/* fixme, make this into an array of structs and redo commands.c to use */
	APP_CMD( list, "/autoaway" );
	APP_CMD( list, "/alias" );
	APP_CMD( list, "/away" );
	APP_CMD( list, "/back" );
	APP_CMD( list, "/clear" );
	APP_CMD( list, "/clr" );
	APP_CMD( list, "/cls" );
	APP_CMD( list, "/connect" );
	APP_CMD( list, "/conf-invite" );
	APP_CMD( list, "/conf-start" );
	APP_CMD( list, "/disconnect" );
	APP_CMD( list, "/emote" );
	APP_CMD( list, "/find" );
	APP_CMD( list, "/flooder-remove" );
	APP_CMD( list, "/follow" );
	APP_CMD( list, "/friend" );
	APP_CMD( list, "/friends" );
	APP_CMD( list, "/goto" );
	APP_CMD( list, "/help" );
	APP_CMD( list, "/history" );
	APP_CMD( list, "/ignore" );
	APP_CMD( list, "/invite" );
	APP_CMD( list, "/join" );
	APP_CMD( list, "/last" );
	APP_CMD( list, "/last_comments" );
	APP_CMD( list, "/leave" );
	APP_CMD( list, "/mail" );
	APP_CMD( list, "/mute" );
	APP_CMD( list, "/me" );
	APP_CMD( list, "/online" );
	APP_CMD( list, "/profile" );
	APP_CMD( list, "/quit" );
	APP_CMD( list, "/revoke" );
	APP_CMD( list, "/rejoin" );
	APP_CMD( list, "/reload" );
	APP_CMD( list, "/regex" );
	APP_CMD( list, "/regex_test" );
	APP_CMD( list, "/tell" );
	APP_CMD( list, "/temp-friend-add" );
	APP_CMD( list, "/temp-friend-remove" );
	APP_CMD( list, "/pm" );
	APP_CMD( list, "/think" );
	APP_CMD( list, "/unalias" );
	APP_CMD( list, "/unignore" );
	APP_CMD( list, "/unmute" );
	APP_CMD( list, "/url" );
	APP_CMD( list, "/viewcam" );
	APP_CMD( list, "/ver" );
	APP_CMD( list, "/date" );
	APP_CMD( list, "/time" );
	APP_CMD( list, "/business-card" );

	APP_CMD( list, "/call" );
	APP_CMD( list, "/image" );
	APP_CMD( list, "/help_full" );
	APP_CMD( list, "/exit" );
	APP_CMD( list, "/quit" );
	APP_CMD( list, "/logoff" );
	APP_CMD( list, "/logout" );
	APP_CMD( list, "/time" );
	APP_CMD( list, "/buddy" );
	APP_CMD( list, "/buddies" );
	APP_CMD( list, "/file" );
	APP_CMD( list, "/exec" );
	APP_CMD( list, "/execlocal" );
	APP_CMD( list, "/exec_e" );
	APP_CMD( list, "/execlocal_e" );
}
