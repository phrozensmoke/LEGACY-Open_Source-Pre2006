/*****************************************************************************
 * packet_handler.c
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
 * Phrozensmoke ['at'] yahoo.com
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include "gyach.h"

#include "aliases.h"
#include "callbacks.h"
#include "commands.h"
#include "friends.h"
#include "gyach_int.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "profname.h"
#include "sounds.h"  /* added, PhrozenSmoke */
#include "bootprevent.h"  /* added, PhrozenSmoke */
#include "animations.h"  /* added, PhrozenSmoke */
#include "conference.h"   /* added, PhrozenSmoke */
#include "packet_handler.h" 
#include "trayicon.h"
#include "webconnect.h"
#include "webcam.h"
#include "plugins.h"
#include "gytreeview.h"
#include "ycht.h"
#include "time.h" 


char *last_chat_room_used=NULL;
char *last_news_alert=NULL;

extern gchar **choose_pixmap(char *user);
extern void set_tuxvironment (GtkWidget *pmwidget, GtkWidget *pmwindow, char *tuxviron);
extern void update_my_buddy_image(PM_SESSION *pm_sess, int is_me);


void display_offline_message(char *sender, char *sent_time, char *msg);
void handle_imvironments();
void handle_buddy_image_uploaded();
void toggle_sms_pm_session(char *who, int sms_on);
void append_char_pixmap_text(const char **pixy, GtkWidget *textw);
int upload_new_buddy_image();
void append_to_open_pms(char *who, char *mytext, int with_timestamp);
char *get_pic_checksum(char *picfile);
void start_chat_online();


char *search_stuff=NULL;
int immed_rejoin=0;
int first_userstat=1;
int got_buddy_ignore_list=0;
int got_y_ignore_list=0;
int got_y_buddy_list=0;
int ignore_ymsg_online=0;
int parsing_initial_statuses=0;
int ignore_mailstat=0;
char *my_avatar=NULL;
char *friend_list_spool=NULL;
struct yahoo_friend *FRIEND_OBJECT = NULL;



	static gchar buf[2048];  /* these are static so they aren't on the stack */ /* 2250 */
	static char tmp[2048];
	static char tmp2[2048];
	static char tmp3[2048];	
	char *packptr;
	char *packsrc;
	char *end;	
	int last;
	time_t time_now;
	struct tm *tm_now;
	GList *pm_lpackptr;
	PM_SESSION *pm_sess;
	int display;
	int accept_pm;
	int count;
	/* char *blankmsg=NULL;  */

	int login_noroom=0;


char ymsg_fields[300][2048];

char *offline_msg[15];
char *offline_sender[15];
char *offline_time[15];

int from_valid_sender(char *sender) {
	/* Messages from ourselves or from invalid-looking 
		names will be ignored...'spoofing' could be used 
		in these cases */
	if (! strcasecmp(sender, ymsg_sess->user) ) {return 0; }
	if (find_profile_name(sender))  {return 0; }
	if (strstr(sender," ")) {return 0;} /* a space in a name is definitely invalid */
	return 1;
}

void control_msg_line_feeds() {	
	/* check for lots of linefeeds, replace with spaces if necessary */
	/* put data in tmp2 */
	if (! limit_lfs ) { return;}
	if ( limit_lfs ) {
		char *lf;
		int lf_count = 0;
		/* Remove tabs and return chars */
		lf = strchr( tmp2, '\r' );
		while( lf ) {*lf = ' ';  lf = strchr( lf + 1, '\r' );}
		lf = strchr( tmp2, '\t' );
		while( lf ) {*lf = ' ';  lf = strchr( lf + 1, '\t' );}

		lf = strchr( tmp2, '\n' );
		while( lf ) {
			lf_count++;
			lf = strchr( lf + 1, '\n' );
		}
	if ( lf_count > limit_lfs ) {
		lf = strchr( tmp2, '\n' );
		while( lf ) {
			*lf = ' ';
			lf = strchr( lf + 1, '\n' );
		}
	}
				}
}





void append_friend_list_spool(char *fcollect) {
	int allowable=0;
	if (! friend_list_spool) {
		friend_list_spool=malloc(3072);
		sprintf( friend_list_spool,"%s","");
	}
	if (! friend_list_spool) {return;}
	allowable=3070-strlen(friend_list_spool);
	if (allowable<25) {return;}
	strncat( friend_list_spool, fcollect, allowable);
}
void reset_friend_list_spool() {
	if (! friend_list_spool) {return;}
	free(friend_list_spool);
	friend_list_spool=NULL;
}
void push_friend_list_spool() {
	if (! friend_list_spool) {return;}
	if (friend_list_spool[0]) {
		populate_friend_list(friend_list_spool);
	}
	reset_friend_list_spool();
}


void jump_to_chat_tab() {
	GtkWidget *findwid=NULL;
	findwid=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
	if (findwid) {gtk_notebook_set_current_page(GTK_NOTEBOOK(findwid), 0);}
}

void send_automated_response(char *inuser, char *inalias, char *myinmsg) {
	if (! inuser) {return;}
	if (! myinmsg) {return;}
	if (strlen(inuser)<1) {return;}
	if (strlen(myinmsg)<1) {return;}
	
	collect_profile_replyto_name();
	snprintf( buf, 1100, "<b>GYACH ENHANCED Automated Response: Sorry (%s / %s), %s </b> Your message was blocked and will not be seen by this user.  This is NOT Y! Tunnel for Windows. This is <i>GYach Enhanced for LINUX</i>, a greatly improved version of the orignal Gyach, with enhancements by PhrozenSmoke:  %s  ( What's linux?  http://www.linux.org ).  Please do not bother responding to this message: This is an automated message.",	inuser, inalias?inalias:inuser, myinmsg, GYACH_URL );
	ymsg_pm( ymsg_sess, inuser, buf );	
	reset_profile_replyto_name();

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\nSENT AUTOMATED RESPONSE to '%s' at  [%s]: \n\n%s\n", inuser, ctime(&time_llnow), buf);
			fflush( capture_fp );
			}
}


void comm_block_notify(char *who, char *who_alias, char *msg1, char *msg2, int  preempt_strike, int sound_event) {
	int spam_flag=0;
	append_timestamp(chat_window, NULL);
	append_char_pixmap_text((const char**)pixmap_pm_ignore , NULL);
	snprintf( buf, 1200,   	"  %s** %s[Gyach-E]%s %s : %s'%s'%s ( %s ) **%s\n", 
		"\033[#af005dm", 
		"\033[#8486afm", 
		"\033[#af005dm", 
		msg1, 
		"\033[#9967b2m", 
		who, 
		"\033[#af005dm", 
		who_alias?who_alias:who,
		YAHOO_COLOR_BLACK );
	append_to_textbox( chat_window, NULL, buf );
	append_to_open_pms(who, buf,1);
	if (strstr(msg1,"[Spam]")) {spam_flag=1;}
	if (preempt_strike && enable_preemptive_strike && (! spam_flag)) {
		if ( (!find_friend(who))  && (!find_temporary_friend(who)) ) { preemptive_strike(who); }
	}
	if (! spam_flag) {
		/* dont respond back to messages flagged as spam, it 
		just results in us being sent more bot messages  */ 
		send_automated_response(who, who_alias?who_alias:who, msg2);
	}
	if (sound_event) {play_sound_event(SOUND_EVENT_REJECT);}
}


int test_automute(char *who)  {
	if (!strcasecmp(ymsg_sess->user,who)) {return 0;}
	if (find_profile_name(who)) {return 0;}
	if (find_temporary_friend(who) && (never_ignore_a_friend) ) {return 0;}
	if (find_friend(who) && (never_ignore_a_friend) ) {return 0;}
	if (immunity_check(who)) {return 0;}
	if (ignore_check(who)) {return 0;}
	if (! mute_check(who)) {return 1;}
return 0;
}


/* added, PhrozenSmoke, centralized method for PM/privacy permission check */

int get_pm_perms(char *who) {
	/* 2= absolutely safe: ourselves, or a friend, temp friend */
	/* 1= should be safe...strangers we are allowing */
	/* 0 = not safe, not approved at all */

	/* always allow system messages */
	if(ymsg_sess->pkt.type== YMSG_SYSMSG) {
		/* Had to change this, because now lots of
		spam bots are using 'system messages' to
		send spam - Yahoo should fix this */
	if (is_valid_system_message(who)) {return 1;}
	}

	/* always allow ourselves */
	if (! strcasecmp(who, ymsg_sess->user) ) {return 2; }
	if (find_profile_name(who))  {return 2; }

	/* Yahoo 'system' messages */
	if (! strcasecmp(who, "yahoomessengerteam") ) {return 1; }
	if (! strcasecmp(who, "system") ) {return 1; }

	/* keep this here to be thorough, if we are blocking everybody, 
		make sure we truly block everybody */
	if ( (! pm_from_friends) && (! pm_from_users) && (! pm_from_all)) {return 0;}

	/* always block flooders/booters */
	if (is_flooder( who)) {
		if ( (! never_ignore_a_friend) ||  (!find_temporary_friend(who)) ) {
				return 0;
			}
	}

	/* always block ignored people who arent friends */
     	if (ignore_check(who)) {
		if ( (! never_ignore_a_friend)  || 
			( (!find_temporary_friend(who)) && (!find_friend(who)) ) 	) {
				return 0;
			}
	}

	/* check for 'guest' users */
	if (ignore_guests)  {
		if (strstr(who, "sbcglobal.net")) {
		if ( (!find_friend(who)) && (!find_temporary_friend(who)) ) { return 0;}
													}
							}

		if ( pm_from_all ) {return 1;}  /*  if allow everybody */

		/* if allow people in the room */
		if ( ( pm_from_users ) &&  ( find_user_row( who ) >= 0 ))  {return 1;}

		/* if allow friends  and is a friend or temp friend, mark very safe  */
		if ( ( pm_from_friends ) && ( find_friend(who) || find_temporary_friend(who) )  ) {  
			return 2;
		 }

return 0;
}



/* added Phrozensmoke, centralized method for creating PM windows */
void focus_pm_entry(GtkWidget *pmwin) {
	GtkWidget *pmstentry=gtk_object_get_data (GTK_OBJECT (pmwin), "pms_entry");
	if (pmstentry) {
		int sposs=0;
		if (gtk_object_get_data (GTK_OBJECT (pmstentry), "is_gtk_entry") != NULL) {
			sposs=gtk_editable_get_position(GTK_EDITABLE(pmstentry));
		}
		gtk_widget_grab_focus(GTK_WIDGET(pmstentry));
		if (sposs > 0 ) {gtk_editable_set_position(GTK_EDITABLE(pmstentry), sposs);}
	}
}

void show_incoming_pm(PM_SESSION *gpm_sess, int mesg) {
	char titlebf[128];
	GtkStyle *windowstyle;
	GdkPixmap *pm_icon=NULL;
	GdkBitmap *mask_icon;

	windowstyle = gtk_widget_get_style( gpm_sess->pm_window );
	if (mesg>0) {
	pm_icon = gdk_pixmap_create_from_xpm_d( gpm_sess->pm_window->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		gpm_sess->sms?(gchar **)pixmap_sms:(gchar **)pixmap_status_cam_male );
	snprintf( titlebf, 124, "* %s - GyachE", gpm_sess->pm_user );  /* buffer overflow limit set */
	gtk_window_set_title( GTK_WINDOW(gpm_sess->pm_window), titlebf );
	} else {
	pm_icon = gdk_pixmap_create_from_xpm_d( gpm_sess->pm_window->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_status_cam );
	if (strstr(GTK_WINDOW(gpm_sess->pm_window)->title, _("[TYPING]") )) {
			snprintf( titlebf, 124, "%s %s - GyachE", gpm_sess->pm_user, _("[TYPING]") );	
	}  else {
		snprintf( titlebf, 124, "%s - GyachE", gpm_sess->pm_user );
		}
	gtk_window_set_title( GTK_WINDOW(gpm_sess->pm_window), titlebf );
	}
	gdk_window_set_icon( gpm_sess->pm_window->window, NULL, pm_icon, mask_icon );
}


void init_pm_buddy_image(PM_SESSION *ppm_sess) {
	if (! show_bimages) {return;}
	if (! emulate_ymsg6) {return;}
	if (! ppm_sess->buddy_image_share) {return; }
	if ( ppm_sess->sms) {return; }

	if (! find_friend(ppm_sess->pm_user)) {return;}

	/* are they officially only or hiding on invisible? */ 
	if (! find_online_friend(ppm_sess->pm_user)) {
		if (! friend_is_invisible(ppm_sess->pm_user)) {
			return;  /* they may not be online, so wait for them to request image */ 
		}
	}

		reset_profile_replyto_name();
		set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(ppm_sess->pm_window)))  );
		if (bimage_url && bimage_hash && bimage_file)  {
			if (upload_new_buddy_image()) { /* still valid image? */ 
				ymsg_bimage_update(ymsg_sess, ppm_sess->pm_user, bimage_hash);
				ymsg_bimage_toggle(ymsg_sess, ppm_sess->pm_user, bimage_share);
				/* ymsg_bimage_notify(ymsg_sess, ppm_sess->pm_user, bimage_url, bimage_hash); */ 
				snprintf(buf, 300, "   %s** '%s' **%s\n",YAHOO_COLOR_ORANGE, 
				_("Your buddy image has been offered to the user."), YAHOO_COLOR_BLACK);
				append_to_open_pms(ppm_sess->pm_user, buf, 1);
						}
			}  else  {upload_new_buddy_image();}


			/* check for buddy images we already have on hard-drive */ 
			FRIEND_OBJECT=yahoo_friend_find(ppm_sess->pm_user);
			if (FRIEND_OBJECT) {
				if (FRIEND_OBJECT->buddy_image_hash) {	
					char bbhash[32];
					char filename[256];

	if (! strcmp(FRIEND_OBJECT->buddy_image_hash, "[BAVATAR]")) {
		if (FRIEND_OBJECT->avatar && (! ppm_sess->buddy_image_file) ) {
			if ( download_yavatar(FRIEND_OBJECT->avatar, 2, NULL)) {
					snprintf( filename, 254, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, FRIEND_OBJECT->avatar, avatar_filetype);
					ppm_sess->buddy_image_file=strdup(filename);
			}
		}
		update_my_buddy_image(ppm_sess, 0);
		return;
	}

					snprintf(bbhash,30, "%s", FRIEND_OBJECT->buddy_image_hash);
					clean_bimage_name(bbhash);
					snprintf( filename, 254, "%s/bimages/bimage-%s.png", 
						GYACH_CFG_DIR, bbhash );	
					if (bimage_exists(bbhash))  {
						if (ppm_sess->buddy_image_file) {free(ppm_sess->buddy_image_file);}
						ppm_sess->buddy_image_file=strdup(filename);
						update_my_buddy_image(ppm_sess, 0);
						reset_current_pm_profile_name();
						return ;  /* we got it, so bail out */ 
					}
				}
			}

		/* try to request their buddy image  */
		ymsg_bimage_accept(ymsg_sess, ppm_sess->pm_user , 1);  
		reset_current_pm_profile_name();
}


PM_SESSION * new_pm_session(char *forwho) {
PM_SESSION *npm_sess;
GtkWidget *tuxviron=NULL;
char titlebuf[128]="";

	GtkTextBuffer *text_buffer;
	GtkTextIter	end_iter;

						npm_sess = (PM_SESSION *)malloc( sizeof( PM_SESSION ));						
						npm_sess->pm_window = create_pm_session();


						/* TUXVironment stuff */
					tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"tuxviron");
					if (tuxviron) {
						gtk_object_set_data (GTK_OBJECT (tuxviron), "session",npm_sess);
									}

tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"encbutton");
					if (tuxviron) {
						gtk_object_set_data (GTK_OBJECT (tuxviron), "session",npm_sess);
									}

						gtk_object_set_data (GTK_OBJECT (npm_sess->pm_window), "imv",strdup(";0"));

						npm_sess->pm_text = lookup_widget( npm_sess->pm_window,
							"pms_text" );
						npm_sess->pm_user = strdup( forwho );
						npm_sess->typing = 0;

tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"save_user");
					if (tuxviron) {
						gtk_object_set_data (GTK_OBJECT (tuxviron), "who",npm_sess->pm_user);
									}

						npm_sess->encryption_type = 0;
						npm_sess->encrypted_myway = 0;
						npm_sess->encrypted_theirway = 0;
						npm_sess->my_gpg_passphrase=-1;
						npm_sess->their_gpg_passphrase=-1;

						npm_sess->buddy_image_share=bimage_share;
						if ( (bimage_friend_only) && (! find_friend(npm_sess->pm_user)) && 
							(! find_temporary_friend(npm_sess->pm_user)) ) {
							npm_sess->buddy_image_share=0;
						}
						npm_sess->buddy_image_size=bimage_size;
						npm_sess->buddy_image_visible=0;
						npm_sess->window_style=pm_window_style;
						npm_sess->buddy_image_file=NULL;
						npm_sess->bimage_panel=NULL;
						npm_sess->bimage_me=NULL;
						npm_sess->bimage_them=NULL;

						if (show_bimages)  {
			tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"panel");
			if (tuxviron) {npm_sess->bimage_panel=tuxviron; npm_sess->buddy_image_visible=1;}
			tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"bimage_me");
			if (tuxviron) {npm_sess->bimage_me=tuxviron; }
tuxviron=gtk_object_get_data(GTK_OBJECT(npm_sess->pm_window),"bimage_them");
			if (tuxviron) {npm_sess->bimage_them=tuxviron; }

						if (npm_sess->buddy_image_share && bimage_file )  {
							update_my_buddy_image(npm_sess, 1);
							}
			}

						gtk_text_view_set_wrap_mode(
							GTK_TEXT_VIEW(npm_sess->pm_text), GTK_WRAP_WORD );
						text_buffer = gtk_text_view_get_buffer(
							GTK_TEXT_VIEW(npm_sess->pm_text));
						gtk_text_buffer_get_end_iter( text_buffer, &end_iter );
						gtk_text_buffer_create_mark( text_buffer, "end_mark",
							&end_iter, 0 );

						pm_list = g_list_append( pm_list, npm_sess );

		append_char_pixmap_text((const char**)pixmap_pm_join, npm_sess->pm_text);	
		snprintf(titlebuf, 126, "  %s%s[%s]%s%s\n", 
			YAHOO_STYLE_BOLDON, "\033[#5791B1m", forwho, 
				YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
		append_to_textbox_color( npm_sess->pm_window,
		npm_sess->pm_text, titlebuf );

						npm_sess->sms=0;
						FRIEND_OBJECT=yahoo_friend_find(forwho);
						if (FRIEND_OBJECT) {
							if (FRIEND_OBJECT->insms) {	toggle_sms_pm_session(forwho, 1); }
						}

						/* setup the callback for url highlighting */
						gtk_signal_connect_after (GTK_OBJECT (npm_sess->pm_text),
							"motion_notify_event",
							GTK_SIGNAL_FUNC(chat_motion_notify), NULL);

						GTK_WIDGET_UNSET_FLAGS( GTK_WIDGET(npm_sess->pm_text),
							GTK_CAN_FOCUS);

						gtk_widget_set_events( GTK_WIDGET(npm_sess->pm_text),
							GDK_POINTER_MOTION_MASK
							| GDK_POINTER_MOTION_HINT_MASK );

init_pm_buddy_image(npm_sess);
focus_pm_entry(npm_sess->pm_window);
gtk_widget_show_all(npm_sess->pm_window);
show_incoming_pm(npm_sess, 0);  /* do this last!    */ 
return npm_sess;

}


/* added PhrozenSmoke, proper handling of (multiple) offline messages in a single packet */
void clear_offline_messages() {
	char *nostr=NULL;
	offline_msg[0]=nostr;
	offline_sender[0]=nostr;
	offline_time[0]=nostr;
}

int store_offline_message(char *sender, char *sent_time, char *sent_msg) {
	int msg_count=0;
	char *nostr=NULL;
	if (is_flooder(sender)) {return 0;}
	while (offline_msg[msg_count] != NULL) {
		if (msg_count==13) {return 1;}
		msg_count++;
								   }
	offline_msg[msg_count]=strdup(sent_msg);
	offline_sender[msg_count]=strdup(sender);
	offline_time[msg_count]=strdup(sent_time);
	offline_msg[msg_count+1]=nostr;
	offline_sender[msg_count+1]=nostr;
	offline_time[msg_count+1]=nostr;
	return 1;
}


void parse_offline_messages(char *packet) {
	char	*token=NULL;
	char *tmp_str=NULL;
	int msg_counter=0;
	char *tmp_sender=NULL;
	char *tmp_time=NULL;
	char *tmp_msg=NULL;
	int got_sn=0;
	int got_msg=0;
	int got_time=0;
	int parts=0;
	time_t mytime=0;
	tmp_str=strdup(packet);
	token=strtok(tmp_str,YMSG_SEP);


	while (token != NULL)  {

		if ( strcmp( token, "4" )==0)  {  
			/*got username packet header*/  
			got_sn=1;   /* next token should contain username */
			parts=0;
			token=strtok(NULL,YMSG_SEP);
			continue;
							  }


		if (got_sn) {
			if (tmp_sender) {free(tmp_sender);}
			tmp_sender=strdup(token);
			got_sn=0;
			parts++;
			continue;
				}

		if ( strcmp( token, "14" )==0)  {  
			/*got username packet header*/  
			got_msg=1;   /* next token should contain message */
			token=strtok(NULL,YMSG_SEP);
			continue;
							  }

		if (got_msg) {
			if (tmp_msg) {free(tmp_msg);}
			snprintf(tmp2, 1200, "%s", token);
			control_msg_line_feeds();
			tmp_msg=strdup(tmp2);
			got_msg=0;
			parts++;
			continue;
				}

		if ( strcmp( token, "15" )==0)  {  
			/*got time packet header*/  
			got_time=1;   /* next token should contain time */
			token=strtok(NULL,YMSG_SEP);
			continue;
							  }

		if (got_time) {
			char timebuf[40]="";
			mytime=time(NULL);
			mytime=mytime-time(NULL)+atol(token);
			if (tmp_time) {free(tmp_time);}
			tmp_time=strdup(ctime_r( &mytime, timebuf));
			got_time=0;
			mytime=0;
			parts++;
			token=strtok(NULL,YMSG_SEP);
			continue;
				}

		if (parts==3)  {	
			if ( store_offline_message(tmp_sender, tmp_time, tmp_msg)  ) {
				msg_counter++;
																												 }
			parts=0;
			continue;
				     }

		if (msg_counter>13) {break;}
		token=strtok(NULL,YMSG_SEP);

					 } /* end while not NULL */	

			if (tmp_msg) {free(tmp_msg);}
			if (tmp_sender) {free(tmp_sender);}	
			if (tmp_time) {free(tmp_time);}

}


void show_offline_messages() {
	int msg_count=0;
	char *nostr=NULL;

	while (offline_msg[msg_count] != NULL) {
		if (msg_count==14) {break;}
		display_offline_message(offline_sender[msg_count], offline_time[msg_count],
			offline_msg[msg_count]);
		free(offline_sender[msg_count]); offline_sender[msg_count]=nostr;
		free(offline_time[msg_count]); offline_time[msg_count]=nostr;
		free(offline_msg[msg_count]); offline_msg[msg_count]=nostr;

		msg_count++;
								   }
	clear_offline_messages();
}

void align_pms_repyto(PM_SESSION *gpm_sess) {
		/* Set the PM 'My Yahoo ID!' text field to the profile
		name the person sent the PM to */
	gtk_entry_set_text( 
		retrieve_profname_entry(gpm_sess->pm_window),
		get_profile_replyto_name() );
}

void display_offline_message(char *sender, char *sent_time, char *sent_msg) {

			if ( strlen(sender) && strlen(sent_time) && 
		   	  strlen(sent_msg))  {
				int limit=350; /* safe limit for strangers */
				int spammer=0;

				if (is_flooder(sender))  		{ return; }

					if (!get_pm_perms(sender)) {
						char tbuf[75]="";	
						log_possible_flood_attack(sender, 1, "Sent Auto-Rejected Offline Message");	
						strip_html_tags(sent_msg);
						snprintf(tbuf, 70, "%s...", sent_msg);

		if ( enable_chat_spam_filter && (! find_friend(sender)) && 
			strcasecmp( sender, ymsg_sess->user )) {
				if ( (is_chat_spam(sender, sent_msg)  
				  || is_spam_name(sender) ) )  {
						snprintf(tbuf, 73, "%s", "[Spam]");
						spammer=1;
																 }
																 			    }

						if (!disp_auto_ignored ) {snprintf(tbuf, 6, "%s", " ");}
						append_char_pixmap_text((const char**)pixmap_pm_ignore, NULL);
						snprintf(buf, 1024, "  %s** %s : '%s'  - '%s' **\n%s",  
						YAHOO_COLOR_RED,
						_("Ignored offline private message from"),
						sender,_utf( tbuf),  YAHOO_COLOR_BLACK
						);					
						append_to_textbox_color(chat_window,
						NULL , buf );

						if (!spammer) {
							send_automated_response(sender, sender, "this user is not accepting PMs from you. Your offline message will not be shown to this user, and will be deleted, un-read.");	
											  }
						return;
										    } 


					if (find_temporary_friend(sender)) {limit=600;}
					if (find_friend(sender)) {limit=800;}

					if (( pm_lpackptr = find_pm_session( sender )) != NULL ) {
						pm_sess = pm_lpackptr->data;
					} else {
						if ( pm_in_sep_windows) {
							pm_sess=new_pm_session(sender);
							gtk_widget_show_all( pm_sess->pm_window );
							align_pms_repyto(pm_sess);
							}
						 }					

				if (! pm_in_sep_windows) {
	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_profile, NULL);	
				}

				snprintf(buf, 1022, "%s[%s]\n%s%s%s%s",
					YAHOO_COLOR_ORANGE, _("Offline Message"), 
					YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICON,
					sent_time , YAHOO_STYLE_ITALICOFF
				);

					if ( pm_in_sep_windows) {
						append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );
															} else {
						append_to_textbox_color(chat_window, NULL , buf );
																	  }
					snprintf(buf, 1024, "%s%s%s%s%s: ",
						YAHOO_COLOR_PMPURPLE, YAHOO_STYLE_BOLDON, sender,
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
					if ( pm_in_sep_windows) {
						append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );
															} else {
						append_to_textbox_color(chat_window, NULL , buf );
																	  }
					real_show_colors = show_colors;

					if (! strcmp("<ding>",sent_msg)) {
						/* it's a buzz, not a regular PM */
					snprintf(buf,limit, "\n<font size=\"16\">  %s%s%s %s%s %s[ %s ] %s%s\n", 
						"\033[#C65CC6m", YAHOO_STYLE_BOLDON, "** BUZZ!!! **",  
						"\033[#8DA8D4m", "** BUZZ!!! **", "\033[#AE46DBm", 								  sender , YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
				
					} else { snprintf( buf, limit, "%s\n\n", _utf(sent_msg));	} 

					if ( pm_in_sep_windows) {
						// if (show_emoticons) {convert_smileys(buf); }
						append_to_textbox_color( pm_sess->pm_window,
							pm_sess->pm_text, buf );
															} else {
						append_to_textbox_color(chat_window, NULL , buf );
						show_incoming_pm(pm_sess, 1);
																	  }
					real_show_colors = 1;
							           }

}


void append_timestamp(GtkWidget *somewidget, GtkWidget *somewidget2) {
	char timebuf[64]="";
	time_now = time( NULL );
	tm_now = localtime( &time_now );
	snprintf( timebuf, 62, "<font size=\"10\"> %s[%d/%d/%d %02d:%02d:%02d] ",
		YAHOO_COLOR_BLACK, tm_now->tm_mon + 1, tm_now->tm_mday,
		tm_now->tm_year + 1900, tm_now->tm_hour,
		tm_now->tm_min,tm_now->tm_sec );
	append_to_textbox_color(somewidget, somewidget2, timebuf );
}

void append_char_pixmap_text(const char **pixy, GtkWidget *textw) {
		GdkPixbuf *imbuf=NULL;
		GdkPixbuf *spixbuf=NULL;
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixy);
		if (imbuf) {
				int awidth=0;
				int aheight=0;
				int scaleto=24;
				if (! textw) {scaleto=20;}
				awidth = gdk_pixbuf_get_width( imbuf );
				aheight = gdk_pixbuf_get_height( imbuf );
				awidth = ( 1.0 * awidth / aheight ) * scaleto;
				aheight = scaleto;
				spixbuf = gdk_pixbuf_scale_simple( imbuf, awidth, aheight,
					GDK_INTERP_BILINEAR );
				g_object_unref(imbuf);
		}
		if (spixbuf) {
			GtkTextIter iter;
			GtkTextBuffer *t_buffer=NULL;
			int text_length;
			if (textw != NULL) { /* something other than main chat window */
				t_buffer= gtk_object_get_data(GTK_OBJECT(textw),"textbuffer");
								 } else {
				t_buffer=text_buffer22; /* default to chat window itself */
											}
				if (t_buffer==NULL) { t_buffer=text_buffer22;  }
				text_length = gtk_text_buffer_get_char_count(t_buffer);
				gtk_text_buffer_get_iter_at_offset(t_buffer, &iter, text_length );
				gtk_text_buffer_insert_pixbuf( t_buffer, &iter, spixbuf );
				g_object_unref(spixbuf);
		}
}

void append_to_open_pms(char *who, char *mytext, int with_timestamp) {
	/* appends various status messages to PM boxes if they are
		already open for a specific user */
	if (! who) {return;}
	if (strlen(who)<1) {return;}
	if (! mytext) {return;}
	if (( pm_lpackptr = find_pm_session( who )) != NULL ) {
	  char spacey[8];
	  pm_sess = pm_lpackptr->data;
	  if (with_timestamp) {append_timestamp(pm_sess->pm_window,pm_sess->pm_text);} 	
	  snprintf(spacey,6, "%s", " ");
	  append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, spacey );
	  append_char_pixmap_text((const char**)pixmap_status_here_male, pm_sess->pm_text);	
	  append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, spacey );
	  append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, mytext );
	  }
}


void toggle_sms_pm_session(char *who, int sms_on) {
	GList *sspm_lpackptr;
	PM_SESSION *npm_sess;
	char smsbuf[224];
		/* look for open PM windows where the other person's SMS 
		status (on/off) may have changed and make necessary changes */
	if (! who) {return;}
	if (strlen(who)<1) {return;}
	if (( sspm_lpackptr = find_pm_session( who )) != NULL ) {
	  npm_sess= sspm_lpackptr->data;
	} else {return; }  /* no PM window open for 'who' */

	if ( (sms_on > 0)  && (! npm_sess->sms) ) { /* turn it on SMS settings */
		npm_sess->sms=1;		
		if (npm_sess->window_style==0 ) { /* text entry */
				GtkWidget *pmstentry=gtk_object_get_data (GTK_OBJECT (npm_sess->pm_window), "pms_entry");
				if (pmstentry) {gtk_entry_set_max_length(GTK_ENTRY(pmstentry), 152);}
				}
		if (chat_timestamp_pm){append_timestamp(npm_sess->pm_window,npm_sess->pm_text);}
		snprintf(smsbuf, 6, "%s", " ");
		append_to_textbox_color( npm_sess->pm_window, npm_sess->pm_text, smsbuf );
		append_char_pixmap_text((const char**)pixmap_sms, npm_sess->pm_text);	
		snprintf(smsbuf, 220, "  %s[%s]%s\n\n",YAHOO_COLOR_BLUE, 
		_("The user is connected through SMS and will receive your message on a cell phone. Your message can be up to 152 characters."), YAHOO_COLOR_BLACK );
		append_to_textbox_color( npm_sess->pm_window, npm_sess->pm_text, smsbuf );
	}

	if ( (sms_on < 1) && (npm_sess->sms) ) { /* turn it off SMS settings */
		npm_sess->sms=0;
		if (npm_sess->window_style==0 ) { /* text entry */
				GtkWidget *pmstentry=gtk_object_get_data (GTK_OBJECT (npm_sess->pm_window), "pms_entry");
				if (pmstentry) {gtk_entry_set_max_length(GTK_ENTRY(pmstentry), 425);}
				}
		if (chat_timestamp_pm){append_timestamp(npm_sess->pm_window,npm_sess->pm_text);}
		snprintf(smsbuf, 6, "%s", " ");
		append_to_textbox_color( npm_sess->pm_window, npm_sess->pm_text, smsbuf );
		append_char_pixmap_text((const char**)pixmap_stealth_on, npm_sess->pm_text);	
		snprintf(smsbuf, 220, "  %s[%s]%s\n\n",YAHOO_COLOR_GREEN, 
		_("The user is no longer connected through the SMS cell phone service and can now receive regular sized messages."), YAHOO_COLOR_BLACK );
		append_to_textbox_color( npm_sess->pm_window, npm_sess->pm_text, smsbuf );
	}

}


   /* tattle-tale on friends online but invisible */
void check_for_invisible_friend(char *who) {
	if ( (! find_online_friend( who )) && (find_friend(who)) ) {
	   if (! friend_is_invisible(who)) {  /* Prevent unnecessary buddy list refreshing */
		set_buddy_status_full( who,_("Invisible"), 1); 
		update_buddy_clist();
												}
																			 			}
}


void show_back_from_invisible(char *who) {
	if (show_statuses) {
		if (!show_enters) {return;}
		if (yalias_name) {g_free(yalias_name);}
		yalias_name=get_screenname_alias(tmp);

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}

							snprintf(buf, 1024, "  %s%s** %s: '%s' ( %s )  %s **%s%s\n\n",
								YAHOO_STYLE_BOLDON, "\033[#00AABBm", 
								_("Invisible buddy"), who,  yalias_name, _("is now visible"), 
								YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
								append_to_textbox( chat_window, NULL, buf );
								append_to_open_pms(who, buf,chat_timestamp_pm);
						}
}




		/* used to provide security/privacy warnings for possible 'stalkers' */

void show_status_from_unknown_friend(char *who, char *status_type) {

char *tstat=NULL;	
char *whoy=NULL;
char *seppy=NULL;

/* we got some type of status message from a user NOT on our buddy list */
/* We'll show a privacy warning, because they may have us on their list 
                        without our knowledge */

if (ycht_only && ycht_is_running()) {return; }
if (!show_statuses) {  if (!disallow_random_friend_add) {return;}}
if (!show_enters) {  if (!disallow_random_friend_add) {return;}}
if (!who) {return;}
if (strlen(who)<4) {return;}
if (!status_type) {return;}

		seppy=strchr(who, 0x80);
		if (seppy) {
			char tst_friend[64];
			int backup=0;
			*seppy='\0';
			snprintf(tst_friend, 62, "%s", who);
			if (strlen(tst_friend)<2) {return;}
			backup=strlen(tst_friend)-1;
			if (find_profile_name(tst_friend))  {return;}  /* it's my evil twin - profile names ! */
			if (find_friend(tst_friend)) {return;}
			tst_friend[backup]='\0';
			if (find_profile_name(tst_friend))  {return;}  /* it's my evil twin - profile names ! */
			if (find_friend(tst_friend)) {return;}
			}

		if (! strcasecmp(who, ymsg_sess->user)) {return;}  /* it's me, im not a stalker ! */
		if (find_profile_name(who))  {return;}  /* it's my evil twin - profile names ! */
		if (find_friend(who)) {return;}

		tstat=strdup(status_type);
		whoy=_utf(who);

		 snprintf(buf,1500, "\n  %s%s**%s PRIVACY / SECURITY  WARNING:%s  GYach-E  received a status message from user %s'%s'%s who is NOT on your buddy list!  [Status message type: %s]   This  *may* mean the user has you on THEIR buddy list without your knowledge (a stalker?).  Or, the name %s'%s' %s  may be an alternate Yahoo! ID (profile name) for a friend you DO have on your buddy list.  You can remove yourself from this user's buddy list by clicking 'Tools' -> 'Yahoo Privacy Settings...' on the menu.  Or, you can add this user to your buddy list by clicking the 'Add Buddy...' button on the 'Buddies' tab.  See the 'Help' menu for more information. **\n%s%s", YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE, YAHOO_COLOR_RED,  YAHOO_COLOR_ORANGE, YAHOO_COLOR_GREEN, whoy, YAHOO_COLOR_ORANGE, tstat, YAHOO_COLOR_GREEN, whoy, YAHOO_COLOR_ORANGE,   YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF );

		append_to_textbox( chat_window, NULL, buf );
		play_sound_event(SOUND_EVENT_OTHER);
		free(tstat);
}


/* common callback for setting buddy object status */


void set_friend_stats(char *bud)  {

				if ( find_friend( bud )) {
					FRIEND_OBJECT=create_or_find_yahoo_friend(bud);

					if (FRIEND_OBJECT) {				
						FRIEND_OBJECT->away=0;
						FRIEND_OBJECT->idle=0;
						FRIEND_OBJECT->inchat=0;
						FRIEND_OBJECT->insms=0;
						FRIEND_OBJECT->launchcast=0;


					if ( strcmp( ymsg_field( "60" ), "" )) { /* SMS */
						FRIEND_OBJECT->insms =atoi(ymsg_field( "60" ));
						if (ymsg_sess->pkt.type != YMSG_BUDDY_OFF) {
							toggle_sms_pm_session(bud, FRIEND_OBJECT->insms ); 
						}
																		}		

					if ( strcmp( ymsg_field( "197" ), "" )) { /* Avatar */
						int need_ava=0;
						if (! FRIEND_OBJECT->avatar) {
							need_ava=1;
						} else {
							if (strlen(FRIEND_OBJECT->avatar)<3) {need_ava=1;}
								  }
						if (need_ava && (strlen(ymsg_field( "197" )) >3) )  {
									if (FRIEND_OBJECT->avatar) {g_free(FRIEND_OBJECT->avatar);}
									FRIEND_OBJECT->avatar=g_strdup(ymsg_field( "197" ));
						}
																		}	


					if ( strcmp( ymsg_field( "10001" ), "" )) { /* profile names link number */
						FRIEND_OBJECT->identity_id =atoi(ymsg_field( "10001" ));
																		}	

					if ( strcmp( ymsg_field("192"), "" )) { /* buddy image checksum */
						if (FRIEND_OBJECT->buddy_image_hash) {
							g_free(FRIEND_OBJECT->buddy_image_hash);
							FRIEND_OBJECT->buddy_image_hash=NULL;
						}
						if ( (atoi( ymsg_field("192")) != -1) && (atoi( ymsg_field("192")) != 0)) { 
							/* they have a buddy image checksum */ 
							FRIEND_OBJECT->buddy_image_hash=g_strdup(ymsg_field("192"));
						} 
																		}	
	

					if ( strcmp( ymsg_field( "17" ), "" )) { /* chat */
						FRIEND_OBJECT->inchat =atoi(ymsg_field( "17" ));
																		}	


					if ( strcmp( ymsg_field( "13" ), "" )) { /* chat/games? */
						int gamechat=atoi(ymsg_field( "13" ));

						/* in chat */
						if (gamechat==3)  {FRIEND_OBJECT->inchat=1;}
							/* the person just went invisible but is still in chat =2 */
						if (gamechat==2)  {FRIEND_OBJECT->inchat=1;}

						/* in games */
						if (gamechat==5)  {FRIEND_OBJECT->ingames=1;}
							/* the person just went invisible but is still in games =4 */
						if (gamechat==4)  {FRIEND_OBJECT->ingames=1;}

						/* in games AND chat */
						if (gamechat==7)  {
							FRIEND_OBJECT->inchat=1;
							FRIEND_OBJECT->ingames=1;
													  }

							/* the person just went invisible 
								but is still in both games AND chat =6 */
						if (gamechat==6)  {
							FRIEND_OBJECT->inchat=1;
							FRIEND_OBJECT->ingames=1;
													  }

																		}		

						if (! FRIEND_OBJECT->ingames)  {
							if (FRIEND_OBJECT->game_url) {
								g_free(FRIEND_OBJECT->game_url);
								FRIEND_OBJECT->game_url=g_strdup("");
							}
							if (FRIEND_OBJECT->game_stat) {
								g_free(FRIEND_OBJECT->game_stat);
								FRIEND_OBJECT->game_stat=g_strdup("");
							}
						}

					if ( strcmp( ymsg_field( "47" ), "" )) { /* away/idle */
						int away=atoi(ymsg_field( "47" ));
						if ((away==2) && (strcmp(ymsg_field("10"), "0")) ) {
							FRIEND_OBJECT->idle=1;
							if (strlen(FRIEND_OBJECT->idle_stat)<2)  
									{
								g_free(FRIEND_OBJECT->idle_stat);
								FRIEND_OBJECT->idle_stat=g_strdup("00:00:00");
									}

										   }
						if (away==1) {FRIEND_OBJECT->away=1;}
						if (away==0) {FRIEND_OBJECT->away=0; FRIEND_OBJECT->idle=0;}
						if ( (away==1) || (away==0))  {
								g_free(FRIEND_OBJECT->idle_stat);
								FRIEND_OBJECT->idle_stat=g_strdup("");
																	}
																			}	

					if (( atoi( tmp2 ) == 999 ))  {FRIEND_OBJECT->idle=1;}
					if (( atoi( tmp2 ) >0 ) && ( atoi( tmp2 ) <13 ))  {FRIEND_OBJECT->away=1;}

					if (! strncasecmp( ymsg_field( "184" ),  "ystatus=1", 9 ) &&
						 strcmp(ymsg_field( "187" ), "") ) { 
							/* webcam is on */
								/* USUALLY field 187=1 in this case, but I've seen 	
								187=1 in other cases when launchcast wasnt on */
							FRIEND_OBJECT->webcam=1;
					}

					if (! strncasecmp( ymsg_field( "184" ),  "ystatus=2", 9 ) &&
						 strcmp(ymsg_field( "187" ), "") ) { /* Launchcast status */
								/* USUALLY field 187=2 in this case, but I've seen 	
								187=2 in other cases when launchcast wasnt on */
						char radiostat[32]="";
						char *launchptr=NULL;
						char *launchc=strdup(ymsg_field( "184" ));						
						FRIEND_OBJECT->launchcast=1;
						sprintf(radiostat,"%s","");
						launchptr=strstr(launchc, "\tm\t");
							if (launchptr)  {
								char *launchend=NULL;
								char *launchstat=NULL;
								char *launcher;
								launchptr+=3;
								launchend=strchr(launchptr,'\t');
								if (launchend) {*launchend='\0';}
								launcher=strdup(launchptr);
								launchstat=get_launchast_genre(launcher);
								free(launcher);
								if (strlen(launchstat)>1) {
									snprintf(radiostat,29,"\"%s", launchstat);
									strcat(radiostat,"\"");
								}
								free(launchstat);
							}
							free(launchc);
		
							if (FRIEND_OBJECT->radio_stat){
									g_free(FRIEND_OBJECT->radio_stat);
							}
							if (FRIEND_OBJECT->launchcast) {
								FRIEND_OBJECT->radio_stat=g_strdup(radiostat);
											         					} else {
								FRIEND_OBJECT->radio_stat=g_strdup("");
																				  }
						} /* end 184 */


					if ( strcmp( ymsg_field( "137" ), "" )) { /* seconds idle */
						char otherstat[16]="";
						int away=atol(ymsg_field( "137" ));

						/* sometimes Yahoo gives us bogus 'idle' times, so 
							make it look error-ish if they do that...say if they
							are telling us someone has been idle more than 2 months */

							if (away>5356800) {away=-1;}
							if (away< -1) {away=-1;}
							snprintf(otherstat, 15, 
							"%02d:%02d:%02d",
							away/3600, ((away/60)%60), away%60
							);							
		
							if (FRIEND_OBJECT->idle_stat) {g_free(FRIEND_OBJECT->idle_stat);}
							FRIEND_OBJECT->idle_stat=g_strdup(otherstat);
											         					} else {
							if (FRIEND_OBJECT->idle_stat) {g_free(FRIEND_OBJECT->idle_stat);}
							FRIEND_OBJECT->idle_stat=g_strdup("");
																				  }

					if ( (ymsg_sess->pkt.type == YMSG_BACK) || 
					(ymsg_sess->pkt.type == YMSG_BUDDY_ON) ) {
						if (FRIEND_OBJECT->idle_stat) {
								g_free(FRIEND_OBJECT->idle_stat);
								FRIEND_OBJECT->idle_stat=g_strdup("");
								FRIEND_OBJECT->idle=0;
							}
																					}

												  }
										}  else   {		
		show_status_from_unknown_friend(bud, "Update buddy list");
													 }
}


/* common callback for when buddy comes online */

void display_buddy_online(char *frtmp) {
	int from_invisible=0;
	if ((!find_friend(frtmp)) && (! ycht_only) ) {
		show_status_from_unknown_friend(frtmp, "Buddy online"); 
		 return;
								}

	if (ymsg_sess->pkt.type==YMSG_IDACT) {from_invisible=1;}
	if (ymsg_sess->pkt.type==YMSG_BACK) {from_invisible=1;}
	if (ymsg_sess->pkt.type==YMSG_AWAY) {from_invisible=1;}	
	if (ymsg_sess->pkt.type==YMSG_BUDDY_STATUS) {from_invisible=1;}
	if (ymsg_sess->pkt.type==YMSG_BUDDY_ON) {
		if (strcmp(ymsg_field("19"), "")) {from_invisible=1;}  /* came on with custom status */
		if (strcmp(ymsg_field("10"), "0")) {from_invisible=1;} /* a 'busy' or idle status */
		if (strcmp(ymsg_field("187"), "")) {from_invisible=1;} /* custom status */
		if (strcmp(ymsg_field("184"), "")) {from_invisible=1;} /* launchcast or webcam */
		if (emulate_ymsg6) {
			if (! strcmp(ymsg_field("192"), "")) {from_invisible=1;} /* 192 not present */ 
		}
	}

	if (!find_online_friend(frtmp))  {
		if (show_enters) {

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_buddy_online, NULL);	

 						snprintf(buf, 1024, "  %s%s**%s %s: %s%s%s  ",
 						YAHOO_STYLE_BOLDON,YAHOO_COLOR_BLUE,YAHOO_COLOR_RED, _("Buddy"), 		YAHOO_COLOR_PURPLE,frtmp,YAHOO_COLOR_BLUE);

						strncat(buf,_("has logged ON"), 64);
						strcat(buf,". **\n");
						strcat(buf,YAHOO_COLOR_BLACK);
						strcat(buf,YAHOO_STYLE_BOLDOFF);


				/* Tell if friends are in Games or Chat when they sign-on */
				FRIEND_OBJECT=create_or_find_yahoo_friend(frtmp);
				if (FRIEND_OBJECT) {

					if ( (! strcasecmp(ymsg_sess->user, frtmp)) || find_profile_name(frtmp)) {
						if (my_avatar && ( ! FRIEND_OBJECT->avatar)) {
							FRIEND_OBJECT->avatar=g_strdup(my_avatar);
						}
					}

					if (FRIEND_OBJECT->insms) {from_invisible=0;}

					strcat(buf,"\033[#5791B1m");
					/* show people hiding in chat and games while invisible */
					if (FRIEND_OBJECT->ingames) {
						strcat(buf, "  ** ");
						strncat(buf,_("The user is in Yahoo! Games."), 70);
						strcat(buf," **\n");
						from_invisible=1;
																		  }
					if (FRIEND_OBJECT->inchat) {
						strcat(buf, "  ** ");
						strncat(buf,_("The user is in Yahoo! Chat."), 70);
						strcat(buf," **\n");
						from_invisible=1;
																	  }
					strcat(buf,YAHOO_COLOR_BLACK);
													 }

					append_to_textbox(chat_window,NULL,buf);
					append_to_open_pms(frtmp, buf,chat_timestamp_pm);

					}  /* end show_enters */

	add_online_friend( frtmp );		
	play_sound_event(SOUND_EVENT_BUDDY_ON);
	if (ymsg_sess->pkt.type==YMSG_GAMES_ENTER) {from_invisible=0;}
	if (strcmp(ymsg_field("60"), "")) { /* sms */ 
		if (atoi(ymsg_field("60")) ==1) {
			/* SMS users sign on with a custom status but are not 
			signing on 'from invisible', so fix */ 
			from_invisible=0;
		}
		toggle_sms_pm_session(frtmp, atoi(ymsg_field("60") ) ); 
	}

	if (from_invisible) {show_back_from_invisible(frtmp);}
	if (popup_buddy_on) {
		show_popup_dialog(tmp, from_invisible?_("has logged ON\n[is now visible]"):_("has logged ON"), 11);
	}

	snprintf(tmp3, 140, "%s: %s", tmp, _("has logged ON"));	
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 

					 }  /* end find_online_friend */
}


int split( char *str, char *connector )
{
	char	*last;
	char	*orig;
	//char	*packptr;
	char	tmp_str[ 2048 ];
	int	result = 0;
	int i = 0;

	strncpy( tmp_str, str, 2047 );
	str = tmp_str;

	last = orig = str;

	while (( packptr = strstr( str, connector ))) 
	{
		*packptr = '\0';
		strcpy( ymsg_fields[result], last );
		str = last = packptr + strlen( connector );
		result++;
		if (result>295) {break;}
	}

	if ( last != orig ) {
		strcpy( ymsg_fields[result], last );
	} else {
		result--;
	}

	strcpy( ymsg_fields[result], "__END__" );
	strcpy( ymsg_fields[result+1], "__END__" );

	i = 0;
	while( strcmp( ymsg_fields[i], "__END__" )) {
		if (( ymsg_sess->debug_packets ) ) {
			fprintf( stderr, "%-3s = '%s'\n",
				ymsg_fields[i], ymsg_fields[i+1] );			
			fflush( stderr );
			if ( capture_fp ) {
				fprintf( capture_fp, "%-3s = '%s'\n",
					ymsg_fields[i], ymsg_fields[i+1] );	
				fflush( capture_fp );
				}
		}
		i += 2;
	}

	return( result + 1 );
}


char *ymsg_field_p( char *key ) {
	int i = 0;
	static char result[2048];
	//char *packptr;

	result[0] = '\0';
	while( strcmp( ymsg_fields[i], "__END__" )) {
		if ( ! strcmp( ymsg_fields[i], key )) {


			/* Disabled: PhrozenSmoke - dont skip over our small 'C' cookie */ /*
			if ( ymsg_fields[i+1][0] == 'C' ) {
				i += 2;
				continue;
			} */


			if ( !strcmp( key, "59" )) {
				packptr = strstr( ymsg_fields[i+1], "; expires" );
				if ( packptr )
					*packptr = '\0';
			}

			if ( result[0] ) {
				char cookbud[8];
				snprintf(cookbud, 6, "%c ", 0x04);
				strcat( result, cookbud );
			}
			strcat( result, ymsg_fields[i+1] );
		}
		i += 2;
	}
	return( result );
}

char *ymsg_field( char *key ) {
	int i = 0;
	static char result[2048];
	//char *packptr;

	result[0] = '\0';
	while( strcmp( ymsg_fields[i], "__END__" )) {
		if ( ! strcmp( ymsg_fields[i], key )) {
			if ( !strcmp( key, "59" )) {
				packptr = strstr( ymsg_fields[i+1], "; expires" );
				if ( packptr )
					*packptr = '\0';
			}

			if ( result[0] ) {
				strcat( result, "," );
			}
			strcat( result, ymsg_fields[i+1] );

			if ( !strcmp( key, "88" )) { /* ignore list */
				if (! strcmp(result, "")) {strcat(result, "\x0a");}
			}
			if ( !strcmp( key, "87" )) { /* buddy list */
				if (! strcmp(result, "")) {strcat(result, "\x0a");}
			}


		}
		i += 2;
	}
	return( result );
}


void clear_chat_list_members() {
	gy_empty_model(gtk_tree_view_get_model(chat_users), GYTV_TYPE_LIST);
}



/* method added by PhrozenSmoke */
void handle_logoff() {
	int hours;
	int minutes;
	int seconds;

	logged_in = 0;
	immed_rejoin=0;
	got_y_ignore_list=0;
	got_y_buddy_list=0;
	got_buddy_ignore_list=0;
	set_menu_connected( 0 );
	show_mail_stat("0");


	if (ycht_is_running()) { ycht_logout(1);}
	clear_chat_list_members();
	plugins_cleanup_disconnect();
	if (my_avatar) {free(my_avatar); my_avatar=NULL;}

				if (ymsg_sess->sock != -1) {close( ymsg_sess->sock );}

				if ( ymsg_sess->io_callback_tag != 0) {
					gdk_input_remove( ymsg_sess->io_callback_tag );
					ymsg_sess->io_callback_tag = 0;
				}

				ymsg_sess->sock = -1;

				if ( ymsg_sess->ping_callback_tag != 0) {
					gdk_input_remove( ymsg_sess->ping_callback_tag );
					ymsg_sess->ping_callback_tag = 0;
				}

				/* reset cookies */
				ymsg_sess->cookie[0] = '\0';
				reset_friend_list_spool();
				time_now = time(NULL);

				hours = (int)(( time_now - connect_time ) / 3600);
				minutes = (int)(( time_now - connect_time ) % 3600 ) / 60;
				seconds = (int)(( time_now - connect_time ) % 60 );
				connect_time = 0;

				/* show user that we aren't logged in anymore */
					append_timestamp(chat_window, NULL);
	  				snprintf(buf,6, "%s", "   ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_buddy_offline , NULL);	

				snprintf( buf, 512, "  %s%s%s%s%s  %s  %d : %02d : %02d\n\n",
					YAHOO_COLOR_BLUE, YAHOO_STYLE_BOLDON,
					_("Disconnected from Yahoo!    See Ya!"),
					YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK,
					_("Online for")  ,hours, minutes, seconds );
				append_to_textbox_color( chat_window, NULL, buf );

				gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
				gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
					_("Not currently connected to Yahoo!.") );

				remove_all_online_friends( ) ;
				clear_perm_igg_hash();
				if (! strcmp( ymsg_sess->room, "[NONE]") ) {
					login_noroom=1;
					in_a_chat=0;
				}
				if ( auto_reconnect ) {	
					if (in_a_chat) {
						login_noroom=0;
						if (last_chat_room_used) {
							strncpy(ymsg_sess->room, "[NONE]", 62);
							strncpy(ymsg_sess->req_room,last_chat_room_used, 62);
																 }
										}
								login_to_yahoo_chat(); 
											  } 
}


void handle_dead_connection (int broken_pipe, int disable_reconn, int disable_dialog, char *reason) {   
	int org_con=auto_reconnect;
	int disabconn=disable_reconn;
	/* we never fully logged in, something's wrong, so disable auto-reconnect
	this time around */ 
	if (!got_buddy_ignore_list) {disabconn=1;}

	if (broken_pipe && (!disabconn)) {
		snprintf(buf,400, "\n  %s** %s [Probably booted...reconnecting] **%s\n\n",
		YAHOO_COLOR_RED, reason, YAHOO_COLOR_BLACK);
		auto_reconnect=1;
	} else  {
		if (disabconn) {auto_reconnect=0;}
		snprintf(buf, 400, "\n  %s** %s [ %s ] **%s\n\n" ,YAHOO_COLOR_RED, reason, _utf(ymsg_sess->error_msg), YAHOO_COLOR_BLACK);
	}
	append_to_textbox_color( chat_window, NULL, buf );	
	handle_logoff();

	if (broken_pipe && (!disabconn)) {
		snprintf(buf, 400, "%s\n%s",reason, _("(Attempting to reconnect)"));
	} else  {
		snprintf(buf, 400, "%s\n%s",reason, _utf(ymsg_sess->error_msg));
		if (disabconn) {
			login_window = build_login_window();
			gtk_widget_show_all( login_window );
		}
	}
	if (!disable_dialog) {show_ok_dialog(buf);}
	auto_reconnect=org_con;
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid, reason );
}


void check_connection_timedout() {
	if (ycht_only) {return;}
	if (!logged_in) {return;}
	if (got_buddy_ignore_list) { push_friend_list_spool(); return;} /* connection okay */
	else {
		int seconds=0;
		time_now = time(NULL);
		seconds = (int)(( time_now - connect_time ) % 60 );

		if (seconds>=17) {
			if ( got_y_buddy_list || got_y_ignore_list) {
					got_y_ignore_list=1;
					got_y_buddy_list=1;
					if (!got_buddy_ignore_list) {
							got_buddy_ignore_list=1;
							push_friend_list_spool();
							show_offline_messages();
							flooder_buddy_list_protect();
							start_chat_online();
							return;
						}
			}
		}

		if (seconds>=19) {
				/* connection has timed out: We've sent 
				login strings, etc. but have received no 
				verification or error msgs for 17+ seconds, hangup  */
			snprintf( ymsg_sess->error_msg, 127, "%s", 
			_("Could not login: Connection timed out.") );
			handle_dead_connection (0,1,0,_("Could not login: Connection timed out."));
		}
	}
}



/* Begin packet handling callbacks */

void handle_goto_error() {
			  /* added: PhrozenSmoke
			   if we get a 'goto' response back, it means 
			   the user is not in a chat room  */
			show_ok_dialog(_("The user is not currently in a chat room."));
			snprintf(buf, 1024, "  %s%s** %s **%s%s\n",
				"\033[#BA50A8m", YAHOO_STYLE_ITALICON,
				_("The user is not currently in a chat room."), 
				YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF
				);
			append_to_textbox( chat_window, NULL, buf );
}

void handle_webcam() {
	check_boot_attempt();
	if (strcmp( ymsg_field( "61" ), "" )) {
		yahoo_process_webcam_key(ymsg_field( "61" ));
														}
}


void handle_voice_chat_invite() {

	/* we get other 'voice chat' packets that have no name 
	but have some status - and have the person's name in the 
	field "5" , If a PM session is open, print the error message */

	if (strcmp(ymsg_field("10"), "") && strcmp(ymsg_field("19"), "") &&  
	  strcmp(ymsg_field("5"), "") && (! strcmp(ymsg_field("4"), ""))  ) {
		if ( (! strncmp(ymsg_field("10"), "99", 2))  && (! strncmp(ymsg_field("19"), "-", 1)) ) {
			/* A Voice chat invite we sent didn't make it to the recipient either
				because they are unavailable or they have their privacy settings
				setup to block us, these messages have field '10' set to '19', 
				and a field '99' set to some negative number */
			strncpy( tmp, ymsg_field( "5" ), 80);
			/* possible spoofs: Messages from ourselves, so ignore */
				if (! from_valid_sender(tmp) ) {return;}
			if (is_flooder(tmp)) {  return;}
			if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
				pm_sess = pm_lpackptr->data;
				snprintf(buf, 768, "   %s** %s ** %s\n", 
					YAHOO_COLOR_RED, 
					_("The user did not receive your voice chat invitation because they are either unavailable or have their privacy settings configured to block you."),
					YAHOO_COLOR_BLACK);
				append_to_textbox_color( pm_sess->pm_window,
							pm_sess->pm_text, buf );
				play_sound_event(SOUND_EVENT_REJECT);
			}
		}	
		return ;
	}  /* end Voice chat  error messages */
		
		/* avoid repeated invites from idiots and spammers, sound bombs */
		if ( strcmp( ymsg_field( "4" ), "" )) {
				log_possible_flood_attack( ymsg_field( "4" ), 2,"Voice Chat Invites");
				if (is_flooder(ymsg_field( "4" ))) {  return;}

			/* possible spoofs: Messages from ourselves, so ignore */
				if (! from_valid_sender(ymsg_field("4") ) ) {return;}
														   }

			if ( strcmp( ymsg_field( "4" ), "" )  && strcmp( ymsg_field( "233" ), "" ) && 
					strcmp( ymsg_field( "57" ), "" )) {
						strncpy(tmp,ymsg_field( "4" ), 255);  /* who */
						strncpy(tmp2,ymsg_field( "57" ), 255);  /* room */		
						strncpy(tmp3,ymsg_field( "233" ), 255);  /* cookie */		
					
						check_for_invisible_friend(tmp);

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room 
				 */
				accept_pm = get_pm_perms(tmp);

				if ( (! accept_pm) || (! allow_py_voice_helper) )  {
					yalias_name=get_screenname_alias(tmp);
					comm_block_notify(tmp, yalias_name, 
						_("Ignored voice chat invitation from"),  
						"this user is not currently accepting voice chat invitations from you." , 
						1, 1);
							return ;
											    }

						/* Not from a PM, show in room */																					   						yalias_name=get_screenname_alias(tmp);

					if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
						GtkWidget *okbutton;
						pm_sess = pm_lpackptr->data;
						align_pms_repyto(pm_sess);

					if ( auto_raise_pm ) {
						gdk_window_raise( pm_sess->pm_window->window );
						focus_pm_entry(pm_sess->pm_window);
													}

					if (chat_timestamp_pm){ 
						append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
					}
	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_voice, pm_sess->pm_text);	

					snprintf(buf, 1024, "%s%s   ** '%s' %s **%s%s\n",
						YAHOO_COLOR_ORANGE, YAHOO_STYLE_BOLDON, pm_nick_names?yalias_name:tmp,
						_("has enabled voice chat."), 
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
					append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );

					/* dialog box */
					if (allow_py_voice_helper) {
					snprintf(buf, 512, "%s:\n\n'%s'",_("Would you like to enable voice chat for this user?"), tmp);
					strncat(buf,"\n\n", 4);
					strncat(buf,_("Voice chat will be disabled in any other rooms or conferences."), 256);
					okbutton=show_confirm_dialog(buf,"Enable Voice Chat","Cancel");
					if (!okbutton) {return;}
  					gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      		GTK_SIGNAL_FUNC (on_accept_voice_conf), NULL);
					gtk_object_set_data (GTK_OBJECT (okbutton), "pmroom", strdup(tmp2));
					gtk_object_set_data (GTK_OBJECT (okbutton), "cookie", strdup(tmp3));
					gtk_object_set_data( GTK_OBJECT(okbutton), PROFNAMESTR, strdup(get_profile_replyto_name()) );
					play_sound_event(SOUND_EVENT_OTHER);
														}

																										}  else {

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_voice, NULL);	

						snprintf(buf, 256, "  %s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("User")); 
				  		strcat(buf, "'");
				 		strncat(buf,  tmp, 80);  /* other user */
				  		strcat(buf, "' ( ");
				  		strncat(buf, yalias_name, 80);
				  		strcat(buf, " ) ");
				  		strncat(buf, _("has sent you a voice chat invitation."), 80);

						strcat(buf, "  ");
				  		strncat(buf, _("Conference Room"), 40);
						strcat(buf, ": ");
						strncat(buf, _utf(tmp2), 128);

						 /* Status is field '13' */

				  		strcat(buf, YAHOO_STYLE_ITALICOFF);
				  		strcat(buf, " **\n");
						append_to_textbox( chat_window, NULL, buf );
						play_sound_event(SOUND_EVENT_OTHER);
																													} /* else */
																	   }   /* ymsg field '4' */
}


void handle_new_contact() {

			if ( strcmp( ymsg_field( "3" ), "" )) {

			strncpy(tmp,ymsg_field( "3" ), 255);

			if ( strcmp( ymsg_field( "1" ), "" )) {
			GtkWidget *okbutton;
  			GtkWidget *cbutton;
				/* added: PhrozenSmoke, somebody added us as a friend */

			if ( strcmp( tmp, "" )) {
				log_possible_flood_attack( tmp, 2,"Buddy List Adds");
				if (is_flooder(tmp)) { 
					ymsg_reject_buddy(ymsg_sess, tmp); 
					return;				 }
										}

					if ( ignore_check( tmp )) {
						/* automatically reject an igged user*/
						if ( (! find_friend( tmp )) && (! find_temporary_friend(tmp)) ) {
							ymsg_reject_buddy(ymsg_sess, tmp);
							if (!find_friend(tmp)) { preemptive_strike(tmp); }
							return;
										   }
														}

			yalias_name=get_screenname_alias(tmp);

				/* With this setting, only people who are on YOUR friends list 
				   can add you to their friends list, or you have to temporarily disable 
				   the 'add buddy' blocker and allow them to be added - used as a 
				   way to stop random weirdos from adding you to their buddy list */

				if (disallow_random_friend_add &&
					(!find_friend(tmp)) && (!find_profile_name(tmp)) )  {

					ymsg_reject_buddy(ymsg_sess, tmp); /* auto-reject */
					comm_block_notify(tmp, yalias_name, 
						"blocked an attempt to add you as a friend from",  
						"this user is not allowing random people to add him/her to their buddy list. Please ask this user for permission first before attempting to add him/her to your buddy list." , 
						1, 1);
					ymsg_reject_buddy(ymsg_sess, tmp); /* auto-reject again, just in case */ 
					return;
								    }

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_status_here_male, NULL);	

			snprintf(buf, 512, "  %s%s** '%s' ( %s ) ", YAHOO_STYLE_ITALICON, YAHOO_COLOR_GREEN, tmp, yalias_name);
			strncat(buf,_("has added you to their buddy list."), 80);
			strcat(buf," **\n");
			strcat(buf,YAHOO_STYLE_ITALICOFF);
			strcat(buf,YAHOO_COLOR_BLACK);
			append_to_textbox( chat_window, NULL, buf );
			append_to_open_pms(tmp, buf,chat_timestamp_pm);


					/* dialog box */
					snprintf(buf, 768, "%s:\n\n'%s'",_("Would you like to allow this user to add you to their buddy list?"), tmp);

			if ( strcmp( ymsg_field( "14" ), "" )) { /* message */
			strncat(buf,"\n\n", 4);
			strncat(buf,_("Message"), 32);
			strcat(buf," : '");
			strncat(buf, _utf(ymsg_field( "14" )), 150);
			strcat(buf,"' ");

										}

			if ( strcmp( ymsg_field( "15" ), "" )) { /* timestamp if we were offline */
				char timebuf[40]="";
				char *tmp_time=NULL;
				time_t mytime;
				mytime=time(NULL);
				mytime=mytime-time(NULL)+atol(ymsg_field( "15" ));				
				tmp_time=strdup(ctime_r( &mytime, timebuf));
				snprintf(tmp3, 80, "%s", tmp_time);
				free(tmp_time);
				strcat(buf,"\n");
				strncat(buf, tmp3, 82);
				}

				okbutton=show_confirm_dialog_config(buf,"Yes","No",0);
				if (!okbutton) {ymsg_reject_buddy(ymsg_sess, tmp);  return;}
  				gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_add_accept), NULL);
				gtk_object_set_data (GTK_OBJECT (okbutton), "buddy", strdup(tmp));

				cbutton=gtk_object_get_data( GTK_OBJECT(okbutton), "cancel" );
				if (cbutton) {
  					gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      		GTK_SIGNAL_FUNC (on_reject_buddy_add), NULL);
				gtk_object_set_data (GTK_OBJECT (cbutton), "buddy", strdup(tmp));
						 }

				play_sound_event(SOUND_EVENT_OTHER);

															} else {  /* field "1" missing */

				/* somebody has rejected our attempt to add them as a friend */

		if ( strcmp( tmp, "" )) {
				log_possible_flood_attack( tmp, 2,"Buddy List Declines");
				if (is_flooder(tmp)) {return;}
										}

			if ( ignore_check( tmp )) { if (!find_friend(tmp)) { preemptive_strike(tmp); } return;}

			snprintf(buf, 512, "  %s%s** '%s' ( %s ) ", YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED, tmp, yalias_name);
			strncat(buf,_("has declined to be added to your buddy list."), 80);
			if ( strcmp( ymsg_field( "14" ), "" )) { /* message */
			strcat(buf,"  ");
			strncat(buf,_("Message"), 32);
			strcat(buf," : '");
			/* set 75 byte limit on message, avoid 'boots' from large messages */
			strncat(buf, _utf(ymsg_field( "14" )), 130);
			strcat(buf,"' ");

									   }
			strcat(buf," **\n");
			strcat(buf,YAHOO_STYLE_ITALICOFF);
			strcat(buf,YAHOO_COLOR_BLACK);
			append_to_textbox( chat_window, NULL, buf );
			snprintf(buf, 512, "'%s' ( %s ) \n", tmp, yalias_name);
			strncat(buf,_("has declined to be added to your buddy list."), 80);
			strcat(buf,"\n\n");
			strncat(buf,_("Message"), 32);		
			strcat(buf," : '");
			/* set 75 byte limit on message, avoid 'boots' from large messages */
			strncat(buf, _utf(ymsg_field( "14" )), 130);
			strcat(buf,"'");	
			show_ok_dialog(buf);
			play_sound_event(SOUND_EVENT_REJECT);
								
			if (find_online_friend(tmp)) {remove_online_friend(tmp);}	
			if ( find_friend( tmp ))  { /* remove the friend if they exist */				
				remove_friend(tmp);		
							 }
				remove_old_friend_from_list(tmp);	
				remove_buddy_status( tmp);
				update_buddy_clist();
																		}
																	   		}  /* field "3" */
}



void handle_add_buddy() {
			if ( strcmp( ymsg_field( "7" ), "" )) {
			int adderr=0;

			if ( strcmp( ymsg_field( "66" ), "" ) ) {
				adderr=atoi(ymsg_field( "66" ));
			}

			if ( adderr==3 ) {
				/* Buddy could not be added because
				the name doesn't exist */
			snprintf(buf, 400, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Invalid User."),
			_utf(ymsg_field( "7" )), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			snprintf(tmp3, 255, "%s:\n'%s'", _("Invalid User."), _utf(ymsg_field( "7" )));
			show_ok_dialog(tmp3);
			return ;
			}

			if ( adderr==2 ) {
				/* Buddy already exists on the list */
			snprintf(buf, 400, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Buddy already exists."),
			_utf(ymsg_field( "7" )), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			snprintf(tmp3, 255, "%s:\n'%s'", _("Buddy already exists."), _utf(ymsg_field( "7" )));
			show_ok_dialog(tmp3);
			return ;
			}

			if ( adderr>0 ) {
				/* Some other error that prevented adding */
			snprintf(buf, 400, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Buddy could not be added."),
			_utf(ymsg_field( "7" )), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			snprintf(tmp3, 255, "%s:\n'%s'", _("Buddy could not be added."), _utf(ymsg_field( "7" )));
			show_ok_dialog(tmp3);
			return ;
			}

				strncpy(tmp2, ymsg_field( "7" ), 128);
				if (! find_friend(tmp2)) { 
					if (strcmp(ymsg_field( "65" ),"")) {  /* Buddy Group */
						strncpy(tmp3, ymsg_field( "65" ), 96);
					} else {strncpy(tmp3, "Buddies", 96);}

					remove_flooder( tmp2 );
					if (find_temporary_friend(tmp2) ) {remove_temporary_friend(tmp2); }
					if (ignore_check(tmp2) && (! perm_igg_check(tmp2) )) {ignore_toggle( tmp2 );}
					if (mute_check(tmp2)) {mute_toggle( tmp2 );}

					append_new_friend_to_list(tmp2);
					FRIEND_OBJECT=create_or_find_yahoo_friend(tmp2);
					if (FRIEND_OBJECT)  {
						if (FRIEND_OBJECT->buddy_group) {g_free(FRIEND_OBJECT->buddy_group);}
						FRIEND_OBJECT->buddy_group=g_strdup(tmp3);
					}
					set_buddy_status_full( tmp2 ,"",0 ); 
					update_buddy_clist();
					ignore_mailstat=1;
					ymsg_refresh( ymsg_sess);

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_status_here_male, NULL);	

					play_sound_event(SOUND_EVENT_OTHER);
					snprintf(buf, 1024, "%s  ** '%s'  %s **%s\n", "\033[#D8BEFFm", tmp2, 
						_("has been added to your buddy list"),  YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
					append_to_open_pms(tmp2, buf,chat_timestamp_pm);
											  }
										}
}

void handle_rem_buddy() {
			if ( strcmp( ymsg_field( "7" ), "" )) {  /* add real deletion here */
				int errok=0;
				strncpy(tmp2, ymsg_field( "7" ), 128);

				if ( strcmp( ymsg_field( "66" ), "" )) { 
					/* Error '3' means the buddy is not on our list */
					if (atoi(ymsg_field( "66" ))==0) {errok=1;}  /* status okay */
					if (atoi(ymsg_field( "66" ))==2) {errok=1;}
				} else {errok=1;}

				if (!errok) {return;}
					
					if (find_online_friend(tmp2)) {remove_online_friend(tmp2);}			
					if (find_friend(tmp2)) { 
						remove_friend(tmp2);
						play_sound_event(SOUND_EVENT_OTHER);

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_status_here_male, NULL);	

						snprintf(buf, 1024, "%s  ** '%s'  %s **%s\n", "\033[#FFB8AFm", tmp2, 
						_("has been removed from your buddy list"),  YAHOO_COLOR_BLACK );
						append_to_textbox( chat_window, NULL, buf );
												    }
					remove_old_friend_from_list(tmp2);
					remove_buddy_status( tmp2);
					ignore_mailstat=1;
					update_buddy_clist();
					ymsg_refresh( ymsg_sess);

									   						 }
}


void show_connected_display() {
		append_timestamp(chat_window, NULL);
		snprintf(buf,6, "%s", "  ");
		append_to_textbox( chat_window, NULL, buf );
		append_char_pixmap_text((const char**)pixmap_buddy_online , NULL);	
		snprintf(buf , 300, "   %s%s%s\n\n",
								YAHOO_COLOR_GREEN, _("CONNECTED: type /help for help"),
								YAHOO_COLOR_BLACK); 
		append_to_textbox( chat_window, NULL, buf );

		if (using_web_login) {
			snprintf(buf, 1022, "\n%s\n\n", "Chat is available using the older YCHT protocol. Chat 'nicknames' and quick profiles will not be available in chat rooms.");
			append_to_textbox( chat_window, NULL, buf );
		}
		if (ycht_only) {
			snprintf(buf, 1022, "\n%s\n\n", "You are connected using the YCHT protocol. The only feature available is CHAT. Buddy lists, conferences, webcams, and almost all other features are unavailable.");
			append_to_textbox( chat_window, NULL, buf );
		}
}

void handle_get_key() {
	connect_time = time(NULL);
	if (ycht_only) { show_connected_display();  return; }
	strncpy( tmp, ymsg_field( "94" ), 2046);
	clear_offline_messages();
	got_y_ignore_list=0;
	got_y_buddy_list=0;
	got_buddy_ignore_list=0;
	reset_profile_name_config();
	ymsg_login( ymsg_sess, tmp );

	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
		_("Logging in to chat server...") );

	show_connected_display();

	/* reset cookies */
	ymsg_sess->cookie[0] = '\0';
}


void start_chat_online() {
	/* set online if we got the right cookie packet */
	immed_rejoin=0;
						if (! using_web_login) {
							gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
							if (login_noroom) {
								/* dont connect to a room */
								gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
								_("Not currently connected to chat.") );
								login_noroom=0;
								ignore_ymsg_online=0;
								in_a_chat=0;
								is_conference=0;
														} else {
								gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
								_("Setting status to Online...") );
								ignore_ymsg_online=0;
								in_a_chat=1;
								is_conference=0;
						 		ymsg_online( ymsg_sess );			
									  							}			
						 	first_userstat=0;

														} else  { /* web login method */
							connect_time = time(NULL);
							ignore_ymsg_online=0;
							in_a_chat=0;
							is_conference=0;
							show_connected_display();
																  }
}

void handle_get_cookie() {

				/*  store new cookie(s) */
				if ( strcmp( ymsg_field( "59" ), "" )) {
					 
					/* Bug fix: Don't reset here since sometimes Yahoo 
					splits our cookie pieces over two or more packets, 
					cookies are reset when we log off and at 'get_key' time */

					/* ymsg_sess->cookie[0] = '\0'; */

					if ( ymsg_sess->cookie[0] ) {
						strcat( ymsg_sess->cookie, ";" );
						strncat( ymsg_sess->cookie, ymsg_field_p( "59" ), 2046-2-strlen(ymsg_sess->cookie));
					} else {
						strncpy( ymsg_sess->cookie, ymsg_field_p( "59" ), 2046);
					}
					packptr = ymsg_sess->cookie;
					packptr = strchr( packptr, '	' ); /* a tab */
					while( packptr ) {
						*packptr = '=';
						packptr = strchr( packptr, 0x04);

						if ( packptr ) {
							*packptr = ';';
							packptr = strchr( packptr, '	' ); /* a tab */
						}
					}
				}

				/* set our buddy list */
				if ( strcmp( ymsg_field( "87" ), "" )) {
					/* Bug Fix: PhrozenSmoke, empty buddy lists 
					contain just one char: '0a' , so dont parse if empty
					to avoid creating an 'empty' name on the buddy list */

					if ( strcmp( ymsg_field( "87" ), "\x0a" )) {
						append_friend_list_spool( ymsg_field( "87" ));
						}

						got_y_buddy_list=1;
						if (got_y_ignore_list) {
							if (!got_buddy_ignore_list) {
								got_buddy_ignore_list=1;
								show_offline_messages();
								flooder_buddy_list_protect();
								start_chat_online();
								}
							}

				}


				/* our own avatar in Messenger-6 mode */
						/* also there is a field 198 - 0 or 1, avatar or no avatar */
				if (strcmp( ymsg_field( "197" ), "" ) ) {
					if (! strncmp( ymsg_field( "198" ), "1", 1 ) ) {
						strncpy( tmp, ymsg_field( "197" ), 256);
						packptr = tmp;
						end = strchr( packptr, ',' );
						if ( end ) {*end = '\0';}
						if (my_avatar) {free(my_avatar);}
						my_avatar=strdup(packptr);
					}
				}

				/* whether we are sharing our own avatar */ 
				if (strcmp( ymsg_field( "213" ), "" ) ) {
					if (share_blist_avatar>1) {share_blist_avatar=1;}
					if (share_blist_avatar<0) {share_blist_avatar=0;}
					if ( atoi(ymsg_field( "213" )) != share_blist_avatar) {
						/* sync yahoo's settings with our own */
						//ymsg_avatar_toggle(ymsg_sess, share_blist_avatar);
					}
				}

				/* names of people on permanent stealth 'offline' mode */
				if (strcmp( ymsg_field( "185" ), "" ) ) {
					strncpy( tmp, ymsg_field( "185" ), 2046);
					packptr = tmp;
					if (strlen(tmp)<2) {packptr=NULL;}
					else {
						snprintf(buf, 2046, "%s : %s\n\n",
							_("You are permanently 'offline' to"), ymsg_field( "185" ) );
						append_to_textbox( chat_window, NULL, buf );
						}
					while( packptr ) {
						end = strchr( packptr, ',' );
						if ( end ) {*end = '\0';}
						if (find_friend(packptr)) {
							FRIEND_OBJECT=create_or_find_yahoo_friend(packptr);
							if (FRIEND_OBJECT) {
								FRIEND_OBJECT->stealth=3;
							}
						}
						if ( end ) {packptr = end + 1;} 
						else {	packptr = NULL;	}
					}
				}

				/* gather profile names */
				if (strcmp( ymsg_field( "3" ), "" ) ) {
						strncpy(tmp2, ymsg_field( "3" ), 80);
						if (strlen(tmp2)>2) {set_primary_screen_name(tmp2);}
				}
				if ( strcmp( ymsg_field( "89" ), "" ) ) {
						strncpy(tmp3, ymsg_field( "89" ), 640);
						if (strlen(tmp3)>2) {parse_profilenames(tmp3);}
						update_existing_profile_name_widgets();
				}

				/* merge yahoo ignore list into local list */
				if ( strcmp( ymsg_field( "88" ), "" )) {
				 if ( strcmp( ymsg_field( "88" ), "\x0a" )) {
						strncpy( tmp, ymsg_field( "88" ), 2046);
						packptr = tmp;
						if (strlen(tmp)<2) {packptr=NULL;}
						while( packptr ) {
							end = strchr( packptr, ',' );
							if ( end ) {*end = '\0';}
							if ( ! ignore_check( packptr )) {ignore_toggle( packptr );}
							if (!perm_igg_check(packptr)) {add_perm_igg(packptr);}
							if ( end ) {packptr = end + 1;} 
							else {	packptr = NULL;	}
						}
				   }
					got_y_ignore_list=1;
					if (got_y_buddy_list) {
						if (!got_buddy_ignore_list) {
							got_buddy_ignore_list=1;
							show_offline_messages();
							flooder_buddy_list_protect();
							start_chat_online();
							}
						}

				}

}  /* end method */ 



void  handle_online() {
	if (!ignore_ymsg_online) {
		is_conference=0;
		if (!got_buddy_ignore_list) {
				got_buddy_ignore_list=1;
				show_offline_messages();
				flooder_buddy_list_protect();
				start_chat_online();
			}
		if (using_web_login || ycht_is_running()) { ycht_join(); return;}
		ymsg_join( ymsg_sess );
								    } else {ignore_ymsg_online=0;}
}


void check_bulk_sms_cb(char *who) {
	if (strcmp(ymsg_field("60") , "")) {
		toggle_sms_pm_session(who, atoi(ymsg_field("60")) );
	}
}

void handle_away_back_cb() {
	int have_camon=0;
	int was_away=0;

				strncpy( tmp2, ymsg_field( "10" ), 64);
				strncpy( tmp3, ymsg_field( "19" ), 256);

			if (! parsing_initial_statuses) {
				if (!find_friend(tmp)) {
					if (ignore_check( tmp )) {return;}
					if ( mute_check( tmp )) {return;}
				}
			}

				/*
				 * for now don't display for room users unless it 
				 * is a friend or ourselves, this arrives in a chat packet
				 * for everybody else
				 */
		if (! parsing_initial_statuses) {
				if ( find_user_row( tmp ) >= 0 ) {
					if ( (strcasecmp( tmp, ymsg_sess->user )) && 
						(! find_profile_name(tmp)) &&  (! find_friend(tmp))  ) {  
							/* not ourselves , or a friend, no buddy list update needed */
							return;
						}
				}
		}

				if ( ymsg_sess->pkt.type == YMSG_BACK ) {strncpy( tmp2, "0", 3 );}

				if (( atoi( tmp2 ) == 99 ) ) {
					/* custom away message in tmp3 */
				} else if (( atoi( tmp2 ) == 999 ))  { /* idle */
					if (locale_status) { strncpy( tmp3, _("Idle"), 50 ); }
					else {strncpy( tmp3, "Idle", 8 );} 

				} else {
					/* stock away message */
					if (locale_status) { strncpy( tmp3, _(away_msgs[atoi(tmp2)]), 72 );  }
					else { strncpy( tmp3, away_msgs[atoi(tmp2)], 72  ); }
				}

					if(strlen(tmp3)>0) {
						char *tttmp;
						tttmp = strchr(tmp3, '\006');
						if(tttmp) {*tttmp = '\0';}
						tttmp = strchr(tmp3, 0xa0);
						if (tttmp)  {
							/* when people turn their webcams on, the custom status message will often 
								contain '\a0\a0' at the end of the string - This is how Y Messenger 
								5 shows webcams are on, but Y Messenger 6-beta sends a field 
								'187' in the status packet set to '1', 187 absent otherwise */
							*tttmp = '\0';
							have_camon=1;
										}

							/* remove stuff that has no business in status messages */ 
							tttmp = strchr(tmp3, '\n');
							while (tttmp) {
								*tttmp=' ';
								tttmp = strchr(tmp3, '\n');
							}
							tttmp = strchr(tmp3, '\t');
							while (tttmp) {
								*tttmp=' ';
								tttmp = strchr(tmp3, '\t');
							}
							tttmp = strchr(tmp3, '\r');
							while (tttmp) {
								*tttmp=' ';
								tttmp = strchr(tmp3, '\r');
							}
												}

	if (! parsing_initial_statuses) {
		if ((! find_friend(tmp)) && (!find_profile_name(tmp)) && 
		(strcasecmp( tmp, ymsg_sess->user )) ) {
			show_status_from_unknown_friend(tmp, "Away / Back"); 
			return ;
		}
	}

	set_buddy_status( tmp , tmp3);
	set_friend_stats(tmp);

	FRIEND_OBJECT=yahoo_friend_find(tmp);
	if (FRIEND_OBJECT) {
		if (!strcmp(ymsg_field("187"), "")) {
			/* only set this if the webcam field is absent from the status packet */
			FRIEND_OBJECT->webcam=have_camon;
		}
		if (parsing_initial_statuses) {check_bulk_sms_cb(tmp); return;}

		if (FRIEND_OBJECT->webcam) {
			strcat(tmp3, " [");
			strncat(tmp3, _("WEBCAM"), 40);
			strcat(tmp3, "]  ");
		}

		if ( FRIEND_OBJECT->inchat ) {
			strcat(tmp3, " [");
			strncat(tmp3, _("Chat"), 25);
			strcat(tmp3, "]  ");
										 		}
		if ( FRIEND_OBJECT->ingames ) {
			strcat(tmp3, " [");
			strncat(tmp3, _("Games"), 25);
			strcat(tmp3, "]  ");
										 		}
		if ( FRIEND_OBJECT->launchcast ) {
			strcat(tmp3, " [");
			strncat(tmp3, "LaunchCAST", 25);
			strcat(tmp3, "]  ");
										 		}
		if ( FRIEND_OBJECT->insms ) {
			strcat(tmp3, " [");
			strncat(tmp3, "SMS", 5);
			strcat(tmp3, "]  ");
				/* Erase the 'invisible' mark in the status message, 
				as SMS users almost always come on directly with a status */ 
			set_buddy_status( tmp , tmp3);
										 		}
	}

	if (parsing_initial_statuses) {check_bulk_sms_cb(tmp); return;}

	/* a userstat packet with a single person's status in it */ 
	if (ymsg_sess->pkt.type == YMSG_USERSTAT) {
		add_online_friend(tmp);
		set_buddy_status( tmp , tmp3); /* erase any 'invisible' markers */ 
		check_bulk_sms_cb(tmp);
		update_buddy_clist();	
		return;
	}

	if (find_friend(tmp))  {

					yalias_name=get_screenname_alias(tmp);
					/* show online messages if they are back from 'invisible' */
					if (!find_online_friend(tmp))  {						
 						display_buddy_online(tmp);
									   	 					 }

						if (ymsg_sess->pkt.type==YMSG_BUDDY_ON) {
								if (!atoi( tmp2 )) {
								snprintf(buf, 4, "%s", "");
								set_buddy_status( tmp , buf);
								}
						}

					snprintf( buf, 200, "  %s%s%s ( %s ) %s%s ",
						YAHOO_STYLE_BOLDON, "\033[#CE998Em", tmp,  yalias_name, 
						YAHOO_COLOR_PURPLE, YAHOO_STYLE_BOLDOFF );						

						update_buddy_clist();	
				} else {
					return;
				}

				if (!show_statuses) {return;}

				if ( atoi( tmp2 )) {
					/* away */
					chatter_list_status( tmp, pixmap_status_away, "AW" );
					strncat(buf,locale_status?_("is away"):"is away", 32 );
					strcat( buf, " ( " );
					strncat( buf, _utf(tmp3) , 640);
					strcat( buf, " )\n" );
					was_away=1;
				} else {
					/* back */
					chatter_list_status( tmp, choose_pixmap(tmp), "" );
					strncat(buf,locale_status?_("is back"):"is back", 40 ); 
					strcat( buf, "\n" );
					if (ymsg_sess->pkt.type==YMSG_BUDDY_ON) {return;}
				}			


				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}

				if (!strchr(buf,'\n'))  {strcat(buf,"\n");}

	  		snprintf(tmp2,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, tmp2 );
	  		append_char_pixmap_text((const char**)pixmap_status_here, NULL);	

				append_to_textbox( chat_window, NULL, buf );	
				append_to_open_pms(tmp, buf,chat_timestamp_pm);

	snprintf(tmp2, 140, "%s: %s %s %s", tmp, was_away?_("is away"):_("is back") ,
	was_away?"-":"", was_away?tmp3:"");
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp2)); 

}

void handle_away_back() {
	if (ycht_is_running() && ycht_only) {
		if ( ! strcmp( ymsg_field( "14" ), "" )) { return ; }
		snprintf(buf, 256, "   ** %s \n", ymsg_field( "14" ));
		append_to_textbox( chat_window, NULL, buf );
		return ;
	}

	/* user status change */
	if ( ! strcmp( ymsg_field( "7" ), "" )) {
					/* this is just a confirmation that we came back */
					return;
	}

	strncpy( tmp, ymsg_field( "7" ), 350);  /* who */

	/* Sometimes yahoo inadvertently exposes a person's hidden profile 
	name in status messages, occassionally sending a field 7 that looks 
	something like:  pushing_off_da_haters\r\x81my_lovings_cool, 
	and we have to parse these special...also take advantage of yahoo's 
	slip up */
	
	if (  (! strstr(tmp, "\r\x81")) && (! strstr(tmp, "\x80")) && (! strstr(tmp, ",")) ) {
		/* normal, one-name field 7 */
		handle_away_back_cb();
		return;
	} else {
		char *ender=NULL;
		char prfnames[384];
		char *partsy=strdup(tmp);
		char *parter=partsy;
		char *partme="\r\x81";
		int laster=0;
		ender=strstr(parter, "\r\x81");
		if (!ender) {ender=strstr(parter, "\x80"); partme="\x80";}
		if (!ender) {ender=strstr(parter, ","); partme=",";}
		sprintf(prfnames,"%s","");
		while (parter) {
			if (ender) {*ender='\0';}
			snprintf( tmp, 80, "%s",parter );  /* who */
			if (strlen(prfnames)<300) {
				strncat(prfnames,tmp, 80);
				strcat(prfnames," ");
			}
			handle_away_back_cb();
			if (laster) {break;}
			ender+=2;
			parter=ender;
			ender=strstr(parter, partme);
			if (!ender) {laster=1;}
		}
		free(partsy);

		if (strlen(prfnames)>1) {
			snprintf(buf, 600, "\n   Linked screen-names (Profile names):  [%s]\n", prfnames); 
			append_to_textbox( chat_window, NULL, buf );		
		}
	}
	
}


void handle_mail() {
	int was_mail_header=0;
	char mail_al[192];
				/* don't print if Yahoo is telling us our mailbox is empty */
				if ( strcmp( ymsg_field( "9" ), "" )) { show_mail_stat(ymsg_field( "9" ));}
				if (ignore_mailstat) {ignore_mailstat=0; return;}
				if ( atoi( ymsg_field( "9" )) == 0 ) {	return;	}

						snprintf(tmp3, 100, "** %s %s %s **", _("You have"), ymsg_field( "9" ), _("email(s)"));	
						gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
						gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 

	  		snprintf(buf,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_mail, NULL);	

				snprintf( buf, 200, "  %s%s** %s  %s  %s  ", YAHOO_COLOR_ORANGE,
						YAHOO_STYLE_BOLDON,  _("You have"), 
					 ymsg_field( "9" ), "\033[#DEAC2Em" );
				strncat(buf,_("email(s)"), 32);
				strcat(buf,".  ");
				strcat(buf, YAHOO_COLOR_PMBLUE);
				strcat(buf, YAHOO_STYLE_UNDERLINEON);
				strcat(buf,"http://mail.yahoo.com");
				strcat(buf, YAHOO_STYLE_UNDERLINEOFF);
				strcat(buf, "   ");
				strcat(buf,"\033[#a3228bm");
				strncat(buf, _("E-mail Preview"), 45);
				strcat(buf, ": ");
				strcat(buf, YAHOO_COLOR_PMBLUE);
				strcat(buf, YAHOO_STYLE_UNDERLINEON);
				strcat(buf, "y-mail://MAIL");
				strcat(buf, YAHOO_STYLE_UNDERLINEOFF);
				strcat(buf,"\033[#DEAC2Em");
				strcat(buf," **\n");

				snprintf(mail_al, 72, "%s %s %s", _("You have"), ymsg_field( "9" ), _("email(s)"));

				/* added: PhrozenSmoke, show sender and subject of any new mail */
				if ( strcmp( ymsg_field( "42" ), "" )) { /* Sender */
					was_mail_header=1;
					snprintf(mail_al, 35, "%s:\n'",_("New mail from"));
					strcat(buf, "\n");
					strcat(buf, "\033[#B5932Fm");
					strcat(buf, YAHOO_STYLE_BOLDON);
					strcat(buf,"  ** ");
					strncat(buf,_("New mail from"), 35);
					strcat(buf,": ");
					strncat(mail_al, _utf(ymsg_field( "42" )), 35);
					strncat(mail_al, "'\n", 5);
					strncat(buf,_utf(ymsg_field( "42" )), 88);

					if ( strcmp( ymsg_field( "43" ), "" )) {  /* email nickname */
						was_mail_header=1;
						strcat(buf," [");
						strncat(buf,_utf(ymsg_field( "43" )), 75);
						strcat(buf,"]  ");
																				 }

					if ( strcmp( ymsg_field( "18" ), "" )) {  /* subject */
						was_mail_header=1;
						strcat(buf, "\033[#B59239m");
						strcat(buf,"   (");
						strncpy(tmp3, ymsg_field( "18" ), 110);
						strncat(buf,_utf(tmp3), 140);
							strncat(mail_al, "(", 2);
							strncpy(tmp3, ymsg_field( "18" ), 50);
							strncat(mail_al, _utf(tmp3), 75);
							strncat(mail_al, ")", 2);
						strcat(buf,")");
																				 }
					strcat(buf," **\n\n");
																			 }

				append_to_textbox( chat_window, NULL, buf );
				trayicon_mail_alert();
				play_sound_event(SOUND_EVENT_MAIL);
				if (popup_new_mail) {show_popup_dialog(_("You have mail."), mail_al,  12);}
				if (was_mail_header) {
						/* Yahoo showed us a mail header, probably with a count of 
							'1', this may be an incorrect total, so update to get the 
							full total to update the icon, and ignore the next dialog */
					ignore_mailstat=1;
					ymsg_refresh(ymsg_sess);
				}
}





void handle_comment() {
	int comment_limit=0;
	int comment_emote=0;

					/* we are in conference, dont show chat room messages */
					if (is_conference) {return;}

				if ( strcmp( ymsg_field( "114" ), "" )) {
					snprintf(buf, 1024, "%s%s  ** %s ** %s%s\n",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED,
						_("Chat Error: You are not currently in a chat room."),
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
					append_to_textbox( chat_window, NULL, buf );
					return;
				}

				/* user comment/emote/thought */
				strncpy( tmp, ymsg_field( "109" ), 100);
				if ( is_flooder( tmp )) {return;}

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nCHAT ROOM COMMENT:\nRoom: %s\nUser: %s\nComment: %s\n\n", ctime(&time_llnow), ymsg_sess->room, tmp, ymsg_field( "117" ));
				fflush( capture_fp );
			}


				/* ignore comments from users not in the chatter list */
				/* disabled - sometimes Yahoo 'drops' people's names from the room -
					even when they are STILL in the room and talking in the room...so: 
					instead of ignoring them, we will simply re-add their name back into 
				    list of chat room members - phrozen smoke */

				/*  if ( find_user_row( tmp ) < 0 ) {return;}  	*/


				/* if a user is talkin but not in the chatter list, add them  */
				if (find_user_row(tmp)<0) {	chatter_list_add(tmp); 	check_for_invisible_friend(tmp);}
				if ( ignore_check( tmp ) || mute_check( tmp )) {return;	}

				yalias_name=get_screenname_alias(tmp);

				comment_limit=850; /* control msg sizes */
				if (enable_basic_protection)  {comment_limit=625;}
				if (limit_lfs) {comment_limit=575;}
				if (find_temporary_friend(tmp) && (comment_limit<675)) {comment_limit=675;}
				if (find_friend(tmp) && (comment_limit<800)) {comment_limit=800;}
				strncpy( tmp2, ymsg_field( "117" ), comment_limit);  

				/* If the person sent an empty line, and they are not a friend, ignore it...
					Some idiot typing nothing to see their name on the screen */ 
				if ( (!strcmp(tmp2, "")) || (!strcmp(tmp2, " ")) )  {
					if ( (! find_temporary_friend(tmp)) && (! find_friend(tmp)) ) {return;}
				}

				if (enable_chat_spam_filter && is_chat_spam(tmp, tmp2))  {
					if ( test_automute(tmp) ) {
							mute_toggle(tmp);
							snprintf( buf, 512, "  Gyach-E - auto-muted %s '%s' ( %s ) %s : appears to be a spam bot [comment].\n", YAHOO_COLOR_OLIVE, tmp, yalias_name, YAHOO_COLOR_BLACK );
							if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
							count = set_last_comment( tmp, "Auto-Muted: Posted Spam" );
							return;
														}
											 										}


				packptr = get_last_comment( tmp );
				if (( first_post_is_url ) &&
					( ! find_friend( tmp )) &&
					(( strstr( tmp2, "http://" )) ||
					 ( strstr( tmp2, "ftp://" )) ||
					 ( strstr( tmp2, " www." ))) &&
					(( ! strstr( tmp2, "Gyach" )) &&
					 ( ! strstr( tmp2, "pyvoicechat" )) &&
					 ( ! strstr( tmp2, "phpaint" ))) &&
					(( packptr == NULL ) ||
					 ( !strcmp( packptr, "Entered Room" )))) {

					if ( test_automute(tmp) )  {
						char tbuf[55]="";
						/* Changed, PhrozenSmoke - lets just mute bots rather 
							than storing them in the list of ignored users...it's useless
							to igg bots that change names all the time */

						mute_toggle( tmp );
						check_for_invisible_friend(tmp);

					strip_html_tags(tmp2);
					control_msg_line_feeds(); 
					snprintf(tbuf,53, "%s ...", tmp2);
					snprintf( buf, 512, "  Gyach-E - auto-muted %s %s ( %s ) %s, first "
						"post was a URL: '%s'\n",
						YAHOO_COLOR_OLIVE, tmp, yalias_name, 
						YAHOO_COLOR_BLACK, _utf(tbuf) );
					if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
					count = set_last_comment( tmp, "Auto-Muted: First Post was URL" );
					return;
														   }
				}


	/* The person came in and the first thing they posted
	was an emote that was not a 'status' emote, this person
	is not a friend and spam protection is on...so mute them,
	alot of bots now come in and start sending emotes over 
	and over  */

	comment_emote=0;
	if ( enable_chat_spam_filter && strcmp( ymsg_field( "124"), "1" ) 
		&& (! find_temporary_friend(tmp)) && (! find_friend(tmp)) ) {
			comment_emote=1;
	}

	if (  comment_emote && (! strstr(tmp2, "is back"))  && (! strstr(tmp2, "is away"))  && 
		( ( packptr == NULL ) ||  ( !strcmp( packptr, "Entered Room" )) ) 		) {
			if ( test_automute(tmp) ) {
				strip_html_tags(tmp2);
				control_msg_line_feeds(); 
				snprintf(tmp3, 45, "%s", tmp2);
				mute_toggle(tmp);
				snprintf( buf, 500, "  Gyach-E - auto-muted %s '%s' ( %s ) %s : appears to be a spam bot  [First post was an Emote: '%s'].\n", YAHOO_COLOR_OLIVE, tmp, yalias_name,  YAHOO_COLOR_BLACK, tmp3 );
				if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
				count = set_last_comment( tmp, "Auto-Muted: Possible spam bot - first post was Emote" );
				return;
							}
	}

				if ((( packptr = auto_ignore_check( tmp2 )) != NULL ) &&
					( ! find_friend( tmp )) && ( ! find_temporary_friend( tmp )) ) {
					if ( ! ignore_check( tmp )) {
						ignore_toggle( tmp );
						check_for_invisible_friend(tmp);
					} else {
						return;
					}

					snprintf( buf, 512, "  Gyach-E - auto-ignored %s%s%s, regex '%s'\n",
						YAHOO_COLOR_RED, tmp, YAHOO_COLOR_BLACK, packptr );
					if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf ); }
					count = set_last_comment( tmp, "Auto-Ignored: Regex Match" );
					return; 
				} 

				/* keep track of last thing the user said */
				strncpy(tmp3, tmp2,400);
				if (comment_emote) {strncpy(tmp3, "[~ Sent Chat Emote ~]",70);}
				count = set_last_comment( tmp, tmp3 );

				if (comment_emote && (count>2)) {
					if ( test_automute(tmp) ) {
						mute_toggle(tmp);
						snprintf( buf, 500, "  Gyach-E - auto-muted %s '%s' ( %s ) %s : appears to be a spam bot  [repeated posting of Emotes].\n", YAHOO_COLOR_OLIVE, tmp, yalias_name,  YAHOO_COLOR_BLACK);
						if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
						count = set_last_comment( tmp, "Auto-Muted: Possible spam bot - repeated posting of Emotes" );
						return;
							}
				}

				if (( ignore_on_mults ) &&
					( count > ignore_on_mults ) &&
					( ! find_friend( tmp )) && (!find_temporary_friend(tmp)) && 
					( ! ignore_check( tmp ))) {
					ignore_toggle( tmp );
					strncpy( tmp2, ymsg_field( "117" ), 275);  
					strip_html_tags(tmp2);
					control_msg_line_feeds(); 
					snprintf( buf, 512, 
						"  Gyach-E - auto-ignored %s %s ( %s ) %s, same post "
						"%d times: \"%s\"\n", YAHOO_COLOR_RED, tmp, yalias_name, 
						YAHOO_COLOR_BLACK, count, _utf(tmp2) );
					if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
				} else if (( mute_on_mults ) &&
					( count > mute_on_mults ) && 
					 test_automute(tmp) ) {
					mute_toggle( tmp );
					strncpy( tmp2, ymsg_field( "117" ), 275);  
					strip_html_tags(tmp2);
					control_msg_line_feeds(); 
					snprintf( buf, 600, 
							"  Gyach-E - auto-muted %s %s ( %s ) %s, same post "
							"%d times: \"%s\"\n", YAHOO_COLOR_OLIVE, tmp, yalias_name, 
							YAHOO_COLOR_BLACK, count, _utf(tmp2) );
							if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
				}
				if (( suppress_mult ) &&
					( count > 2 )) {
						if ( ( (!find_friend(tmp)) && (!find_temporary_friend(tmp)) ) ||  
						 (!never_ignore_a_friend) )  {return; }
				}

				/* need to make fetch_avatar windows-safe */
#ifndef OS_WINDOWS
				if ( show_avatars ) {
					/* check for an avatar and update if needed */
					if (( packptr = strstr( tmp2, AVATAR_START )) != NULL ) {
						/* avatar exists so try to display if unset */
						packptr += strlen( AVATAR_START );
						end = strstr( packptr, AVATAR_END );
						if ( ! end ) {
							end = strchr( packptr, '>' );
						}
						if ( end ) {
							*end = '\0';
							display_avatar( tmp, packptr );
							snprintf(buf, 4, "%s", " ");
							append_to_textbox_color( chat_window, NULL, buf );
						}
					}
				}
#endif

				strncpy( tmp2, ymsg_field( "117" ), comment_limit);  /* control msg sizes */
				strcat(tmp2," "); /* forces showing of smileys */
				control_msg_line_feeds();

				if ( chat_timestamp ) {append_timestamp(chat_window, NULL);}

				if ( strcasecmp( tmp, ymsg_sess->user )) { /* other users' comments */
					snprintf(buf, 256, "%s%s%s%s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_temporary_friend(tmp) || find_friend(tmp) ) )
							? "\033[#5CC3E9m" : YAHOO_COLOR_PURPLE,
						yalias_name, YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF ); 
						 /* changed: PhrozenSmoke */
						
						check_for_invisible_friend(tmp);

				} else {
					/* this never really happens, we don't recv our comments */
					snprintf(buf, 256, "%s%s%s%s%s",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_BLUE, tmp,
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
				}

				if ( ! strcmp( ymsg_field( "124"), "1" ))
					strcat( buf, ": " );
				else
					strcat( buf, " " );

				append_to_textbox_color( chat_window, NULL, buf );

				if ( ! strcmp( ymsg_field( "124"), "1" )) {
					/* setting total byte limit to 650, to avoid buffer 
						overflow and dangerous 'flooding'
						from people sending large paragraphs - 
						this is more than enough space for 
						standard comments, sometimes flooders 
						have tried to send up to 800 bytes, so we'll limit the likelihood
						that they can lag our screens or boot/crash us. 		
		 			*/
					snprintf( buf, 950,  "%s%s%s\n",
						YAHOO_COLOR_BLACK,_utf( tmp2), ATTR_RESET );
					real_show_colors = show_colors;
					append_to_textbox( chat_window, NULL, buf );
					real_show_colors = 1;
				} else {
						/* emote */
					snprintf( buf, 950, "%s%s%s\n",
						( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#a484b6m" : "\033[#6d8dc0m", _utf(tmp2), ATTR_RESET );

					/* check for status change */
					if ( ! strncasecmp( tmp2, "is away ", 8 )) {
						chatter_list_status( tmp, pixmap_status_away, "AW" );
					} else if ( ! strncasecmp( tmp2, "is back", 7 )) {
						chatter_list_status( tmp, choose_pixmap(tmp), "" );
					}
					append_to_textbox( chat_window, NULL, buf );
				}

}


void handle_exit() {
	if (is_conference) {return;}
	strncpy( tmp, ymsg_field( "109" ), 100);
	if (! strcmp(tmp, "")) {return;}

				if ( strcasecmp( ymsg_sess->user, tmp) && (! find_profile_name(tmp))) {
					/* we are in conference, dont show chat room messages */

					/* someone else left */					

					if (( ! ignore_check( tmp )) &&
						( ! mute_check( tmp )) &&
						( show_enters )) {

				if ( enter_leave_timestamp && chat_timestamp) {
					append_timestamp(chat_window, NULL);
																					}

						yalias_name=get_screenname_alias(tmp);

				if (! strcmp(tmp,yalias_name)) {
						snprintf(buf, 256, "  %s%s  %s",
							( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#B558C8m" :  "\033[#5D5CBBm" ,
							tmp,
							( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#6098C8m" : "\033[#00B8C8m" 							
								 );
															} else {
						snprintf(buf, 256, "  %s%s  [ %s ] %s",
							( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#B558C8m" : "\033[#5D5CBBm" ,
							yalias_name,
							tmp,
							( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#6098C8m" : "\033[#00B8C8m" 			
								);
																	  }

						strncat(buf,_("has left"), 48);
						strcat(buf,YAHOO_COLOR_BLACK);
						strcat(buf, " ");
						strncat(buf,_("the room"), 48);
						strcat(buf, ". \n");

						if (find_user_row(tmp)>=0) {
							append_to_textbox( chat_window, NULL, buf );
							append_to_open_pms(tmp, buf,chat_timestamp_pm);
						}
													}

						check_for_invisible_friend(tmp);
						chatter_list_remove( tmp ); 
							/* changed: PhrozenSmoke */
							/* call chatter_list_remove, AFTER printing 'has left' message to 
								 avoid deleting the alias in screename_alias hash */
						} else {
						/* packet that shows up when I leave */
						chatter_list_remove( tmp ); 
						in_a_chat=0;
								  }

}


void handle_join() {
				if ( strcmp( ymsg_field( "114" ), "" )) {
					/* This happens when Yahoo tells us that the room is full */
					/* added: PhrozenSmoke */
					if (ymsg_sess->pkt.size<20) {
						snprintf(buf, 512, "  %s%s** %s: '%s' **%s%s\n",
						"\033[#BD8B71m", YAHOO_STYLE_ITALICON,
						_("The chat room is full. Please try again later."), ymsg_sess->req_room,
						YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF
							);
						append_to_textbox( chat_window, NULL, buf );
						if (!in_a_chat) {
							set_chat_profile_name( get_current_chat_profile_name());
							ymsg_logout(ymsg_sess);
							set_chat_profile_name( get_default_profile_name());
						}
						set_current_chat_profile_name( get_default_profile_name());
						return;
										  }			

											}

				/* fixme, need to verify what exactly constitutes a join pkt */
				strncpy( tmp, ymsg_field( "126" ), 255);		/* dunno what, some serial # */
				strncpy( tmp2, ymsg_field( "104" ), 255);	/* room name */
				if (( strcmp( tmp, "" )) ||
					( ! strcmp( tmp2, "" ))) {
					/* I just joined the room, so get userlist */
					if ( strcmp( ymsg_field( "104" ), "" )) {
							
						is_conference=0;						
						ymsg_conference_leave(ymsg_sess);	
						jump_to_chat_tab();
						in_a_chat=1;
						set_chat_profile_name(get_current_chat_profile_name());
						set_current_chat_profile_name( get_default_profile_name());

						gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
						snprintf( buf, 300, "%s: %s", _("Current Room"), ymsg_field( "104" ));
						gtk_statusbar_push( GTK_STATUSBAR(chat_status),
							st_cid, buf );
						strncpy( ymsg_sess->room, ymsg_field( "104" ), 62);

						if (last_chat_room_used) {free(last_chat_room_used); last_chat_room_used=NULL; }
						last_chat_room_used=strdup(ymsg_field( "104" ));

	  				snprintf(buf,6, "%s", "\n   ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_join , NULL);	

						snprintf( buf, 225, "\n   %s%s:  %s%s%s%s%s\n", YAHOO_COLOR_BLUE,
							_("You are now in") ,"\033[#A07EBDm", 
							YAHOO_STYLE_BOLDON,
							ymsg_field( "104" ), YAHOO_STYLE_BOLDOFF, ATTR_RESET );
						if ( strcmp( ymsg_field( "105" ), "" )) {  /* room topic */
							strcat( buf, "\033[#7C9EC3m" );
							strcat( buf, "   '" );
							strncpy(tmp2, ymsg_field( "105" ) , 130);
							strip_html_tags(tmp2);
							strncat( buf, tmp2, 130);
							strcat( buf, "'" );
							strcat( buf, ATTR_RESET );
							strcat( buf, "\n" );
						}
						/* now, we need to collect some info needed for voice chat - 
							 PhrozenSmoke:   field 130 = chat room cookie, 128 & 129 = 
							 room 'serial number', 61 = some other cookie (use unknown), 
							 also useful...108 = number of people in the room, 
							 104 = room name - the 'chat room' cookie changes with 
							 each room and is different from the 'session' cookie we get 
							 when we log on.   If a room was opened because it didnt 
							 exist, but the user didn't explicitly 'make' the room, then 
						     the chat serial # (129) will be '0', and there will be no 
							 chat room cookie (130)...if, somebody deliberately made a 
							 room with voice chat DISABLED, there will be a chat serial 
							 (129), but no chat room cookie (130). - I THINK these are 
							 the reasons, but either way - we CANNOT enter voice 
							 chat without both fields 128/129 AND 130...and 128/129 
							 must not = 0...field 128 is the room 'category' not the room
							 itself  the actual room serial is in 129, sometimes 128 and 129 
							 are the same (like user-created rooms), but 129 is the field 
							 we need. */

							   is_conference=0;

							   strncpy(current_chat_cookie,"", 118);  /* clear old */
							   strncpy(current_chat_serial,"" ,48);  /* clear old */
							   strncpy(current_chat_name,ymsg_field( "104" ), 73);
							   strncpy(current_chat_user,get_chat_profile_name(), 73);
							   voice_enabled=0;
								if ( strcmp( ymsg_field( "130" ), "" )) {
							   		strncpy(current_chat_cookie,ymsg_field( "130" ), 118);			
									/* printf("Chat Cookie: %s \n", current_chat_cookie);
									fflush(stdout);  */
																								}
								if ( strcmp( ymsg_field( "129" ), "" )) {
									strncpy(current_chat_serial,ymsg_field( "129" ), 48);
									/* printf("Chat Serial: %s \n", current_chat_serial);
									fflush(stdout); */
																								}			
								if ( (strlen(current_chat_cookie)>3) && 										(strlen(current_chat_serial)>3) ) {
														 voice_enabled=1; }

								/* printf("VOICE CHAT:  %d\n",voice_enabled);
								fflush(stdout);  */

								/* report if chat is enabled */
								strcat(buf,"\n");
								strcat(buf,YAHOO_STYLE_BOLDON);
								strcat(buf,YAHOO_COLOR_PURPLE);
								strncat(buf,_("Voice Chat"), 25);
								strcat(buf,":  ");
								  if (voice_enabled==1) { 
										strcat(buf,YAHOO_COLOR_GREEN); 
										strncat(buf, _("Enabled"), 18);
										strcat(buf, "  ");
										append_char_pixmap_text((const char**)pixmap_pm_voice, NULL);
																  }  else {
										strcat(buf,YAHOO_COLOR_RED); 
										strncat(buf,_("Disabled"), 18);
																			}
								strcat(buf,YAHOO_COLOR_BLACK);
								strcat(buf,YAHOO_STYLE_BOLDON);
								strcat(buf,"\n");

								voice_launch(0);
					
					}

					strncpy( tmp, ymsg_field( "109" ),1300);
					packsrc = tmp;
					packptr = tmp2;
					while( *packsrc ) {
						*packptr = *(packsrc++);
						if ( *packptr == ',' ) {
							packptr++;
							*packptr = ' ';
						}
						packptr++;
					}
					*packptr = '\0';

					if ( strcmp( ymsg_field( "104" ), "" )) {
						strcat(buf,"\n");
						strcat(buf,YAHOO_COLOR_BLUE);
						strncat( buf, _("Members in this room"), 32 );
						strcat(buf,"  ");
					} else {
						if ( tmp[0] == '\0' ) {	return; }
						snprintf(buf, 100, "%s%s<font size=\"10\">%s:  ", 
								ATTR_RESET, 
								YAHOO_COLOR_BLACK,
							 _("Members in this room") );
							  }

					if ( strcmp( ymsg_field( "108" ), "" )) {  /* number of chatters */
						strcat(buf," [ ");
						strncat(buf,ymsg_field( "108" ),5);
						strcat(buf," ] ");					
						strcat(buf,":\n");
																		  }
					
					strcat(buf,YAHOO_COLOR_BLACK);
					strcat(buf,"<font size=\"10\">");
					strncat( buf, tmp2, 1302 );



					strcat( buf, "\n\n" );
					append_to_textbox( chat_window, NULL, buf );

					if ( strcmp( ymsg_field( "104" ), "" )) {
						/* added: PhrozenSmoke */
						parse_screenname_aliases(ymsg_sess->pkt.data, 1); 
						chatter_list_populate( ymsg_field( "109" ), 1 ); 

						/* This is a bug fix, PhrozenSmoke: sometimes Yahoo
						doesn't have our name in the user list when we join a room
						and doesn't send a 'join' packet with our name, causing 
						our name not to show up in the chat room list sometimes...let's make sure our
						name is always in the chat room list */

							if (find_user_row(get_chat_profile_name())<0) {	
								chatter_list_add(get_chat_profile_name()); 	
								check_for_invisible_friend(get_chat_profile_name());
																					}	

					} else {
						parse_screenname_aliases(ymsg_sess->pkt.data, 0); 
						chatter_list_populate( ymsg_field( "109" ), 0 );
					}

						 /* response code 108 = number of members in room */
				} else if ( ! strcmp( ymsg_field( "108" ), "1" ) ) {

					int postit=0;

					/* we are in conference, dont show chat room messages */
					if (is_conference) {return;}

					/* someone else joined the room while I'm in there */
					strncpy( tmp, ymsg_field( "109" ), 1000);

					check_for_invisible_friend(tmp);


					/* ignore guest users, check field 112, which we only receive on single
						user joins for some odd reason, most 'guest' users are spam bots */
	
						if (ignore_guests || enable_chat_spam_filter)  {
							if ( strcmp( ymsg_field( "112" ), "" )) {
								if ( strcmp( ymsg_field( "112" ), "0" )) {


								/* When the person is a 'guest' user
									the field '112' will be something other 
									than '0' - normally something like 
									'2230060' or '2457292'. In my experience
									almost ALL these 'guest' users are spam bots.
								*/
				
				// printf("112 field  [%s]:  %s\n", tmp, ymsg_field( "112" )); fflush(stdout);	
				
									if (test_automute(tmp)) {
											mute_toggle(tmp);
											snprintf(buf, 512, "Gyach-E auto-muted guest user %s '%s' %s %s\n",
											YAHOO_COLOR_OLIVE, tmp, YAHOO_COLOR_BLACK,
											"[possible spam bot]" );
											append_to_textbox( chat_window, NULL, buf );
															  			}
																						   }
																				  }
																									}


						/* added by PhrozenSmoke */
						if ( strcmp( ymsg_field( "141" ), "" )) {
							/* new room member has alias, save it */
							/* update the alias */ /* in users.c  screename_alias hash */
							update_screenname_alias( tmp, ymsg_field( "141" ) );
																				}

					chatter_list_add( tmp );  /* spam bots names caught here */

					/* keep track of user enters for auto-ignore/mute */
					count = set_last_comment( tmp, "Entered Room" );
					if (( ignore_on_mults ) && (count>2) &&  
						( count > ignore_on_mults ) &&
						( ! ignore_check( tmp ))) {

					if ( ( (!find_friend(tmp)) && (!find_temporary_friend(tmp)) ) ||  
					(!never_ignore_a_friend) )  {
						if (yalias_name) {g_free(yalias_name);}
						yalias_name=get_screenname_alias(tmp);
						ignore_toggle( tmp );
						snprintf( buf, 512, 
							"  Gyach-E - auto-ignored %s%s ( %s )%s, entered/left "
							"room %d times without commenting\n",
							YAHOO_COLOR_RED, tmp,yalias_name,
							YAHOO_COLOR_BLACK, count );
						append_to_textbox( chat_window, NULL, buf );
														     }
					} else if (( mute_on_mults ) &&
						( count > mute_on_mults ) && 
						test_automute(tmp) ) {

						if (yalias_name) {g_free(yalias_name);}
						yalias_name=get_screenname_alias(tmp);
						mute_toggle( tmp );
						snprintf( buf, 600, 
								"  Gyach-E - auto-muted %s %s ( %s ) %s, entered/left "
								"room %d times without commenting\n",
								YAHOO_COLOR_OLIVE, tmp, yalias_name,
								YAHOO_COLOR_BLACK, count );
								append_to_textbox( chat_window, NULL, buf );

					}


					if ( show_enters && ( ! ignore_check( tmp )) && ( ! mute_check( tmp )) ) { postit=1; }
					if (find_profile_name(tmp)) {postit=0;}
					if (!strcasecmp( tmp, ymsg_sess->user )) {postit=0;}

						/* added: PhrozenSmoke, code to show 'alias' or 'nickname' for a person */
						/* code: 141 = alias  */
						/* Right now, we can only show aliases for people that join the room
							AFTER we have come into the room...as well as people already IN the room */

				if ( enter_leave_timestamp && chat_timestamp && postit) {
					append_timestamp(chat_window, NULL);
																								  }


						if ( strcmp( ymsg_field( "141" ), "" ) && strcmp( ymsg_field( "141" ), tmp ) ) {
							/* new room member has unique alias, show it */
						if (yalias_name) {g_free(yalias_name);}
						yalias_name=get_screenname_alias(tmp);
						snprintf(buf, 512, "  %s%s%s [ %s ] %s",YAHOO_STYLE_BOLDON,  	
						( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#7CC0DEm" : YAHOO_COLOR_BLUE ,
						yalias_name, tmp, 
						( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#B996DEm" : YAHOO_COLOR_RED
							);

							/* to avoid Gtk 'utf-8' warnings on Gtk-2  */
							strncat(buf,	_("has entered"), 32);	
							strcat(buf,YAHOO_COLOR_BLACK);	
							strcat(buf," ");	
							strncat(buf,_("the room"), 32);	
							strcat(buf,YAHOO_STYLE_BOLDOFF);	
							strcat(buf,". \n");	
																					   } else  {  /* no alias for user */

						  snprintf(buf, 512, "  %s%s%s %s",
							YAHOO_STYLE_BOLDON,
						( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#7CC0DEm" : YAHOO_COLOR_BLUE ,
							tmp,
						( highlight_friends && (find_friend(tmp) || find_temporary_friend(tmp)) )
							? "\033[#B996DEm" : YAHOO_COLOR_RED
								  );  		
	
							/* to avoid Gtk 'utf-8' warnings on Gtk-2  */
							strncat(buf,_("has entered"), 32);	
							strcat(buf,YAHOO_COLOR_BLACK);	
							strcat(buf," ");	
							strncat(buf,_("the room"), 32);	
							strcat(buf,YAHOO_STYLE_BOLDOFF);	
							strcat(buf,". \n");	
																									}

						if (postit) {
							append_to_textbox( chat_window, NULL, buf ); 
							append_to_open_pms(tmp, buf,chat_timestamp_pm);
						}

					    /* added: PhrozenSmoke, show useful info: age/sex/loc if available */
						/* response code: 110=member's age */
						/* response code: 142=member's location */
						/* response code: 113=member: male/female/unknown + webcam */

						if ( strcmp( ymsg_field( "110" ), "" )  || strcmp( ymsg_field( "142" ), "" )  || strcmp( ymsg_field( "113" ), "" )) {  /* extra info avail. for at least one */
							int has_cam=0;
							int is_fem=0;
							int is_man=0;
							char gender[32]="";
							char age[24]="";
							char webcam[32]="";
							gchar loc[72]="";
							gchar short_profile[168];

							if (yalias_name) {g_free(yalias_name);}
							yalias_name=get_screenname_alias(tmp);

							if ( strcmp( ymsg_field( "110" ), "" ))  {  /* got age */
							  if (!strncmp( ymsg_field( "110" ), "0",1 )) 
								{ 
								snprintf(age,22, "%s", _("Age: ?, "));
								if (mute_noage && test_automute(tmp)) {mute_toggle(tmp);}
								} else {

								snprintf(age,22, "%s%s, ", _("Age: "), _utf(ymsg_field( "110" )) );

									if (atoi(ymsg_field( "110" ))>90) { /* fake age */
								if (mute_noage && test_automute(tmp)) {mute_toggle(tmp);}
																					 }
									if (atoi(ymsg_field( "110" ))<18) { /* minor */
								if (mute_minors && test_automute(tmp)) {mute_toggle(tmp);}
																					 }

									 }
														} else {
								snprintf(age,22, "%s", _("Age: ?, "));
								if (mute_noage && test_automute(tmp)) {mute_toggle(tmp);}
														}

							if ( strcmp( ymsg_field( "142" ), "" ))  {  /* got location */
								snprintf(loc,70, "%s%s, ", _("Location: "), _utf(ymsg_field( "142" )) );
														} else {
								snprintf(loc,70, "%s", _("Location: ?, "));
															 }

							if ( strcmp( ymsg_field( "113" ), "" ))  {  /* got attribs m/f */
								/* so far have gotten the following numbers back for Attribs: 
										33792 = male??
										1024 = unknown gender ??
										1040 = unknown with cam
										66560 = female ???
										66576 = female with cam
										33808 = male with cam?
								*/
								if ( strstr(ymsg_field( "113" ), "33") != NULL) {
									is_man=1;
									snprintf(gender,30, "%s, ", _("Sex: Male"));
									if (mute_males && test_automute(tmp)) {mute_toggle(tmp);}
									if ( strstr(ymsg_field( "113" ), "33808") != NULL) 
									{
									snprintf(webcam,30, "%s", _("Cam: Yes"));	
									has_cam=1;									
									} else {snprintf(webcam,30, "%s", _("Cam: No"));}
										  																    }
								if ( strstr(ymsg_field( "113" ), "66") != NULL) {
									is_fem=1;
									if (mute_females && test_automute(tmp)) {mute_toggle(tmp);}
									snprintf(gender,30, "%s, ", _("Sex: Female"));	
									if ( strstr(ymsg_field( "113" ), "66576") != NULL) 
									{
									snprintf(webcam,30, "%s", _("Cam: Yes"));	
									has_cam=1;									
									} else {snprintf(webcam,30, "%s", _("Cam: No"));}
																		  	  																	}
								if ( strstr(ymsg_field( "113" ), "10") != NULL) {
										if (mute_nogender && test_automute(tmp)) {mute_toggle(tmp);}
								snprintf(gender,30, "%s, ", _("Sex: ?"));
									if ( strstr(ymsg_field( "113" ), "1040") != NULL) 
									{
									snprintf(webcam,30, "%s", _("Cam: Yes"));	
									has_cam=1;
									} else {snprintf(webcam,30, "%s", _("Cam: No"));}
										  																    }

								/* strcat( buf, "Attribs: ");  
								strncat( buf, ymsg_field( "113" ), 32);  */
															 } else {
										if (mute_nogender && test_automute(tmp)) {mute_toggle(tmp);}
								snprintf(gender,30, "%s", _("Sex: ?"));
								snprintf(webcam,30, "%s", _("Cam: ?"));
																   }

							/* print a short profile */
							if (postit && (is_man  || is_fem || has_cam)) {
								snprintf(buf, 8, "%s",  "  ");
								append_to_textbox( chat_window, NULL, buf ); 
								if (is_man) {append_char_pixmap_text((const char**)pixmap_status_here_male, NULL);}
								if (is_fem) {append_char_pixmap_text((const char**)pixmap_status_here_female, NULL);}
								if (has_cam) {append_char_pixmap_text((const char**)pixmap_webcam, NULL);}
							}

							snprintf( buf, 420,  "  <font size=\"10\">%s%s:  (%s%s%s%s)\n",  
								YAHOO_COLOR_BLACK, yalias_name,
								age , loc, gender , webcam );
							if (postit) { append_to_textbox( chat_window, NULL, buf ); }

							snprintf(short_profile, 166, 
								"%s%s%s%s", age , loc, gender, webcam
							);
							set_quick_profile(tmp,short_profile);
							if ( (!mute_check(tmp)) && 
								(!ignore_check(tmp))	) {
							chatter_list_status( tmp, choose_pixmap(tmp), "" );
																   }
														} /* end if age/sec/loc  */						


				}    /* end  case YMSG_JOIN */

}


/* The windows version of Yahoo Messenger has a feature
    that allows searching the web by typing 's: [search term]' in 
    the PM window...the first match found on Yahoo's search 
    engine is then displayed in the PM window.  I think that 
    feature is cool except for 3 problems: Yahoo's search engine
    sucks, the search result data is returned in an overly complicated
    XML format, and there is no way to control the number of 
    search results returned.  Gyach-E will implement this feature 
    by using GOOGLE (a better search engine), forcing ONE 
    search result, and display it on the 'My Yahoo' tab.  */

void handle_y_search_results(GtkWidget *window, GtkWidget *textbox, char *intext) {
	char search_term[32];
	char search_url[256];
	char *starter=NULL;
	GtkWidget *tmp_widget=NULL;

	if (!intext) {return;}
	if ( strncmp(intext, "s: ", 3)) {return;}
	starter=strstr(intext, "s: ");
	if (!starter) {return;}
	snprintf(search_term, 30, "%s", starter+3);
	if (strlen(search_term)<2) {return;}
	snprintf(search_url, 254, "http://www.google.com/search?as_q=&num=1&hl=en&ie=UTF-8&btnG=Google+Search&as_epq=%s&as_oq=&as_eq=&lr=&as_ft=i&as_filetype=&as_qdr=all&as_nlo=&as_nhi=&as_occt=any&as_dt=i&as_sitesearch=&safe=", gyach_url_encode(search_term)		);


	append_timestamp(window, textbox);
	snprintf(buf, 512, "   %s** %s: %s'%s'%s **%s\n", 
		YAHOO_COLOR_OLIVE, _("Web Search"), 
		YAHOO_COLOR_PMBLUE, search_term, 
		YAHOO_COLOR_OLIVE, YAHOO_COLOR_BLACK);
	append_to_textbox( window, textbox, buf );

	load_myyahoo_URL(search_url);

	tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
	if (tmp_widget) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
	}
	gdk_window_raise(chat_window->window );
}



void probe_for_bimage(char *who) {
	int got_bimage=0;
	if ( (ymsg_sess->pkt.type != YMSG_PM) && 
		(ymsg_sess->pkt.type != YMSG_BIMAGE_TOGGLE) ) {return;}

	if (! emulate_ymsg6) {return;}
	if (! show_bimages) {return;}

	/* request buddy image if available */		
		if (strcmp(ymsg_field("206") , "")) {got_bimage=atoi(ymsg_field("206") );}

		if ((got_bimage==1)  && ( pm_sess->buddy_image_file ) )  {
			if (! strstr(pm_sess->buddy_image_file, "/yavatars/")) {
				/* we got some kind of buddy image loaded that 
					aint the avatar */ 
				free(pm_sess->buddy_image_file);
				pm_sess->buddy_image_file=NULL;
				update_my_buddy_image(pm_sess, 0);
			}
		}				


		if (got_bimage && (! pm_sess->buddy_image_file ) )  {

			if (got_bimage==1) { /* avatar */
				/* if it's an avatar we already know about, we can load it ourselves */
				/* I think Yahoo expects this anyways */
				FRIEND_OBJECT=yahoo_friend_find(who);
				if (FRIEND_OBJECT) {

					if (! FRIEND_OBJECT->buddy_image_hash) {
						FRIEND_OBJECT->buddy_image_hash=g_strdup("[BAVATAR]");
					} else {
						if (strcmp(FRIEND_OBJECT->buddy_image_hash, "[BAVATAR]")) {
							/* set a fake internal checksum so we know next 
							time we open a PM box for this user */ 
							g_free(FRIEND_OBJECT->buddy_image_hash);
							FRIEND_OBJECT->buddy_image_hash=g_strdup("[BAVATAR]");
						}
					}

					if (FRIEND_OBJECT->avatar) {
						if ( download_yavatar(FRIEND_OBJECT->avatar, 2, NULL)) {
							char icmd[256]="";
							snprintf( icmd, 254, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, FRIEND_OBJECT->avatar, avatar_filetype);
							pm_sess->buddy_image_file=strdup(icmd);
							update_my_buddy_image(pm_sess, 0);
							return;
						}
					}

				}
			}

			ymsg_bimage_accept(ymsg_sess, who , 1);  /* try to request it */
		}

}



void handle_pm() {
	int is_real_system_msg=0;
	int is_pm_spam=0;
	char pmbuf[88]="";

	/* Check for error messages, such as 'user's message box is full */
	/* Error messages usually come with no name in field 4 */
	if ( strcmp( ymsg_field( "16" ), "" ))  {
			strncpy(tmp3, ymsg_field( "16" ), 350);
			if (strstr(tmp3, "The file was not")) {  /* file sending error occurred */ 
				show_ok_dialog(tmp3);
			}
			snprintf(buf, 512, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Private Message Error"),
			tmp3, YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			return ;
													  }

				   if ((! got_buddy_ignore_list) && (! ycht_only) ) {  /* handle offline messages */
					if (ymsg_sess->pkt.type != YMSG_SYSMSG)  { /* let sys messages through */
						parse_offline_messages(ymsg_sess->pkt.data);
						return;
																	  }
									   					 }

	/* we get other 'PM' packets that have no name 
	but have some status - and have the person's name in the 
	field "5" ,  these are usually status packets indicating the 
	person is either away or idle */

	if (strcmp(ymsg_field("10"), "") && strcmp(ymsg_field("19"), "") &&  
	  strcmp(ymsg_field("5"), "") && 
	  (! strcmp(ymsg_field("4"), "")) && (! strcmp(ymsg_field("1"), "")) ) {
			strncpy( tmp, ymsg_field( "5" ), 80);
			/* possible spoofs: Messages from ourselves, so ignore */
			if (! from_valid_sender(tmp) ) {return;}
			if (is_flooder(tmp)) {  return;}

			if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
				pm_sess = pm_lpackptr->data;

		if ( ! strncmp(ymsg_field("10"), "999", 3) ) {
				/*  A PM we sent might not be seen because the person is 'idel' */
				snprintf(buf, 300, "   %s** %s ** %s\n", YAHOO_COLOR_RED, 
					_("The user is currently IDLE and might not receive your message."),
					YAHOO_COLOR_BLACK);
				append_to_textbox_color( pm_sess->pm_window,
							pm_sess->pm_text, buf );
			}

		if ( (! strncmp(ymsg_field("10"), "99", 2))  && (! strncmp(ymsg_field("19"), "-", 1)) ) {
			/*  A PM we sent might not be seen because the person is 'away' */
				snprintf(buf, 300, "   %s** %s ** %s\n", YAHOO_COLOR_RED, 
					_("The user is currently AWAY and might not receive your message."),
					YAHOO_COLOR_BLACK);
				append_to_textbox_color( pm_sess->pm_window,
							pm_sess->pm_text, buf );
			}
		}	
		return ;
	}  /* end PM error messages */



	/* Field 4 = screenname, Field 1 = nice display name */
	/* We'll try display name first, fallback to plain lowercase screenname */ 
	if (! strcmp( ymsg_field( "1" ), "" ))  {  
		if (! strcmp( ymsg_field( "4" ), "" ))  { return ; }
													 }  /* no name in fields 1 or 4 */

	if (! strcmp( ymsg_field( "1" ), "" ))  {  
				strncpy( tmp, ymsg_field( "4" ), 2046);  /* from who */
													 } else {strncpy( tmp, ymsg_field( "1" ), 2046);}

				/* first handle possible offline messages properly, which sometimes 
				   contain several PMs in a single packet which produces a 
				   comma-separated list of 'from' names that need to be specially 
				   processed to avoid bogus ignore triggers and missing offline msgs
				   Occassionally, offline PMs come in before we have our friends/ignore 
				   list, so we'll store them until we get the list  */

				   if (strstr(tmp,",") || (strcmp(ymsg_field( "31" ), ""))  ) {
						parse_offline_messages(ymsg_sess->pkt.data);
						show_offline_messages();
						return;
							     }

					if ( capture_fp) {	
						char *pmstype="INSTANT MESSAGE";
						if (ymsg_sess->pkt.type == YMSG_SYSMSG) {
							pmstype="SYSTEM MESSAGE";
							}
						if (ymsg_sess->pkt.type == YMSG_GAMEMSG) {
							pmstype="GAME MESSAGE";
							}
						time_t time_llnow = time(NULL);
						fprintf(capture_fp, "\n%s\n%s:\nUser: %s\nMessage: %s\n\n", ctime(&time_llnow), pmstype, tmp,  ymsg_field( "14" ));
						fflush( capture_fp );
											}

				   if (is_flooder(tmp)) {return;}

					if (ymsg_sess->pkt.type != YMSG_SYSMSG) {
						/* possible spoofs: Messages from ourselves, so ignore */
						if (! from_valid_sender(tmp) ) {return;}
					}

					yalias_name=get_screenname_alias(tmp);

					if (ymsg_sess->pkt.type == YMSG_SYSMSG) {
					  if (enable_chat_spam_filter) {
						if (is_valid_system_message(tmp) && 
							is_valid_game_sys_msg_content(tmp, ymsg_field( "14" )))  {
								is_real_system_msg=1;
								} else {is_pm_spam=1;} /* Bogus system msg! - Spam bot */
																		} else {is_real_system_msg=1;}
					}

					if ( (ymsg_sess->pkt.type == YMSG_GAMEMSG) 
					|| (ymsg_sess->pkt.type == YMSG_PM_RECV) ) {
						if (enable_chat_spam_filter) {
							if (!is_valid_game_sys_msg_content(tmp, ymsg_field( "14" ))) {
								is_pm_spam=1;  /* Bogus game msg, spam bot */
							}
						}
					}

				/* added, PhrozenSmoke  block excessively large PMs from strangers
					and warn about a possible boot attempt */

	if (! is_real_system_msg) {
		if (is_boot_attempt(tmp, "large private messages", ymsg_sess->pkt.size)) {return;}
										 }

			if ((ymsg_sess->pkt.size>625) && 
			  (! is_real_system_msg) )  { /* let y! sys messages through */
				if ( strcasecmp( tmp, ymsg_sess->user )) {

					log_possible_flood_attack(tmp, 5,"large private messages");	
					if (is_flooder(tmp)) {return;}

					if (( ! find_friend( tmp )) && ( ! find_temporary_friend( tmp )) && 
					( ! ignore_check( tmp ))) {
						char *blankmsg=NULL;
						ignore_toggle( tmp );
						strncpy( tmp2, ymsg_field( "14" ), 70); 
						snprintf(pmbuf, 83, "%s...", tmp2);
						if (!disp_auto_ignored ) {snprintf(pmbuf, 63, "%s", " ");}
						snprintf( buf, 768, 
						"%s  ** Gyach-E - auto-ignored '%s' ( %s )  for sending possible PM boot codes (excessively large message).:  '%s'  **%s\n", YAHOO_COLOR_RED, tmp, yalias_name, _utf(pmbuf), YAHOO_COLOR_BLACK );
						append_to_textbox( chat_window, NULL, buf);
						/* NOT launching the pre-emptive strike here because if this is a 
						   'flood' of large PM packets, we will only lag the connection more */
						snprintf(buf, 200, "/tell %s <ding>",tmp);  /* send a buzz */
						chat_command( buf );
						ymsg_conference_decline( ymsg_sess, tmp, tmp, blankmsg); /* send a conf decline */
							return ;
														}

																		   }
										   }


				if (! strcmp( ymsg_field( "14" ), "" ))  { 
					if (! is_real_system_msg) {
						log_possible_flood_attack(tmp, 1,"empty PM messages");	
					} 
					return ; 										}  
				/* empty message field */

				/* field 14 = PM message text */
				/* changed: PhrozenSmoke - support for 'buzz' feature...
					 if the PM message text is '<ding>' the user has 'buzzed' us 
					 to get our attention....maybe show a dialog or play a sound here 
					 eventually */

				if (! strcmp("<ding>",ymsg_field( "14" ))) 
						{

					log_possible_flood_attack(tmp, 2,"PM Buzz");	 
					if (is_flooder(tmp)) {return;}

					accept_pm=0;

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want webcam invites either.
				 */

				accept_pm =  get_pm_perms(tmp);

	if (is_pm_spam && (find_user_row(tmp) >= 0) && 
 	   test_automute(tmp)) {
		/* If spammer is in chat room, shut them up */
		accept_pm=0;
		mute_toggle(tmp);
		snprintf(buf, 255, "  Gyach-E - auto-muted %s '%s'  ( %s ) %s : appears to be a spam bot [PM comment].\n", YAHOO_COLOR_OLIVE, tmp , yalias_name, YAHOO_COLOR_BLACK );
		append_to_textbox( chat_window, NULL, buf );
	}		

	if (! allow_pm_buzzes) {
		send_automated_response(tmp, tmp, "this user is not currently allowing you to 'Buzz' them. Your 'Buzz' will not be seen or heard by this user.");
		return; 
	}

				if (! accept_pm)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored BUZZ from user") , 
						"this user is not currently allowing you to 'Buzz' them. Your 'Buzz' will not be seen or heard by this user." , 
						1, 1);
						return;
							     			}

					if ( chat_timestamp_pm ) {append_timestamp(chat_window, NULL);	}

					/* also show in chat room window in case we aren't watchin PM box */
	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_buzz, NULL);	

					snprintf(buf, 1024, "<font size=\"16\">  %s%s%s %s%s %s[ %s ]  %s  %s%s\n", 
						"\033[#C65CC6m", YAHOO_STYLE_BOLDON, "** BUZZ!!! **",  
						"\033[#8DA8D4m", "** BUZZ!!! **", "\033[#AE46DBm", 	tmp,  pm_nick_names?yalias_name:"" , YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
						append_to_textbox( chat_window, NULL, buf );

					if ( (! pm_in_sep_windows) || (mute_check(tmp)) )  {
						if (find_friend(tmp) || find_temporary_friend(tmp))  {
							play_sound_event(SOUND_EVENT_BUZZ);							
																								  		}
							if (popup_buzz) {show_popup_dialog("BUZZ!!!", tmp, 1);}

						snprintf(tmp3, 100, "%s:  %s", tmp, "BUZZ!!!");	
						gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
						gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 
						return;
					}

					/* Now show in PM */
					if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
						pm_sess = pm_lpackptr->data;
					} else {
						/* open a new window if one doesn't exist already */
						pm_sess=new_pm_session(tmp);					
						gtk_widget_show_all( pm_sess->pm_window );	
					}
					
					align_pms_repyto(pm_sess);

					if ( auto_raise_pm ) {
						gdk_window_raise( pm_sess->pm_window->window );
						focus_pm_entry(pm_sess->pm_window);
					}

					if ( chat_timestamp_pm ) {
						append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
														}

	  				snprintf(tmp2,6, "%s", " ");
	  				append_to_textbox( pm_sess->pm_window, 	pm_sess->pm_text, tmp2 );
	  				append_char_pixmap_text((const char**)pixmap_pm_buzz, pm_sess->pm_text);	

					append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );

					show_incoming_pm(pm_sess, 1);

						/* Don't allow strangers to 'buzz' us with sounds */
						if (find_friend(tmp) || find_temporary_friend(tmp))  {
							play_sound_event(SOUND_EVENT_BUZZ);							
																								  		}
						if (popup_buzz) {show_popup_dialog("BUZZ!!!", tmp, 1);}

						snprintf(tmp3, 100, "%s:  %s", tmp, "BUZZ!!!");	
						gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
						gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 

	/* request buddy image if available */
	probe_for_bimage(tmp);

						return;

						} else { /* copy normal message text */
				strncpy( tmp2, ymsg_field( "14" ), 1175);  /* control message sizes */
								   }


				if ( (!strncasecmp(tmp2, "s: ", 3)) && (strlen(tmp2)>3) ) {
					log_possible_flood_attack(tmp, 3,"PM Web Searches");	 
					if (is_flooder(tmp)) {return;}
				} else {
					log_possible_flood_attack(tmp, 1,"Blocked Private Message");	 
					if (is_flooder(tmp)) {return;}
				}

					if ( ignore_check( tmp )) {
						if ( (! find_temporary_friend(tmp)) && (! find_friend(tmp))  ) {
							preemptive_strike(tmp); 
							send_automated_response(tmp, tmp, "this user is not accepting PMs from you. Your message will not be shown to this user.");
							return;
						}
									   					}

				/* check the fields determining who to accept PMs from */
				accept_pm = get_pm_perms(tmp);
				check_for_invisible_friend(tmp);

		if ( enable_chat_spam_filter && strcasecmp( tmp, ymsg_sess->user )
			 && (! find_friend(tmp))  && (! find_temporary_friend(tmp)) ) {
				if ( (is_chat_spam(tmp, tmp2)  || is_spam_name(tmp) ) )  {is_pm_spam=1;}
											  																}

		if (is_pm_spam) {
			snprintf(pmbuf, 73, "%s", "[Spam]");
			accept_pm=0;
		}

	if (is_pm_spam && (find_user_row(tmp) >= 0) && 
 	   test_automute(tmp)) {
		/* If spammer is in chat room, shut them up */
		mute_toggle(tmp);
		snprintf(buf, 255, "  Gyach-E - auto-muted %s '%s' %s : appears to be a spam bot [PM comment].\n", YAHOO_COLOR_OLIVE, tmp , YAHOO_COLOR_BLACK );
		append_to_textbox( chat_window, NULL, buf );
	}

				packptr = get_last_comment( tmp );
				if (( first_post_is_pm ) &&
					( ! find_friend( tmp )) && (!find_temporary_friend(tmp)) && 
					(( packptr == NULL ) ||
					 ( !strcmp( packptr, "Entered Room" )))) {
					if ( ! ignore_check( tmp )) {
						ignore_toggle( tmp );
					}						
					if (! is_pm_spam)  {
						strip_html_tags(tmp2);
						control_msg_line_feeds(); 
						snprintf(pmbuf, 70, "%s...", tmp2);
						if (!disp_auto_ignored ) {snprintf(pmbuf, 63, "%s", " ");}
					}		
					snprintf( tmp3, 600, "Auto-ignored user whose first post was a PM  [\"%s\"]  ",
						_utf(pmbuf) );

					comm_block_notify(tmp, yalias_name, 
						tmp3,  
						 "this user does not accept PMs from people who come in the room and immediately start PMing people (Talk in the ROOM first).", 
						1, 1);
					return;
				}

				if (( url_from_nonroom_user ) &&
					(( strstr( tmp2, "http://" )) ||
					 ( strstr( tmp2, "ftp://" )) ||
					 ( strstr( tmp2, " www." ))) &&
					( ! find_friend( tmp )) && (!find_temporary_friend(tmp)) && 
					( find_user_row( tmp )) < 0 ) {
					if ( ! ignore_check( tmp )) {
						ignore_toggle( tmp );
					}

					if (! is_pm_spam)  {
						strip_html_tags(tmp2);
						control_msg_line_feeds(); 
						snprintf(pmbuf, 70, "%s...", tmp2);
						if (!disp_auto_ignored ) {snprintf(pmbuf, 63, "%s", " ");}
					}		
					snprintf( tmp3, 600, "Auto-ignored user who sent a URL in a PM from oustside the room [\"%s\"]  ", _utf(pmbuf) );

					comm_block_notify(tmp, yalias_name, 
						tmp3,  
						 "this user does not accept URLs from other users not on their buddy list or in their current room.", 
						1, 1);
					return;
				}

				if ( ! accept_pm ) {					
					if (! is_pm_spam)  {
						strip_html_tags(tmp2);
						control_msg_line_feeds(); 
						snprintf(pmbuf, 70, "%s...", tmp2);
						if (!disp_auto_ignored ) {snprintf(pmbuf, 63, "%s", " ");}
					}					

					if ( find_friend( tmp ) || find_temporary_friend(tmp) ) {
						snprintf(tmp3, 600, "%s ( \"%s\" ) ", _("Ignored PM from Friend"), pmbuf); 
						comm_block_notify(tmp, yalias_name, 
							tmp3,  
							"but I have my PMs shut off right now (even to Buddies), so I won't see the message you just sent." , 	0, 1);
						set_last_comment( tmp, "Regular Private Message" ); 
						return;
					} else if ( find_user_row( tmp ) >= 0 ) {
				snprintf(tmp3, 600, "%s ( \"%s\" ) ", _("Ignored PM from User in Room"), pmbuf); 
				comm_block_notify(tmp, yalias_name, 
						tmp3,  
						"this user is not accepting PMs from you right now. Your message will not be shown to this user.  Maybe you should try talking IN THE ROOM instead." , 
						1, 1);
					return; 
					} else  {						
		snprintf(tmp3, 600, "%s ( \"%s\" ) ", 
			_("Ignored PM from User NOT in the Room"), pmbuf); 
		comm_block_notify(tmp, yalias_name, tmp3,  
				"but I have my PMs shut off to strangers, so I won't see the message you just sent.", 1, 1); 
		return;
				}

			return; /* accept_pm is false */  
		}


				if (( packptr = auto_ignore_check( tmp2 )) != NULL ) {
					if ( ! ignore_check( tmp )) {ignore_toggle( tmp );} 
					snprintf( buf, 512, "  Gyach-E - auto-ignored PM %s%s ( %s ) %s, regex '%s'\n",
						YAHOO_COLOR_RED, tmp, yalias_name, YAHOO_COLOR_BLACK, packptr );
					if (disp_auto_ignored ) {append_to_textbox( chat_window, NULL, buf );}
					send_automated_response(tmp, yalias_name, "this user is not accepting instant messages from you. Your message was blocked and will not be seen by this user.");
					return; 
				} 

				count = set_last_comment( tmp, "Regular Private Message" ); 

				if (( ! pm_in_sep_windows ) ||
					( mute_check( tmp ))) {
					int limit=400;
					char extra[25]="";
					sprintf(extra,"%s", "");
					if (find_friend( tmp )) {limit=900;}  /* liberal   limits for friends */
					if (find_temporary_friend( tmp )) {limit=600;}  /* liberal   limits for temp friends */

					if (ymsg_sess->pkt.type== YMSG_SYSMSG)  { /* system message */
						strncpy (extra, _("[System Message]"), 23);
						limit=1300;
						/* This happens when a 'web login' fails - 
							we get the system message 'Your account is deactivated'
							and Yahoo disconnects us, so show message and force logout
						*/
							ymsg_sess->pkt.type = YMSG_GYE_EXIT; 
							show_yahoo_packet();
							show_ok_dialog(_("LOGIN FAILED!"));
							login_window = build_login_window();	
							gtk_widget_show_all( login_window );			
										    	   					}

					if (ymsg_sess->pkt.type== YMSG_GAMEMSG)  { /* game message */
					strncpy (extra, _("[Game Message]"), 23);
										    	   }

					if ( chat_timestamp_pm ) {append_timestamp(chat_window, NULL);}

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_profile, NULL);	

					control_msg_line_feeds();
					if ( (pm_nick_names || (find_user_row(tmp)>=0)) && 
						strcmp(tmp, yalias_name) ) {
					snprintf( buf, limit, "        %s[%s%s%s  %s%s%s ( %s ) %s%s] %s%s  %s%s\n",
						"\033[#B88E64m", YAHOO_STYLE_ITALICON, 
						_("Instant Message from"), YAHOO_STYLE_ITALICOFF, 
						YAHOO_COLOR_RED, YAHOO_STYLE_BOLDON, tmp, yalias_name,
						"\033[#B88E64m", YAHOO_STYLE_BOLDOFF,
						YAHOO_STYLE_BOLDON, _utf(tmp2), extra, YAHOO_STYLE_BOLDOFF );

					} else {
					snprintf( buf, limit, "        %s[%s%s%s  %s%s%s%s%s] %s%s  %s%s\n",
						"\033[#B88E64m", YAHOO_STYLE_ITALICON, 
						_("Instant Message from"), YAHOO_STYLE_ITALICOFF, 
						YAHOO_COLOR_RED, YAHOO_STYLE_BOLDON, tmp,
						"\033[#B88E64m", YAHOO_STYLE_BOLDOFF,
						YAHOO_STYLE_BOLDON, _utf(tmp2), extra, YAHOO_STYLE_BOLDOFF );
					}

					append_to_textbox_color( chat_window, NULL, buf );
					play_sound_event(SOUND_EVENT_PM);

					if ( (!strncasecmp(tmp2, "s: ", 3)) && (strlen(tmp2)>3) &&
						enable_pm_searches  ) { /* web search */ 
						strip_html_tags(tmp2);
						handle_y_search_results(chat_window, NULL, tmp2);
					}

					/* auto-reply if necessary */
					if (( my_status ) &&
						( auto_reply_when_away ) &&
						( ! ignore_check( tmp )) &&
						( auto_reply_msg )) {
							snprintf(tmp3, 512 , " [Auto-Reply] : %s", auto_reply_msg );
						ymsg_pm( ymsg_sess, tmp, _b2loc(tmp3) );
					}

				} else {
					/* here we would send to a private pm session */
					/* duplicate to both for now for testing */
					int limit=450;
					char extra[25]="";
					GtkWidget *t_widget;
					sprintf(extra,"%s", "");
					if (find_friend( tmp )) {limit=1200;}  /* liberal   limits for friends */
					if (find_temporary_friend( tmp )) {limit=750;}  /* liberal   limits for temp friends */

					if (ymsg_sess->pkt.type== YMSG_SYSMSG)  { /* system message */
						strncpy (extra, _("[System Message]"), 23);
						limit=1300;
						/* This happens when a 'web login' fails - 
							we get the system message 'Your account is deactivated'
							and Yahoo disconnects us, so show message and force logout
						*/
							ymsg_sess->pkt.type = YMSG_GYE_EXIT; 
							show_yahoo_packet();
							show_ok_dialog(_("LOGIN FAILED!"));
							login_window = build_login_window();	
							gtk_widget_show_all( login_window );			
										    	   					}

					if (ymsg_sess->pkt.type== YMSG_GAMEMSG)  { /* game message */
					strncpy (extra, _("[Game Message]"), 23);
										    	   }

					if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
						pm_sess = pm_lpackptr->data;
					} else {
						/* open a new window if one doesn't exist already */
						pm_sess=new_pm_session(tmp);
						gtk_widget_show_all( pm_sess->pm_window );
					}

					if (pm_sess->sms) {strncpy (extra, "[SMS]", 23);}

					
					align_pms_repyto(pm_sess);

					if ( auto_raise_pm ) {
						gdk_window_raise( pm_sess->pm_window->window );
						focus_pm_entry(pm_sess->pm_window);
					}

					t_widget=gtk_object_get_data (GTK_OBJECT (pm_sess->pm_window), "pms_status");	
					if (t_widget) {
								gtk_label_set_text(GTK_LABEL(t_widget), "      ");
								gtk_widget_show_all(t_widget);
											  }

					if ( chat_timestamp_pm ) {
						append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
														}

					snprintf(buf, 256, "%s%s%s%s%s: ",
						YAHOO_COLOR_PMPURPLE, YAHOO_STYLE_BOLDON, pm_nick_names?yalias_name:tmp,
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );

					append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );

					real_show_colors = show_colors;

		if (pm_sess->encrypted_myway && pm_sess->encrypted_theirway) { /* encryption */
			set_current_gpgme_passphrase(pm_sess->their_gpg_passphrase);

			/* printf("Passes:\nMe (%s) %d\nThem (%s) %d\n",
				ymsg_sess->user, pm_sess->my_gpg_passphrase, 
				tmp, pm_sess->their_gpg_passphrase
			);
			fflush(stdout); */

			if (pm_sess->encryption_type==41) { /* send GPG passphrase num */
					if (pm_sess->my_gpg_passphrase==pm_sess->their_gpg_passphrase) {
						pm_sess->my_gpg_passphrase = make_gpgme_passphrase_nmbr();
						ymsg_encryption_gpgme_notify(ymsg_sess, tmp, pm_sess->my_gpg_passphrase);
																																		}
																	}
			
			strncpy(tmp3,tmp2, 2040);
			snprintf(tmp2, limit, "%s", _utf(gyache_decrypt_message(tmp, tmp3, pm_sess->encryption_type)));
			control_msg_line_feeds();
			snprintf( buf, limit, "%s  %s\n", tmp2 , extra ); 

			/* printf("decrypted\n"); fflush(stdout); */

		} else { /* no encryption */
			control_msg_line_feeds();
			snprintf( buf, limit, "%s  %s\n", _utf(tmp2) , extra );
				}

					// if (show_emoticons) {convert_smileys(buf); }
					append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, buf );

					real_show_colors = 1;
					show_incoming_pm(pm_sess, 1);
					play_sound_event(SOUND_EVENT_PM);
					if (strcmp( ymsg_field( "63" ), "" ))  {handle_imvironments();}

					/* request buddy image if available */
					probe_for_bimage(tmp);

					if ( (!strncasecmp(tmp2, "s: ", 3)) && (strlen(tmp2)>3) && 
						enable_pm_searches ) { /* web search */ 
						strip_html_tags(tmp2);
						handle_y_search_results(pm_sess->pm_window, pm_sess->pm_text, tmp2);
					}
				
				/* auto-reply if necessary */
				if (( my_status ) &&
					( auto_reply_when_away ) &&
					( ! ignore_check( tmp )) &&
					( auto_reply_msg )) {
						snprintf(tmp3, 512 , " [Auto-Reply] : %s", auto_reply_msg );
						append_to_textbox_color( pm_sess->pm_window,
						pm_sess->pm_text, tmp3 );

						if (pm_sess->encrypted_myway && pm_sess->encrypted_theirway) {
							ymsg_pm( ymsg_sess, tmp,  gyache_encrypt_message(get_profile_replyto_name(), tmp3 , pm_sess->encryption_type)  );
							} else {ymsg_pm( ymsg_sess, tmp, _b2loc(tmp3) ); }
					}

					if (pm_sess->encrypted_myway && pm_sess->encrypted_theirway) {return;}

				}


}




void handle_game_notice() {
		char *typptr=NULL;
		typptr=strchr(tmp,',');  /* cut off if its comma-separated */
		if (typptr) {*typptr='\0';}

					char url_room[384]="";
					char game_room[256]="";
					strncpy( buf, ymsg_field( "14"), 1000);
					packptr = buf;

					sprintf(game_room,"%s","");
					sprintf(url_room,"%s","");

					if (packptr) {
						end = strstr( packptr, "ante?" );
						if (end)  {
							packptr=end;
							end = strchr( packptr, '\t' );
							if (end) {
								*end = '\0';
								snprintf(url_room, 254, "http://games.yahoo.com/games/%s&nosignedcab=yes", packptr);
								packptr=end+1;
								    }
							     }
						end = strstr( packptr, "Yahoo!" );
						if (end)  {
							char *mptr;
							packptr=end;
							mptr = strchr( packptr, '\n' );
							snprintf(game_room, 35, "%s : ", _("Yahoo! Games") );
							while (mptr) {
								*mptr = '\0';
								strcat(game_room," ");
								strncat(game_room,packptr, 64);
								packptr=mptr+1;
								mptr = strchr( packptr, '\n' );
								if (!mptr) {
									strcat(game_room," ");
									strncat(game_room,packptr, 64);				
									      }
								if (strlen(game_room)>305) {break;}
								    }
							     }

						/* NEW */
						if (strlen(game_room)<1) {
							/* They are in games but dont want us to know, 
							or are pretending to be invisible...blow their cover */

							snprintf(url_room, 150, "%s", 
							_("The user is invisible")
							);
							snprintf(game_room, 150, "%s", 
							_("The user is hiding which game room they are in")
							);
																 }
				

						if (strlen(game_room)>1 ) {

							if (!find_friend(tmp)) {show_status_from_unknown_friend(tmp, "Games Status");}
							else {					


								FRIEND_OBJECT=create_or_find_yahoo_friend(tmp);
								if (FRIEND_OBJECT) {
									if (FRIEND_OBJECT->game_stat) {g_free(FRIEND_OBJECT->game_stat);}
									FRIEND_OBJECT->game_stat=g_strdup(_utf(game_room));
									FRIEND_OBJECT->ingames=1;

									if (strlen(url_room)>2) {
										/* dont allow hiding if we already have it */
										if (FRIEND_OBJECT->game_url) {g_free(FRIEND_OBJECT->game_url);}
										FRIEND_OBJECT->game_url=g_strdup(url_room);
									}
															  }

								if (!find_online_friend(tmp)) {display_buddy_online(tmp);}

								update_buddy_clist();

									} /* end else */

							if (!show_statuses) {return;}

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}


	  		snprintf(buf,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_games, NULL);	

					snprintf(buf, 1024, "  %s%s** %s: ( %s ) %s: %s'%s'%s [ %s ] **%s%s\n",
						YAHOO_STYLE_ITALICON, "\033[#880099m", 
						_("Buddy"), tmp, 
						_("is in Yahoo! game room"), 
						YAHOO_COLOR_RED, game_room, 
						 "\033[#880099m", url_room ,
						YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
						append_to_open_pms(tmp, buf,chat_timestamp_pm);
											}
						  }

}


void handle_ygame_notify() {
	/* This handles weird-o looking packets that are 
	sent when ppl are in games that look something 
	like this... */
		/*  [5ÀphrozensmokeÀ4Àltl__hy?ltl__hy(0À14ÀF5613BC6À13À0À49ÀyÀ] */ 
		/* honestly, i have no idea what purpose they serve, but they at 
		least let us know about possible invisible users hiding in games */

	if ( strcmp( ymsg_field( "4" ), "" )) {
		char *stopper=NULL;
		strncpy( tmp, ymsg_field( "4" ), 100);
		stopper=strchr(tmp,'?');
		if (stopper) {*stopper='\0';}
		check_for_invisible_friend(tmp);
		FRIEND_OBJECT=yahoo_friend_find(tmp);
		if (FRIEND_OBJECT) {
			if (!FRIEND_OBJECT->ingames) {
				FRIEND_OBJECT->ingames=1;
				update_buddy_clist();
			}
		}
	}	
}



void handle_webcam_invite_accept() {
					log_possible_flood_attack(tmp, 2,"Webcam Invite Accept Messages");	 
					if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					yalias_name=get_screenname_alias(tmp);
					accept_pm=0;

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied here
				 */
				accept_pm =  get_pm_perms(tmp);

				check_for_invisible_friend(tmp);

				if (! accept_pm)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored webcam invitation acceptance message from"),  
						"this user is not currently allowing you to view his/her webcam." , 
						1, 1);

					return ;
											 }

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_webcam, NULL);	

					snprintf(buf, 512, "  %s%s** '%s' ( %s )  %s ** %s%s\n", YAHOO_STYLE_ITALICON, YAHOO_COLOR_BLUE, tmp, yalias_name, _("has accepted your webcam invitation"),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
					append_to_textbox_color( chat_window, NULL, buf );
					append_to_open_pms(tmp, buf,chat_timestamp_pm);
					play_sound_event(SOUND_EVENT_OTHER);

}


void handle_webcam_invite_reject() {
					log_possible_flood_attack(tmp, 2,"Webcam Invite Decline Messages");	 
					if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					yalias_name=get_screenname_alias(tmp);
					accept_pm=0;

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied here
				 */
				accept_pm =  get_pm_perms(tmp);

				check_for_invisible_friend(tmp);

				if (! accept_pm)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored webcam invitation decline message from"),  
						"this user is not currently allowing you to view his/her webcam." , 
						1, 1);
					return ;
											 }

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_webcam, NULL);	

					snprintf(buf, 512, "  %s%s** '%s' ( %s )  %s ** %s%s\n", YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED, tmp, yalias_name, _("has declined your webcam invitation"),  YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
					append_to_textbox_color( chat_window, NULL, buf );
					append_to_open_pms(tmp, buf,chat_timestamp_pm);
					play_sound_event(SOUND_EVENT_OTHER);

					/* This can be a bit annoying */ 
					/* 
					if (find_friend(tmp) || find_temporary_friend(tmp)) { 
						snprintf(buf, 512, "%s %s", tmp, _("has declined your webcam invitation"));
						show_ok_dialog(buf);
												   													}
					*/ 

}






void handle_webcam_invite() {
					log_possible_flood_attack(tmp, 2,"Webcam Invitations");	 
					if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					/* These features will come in handy and already be in place when webcam support 
						is finally added */
					/* no invites from ignored users */

					yalias_name=get_screenname_alias(tmp);

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want webcam invites either.
				 */
				accept_pm =  get_pm_perms(tmp);
				if (auto_reject_invitations) {accept_pm=0;}
				if (! accept_pm)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored webcam invitation from"),  
						"this user is not currently accepting webcam invitations from you." , 
						1, 1);
					ymsg_webcam_invite_reject(ymsg_sess, tmp);
					return ;
											 }

					check_for_invisible_friend(tmp);

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_webcam, NULL);	

					snprintf(buf, 128 , "  %s%s** ", YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE);
					strncat(buf,_("webcam invitation from"), 70);
					strcat(buf," :  '");
					strncat(buf,tmp, 80);  /* their name */
					strcat(buf,"'  (");
					strncat(buf,yalias_name, 80);
					strcat(buf,")  **\n");
					strcat(buf,YAHOO_STYLE_ITALICOFF);
					strcat(buf,YAHOO_COLOR_BLACK);
					append_to_textbox_color( chat_window, NULL, buf );		
					append_to_open_pms(tmp, buf,chat_timestamp_pm);			
					
if (!enable_webcam_features) {
		/* SEND a message for people with webcam support disabled  */
		send_automated_response(tmp, yalias_name, "this user is not able to view your webcam right now - maybe some other time.");
		ymsg_webcam_invite_reject(ymsg_sess, tmp);
		return; 
										}
	play_sound_event(SOUND_EVENT_OTHER);
	yahoo_webcam_invite_msg(tmp);
}



void handle_contact_info() {
	char *typptr=NULL;
	typptr=strchr(tmp,',');  /* cut off if its comma-separated */
	if (typptr) {*typptr='\0';}

				/* The same person shouldnt be sending this over and over */
					log_possible_flood_attack(tmp, 2,"Buddy List Contact Information");	 
					if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					/* We REALLY shouldnt be getting sent contact info..i.e. business cards 
						from people who are not friends, since on windows you have to right 
						click on a buddy on your buddy list to send it...may be a stalker/booter */

					if (!find_friend(tmp)) { /* add an extra 'danger' point */
						log_possible_flood_attack(tmp, 1,"Buddy List Contact Information From Stranger");	 
						show_status_from_unknown_friend(tmp, "Buddy List Contact Information From Stranger");
												  }
					if (is_flooder(tmp)) {return;}

					yalias_name=get_screenname_alias(tmp);

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want webcam invites either.
				 */
				accept_pm =  get_pm_perms(tmp);
				check_for_invisible_friend(tmp);

				if (! accept_pm)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored personal contact information from"),  
						"this user is not currently accepting contact information (addressbook entries) from you." , 1, 1);
					return ;
											 }

	strncpy( tmp3, ymsg_field( "14"), 1200);
	packptr = tmp3;
	last = 0;
	append_timestamp(chat_window, NULL);
	  				snprintf(buf,6, "%s", "  ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_profile , NULL);	
	  				snprintf(buf,6, "%s", " \n");
	  				append_to_textbox( chat_window, NULL, buf );

	snprintf(buf, 400, "  %s%s*** %s ***\n  *** %s ( %s ) ***\n     " ,
		YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE,
		_("CONTACT INFORMATION"), tmp, yalias_name
	);

	while( packptr ) {
		end = strchr( packptr, '\t' );
		if ( end ) {*end = '\0';} 
		else {last = 1;	}
		strncat(buf, _utf(packptr), 125 );  /* avoid buffer overflows */
		strcat(buf, "\n     ");
		if ( last ) {packptr = NULL;} 
		else {packptr = end + 1;}
		if (strlen(buf)>1200) {break;}  /* avoid buffer overflows */
							}  // end while

	strcat(buf,"\n");
	strcat(buf,YAHOO_STYLE_ITALICOFF);
	strcat(buf,YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, buf );
	show_ok_dialog(_("A user has sent you their personal contact information.  Please check the window named 'Chat'."));
	play_sound_event(SOUND_EVENT_OTHER);

}



void handle_typing_notice() {
 	GtkWidget *t_widget;
	char *typptr=NULL;

	if (strchr(tmp,',')) {
		strncpy(tmp3, tmp, 256);
		typptr=strrchr(tmp3,',');  /* cut off if its comma-separated */
		typptr++;  /* take the last name in the list */ 
		/* if (typptr) {*typptr='\0';}  */
		snprintf(tmp,96, "%s", typptr);
	}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}
		if (is_flooder(tmp)) {return;}

	if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) 
	{
		pm_sess = pm_lpackptr->data;
		t_widget=gtk_object_get_data(GTK_OBJECT(pm_sess->pm_window), "pms_status");	

		if ( GTK_WINDOW(pm_sess->pm_window)->title[0] == '*' ) {
			strncpy( buf, "* " , 8);
		} else {
			buf[0] = '\0';
				  }
			strncat( buf, tmp, 60 );
			if ( ! strcmp( ymsg_field( "13" ), "1" )) {
				strncat( buf, _(" [TYPING]"), 30 );
				if (t_widget) {
					snprintf(tmp3, 600, "<span foreground=\"#a04dd1\"  font_desc=\"sans bold 12\" background=\"#e8eef7\">[</span><span foreground=\"#4251b6\"  font_desc=\"sans bold 12\" background=\"#e8eef7\">%s</span><span foreground=\"#a04dd1\"  font_desc=\"sans bold 12\" background=\"#e8eef7\">]</span>", _("[TYPING]"));
					gtk_label_set_markup(GTK_LABEL(t_widget), tmp3);
					/* gtk_label_set_text(GTK_LABEL(t_widget), _("[TYPING]")); */ 
					gtk_widget_show_all(t_widget);
								}
																		  } else   {
				if (t_widget) {
					gtk_label_set_text(GTK_LABEL(t_widget), "      ");
					gtk_widget_show_all(t_widget);
									}
																					 }
				strcat( buf, " - GyachE" );
				gtk_window_set_title( GTK_WINDOW(pm_sess->pm_window),buf );

	} else { 
		/* If we are receiving a typing status, but dont have a PM 
		window open...OR we have GyachE setup to show PMs 
		on the chat screen instead of in separate PM windows, 
		show the typing status in the chat status bar instead. */
	if ( ! strcmp( ymsg_field( "13" ), "1" )) {  /* typing */
		snprintf(buf, 65, "%s", tmp);
		strncat(buf, "  ", 5);
		strncat(buf, _("[TYPING]"), 30);
		} else {snprintf(buf, 8, "%s", "  ");}  /* not typing */
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,buf);
	check_for_invisible_friend(tmp);  
		  }

}


void handle_buddy_on() {
	if ( strcmp( ymsg_field( "16" ), "" ))  {  /* IDACT error */
			snprintf(buf, 400, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Error"),
			ymsg_field( "16" ), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			return ;
													  }

	 if ( (strcmp( ymsg_field( "8" ), "" ))  || (strchr(ymsg_field( "7" ), ','))  ) {
		int withold_post=0;
		/* If field 8 is present, we have a list of
		online buddies, either from YMSG_BUDDYON
		or YMSG_USERSTAT packet, or another list with multiple names...
		these are 'follow-up' packets when all the names cant fit in one  */

					  /* reset status if we refreshed buddy list */
					if (ymsg_sess->pkt.type==YMSG_USERSTAT && first_userstat) {	
						/* we get list of online buddies already
						   through a ymsg_buddyon packet, 
						   dont show buddy list twice */
						first_userstat=0;
						return;
					} 

					/* initial buddy list of people online */
					if ( ! strcmp( ymsg_field( "7" ), "" )) {return;}

					strncpy( buf, ymsg_field( "7"), 2046);
					packptr = buf;
					last = 0;
					while( packptr ) {
						end = strchr( packptr, ',' );
						if ( end ) {*end = '\0';} else {last = 1;}
						add_online_friend( packptr );
						if ( last ) {packptr = NULL;} else {packptr = end + 1;}
					}

					if (ymsg_sess->pkt.type == YMSG_USERSTAT) {withold_post=1;}
					/* if ( ! strcmp( ymsg_field( "8" ), "" ) ) {withold_post=1;} */ 
					parse_initial_statuses( ymsg_sess->pkt.data); /* added: PhrozenSmoke */
					update_buddy_clist();  /* added: PhrozenSmoke */
					if (! withold_post) { push_friend_list_spool();	show_friends();  }
					return;
		}  /* end presence of field 8 */

			
				if (ymsg_sess->pkt.type==YMSG_IDACT) {
					if ( ! strcmp( ymsg_field( "3" ), "" )) {return; }
					strncpy( tmp, ymsg_field( "3"), 125);
				} else {  
					 /* YMSG_BUDDY_OFF*/
					if ( ! strcmp( ymsg_field( "7" ), "" )) {return;}
					strncpy( tmp, ymsg_field( "7"), 125);
					}

				/* we either just came on or came back from invisible with a status */
				if ( ! strcasecmp( tmp, ymsg_sess->user ) || 
					(find_profile_name(tmp)) ) {
					/* we just signed on */
					if (!got_buddy_ignore_list) {return;}	

					if ( (strcmp( ymsg_field( "19" ), "" )) || (strcmp( ymsg_field( "10" ), "0" )) ) {

						snprintf(buf, 512, "  %s%s** %s  [ %s ] **%s%s\n", 
						YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE,
						_("You are no longer invisible"), tmp, 
						YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
						append_to_textbox_color( chat_window, NULL, buf );
					}
																}

				if ( strcmp( ymsg_field( "10" ), "" ) && 
					(ymsg_sess->pkt.type != YMSG_IDACT) ) {
					/* Check for initial status  */ /* added: PhrozenSmoke */
					handle_away_back_cb();
					return;
										      							} 

 				display_buddy_online(tmp);
				update_buddy_clist();  /* added: PhrozenSmoke */
}


void handle_buddy_off() {
		if (ymsg_sess->pkt.type==YMSG_SIGNOFF) {
			/* In Messenger-6, this packet usually has field 66 set to 1 */
			if ( (! ymsg_sess->pkt.size) || (strcmp(ymsg_field("66"), "")) ) {
					/* Yahoo booted us, or we logged off */
					handle_logoff();
					return; 
			}
		}

				if ( ! ymsg_sess->pkt.size ) {
					/* Yahoo booted us, or we logged off */
					handle_logoff();
					return; 
				}

	if ( strcmp( ymsg_field( "16" ), "" ))  {  /* IDDEACT error */
			snprintf(buf, 400, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Error"),
			ymsg_field( "16" ), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			return ;
													  }

				if (ymsg_sess->pkt.type==YMSG_IDDEACT) {
					if ( ! strcmp( ymsg_field( "3" ), "" )) {return; }
					strncpy( tmp, ymsg_field( "3"), 125);
				} else {  
					 /* YMSG_BUDDY_OFF*/
					if ( ! strcmp( ymsg_field( "7" ), "" )) {return;}
					strncpy( tmp, ymsg_field( "7"), 125);
					}

				if ( (! strcasecmp( tmp, ymsg_sess->user )) || 
					(find_profile_name(tmp)) ) {
					snprintf(buf, 512, "%s%s  ** %s  [ %s ] **%s%s\n", 
					YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE,
					_("You are now invisible"),tmp,
					YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
					append_to_textbox_color( chat_window, NULL, buf );
					if ( find_online_friend( tmp )) {
						remove_online_friend( tmp );				
						set_friend_stats(tmp);
						/* parse_initial_statuses( ymsg_sess->pkt.data); */ /* added: PhrozenSmoke */
						set_buddy_status_full( tmp, _("Invisible"), 1);
						update_buddy_clist();
						play_sound_event(SOUND_EVENT_BUDDY_OFF);
						if (popup_buddy_off) {show_popup_dialog(tmp, _("has logged OFF\n[You are now invisible]"),2 ); }
										   }
					return;
				}

				if (!find_friend(tmp)) {show_status_from_unknown_friend(tmp, "Buddy offline");}

				display = 1;
				if ( find_online_friend( tmp ) || ycht_only) {

							FRIEND_OBJECT=create_or_find_yahoo_friend(tmp);
							if (FRIEND_OBJECT) {								
								if (FRIEND_OBJECT->idle_stat) {g_free(FRIEND_OBJECT->idle_stat);}
								if (FRIEND_OBJECT->main_stat) {g_free(FRIEND_OBJECT->main_stat);}
								if (FRIEND_OBJECT->radio_stat) {g_free(FRIEND_OBJECT->radio_stat);}
								FRIEND_OBJECT->main_stat=g_strdup("");
								FRIEND_OBJECT->idle_stat=g_strdup("");
								FRIEND_OBJECT->radio_stat=g_strdup("");
								FRIEND_OBJECT->ingames=0;
								FRIEND_OBJECT->inchat=0;
								FRIEND_OBJECT->insms=0;
								FRIEND_OBJECT->idle=0;
								FRIEND_OBJECT->away=0;
								FRIEND_OBJECT->webcam=0;
								FRIEND_OBJECT->launchcast=0;
								toggle_sms_pm_session(tmp, 0); 
														  }

					remove_online_friend( tmp );

				if ( strcmp( ymsg_field( "10" ), "" )) {
					/* Run this: when users are invisible, 
				  	 we can still see when they are in a chat and games!  */
					/* parse_initial_statuses( ymsg_sess->pkt.data);  */
					set_friend_stats(tmp);
					if (FRIEND_OBJECT) {FRIEND_OBJECT->insms=0;}
																		 }

					update_buddy_clist();  /* added: PhrozenSmoke */
					play_sound_event(SOUND_EVENT_BUDDY_OFF);
				} else {
					display = 0;
				}

				if ( display && show_enters ) {
				int went_invisible=0;
				int used_stealth=0;

				if (ymsg_sess->pkt.type==YMSG_IDDEACT) {went_invisible=1;}

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}

 				snprintf( buf, 225, "%s%s  **%s %s: %s%s%s  ",
 					YAHOO_STYLE_BOLDON,YAHOO_COLOR_GREEN, YAHOO_COLOR_RED, _("Buddy"), YAHOO_COLOR_PURPLE,tmp,YAHOO_COLOR_GREEN);

				strncat(buf,_("has logged OFF"), 35);
				strcat(buf,". **\n");
				strcat(buf,YAHOO_COLOR_BLACK);
				strcat(buf,YAHOO_STYLE_BOLDOFF);

				/* Snooping on invisible friends!  hehehe */
				
				if (FRIEND_OBJECT) {
					strcat(buf,"\033[#5791B1m");
					/* show people hiding in chat and games while invisible */
					if (FRIEND_OBJECT->ingames) {
						strcat(buf, "  ** ");
						strncat(buf,_("The user is still in Yahoo! Games. (invisible)"), 75);
						strcat(buf," **\n");
						went_invisible=1;
																		  } else {
							if (FRIEND_OBJECT->game_stat) {g_free(FRIEND_OBJECT->game_stat);}
								FRIEND_OBJECT->game_stat=g_strdup("");
							if (FRIEND_OBJECT->game_url) {g_free(FRIEND_OBJECT->game_url);}
								FRIEND_OBJECT->game_url=g_strdup("");
																					}
					if (FRIEND_OBJECT->inchat) {
						strcat(buf, "  ** ");
						strncat(buf,_("The user is still in Yahoo! Chat. (invisible)"), 75);
						strcat(buf," **\n");
						went_invisible=1;
																	  }
					strcat(buf,YAHOO_COLOR_BLACK);
													 }


				/* believe it or not, we can tell the difference 
					between a user who is truly offline, from one
					who went invisible... the 'buddy offline' packet
					only contains field '138' (usually set to '1') when 
					the user is invisible; it does not have '138' when
					they fully disconnect from Yahoo - this is on Messenger
					5...on Messenger 6 the packet will have fields 
					213, 197, 198...all, some, or any combination of them,
					these fields are NOT present when somebody truly 
					logoff */

					if ( strcmp( ymsg_field( "138" ), "" ) || strcmp( ymsg_field( "213" ), "" ) ||
						strcmp( ymsg_field( "198" ), "" ) || strcmp( ymsg_field( "197" ), "" ) ) {
						/* This use went INVISIBLE!, not offline
							blow their cover */
						went_invisible=1;
																				}


					/* Also, on Messenger 6, if a person went invisible 
					because they were using Stealth settings to hide from 
					you, there will be a field 138, and field 10 may be 
					set to something other than the usual '0', field 10 is 
					always set to zero on a real sign off...another tell-tell
					sign is the presence of field 184 (launchcast/webcam) or 
					187 (custon status)...since field 138 can be set because
					somebody is simply invisible on Messenger 5, we dont 
					know for sure that stealth is used unless 10>0 or 
					field 184 or 187 are present.  */ 

					if (  (atoi(ymsg_field("10"))>0)   ||
						strcmp( ymsg_field( "184" ), "" ) || strcmp( ymsg_field( "187" ), "" ) ) {
						/* This use went INVISIBLE!, not offline
							blow their cover */
						used_stealth=1;
						went_invisible=1;
																				}

						if (went_invisible) {
						strcat(buf,"\033[#5791B1m");
						strcat(buf, "  ** ");
						strncat(buf,_("The user is still on Yahoo! Messenger. (invisible)"), 75);
						strcat(buf," **\n");

						if (used_stealth) {
							strcat(buf, "  ** ");
							strncat(buf,_("The user used Stealth settings to be invisible."), 75);
							strcat(buf," **\n");
						}

						strcat(buf,YAHOO_COLOR_BLACK);
						set_buddy_status_full( tmp, used_stealth?_("Invisible - Stealth Settings"):_("Invisible"), 1);
						update_buddy_clist();
						}

					if (popup_buddy_off) {
						char offaway[96];
						snprintf(offaway, 40, "%s", _("has logged OFF"));
						if (went_invisible) {
							strcat(offaway,"\n[");
							strncat(offaway, used_stealth?_("Invisible - Stealth Settings"):_("Invisible"), 50);
							strcat(offaway,"]");
						}
						show_popup_dialog(tmp, offaway,6 ); 
														}


	  		snprintf(tmp3,6, "%s", "  ");
	  		append_to_textbox( chat_window, NULL, tmp3 );
			if (went_invisible) {
	  			append_char_pixmap_text((const char**)pixmap_stealth_off, NULL);	
			} else {
	  			append_char_pixmap_text((const char**)pixmap_buddy_offline, NULL);	
			}
	  		snprintf(tmp3,6, "%s", " \n");
	  		append_to_textbox( chat_window, NULL, tmp3 );

					append_to_textbox_color( chat_window, NULL, buf );
					append_to_open_pms(tmp, buf,chat_timestamp_pm);

	snprintf(tmp3, 140, "%s: %s", tmp, _("has logged OFF"));	
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 

																	}

}


void handle_chat_invite() {
	char cmsg[150]="";

	if (ymsg_sess->pkt.type==0x17)  {  /* invitation could not be sent */
			snprintf(buf, 300, "  %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Error"),
			_("Could not send invitation"), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, buf );
			return;
				  					}

				strncpy( tmp, ymsg_field( "119" ), 255);
				strncpy( tmp2, ymsg_field( "104" ), 255);
				if ( ! strcmp( tmp, "" )) {
					snprintf(buf, 1024, "  %s** %s  '%s' **%s\n",
						YAHOO_COLOR_GREEN, 
						_("User has been invited to"), 
						_utf(tmp2),
						YAHOO_COLOR_BLACK );
					if (ymsg_sess->last_pkt.type != 0x17) {
						/* If the last packet was a chat invite 'failed' message,
						ignore this confirmation message */
						append_to_textbox_color( chat_window, NULL, buf );	
					}
					return;
				}

				if (is_boot_attempt(tmp, "large chat invitations", ymsg_sess->pkt.size)) {return;}
				log_possible_flood_attack(tmp, 1,"Chat Invitations");	 
				if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}
			
				if (yalias_name) {g_free(yalias_name);}
				yalias_name=get_screenname_alias(tmp);

				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want chat invites either.
				 */

		accept_pm = get_pm_perms(tmp);
		if (auto_reject_invitations) {accept_pm=0;}
		check_for_invisible_friend(tmp);

				if (! accept_pm)  {
					snprintf( tmp3,400, "%s : '%s'  ['%s'] ", 
						_("Ignored chat invitation from"), tmp,  tmp2 );
					comm_block_notify(tmp, yalias_name, 
						tmp3,  
						"this user is not currently accepting chat invitations from you." , 
						1, 1);
					return ;
											 }

			sprintf(cmsg,"%s","");
			strcat(cmsg,"  ");
			strncat(cmsg,_("Message"), 20);
			strcat(cmsg," : '");

			if ( strcmp( ymsg_field( "117" ), "" )) { /* message */
			/* set 100 byte limit on message, avoid 'boots' from large messages */
			strncat(cmsg, _utf(ymsg_field( "117" )), 100);
									   } else {strncat(cmsg, _("None"), 20);}
			strcat(cmsg,"' ");

	  		snprintf(buf,6, "%s", "  ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_pm_join, NULL);	

				sprintf( buf,
					"\n%s  ** %s (%s%s%s%s%s) : %s%s '%s' / '%s'  %s%s %s **%s\n",
					YAHOO_COLOR_PURPLE, _("You have been invited to"), 
					YAHOO_COLOR_RED, YAHOO_STYLE_BOLDON,
					_utf(tmp2), YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_PURPLE,
					YAHOO_COLOR_BLUE, YAHOO_STYLE_BOLDON, tmp, yalias_name,
					 YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_PURPLE, 
					cmsg, YAHOO_COLOR_BLACK );

				if (accept_pm) {
					GtkWidget *okbutton;
					append_to_textbox_color( chat_window, NULL, buf );
					append_to_open_pms(tmp, buf,chat_timestamp_pm);

				snprintf(buf, 2000, "%s:  '%s'\n\n%s: %s  ( %s )\n\n%s\n", _("You have been invited to"), 
						_utf(tmp2), _("by"),  tmp, yalias_name, cmsg);
				okbutton=show_confirm_dialog(buf,"Join Room","Cancel");
				if (!okbutton) {return;}
				gtk_object_set_data(GTK_OBJECT(okbutton), "chatroom", strdup(tmp2));
  				gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      	GTK_SIGNAL_FUNC (on_room_invite_accept_cb), NULL);
				gtk_object_set_data( GTK_OBJECT(okbutton), PROFNAMESTR, strdup(get_profile_replyto_name()) );
				play_sound_event(SOUND_EVENT_OTHER);
				}

}


void handle_logout() {
				in_a_chat=0;
				// voice_enabled=0;  /* disabled, enables eavesdropping lol */
				if (!is_conference) { clear_chat_list_members();}

				/* show user that we aren't in chat  anymore */				
				snprintf(buf, 512, "%s%s  [** %s **] %s%s\n\n",
					YAHOO_COLOR_BLUE, YAHOO_STYLE_BOLDON,
					_("You have left the Yahoo chat area."),
					YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK );
				append_to_textbox_color( chat_window, NULL, buf );
				if (!is_conference) { strncpy( ymsg_sess->room, "[NONE]", 62 );}
				gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
				gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
					_("Not currently connected to chat.") );
				if (immed_rejoin)  {
					ymsg_online(ymsg_sess);
					immed_rejoin=0;
							  } else {
					if (!is_conference) {set_current_chat_profile_name(get_default_profile_name());}
					}

}


void handle_file_transfer() {
				strncpy( tmp, ymsg_field( "4" ), 256);  /* from */
				if (!strcmp(tmp, "")) {return;}

				if ( ! strcmp( tmp, "FILE_TRANSFER_SYSTEM" )) {  /* PhrozenSmoke, changed messages */
					/* we sent a file */
					strncpy( tmp2, ymsg_field( "14" ), 768);

		if (strcmp(tmp2, "")) {
			if (strstr(tmp2, "http")) {		
				if ( strstr(tmp2, ".friend_icon.png") )  { /* a buddy image */
					handle_buddy_image_uploaded();
					bimage_timestamp= ((int) time(NULL)) + 86400;
					return;
					}
				}
			}

	  				snprintf(buf,6, "%s", "\n  ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_file , NULL);

					snprintf(buf, 1024, "\n%sFILE:%s %s \n",
						YAHOO_COLOR_GREEN, YAHOO_COLOR_BLACK, _utf(tmp2) );
					append_to_textbox_color( chat_window, NULL, buf );
					play_sound_event(SOUND_EVENT_OTHER);
				} else {

				if (! from_valid_sender(tmp) ) {return;} /* its from us, so ignore */
				log_possible_flood_attack(tmp, 1,"FILE TRANSFER SYSTEM File");	 
				if (is_flooder(tmp)) {return;}

					/* we received a file */
					yalias_name=get_screenname_alias(tmp);
					check_for_invisible_friend(tmp);


				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want files being 
					sent either.
				 */
				accept_pm =  get_pm_perms(tmp);

				if (allow_no_sent_files || (! accept_pm))  {
				comm_block_notify(tmp, yalias_name, 
						_("Ignored file sent from"),  
						"this user is not currently accepting files from you.", 
						(! accept_pm)?1:0, 1);
				return;
								    }


					strncpy( tmp2, ymsg_field( "20" ), 900);
					strncpy( tmp3, ymsg_field( "14" ), 512);
				  append_timestamp(chat_window, NULL);

	  				snprintf(buf,6, "%s", "  ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_file , NULL);	

				  snprintf(buf, 256, "\n%s%s** %s: ",YAHOO_STYLE_BOLDON, YAHOO_COLOR_PURPLE, _("User")); 

				  strcat(buf, "'");
				  strncat(buf, tmp, 80);
				  strcat(buf, "' ( ");
				  strncat(buf,yalias_name, 80);

				  strcat(buf," ) ");
				  strncat(buf, _("has sent you a file using the file transfer system."), 90);
				  strcat(buf, " **\n");
				  if (strcmp( ymsg_field( "27" ), "" ))  {  /* File name */
				  	strcat(buf, "   ");
				  	strncat(buf, _("File name"), 40);
				  	strcat(buf, ":  '");
				  	strncat(buf, _utf(ymsg_field( "27" )),200); /* avoid buffer overflow from excess. long file names */
				  	strcat(buf, "'\n");
										    								  }

	//printf("debug-5\n"); fflush(stdout);

				  if (strcmp( ymsg_field( "28" ), "" ))  {  /* File size */
				  	strcat(buf, "   ");
				  	strncat(buf, _("File size"), 40);
				  	strcat(buf, ":  ");
				  	strncat(buf, ymsg_field( "28" ), 16);
				  	strcat(buf, " ");
				  	strncat(buf, _("bytes"), 24);
				  	strcat(buf, "\n");
										    	}

	//printf("debug-6\n"); fflush(stdout);

				  if (strcmp( ymsg_field( "14" ), "" ))  {  /* message */
				  	strcat(buf, "   ");
				  	strncat(buf, _("Message"), 32);
				  	strcat(buf, ":  '");

					if(strlen(tmp3)>0) {
						char *tttmp;
						tttmp = strchr(tmp3, '\006');
						if(tttmp) {*tttmp = '\0';}
												}

				  	strncat(buf, _utf(tmp3),95);  /* to avoid buffer overflow from booters, limit 95 bytes */
				  	strcat(buf, "'\n");
										    	 }

	//printf("debug-7\n"); fflush(stdout);

				  if (strcmp( ymsg_field( "20" ), "" ))  {  /* download URL */
				  	strcat(buf, "   ");
				  	strncat(buf, _("Download"), 32);
				  	strcat(buf, ":  ");
				  	strncat(buf, _utf(tmp2),355);   /* avoid buffer overflow from excess. long file names */
				  	strcat(buf, "\n");
										    	}

		if (strcmp( ymsg_field( "38" ), "" ))  {  /* Expiration date */
			time_t mytime=0;
			char timebuf[38]="";
			char *tmp_time=NULL;
			mytime=time(NULL);
			mytime=mytime-time(NULL)+atol(ymsg_field( "38" ));
			tmp_time=strdup(ctime_r( &mytime, timebuf));
			strcat(buf, "   ");
			strncat(buf, _("Available Until"), 40);
			strcat(buf, ":  ");
			strncat(buf, _utf(tmp_time),40); 
			strcat(buf, "\n");
			if (tmp_time) {free(tmp_time);}
		}

	//printf("debug-8\n"); fflush(stdout);

				  strcat(buf, YAHOO_STYLE_BOLDOFF);
				  strcat(buf, "\n\n");
				  append_to_textbox( chat_window, NULL, buf );
				  append_to_open_pms(tmp, buf,chat_timestamp_pm);
				  show_ok_dialog(_("A file has been sent to you.  Please check the window named 'Chat'."));

	//printf("debug-9\n"); fflush(stdout);
					play_sound_event(SOUND_EVENT_OTHER);
				}

}


void handle_games_enter() {
			if ( strcmp( ymsg_field( "7" ), "" )) {
				strncpy( tmp, ymsg_field( "7" ), 255);
				strncpy( tmp2, ymsg_field( "10" ), 255);
				strncpy( tmp3, ymsg_field( "19" ), 512);

				if (!find_friend(tmp)) {show_status_from_unknown_friend(tmp, "Games Enter");}

					if (show_enters) {

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}


	  		snprintf(buf,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_games, NULL);	

				snprintf(buf, 300, "  %s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("Buddy")); 
				strcat(buf, "'");
				strncat(buf, tmp, 255);
				strcat(buf, "' ");
				strncat(buf, _("has started playing in Yahoo! Games."), 128);
				strcat(buf, " **");
				strcat(buf, YAHOO_STYLE_ITALICOFF);
				strcat(buf, "\n");
				append_to_textbox( chat_window, NULL, buf );
				append_to_open_pms(tmp, buf,chat_timestamp_pm);
											}

					//set_buddy_status_full( tmp,_("has started playing in Yahoo! Games."), 1);
				if (find_friend(tmp)) {
					set_friend_stats(tmp);

							FRIEND_OBJECT=create_or_find_yahoo_friend(tmp);
							if (FRIEND_OBJECT) {
								if (FRIEND_OBJECT->game_stat) {g_free(FRIEND_OBJECT->game_stat);}
								FRIEND_OBJECT->game_stat=g_strdup(_("has started playing in Yahoo! Games."));
								FRIEND_OBJECT->ingames=1;
														  }

				    display_buddy_online(tmp);
					update_buddy_clist();
											 }

											 			}
}


void handle_games_leave() {
			if ( strcmp( ymsg_field( "7" ), "" )) {
				strncpy( tmp, ymsg_field( "7" ), 255);
				strncpy( tmp2, ymsg_field( "10" ), 512);
				strncpy( tmp3, ymsg_field( "13" ), 512);

				if (!find_friend(tmp)) {show_status_from_unknown_friend(tmp, "Games Exit");}

					if (show_enters) {

				if ( enter_leave_timestamp) {append_timestamp(chat_window, NULL);}


	  		snprintf(buf,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_games, NULL);	

				snprintf(buf, 300, "  %s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("Buddy")); 
				strcat(buf, "'");
				strncat(buf, tmp, 255);
				strcat(buf, "' ");
				strncat(buf, _("has stopped playing in Yahoo! Games."), 128);
				strcat(buf, " **");
				strcat(buf, YAHOO_STYLE_ITALICOFF);
				strcat(buf, "\n");
				append_to_textbox( chat_window, NULL, buf );
				append_to_open_pms(tmp, buf,chat_timestamp_pm);
								 }


				  if (find_friend(tmp))  	{

					if (! find_online_friend( tmp )) {
						/* status change for an invisible friend */
					if (show_enters) {
					snprintf(buf, 512, "%s%s**%s: ( %s ) '%s' **%s%s\n",
						YAHOO_STYLE_ITALICON, YAHOO_COLOR_BLUE, 
						_("Invisible Buddy"), tmp, _("has stopped playing in Yahoo! Games."),
						YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
											 }
															      } 

					ignore_mailstat=1;
					reset_all_online_friends_statuses();
					set_friend_stats(tmp);

			FRIEND_OBJECT=yahoo_friend_find(tmp);
			if (FRIEND_OBJECT) {
			if (FRIEND_OBJECT->game_stat) {
				g_free(FRIEND_OBJECT->game_stat);
				FRIEND_OBJECT->game_stat=g_strdup("");
				}
			if (FRIEND_OBJECT->game_url) {
				g_free(FRIEND_OBJECT->game_url); 
				FRIEND_OBJECT->game_url=g_strdup("");
				}
				FRIEND_OBJECT->ingames=0;
			}

					if (strcmp(tmp3,"")) {  /* see if they went offline */
							if (atoi(tmp3)==0) {
								remove_online_friend(tmp); 
								sprintf(tmp2, "%s", "");
								set_buddy_status( tmp , tmp2);
								}
					}
					ymsg_refresh( ymsg_sess);

					/* sometimes when a person leaves games, they also sign out of 
						Yahoo completely, other times, the person stays online because they 
						have both games and messenger running - most times yahoo 
						doesnt tell us if a person leaving games has logged out of Yahoo 
						completely, only way to know is to refresh the list*/
													}
																		 }
}


void handle_create_room() {
				if ( strcmp( ymsg_field( "114" ), "" )) {
					/* This happens when Yahoo tells we couldnt create a room*/
					/* added: PhrozenSmoke */
					if (ymsg_sess->pkt.size<20) {
						snprintf(buf, 512, "  %s%s** %s **%s%s\n",
						"\033[#BD8B71m", YAHOO_STYLE_ITALICON,
						_("The room you tried to create could not be created."), 
						YAHOO_COLOR_BLACK, YAHOO_STYLE_ITALICOFF
							);
						append_to_textbox( chat_window, NULL, buf );
						return;
										  }			

											}
}


void show_reactivation_box(char *im_url) {
	char secimage[96];
	char *secptr=NULL;
	char *secim=NULL;

	GdkPixbuf *imbuf=NULL;
	GdkPixbuf *imbuf2=NULL;
	char *dwl_image=NULL;
	char *react_im=NULL;
	int height=0;
	int width=0;
	dwl_image=download_image(im_url);
	if (! dwl_image) {return;}
	react_im=strdup(dwl_image);
	if (strlen(react_im)<2) {free(react_im); return;}
	imbuf=gdk_pixbuf_new_from_file(react_im, NULL);
	if (!imbuf) {free(react_im); return;}

	unlink(react_im);

	secptr=strrchr(im_url,'/');
	if (!secptr) {free(react_im); return;}
	snprintf(secimage, 94, "%s", secptr+1);
	secptr=strrchr(secimage,'.');
	if (secptr) {*secptr='\0';}
	secim=strdup(secimage);
	set_account_unlocker_image(secim);
	free(secim);

	width = gdk_pixbuf_get_width( imbuf );
	height = gdk_pixbuf_get_height( imbuf );


	if (width>450) {
		height = ( 1.0 * height / width ) * 450;
		width = 450;
		}
	if (height>150) {height=150;}

	imbuf2=gdk_pixbuf_scale_simple( imbuf, width, height,
				GDK_INTERP_BILINEAR );
	if (!imbuf2) {g_object_unref(imbuf); free(react_im); return;}

	if (! login_window)  {login_window = build_login_window();}
	show_credentials_box(login_window, imbuf2);
	gtk_widget_show_all( login_window );
	gtk_window_set_position (GTK_WINDOW (login_window), GTK_WIN_POS_CENTER);
	g_object_unref(imbuf);
	g_object_unref(imbuf2);
	free(react_im);
}

void handle_login() {
			if ( strcmp( ymsg_field( "66" ), "" ))  {  
				/* added PhrozenSmoke: handle login status messages, bad password, etc. */
				int bad_login=0;
				int bad_password=0;
				int invalid_user=0;
				int reactivation_avail=0;

				snprintf(buf, 4, "%s", "");

				if (! strcmp( ymsg_field( "66" ), "13" ))  {  /* bad password */
					bad_login=1;
					snprintf(buf, 256, "  \n%s%s%s : %s",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("LOGIN FAILED!"), YAHOO_COLOR_BLUE);
					strncat(buf, _("INCORRECT PASSWORD"), 60);
					strcat(buf,"\n");
					bad_password=1;
											   }


				if (strstr( ymsg_field( "66" ), "Invalid User" ) || 
					(! strcmp( ymsg_field( "66" ), "3" )) )  {  /* Invalid User */
					bad_login=1;
					snprintf(buf, 256, "  \n%s%s%s : %s",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("LOGIN FAILED!"), YAHOO_COLOR_BLUE);
					strncat(buf, _("Invalid User."), 60);
					strcat(buf,"\n");
					bad_password=1;
					invalid_user=1;
											   }

				if (! strcmp( ymsg_field( "66" ), "99" ))  {  /* dup. logins */
					bad_login=1;
					snprintf(buf, 256, "  \n%s%s%s : %s",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("LOGIN FAILED!"), YAHOO_COLOR_BLUE);
					strncat(buf, _("DUPLICATE YAHOO! SCREEN NAMES LOGGED IN"), 256);
					strcat(buf,"\n");
											   }

				if (  (! strcmp( ymsg_field( "66" ), "14" )) ||  
					(! strcmp( ymsg_field( "66" ), "29" )) )  {  /* account locked */
					bad_login=1;
					snprintf(buf, 256, "  \n%s%s%s : %s",
						YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("LOGIN FAILED!"), YAHOO_COLOR_BLUE);
					strncat(buf, _("YOUR YAHOO! ACCOUNT HAS BEEN LOCKED BECAUSE OF TOO MANY FAILED LOGIN ATTEMPTS"), 256);
					strncat(buf,"  [http://login.yahoo.com]\n", 80);

					if ( strcmp( ymsg_field( "20" ), "" )){  /* reactivation URL */
						strncpy(tmp2, ymsg_field( "20" ), 512);
						if (strstr(tmp2,".jpg") || strstr(tmp2,".png") || strstr(tmp2,".gif") ) {
							reactivation_avail=1;
							/* Yahoo gave us a URL to an image we can display and
								type whatever 'verification' word is in the image */							
						} else {
							strcat(buf,YAHOO_COLOR_BLACK);
							strncat(buf,"Please visit the following site for information about fixing the problem", 128);
							strcat(buf,": ");
							strncat(buf,ymsg_field( "20" ), 450);
							strcat(buf,"\n");
								}
											}

											 	}
				

					if (bad_login==1)  {
						snprintf( ymsg_sess->error_msg, 127, "%s: %s", _("Could not connect to Yahoo!"), invalid_user?_("Invalid User."):_("Incorrect password.") );
						append_to_textbox( chat_window, NULL, buf );
						handle_dead_connection (0,1,0, invalid_user?_("Invalid User."):_("Incorrect password."));
						if (reactivation_avail) {show_reactivation_box(tmp2);}
								   			}

		snprintf(buf, 8, "%s", "");
			}
}



void handle_p2pask() {
			if ( strcmp( ymsg_field( "16" ), "" )) {
				if (strlen(ymsg_field( "16" ))>1) {
					append_timestamp(chat_window, NULL);
					snprintf(buf, 512, "   %s** Cannot check to see if user is online (P2P Ask Error) - This generally means the user IS online but has you blocked, not on their buddy list, or on their 'ignore' list: '%s' **%s\n", YAHOO_COLOR_RED, _utf(ymsg_field( "16" )), YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
				}
				return; 
			}

			if ( strcmp( ymsg_field( "12" ), "" )) {
				if (! strcmp(ymsg_field( "12" ), "P33RtoPE3Rchck==" )) {
					if (strcmp(ymsg_field( "11" ), "1487248657")==0) {
							/* bounced back, they are offline */ 
					char *typptr=NULL;
					strncpy( tmp2, ymsg_field( "4"), 125);
					if (strlen(tmp2)<1) {return; }
					typptr=strchr(tmp2,',');  /* cut off if its comma-separated */
					if (typptr) {*typptr='\0';}
					append_timestamp(chat_window, NULL);
					snprintf(buf, 512, "   %s** The user appears to be OFFLINE (P2P Ask Fail): '%s' **%s\n", YAHOO_COLOR_BLUE, tmp2 , YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
					}
				return;
				}
			}

			if ( strcmp( ymsg_field( "4" ), "" )) {
				char *typptr=NULL;
					/* sometimes we get TWO values for
					the name in the '4' field, we'll take the first value */
				strncpy( tmp, ymsg_field( "4"), 125);	
				typptr=strchr(tmp,',');  /* cut off if its comma-separated */
				if (typptr) {*typptr='\0';}

				if (! from_valid_sender(tmp) ) {return;}
				if (is_flooder(tmp)) {return;}

				if (strlen(tmp)>1)  {
				  snprintf(buf, 256, "%s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("User")); 
				  strcat(buf, "'");
				  strncat(buf, tmp, 80);
				  strcat(buf, "' ");
				  strncat(buf, _("peer-to-peer capability check."), 80);
				  strcat(buf, " **");
				  strcat(buf, YAHOO_STYLE_ITALICOFF);
				  strcat(buf, "\n");
				  append_to_textbox( chat_window, NULL, buf );
				  	append_to_open_pms(tmp, buf,chat_timestamp_pm);
				  /* tell Yahoo we can accept p2p */

				 if (! find_friend(tmp)) {
					log_possible_flood_attack(tmp, 1,"peer-to-peer capability check.");	 					
					show_status_from_unknown_friend(tmp, "Check Buddy's Peer-to-Peer File Capabilities");
												}

				 collect_profile_replyto_name();   
				 ymsg_p2p_ack( ymsg_sess, tmp ); 
				 reset_profile_replyto_name();

				  if (find_friend(tmp))  	{
					if (! find_online_friend( tmp )) {
						/* status change for an invisible friend */
					append_timestamp(chat_window, NULL);
					snprintf(buf, 512, "%s%s**%s: ( %s ) '%s' **%s%s\n",
						YAHOO_STYLE_ITALICON, YAHOO_COLOR_BLUE, 
						_("Invisible Buddy"), tmp, _("peer-to-peer capability check."),
						YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK );
					append_to_textbox( chat_window, NULL, buf );
					set_buddy_status_full( tmp,_("peer-to-peer capability check."),1);
					update_buddy_clist();
															   } 

													}
											   }
																		 }

}



void handle_imvironments() {

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					if (ymsg_sess->pkt.type==YMSG_P2P)  {
						log_possible_flood_attack(tmp, 1,"IMVironment Setting" );	 
						if (is_flooder(tmp)) {return;}
																				}

				/* avoid repeated send attempts from idiots and spammers */
				accept_pm =  get_pm_perms(tmp);

						if (yalias_name) {g_free(yalias_name);}
						yalias_name=get_screenname_alias(tmp);

				if ((! accept_pm) && (ymsg_sess->pkt.type==YMSG_P2P))  {
					comm_block_notify(tmp, yalias_name, 
						"Blocked an IMVironment setting from",  
						"this user is not currently accepting IMVironment settings from you.", 
						0, 0);
					return ;
																								}  /* end accept_pm */

				if (strcmp( ymsg_field( "63" ), "" ))  {				

					strncpy(tmp3,ymsg_field( "63" ), 200);
					if ((! strstr( tmp3, ";0")) && (! strstr( tmp3, ";20"))) { return ;} /* invalid, possible boot code */

					if (!strncmp( tmp3, ";0", 2 )) {
						snprintf(buf, 512, "  %s** %s ( %s ) %s **%s\n",
						YAHOO_COLOR_ORANGE, tmp, yalias_name, 
						_("has disabled IMVironments"),
						YAHOO_COLOR_BLACK );
												     								} else {
						snprintf(buf, 512, "  %s** %s ( %s ) %s '%s' **%s\n",
						YAHOO_COLOR_ORANGE, tmp, yalias_name, 
						_("has enabled IMVironment"), _utf(tmp3),
						YAHOO_COLOR_BLACK );
																							 }

					if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
						int post_notice=0;
						char *tuxviron=NULL;						

						pm_sess = pm_lpackptr->data;
						align_pms_repyto(pm_sess);

								if ((ymsg_sess->pkt.type==YMSG_P2P) && enable_tuxvironments)  {
									check_for_invisible_friend(tmp);
									if ((! strncmp(tmp3, "photos", 6)) && plugins_yphotos_loaded()) {
										log_possible_flood_attack(tmp, 1,"Photo IMVironment" );
										if (is_flooder(tmp)) {return;}
										plugins_yphoto_handle_incoming(tmp, ymsg_field("14"), atoi(ymsg_field("13") ) );
										return;
									}
								}

						tuxviron=gtk_object_get_data(GTK_OBJECT(pm_sess->pm_window),"imv");	
						if (tuxviron) {
							if (strncmp( tmp3,tuxviron, 2 )) {post_notice=1;}
										  } else { if (strncmp( tmp3, ";0", 2 )) { post_notice=1; }  }


						if (post_notice) {
							append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );

							if (tuxviron) {free(tuxviron); }
							if (!strncmp( tmp3, ";0", 2 )) 				{
								gtk_object_set_data (GTK_OBJECT (pm_sess->pm_window), "imv",strdup(";0"));
								if (enable_tuxvironments) 		{
									if (ymsg_sess->pkt.type !=YMSG_P2P)  {
										log_possible_flood_attack(tmp, 1,"PM IMVironment Changed" );	
																						  	 }								
								set_tuxvironment (pm_sess->pm_text, pm_sess->pm_window, "TUXVIRONMENT_OFF");
																			}
																	  					 } else {
								gtk_object_set_data (GTK_OBJECT (pm_sess->pm_window), "imv",strdup(tmp3));
								if (enable_tuxvironments) 	{
									if (ymsg_sess->pkt.type !=YMSG_P2P)  {
										log_possible_flood_attack(tmp, 1,"PM IMVironment Changed" );	
																						  			 }	
								set_tuxvironment (pm_sess->pm_text, pm_sess->pm_window, tmp3);
																			}
																									}
											  }
														 										} else {
						if (ymsg_sess->pkt.type==YMSG_P2P) {
							append_to_textbox( chat_window, NULL, buf );
																				 }
						check_for_invisible_friend(tmp);
																     									 }

										     								} /* field 63 */

}


void handle_p2p_snoop() {

				  /* Somebody is trying to snoop for 'shared files' 
					added: PhrozenSmoke, I think this is useful information that somebody is 
					trying to snoop shared files
				  */
				  if ( strcmp( ymsg_field( "1" ), "" ) || strcmp( ymsg_field( "4" ), "" ) )  {
						char *tttmpp;

					strncpy( tmp, ymsg_field( "1"), 125);
					if (! strcmp( tmp, "" )) {strncpy( tmp, ymsg_field( "4"), 125); }

						tttmpp = strchr(tmp, ',');
						if(tttmpp) {*tttmpp = '\0';}

					log_possible_flood_attack(tmp, 2,"Peer-to-Peer File Snooping" );
					if (! from_valid_sender(tmp) ) {return;}	 
					if (is_flooder(tmp)) {return;}

					if (! strcmp( ymsg_field( "13" ), "9" ))  {  /* stopped snooping */
						snprintf(buf, 256, "%s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("User")); 
				  		strcat(buf, "'");
				 		strncat(buf, tmp, 80);
				  		strcat(buf, "' ");
				  		strncat(buf, _("has stopped trying to view your shared files (peer-to-peer)."),  128);
				  		strcat(buf, YAHOO_STYLE_ITALICOFF);
				  		strcat(buf, " **\n");
						append_to_textbox(chat_window,NULL,buf);
						append_to_open_pms(tmp, buf,chat_timestamp_pm);
						check_for_invisible_friend(tmp);
						return;
																		
																			}  else  { /* started snooping */ 
						snprintf(buf ,128, "%s%s** %s: ",YAHOO_STYLE_ITALICON, YAHOO_COLOR_PURPLE, _("User")); 
				  		strcat(buf, "'");
				 		strncat(buf, tmp, 80);
				  		strcat(buf, "' ");
				  		strncat(buf, _("is attempting to view your shared files (peer-to-peer)."), 128);
				  		strcat(buf, "  ");
				  		strncat(buf, _("[FEATURE NOT SUPPORTED]"), 64);
				  		strcat(buf, YAHOO_STYLE_ITALICOFF);
				  		strcat(buf, " **\n");
						append_to_textbox( chat_window, NULL, buf );
						append_to_open_pms(tmp, buf,chat_timestamp_pm);
						/* We don't support/allow shared file SNOOPING on GYach
						   Enhanced, punish them for being nosey */
						send_automated_response(tmp, yalias_name, "this user does not allow viewing of his/her shared files.");
						if ( (!find_friend(tmp))  && (!find_temporary_friend(tmp)) ) { preemptive_strike(tmp); }
						check_for_invisible_friend(tmp);
						return;
		 																					}	
																			}

}  /* end handle p2p snoop */




void handle_p2p_file()  {
				if (!strcmp(ymsg_field( "1"), "")) {
					if (!strcmp(ymsg_field( "4"), "")) {return;}
				} /* from who */
				if (!strcmp(ymsg_field( "20"), "")) {return;} /* dowload url */
				if (!strcmp(ymsg_field( "28"), "")) {return;} /* file size */
				if (!strcmp(ymsg_field( "27"), "")) {return;} /* short filename */
				if (!strcmp(ymsg_field( "53"), "")) {return;} /* long filename */

				strncpy( tmp, ymsg_field( "1"), 125);
				if (! strcmp( tmp, "" )) {strncpy( tmp, ymsg_field( "4"), 125); }

				strncpy(tmp3, ymsg_field( "14" ), 80);
				/* a file was sent to us */

				if (strlen(tmp)>0)  {
						char *tttmpp;
						tttmpp = strchr(tmp, ',');
						if(tttmpp) {*tttmpp = '\0';}

					log_possible_flood_attack(tmp, 2,"Peer-to-Peer File" );	 
					if (! from_valid_sender(tmp) ) {return;}
					if (is_flooder(tmp)) {return;}

					if (yalias_name) {g_free(yalias_name);}
					yalias_name=get_screenname_alias(tmp);

				accept_pm=0;

				check_for_invisible_friend(tmp);


				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room,
					If we got PM's blocked, we probably don't want files being 
					sent either.
				 */
				accept_pm =  get_pm_perms(tmp);

				if ( (! accept_pm) || allow_no_sent_files)  {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored file sent from"),  
						"this user is not currently accepting files from you.", 
						(! accept_pm)?1:0, 1);
					return ;
											 									 }

					if(strlen(tmp3)>0) {
						char *tttmp;
						tttmp = strchr(tmp3, '\006');
						if(tttmp) {*tttmp = '\0';}
												}

				/* Tell Yahoo we are behind a firewall, forcing the file to be resent using 
					the File Transfer system instead - easier to download, and safer 
					because we dont reveal our IP address to the other user */

				strncpy(tmp2,ymsg_field( "27" ) , 350);
				strncpy(buf,ymsg_field( "53" ) , 350);
				ymsg_p2p_redirect( ymsg_sess, tmp, tmp2, buf);

	  				snprintf(buf,6, "%s", "  ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_file , NULL);	

				  snprintf(buf ,1600, "\n  %s%s** %s: '%s' ( %s )  %s **\n   %s:  '%s'\n   %s:  %s %s\n   %s:  '%s'\n   %s:  %s\n%s%s\n\n",  YAHOO_STYLE_BOLDON, YAHOO_COLOR_PURPLE, _("User"), tmp, yalias_name, _("has sent you a file using peer-to-peer."),  _("File name"),  ymsg_field( "27" ) ,  _("File size"), ymsg_field( "28" ), _("bytes"),  _("Message"), tmp, _("Download") , ymsg_field( "20" ), _("Warning: You will probably be unable to download this peer-to-peer file.  GYach Enhanced will instruct Yahoo! to re-send you the file using the File Transfer system instead.  The re-sent file arrive within a few minutes, depending on the file size."),  YAHOO_STYLE_BOLDOFF); 


				  append_to_textbox( chat_window, NULL, buf );
				  play_sound_event(SOUND_EVENT_OTHER);
				  return; 
											   }

} /* end handle p2p file */



void handle_p2p_confirmation() {
	/*  PEER-TO-PEER confirmation ticket sent  */
				char *typptr=NULL;
				strncpy( tmp2, ymsg_field( "4"), 125);
				if (strlen(tmp2)<1) {return; }
				typptr=strchr(tmp2,',');  /* cut off if its comma-separated */
				if (typptr) {*typptr='\0';}
				if (! from_valid_sender(tmp2) ) {return;}
				log_possible_flood_attack(tmp2, 1,"peer-to-peer confirmation ticket.");	
				if (is_flooder(tmp2)) {return;}
				append_timestamp(chat_window, NULL);
				snprintf(buf, 512, "   %s** The user appears to be ONLINE (P2P Confirmation) : '%s' **%s\n",  YAHOO_COLOR_ORANGE, tmp2, YAHOO_COLOR_BLACK );
				append_to_textbox( chat_window, NULL, buf );
				append_to_open_pms(tmp2, buf,chat_timestamp_pm);
				check_for_invisible_friend(tmp2);
}



/*  Handles special features lile peer-to-peer and Imvironments, YMSG_P2P */

void handle_special_features() {
			if (! strcmp( ymsg_field( "49" ), "" )) {

			if ( strcmp( ymsg_field( "1" ), "" )) {
					log_possible_flood_attack(ymsg_field( "1" ), 2,"Unknown Special Feature" );	 
					if (is_flooder(tmp)) {return;}
																}
			if ( strcmp( ymsg_field( "4" ), "" )) {
					log_possible_flood_attack(ymsg_field( "4" ), 2,"Unknown Special Feature" );	 
					if (is_flooder(tmp)) {return;}
																}

			if (ymsg_sess->debug_packets) {
				printf("*** DEBUG: UNKNOWN SPECIAL FEATURE PACKET\nTYPE:  [%d]\nDATA:  %s\n",ymsg_sess->pkt.type, ymsg_sess->pkt.data);
				fflush(stdout);
			}
			return; 
									      }


			/*  PEER-TO-PEER confirmation ticket sent  */
			if ( strcmp( ymsg_field( "4" ), "" )  && 
			  (! strcmp( ymsg_field( "49" ), "PEERTOPEER" ))  ) 	 {
						handle_p2p_confirmation();
						return; 
			  }

			/* Some type of special setting: Imvironment, etc. */
			if ( strcmp( ymsg_field( "1" ), "" )  && strcmp( ymsg_field( "49" ), "" )) 
			 {
				strncpy( tmp, ymsg_field( "1"), 125);
				if (strlen(tmp)<1) {return; }

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

				/* IMVironment setting */
				if (strstr( ymsg_field( "49" ), "IMVIRONMENT" ))   {
						handle_imvironments();
						return;  /* imvironment */
											 											}  /* 'IMVIRONMENT' in field 49 */

			  }  /* end imvironment, fields 49 and 1  */



	if (strstr( ymsg_field( "49" ), "FILEXFER" ))   {

			/* a peer-to-peer file has been made 
			available to us or somebody is trying 
			to snoop us for shared files,  */

			if ( strcmp( ymsg_field( "1" ), "" )  && strcmp( ymsg_field( "20" ), "" )) 
				{
						collect_profile_replyto_name();  
						handle_p2p_file();
						reset_profile_replyto_name();
						return;
				}  else {						
						handle_p2p_snoop();
						return;
						  }   /* end peer-to-peer */
																 }  /* end 'FILEXFER  */


			if ( strcmp( ymsg_field( "1" ), "" )) {
					log_possible_flood_attack(ymsg_field( "1" ), 2,"Unknown Special Feature" );	 
					if (is_flooder(tmp)) {return;}
																}

			if (ymsg_sess->debug_packets) {
				printf("*** DEBUG: UNKNOWN SPECIAL FEATURE PACKET\nTYPE:  [%d]\nDATA:  %s\n",ymsg_sess->pkt.type, ymsg_sess->pkt.data);
				fflush(stdout);
				}

}  /* end handle_p2p */



void handle_yping()  {
	/* the official Yahoo client on windows does this: sends the 'passthrough'
           packet - may be useful later */

	if ( strcmp( ymsg_field( "143" ), "" ) && strcmp( ymsg_field( "144" ), "" ))  {
	ymsg_passthrough( ymsg_sess );
	if (!got_buddy_ignore_list) {
		got_buddy_ignore_list=1; 
		show_offline_messages();
		flooder_buddy_list_protect();
		start_chat_online();
		}
	if (emulate_ymsg6 && activate_profnames) {
		if (my_status != 12) { /* as long as we aren't invisible */
			activate_all_profile_names();
										}
								}
	 }
}



void handle_perm_igg() {

	if ( strcmp( ymsg_field( "0" ), "" ) && strcmp( ymsg_field( "13" ), "" ))  {
		strncpy(tmp,ymsg_field( "0" ),  255);

		if (atoi(ymsg_field( "13" ))==1) { /* add ignore */
			snprintf(buf, 512, "%s%s ** '%s' %s **%s%s\n", 
			YAHOO_COLOR_RED, YAHOO_STYLE_BOLDON, tmp, 
			_("added to permanent Yahoo ignore list"), YAHOO_COLOR_BLACK, 
			YAHOO_STYLE_BOLDOFF);
			append_to_textbox(chat_window,NULL,buf);

			if (! perm_igg_check(tmp))  {add_perm_igg(tmp);}
			if (! ignore_check(tmp)) {ignore_toggle(tmp);}
			update_perm_igg_clist();
			/* take our name off their buddy list */
			ymsg_reject_buddy(ymsg_sess, tmp);
			ymsg_reject_buddy(ymsg_sess, tmp);
		     						} 

		if (atoi(ymsg_field( "13" ))==2) { /* remove ignore */
			snprintf(buf, 512, "%s%s ** '%s' %s **%s%s\n", 
			YAHOO_COLOR_RED, YAHOO_STYLE_BOLDON, tmp, 
			_("removed from permanent Yahoo ignore list"), YAHOO_COLOR_BLACK, 
			YAHOO_STYLE_BOLDOFF);
			append_to_textbox(chat_window,NULL,buf);
			if (perm_igg_check(tmp))  {remove_perm_igg(tmp);}
			if (ignore_check(tmp)) {ignore_toggle(tmp);}
			update_perm_igg_clist();
		     						} 
	}

}


/* This is the first app in a long time to support Yahoo calendar events*/

void handle_calendar() {
	/* field 13 - status */
	/* field 21 - always '0' i think */
	/* field 20 - calendar url */
	/* field 14 - notes on the event (contains newlines chars) */
	/* Strangely enough, we also get sent calendar packets
	when we add notes to the Yahoo 'Notepad': These have 
	and empty field 14 and field 20 */

	int activate=0;
	append_timestamp(chat_window, NULL);
	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_buzz, NULL);	
	  				snprintf(buf,6, "%s", " \n");
	  				append_to_textbox( chat_window, NULL, buf );

	strncpy(tmp,ymsg_field( "20" ),355);
	strncpy(tmp2,ymsg_field( "14" ),512);
	strncpy(tmp3,ymsg_field( "14" ),255);

	if ( strcmp( tmp3, "" ))  { 
		char *tttmp;
		tttmp = strchr(tmp3, '\n');
		if(tttmp) {*tttmp = '\0';}
		tttmp = strchr(tmp3, '\r');
		if(tttmp) {*tttmp = '\0';}
								 }

	if (strcmp( tmp2, "" ) || strcmp( tmp, "" )) {activate=1;}

	/* Sometimes calendar events ask us to set our status to 'Busy'
	    we'll do this as long as we aren't invisible */
	if ((my_status != 12) && activate) {
		if (strcmp( ymsg_field( "13" ), "" )) {
			if (atoi(ymsg_field( "13" ))==1) {cmd_away( "2" );}
		}
							}

	if (! strcmp( tmp2, "" ) )  { 
		/* We got an empty calendar event that just asks us to check
			our calendar and set our status to Busy */
		strncpy(tmp2,_("Unknown Yahoo Calendar Event"), 90); 
		strncpy(tmp3,_("Unknown Yahoo Calendar Event"), 90); 
		strncpy(tmp,"http://calendar.yahoo.com", 90); 
								  }

	snprintf(buf, 2000, "%s%s*** %s ***\n*** %s ***\n%s\n\n%s%s : %s\n\n" ,
		YAHOO_STYLE_ITALICON, "\033[#FFBAA9m",
		_("Yahoo Calendar Event"), _utf(tmp3),_utf(tmp2),
		YAHOO_COLOR_PURPLE, _("You should check your Yahoo Calendar"), _utf(tmp)
		);
	append_to_textbox( chat_window, NULL, buf );

	if (activate) {
		snprintf(buf, 2000, "%s:\n%s\n\n%s\n\n%s", _("Yahoo Calendar Event"), _utf(tmp3), _("You should check your Yahoo Calendar"),  _("Please check the window named 'Chat'."));
		show_ok_dialog(buf);
		play_sound_event(SOUND_EVENT_BUZZ);  /* a nice alarm sound */
	} else {play_sound_event(SOUND_EVENT_OTHER);}
}


void clean_news_html() {
	char *newsptr=tmp3;
	char *divptr=NULL;

	/* Explanation: Yahoo gives us news alert HTML data that may look 
	like this: 

'<div style=position:relative;left:10;width:95%;background-color:white;font-family:arial;font-size:11px><a href="http://news.yahoo.com/"><img src="http://us.i1.yimg.com/us.yimg.com/i/mesg/alerts/news.gif" border=0 target="_new"></a><br>PARIS (AP) Anti-terrorist police take three suspects into custody in connection with French railway bomb threats&#x2C; police say.</div>'

	The GtkHtml-2 widget does not seem to handle the 'div' tag well,
	it causes broken links, etc., so it will be removed.  However, 
	we MUST keep the little Yahoo logo that Yahoo sends us in 
	the news alert:  I read the Terms of Service for Yahoo's news
	services, including for Messenger 'news alerts, and it is completely against
	their terms of service to remove the logo they send that gives
	them credit for the news publication, though they place no other
	restrictions on how their news data can be displayed.  So please
	do not remove their code that sends the 
	'http://us.i1.yimg.com/us.yimg.com/i/mesg/alerts/news.gif' image!
	*/
	
	snprintf(buf,1200,"<font size=\"3\" color=\"#000000\" family=\"arial\">%s", tmp3);
	if (!strncasecmp(newsptr,"<div",4)) {
		divptr=strchr(newsptr,'>');
		if (divptr) {divptr++; newsptr=divptr;}
		divptr=strstr(newsptr,"</div>");
		if (divptr) {*divptr='\0';}
		divptr=strstr(newsptr,"</DIV>");
		if (divptr) {*divptr='\0';}
		snprintf(buf,1200,"<font size=\"3\" color=\"#000000\" family=\"arial\">%s", newsptr);
	}

	/* support the newer versions of the News alerts at beta.alerts.yahoo.com */ 

	divptr=strstr(newsptr, "</TABLE>\n");
	if (! divptr) {divptr=strstr(newsptr, "</table>\n");}
	if (divptr) {
		int ycredspan=0;  /* credits for yahoo news in spanish? */ 

		if (ymsg_sess->cookie[0] && 
			( strstr(ymsg_sess->cookie,"lg=e1")  || strstr(ymsg_sess->cookie,"lg=mx")
			|| strstr(ymsg_sess->cookie,"lg=ar")  || strstr(ymsg_sess->cookie,"lg=es") )) {
				ycredspan=1;
			}

		divptr += strlen("</TABLE>\n");
		snprintf(buf,1200,"<font size=\"3\" color=\"#000000\" family=\"arial\"><a href=\"http://news.yahoo.com\"><img src=\"%s\" border=\"0\"></a><br>%s", 
			ycredspan?"http://us.i1.yimg.com/us.yimg.com/i/e1/news.gif":"http://us.yimg.com/i/mesg/alerts/news.gif",
			divptr);
		divptr=strstr(buf, "</BODY>");
		if (divptr) {*divptr='\0';}
		divptr=strstr(buf, "</body>");
		if (divptr) {*divptr='\0';}
	}

	strcat(buf,"</font>");
}


void handle_news_alert() {
	/* This is a new YMSG packet Yahoo started using that 
	sends very cool 'news bulletins' about topics of interest
	that can be configured by the user at alerts.yahoo.com,
	it's supported in the newer versions of Y! Messenger for 
	Windoze. Gyach Enhanced is the first to support it on Linux; 
	As a news-junkie, I love this feature...just had to add it.
	So far, the HTML data yahoo sends does not appear
	to include images or links to ads...cool, hope it stays that way */

	/* Packet Type: 0x12c,  Fields: 
		5-user (our name)
		4-service name (usually something like 'Yahoo! Alerts')
		14-msg (format: NewsAlertName\nHeadLine\nURL; field contains newline chars)
		81='2' (can be any number, the 'id' of the news alert setup at alerts.yahoo.com)
		82='u' (?)
		83 - News Headline, or 'alert name' if using the newer Alerts beta.alerts.yahoo.com 
		84='0' (?), 85  = 'n/u' (?), 86  = 'us' (country?)
		20=HTML Data string containing a description of the story and link(s)
		133 = '0' or some non-zero hash number, 
		131 = ''  or 'Edit Alerts', 
		132 = '' or URL for editing alerts
	*/  /* Obviously most importants fields are 4, 14, 83, and 20 */

	char newsurl[170];
	int have_edit_url=0;
	GtkWidget *ynews_alert_menu=NULL;

	/* Check for possibly invalid packets, just in case people 
	start trying to spoof these */
	if (! strcmp( ymsg_field( "4" ), "" )) { /* Field 4 is empty, check field 1 */
		if (! strcmp( ymsg_field( "1" ), "" )) {return;}  /* empty field */
		if (! strstr( ymsg_field( "1" ), "Yahoo!")) {return;}  /* Not from Yahoo */
	}

	/* Make sure this news alert isnt a duplicate: 
	Sometimes we can set up news 'filters' that overlap
	and cause the same article to be sent more than once,
	and in rapid succession, guard against this */


	snprintf(newsurl,130, "%s", ymsg_field( "83"));  /* Headline */
	packptr=strchr(ymsg_field( "14"), '\n');
	if (packptr) {
		snprintf(newsurl,130, "%s", packptr+1);  /* Headline */
		packptr=strchr(newsurl, '\n');
		if (packptr) {*packptr='\0';}
		packptr=strstr(newsurl, "abstract Id:");
		if (packptr) {*packptr='\0';}
		packptr=strstr(newsurl, " Id: ");
		if (packptr) {*packptr='\0';}
	}


	/* Clip out headlines for 'abstract' news alerts like...
			more details or search for this story on <a 					href="http://news.search.yahoo.com/search/news/?fr=sfp&ei=UTF-8&p=MOSUL&#44; 					Iraq (AP) Police say a car bomb has exploded near an Iraqi National Guard patrol in the northeastern city of Mosul.">Yahoo! News Search</a>
	*/

	if (! strncasecmp(newsurl,"abstract",8)) {
		packptr=strstr(ymsg_field( "20"), "news.search.yahoo.com/search/news/?");
		if (packptr) {
			char *abs_tr=NULL;
			abs_tr=strstr(packptr, "&p=");
			if (abs_tr) {
				snprintf(newsurl,130, "%s", abs_tr+3);  /* Abstract Headline */
				packptr=strstr(newsurl, "\">");
				if (packptr) {*packptr='\0';}
			}
		}
	}


	if (! strcmp(newsurl,"")) {snprintf(newsurl,130, "%s", ymsg_field( "83"));}

	if (last_news_alert) {
		/* Same as the last news alert, so ignore */
		if (! strcasecmp(last_news_alert, newsurl)) {return;}
		else {free(last_news_alert);}
	}
	last_news_alert=strdup(newsurl);  /* remember last headline */


	/* now clip it */ 
	newsurl[85]='\0';

	if (strncasecmp(newsurl,ymsg_field( "83"), strlen(newsurl) )) {
		strcat(newsurl," [");
		strncat(newsurl, ymsg_field( "83"), 20);
		strcat(newsurl,"]");
	}

	/* Take out shit that sometimes appears in the headlines like '&#x2C;' tags */
	strip_html_tags(newsurl);

	ynews_alert_menu=gtk_object_get_data (GTK_OBJECT (chat_window), "ynews_alert_menu");

	/* Set status if requested */ 
	if ((ynews_alert_menu) && (GTK_CHECK_MENU_ITEM(ynews_alert_menu)->active)) {
		if ( (my_status==0) || (my_status==10) ) {
			time_t time_llnow = time(NULL);
			snprintf(tmp2,140,"%s", last_news_alert?last_news_alert:newsurl ); 
			strcat(tmp2,"...");
			snprintf(buf, 250, "Yahoo News Alert: '%s'  (%s)  http://news.yahoo.com", 
			tmp2, ctime(&time_llnow) );
			cmd_away(buf);
		}
	}

	snprintf(tmp2,100,"%s", newsurl ); 
	strcat(tmp2,"...");


	strncpy(tmp3, ymsg_field( "14"), 512);  /* msg blurb */
	packptr=strstr(tmp3, " Id: ");
	if (packptr) {*packptr='\0';}

	if (strstr( ymsg_field( "4" ), "Yahoo!")) {
		strncpy(tmp, ymsg_field( "4"), 75);  /* Service name */
	} else {
		strncpy(tmp, ymsg_field( "1"), 75);  /* Service name */
		 }

	append_timestamp(chat_window, NULL);

	  				snprintf(buf,6, "%s", " ");
	  				append_to_textbox( chat_window, NULL, buf );
	  				append_char_pixmap_text((const char**)pixmap_pm_buzz, NULL);	
	  				snprintf(buf,6, "%s", " \n");
	  				append_to_textbox( chat_window, NULL, buf );

	snprintf(buf, 640, "  %s%s*** %s  ( %s, \"%s\" ) ***\n  *** '%s' ***\n     " ,
		YAHOO_STYLE_ITALICON, "\033[#C4753Em",
		_("News Bulletin"), tmp, ymsg_field( "83"), tmp2	);

	packptr = tmp3;
	last = 0;
	sprintf(newsurl,"%s",""); /* clear the data */

	while( packptr ) {
		end = strchr( packptr, '\n' );
		if ( end ) {*end = '\0';} 
		else {last = 1;	}
		strncat(buf, _utf(packptr), 175 );  /* avoid buffer overflows */
		strcat(buf, "\n     ");
		if (!strncasecmp(packptr,"http://",7)) {snprintf(newsurl,168, "%s",packptr);}
		if ( last ) {packptr = NULL;} 
		else {packptr = end + 1;}
		if (strlen(buf)>1024) {break;} /* avoid buffer overflows */
							}  // end while

	/* Sometimes Yahoo provides no link URL in field 14, meaning
	they want the link to point to their news front page */
	if ( (strlen(newsurl)<2) || 
	  (!strstr(newsurl,"http://")) ) {snprintf(newsurl,168, "%s","http://news.yahoo.com");}

	strcat(buf,"\033[#CDB8ACm");
	strcat(buf,"[ ");
	strncat(buf,_("See the 'My Yahoo' window for more information."), 125);
	strcat(buf," ] \n\n");
	strcat(buf,YAHOO_STYLE_ITALICOFF);
	strcat(buf,YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, buf ); /* to chat window */
	if ( strcmp( ymsg_field( "20" ), "" )) {  /* HTML data msg blurb */
		strncpy(tmp3,ymsg_field( "20" ) , 1600);  /* must put this in tmp3 */
		clean_news_html();

		if ( strcmp( ymsg_field( "132" ), "" )) { 
			if (strstr(ymsg_field( "132" ), "http://")) {have_edit_url=1;}
		}

		snprintf(tmp3, 3000, "<b><font size=\"4\"  color=\"#660099\">%s : '%s'</font></b><br><font size=\"3\"  color=\"#000000\"><a href=\"%s\">%s</a></font><br> &nbsp;<br>%s<br><br><font size=\"3\"  color=\"#000000\"><b><a href=\"%s\">%s</a></b>  &nbsp; &nbsp; <a href=\"%s\">%s</a></font>",
		tmp,tmp2, newsurl, newsurl, buf, newsurl, "MORE...",
		have_edit_url?ymsg_field( "132" ):"http://alerts.yahoo.com", 
		_(" My News Alerts ")
		);
		/* Display the HTML data on the 'My Yahoo' component */
		set_myyahoo_html_data(_utf(tmp3));
	}
	play_sound_event(SOUND_EVENT_OTHER);	
	if (popup_news_bulletin) {
		snprintf(tmp3, 255, "%s :\n'%s", tmp, tmp2);
		strcat(tmp3,"'");
		/* optional popup message */
		show_popup_dialog(_("News Bulletin"),_utf( tmp3),  7);
		}
	snprintf(tmp3, 255, "%s: '%s", tmp, tmp2);
	strcat(tmp3,"'"); 
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,_utf(tmp3)); 
}



void handle_encryption_notify() {
	int located_pms=0;
	char *typptr=NULL;
	typptr=strchr(tmp,',');  /* cut off if its comma-separated */
	if (typptr) {*typptr='\0';}

				/* The same person shouldnt be sending this over and over
					but its a relatively minor threat */

					log_possible_flood_attack(tmp, 1,"Encryption change");	 
					if (is_flooder(tmp)) {return;}

	/* possible spoofs: Messages from ourselves, so ignore */
		if (! from_valid_sender(tmp) ) {return;}

					if (! get_pm_perms(tmp)) {
						ymsg_encryption_notify(ymsg_sess, tmp, -1);
						send_automated_response(tmp, tmp, "this user is not currently accepting encrypted instant messages from you.");
						return;
					}

					if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
						pm_sess = pm_lpackptr->data;
						located_pms=1;
					}

					strncpy( tmp3, ymsg_field( "14"), 512);

					if (!strcmp(tmp2, ENCRYPTION_START_GPGME))  {  
								/* set GPG passphrase */
						if (located_pms)  {
					gtk_widget_set_sensitive(GTK_WIDGET(				gtk_object_get_data(GTK_OBJECT(pm_sess->pm_window),"profwidget" )), FALSE);
								pm_sess->their_gpg_passphrase=atoi(tmp3);
								if (pm_sess->my_gpg_passphrase<0) {																								pm_sess->my_gpg_passphrase = make_gpgme_passphrase_nmbr();
								   ymsg_encryption_gpgme_notify(ymsg_sess, tmp, pm_sess->my_gpg_passphrase);
																								}
													}
					return;
					}


					if (!strcmp(tmp2, ENCRYPTION_OFF))  {  /* request to turn encryption off */
						if (located_pms)  {
							align_pms_repyto(pm_sess);
							if (pm_sess->encrypted_myway) {
									pm_sess->encrypted_myway=0;
									pm_sess->encrypted_theirway=0;
									pm_sess->my_gpg_passphrase=-1;
									pm_sess->their_gpg_passphrase=-1;

								append_char_pixmap_text((const char**)pixmap_pm_encrypt, 
									pm_sess->pm_text);	
									snprintf(buf, 512, "   %s** '%s' %s %s  [%s] **%s\n",
										YAHOO_COLOR_BLUE, tmp,_("has stopped encrypting the instant message session."), _("Encrypted session stopped."), 	get_encryption_description(pm_sess->encryption_type), YAHOO_COLOR_BLACK );
									pm_sess->encryption_type=0;
									append_to_textbox_color( pm_sess->pm_window,
										pm_sess->pm_text, buf );	
									ymsg_encryption_notify(ymsg_sess, tmp, 0);
									return ;
							}	else {pm_sess->encrypted_theirway=0;}
						}
					return;
					}

					if (!strcmp(tmp2, ENCRYPTION_UNAVAIL))  {  /* user cannot */
						if (located_pms)  {
							align_pms_repyto(pm_sess);
							if (pm_sess->encrypted_myway) {
									pm_sess->encrypted_myway=0;
									pm_sess->encrypted_theirway=0;
									pm_sess->my_gpg_passphrase=-1;
									pm_sess->their_gpg_passphrase=-1;

								append_char_pixmap_text((const char**)pixmap_pm_encrypt, 
									pm_sess->pm_text);	
									snprintf(buf, 1024, "   %s** '%s' %s %s  [%s] **%s\n",
										YAHOO_COLOR_BLUE, tmp,_("does not have support for the requested encryption."),_("Encrypted session stopped."), get_encryption_description(pm_sess->encryption_type), YAHOO_COLOR_BLACK );
									pm_sess->encryption_type=0;
									append_to_textbox_color( pm_sess->pm_window,
										pm_sess->pm_text, buf );
									return ;
							}	else {pm_sess->encrypted_theirway=0;}
						}
					return;
					}

					if (!strcmp(tmp2, ENCRYPTION_START_TAG))  {  /* start encryption */
						if (!located_pms) {
							/* see if we are allowed to start a PM window */
							if (! pm_in_sep_windows)  {
								/* The user doesnt like PM windows, so disable encryption */
								ymsg_encryption_notify(ymsg_sess, tmp, 0);
								send_automated_response(tmp, tmp, "this user does not have support for encrypted messaging currently enabled.");
								return; 
							} else {
							pm_sess=new_pm_session(tmp);
							located_pms=1;
							gtk_widget_show_all( pm_sess->pm_window );							
									  }
						}

						if (located_pms)  {
							align_pms_repyto(pm_sess);
							if (pm_sess->encryption_type != atoi(tmp3)) {
									char *endesc=get_encryption_description(atoi(tmp3));
									int enavail=encryption_type_available(atoi(tmp3));

									if (!enable_encryption) {enavail=0;}

									/* Encryption started or changed */
									pm_sess->encrypted_myway=0;  /* reset our side */
									pm_sess->encrypted_theirway=1;
									if (enavail) {acknowledge_encryption(pm_sess, atoi(tmp3));}
									else {acknowledge_encryption(pm_sess, -1);}

									if (enavail) {
								append_char_pixmap_text((const char**)pixmap_pm_encrypt, 
									pm_sess->pm_text);	
									snprintf(buf, 512, "   %s** '%s' %s %s [%s] **%s\n",
										YAHOO_COLOR_GREEN, tmp,
										_("has enabled encrypted instant messaging."),_("Encrypted session started.") , endesc, YAHOO_COLOR_BLACK );
									pm_sess->encrypted_myway=1;
									pm_sess->encryption_type=atoi(tmp3);

						/* It's important that we lock our profile identity in place
							once we start using encryption stuff, otherwise 
							encryption/decryption can fail for some algorithms */					
					gtk_widget_set_sensitive(GTK_WIDGET(				gtk_object_get_data(GTK_OBJECT(pm_sess->pm_window),"profwidget" )), FALSE);

								if (pm_sess->encryption_type==41) { 
								if (pm_sess->my_gpg_passphrase<0) {																								pm_sess->my_gpg_passphrase = make_gpgme_passphrase_nmbr();
								   ymsg_encryption_gpgme_notify(ymsg_sess, tmp, pm_sess->my_gpg_passphrase);
																								}
																		}

													} else {
								append_char_pixmap_text((const char**)pixmap_pm_encrypt, 
									pm_sess->pm_text);	
									snprintf(buf, 512, "   %s** '%s' %s  [%s]  - %s %s **%s\n",
										YAHOO_COLOR_RED, tmp,
										_("attempted to start an encrypted instant messaging session"),endesc, _("Your system does not support this encryption or you have encryption disabled."), _("Encrypted session stopped."), YAHOO_COLOR_BLACK );
									pm_sess->encrypted_myway=0;
									pm_sess->encryption_type=0;
									pm_sess->encrypted_theirway=0;
															  }
									append_to_textbox_color( pm_sess->pm_window,
										pm_sess->pm_text, buf );
									if (!enavail) {send_automated_response(tmp, tmp, "this user does not have support for encrypted messaging currently enabled.");}
									return ;
							}	else {
								/* got confirmation from the other user */

								char *endesc=get_encryption_description(pm_sess->encryption_type);
								pm_sess->encrypted_theirway=1;
								pm_sess->encrypted_myway=1;
								append_char_pixmap_text((const char**)pixmap_pm_encrypt, 
									pm_sess->pm_text);	
								snprintf(buf, 512, "   %s** '%s' %s %s [%s] **%s\n",
										YAHOO_COLOR_ORANGE, tmp,
										_("has enabled encrypted instant messaging."), _("Encrypted session started."), endesc, YAHOO_COLOR_BLACK );
									append_to_textbox_color( pm_sess->pm_window,
										pm_sess->pm_text, buf );

								if (pm_sess->encryption_type==41) { /* send GPG passphrase num */
								   pm_sess->my_gpg_passphrase = make_gpgme_passphrase_nmbr();
								   ymsg_encryption_gpgme_notify(ymsg_sess, tmp, pm_sess->my_gpg_passphrase);
																							}

										}
						}
					return;
					}

}


void handle_group_rename() {
	if (!strcmp(ymsg_field( "67"), "")) {return;}
	if (!strcmp(ymsg_field( "65"), "")) {return;}
	strncpy(tmp,ymsg_field( "65"), 80);  /* new name */
	strncpy(tmp2,ymsg_field( "67"), 80);  /* old name */
	rename_buddy_group(tmp2, tmp);
	update_buddy_clist();
}

void handle_game_invite() {
	char game_name[24]="";
	int game_stat=0;
	if (!strcmp(ymsg_field( "4"), "")) {return;}  /* from */
	if (!strcmp(ymsg_field( "5"), "")) {return;} /* To */
	if (!strcmp(ymsg_field( "180"), "")) {return;}  /* game name */
	if (!strcmp(ymsg_field( "13"), "")) {return;} 
		/* 13-stats: 1-invite, 2-accept, 3-decline, 4-cancelled */

	/* Gotta do more here, for now protect against flooding and boot attempts */
	strncpy(tmp, ymsg_field("4"), 80);
	log_possible_flood_attack(tmp, 2,"Blocked Game Invitation");
	/* possible spoofs: Messages from ourselves, so ignore */
	if (! from_valid_sender(tmp) ) {return;}
	if (is_flooder(tmp)) {return;}
	check_for_invisible_friend(tmp);
	if (!emulate_ymsg6) {return;}

	yalias_name=get_screenname_alias(tmp);
	if ((! get_pm_perms(tmp)) || auto_reject_invitations) {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored Game Invitation from user"),  
						"this user is not currently accepting game invitations from you." , 
						1, 1);
		ymsg_game_invite(ymsg_sess, tmp, ymsg_field( "180"), 3); /* auto decline */
		return;
	}

if ( (find_temporary_friend(tmp) ) || (find_friend(tmp)) ) {
	set_last_comment( tmp, "Regular Y! Game Invitation" );
}

	game_stat=atoi(ymsg_field( "13"));
	if (!game_stat) {return;}  /* a status of zero? never seen it, unknown meaning */
	if (game_stat>4) {return;}  /* a status more than 4? never seen it, unknown meaning */

	strncpy(tmp2, ymsg_field("180"), 10); /* game name */
	strncpy(tmp3, ymsg_field("11"), 60);  /* invite cookie if it exists */
	
	snprintf(game_name,22,"%s", "Unknown Game");
	if (!strcasecmp(tmp2,"do")) {snprintf(game_name,22,"%s", "Dominoes");}
	if (!strcasecmp(tmp2,"ch")) {snprintf(game_name,22,"%s", "Chess");}
	if (!strcasecmp(tmp2,"ck")) {snprintf(game_name,22,"%s", "Checkers");}
	if (!strcasecmp(tmp2,"ww")) {snprintf(game_name,22,"%s", "Word Racer");}
	if (!strcasecmp(tmp2,"ph")) {snprintf(game_name,22,"%s", "Pearl Hunter");}
	if (!strcasecmp(tmp2,"bg")) {snprintf(game_name,22,"%s", "Backgammon");}
	if (!strcasecmp(tmp2,"ttb")) {snprintf(game_name,22,"%s", "Toki Toki Boom");}
	if (!strcasecmp(tmp2,"pl")) {snprintf(game_name,22,"%s", "Pool");}
	if (!strcasecmp(tmp2,"lt")) {snprintf(game_name,22,"%s", "Literati");}

	  		snprintf(buf,6, "%s", " ");
	  		append_to_textbox( chat_window, NULL, buf );
	  		append_char_pixmap_text((const char**)pixmap_games, NULL);	

	snprintf(buf, 512, "  %s** '%s'  ( %s ) : %s  [%s] **%s\n",
		YAHOO_COLOR_PURPLE, tmp,yalias_name, _("Has sent you a Game Invitation"), game_name, YAHOO_COLOR_BLACK );
	if (game_stat==2) {
		snprintf(buf, 512, "  %s** '%s'  ( %s ) : %s  [%s] **%s\n",
		YAHOO_COLOR_PURPLE, tmp,yalias_name, _("Has accepted your Game Invitation"), game_name, YAHOO_COLOR_BLACK );
	}
	if (game_stat==3) {
		snprintf(buf, 512, "  %s** '%s'  ( %s ) : %s  [%s] **%s\n",
		YAHOO_COLOR_PURPLE, tmp,yalias_name, _("Has declined your Game Invitation"), game_name, YAHOO_COLOR_BLACK );
	}
	if (game_stat==4) {
		snprintf(buf, 512, "  %s** '%s'  ( %s ) : %s  [%s] **%s\n",
		YAHOO_COLOR_PURPLE, tmp,yalias_name, _("Has cancelled the Game Invitation"), game_name, YAHOO_COLOR_BLACK );
	}

	if ( (game_stat==1) || (game_stat==2) ) {
		strcat(buf,YAHOO_COLOR_RED);
		strncat(buf,_("This feature is not yet supported by Gyach-E. If you wish to join this user in the game room, login to http://games.yahoo.com."), 350);
		strcat(buf,YAHOO_COLOR_BLACK);
		strcat(buf,"\n");
	}

	if ( chat_timestamp_pm ) {append_timestamp(chat_window, NULL);	}
	append_to_textbox( chat_window, NULL, buf);
	append_to_open_pms(tmp, buf,chat_timestamp_pm);
	play_sound_event(SOUND_EVENT_OTHER);
}


void handle_audible() {
	GtkWidget *t_widget;
	int is_spam=0;
	int to_the_pm=0;
	int unknown_aud=0;
	char *myalias=NULL;

	if (!strcmp(ymsg_field( "4"), "")) {return;}
	if (!strcmp(ymsg_field( "5"), "")) {return;}
	if (!strcmp(ymsg_field( "230"), "")) {return;}
	if (!strcmp(ymsg_field( "231"), "")) {return;}

	strncpy(tmp, ymsg_field("4"), 80);

	log_possible_flood_attack(tmp, 2,"Blocked Y! Audible Animation");
	if (is_flooder(tmp)) {return;}
	check_for_invisible_friend(tmp);
	if (!emulate_ymsg6) {return;}
	if (! enable_audibles) {return;}
	/* possible spoofs: Messages from ourselves, so ignore */
	if (! from_valid_sender(tmp) ) {return;}

	yalias_name=get_screenname_alias(tmp);
	accept_pm=0;
	accept_pm =  get_pm_perms(tmp);

	if (! accept_pm) {
					comm_block_notify(tmp, yalias_name, 
						_("Ignored Audible Animation from user"),  
						"this user is not currently accepting 'Audibles' from you." , 
						1, 1);
		return;
	}

	strncpy(tmp2, ymsg_field("231"), 250);  /* msg, limit to avoid possible boot codes */
	strncpy(tmp3, ymsg_field("230"), 150); /* file name - base.us.flirts.... etc.   */

	if ( enable_chat_spam_filter && strcasecmp( tmp, ymsg_sess->user )
		&& (! find_profile_name(tmp)) 
		&& (! find_friend(tmp))  && (! find_temporary_friend(tmp)) ) {
		if ( (is_chat_spam(tmp, tmp2)  || is_spam_name(tmp) ) )  {is_spam=1;}
											  															}

	if (is_spam) {
		/* I'm willing to bet people will start using field '231' of the 
			Audibles packet to send Spam - protect against this */ 
		snprintf( tmp3,512,  "%s  [Spam] ", _("Ignored Audible Animation from user") );
					comm_block_notify(tmp, yalias_name, 
						tmp3,  
						"this user is not currently accepting 'Audibles' from you." , 
						1, 1);

		if ((find_user_row(tmp) >= 0) &&  test_automute(tmp)) {
			/* If spammer is in chat room, shut them up */
			mute_toggle(tmp);
			snprintf(buf, 300, "  Gyach-E - auto-muted %s '%s'  ( %s ) %s : appears to be a spam bot [Audible comment].\n", YAHOO_COLOR_OLIVE, tmp , yalias_name, YAHOO_COLOR_BLACK );
			append_to_textbox( chat_window, NULL, buf );
		}	
		return;
	}

if ( (find_temporary_friend(tmp) ) || (find_friend(tmp)) ) {
	set_last_comment( tmp, "Regular Y! Audible Animation" );
}

	control_msg_line_feeds();

	if (( pm_in_sep_windows ) && (! mute_check( tmp ))) {
	 to_the_pm=1;

	if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
		pm_sess = pm_lpackptr->data;
		} else {
		/* open a new window if one doesn't exist already */
		pm_sess=new_pm_session(tmp);
		gtk_widget_show_all( pm_sess->pm_window );
		}

		
		align_pms_repyto(pm_sess);		set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

		if ( auto_raise_pm ) {
				gdk_window_raise( pm_sess->pm_window->window );
				focus_pm_entry(pm_sess->pm_window);
		}
		t_widget=gtk_object_get_data (GTK_OBJECT (pm_sess->pm_window), "pms_status");	
		if (t_widget) {
				gtk_label_set_text(GTK_LABEL(t_widget), "      ");
				gtk_widget_show_all(t_widget);
							}

	} 

	if ( chat_timestamp_pm ) {
		if (to_the_pm)  {append_timestamp(pm_sess->pm_window,pm_sess->pm_text);} 
		else {append_timestamp(chat_window, NULL);}
	}

	if (pm_nick_names) {myalias=get_screenname_alias(tmp);}

	snprintf( buf, 446, "%s%s%s%s%s  ",
			YAHOO_COLOR_PMPURPLE, YAHOO_STYLE_BOLDON, 
			myalias?myalias:tmp,
			YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );

	if (to_the_pm)  {
		append_to_textbox_color( pm_sess->pm_window,
							pm_sess->pm_text, buf );
	} else {
		append_to_textbox_color( chat_window, NULL, buf );
	}
	
	init_audibles();

	if (check_gy_audible(tmp3)) {
		struct stat audsbuf;
		char *audy_file=NULL;
		audy_file=get_gy_audible_disk_name( tmp3 );
		if (! audy_file) {unknown_aud=1;}
		else {
			snprintf(buf,350,"%s/audibles/%s.gif", PACKAGE_DATA_DIR, audy_file);
			if ( stat( buf, &audsbuf )) {unknown_aud=1; }       /* not on the disk */
		}
		if (! unknown_aud) {			
			if (to_the_pm)  { ct_append_pixmap(buf, pm_sess->pm_text);}
			else {ct_append_pixmap(buf, NULL);}
		}
		if (audy_file) {free(audy_file); audy_file=NULL;}
	} else {unknown_aud=1;}

	snprintf( buf, 446, " %s . o O ( %s ) %s  %s\n",
			"\033[#A7708Cm", tmp2,YAHOO_COLOR_BLACK, (unknown_aud || (! to_the_pm) )?_("[Audible]"):""  );
	if (to_the_pm)  {append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );}
	else {append_to_textbox_color( chat_window, NULL, buf );}

	if (! unknown_aud) {
		/* Play mp3 here */
		if (! mute_check( tmp )) {play_audible(tmp3);}
	}
	if (to_the_pm)  {
		show_incoming_pm(pm_sess, 1);
		reset_current_pm_profile_name();
	}
	if (myalias) {g_free(myalias); myalias=NULL;}

}


void handle_stealth() {
	int stealth_set=1;
	if (!emulate_ymsg6) {return;}
	if (!strcmp(ymsg_field("7"), "")) {return;}
	strncpy(tmp, ymsg_field("7"), 96);
	if (ymsg_sess->pkt.type==YMSG_STEALTH_PERM) {
		strncpy(tmp2, ymsg_field("31"), 10);
	} else {strncpy(tmp2, ymsg_field("31"), 10);}
	
	stealth_set=atoi(tmp2);
	if (find_temporary_friend(tmp) || find_friend(tmp)) {
		FRIEND_OBJECT=yahoo_friend_find(tmp);
		if (FRIEND_OBJECT) {
			if (ymsg_sess->pkt.type==YMSG_STEALTH_PERM) {
				if (stealth_set==1) {FRIEND_OBJECT->stealth=3;}
				else {FRIEND_OBJECT->stealth=0;}
			} else {
				if (stealth_set==2) {FRIEND_OBJECT->stealth=2;}
				else {FRIEND_OBJECT->stealth=1;}
			}
		update_buddy_clist();
		}
	}


	if (ymsg_sess->pkt.type==YMSG_STEALTH_PERM) {
		if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
	  		snprintf(buf,6, "%s", "   ");
	  		append_to_textbox( chat_window, NULL, buf );
			if (stealth_set==1) {
	  			append_char_pixmap_text((const char**)pixmap_stealth_off, NULL);
			} else {
	  			append_char_pixmap_text((const char**)pixmap_stealth_on, NULL);
			}	
		snprintf(buf, 768, " %s *** ' %s ' %s *** %s\n",
					(stealth_set==1)?YAHOO_COLOR_RED:YAHOO_COLOR_GREEN, tmp, 
					(stealth_set==1)?_("now sees you as PERMANENTLY offline"):_("no longer sees you as PERMANENTLY offline"), 
					YAHOO_COLOR_BLACK );		
		append_to_textbox(chat_window,NULL,buf);
		append_to_open_pms(tmp, buf, 1);
		play_sound_event(SOUND_EVENT_OTHER);
	} else {
		if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
	  		snprintf(buf,6, "%s", "   ");
	  		append_to_textbox( chat_window, NULL, buf );
			if (stealth_set==2) {
	  			append_char_pixmap_text((const char**)pixmap_stealth_off, NULL);
			} else {
	  			append_char_pixmap_text((const char**)pixmap_stealth_on, NULL);
			}	
		snprintf(buf, 768, " %s *** ' %s ' %s *** %s\n",
					(stealth_set==2)?YAHOO_COLOR_RED:YAHOO_COLOR_GREEN, tmp, 
					(stealth_set==2)?_("now sees you as offline"):_("now sees you as online"), 
					YAHOO_COLOR_BLACK );
		append_to_textbox(chat_window,NULL,buf);
		append_to_open_pms(tmp, buf, 1);
		play_sound_event(SOUND_EVENT_OTHER);
	}
	
}



void handle_avatar_toggled() {
	int avon=1;
	strncpy(tmp, ymsg_field("5"), 80); /* to */
	strncpy(tmp2, ymsg_field("4"), 80); /* from */
	strncpy(tmp3, ymsg_field("213"), 10); /* avatar on or off */

	if (strcmp(ymsg_field("66"), "")) {return;}  /* error */
	if (!emulate_ymsg6) {return;}
	if (is_flooder(tmp2)) {return;}

	/*  If we have a 'from' field '4', this is another user trying 
		to hide or show their avatar... we ignore this since users 
		can configure their own avatar settings in GyachE */
	avon=atoi(tmp3);
	if (strcmp(tmp2,"")) {
		check_for_invisible_friend(tmp2);
		if (! find_friend(tmp2)) {
			show_status_from_unknown_friend(tmp2, avon?"Buddy List Avatar Turned On":"Buddy List Avatar Turned Off");
		}
		return;
	}

	if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
	snprintf(buf, 640, "   %s** %s **%s\n",
		avon?YAHOO_COLOR_GREEN:YAHOO_COLOR_ORANGE, 
		avon?_("You are now sharing your buddy list avatar."):_("You are no longer sharing your buddy list avatar."), 
		YAHOO_COLOR_BLACK);
	append_to_textbox( chat_window, NULL, buf );
}


void handle_avatar_changed() {
	strncpy(tmp, ymsg_field("5"), 80); /* to */
	strncpy(tmp2, ymsg_field("4"), 80); /* from */

	if (!emulate_ymsg6) {return;}
	if (is_flooder(tmp2)) {return;}

	if (strcmp(tmp2,"")) {
			check_for_invisible_friend(tmp2);
		/*  If we have a field '4' an avatar from a buddy was updated */
		if ( (! find_friend(tmp2)) && (!find_profile_name(tmp2)) && 
			(strcasecmp(ymsg_sess->user, tmp2)) ) {
			show_status_from_unknown_friend(tmp2, "Buddy List Avatar Changed");
			return;
		}

		if ( (find_profile_name(tmp2))  || (!strcasecmp(ymsg_sess->user, tmp2)) )  {
			/* our own avatar */
				int av_deleted=0;
				if (! strcmp(ymsg_field("198"),"")) {return;}
				if (! strcmp(ymsg_field("198"),"0")) {av_deleted=1;} 

				
			if (strcmp(ymsg_field("197"),"") && (!  av_deleted) ) {
				if (my_avatar) {delete_yavatars(my_avatar); free(my_avatar); my_avatar=NULL;}	
				my_avatar=strdup(ymsg_field("197"));
			} else {
				/* avatar was deleted */
				if (my_avatar) {delete_yavatars(my_avatar); free(my_avatar); my_avatar=NULL;}	
			}
		if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
		snprintf(buf, 350, "   %s** %s **%s\n",YAHOO_COLOR_ORANGE, 
					av_deleted?_("Your avatar has been deleted."):_("Your avatar has been updated."), YAHOO_COLOR_BLACK);
		append_to_textbox( chat_window, NULL, buf );
	 }

		FRIEND_OBJECT=yahoo_friend_find(tmp2);
		if (FRIEND_OBJECT) {

			if (! strcmp(ymsg_field("198"),"0"))  {  /* avatar deleted */
				if (FRIEND_OBJECT->avatar) {
						delete_yavatars(FRIEND_OBJECT->avatar);
						g_free(FRIEND_OBJECT->avatar);
						FRIEND_OBJECT->avatar=NULL;

				if (show_yavatars) {
					update_buddy_clist();
					if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
					snprintf(buf, 350, "   %s** '%s': %s **%s\n",YAHOO_COLOR_ORANGE, 
						tmp2, _("The user has deleted their avatar."), YAHOO_COLOR_BLACK);
					append_to_textbox( chat_window, NULL, buf );
					append_to_open_pms(tmp2, buf,chat_timestamp_pm);
					}
				return ;
				}
			}

			if (strcmp(ymsg_field("197"),"") && (! strcmp(ymsg_field("198"),"1")) ) {
				if (FRIEND_OBJECT->avatar) {
					if (strcmp(FRIEND_OBJECT->avatar, ymsg_field("197"))) {
						delete_yavatars(FRIEND_OBJECT->avatar);
						g_free(FRIEND_OBJECT->avatar);
						FRIEND_OBJECT->avatar=g_strdup(ymsg_field("197"));
					} else {return;	}  /* same avatar as what we have */
				} else {FRIEND_OBJECT->avatar=g_strdup(ymsg_field("197")); } /* new avatar */
				
				if (show_yavatars) {
					update_buddy_clist();
					if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
					snprintf(buf, 350, "   %s** '%s': %s **%s\n",YAHOO_COLOR_ORANGE, 
						tmp2, _("The user has changed their avatar."), YAHOO_COLOR_BLACK);
					append_to_textbox( chat_window, NULL, buf );
					append_to_open_pms(tmp2, buf,chat_timestamp_pm);
					}
			}
		}
		return;
	}  else {  /* myself */
		/* No name in field '4', our own avatar was updated */  

				int av_deleted=0;
				if (! strcmp(ymsg_field("198"),"")) {return;}
				if (! strcmp(ymsg_field("198"),"0")) {av_deleted=1;} 

		/* whether we are sharing our own avatar */ 
		if (strcmp( ymsg_field( "212" ), "" ) ) {
			if (share_blist_avatar>1) {share_blist_avatar=1;}
			if (share_blist_avatar<0) {share_blist_avatar=0;}
			if ( atoi(ymsg_field( "212" )) != share_blist_avatar) {
				/* sync yahoo's settings with our own */
				ymsg_avatar_toggle(ymsg_sess, share_blist_avatar);
				}
			}

		if (my_avatar) {delete_yavatars(my_avatar);	free(my_avatar); my_avatar=NULL; }
		if (strcmp(ymsg_field("197"),"") && (! av_deleted)  ) {	
			my_avatar=strdup(ymsg_field("197"));
		}

		if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
		snprintf(buf, 350, "   %s** %s **%s\n",YAHOO_COLOR_ORANGE, 
					av_deleted?_("Your avatar has been deleted."):_("Your avatar has been updated."), YAHOO_COLOR_BLACK);
		append_to_textbox( chat_window, NULL, buf );

	} /* end else */
}

void handle_buddy_image_uploaded() {
	/* We uploaded a buddy image and Yahoo is sending us 
	the URL to share with others */


	if (strcmp(ymsg_field("66"), "")) {return;}  /* error */

	/* For now, we may be using the regular FileTransferSystem
	   until we work out problems sending the 'buddy image upload' packet 
	   to Yahoo's servers */

	if ( strcmp(ymsg_field("4"), "") ) {
		if (ymsg_sess->pkt.type != YMSG_FILETRANSFER) { return; }
		if (strcmp(ymsg_field("4"), "FILE_TRANSFER_SYSTEM")) {return;}
	}
	/* This is NOT from Yahoo's server, probably a spoof */
	/* NO field '4' 'from' tag should exist in this packet */

	/* This is NOT from Yahoo's server, probably a spoof */
	/* NO field '1' 'from' tag should exist in this packet */

	if ( strcmp(ymsg_field("1"), "") ) {return; }

	if (ymsg_sess->pkt.type == YMSG_BIMAGE_SEND) {
		if (! strcmp(ymsg_field("20"), "") ) {return; }  /* no user */
		if (! strstr(ymsg_field("20"), "friend_icon.png" ) ) {return; }  /* unknown file type */ 
		if (! strcmp(ymsg_field("5"), "") ) {return; }  /* from...us */
			/* check for spoofing */
		if ( strcasecmp(ymsg_field("5"), ymsg_sess->user)) {
			if (! find_profile_name(ymsg_field("5")) ) {return;}
		}
	}
 

	if (ymsg_sess->pkt.type == YMSG_FILETRANSFER) {
		char *starter=NULL;
		char *stopper=NULL;
			/* if we used the regular file server to send the buddy image, 
			our URL comes back in field 14 in a message, 
			The message looks like this: 
				'Your file has been sent and has also been uploaded to the following location: http://us.f1.yahoofs.com/msgr/phrozensmoke/.tmp/chgt.gif?msjk3pABl.x7L80w'
			 */

		if (!strcmp(ymsg_field("14"), "")) {return;}

		strncpy(tmp3, ymsg_field("14"), 384);  /* URL */
		starter=strstr(tmp3,"http");
		if (! starter) {return;}
		snprintf(tmp, 384, "%s", starter);
		stopper=strchr(tmp, '\n');
		if (stopper) {*stopper='\0';}
		stopper=strchr(tmp, '\r');
		if (stopper) {*stopper='\0';}
		snprintf(tmp2, 25, "%d", ((int) time(NULL))+86400);
	} else {   /* YMSG_BIMAGE_SEND  */ 
		strncpy(tmp, ymsg_field("20"), 384);  /* URL */
		strncpy(tmp2, ymsg_field("38"), 25);  /* expiration timestamp */
		}
	
	if ( (strcmp(tmp, "")) && (strcmp(tmp2, "")) ) {
		GList *this_session = pm_list;
			if (bimage_url) {free(bimage_url);}
			bimage_url=strdup(tmp);
			bimage_timestamp=atoi(tmp2);

			if (!bimage_hash) {
				char *checky=NULL;
				checky=strdup(tmp2);
				if (bimage_file) {
					char *checksum=NULL;
					checksum=get_pic_checksum(bimage_file);
					if (checksum) {
						free(checky);
						checky=strdup(checksum);
						free(checksum);
					} 
				}
				bimage_hash=strdup(checky);
				free(checky);
			}

			/* broadcast image change to all friends */
			ymsg_bimage_update(ymsg_sess, NULL, bimage_hash);
				snprintf(buf, 300, "   %s** '%s' **%s\n",YAHOO_COLOR_ORANGE, 
					 _("Your buddy image has been offered to the user."), YAHOO_COLOR_BLACK);

			while( this_session ) {   /* any open PM sessions */
				pm_sess = (PM_SESSION *)this_session->data;
				this_session = g_list_next( this_session );

				set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

				if (pm_sess->buddy_image_share && 
				  get_pm_perms(pm_sess->pm_user) ) {					
					ymsg_bimage_update(ymsg_sess, pm_sess->pm_user, bimage_hash);
					ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, bimage_share);
					/* ymsg_bimage_notify(ymsg_sess, pm_sess->pm_user, bimage_url, bimage_hash);  */ 
						append_to_open_pms(pm_sess->pm_user, buf, 1);
				} else {ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, 0);}

				reset_current_pm_profile_name();
			}

				if (enter_leave_timestamp) {append_timestamp(chat_window, NULL);}
	  			append_char_pixmap_text((const char**)pixmap_pm_file , NULL);
				snprintf(buf, 768, "   %s** '%s': %s **%s\n",YAHOO_COLOR_ORANGE, 
					 _("Your buddy image has been uploaded"), bimage_url, 
					YAHOO_COLOR_BLACK);
				append_to_textbox( chat_window, NULL, buf );
				play_sound_event(SOUND_EVENT_OTHER);
	}
}

void handle_buddy_image_toggle() {
	/* A person toggles permission for us seeing their buddy image */
	/* We aint gonna turn off the image just because they ask us to 
	   and if the image has changed, a different packet is sent */  
	/* we pretty much ignore this packet unless they 
	are asking us to show their avatar, but here's the structure...
		4-from
		5-to (us)
		206 - 0:No image, 1:Avatar, 2: Photo
	*/
		strncpy(tmp,ymsg_field("4"), 80);
		if (! from_valid_sender(tmp) ) {return;}
		if (is_flooder(tmp)) {return;}
		if (!emulate_ymsg6) { return;}
		if (! show_bimages )  {	return;	}
		check_for_invisible_friend(tmp);

		if (!strcmp(ymsg_field("206"), "")) {return;}

	if (atoi(ymsg_field("206") )==2) {  /* image */ 
		if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
			pm_sess = pm_lpackptr->data;
			if (! pm_sess->buddy_image_file) {
				reset_profile_replyto_name();
				set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
				ymsg_bimage_accept(ymsg_sess, tmp , 1);  /* request it since we dont have it */
				reset_current_pm_profile_name();
			}
		}
	}

	if (atoi(ymsg_field("206") )==0) {  /* no image */ 
		if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
			pm_sess = pm_lpackptr->data;
			/* clear the old image because it may mean a new buddy 
				image is coming */ 
			if (pm_sess->buddy_image_file) {free(pm_sess->buddy_image_file);}
			pm_sess->buddy_image_file=NULL;
			FRIEND_OBJECT=yahoo_friend_find(tmp);
			if (FRIEND_OBJECT) {
				if (FRIEND_OBJECT->buddy_image_hash) {
					g_free(FRIEND_OBJECT->buddy_image_hash);
					FRIEND_OBJECT->buddy_image_hash=NULL;
				}
			}
			/* dont update the PM screen though */ 
		}
		return; 
	}

	if (atoi(ymsg_field("206") )==1) { /* avatar */ 
		/* Show their avatar as a buddy image */ 
		if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
			pm_sess = pm_lpackptr->data;
			if (pm_sess->buddy_image_file) {
				free(pm_sess->buddy_image_file);
				pm_sess->buddy_image_file=NULL;
			}
			probe_for_bimage(tmp);
		}
	}
	return;
}



void handle_buddy_image_changed() {

	strncpy(tmp, ymsg_field("4"), 80);

	log_possible_flood_attack(tmp, 1,"Buddy Image Changed");
	if (is_flooder(tmp)) {return;}
	/* possible spoofs: Messages from ourselves, so ignore */
	if (! from_valid_sender(tmp) ) {return;}
	check_for_invisible_friend(tmp);

	if (! strcmp(ymsg_field("192"), ""))  {return ; }  /* no image serial */

	/* Don't so this...we missed some images this way */ 
	/* if (! strncmp(ymsg_field("192"), "-", 1))  {return ; }  */   /* negative  image serial */

	if (! get_pm_perms(tmp)) { 
		/* ymsg_bimage_accept(ymsg_sess, tmp , 0);  */ 
		ymsg_bimage_toggle(ymsg_sess, tmp, 0);
		return;
		}

	set_last_comment(tmp,"Regular Buddy Image Changed");

	if (!emulate_ymsg6) { return;}
	if (! show_bimages )  {
		ymsg_bimage_toggle(ymsg_sess, tmp, 0);
		return;
		}

	strncpy(tmp2, ymsg_field("192"), 35);
	
	FRIEND_OBJECT=yahoo_friend_find(tmp);
	if (FRIEND_OBJECT) {
		if (FRIEND_OBJECT->buddy_image_hash) {
			g_free(FRIEND_OBJECT->buddy_image_hash);
			FRIEND_OBJECT->buddy_image_hash=NULL;
		}
		if ( (atoi( tmp2) != -1) && (atoi(tmp2) != 0)) { 
			FRIEND_OBJECT->buddy_image_hash=g_strdup(tmp2);
		} 
	}	

	if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
		pm_sess = pm_lpackptr->data;
			clean_bimage_name(tmp2);
			snprintf( tmp3, 254, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, tmp2 );	
			if (pm_sess->buddy_image_file) { /* same image */
						if (!strcmp(pm_sess->buddy_image_file, tmp3)) {return;}
			}
			if (bimage_exists(tmp2))  {
				if (pm_sess->buddy_image_file) {free(pm_sess->buddy_image_file);}
				pm_sess->buddy_image_file=strdup(tmp3);
				update_my_buddy_image(pm_sess, 0);
				return ;
			} else {
				reset_profile_replyto_name();
				set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
				ymsg_bimage_accept(ymsg_sess, tmp , 1);  /* request it since we dont have it */
				reset_current_pm_profile_name();
			}
	} else { 
		 /*  ymsg_bimage_accept(ymsg_sess, tmp , 0);  */ 
		return;
		  }
}


char *get_pic_checksum(char *picfile) {
	char checksum[16];
	int jj=0;
	FILE *file;
	struct stat st;
	if (picfile) {
		if (! stat(picfile, &st)) {
			file = fopen(picfile, "rb");
			if (file) {
				GString *s = g_string_sized_new(st.st_size);
				size_t len;
				g_string_set_size(s, st.st_size);
				len = fread(s->str, 1, st.st_size, file);
				fclose(file);
				g_string_set_size(s, len);
				jj = g_string_hash(s);
				g_string_free(s, TRUE);
			}
		}
	}

	if (jj==0) {return NULL;}
	snprintf(checksum,14,"%d", jj);
	return strdup(checksum);
}

int upload_new_buddy_image() {
		/* This checks the validity of our buddy image and settings 
		and uploads or re-uploads the buddy image if necessary */ 
	char filename[256];
	struct stat sbuf;
	if (! bimage_file) {return 0;}
	if (! emulate_ymsg6) {return 0;}
	if (! bimage_share) {return 0;}

	if (  (! bimage_hash) || (time(NULL)>bimage_timestamp) )  {
		char *checksum=NULL;
		char *filepic=NULL;
		convert_buddy_image();
		snprintf( filename, 254, "%s/.friend_icon.png", GYACH_CFG_DIR);
		if (stat( filename, &sbuf )) {return 0;} /* Our image aint there */

		if (bimage_hash) {free(bimage_hash); bimage_hash=NULL;}
		if (bimage_url) {free(bimage_url); bimage_url=NULL;}
		filepic=strdup(filename);
		checksum=get_pic_checksum(filepic);
			/* Either the file aint readable or no checksum was created */
		if (! checksum) {free(filepic); return 0;}
		bimage_timestamp=((int) time(NULL) )+86400;  /* 24 hours */ 
		bimage_hash=strdup(checksum);
		ymsg_upload_bimage(ymsg_sess, filepic);
		free(filepic);
		free(checksum);
		return 0;
	}
	if (! bimage_url) {return 0;} /* we're waiting for a URL back from Yahoo */
	return 1; /* everything is already okay */
}

void handle_buddy_image_sent() {

	strncpy(tmp, ymsg_field("4"), 80);

	log_possible_flood_attack(tmp, 1,"Buddy Image Sent");
	if (is_flooder(tmp)) {return;}
	/* possible spoofs: Messages from ourselves, so ignore */
	if (! from_valid_sender(tmp) ) {return;}
	check_for_invisible_friend(tmp);
	if (!emulate_ymsg6) { set_last_comment(tmp,"Regular Buddy Image Sent"); return;}
	if (! get_pm_perms(tmp)) { 
				ymsg_bimage_toggle(ymsg_sess, tmp, 0);
				/* ymsg_bimage_accept(ymsg_sess, tmp , 0);  */ 
				return;}
	if (! show_bimages )  {
		/* ymsg_bimage_accept(ymsg_sess, tmp , 0); */ 
		ymsg_bimage_toggle(ymsg_sess, tmp, 0);
		set_last_comment(tmp,"Regular Buddy Image Sent"); 
		return;
		}

if ( (find_temporary_friend(tmp) ) || (find_friend(tmp)) ) {
	set_last_comment(tmp,"Regular Buddy Image Sent");
}

	if (( pm_lpackptr = find_pm_session( tmp )) != NULL ) {
		pm_sess = pm_lpackptr->data;

		reset_profile_replyto_name();
		set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

			if (! strcmp(ymsg_field("20"), ""))  { 
				/* no URL, somebody is requesting ours */				
				/* somebody wants our image and has permission */
				/* or they are offering their image */

				if (atoi(ymsg_field("13") )>0) {
				if (! pm_sess->buddy_image_file) {ymsg_bimage_accept(ymsg_sess, tmp , 1); }
				if (! pm_sess->buddy_image_share)  {
					ymsg_bimage_toggle(ymsg_sess, tmp, 0);
					reset_current_pm_profile_name();
					return ;
				}

			/* field 13=3, i think this is just confirmation, maybe an error msg */
			if (atoi(ymsg_field("13") )>2) {
				snprintf(buf, 300, "   %s** '%s' **%s\n",YAHOO_COLOR_GREEN, 
					 _("The user has declined to view your buddy image or is refusing to send their own."), YAHOO_COLOR_BLACK);
					append_to_open_pms(tmp, buf, 1);
				if (! pm_sess->buddy_image_file)  {
					pm_sess->buddy_image_file=strdup("/gyErro2tid98ojpe.jpg");
				}
				reset_current_pm_profile_name(); 
				return ;
 
			}

				if (bimage_url && bimage_hash && bimage_file)  {
					/* FIXME, need to check the timestamp here and upload if necessary */
					/* If the timestamp for our image is too old, need to create a new 
						hash name, re-upload, and send a notification to all PM windows */

					if (upload_new_buddy_image()) {
						/* ymsg_bimage_update(ymsg_sess, pm_sess->pm_user, bimage_hash);
						ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, bimage_share); */ 
						ymsg_bimage_notify(ymsg_sess, pm_sess->pm_user, bimage_url, bimage_hash);
				snprintf(buf, 300, "   %s** '%s' **%s\n",YAHOO_COLOR_ORANGE, 
					 _("Your buddy image has been sent."), YAHOO_COLOR_BLACK);
					append_to_open_pms(tmp, buf, 1);
						}
					}  else  {
						upload_new_buddy_image();
					}

				}
				reset_current_pm_profile_name();
				return; 
			}

			if (! strcmp(ymsg_field("192"), ""))  {reset_current_pm_profile_name(); return ; } 
			 /* no image serial */

			strncpy(tmp2, ymsg_field("192"), 35);
			clean_bimage_name(tmp2);
			snprintf( tmp3, 254, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, tmp2 );	
			if (pm_sess->buddy_image_file) { /* same image */
						if (!strcmp(pm_sess->buddy_image_file, tmp3)) {
							update_my_buddy_image(pm_sess, 0);
							reset_current_pm_profile_name();
							return;
						}
			}
			if (bimage_exists(tmp2))  {
					if (pm_sess->buddy_image_file) {free(pm_sess->buddy_image_file);}
					pm_sess->buddy_image_file=strdup(tmp3);
					update_my_buddy_image(pm_sess, 0);
					reset_current_pm_profile_name();
					return; 
			} else {
				/* download it because we dont have it */
				strncpy(tmp3, ymsg_field("20"), 300);  /* URL */
				if (! strcmp(tmp3, "")) {reset_current_pm_profile_name(); return;}

				/* check the validity of the file we have been sent */
				if (! strstr(tmp3, "friend_icon.png")) {
					reset_current_pm_profile_name(); return;
				}		

				if (download_yavatar(ymsg_field("192"), 4, tmp3)) {
					if (pm_sess->buddy_image_file) {free(pm_sess->buddy_image_file);}
					snprintf( tmp3, 254, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, tmp2 );	
					pm_sess->buddy_image_file=strdup(tmp3);
					update_my_buddy_image(pm_sess, 0);
				}
			}

	} else {
		/* ymsg_bimage_accept(ymsg_sess, tmp , 0); */
		/* just ignore this for now */
		return;
		}
}



/* Begin actual packet handling */
/* changed below, we will logoff when we receive/send a 'buddy off' message for ourself, 
   NOT when we logout out of chat, this can allow us to use this program as a 'pager' 
   not just a chat program - PhrozenSmoke */

void handle_yahoo_packet() {
	int org_con=auto_reconnect;
	int reveal_packets=0;  /* added PhrozenSmoke: whether to show packets on stdout */


	if ( ymsg_sess->pkt.size ) {
		split( ymsg_sess->pkt.data, YMSG_SEP );

		/* added by PhrozenSmoke, for debugging only */
		if (reveal_packets==1) { 
			printf("\nPACKET RECEIVED:  TYPE[%d]  SIZE[%d]   %s\n",ymsg_sess->pkt.type,ymsg_sess->pkt.size, ymsg_sess->pkt.data);
			fflush(stdout);
											  }

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nINCOMING PACKET RECEIVED:  TYPE [%d]   SIZE [%d]   DATA: [%s]\n", ctime(&time_llnow), ymsg_sess->pkt.type,ymsg_sess->pkt.size, ymsg_sess->pkt.data);
				fflush( capture_fp );
			}

	} else {
		strcpy( ymsg_fields[0], "__END__" );
		strcpy( ymsg_fields[1], "__END__" );
	}

	reset_profile_replyto_name();

	/* clear all buffers, just to be safe */
	memset( buf, 0, sizeof( buf ));
	memset( tmp, 0, sizeof( tmp ));
	memset( tmp2, 0, sizeof( tmp2 ));
	memset( tmp3, 0, sizeof( tmp3 ));


	switch( ymsg_sess->pkt.type ) {

		case YMSG_CONFMSG:  /* added: PhrozenSmoke, conferences, experimental */
		case YMSG_CONFLOGOFF:
		case YMSG_CONFLOGON:
		case YMSG_CONFDECLINE:
		case YMSG_CONFADDINVITE:
		case YMSG_CONFINVITE:
		case YMSG_GYE_CONFEXIT:  /* internal: forces conference disconnect */
			handle_conference_packet(ymsg_sess->pkt.data, ymsg_sess->pkt.type, ymsg_sess->pkt.size);
			break;

		case YMSG_GOTO:
			handle_goto_error();
			break;

		case YMSG_REJECTBUDDY:
 			/* we get this back after we deny a person rights to be on our 
				buddy list, nothing important in it */
			break;

		case YMSG_WEBCAM:  /* webcam  */
			handle_webcam();
			break;

		case YMSG_VOICECHAT:  /* added: PhrozenSmoke, voice chat invite */
			handle_voice_chat_invite();
			break;

		case YMSG_NEW_CONTACT:  
			handle_new_contact();
			break;

		case YMSG_ADD_BUDDY:
			handle_add_buddy();
			break;

		case YMSG_REM_BUDDY:
			handle_rem_buddy();
			break;

		case YMSG_GET_KEY:
				handle_get_key();
				break;

		case YMSG_COOKIE:
				handle_get_cookie();
				break;

		case YMSG_ONLINE:
				handle_online();
				break;

		case YMSG_MAIL:
				handle_mail();
				break;

		case YMSG_AWAY:
		case YMSG_BACK:
		case YMSG_BUDDY_STATUS:
				handle_away_back();
				break;

		case YMSG_COMMENT: 
				handle_comment();
				break;

		case YMSG_JOIN:
				handle_join();
				break;

		case YMSG_EXIT:
		case 0x9A:  /* YAHOO_SERVICE_CHATLEAVE, not sure */
				handle_exit();
				break;

		case YMSG_GAMEMSG:   /* added, PhrozenSmoke, treat as PM */
		case YMSG_SYSMSG:   /* added, PhrozenSmoke, treat as PM */
		case YMSG_PM:
		case YMSG_PM_RECV:
				collect_profile_replyto_name();
				handle_pm();
				reset_profile_replyto_name();
				if (yalias_name) {g_free(yalias_name); yalias_name=NULL;}
				break;

		case YMSG_NOTIFY:
				strncpy( tmp, ymsg_field( "4" ), 512);		
				strncpy( tmp2, ymsg_field( "49" ), 512);	
				/* message that tell us the user is typing to us or stopped */
				/* typing is stored in '49' field */
				/* added: PhrozenSmoke, the 'NOTIFY' can also tell us about a 
					webcam invitation, for now we will just print it on the screen */

				if ( ! strcmp( tmp2, "TYPING" )) {
					collect_profile_replyto_name();
					handle_typing_notice();
					reset_profile_replyto_name();
					return;
										}

				if ( ! strcmp( tmp2, "WEBCAMINVITE" )) {
					char *typptr=NULL;
					typptr=strchr(tmp,',');  /* cut off if its comma-separated */
					if (typptr) {*typptr='\0';}
					strncpy( tmp3, ymsg_field( "14" ), 512);	
					if ( ! strcmp( tmp3," " )) {handle_webcam_invite(); return;}
					if ( ! strcmp( tmp3,"-1" )) {handle_webcam_invite_reject(); return;}
					if ( ! strcmp( tmp3,"1" )) {handle_webcam_invite_accept(); return;}
					check_boot_attempt();
					return;
											     }

				if ( ! strcmp( tmp2, "GAME" )) {
					handle_game_notice();
					return;
									     }

				if ( ! strcmp( tmp2, "CONTACTINFO" )) { /* Yahoo 'business' cards */
					collect_profile_replyto_name();
					handle_contact_info();
					reset_profile_replyto_name();
					return;
									     }

				if ( ! strcmp( tmp2, ENCRYPTION_OFF )) { /* Encryption notify */
					collect_profile_replyto_name();
					handle_encryption_notify();
					reset_profile_replyto_name();
					return;
									     }
				if ( ! strcmp( tmp2, ENCRYPTION_UNAVAIL)) { /* Encryption notify */
					collect_profile_replyto_name();
					handle_encryption_notify();
					reset_profile_replyto_name();
					return;
									     }
				if ( ! strcmp( tmp2, ENCRYPTION_START_TAG)) { /* Encryption notify */
					collect_profile_replyto_name();
					handle_encryption_notify();
					reset_profile_replyto_name();
					return;
									     }
				if ( ! strcmp( tmp2, ENCRYPTION_START_GPGME)) { 
						/* GPG Encryption notify */
					collect_profile_replyto_name();
					handle_encryption_notify();
					reset_profile_replyto_name();
					return;
									     }

				if ( strcmp( tmp2, "y" )==0) {
					handle_ygame_notify();
					return;
									     }

				if (reveal_packets ||  ymsg_sess->debug_packets) {
					printf("*** UNKNOWN NOTIFY PACKET [%d]\n%s\nNOTIFY: %s\n",ymsg_sess->pkt.type, ymsg_sess->pkt.data,  tmp2);  	
					fflush(stdout);
				}
				break;

		case YMSG_BUDDY_ON:
		case YMSG_IDACT:  /* YAHOO_SERVICE_IDACT */
		case YMSG_USERSTAT:  /* YAHOO_SERVICE_USERSTAT */
				handle_buddy_on();
				break;

		case YMSG_GYE_EXIT:  
			/* added PhrozenSmoke, used internally to force a disconnect */
			auto_reconnect=0;
			flooder_buddy_list_protect();
			handle_logoff();
			auto_reconnect=org_con;
			break;

		case YMSG_BUDDY_OFF:
		case YMSG_IDDEACT:  /* YAHOO_SERVICE_IDDEACT */
		case YMSG_SIGNOFF:  /* When running in Messenger-6 mode */ 
				handle_buddy_off();
				break;

		case 0xc:  /* chatinvite, not sure if this is used anymore, may be mishandled */
		case YMSG_INVITE: 
		case 0x17:   /* invitation failed */
				handle_chat_invite();
				break;

		case YMSG_FILETRANSFER:
				handle_file_transfer();
				break;

		case YMSG_LOGOUT:  /* we left Yahoo CHAT, not Yahoo itself */
				handle_logout();
				break;

		case YMSG_GAMES_ENTER:   /* added: PhrozenSmoke  */
			handle_games_enter();
			break;

		case YMSG_GAMES_LEAVE:   /* added: PhrozenSmoke  */
			handle_games_leave();
			break;

		case YMSG_CREATE_ROOM:   /* added: PhrozenSmoke */
			handle_create_room();
			break;

		case YMSG_LOGIN:  /* a login response, added PhrozenSmoke */
			handle_login();
			break;

		case YMSG_P2PASK:   /* added: PhrozenSmoke */
			handle_p2pask();
			break;

		case YMSG_P2P:   
			/* added: PhrozenSmoke */
			/* chr '77' can also refer to regular special features, imvironments, etc. */
			handle_special_features();
			break;

		case YMSG_MSGRPING:   /* added: PhrozenSmoke - a ping from Yahoo */
			handle_yping();
			break;

		case YMSG_CALENDAR:   
			/* added: PhrozenSmoke - YAHOO_SERVICE_CALENDAR, calendar event */
			handle_calendar();
			break;

		case YMSG_NOTICIAS:   /* NEWS bulletins */
			/* added: PhrozenSmoke - Yahoo user-customized 'news bulletin' packets */
			handle_news_alert();
			break;

		case YMSG_GOTGROUP_RENAME:   
			/* ignore this, handled by 'YMSG_GROUP_RENAME' packet that
			follows */
			break;

		case YMSG_GROUP_RENAME:
			handle_group_rename();
			break;

		case YMSG_STEALTH_PERM:
		case YMSG_STEALTH:
			handle_stealth();
			break;

		case YMSG_ADDIGG:  /* 'addignore' - generally an empty packet */
		case YMSG_TOGGLEIGG:  /* added/removed a user on Y! permenant ignore list */
			handle_perm_igg();
			break;

		case YMSG_SETTINGS:
			/* This is just Yahoo telling Windows messenger which tab to show first */
			/* We'll ignore this - it aint relevant here, new in Messenger 6 (Beta) */
			/* Field '211' which tab to show such as 'Tab:ymsgr_games=1,0\n' */
			break;

		case YMSG_VERIFY:
			/* When running in Messenger-6 mode */
			ymsg_get_key(ymsg_sess);
			break;

		case YMSG_AUDIBLE:
			/* When running in Messenger-6 mode */
			collect_profile_replyto_name();
			handle_audible();
			reset_profile_replyto_name();
			break; 

		case YMSG_GAME_INVITE:
			/* When running in Messenger-6 mode */
			collect_profile_replyto_name();
			handle_game_invite();
			reset_profile_replyto_name();
			break; 

		case YMSG_AVATAR_TOGGLE:
			/* When running in Messenger-6 mode */
			handle_avatar_toggled();
			break; 

		case YMSG_AVATAR_UPDATED:
			/* When running in Messenger-6 mode */
			handle_avatar_changed();
			break; 

		case YMSG_BIMAGE_SEND:
			/* When running in Messenger-6 mode */
			handle_buddy_image_uploaded();
			break; 

		case YMSG_BIMAGE_TOGGLE:
			/* When running in Messenger-6 mode */
			collect_profile_replyto_name();
			handle_buddy_image_toggle();
			reset_profile_replyto_name();
			break; 

		case YMSG_BIMAGE_UPDATED:
			/* When running in Messenger-6 mode */
			collect_profile_replyto_name();
			handle_buddy_image_changed();
			reset_profile_replyto_name();
			break; 

		case YMSG_BIMAGE:
			/* When running in Messenger-6 mode */
			collect_profile_replyto_name();
			handle_buddy_image_sent();
			reset_profile_replyto_name();
			break; 

		case YMSG_VISIBILITY:
			/* When running in Messenger-6 mode */
			/* shouldn't get this, but just in case, ignore it. */
			break;


		default:
			/* unknown packet */
			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\n*** DEBUG: UNKNOWN INCOMING PACKET ***\nTYPE [%d]   SIZE [%d]   DATA [%s]\n", ctime(&time_llnow), ymsg_sess->pkt.type,ymsg_sess->pkt.size, ymsg_sess->pkt.data);
				fflush( capture_fp );
			}

			if (reveal_packets || ymsg_sess->debug_packets) {
				printf("*** DEBUG: UNKNOWN PACKET\nTYPE:  [%d]\nDATA:  %s\n",ymsg_sess->pkt.type, ymsg_sess->pkt.data);
				fflush(stdout);
			}

	
	} /* end switch */

}  /* end show_packet */


// }  /* end show_packet */





/* TESTING METHODS below, commented out */

/* for testing REMOVE */  /* 
void fake_offlines() {
	u_char buf[4096];
	u_char *ptr = buf;
	snprintf( ptr, 4093,  "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"1", YMSG_SEP, "cj", YMSG_SEP,
		"4", YMSG_SEP, "PhrozenSmoke", YMSG_SEP,
		"14", YMSG_SEP, "This is the first test message", YMSG_SEP,
		"97", YMSG_SEP, "1", YMSG_SEP,
		"15", YMSG_SEP, "1063337456", YMSG_SEP,
		"63", YMSG_SEP, ";0", YMSG_SEP,
		"1", YMSG_SEP, "cj", YMSG_SEP,
		"4", YMSG_SEP, "PhrozenSmoke", YMSG_SEP,
		"14", YMSG_SEP, "This is the 2nd test message http://www.hotmail.com", YMSG_SEP,
		"97", YMSG_SEP, "1", YMSG_SEP,
		"15", YMSG_SEP, "1063337469", YMSG_SEP,
		"63", YMSG_SEP, ";0", YMSG_SEP
		);
	parse_offline_messages(ptr);
}

void offline_msg_test() {
	clear_offline_messages();
	fake_offlines();
	show_offline_messages();
}
*/ 


/* for testing REMOVE */  /* The new News packets, beta.alerts.yahoo.com */ /* 

void run_news_test() {
	u_char ttbuf[2048];
	u_char *ptr = ttbuf;

	snprintf( ptr, 2000, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"5", YMSG_SEP, "PhrozenSmoke", YMSG_SEP,
		"132", YMSG_SEP, "http://beta.alerts.yahoo.com/main.php?view=my_alerts", YMSG_SEP,
		"4", YMSG_SEP, "Yahoo! Alerts", YMSG_SEP,
		"14", YMSG_SEP, "n - BreakingNews\nabstract Id: d0300002215e2\nhttp://news.yahoo.com", YMSG_SEP,
		"83", YMSG_SEP, "Breaking News (AP, Reuters)", YMSG_SEP,
		"20", YMSG_SEP, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\">\n<HTML>\n<HEAD><TITLE>Yahoo! Alerts</TITLE>\n<META http-equiv=Content-Type \ncontent=\"text/html; charset=windows-1252\"><BASEFONT face=arial&#44;sans-serif \nsize=2>\n</HEAD>\n<BODY><B>Yahoo! Alerts</B> \n<TABLE cellSpacing=0 cellPadding=0 width=\"100%\" border=0>\n  <TBODY>\n  <TR>\n    <TD noWrap bgColor=#acb59a height=1><SPACER height=\"1\" width=\"1\" \n      type=\"block\"></TD></TR>\n  <TR>\n    <TD bgColor=#effada height=24><FONT size=2><FONT color=#ec661f><B>Breaking\n      News</B></FONT> Sunday&#44; September 26&#44; 2004&#44; 11:06 PM PDT </FONT></TD></TR>\n  <TR>\n    <TD noWrap bgColor=#acb59a height=1><SPACER height=\"1\" width=\"1\" \n      type=\"block\"></TD></TR></TBODY></TABLE>\n<P>\nMOSUL&#44; Iraq (AP) Police say a car bomb has exploded near an Iraqi National Guard patrol in the northeastern city of Mosul. </P>\n  <BR>\n  <p>\n    Visit <a href=\"http://news.yahoo.com\">Yahoo! News</a> for more details or search for this story on <a href=\"http://news.search.yahoo.com/search/news/?fr=sfp&ei=UTF-8&p=MOSUL&#44; Iraq (AP) Police say a car bomb has exploded near an Iraqi National Guard patrol in the northeastern city of Mosul.\">Yahoo! News Search</a>.  \n  </p>\n  <br>\n</td></tr></table>\n</BODY></BASEFONT></HTML>", YMSG_SEP
		);

ymsg_sess->pkt.size=strlen(ptr);
ymsg_sess->pkt.type=0x12c;
memcpy( ymsg_sess->pkt.data, ttbuf, ymsg_sess->pkt.size );
ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';
handle_yahoo_packet();
}   

 */ 

/* for testing REMOVE */  /*   
void run_news_test() {
	u_char ttbuf[2048];
	u_char *ptr = ttbuf;

	snprintf( ptr, 2000, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"5", YMSG_SEP, "PhrozenSmoke", YMSG_SEP,
		"4", YMSG_SEP, "Yahoo! Alerts", YMSG_SEP,
		"14", YMSG_SEP, "News - Gay Marriage\nOregon AG Says Gay Marriage Ban Invalid\nhttp://story.news.yahoo.com/news?tmpl=story&u=/ap/20040313/ap_on_re_us/gay_marriage_7", YMSG_SEP,
		"83", YMSG_SEP, "MERIDEN&#x2C; Conn. (AP) Nearly 20&#x2C;000 SBC Communications workers in Connecticut&#x2C; Ohio and Michigan begin four-day strike over health care benefits and job outsourcing.", YMSG_SEP,
		"20", YMSG_SEP, "<div style=position:relative;left:10;width:95%;background-color:white;font-family:arial;font-size:11px><img src=\"http://us.i1.yimg.com/us.yimg.com/i/mesg/alerts/news.gif\"><br><a href=\"http://story.news.yahoo.com/news?tmpl=story&u=/ap/20040313/ap_on_re_us/gay_marriage_7\" target=\"_new\"><b>Oregon AG Says Gay Marriage Ban Invalid</b></a><br>Supporters of gay marriage are applauding an opinion by the attorney general that banning gay marriage probably violates Oregon's constitution.</div>", YMSG_SEP
		);

ymsg_sess->pkt.size=strlen(ptr);
ymsg_sess->pkt.type=0x12c;
memcpy( ymsg_sess->pkt.data, ttbuf, ymsg_sess->pkt.size );
ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';
handle_yahoo_packet();
}   */ 



/* for testing REMOVE */ /* 
void run_status_test() {
	u_char ttbuf[2048];
	u_char *ptr = ttbuf;

	snprintf( ptr, 2000, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		"8", YMSG_SEP, "1", YMSG_SEP,
		"7", YMSG_SEP, "phrozensmoke\r\x81smokeydog", YMSG_SEP,
		"10", YMSG_SEP, "99", YMSG_SEP,
		"19", YMSG_SEP, "SLeePinggggggg", YMSG_SEP,	
		"47", YMSG_SEP, "1", YMSG_SEP,	
		"13", YMSG_SEP, "7", YMSG_SEP,	
		"197", YMSG_SEP, "1EljnJhwCAAEDPIEr4AcOAQ==", YMSG_SEP,	
		"184", YMSG_SEP, "YSTATUS=2\tu\t1376579776\ts\t3391323\tp\t1\tm\t633\td\t0", YMSG_SEP
		);

ymsg_sess->pkt.size=strlen(ptr);
ymsg_sess->pkt.type=0xc6;
memcpy( ymsg_sess->pkt.data, ttbuf, ymsg_sess->pkt.size );
ymsg_sess->pkt.data[ ymsg_sess->pkt.size ] = '\0';
handle_yahoo_packet();
}     */ 

/* end for TESTING */




