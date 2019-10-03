/*****************************************************************************
 * callbacks.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <unistd.h>
#include "gyach.h"
#include "gyach_int.h"

#include <gtk/gtk.h>

#include <sys/utsname.h>

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include <time.h>

#include "aliases.h"
#include "callbacks.h"
#include "gyach_int.h"
#include "commands.h"
#include "friends.h"
#include "history.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "main.h"
#include "roomlist.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "conference.h"
#include "webconnect.h"
#include "ycht.h"
#include "webcam.h"
#include "plugins.h"
#include "profname.h"
#include "gytreeview.h"
#include "sounds.h"

GtkWidget *favorites_window = NULL;
GtkWidget *file_sel;
GtkWidget *edit_win = NULL;
char *last_loginname=NULL;

GtkTreeView *setup_list = NULL;
char *setup_row_selected = NULL;
GtkTreeIter setupiter;
char setup_which[32] = "";
static gchar callbackbuf[1152];

GtkWidget *room_maker_window=NULL;
GtkWidget *favroom_setup_menu=NULL;
GtkWidget *budpop_menu=NULL;
GtkWidget *budgrp_menu=NULL;

char *create_room_selected=NULL;
char *selected_buddy_group=NULL;
char *selected_buddy =NULL;
char *stealth_sel_buddy=NULL;
char *user_for_file=NULL;

int web_login_warn_shown=0;

 /* keep these here */
extern PM_SESSION *new_pm_session(char *forwho);
extern int unlock_yahoo_account(YMSG_SESSION *session, char *mysecret_word);
extern void show_incoming_pm(PM_SESSION *gpm_sess, int mesg);
void update_my_buddy_image(PM_SESSION *pm_sess, int is_me);


#define FILE_MSG_SAVE	"Save Current Text"
#define FILE_MSG_LOG	"Log To File"

char *TMP_FRIEND_CB="~[Temporary Friends]~";


void
on_b_send_clicked                      (GtkButton       *button,
                                        gpointer         user_data)
{
	char *text;

	text = strdup(gtk_entry_get_text(GTK_ENTRY(chat_entry)));
	gtk_entry_set_text(GTK_ENTRY(chat_entry), "");

	if ( ! strlen( text ))  {
		free(text);    /*  added: PhrozenSmoke, seg fault watch */
		return;
								}

	history_add( text );

	/* ok, we have something to send */
	chat_command( text );
	free(text);  /*  added: PhrozenSmoke, seg fault watch */
	/*  ct_scroll_to_bottom();  */  
}


void
on_b_clear_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_entry_set_text(GTK_ENTRY(chat_entry), "");
}


void
on_window1_map                         (GtkWidget       *widget,
                                        gpointer         user_data)
{
	chat_window = widget;
}


void
on_chat_entry_map                      (GtkWidget       *widget,
                                        gpointer         user_data)
{
	chat_entry = widget;
}


void
on_chat_entry_activate                 (GtkEditable     *editable,
                                        gpointer         user_data)
{
	on_b_send_clicked( NULL, NULL );
}


void
on_aliases_map                        (GtkWidget       *widget,
                                        gpointer         user_data)
{
	chat_aliases = GTK_TREE_VIEW(widget);
	read_aliases();
}


void collect_chat_list_selected(GtkTreeSelection *selection, gpointer data)  {
	GtkTreeModel *model;
	if ( user_selected ) {
		free( user_selected );  
		user_selected=NULL;
	}
	if ( gtk_tree_selection_get_selected(selection, &model, &chat_user_iter)) {
		gchar *sfound;
		gtk_tree_model_get(model, &chat_user_iter,GYQUAD_COL2, &sfound, -1);	
		user_selected=strdup(sfound);
		g_free(sfound);
	} 
}

void onChatUsersRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data) {

	if (!user_selected) {
			gchar *sfound;
			gtk_tree_model_get_iter(gtk_tree_view_get_model(tv), &chat_user_iter, tp);
			gtk_tree_model_get(gtk_tree_view_get_model(tv), &chat_user_iter,GYQUAD_COL2, &sfound, -1);	
			user_selected=strdup(sfound);
			g_free(sfound);
	}
	if (!user_selected) {return;}
	on_pm_activate  (NULL, NULL);
}

void display_chat_user_popup() {
	if (! chat_user_menu)  {chat_user_menu = create_user_menu();}
	/* right clicked popup user menu */
	gtk_menu_popup((GtkMenu *) chat_user_menu,
	NULL, NULL, NULL, NULL, 1, 0);
}

gboolean on_chatlist_kb_popup  (GtkWidget  *widget, gpointer user_data) {
	display_chat_user_popup();
	return FALSE;
}

gboolean
on_users_clicked                       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	if ((event->button == 3 && event->type == GDK_BUTTON_PRESS)) {
				display_chat_user_popup();
	}
	return FALSE;
}



void
on_users_map                           (GtkWidget       *widget,
                                        gpointer         user_data)
{
	chat_users = GTK_TREE_VIEW(widget);
}


void onAliasesRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		GtkTreeModel *model;
		GtkTreeIter iter;
		model=gtk_tree_view_get_model(tv);
		if (gtk_tree_model_get_iter(model, &iter, tp)) {
			gchar *iname;
			char *tname=NULL;
			gtk_tree_model_get(model, &iter,  GYTRIPLE_COL1, &iname, -1);
			tname=strdup(iname);
				if ( user_selected ) {
					check_alias(tname, user_selected );
					/* free( user_selected );
						user_selected = NULL;  */
				} else {check_alias( tname, "" );}
			free(tname);
			g_free(iname);
		}
}

void display_alias_pop_menu() {
	if ( alias_window ) {return;}
	if (! alias_menu) {alias_menu = create_alias_menu();}
	gtk_menu_popup((GtkMenu *) alias_menu,
	NULL, NULL, NULL, NULL, 1, 0);
}

gboolean on_aliases_clicked  (GtkWidget  *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS)
	{	/* right clicks */
		display_alias_pop_menu();
	}
	return FALSE;
}

gboolean on_aliases_kb_popup  (GtkWidget  *widget, gpointer user_data) {
	/* Shift-F10 pressed */
	display_alias_pop_menu();
	return TRUE;
}


void
on_connect_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( logged_in )
		return;

	if (login_window) {gtk_widget_show_all( login_window ); }
	else {
		login_window = build_login_window();
		gtk_widget_show_all( login_window );
		}
}


void
on_disconnect_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

	if ( ! logged_in )
		return;
	ymsg_leave_chat( ymsg_sess );
	ymsg_messenger_logout(ymsg_sess);
	ymsg_sess->pkt.type = YMSG_GYE_EXIT; /*  force logout */
	show_yahoo_packet();

}


void on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ymsg_sess->io_callback_tag > 0) {
		gdk_input_remove( ymsg_sess->io_callback_tag );
		ymsg_sess->io_callback_tag = 0;
	}

	if ( ymsg_sess->ping_callback_tag >0) {
		gtk_timeout_remove( ymsg_sess->ping_callback_tag );
		ymsg_sess->ping_callback_tag = 0;
	}

	close( ymsg_sess->sock ); 
	ymsg_sess->sock = -1; 
	ymsg_sess->quit = 1; 

	write_config();
	history_save();

	gdk_threads_leave();

	exit(0);
	/* gtk_main_quit(); */
}


void
on_room_list_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	if ( ! room_window ) {
		room_window = create_room_list();
		gtk_widget_show_all( room_window );

		populate_room_list(0);
	} else {
		gtk_widget_show_all( room_window );

		if ( ! cache_room_list ) {
			tmp_widget = lookup_widget( room_window, "room_tree" );
			gy_empty_model(gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget)), GYTV_TYPE_TREE);
			populate_room_list(0);
		}
	}
}


void
on_favorites_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GList *this_item = NULL;
	GtkWidget *tmp_widget;
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *answer=NULL;
	GdkPixbuf *imbuf=NULL;

	if ( favorites_window )
		return;

	favorites_window = create_favorites();
	tmp_widget = lookup_widget( favorites_window, "fav_room_list" );
	model=freeze_treeview(tmp_widget );

	this_item = favroom_list;
	while( this_item ) {	
	answer = strdup(this_item->data);

	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_pm_join);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, answer,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "",  -1);
	if (imbuf) {g_object_unref(imbuf);}
	free(answer);

	this_item = g_list_next( this_item );
	}
	unfreeze_treeview(tmp_widget, model);
	set_basic_treeview_sorting(tmp_widget, GYLIST_TYPE_SINGLE);
	gtk_widget_show_all( favorites_window );
}


void on_add_to_favorites_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GList *this_item = NULL;

	if (! strcmp( ymsg_sess->room, "[NONE]") ) {return; }

	this_item = favroom_list;
	while( this_item ) {
		if ( ! strcmp( this_item->data, ymsg_sess->room )) {
			snprintf(callbackbuf, 380, "  %s%s[%s: '%s']%s%s\n" ,
				YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED,
				_("The following room is already in your favorites list"), 
				ymsg_sess->room,
				YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK
				  );
			append_to_textbox_color(chat_window,NULL,callbackbuf);
			return;
		}
		this_item = g_list_next( this_item );
	}

	if (!is_conference)  {
	favroom_list = g_list_append( favroom_list, strdup( ymsg_sess->room ));
	write_config();
			snprintf(callbackbuf, 380, "  %s%s[%s: '%s']%s%s\n" ,
				YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE,
				_("The following room has been added to your favorites list"), 
				ymsg_sess->room,
				YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK
				  );
			append_to_textbox_color(chat_window,NULL,callbackbuf);
				     } else  {
			snprintf(callbackbuf, 380, "  %s%s[%s]%s%s\n" ,
				YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED,
				_("You cannot add a conference room to your favorites list"), 
				YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK
				  );
			append_to_textbox_color(chat_window,NULL,callbackbuf);
					        }
}


void on_help_activate   (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *tmp_widget=NULL;
	tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
	snprintf(callbackbuf, 350, "file:%s/doc/%s", 
		PACKAGE_DATA_DIR,
		user_data?"gyache-ylinks.html":"gyache-help.html");
	load_myyahoo_URL(callbackbuf);
	if (tmp_widget) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
	}
	gdk_window_raise(chat_window->window );
}


void  on_about_activate    (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkWidget *tmp_widget;
	char *info_data=NULL;
	struct utsname un;

	info_data=malloc(4096);
	if (! info_data) {return;}
	uname(&un);

		snprintf(info_data, 4000,
			"<table border=\"2\" cellpadding=\"4\" width=\"95%%\"><tr><td width=\"100%%\" bgcolor=\"#eeeeFF\"><center><b><font size=\"4\"  color=\"#660099\">%s</b></center></font><font size=\"3\" color=\"#000000\" face=\"Arial\"><br><br><b><a href=\"%s\">Gyach Enhanced</a> (Gyach-E) version %s</b><br>"

		"Copyright (c) 2003-2006 Erica Andrews.<br>Copyright (c) 2000-2003 Chris Pinkham.<br><br>This software comes with No Warranty and No Technical Support. This is free software and is distributed under the GNU General Public License (GPL).  You may redistribute it under certain conditions.  Please see the COPYING or LICENSE file for details." 
			"<br><br>"
			"&nbsp; &nbsp;<b>Project Started:</b>   July 2000<br>"
			"&nbsp; &nbsp;<b>This Binary Compiled:</b>   " __DATE__ " at " __TIME__ "<br>"
			"&nbsp; &nbsp;<b>GTK+ Version:</b>   %d.%d.%d<br>"
			"&nbsp; &nbsp;<b>Currently Running on:</b>   %s %s [%s]<br>"
			"&nbsp; &nbsp;<b>Author E-Mail:</b>   %s<br>"
			"&nbsp; &nbsp;<b>Web Page:</b>   <a href=\"%s\">%s</a><br>"
			"&nbsp; &nbsp;<b>Old Gyach-Author E-Mail:</b>   %s<br>"
			"&nbsp; &nbsp;<b>Old Gyach-Web Page:</b>   <a href=\"%s\">%s</a><br>",

			_("About GYach Enhanced"), GYACH_URL, VERSION, gtk_major_version, 
			gtk_minor_version, gtk_micro_version,
			un.sysname, un.release, un.machine, GYACH_EMAIL, GYACH_URL,
			GYACH_URL,  GYACH_EMAIL_OLD, GYACH_URL_OLD, GYACH_URL_OLD
	);

		strcat(info_data,"</td></tr></table><br>");

		tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
		set_myyahoo_html_data(info_data);
		free(info_data);
		if (tmp_widget) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
			}
		gdk_window_raise(chat_window->window );
}



void  on_profile_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected ) { return ; }
	snprintf(callbackbuf,100, "/profile %s", user_selected);
	chat_command(callbackbuf);					
}

/* added: PhrozenSmoke, callback for opening blank PM windows */
void on_blank_pm_window            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	pm_window=blank_pm_window(NULL);
}


void on_qprofile_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	char *profile=NULL;
	char *ualias=NULL;
	if ( ! user_selected ) {return; }

	if (is_conference) {
		snprintf(callbackbuf, 512,  "\n%s[%s]\n", 
		YAHOO_COLOR_ORANGE,
		_("QUICK PROFILES ARE NOT AVAILABLE IN CONFERENCE ROOMS.")
		);
		append_to_textbox(chat_window,NULL,callbackbuf);
		return; 
				   }

	profile=get_quick_profile(user_selected);
	ualias=get_screenname_alias(user_selected);
	if (!profile) {return;}
	if (!ualias) {return; }
	snprintf(callbackbuf, 1023, 
	"\n%s[%s]\n%s%s ( %s ) : %s\n\n", 
	YAHOO_COLOR_ORANGE, _("QUICK PROFILE"), 
	YAHOO_COLOR_RED, user_selected, ualias, profile
	);
	append_to_textbox(chat_window,NULL,callbackbuf);
	free(profile); profile=NULL;
	free(ualias); ualias=NULL;
}




/* CHANGED: PhrozenSmoke */
void on_pm_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected ) {
		return;
	} else {
		GList *mypm_lpackptr;
		PM_SESSION *mypm_sess;

		if (strcasecmp( user_selected , ymsg_sess->user ) && (! find_profile_name(user_selected ))) {

		/* old way */
		/* pm_window=blank_pm_window(user_selected );  */
					if (( mypm_lpackptr = find_pm_session( user_selected )) != NULL ) {
						mypm_sess = mypm_lpackptr->data;
					} else {
						mypm_sess=new_pm_session(user_selected);
					}
					gtk_widget_show_all(mypm_sess->pm_window );
					gdk_window_raise(mypm_sess->pm_window->window );
														}
		}	
}


void on_copy_room_user_name (GtkMenuItem *menuitem, gpointer nick_user_data) {
	gchar *myalias=NULL;
	if ( ! user_selected ) {return;}
	if (nick_user_data) {
		myalias=get_screenname_alias(user_selected);
			} else {myalias=g_strdup(user_selected);}
	gtk_entry_set_text( GTK_ENTRY(chat_entry), myalias );
	gtk_editable_set_position(GTK_EDITABLE(chat_entry), strlen(myalias));
	if (myalias) {g_free(myalias); myalias=NULL;}
}



void  on_add_to_friends_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;   /* FIX ME - need to do real add here */

	add_friend(user_selected);
}


void
on_ignore_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;

	/* Added: PhrozenSmoke - whether to 'broadcast' to the room we are ignoring a person, 
            or do a 'quiet' ignore and tell no one.  Default is to NOT  'broadcast'  
	    From 'ignore.h'	
	*/ 
	BROADCAST_IGNORE=0;  /* do a quiet ignore unless we explicitly say to 'broadcast' */

	if (! ignore_check( user_selected )) { ignore_toggle( user_selected ); }
}


void
on_ignore_broadcast_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;

	/* Added: PhrozenSmoke - whether to 'broadcast' to the room we are ignoring a person, 
            or do a 'quiet' ignore and tell no one.  Default is to NOT  'broadcast'  
	    From 'ignore.h'	
	*/ 
	BROADCAST_IGNORE=1;  /* do a a LOAD ignore, we explicitly said to 'broadcast' */

	if (! ignore_check( user_selected )) { ignore_toggle( user_selected );  }
	BROADCAST_IGNORE=0;  /* turn 'broadcasting' off */
}



void
on_unignore                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;

	if ( ignore_check( user_selected )) { ignore_toggle( user_selected );  }
}

void
on_unmute                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;

	if ( mute_check( user_selected )) { mute_toggle( user_selected );  }
}

void
on_domute                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! user_selected )
		return;

	if (! mute_check( user_selected )) { mute_toggle( user_selected );  }
}


void on_doimmunity    (GtkMenuItem     *menuitem,  gpointer         user_data)
{
	if ( ! user_selected ) {return;}
	immunity_add(user_selected);
}


void on_add_tmp_friend  (GtkMenuItem   *menuitem, gpointer user_data)
{
	if ( ! user_selected )
		return;
       add_temporary_friend( user_selected );
}



void  on_pm_cancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy( pm_window );
}


void
on_pm_user_map                         (GtkWidget       *widget,
                                        gpointer         user_data)
{
	pm_user = widget;
}


void
on_pm_entry_activate                   (GtkEditable     *editable,
                                        gpointer         user_data)
{
	on_pm_send_clicked( NULL, NULL );
}


void on_login_button_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	logged_in = 0;
	valid_user = 1;
	login_invisible=0;	
	login_noroom=0;
	int java_chat=0;

	login_name_changed=1;

	tmp_widget = lookup_widget( login_window, "chat_port" );
	ymsg_sess->port=atoi( gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)) );

	if ( (ymsg_sess->port<20)  || (ymsg_sess->port>8002) ) {
		show_ok_dialog(_("Invalid Port."));
		ymsg_sess->port= YMSG_CHAT_PORT;
		return;
	}

	tmp_widget = lookup_widget( login_window, "chat_server" );
	strncpy( ymsg_sess->host,
		gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)), 125);

	if (strlen(ymsg_sess->host)<2) {return;} /* no host */ 

	if (last_chatserver) {	free(last_chatserver); }
	last_chatserver=strdup(ymsg_sess->host);

	using_web_login=0;
	if (! strcasecmp(ymsg_sess->host, "wcs2.msg.dcn.yahoo.com")) {using_web_login=1;}
	if (! strcasecmp(ymsg_sess->host, "wcs1.msg.dcn.yahoo.com")) {using_web_login=1;}
	if (! strncasecmp(ymsg_sess->host, "YCHT", 4))  {using_web_login=1; java_chat=1;}
	if (! strncasecmp(ymsg_sess->host, "WEB LOGIN", 9)) {using_web_login=1;}

	if (using_web_login )  {		
		if ( (! gtk_object_get_data( GTK_OBJECT(button), "confirmed" )) && 
		(! web_login_warn_shown)  ) {
			GtkWidget *okbutton;

			snprintf(callbackbuf, 650, "%s\n        '%s'\n%s",
			java_chat?_("The following server uses the 'web login'\nconnection method (Java Chat):"):_("The following server uses the 'web login'\nconnection method:"),
			ymsg_sess->host,
			java_chat?_("This is an experimental, backup connection method,\nsupporting only chat rooms. This server\ndoes not allow you to login as invisible, use conferences\n, change your away status, view webcams, or add/remove\nbuddies on your buddy list. Several other major\nfeatures may also be unavailable. Continue?"):_("This is an experimental, backup connection method.\nThis server does not allow you to login as invisible, change\nyour away status, or add/remove\nbuddies on your buddy list. Several other major\nfeatures may also be unavailable. Continue?")
			);

			okbutton=show_confirm_dialog(callbackbuf,"YES","NO");
			if (!okbutton) {return;}
  			gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      	GTK_SIGNAL_FUNC (on_login_button_clicked), NULL);
   			gtk_object_set_data( GTK_OBJECT(okbutton), "confirmed", "OK");
			web_login_warn_shown=1;
			return;
																									  } else {
			tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
			if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
																												}
							   }

	tmp_widget = lookup_widget( login_window, "chat_username" );
	strncpy( ymsg_sess->user,
		gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)), 60);

	if (last_loginname) {
		if (!strcasecmp(last_loginname, ymsg_sess->user)) {login_name_changed=0;}
	}

	if ( (! login_name_changed) && (login_credentials) ) {
			if (verify_passphrase) {free(verify_passphrase); verify_passphrase=NULL;}
			verify_passphrase=strdup( gtk_entry_get_text( GTK_ENTRY(login_credentials)) );
			if (strlen(verify_passphrase)<1) {return;}	
	      } else {
		if (last_credentials_key) {free(last_credentials_key);}
		last_credentials_key=NULL;
		if (verify_passphrase) {free(verify_passphrase); verify_passphrase=NULL;}
		}

	if (last_loginname) {free(last_loginname);}
	last_loginname=strdup(ymsg_sess->user);

	tmp_widget = lookup_widget( login_window, "chat_password" );
	strncpy( ymsg_sess->password,
		gtk_entry_get_text( GTK_ENTRY(tmp_widget)), 30 );

	if (strlen(ymsg_sess->password)<1) {
		show_ok_dialog(_("Incorrect password."));
		return;
													}

	if ( password ) {
		free( password );
		password = NULL;
	}

	/* remember password, will be saved when we write_config() */
	if ( remember_password ) {
		password = strdup( ymsg_sess->password );
	}

	tmp_widget = lookup_widget( login_window, "chat_room" );
	strncpy( ymsg_sess->req_room,
		gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)), 62);

	strncpy( ymsg_sess->room,  "[NONE]", 62);

	if (last_chatroom) {free(last_chatroom); }
	last_chatroom=strdup(ymsg_sess->req_room);

	tmp_widget = lookup_widget( login_window, "noroom_button" );
	login_noroom=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( login_window, "invis_button" );
	login_invisible=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	write_config();

	gtk_widget_destroy( login_window );
	login_window=NULL;

	if (verify_passphrase && last_credentials_key) {
		if ( ! unlock_yahoo_account(ymsg_sess, verify_passphrase)) {	
			login_window = build_login_window();
			gtk_widget_show_all(login_window);
			return;
			}
		if (last_credentials_key) {free(last_credentials_key);}		
		last_credentials_key=NULL;
	}

	login_credentials=NULL;
	login_to_yahoo_chat();
}


void on_cancel_login_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	username[0] = '\0';
	gtk_widget_destroy( login_window );
	login_window=NULL;
	valid_user = 1;

	logged_in = 0;
}


void
on_cancel_favs_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	gtk_widget_destroy( favorites_window );
	favorites_window = NULL;
}



void
on_close_profile_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( ! profile_window )
		return;

	gtk_widget_destroy( profile_window );
	profile_window = NULL;
}


void
on_alias_edit_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	edit_alias();
}


void on_alias_delete_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	selection=gtk_tree_view_get_selection(chat_aliases);
	if (! gtk_tree_selection_get_selected(selection, &model, &iter)) {return;}
	gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
	alias_selected = -1;
	save_aliases();
}


void
on_alias_new_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkTreeSelection *selection;
	selection=gtk_tree_view_get_selection(chat_aliases);
	gtk_tree_selection_unselect_all(selection);
	alias_selected = -1;
	edit_alias();
}


void
on_alias_send_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *alias_ptr;
	char *alias_tmp;
	selection=gtk_tree_view_get_selection(chat_aliases);
	if (! gtk_tree_selection_get_selected(selection, &model, &iter)) {return;}

	gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL1, &alias_ptr, -1);
	alias_tmp=strdup(alias_ptr);
	if ( user_selected ) {
		check_alias( alias_tmp, user_selected );
	} else {
		check_alias(alias_tmp, "" );
	}
	g_free(alias_ptr);
	free(alias_tmp);
}


void
on_alias_save_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *alias_name;
	gchar *cmd_value1;
	gchar *cmd_value2;
	GtkWidget *tmp_widget;

	if ( alias_window ) {
		/* save the entry */
	
	selection=gtk_tree_view_get_selection(chat_aliases);
	
	if (! gtk_tree_selection_get_selected(selection, &model, &iter)) {
		/* NOthing selected, so append */
		gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model(chat_aliases)), &iter);
	} 


		tmp_widget = lookup_widget( alias_window, "alias_name" );
		snprintf(callbackbuf, 11, "%s", gtk_entry_get_text(GTK_ENTRY(tmp_widget)) );
		alias_name=strdup(callbackbuf);
		tmp_widget = lookup_widget( alias_window, "cmd_value1" );
		cmd_value1=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));
		tmp_widget = lookup_widget( alias_window, "cmd_value2" );
		cmd_value2=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

			gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model(chat_aliases)), &iter,  
			GYTRIPLE_COL1, alias_name, GYTRIPLE_COL2, cmd_value1, 
			GYTRIPLE_COL3, cmd_value2, -1);

		alias_selected = -1;
		gtk_tree_selection_unselect_all(selection);
		free(cmd_value2);   free(cmd_value1);   free(alias_name);
		save_aliases();

		/* now get rid of the window */
		on_alias_cancel_clicked( button, user_data );
	}
}


void
on_alias_cancel_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( alias_window ) {
		gtk_widget_hide( alias_window );
		gtk_widget_destroy( alias_window );
		alias_window = NULL;
	}
}



void  on_follow_activate   (GtkMenuItem *menuitem, gpointer  user_data)
{
	if ( user_selected ) {
		snprintf( callbackbuf, 200, "/follow %s", user_selected );
		try_command( callbackbuf );
	}
}



gboolean
on_chat_entry_key_press_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	GtkTextBuffer *text_buffer;
	GtkClipboard *clip;
	GtkWidget *tmp_widget;

	if ( event->type == GDK_KEY_PRESS ) {

		/* printf( "keyval pressed = %d\n", event->key.keyval ); fflush( stdout ); */


		switch( event->key.keyval ) {
			case 65289:	/* tab, try to expand */
						tab_complete();
						return( TRUE );
						break;

			case 65362:	/* up arrow, scroll up to previous message */
						gtk_entry_set_text( GTK_ENTRY(widget), history_prev());
						return( TRUE );
						break;

			case 65364:	/* down arrow, scroll down to next message */
						gtk_entry_set_text( GTK_ENTRY(widget), history_next());
						return( TRUE );
						break;

			case    99: /* c, try to copy if CTRL-C */
						if ( event->key.state & GDK_CONTROL_MASK ) {
							tmp_widget = ct_widget();

							text_buffer = gtk_text_view_get_buffer(
								GTK_TEXT_VIEW(tmp_widget) );
							clip = gtk_clipboard_get( GDK_SELECTION_CLIPBOARD );

							/* fixed: PhrozenSmoke,  allow smart use of Ctrl+C on 
								both the GtkEntry and TextBuffer, depending on what's
								selected */
							/* Clear old text we may have copied before */
							/* This allows us to truly see if we are trying to 
								copy selected data in the TextBuffer on in the Entry */

							gtk_clipboard_clear(clip);

							/* Try copying the GtkEntry first */
							gtk_editable_copy_clipboard  (GTK_EDITABLE(widget));

							/* Check if text was actually copied from the GtkEntry */
							if (! gtk_clipboard_wait_is_text_available (clip))  {
								/* Nothing was copied! Copy from the TextBuffer... */
								gtk_text_buffer_copy_clipboard( text_buffer, clip );
																										}

							/*  printf("\nclip:  %d\n\n", gtk_clipboard_wait_is_text_available (clip) ); fflush(stdout);   */

							return( TRUE );
						}
						break;

			case   102: /* f, try to find if CTRL-F */
						if ( event->key.state & GDK_CONTROL_MASK ) {
							 jump_to_chat_tab();

							search_pos = -1;
							if ( search_text ) {
								free( search_text );
								search_text = NULL;
							}

							if ( ! find_window ) {
								find_window = create_find_window();
							} else {
								if ( search_text ) {
									tmp_widget =lookup_widget( find_window,
										"search_text" );
									gtk_entry_set_text(GTK_ENTRY(tmp_widget),
										search_text );

									tmp_widget = lookup_widget( find_window,
										"case_sensitive" );
									gtk_toggle_button_set_active(
										GTK_TOGGLE_BUTTON(tmp_widget) ,
										search_case_sensitive );
								}
							}
							gtk_widget_show_all( find_window );
							return( TRUE );
						}
						break;

			case   103: /* g, try to find next if CTRL-G */
						if ( event->key.state & GDK_CONTROL_MASK ) {
							search_chat_text( search_case_sensitive );
							return( TRUE );
						}
						break;

			case   118: /* v, try to past if CTRL-V */
						/* since gtk_entry has a gtk_editable, it already */
						/* implements CTRL-V so we do nothing here */
						if ( event->key.state & GDK_CONTROL_MASK ) {
						}
						break;

		}
	}

	return FALSE;
}


void
on_save_current_text_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	char fldate[24];
	time_t timell_now;
	struct tm *tmll_now;

	if ( file_sel )
		return;

	file_sel = create_fileselection();
	if ( ! file_sel )
		return;

	gtk_widget_show( file_sel );
	timell_now = time( NULL );
	tmll_now = localtime( &timell_now );
	if (! strftime(fldate, 22, "_%b-%d-%Y",tmll_now ) ) {sprintf(fldate,"%s","");}
	snprintf(callbackbuf, 300, "%s/gyachE-capture%s.txt", GYACH_CFG_DIR, fldate );
	gtk_file_selection_set_filename(GTK_FILE_SELECTION (file_sel),
	        callbackbuf );
	gtk_window_set_title( GTK_WINDOW(file_sel), FILE_MSG_SAVE );
}


void
on_log_to_file_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	char fldate[24];
	time_t timell_now;
	struct tm *tmll_now;

	/* see if we are closing the file */
	if ( capture_fp ) {
		fclose( capture_fp );
		capture_fp = NULL;

		free( capture_filename );
		capture_filename = NULL;

		tmp_widget = lookup_widget( chat_window, "log_to_file" );
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 0 );
		return;
	}

	if ( file_sel )
		return;

	file_sel = create_fileselection();
	if ( ! file_sel ) {
		tmp_widget = lookup_widget( chat_window, "log_to_file" );
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 0 );
		return;
	}

	gtk_widget_show( file_sel );
	timell_now = time( NULL );
	tmll_now = localtime( &timell_now );
	if (! strftime(fldate, 22, "_%b-%d-%Y",tmll_now ) ) {sprintf(fldate,"%s","");}
	snprintf( callbackbuf, 300, "%s/gyachE-LOG%s.txt", GYACH_CFG_DIR,fldate );
	gtk_file_selection_set_filename(GTK_FILE_SELECTION (file_sel),
	        callbackbuf );
	gtk_window_set_title( GTK_WINDOW(file_sel), FILE_MSG_LOG );
}


void
on_status_here_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{

	if (( my_status != 0 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))))
		cmd_away( "0" );
}


void
on_status_be_right_back_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 1 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "1" );
}


void
on_status_busy_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 2 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "2" );
}


void
on_status_not_at_home_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 3 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "3" );
}


void
on_status_not_at_my_desk_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 4 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "4" );
}


void
on_status_not_in_the_office_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 5 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "5" );
}


void
on_status_on_the_phone_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 6 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "6" );
}


void
on_status_on_vacation_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 7 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "7" );
}


void
on_status_out_to_lunch_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 8 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "8" );
}


void
on_status_stepped_out_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 9 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "9" );
}


void
on_status_invisible_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 12 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "12" );
}


/* added, PhrozenSmoke : Idle away */


void on_status_idle_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	if (( my_status != 999 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "999" );
}



void
on_file_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	time_t time_now;

	if ( ! file_sel )
		return;

	capture_filename = strdup(
		gtk_file_selection_get_filename( GTK_FILE_SELECTION( file_sel )));

	if ( ! strcmp( GTK_WINDOW(file_sel)->title, FILE_MSG_SAVE )) {
		/* fixme, what about error opening */
		/* fixed: PhrozenSmoke, error dialog shows from gyach_int_text.c */
		capture_text_to_file( capture_filename );
		free( capture_filename );
	} else if ( ! strcmp( GTK_WINDOW(file_sel)->title, FILE_MSG_LOG )) {
		capture_fp = fopen( capture_filename, "ab" );
		if ( capture_fp ) {
			time_now = time(NULL);
			fprintf( capture_fp, "\nGyach Enhanced, version %s\nCopyright (c) 2003-2006, Erica Andrews\n%s\nLicense: GNU General Public License\n\nCONNECTION LOG\n", VERSION, GYACH_URL );
			fprintf( capture_fp, "%s", ctime(&time_now) );
			fprintf( capture_fp, "\n_____________________\n\n" );
			fflush( capture_fp );
		} else {
			/* fixme, how about an error message?? */
			/* fixed: PhrozenSmoke, added dialog box */
			show_ok_dialog(_("Error opening file for saving."));
			tmp_widget = lookup_widget( chat_window, "log_to_file" );
			gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 0);
		}
	}

	gtk_widget_destroy( file_sel );
	file_sel = NULL;
}


void
on_file_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	if ( ! strcmp( GTK_WINDOW(file_sel)->title, FILE_MSG_LOG )) {
		tmp_widget = lookup_widget( chat_window, "log_to_file" );
		gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget), 0 );
	}

	gtk_widget_hide( file_sel );
	gtk_widget_destroy( file_sel );
	file_sel = NULL;
}


void on_setup_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if ( ! setup_window ) {
		setup_window = create_setup_window();
		set_setup_options();
	}
	/* finally show the window */
	gtk_widget_show_all( setup_window );

	/* Show a specific tab first? */
	if (user_data) {
		GtkWidget *tmp_widget=NULL;
		char *sel_tab=(char *) user_data;
		tmp_widget=gtk_object_get_data(GTK_OBJECT(setup_window), "notebook1");
		if (!tmp_widget) {return;}
		if ( (!strcmp(sel_tab,"color")) || (!strcmp(sel_tab,"font")) ) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 1);
			if (!strcmp(sel_tab,"font")) {
				GtkWidget *tmp_wid=NULL;
				tmp_wid=gtk_object_get_data(GTK_OBJECT(setup_window), "notebook2");
				if (tmp_wid) {gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_wid),1);}
			}
		}
	}
}

void on_setup_activate_button    (GtkButton       *button,  gpointer         user_data)
{
on_setup_activate  (NULL,user_data);
}


void on_save_setup_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( ! setup_window )
		return;

	get_setup_options();
	set_text_entry_styles();
	write_config();

	/* destroy the window */
	gtk_widget_destroy( setup_window );
	setup_window = NULL;
}


void
on_cancel_setup_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( ! setup_window )
		return;

	gtk_widget_destroy( setup_window );
	setup_window = NULL;
}


void
on_status_custom_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 10 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) )) {
		if ( custom_away_message )
			cmd_away( custom_away_message );
		else
			cmd_away( "Be Right Back" );
	}
}

void on_addfavroom_insert_activate  (GtkMenuItem  *menuitem, gpointer  user_data)
{
	GList *this_item = NULL;
	 if (!room_selected) {return;}

	this_item = favroom_list;
	while( this_item ) {
		if ( ! strcmp( this_item->data, room_selected )) {
			snprintf(callbackbuf, 350, "  %s%s[%s: '%s']%s%s\n" ,
				YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED,
				_("The following room is already in your favorites list"), 
				room_selected,
				YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK
				  );
			append_to_textbox_color(chat_window,NULL,callbackbuf);
			return;
		}
		this_item = g_list_next( this_item );
	}
	favroom_list = g_list_append( favroom_list, strdup( room_selected ));
	write_config();
			snprintf(callbackbuf, 350, "  %s%s[%s: '%s']%s%s\n" ,
				YAHOO_STYLE_ITALICON, YAHOO_COLOR_ORANGE,
				_("The following room has been added to your favorites list"), 
				room_selected,
				YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK
				  );
			append_to_textbox_color(chat_window,NULL,callbackbuf);
}




void
on_cancel_room_clicked  (GtkButton *button, gpointer user_data)
{
	if ( room_window ) {gtk_widget_hide( room_window );	}
}



void collect_roomlist_selected_room()  {
		GtkTreeModel *model;
		GtkTreeIter iter;		
		GtkTreeSelection *selection;
			
		if (!room_window) {return;}
		selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(lookup_widget( room_window, "room_tree" )));
		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			ROOM_ENTRY *room_item;
			gtk_tree_model_get(model, &iter,GYROOM_DATA, &room_item, -1);		

			if ( room_selected ) {
				free( room_selected );  
				room_selected=NULL;
			}
			if (room_item) {
				if ((!room_item->top_level) && room_item->room_name && 
					strcmp(room_item->room_name, _("---- downloading list ----")) ) {
						room_selected=strdup( room_item->room_name );
					}
			}
		}
}

void display_room_lister_popup() {
		collect_roomlist_selected_room();
		if (!room_selected) {return ;}
		if (!favroom_setup_menu) {favroom_setup_menu=create_addfavroom_menu();}

		/* if the Setupwindow is running, we need to save all options and 
			hide it */
		if (setup_window) {
			get_setup_options();
			set_text_entry_styles();
			write_config();
			/* destroy the window */
			gtk_widget_destroy( setup_window );
			setup_window = NULL;
		}

		gtk_widget_show_all(GTK_WIDGET(favroom_setup_menu));
		/* right clicked popup action window */
		gtk_menu_popup((GtkMenu *) favroom_setup_menu,
			NULL, NULL, NULL, NULL, 1, 0);
}

gboolean on_room_tree_kb_popup (GtkWidget  *widget, gpointer user_data) {
	/* Shift-F10 pressed */
	display_room_lister_popup();
	return TRUE;
}


void on_goto_room_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	collect_roomlist_selected_room();
	if (!room_selected) {return;}
	snprintf( callbackbuf, 128, "/join %s", room_selected );
	set_current_chat_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(GTK_WIDGET(button))))  );
	chat_command( callbackbuf );
	if ( auto_close_roomlist )
		gtk_widget_hide( room_window );
}

void onRoomListActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
				collect_roomlist_selected_room();
				if (!room_selected) {return;}
				snprintf( callbackbuf, 128 , "/join %s", room_selected );
	set_current_chat_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(GTK_WIDGET(tv))))  );
				chat_command( callbackbuf );
				if ( auto_close_roomlist ) {gtk_widget_hide( room_window );}
}


gboolean on_room_tree_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	/* Right mouse button, PhrozenSmoke */
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		display_room_lister_popup();
		}

	return FALSE;
}


void
on_setup_insert_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	if ( edit_win ) {return;}
	if (! setup_list) {return;}

	edit_win = create_edit_ignore_window();
		gtk_window_set_position (GTK_WINDOW (edit_win ), GTK_WIN_POS_CENTER);
		gtk_window_set_modal (GTK_WINDOW (edit_win ), 1);
	gtk_widget_show_all( edit_win );

	tmp_widget = lookup_widget( edit_win, "edit_ignore_label" );
	if ( ! strcmp( setup_which, "friend_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add Friend:") );
	} else if ( ! strcmp( setup_which, "regex_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add Ignore Regex:") );
	} else if ( ! strcmp( setup_which, "ignore_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add User To Ignore:") );
	} else if ( ! strcmp( setup_which, "login_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add Yahoo Login:" ));
	} else if ( ! strcmp( setup_which, "favroom_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add Favorite Room:") );
	/* added, PhrozenSmoke */
	} else if ( ! strcmp( setup_which, "fav_room_list" )) {
		gtk_label_set_text( GTK_LABEL(tmp_widget), _("Add Favorite Room:") );
	}

	tmp_widget = lookup_widget( edit_win, "edit_ignore_text" );
	gtk_entry_set_text( GTK_ENTRY(tmp_widget), "" );
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}
	if (setup_menu) {gtk_menu_popdown((GtkMenu *) setup_menu);}
}


void collect_setup_list_selected()  {
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		if (!setup_list) {
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}
		return;
		}
		selection=gtk_tree_view_get_selection(setup_list);
		if ( gtk_tree_selection_get_selected(selection, &model, &setupiter)) {
			gchar *sfound;
			gtk_tree_model_get(model, &setupiter,GYSINGLE_COL1, &sfound, -1);		
			if ( setup_row_selected ) {	free( setup_row_selected );  }
			setup_row_selected=strdup(sfound);
			g_free(sfound);
		} else {
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}
		}
}


void
on_setup_delete_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (! setup_list) {return;}
	collect_setup_list_selected();

	if ( setup_row_selected) {

		if ( ! strcmp( setup_which, "friend_list" )) {
			if ( using_web_login || ycht_is_running() ) {
				show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
			} else { remove_friend(setup_row_selected); }
		}

		/* added PhrozenSmoke */
		if ( ! strcmp( setup_which, "fav_room_list" )) {
			GList *this_item = NULL;
			this_item = favroom_list;
			while( this_item ) {
				if ( ! strcmp( this_item->data, setup_row_selected )) {
					favroom_list = g_list_remove_link( favroom_list, this_item );
					gyach_g_list_free( this_item );
					break;
																	  			  }
				this_item = g_list_next(this_item);
									  }
		}

		/* added, PhrozenSmoke - makes the setup ignore list deletion actually functional */
		if ( ! strcmp( setup_which, "ignore_list" )) {
			snprintf(callbackbuf,128,"/ignore %s", setup_row_selected );
			chat_command(callbackbuf);
		}

		gtk_list_store_remove(GTK_LIST_STORE(gtk_tree_view_get_model(setup_list)), &setupiter);
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}

	if ( ! strcmp( setup_which, "fav_room_list" )) {write_config();}
	}
	if (setup_menu) {gtk_menu_popdown((GtkMenu *) setup_menu);}
}


void on_setup_edit_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	if ( edit_win ) {return;}
	if (! setup_list) {return;}
	collect_setup_list_selected();

	if ( setup_row_selected ) {

		edit_win = create_edit_ignore_window();
		gtk_window_set_position (GTK_WINDOW (edit_win ), GTK_WIN_POS_CENTER);
		gtk_window_set_modal (GTK_WINDOW (edit_win ), 1);
		gtk_widget_show_all( edit_win );

		tmp_widget = lookup_widget( edit_win, "edit_ignore_label" );
		if ( ! strcmp( setup_which, "friend_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Friend") );
		} else if ( ! strcmp( setup_which, "regex_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Ignore Regex") );
		} else if ( ! strcmp( setup_which, "ignore_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Ignored User") );
		} else if ( ! strcmp( setup_which, "favroom_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Favorite Room") );
		} else if ( ! strcmp( setup_which, "login_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Yahoo Login") );
		/* added, PhrozenSmoke */
		} else if ( ! strcmp( setup_which, "fav_room_list" )) {
			gtk_label_set_text( GTK_LABEL(tmp_widget), _("Edit Favorite Room") );
		}

		tmp_widget = lookup_widget( edit_win, "edit_ignore_text" );
		gtk_entry_set_text( GTK_ENTRY(tmp_widget),setup_row_selected );
	}

	if (setup_menu) {gtk_menu_popdown((GtkMenu *) setup_menu);}
}

void display_setup_list_popup(GtkWidget *widget) {
		GtkWidget *tmp_widget;
		setup_list = NULL;
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}

		tmp_widget = lookup_widget( setup_window, "friend_list" );
		if ( tmp_widget == widget )
			strncpy( setup_which, "friend_list", 30 );

		tmp_widget = lookup_widget( setup_window, "regex_list" );
		if ( tmp_widget == widget )
			strncpy( setup_which, "regex_list", 30 );

		tmp_widget = lookup_widget( setup_window, "ignore_list" );
		if ( tmp_widget == widget )
			strncpy( setup_which, "ignore_list", 30 );

		tmp_widget = lookup_widget( setup_window, "login_list" );
		if ( tmp_widget == widget )
			strncpy( setup_which, "login_list" , 30);

		tmp_widget = lookup_widget( setup_window, "favroom_list" );
		if ( tmp_widget == widget )
			strncpy( setup_which, "favroom_list", 30 );

		setup_list = GTK_TREE_VIEW(widget);
		collect_setup_list_selected();
		if (!setup_menu) {setup_menu=create_setup_menu();}
		gtk_widget_show_all(GTK_WIDGET(setup_menu));

		/* right clicked popup action window */
		gtk_menu_popup((GtkMenu *) setup_menu,
			NULL, NULL, NULL, NULL, 1, 0);

		/* printf("Menu popped up:  %s\n", setup_menu ? "is not null":"is null");
		fflush(stdout);
		printf("Menu popped up-tmp_widget:  %s\n", tmp_widget ? "is not null":"is null");
		fflush(stdout); */ 
}

gboolean on_setup_list_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		display_setup_list_popup(widget);
	}

	return FALSE;
}

void on_setuplist_tree_kb_popup(GtkWidget *widgy, gpointer user_data) {
	display_setup_list_popup(widgy);
}


void on_edit_ig_save_clicked                (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	char *answer;
	GdkPixbuf *imbuf=NULL;

	if (! setup_list) {return;}
	tmp_widget = lookup_widget( edit_win, "edit_ignore_text" );

	if ( setup_row_selected ) {
		/* editing a current row, so delete first */
		if ( ! strcmp( setup_which, "friends_list" )) {
			if ( using_web_login || ycht_is_running() ) {
				show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
			} else {	remove_friend( setup_row_selected ); }
		}

		/* added PhrozenSmoke */
		if ( ! strcmp( setup_which, "fav_room_list" )) {
			GList *this_item = NULL;
			this_item = favroom_list;

			while( this_item ) {
				if ( ! strcmp( this_item->data, setup_row_selected )) {
					favroom_list = g_list_remove_link( favroom_list, this_item );
					gyach_g_list_free( this_item );
					break;
																	  			  }
				this_item = g_list_next(this_item);
									  }
		}

		gtk_list_store_remove(GTK_LIST_STORE(gtk_tree_view_get_model(setup_list)), &setupiter);
			if ( setup_row_selected ) {
				free( setup_row_selected );  
				setup_row_selected=NULL;
			}
	}

	answer=strdup(gtk_entry_get_text( GTK_ENTRY(tmp_widget)));
	gtk_list_store_append(GTK_LIST_STORE(gtk_tree_view_get_model(setup_list)), &setupiter);

		if ( ! strcmp( setup_which, "friend_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here_male);
		} else if ( ! strcmp( setup_which, "regex_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
		} else if ( ! strcmp( setup_which, "ignore_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
		} else if ( ! strcmp( setup_which, "favroom_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_pm_join);
		} else if ( ! strcmp( setup_which, "login_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here);
		} else if ( ! strcmp( setup_which, "fav_room_list" )) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_pm_join);
		}

	gtk_list_store_set(GTK_LIST_STORE(gtk_tree_view_get_model(setup_list)), &setupiter, 
			GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, answer,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "",  -1);
	if (imbuf) {g_object_unref(imbuf);}

	gtk_tree_selection_select_iter(gtk_tree_view_get_selection(setup_list), &setupiter);
	if ( edit_win ) {gtk_widget_destroy( edit_win );}
	edit_win = NULL;
	if ( ! strcmp( setup_which, "friend_list" )) {add_friend( answer );}

	/* added PhrozenSmoke */
	if ( ! strcmp( setup_which, "fav_room_list" )) {
		favroom_list = g_list_append( favroom_list, strdup(answer));
		write_config();
	}
   free(answer);
}


void
on_edit_ig_cancel_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( edit_win )
		gtk_widget_destroy( edit_win );

	edit_win = NULL;
}


void
on_status_autoaway_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (( my_status != 11 ) &&
		(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem)) ))
		cmd_away( "11" );
}


gboolean
on_pm_window_destroy_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if (pm_window) {gtk_widget_destroy(pm_window);}
	pm_window=NULL;
  return FALSE;
}


gboolean
on_favorites_destroy_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( favorites_window )
		gtk_widget_destroy( favorites_window );

	favorites_window = NULL;

	return( TRUE );
}


gboolean
on_profile_window_destroy_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( profile_window )
		gtk_widget_destroy( profile_window );

	profile_window = NULL;

	return( TRUE );
}


gboolean
on_alias_window_destroy_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{

  return FALSE;
}


gboolean
on_setup_window_destroy_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( setup_window )
		gtk_widget_destroy( setup_window );

	setup_window = NULL;

	return( TRUE );
}


gboolean
on_edit_ignore_window_destroy_event    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( edit_win )
		gtk_widget_destroy( edit_win );

	edit_win = NULL;

	return( TRUE );
}


gboolean
on_room_list_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( room_window ) {
		gtk_widget_hide( room_window );
	}

	return( TRUE );
}


void
on_exit_clicked                    (GtkButton *button,
                                        gpointer         user_data)
{
	on_quit_activate( NULL, NULL );
}


void
on_tgl_bold_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	use_bold = gtk_toggle_button_get_active( togglebutton );
}


void
on_tgl_italics_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	use_italics = gtk_toggle_button_get_active( togglebutton );
}


void
on_tgl_underline_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	use_underline = gtk_toggle_button_get_active( togglebutton );
}




/* added PhrozenSmoke: a SHARED method for sending PM text, it seems kind of dumb 
     that we had two large callbacks for sending text with the 'Send' button or by sending 
     text by hitting 'Enter' - let's be sensible and use ONE method for sending PM text - 
     before this, on_pms_send_activate and on_pms_entry_activate were re-writes of 
     the same method, just looking up different widgets.  Plus, the methods were inconsistent 
     'on_pms_entry_activate would bring us back from 'away' but on_pms_send_activate 
     didn't - let's keep behavior consistent */

/*  Also, this new shared method finally adds support for fonts, font sizes, colors, 
      faders, and smileys in PMs, instead of plain boring black text */

void pm_clear_text_view(GtkWidget *textv) {
	GtkTextIter tstart;
	GtkTextIter tend;
	int text_length;
	GtkTextBuffer *text_buffer;
	text_buffer=gtk_text_view_get_buffer( GTK_TEXT_VIEW(textv) );
	text_length = gtk_text_buffer_get_char_count( text_buffer );
	gtk_text_buffer_get_iter_at_offset( text_buffer, &tstart, 0 );
	gtk_text_buffer_get_iter_at_offset( text_buffer, &tend, text_length );
	gtk_text_buffer_delete( text_buffer, &tstart, &tend );
	gtk_text_buffer_get_end_iter( text_buffer, &tend );
	gtk_text_buffer_create_mark( text_buffer, "end_mark", &tend, 0 );
}

void send_pm_session_text    (PM_SESSION *pm_sess)
{
	GtkWidget *pms_entry;
	char *text;
	char buf[1408] = "/tell ";   /* leave extra space for formatting */
	char textbuf[1152]="";
	char fontbuf[150]="";
	char colbuf[20]="";
	char *fade1=NULL;
	char *fade2=NULL;
	char *tuxviron=NULL;
	char *myalias=NULL;
	/* Support pm '/' commands /t, /e, /clear - think, emote, clear */
	int pm_command=0;
	int search_res=0;


		pms_entry = lookup_widget( pm_sess->pm_window, "pms_entry" );

		if (pm_sess->window_style==1) {
			int  length;
			GtkTextIter start;
			GtkTextIter end;
			gchar *ptr;
			GtkTextBuffer *mybuffer;
			mybuffer=gtk_object_get_data (GTK_OBJECT (pms_entry), "textbuffer");
			length = gtk_text_buffer_get_char_count( mybuffer );
			gtk_text_buffer_get_iter_at_offset( mybuffer, &start, 0 );
			gtk_text_buffer_get_iter_at_offset(mybuffer, &end, length );
			ptr = gtk_text_buffer_get_text(mybuffer, &start, &end, 0 );
			text = strdup(ptr);
			g_free( ptr );
			pm_clear_text_view(pms_entry);
		} else {
			text = strdup(gtk_entry_get_text(GTK_ENTRY(pms_entry)));
			gtk_entry_set_text(GTK_ENTRY(pms_entry), "");
				}

		if ( ! strlen( text ))  {
			free(text);   /* added PhrozenSmoke, seg fault watch */
			return;

										}

		if ( (!strncasecmp(text,"/clear", 6)) || 
			(!strncasecmp(text,"/clr", 4)) ) { /* request to clear PM box */
			pm_clear_text_view(pm_sess->pm_text);
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			pm_sess->typing = 0;
			ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
			reset_current_pm_profile_name();
			return; 
		}

		if ( (!strncasecmp(text,"/date", 5)) || (!strncasecmp(text,"/time", 5)) ) { 
			time_t time_llnow= time(NULL);
			snprintf(buf,3, "%s","\n");
			append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );
			append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
			snprintf(buf,50, "%s\n", ctime(&time_llnow ));
			append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			pm_sess->typing = 0;
			ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
			reset_current_pm_profile_name();
			return; 
		}


		if ( ! strncasecmp(text,"/help", 5) ) { /* show help */
			snprintf( buf, 1000, "\n%s/adv - Send Gyach-E URL info\n/clear - Clear this window\n/clr - Clear this window\n/e - Send *emote* text\n/t - Send . o O ( think ) text\ns: [term] - search the web for 'term' and share\n/date - show current data/time locally\n/time - show current data/time locally\n/ver - Send Gyach-E URL info\n/help - Show PM help\n", YAHOO_COLOR_PMPURPLE
			 );
			append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			pm_sess->typing = 0;
			ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
			reset_current_pm_profile_name();
			return; 
		}

		if (!strncasecmp(text,"/t ", 3)) { /* 'Think' text */
			pm_command=1;
		}
		if (!strncasecmp(text,"/e ", 3)) { /* 'Emote' text */
			pm_command=2;
		}

		if (!strncasecmp(text,"s: ", 3)) { /* web search */
			search_res=1;
		}

		if ( (!strncasecmp(text,"/ver", 4)) || (!strncasecmp(text,"/adv", 4)) ) {  
				struct utsname un;
				uname( &un );
				 /* Gyach-E version */ 
			snprintf( textbuf, 450, 
				"%sGYach Enhanced %s version %s  [%s%s%s] under %s  %s.  GYach Enhanced supports webcams, faders, Y! 'nicknames', and %spY! Voice Chat%s (the FIRST Y! voice chat program for Linux.)",
				YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF, VERSION,
				YAHOO_STYLE_URLON, GYACH_URL, YAHOO_STYLE_URLOFF,
				un.sysname, un.release, YAHOO_STYLE_BOLDON, YAHOO_STYLE_BOLDOFF);

			free(text);
			text=strdup(textbuf);
			snprintf(textbuf, 3,"%s","");
										}

	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

		strncat( buf, pm_sess->pm_user , 75);
		strcat( buf, " " );

		/* added by PhrozenSmoke:  add formatting to PM messages */

	if ( use_color && 
	  (! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) ) {
		if ( ! strcmp( use_color, "black" )) {
			strcat( textbuf, YAHOO_COLOR_BLACK );
		} else if ( ! strcmp( use_color, "blue" )) {
			strcat( textbuf, YAHOO_COLOR_BLUE );
		} else if ( ! strcmp( use_color, "cyan" )) {
			strcat( textbuf, YAHOO_COLOR_CYAN );
		} else if ( ! strcmp( use_color, "gray" )) {
			strcat( textbuf, YAHOO_COLOR_GRAY );
		} else if ( ! strcmp( use_color, "green" )) {
			strcat( textbuf, YAHOO_COLOR_GREEN );
		} else if ( ! strcmp( use_color, "pink" )) {
			strcat( textbuf, YAHOO_COLOR_PINK );
		} else if ( ! strcmp( use_color, "purple" )) {
			strcat( textbuf, YAHOO_COLOR_PURPLE );
		} else if ( ! strcmp( use_color, "yellow" )) {
			strcat( textbuf, "\033[#ffff00m" );
		} else if ( ! strcmp( use_color, "orange" )) {
			strcat( textbuf, "\033[#ffc800m" );
		} else if ( ! strcmp( use_color, "red" )) {
			strcat( textbuf, YAHOO_COLOR_RED );
		} else if ( ! strcmp( use_color, "custom" )) {
			snprintf( colbuf, 19, "\033[#%02x%02x%02xm",
				custom_color_red, custom_color_green, custom_color_blue );			
				strcat(textbuf,colbuf);	
		}
	}

	if (strstr(text,YAHOO_FONT_TAG_START)==NULL) {
		/* only append if 'cmd' contains no 'font' tag */
		snprintf( fontbuf, 148, "%s %s%s\" %s%d\">",
			YAHOO_FONT_TAG_START, YAHOO_FONT_FACE_START, 
			font_family?font_family:"Sans",
			YAHOO_FONT_SIZE_START, font_size );
			strcat(textbuf,fontbuf);
																				 }

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) 
			{  
			 /* fader use requested */
			if (strlen(gtk_entry_get_text(GTK_ENTRY(fader_text_start)))>0) {
				fade1=strdup(gtk_entry_get_text(GTK_ENTRY(fader_text_start)));
				strncat(textbuf,fade1, 180);
																													 }
			} 

		if ( use_bold )
			strcat( textbuf, YAHOO_STYLE_BOLDON );

		if ( use_italics )
			strcat( textbuf, YAHOO_STYLE_ITALICON );

		if ( use_underline )
			strcat( textbuf, YAHOO_STYLE_UNDERLINEON );		

			if (pm_command==2) { /* emote */  strcat(textbuf,"** ");}
			if (pm_command==1) { /* think */  strcat(textbuf,". o O ( ");}
			if (pm_command) { 
				strncat( textbuf, text+3, 427 ); 
			} else {strncat( textbuf, text, 427 ); } /* plain text */
			if (pm_command==2) { /* emote */  strcat(textbuf," **");}
			if (pm_command==1) { /* think */  strcat(textbuf," )");}

		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) 
			{  
			 /* fader use requested */
			if (strlen(gtk_entry_get_text(GTK_ENTRY(fader_text_end)))>0) {
				fade2=strdup(gtk_entry_get_text(GTK_ENTRY(fader_text_end))) ;
				strncat(textbuf,fade2, 25);	
																													} 
			}			

			sharing_bimages=pm_sess->buddy_image_share;

			if (pm_sess->sms)   {
					/* just to be safe, turn encryption off is the recipient is on SMS */ 
					pm_sess->encrypted_myway=0;
					pm_sess->encrypted_theirway=0;
			}

			if ( (enable_tuxvironments || 
				(pm_sess->encrypted_myway &&  pm_sess->encrypted_theirway)) &&
				(! pm_sess->sms)  && (! search_res) ) {
					char *tosend=_b2loc(textbuf);

					if (pm_sess->encrypted_myway &&  pm_sess->encrypted_theirway) {
						set_current_gpgme_passphrase(pm_sess->my_gpg_passphrase);
						tosend=gyache_encrypt_message(get_current_pm_profile_name(),_b2loc( textbuf), pm_sess->encryption_type);
					}

				tuxviron=gtk_object_get_data(GTK_OBJECT(pm_sess->pm_window),"imv");			

				if (tuxviron) {
					if (strlen(tuxviron)>3) {
						ymsg_pm_full(ymsg_sess, pm_sess->pm_user, tosend, tuxviron );
													} else {
										ymsg_pm(ymsg_sess, pm_sess->pm_user, tosend );
															  }
								 } else {ymsg_pm(ymsg_sess, pm_sess->pm_user, tosend );}

											    } else {

									/* NO Tuvironment, no encryption */ 
									if (pm_sess->sms || search_res) { 
											/* send plain text, SMS or search result */  
											strncat(buf,text, 154); 
									}  else {strncat(buf,textbuf, 1152); }
									chat_command(_b2loc(buf));
														 }

		if ( chat_timestamp_pm ) {
			append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
		}


		if (pm_nick_names) {myalias=get_screenname_alias(get_current_pm_profile_name());}

		snprintf( buf, 1406, "%s%s%s%s%s%s %s \n",
			YAHOO_COLOR_PMBLUE, YAHOO_STYLE_BOLDON, 
			myalias?myalias:get_current_pm_profile_name(),
			pm_command?" ":":", 	/* emote or think vs plain */
			YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF, textbuf );
		// if (show_emoticons) {convert_smileys(buf); }
		append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, buf );
		/* g_warning("TEXT IM:  %s",textbuf);  */

		if (myalias) {g_free(myalias); myalias=NULL;}
		reset_current_pm_profile_name();

		if (enable_tuxvironments) {
			if (tuxviron) {
				gtk_widget_queue_draw( GTK_WIDGET(pm_sess->pm_text));
							  }
											  }


	if (search_res) {handle_y_search_results(pm_sess->pm_window, pm_sess->pm_text, text);}

		free( text );
		if (fade1 != NULL) {free(fade1);}
		if (fade2 != NULL) {free(fade2);}

		if (( my_status ) &&
			( my_status != 12 ) &&
			( pm_brings_back )) {
			cmd_away( "0" );
										  }

		show_incoming_pm(pm_sess, 0);

}


/* end added PhrozenSmoke */

void  on_pm_send_clicked   (GtkButton   *button,  gpointer user_data)
{
	GtkWidget *pm_user;
	GtkWidget *pm_entry;
	GList *mypm_lpackptr;
	PM_SESSION *mypm_sess;
	GtkWidget *pms_entry=NULL;
	char *text=NULL;
	char *myuser;

		pm_user = lookup_widget( pm_window, "pm_user" );
		snprintf( callbackbuf,  78, "%s", gtk_entry_get_text(GTK_ENTRY(pm_user)) );
		myuser = skip_whitespace( callbackbuf );
		if (strlen(myuser)<1) {return;}
		pm_entry = lookup_widget( pm_window, "pm_entry" );		
		text=strdup(gtk_entry_get_text(GTK_ENTRY(pm_entry)));
		if (strlen(text)==0) {
			free(text);
			return ;
				    }
		if (user_selected) {free(user_selected); user_selected=NULL;}
		user_selected=strdup(myuser);
		on_pm_activate  (NULL, NULL);

		if (( mypm_lpackptr = find_pm_session( user_selected )) != NULL ) {
			mypm_sess = mypm_lpackptr->data;
			pms_entry = lookup_widget( mypm_sess->pm_window, "pms_entry" );
			if (pms_entry) {
				gtk_entry_set_text( 
					retrieve_profname_entry(mypm_sess->pm_window),
					retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_entry)))  );

				if (mypm_sess->window_style==1) {
					gtk_text_buffer_set_text(gtk_object_get_data(GTK_OBJECT(pms_entry), "textbuffer"), text, strlen(text));
				} else {
					gtk_entry_set_text(GTK_ENTRY(pms_entry),text );
						}

				send_pm_session_text(mypm_sess);
				}
		}
			free(text);
			if (user_selected) {free(user_selected); user_selected=NULL;}
			gtk_widget_destroy( pm_window );
			pm_window=NULL;
}





void
on_pms_entry_activate                  (GtkEditable     *editable,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(editable), "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
		pm_sess->typing = 0;
		ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
		send_pm_session_text(pm_sess);	
		reset_current_pm_profile_name();	
	}
}



void
on_pms_send_clicked                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "pms_send" )) != NULL ) {
		pm_sess = pm_lptr->data;
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
		pm_sess->typing = 0;
		ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
		send_pm_session_text(pm_sess);
		focus_pm_entry(pm_sess->pm_window);
		reset_current_pm_profile_name();
	}
}


/* added PhrozenSmoke: A 'buzz' feature */
void on_pms_buzz_clicked  (GtkButton  *button, gpointer user_data)  {

GList *pm_lptr;
PM_SESSION *pm_sess;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "buzz_user" )) != NULL ) {
		pm_sess = pm_lptr->data;

	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
		snprintf(callbackbuf, 256, "<font size=\"16\">  %s%s%s %s%s %s%s\n", 
						"\033[#C65CC6m", YAHOO_STYLE_BOLDON, "** BUZZ!!! **",  
						"\033[#8DA8D4m", "** BUZZ!!! **", 
						YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
		append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, callbackbuf );
		snprintf(callbackbuf, 150, "/tell %s <ding>", pm_sess->pm_user);
		chat_command(callbackbuf);
		focus_pm_entry(pm_sess->pm_window);
		reset_current_pm_profile_name();
	     }
}
/*  end buzz feature */


void
on_pms_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	GtkWidget *pms_entry;
	PM_SESSION *pm_sess;


	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "pms_clear" )) != NULL ) {
		pm_sess = pm_lptr->data;
		pms_entry = lookup_widget( pm_sess->pm_window, "pms_entry" );
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

		if (pm_sess->window_style==1) {
			pm_clear_text_view(pms_entry);
		} else {
			gtk_entry_set_text(GTK_ENTRY(pms_entry), "");
		}		
			if (pm_sess->typing ) {
				pm_sess->typing = 0;
				ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
											}
		focus_pm_entry(pm_sess->pm_window);
		reset_current_pm_profile_name();
	}
}


/* Closes window and frees all PM session resources, and turns typing status
   and encryption stuff off */

void shutdown_pms_session (PM_SESSION *pm_sess)
{
	if ( pm_sess  != NULL ) {
		char *tuxenv=NULL;
		int encrypt_say_goodbye=0;

		if (plugins_yphotos_loaded()) {
		plugins_remove_yphoto_album(pm_sess->pm_user);
		}

	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

			if (pm_sess->typing ) {
				pm_sess->typing = 0;
				ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );
											}

		if (pm_sess->encrypted_myway) {
			/* tell them encryption is off */
			encrypt_say_goodbye=1;
			snprintf(callbackbuf, 80, "%s", pm_sess->pm_user);
			pm_sess->encrypted_myway=0;
		}

		/* memory leak fix, also seg-fault watch */	
		/* clear tuxenvironment stuff */
		tuxenv=gtk_object_get_data (GTK_OBJECT (pm_sess->pm_window ), "imv");
		if (tuxenv != NULL) {
			free(tuxenv);
			tuxenv=NULL;
			gtk_object_set_data (GTK_OBJECT (pm_sess->pm_window ), "imv", tuxenv);
		}

	/* clear logging stuff */
		tuxenv=gtk_object_get_data (GTK_OBJECT (pm_sess->pm_text ), "logfilename");
		if (tuxenv != NULL) {
			free(tuxenv);
			tuxenv=NULL;
			gtk_object_set_data (GTK_OBJECT (pm_sess->pm_text), "logfilename", tuxenv);
		}

	if (gtk_object_get_data(GTK_OBJECT(pm_sess->pm_text),"logfile") != NULL) {
		/* Close any open log-files */
		FILE *lastfile;
		lastfile=gtk_object_get_data(GTK_OBJECT(pm_sess->pm_text),"logfile");
		fprintf(lastfile, "\n" );
		fflush(lastfile);
		fclose(lastfile);
		lastfile=NULL;
		gtk_object_set_data(GTK_OBJECT(pm_sess->pm_text),"logfile", lastfile);
	}

		gtk_widget_destroy( pm_sess->pm_window );
		pm_list = g_list_remove( pm_list, pm_sess );

		/* memory leak fix, also seg-fault watch */
		if (pm_sess->pm_user) {free(pm_sess->pm_user); pm_sess->pm_user=NULL;}
		if (pm_sess->buddy_image_file) {free(pm_sess->buddy_image_file); pm_sess->buddy_image_file=NULL;}

		free( pm_sess );

		/* We want to make sure we send this after the window and text buffer 
			are destroyed to avoid Gtk problems if/when a response comes back */
		if (encrypt_say_goodbye) {ymsg_encryption_notify(ymsg_sess,callbackbuf,0);}
	}
	reset_current_pm_profile_name();
}

void on_pms_close_clicked (GtkWidget *button, gpointer user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "pms_close" )) != NULL ) {
		pm_sess = pm_lptr->data;
		shutdown_pms_session (pm_sess);
	}
}

gboolean on_pm_session_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( widget, "" )) != NULL ) {
		pm_sess = pm_lptr->data;
		shutdown_pms_session (pm_sess);
	}
	return( TRUE );
}


void on_pms_ignore_clicked_cb  (GtkButton *button, gpointer user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *tmp_widget;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(user_data), "pms_ignore" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if ( ! ignore_check( pm_sess->pm_user )) {ignore_toggle( pm_sess->pm_user );}
		shutdown_pms_session (pm_sess);
	}
	if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
}

void on_pms_ignore_clicked  (GtkButton *button, gpointer user_data)
{
GtkWidget *okbutton=NULL;
okbutton=show_confirm_dialog(_("Are you sure you want to ignore this and close this window?"),"YES","NO");
if (!okbutton) {return;}
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_pms_ignore_clicked_cb), button);
}


void
on_chat_status_map                     (GtkWidget       *widget,
                                        gpointer         user_data)
{
	chat_status = widget;
}


void on_pms_profile_clicked  (GtkButton  *button,  gpointer  user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "pms_profile" )) != NULL ) {
		pm_sess = pm_lptr->data;
		snprintf(callbackbuf,100, "/profile %s", pm_sess->pm_user);
		chat_command(callbackbuf);		
		focus_pm_entry(pm_sess->pm_window);
	}
	return;
}


void on_room_tree_tree_expand  (GtkTreeView *treeview, GtkTreeIter *iterr, GtkTreePath *treepath,  gpointer user_data)
{
	ROOM_ENTRY *room_item;

	gtk_tree_model_get(gtk_tree_view_get_model(treeview), 
			iterr, GYROOM_DATA, &room_item, -1);

	if ( ! room_item->populated ) {
		populate_child_room_list( room_item->room_id, treeview, treepath );
		room_item->populated = 1;

		if ( room_item->temp_child ) {
			GtkTreeIter childiter;
			if (gtk_tree_model_get_iter(gtk_tree_view_get_model(treeview), &childiter, 
					room_item->temp_child  ) ) {
			gtk_tree_store_remove(GTK_TREE_STORE(gtk_tree_view_get_model(treeview)), &childiter);
			gtk_tree_path_free(room_item->temp_child);
			room_item->temp_child = NULL;
			/* gtk_tree_store_set(GTK_TREE_STORE(gtk_tree_view_get_model(treeview)), iterr, 
			GYROOM_DATA, room_item, -1); */ 
			}
		}
	}
	/* DO NOT FREE This the gpointer we obtain and the one in the tree store
	are one in the same! */
	/* g_free(room_item); */
}

void display_favorite_list_popup(GtkWidget *widget) {
		if ( favorites_window ) {
			setup_list = GTK_TREE_VIEW(lookup_widget( favorites_window, "fav_room_list" ));
			if ( GTK_WIDGET(setup_list) == widget ) {strncpy( setup_which,"fav_room_list", 30);}
											} else {return ;}

		setup_list = GTK_TREE_VIEW(widget);
		collect_setup_list_selected();
		if (!setup_menu) {setup_menu=create_setup_menu();}
		gtk_widget_show_all(GTK_WIDGET(setup_menu));

		/* right clicked popup action window */
		gtk_menu_popup((GtkMenu *) setup_menu,
			NULL, NULL, NULL, NULL, 1, 0);
}

void  jump_to_favorite_room(GtkWidget *widget)
{
	if ( favorites_window ) {
			setup_list = GTK_TREE_VIEW(lookup_widget( favorites_window, "fav_room_list" ));
			strncpy( setup_which, "fav_room_list", 30 );
									} else {return ;}

	collect_setup_list_selected();

	if (! setup_row_selected ) {return;	}
	/* join room */
	snprintf( callbackbuf, 128,  "/join %s", setup_row_selected );
	set_current_chat_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(GTK_WIDGET(widget))))  );
	chat_command( callbackbuf );
	/* printf("tried to join-222:  %s \n", callbackbuf); fflush(stdout); */

	if ( auto_close_roomlist ) {
		gtk_widget_destroy( favorites_window );
		favorites_window = NULL;
								 	}
}


gboolean on_favroom_list_button_press_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	/* added: PhrozenSmoke, right mouse clicks pop up menu for editing favorites
		like the menu seen in the Setup window */

	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		display_favorite_list_popup(widget);
	}
  return FALSE;
}

void on_favoriteroom_tree_kb_popup(GtkWidget *widgy, gpointer user_data) {
	display_favorite_list_popup(widgy);
}

/* double-clicks on fav room list */

void onFavoriteRoomRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		jump_to_favorite_room(GTK_WIDGET(tv));
}

void on_goto_fav_clicked (GtkButton *button, gpointer user_data)
{
	jump_to_favorite_room(GTK_WIDGET(button));
}


gboolean on_login_window_destroy_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data) 
{
	login_window=NULL;
	return (0);
}


gboolean
on_chat_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( ymsg_sess->io_callback_tag ) {
		gdk_input_remove( ymsg_sess->io_callback_tag );
		ymsg_sess->io_callback_tag = 0;
	}

	if ( ymsg_sess->ping_callback_tag ) {
		gtk_timeout_remove( ymsg_sess->ping_callback_tag );
		ymsg_sess->ping_callback_tag = 0;
	}

	close( ymsg_sess->sock ); 
	ymsg_sess->sock = -1; 
	ymsg_sess->quit = 1; 

	write_config();
	history_save();

	gdk_threads_leave();

	exit(0);
	/* gtk_main_quit(); */

	return FALSE;
}


void
on_packet_debugging_activate           (GtkCheckMenuItem     *menuitem,
                                        gpointer         user_data)
{
	GtkWidget *checkitem;
	gboolean value;

	checkitem = lookup_widget( chat_window, "packet_debugging" );
	value = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(checkitem));
		if ( value ) {
			ymsg_sess->debug_packets = 1;
		} else {
			ymsg_sess->debug_packets = 0;
		}
	write_config();
}


gboolean
on_pms_text_button_press_event         (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
#ifdef USE_GTK2
	int x, y;
	gint buf_x, buf_y;
#endif
	gchar *ptr;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {
#ifdef USE_GTK2
		x = event->x;
		y = event->y;

		gtk_text_view_window_to_buffer_coords( GTK_TEXT_VIEW(widget),
			GTK_TEXT_WINDOW_TEXT, x, y, &buf_x, &buf_y );
		ptr = get_word_at_pos( GTK_TEXT_VIEW(widget), buf_x, buf_y );
#else
		ptr = get_word_at_cursor( GTK_TEXT(widget));
#endif
		if ( check_clicked_text( ptr )) {
			return( TRUE );
		}
	}

	return FALSE;
}


void on_color_custom_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	int set;

	tmp_widget = lookup_widget( setup_window, "color_custom" );
	set =  gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (tmp_widget ));

	tmp_widget = lookup_widget( setup_window, "colorselection" );
	if ( set ) {
		gtk_widget_set_sensitive( tmp_widget, 1 );
	} else {
		gtk_widget_set_sensitive( tmp_widget, 0 );
	}
}


void on_enable_chat_spam_filter_toggled  (GtkToggleButton *togglebutton, gpointer  user_data)
{
	GtkWidget *tmp_widget;
	int set;

	set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (togglebutton));
	tmp_widget = lookup_widget( setup_window,"ignore_guests");
	if (!tmp_widget) {return;}

	if ( set ) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (tmp_widget ), TRUE);
		gtk_widget_set_sensitive( tmp_widget, 0 );
	} else {
		gtk_widget_set_sensitive( tmp_widget, 1 );
	}
}


void on_reload_room_list_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean valid;
	char *intls;

	tmp_widget = lookup_widget( room_window, "room_tree" );
	model=gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget));

	/* Added, PhrozenSmoke: To eliminate memory leaks created
	by refreshing the room list, we needed to free all the ROOM_ENTRY
	stuff that is malloced containing strdup'ed strings that is set 
	as row data, free everything up before clearing the room tree...
	Note: We don't need to apply this to the 'create a room' UI because
	the room creation tree is re-used once created and is not 'refreshable'  */	

	valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid) {	
		ROOM_ENTRY *room_item;
		gtk_tree_model_get(model, &iter,GYROOM_DATA, &room_item, -1);
		if (room_item->room_name) {
			free(room_item->room_name);
			room_item->room_name=NULL;
		}
		if (room_item->room_id) {
			free(room_item->room_id);
			room_item->room_id=NULL;
		}
		if (room_item->temp_child) {
			gtk_tree_path_free(room_item->temp_child);
			room_item->temp_child=NULL;
		}
		valid =gtk_tree_model_iter_next(model, &iter);
	}

	gy_empty_model(model, GYTV_TYPE_TREE);

	tmp_widget = lookup_widget( room_window, "intlcombo" );
		if (tmp_widget) {
		intls=strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)));
		set_room_list_country_code(intls);
		free(intls);
			 				  }	
	populate_room_list(0);
}


gboolean
on_pms_entry_key_press_event           (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;	


		if ( (event->keyval==GDK_Tab)  && (pm_sess->window_style==1))  {			
			return TRUE;  /* This stops the Tab char from being inserted */
		}

		if ( (event->keyval==GDK_Return)  && (pm_sess->window_style==1))  {
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			pm_sess->typing = 0;
			if (! pm_sess->sms) {ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );}
			send_pm_session_text(pm_sess);	
			reset_current_pm_profile_name();	
			return TRUE;  /* This stops the return char from being inserted */
		}

		if ((event->keyval==GDK_BackSpace) || (event->keyval==GDK_Delete) 
		 || (event->keyval==GDK_KP_Delete)  )  {
			if (pm_sess->typing ) {
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
				pm_sess->typing = 0;
				if (! pm_sess->sms) {ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );}
				reset_current_pm_profile_name();
											}
			return FALSE;
										 						}

		if ( pm_sess->window_style==1)  {
			/* textbox */
			GtkWidget *pms_entry;
			int txlimit=422;
			pms_entry = lookup_widget( pm_sess->pm_window, "pms_entry" );
			if (pm_sess->sms) {txlimit=151;}
			if (gtk_text_buffer_get_char_count( GTK_TEXT_BUFFER(gtk_object_get_data (GTK_OBJECT (pms_entry), "textbuffer") ) )> txlimit ) {
					/* Set the input text limit if we've exceeded it */
			return TRUE;

			}
		}

		/* Don't turn typing status on for special characters 
		like the 'up' and 'down' arrows, 'insert', 'ALT', 'CTRL' key...etc. */

		if (event->keyval > GDK_3270_Enter) {
			if (event->keyval < GDK_KP_Equal) {return FALSE;}
			if (event->keyval > GDK_KP_9) {return FALSE;}			
		}

		if ( ! pm_sess->typing ) {
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			pm_sess->typing = 1;
			if (! pm_sess->sms) {ymsg_typing( ymsg_sess, pm_sess->pm_user, pm_sess->typing );}
			reset_current_pm_profile_name();
		}
	}

	return FALSE;
}


void
on_find_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	int case_sensitive;

	tmp_widget = lookup_widget( find_window, "search_text" );

	search_pos = -1;
	search_text = strdup( gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	tmp_widget = lookup_widget( find_window, "case_sensitive" );
	case_sensitive = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	search_chat_text( case_sensitive );

}


void
on_clear_find_button_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	tmp_widget = lookup_widget( find_window, "search_text" );
	gtk_entry_set_text(GTK_ENTRY(tmp_widget), "");

	search_pos = -1;
	if ( search_text ) {
		free( search_text );
		search_text = NULL;
	}
}


void
on_cancel_find_button_clicked          (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	tmp_widget = lookup_widget( find_window, "search_text" );
	gtk_entry_set_text(GTK_ENTRY(tmp_widget), "");

	gtk_widget_hide( find_window );
	search_pos = -1;
	if ( search_text ) {
		free( search_text );
		search_text = NULL;
	}
}


void
on_find_next_button_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	search_chat_text( search_case_sensitive );
}


gboolean
on_find_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	if ( find_window )
		gtk_widget_destroy( find_window );

	find_window = NULL;

	search_pos = -1;
	if ( search_text ) {
		free( search_text );
		search_text = NULL;
	}

	return( TRUE );
}


void
on_search_text_activate                (GtkEditable     *editable,
                                        gpointer         user_data)
{
	on_find_button_clicked( NULL, NULL );
}


void on_browser_clicked  (GtkButton  *button, gpointer user_data)
{
	char *browser = (char *)user_data;
	GtkWidget *tmp_widget;

	tmp_widget = lookup_widget( setup_window, "browser_command" );

	if ( ! strcmp( browser, "netscape" )) {
		gtk_entry_set_text(GTK_ENTRY(tmp_widget),
			"netscape -noraise -remote 'openURL(%s,GyachE)' &");
	} else if ( ! strcmp( browser, "mozilla" )) {
		gtk_entry_set_text(GTK_ENTRY(tmp_widget),
			"mozilla -remote 'openURL(%s,new-window)' &");
	} else if ( ! strcmp( browser, "opera" )) {
			 gtk_entry_set_text(GTK_ENTRY(tmp_widget),
			"opera -remote 'openURL(%s,new-page)' &");
	} else if ( ! strcmp( browser, "konquerer" )) {
		gtk_entry_set_text(GTK_ENTRY(tmp_widget),
			"kfmclient openURL '%s' &");
	} else {
		gtk_entry_set_text(GTK_ENTRY(tmp_widget),
			"netscape -noraise -remote 'openURL(%s)'  &");
	}
}


/* added: PhrozenSmoke, callbacks for room creation */

void free_create_room_tree_data(GtkWidget *tmp_widget) {
	GtkTreeIter iter;
	GtkTreeModel *model;
	gboolean valid;

	model=gtk_tree_view_get_model(GTK_TREE_VIEW(lookup_widget(tmp_widget,"croom_tree")));
	valid = gtk_tree_model_get_iter_first(model, &iter);
	while(valid) {	
		ROOM_ENTRY *room_item;
		gtk_tree_model_get(model, &iter,GYROOM_DATA, &room_item, -1);
		if (room_item->room_name) {
			free(room_item->room_name);
			room_item->room_name=NULL;
		}
		if (room_item->room_id) {
			free(room_item->room_id);
			room_item->room_id=NULL;
		}
		if (room_item->temp_child) {
			gtk_tree_path_free(room_item->temp_child);
			room_item->temp_child=NULL;
		}
		valid =gtk_tree_model_iter_next(model, &iter);
	}
	gy_empty_model(model, GYTV_TYPE_TREE);
}

gboolean on_cancel_croom_clicked (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *tmp_widget;
	tmp_widget = lookup_widget( widget, "mywindow" );
	if (tmp_widget)  {
		free_create_room_tree_data(tmp_widget);
		gtk_widget_destroy( tmp_widget );
		room_maker_window=NULL;
						    }
	return( TRUE );
}

gboolean  on_cancel_croom_clickedw             (GtkWidget       *widget,       
                                        GdkEvent        *event,                
                                        gpointer         user_data)
{
	return on_cancel_croom_clicked(widget,user_data);
}

gboolean on_refresh_intl_croom_clicked (GtkWidget *widget, gpointer user_data)
{
	GtkWidget *tmp_widget;
	tmp_widget = lookup_widget( widget, "mywindow" );
	if (tmp_widget)  {
		GtkWidget *tmp_widget2;
		free_create_room_tree_data(tmp_widget);
		tmp_widget2= lookup_widget( tmp_widget, "intlcombo" );
		if (tmp_widget2) {
			char *intls=NULL;
			intls=strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(tmp_widget2)->entry)));
			set_room_list_country_code(intls);
			free(intls);
			populate_room_list(1);
			 				 	 }	
						    }
	return( TRUE );
}


void collect_select_create_room()  {
		GtkTreeModel *model;
		GtkTreeIter iter;		
		GtkTreeSelection *selection;
			
		if (!room_maker_window) {return;}
		selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(lookup_widget( room_maker_window, "croom_tree" )));
		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			ROOM_ENTRY *room_item;
			gtk_tree_model_get(model, &iter,GYROOM_DATA, &room_item, -1);		

			if ( create_room_selected ) {
				free( create_room_selected );  
				create_room_selected=NULL;
			}
			if ( room_item ) {
				if (room_item->room_id){
					create_room_selected = strdup( room_item->room_id);
					/* printf("created: %s\n", create_room_selected); fflush(stdout); */
				}
				/* DO NOT FREE this pointer!, we are working with it 
				directly, not a reference to it freeing it here also frees it in the tree store */
				/*  g_free(room_item);  */
									  }
		}
}

gboolean
on_create_croom_clicked              (GtkButton       *widget,                       
                                        gpointer         user_data)
{
	GtkWidget *taccess1=NULL;
	GtkWidget *taccess2=NULL;
	GtkWidget *taccess3=NULL;
	GtkWidget *taccess4=NULL;
	GtkWidget *troomentry=NULL;
	GtkWidget *tmessentry=NULL;

	char catnumber[24]="";
	char acc1[16]="";  /* permissions, voice chat, ymsg field 126 */
	char room[32]="";
	char topic[56]="";

	collect_select_create_room();

	taccess1 = gtk_object_get_data(GTK_OBJECT(widget), "acccess1" );
	taccess2 = gtk_object_get_data( GTK_OBJECT(widget), "acccess2" );
	taccess3 = gtk_object_get_data( GTK_OBJECT(widget), "acccess3" );
	taccess4 =gtk_object_get_data( GTK_OBJECT(widget), "acccess4" );
	troomentry = gtk_object_get_data( GTK_OBJECT(widget), "roomentry" );
	tmessentry = gtk_object_get_data( GTK_OBJECT(widget), "messentry" );


  	if ( taccess1  && taccess2 && taccess3 && taccess4 && troomentry && tmessentry)  

		{
			snprintf(room,31, "%s",  gtk_entry_get_text(GTK_ENTRY(troomentry)));
			snprintf(topic, 54, "%s",  gtk_entry_get_text(GTK_ENTRY(tmessentry)));

			if ( create_room_selected) {snprintf(catnumber,22, "%s",create_room_selected);}
			else {snprintf(catnumber,22, "%s", "1600000002");}

		  /* using info I got from some network scans, not sure about all these parameters...
			  right now, the 'voice chat' param is only know to be correct for PUBLIC rooms, 
			 and I think by default voice chat will always be enabled for private and 'invite only' rooms */

			/* public room */
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess1)))  
			{
					/* voice chat or not? */
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess4)))  
				 {   /* voice enabled */
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess3))) 
					{
					  /* invite only */
					snprintf(acc1,14,"%s","65540" ); 
					}  else  {  
					/* NOT invite only */										
					snprintf(acc1,14,"%s","65536" ); 
						    }
				 } else  {  /* voice disabled */
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess3))) 
					{
					  /* invite only */
					 snprintf(acc1,14,"%s","4" );
					}  else  {  
					/* NOT invite only */			
					 snprintf(acc1,14,"%s","0" );
						    }
				 }
			}  



			 /* private rooms */
			if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess2))) 
			{
				if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess4)))  
				 {   /* voice enabled */
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess3))) 
					{
					  /* invite only */
					snprintf(acc1,14,"%s","65542" ); 
					}  else  {  
					/* NOT invite only */										
					snprintf(acc1,14,"%s","65538" ); 
						    }
				 } else  {  /* voice disabled */
					if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess3))) 
					{
					  /* invite only */
					 snprintf(acc1,14,"%s","6" );
					}  else  {  
					/* NOT invite only */			
					 snprintf(acc1,14,"%s","2" );
						    }
				 }
			}

	
		} else {return 0; }

/* If no topic is set, set the topic to the room name */
if (strlen(topic)==0) {snprintf(topic,54, "%s", room);}
/* if no attributes are set, default to public room with voice (non-invite-only) */
if (strlen(acc1)==0) {snprintf(acc1,14,"%s", "65536");}

/* for private  rooms,
   Let's place the room in an obscure category '1600000002'
   In case Yahoo screws up and lists the room anyways, it will be in a category 
   few people go to.
 */
if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(taccess2)))  {
	snprintf(catnumber, 22, "%s", "1600000002");
													 }



/* 
printf("CREATE ROOM:\n\nRoom:%s\nCat: %s\nTopic: %s\nAttribs: %s \n", room, catnumber, topic, acc1);
fflush(stdout);  */

  /* send some Ymesgs here */
	

	if (strlen(topic) && strlen(room) && strlen(catnumber) && strlen(acc1)) 
	{
	/* printf("Sending ymesg...\n");  fflush(stdout);  */
	set_current_chat_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(GTK_WIDGET(widget))))  );
	if ((!in_a_chat) || is_conference) {
		ignore_ymsg_online=1;
		ymsg_conference_leave(ymsg_sess);
		ymsg_online(ymsg_sess);
			     }
	ymsg_create_room(ymsg_sess, catnumber, room, topic, acc1 );
	/* printf("ymesg sent\n");  fflush(stdout);  */
	return on_cancel_croom_clicked(GTK_WIDGET(widget),user_data);
	}


	return 0;
}


void on_create_room_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
	if (ymsg_sess->user ) {
		if (strlen(ymsg_sess->user )>0 ) {
			snprintf(callbackbuf, 80, "%s",  ymsg_sess->user);
			if (! room_maker_window) {
		    room_maker_window=create_room_maker (callbackbuf);
			populate_room_list(1);
														}
								 							 }
									}
}




/* added: PhrozenSmoke, buddy list callbacks */

void display_buddy_grp_menu() {
	if (! budgrp_menu) {
		budgrp_menu=create_budgrp_popup_menu();
	}
	gtk_menu_popup((GtkMenu *) budgrp_menu,
	NULL, NULL, NULL, NULL, 1, 0);
}

void display_buddy_list_popup_menu() {
		if ( ! selected_buddy ) {
			if ( selected_buddy_group ) {display_buddy_grp_menu();}
			return;
		}
		if (!budpop_menu) {budpop_menu=create_budlist_popup_menu();}
		gtk_widget_show_all(GTK_WIDGET(budpop_menu));
		/* right clicked popup action window */
		gtk_menu_popup((GtkMenu *) budpop_menu,
			NULL, NULL, NULL, NULL, 1, 0);
}

void on_display_buddy_list_popup_menu_kb(GtkWidget *widget, gpointer user_data) {
	display_buddy_list_popup_menu();
}
void onBuddyListRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data) {

		GtkTreeIter iter;
		gchar *sfound;
		char *gtemp=NULL;
		if (!tp) {return;}
		gtk_tree_model_get_iter(gtk_tree_view_get_model(tv), &iter, tp);

		if ( selected_buddy_group ) {
			free( selected_buddy_group );
			selected_buddy_group=NULL;
		}
		if ( selected_buddy ) {
			free( selected_buddy );  
			selected_buddy=NULL;
		}

		if (budgrp_menu) {gtk_menu_popdown((GtkMenu *) budgrp_menu);}
		if (budpop_menu) {gtk_menu_popdown((GtkMenu *) budpop_menu);}

		gtk_tree_model_get(gtk_tree_view_get_model(tv), &iter,GYDOUBLE_TOP, &sfound, -1);
		gtemp=strdup(sfound);		
			if (!strcmp(gtemp, "GYEBuddy_TopLvl")) {  /* buddy group selected */
				gchar *gfound;
				g_free(sfound);
				free(gtemp);
				gtk_tree_model_get(gtk_tree_view_get_model(tv), &iter,GYDOUBLE_COL1, &gfound, -1);	
				if (strcmp(gfound, TMP_FRIEND_CB)) {
					selected_buddy_group=strdup(gfound);
				}
				
				g_free(gfound);
				return;
			}	else {  /* regular buddy selected */
				gchar *gfound;
				if (strcmp(gtemp, TMP_FRIEND_CB)) {
					selected_buddy_group=strdup(gtemp);
				}
				free(gtemp);
				g_free(sfound);
				gtk_tree_model_get(gtk_tree_view_get_model(tv), &iter,GYDOUBLE_COL1, &gfound, -1);	
				if (strcmp(gfound, TMP_FRIEND_CB)) {
					selected_buddy=strdup(gfound);
				}
				g_free(gfound);
			            }

	if ( ! selected_buddy ) {return;}
	if (  strcasecmp( selected_buddy, ymsg_sess->user ) && (! find_profile_name(selected_buddy))) {
		GList *mypm_lpackptr;
		PM_SESSION *mypm_sess;
					if (( mypm_lpackptr = find_pm_session( selected_buddy )) != NULL ) {
						mypm_sess = mypm_lpackptr->data;
					} else {
						/* open a new window if one doesn't exist already */
						mypm_sess=new_pm_session(selected_buddy);
					}
					gtk_widget_show_all(mypm_sess->pm_window );
					gdk_window_raise(mypm_sess->pm_window->window );
	 } 
 }



gboolean on_select_a_buddy    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	/* Right mouse button clicks: Popup menu */
	if (event->button == 3 && event->type == GDK_BUTTON_PRESS) {
		if (selected_buddy) {display_buddy_list_popup_menu(); return FALSE;}
		if (selected_buddy_group) {display_buddy_grp_menu(); return FALSE; }
	}

	return FALSE;
}


void
on_buddy_pm_activate                         (GtkButton     *bbutton,
                                        gpointer         user_data)
{
	if ( ! selected_buddy) {	return; }	

	if (  strcasecmp( selected_buddy, ymsg_sess->user ) && (! find_profile_name(selected_buddy))) {
		GList *mypm_lpackptr;
		PM_SESSION *mypm_sess;

					if (( mypm_lpackptr = find_pm_session( selected_buddy )) != NULL ) {
						mypm_sess = mypm_lpackptr->data;
					} else {
						/* open a new window if one doesn't exist already */
						mypm_sess=new_pm_session(selected_buddy);
					}
					gtk_widget_show_all(mypm_sess->pm_window );
					gdk_window_raise(mypm_sess->pm_window->window );

	/* old way */
	/* pm_window=blank_pm_window(selected_buddy); */
	}
}


void  on_buddy_profile_activate       (GtkButton     *bbutton,
                                        gpointer         user_data)
{
	if ( ! selected_buddy ) { return ; }
	snprintf(callbackbuf,100, "/profile %s", selected_buddy);
	chat_command(callbackbuf);					
 }



void  on_buddy_buzz    (GtkButton  *button, gpointer  user_data)  {
	if ( ! selected_buddy) {return;  }
	if (  strcasecmp( selected_buddy, ymsg_sess->user ) && (! find_profile_name(selected_buddy))) {
		snprintf(callbackbuf, 150,  "/tell %s <ding>", selected_buddy);
		chat_command(callbackbuf);
																					  }
}


void
on_buddy_join_in_chat                    (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( ! selected_buddy) {return;  }
		if (strcasecmp( selected_buddy, ymsg_sess->user ) && (! find_profile_name(selected_buddy))) {
		snprintf(callbackbuf, 128,  "/goto %s", selected_buddy);
		set_current_chat_profile_name( get_default_profile_name());
		chat_command(callbackbuf);
																								}
}

void
on_buddy_invite_to_chat                    (GtkButton       *button,
                                        gpointer         user_data)
{
	if ( ! selected_buddy) {return;  }

	if (  strcasecmp( selected_buddy, ymsg_sess->user ) && (! find_profile_name(selected_buddy))) {
		if (is_conference)  {
			use_multpm_settings=0;
			use_chat_settings=0;
			open_make_conference_window_with_name(selected_buddy);
			return;
					    }
		if (in_a_chat) {
			use_chat_settings=1;
			use_multpm_settings=0;
			open_make_conference_window_with_name(selected_buddy);
			return;
				    }
									
										}
}


/* PhrozenSmoke:  more Buddy list callbacks */
void on_add_this_buddy(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newbuddy[65]="";
	char *nbuddy=newbuddy;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newbuddy[i]=etext[i]; }
														}
						}

	if ( ! find_friend( nbuddy )) { add_friend(nbuddy);}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	if (etext) {g_free(etext);}


}

void on_click_add_a_buddy(GtkButton *widget, gpointer user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog("Buddy","Add Buddy","Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_add_this_buddy), NULL);

if (selected_buddy)  {
	GtkWidget *tmp_entry;
	if (find_profile_name(selected_buddy)) {return;}
	if (!strcasecmp(selected_buddy, ymsg_sess->user)) {return;}
	if (! find_temporary_friend(selected_buddy)) {return; }
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	if (tmp_entry) {gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy); }
			      }
}


void on_click_revoke_this_buddy(GtkButton *widget, gpointer user_data) {
  GtkWidget *tmp_widget;
  char *revbuddy;
  revbuddy=gtk_object_get_data( GTK_OBJECT(widget), "buddy" );
  if (! revbuddy) { return; }
  snprintf(callbackbuf, 128, "/revoke %s", revbuddy);
  chat_command(callbackbuf);
  free(revbuddy);
	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}
}



void on_click_del_this_buddy(GtkButton *widget, gpointer user_data) {
	int was_tmp_friend=0;
	GtkWidget *tmp_widget;
	GtkWidget *okbutton;
	char *nsel_buddy=NULL;

	if ( ! selected_buddy) {return;  }

	if (find_temporary_friend( selected_buddy) ) {
		remove_temporary_friend(selected_buddy);
		was_tmp_friend=1;
	}

	/* Patches a segfault that was caused by calling 'revoke' after remove */

	if (! was_tmp_friend) {
		nsel_buddy=strdup(selected_buddy);   
		if (find_friend( selected_buddy )) {
			if ( using_web_login || ycht_is_running() ) {
				show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
				return;
			} else { remove_friend(selected_buddy); }
							 	}
	}

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	if (tmp_widget)  {gtk_widget_destroy( tmp_widget );	}

	if (was_tmp_friend) {return;}

okbutton=show_confirm_dialog(_("Would you like to remove yourself\nfrom this person's buddy list?"),"YES","NO");
if (!okbutton) {free(nsel_buddy); return;}
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_click_revoke_this_buddy), NULL);
   gtk_object_set_data( GTK_OBJECT(okbutton), "buddy", nsel_buddy);

}

void on_click_del_a_buddy(GtkButton *widget, gpointer user_data) {
GtkWidget *okbutton;
	if ( ! selected_buddy) {return;  }

/* added to GETTEXT here */
okbutton=show_confirm_dialog("Are you sure you want to remove this buddy?","YES","NO");
if (!okbutton) {return;}
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_click_del_this_buddy), NULL);
}


void on_send_user_file_cb(GtkButton *widget, gpointer user_data) {
	gchar *somefile=NULL;
	GtkWidget *tmp_entry;
	if (! user_for_file) { return; }

	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	if (tmp_entry) {
		somefile=g_strdup(gtk_file_selection_get_filename(GTK_FILE_SELECTION(tmp_entry) ) );
		gtk_widget_destroy( tmp_entry );
		if (somefile)  {
		if (strlen(somefile)>0) {
			preselected_filename(somefile);  /* remember for next time */
		  if (  strcasecmp(user_for_file, ymsg_sess->user ) && (! find_profile_name(user_for_file))) {
			snprintf(callbackbuf, 400, "/file %s %s", somefile, user_for_file);
			/* printf("Some file: %s\n", callbackbuf); fflush(stdout); */
			chat_command( callbackbuf);
																						   }
											}
		g_free(somefile);
							  }
						}

 }

void on_send_buddy_file(GtkButton *widget, gpointer user_data) {
GtkWidget *okbutton;
	if ( ! selected_buddy) {return; }
	if (find_profile_name(selected_buddy)) {return;}
	if (!strcasecmp(selected_buddy, ymsg_sess->user )) {return;}
	if (user_for_file) {free(user_for_file); }
	user_for_file=strdup(selected_buddy);

	
	okbutton=create_fileselection2();
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_user_file_cb),
                      NULL);
	
}

void on_send_file_to_user(GtkWidget *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newbuddy[65]="";
	char *nbuddy=newbuddy;
	GtkWidget *okbutton;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newbuddy[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	if (strlen(nbuddy) >0) {
		if (user_for_file) {free(user_for_file); }
		user_for_file=strdup(nbuddy);
		okbutton=create_fileselection2();
  		gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_user_file_cb),
                      NULL);
								    }
	if (etext) {g_free(etext);}

}


void on_send_file_to_user_cb(GtkMenuItem *widget, gpointer user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog("User","OK","Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_file_to_user), NULL);
}


void on_pms_goto_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	char *ptr;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "goto_user" )) != NULL ) {
		pm_sess = pm_lptr->data;
	set_current_chat_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
		ptr = strdup( pm_sess->pm_user );
		snprintf(callbackbuf, 128, "/goto %s", ptr);
		chat_command(callbackbuf);
		free(ptr);
		focus_pm_entry(pm_sess->pm_window);
	}
}


void on_pms_sendfile_clicked           (GtkButton       *button,
                                        gpointer         user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *okbutton;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "send_user" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (user_for_file) {free(user_for_file); }
		user_for_file=strdup( pm_sess->pm_user );
		okbutton=create_fileselection2();
  		gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_user_file_cb),
                      NULL);

	}

}


/* added: PhrozenSmoke, put 'Find...' window on menu */
void on_find_menu_clicked(GtkMenuItem *mitem, gpointer user_data)  {
	jump_to_chat_tab();

							search_pos = -1;
							if ( ! find_window ) {
							if ( search_text ) {
								free( search_text );
								search_text = NULL;
														}
								find_window = create_find_window();
							} 
							gtk_widget_show_all( find_window );
}


int on_buddy_profile_activate_timeout2       (guint data)
{
	if ( ! user_for_file ) { return FALSE; }
	snprintf(callbackbuf,100, "/profile %s", user_for_file );
	chat_command(callbackbuf);	
	if (user_for_file) {free(user_for_file); user_for_file=NULL; }
	 return FALSE;
}


void on_open_profile(GtkWidget *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newbuddy[65]="";
	char *nbuddy=newbuddy;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newbuddy[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	if (strlen(nbuddy) >0) {
		if (user_for_file) {free(user_for_file); }
		user_for_file=strdup( nbuddy);
		gtk_timeout_add( 10 , (void *)on_buddy_profile_activate_timeout2, NULL );
								    }
	if (etext) {g_free(etext);}

}



void on_open_profile_cb(GtkMenuItem *widget, gpointer user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog("User","OK","Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_open_profile), NULL);
}


void  on_open_goto                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newbuddy[65]="";
	char *nbuddy=newbuddy;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(button), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newbuddy[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	if (strlen(nbuddy) >0) {
		if (  strcasecmp(nbuddy, ymsg_sess->user ) && (! find_profile_name(nbuddy)) ) {
		snprintf(callbackbuf, 128, "/goto %s", nbuddy);
		set_current_chat_profile_name( get_default_profile_name());
		chat_command(callbackbuf);
																				}
								    }
	if (etext) {g_free(etext);}

}


void on_open_goto_cb(GtkMenuItem *widget, gpointer user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog("User","OK","Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_open_goto), NULL);
}





/* added, PhrozenSmoke - Conference callbacks  */

void on_start_conference(GtkMenuItem *widget, gpointer user_data) {
	use_chat_settings=0;
	snprintf(callbackbuf, 25, "%s","/conf_start");
	chat_command(callbackbuf);
}




void on_invite_to_conf(GtkMenuItem *widget, gpointer user_data) {

	if (is_conference) {use_chat_settings=0;}
	if (in_a_chat) {use_chat_settings=1; }

	if (is_conference || in_a_chat) {
		snprintf(callbackbuf, 25, "%s","/conf_start");
		chat_command(callbackbuf);
	}
}


void  on_conf_accept_cb (GtkButton *button, gpointer  user_data)
{
	GtkWidget *tmp_widget;
	char *enteras=NULL;
	enteras=gtk_object_get_data( GTK_OBJECT(button),PROFNAMESTR );
	if (enteras)  {
		set_current_chat_profile_name(enteras);
		free(enteras); enteras=NULL;
	}
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	accept_invite();
}

void  on_conf_decline_cb (GtkButton *button, gpointer  user_data)
{
	GtkWidget *tmp_widget;
	char *enteras=NULL;
	enteras=gtk_object_get_data( GTK_OBJECT(button),PROFNAMESTR );
	if (enteras)  {
		set_current_chat_profile_name(enteras);
		free(enteras); enteras=NULL;
	}
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	decline_invite(NULL);
}


void  on_accept_voice_conf                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	char *pmroom=NULL;
	char *cookie=NULL;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	pmroom =gtk_object_get_data( GTK_OBJECT(button), "pmroom" );
	cookie =gtk_object_get_data( GTK_OBJECT(button), "cookie" );
	if (pmroom && cookie)  {
		char *currusr=gtk_object_get_data( GTK_OBJECT(button),PROFNAMESTR);
		set_current_pm_profile_name( currusr?currusr:ymsg_sess->user );
		set_voice_conference_cookie(cookie);
		voice_launch_conference(1, currusr?currusr:ymsg_sess->user , pmroom);
		free(pmroom);
		free(cookie);
		if (currusr) {free(currusr);}
					 }
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
}

void  on_activate_voice_conf                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(button), "voice_chat" )) != NULL ) {
	pm_sess = pm_lptr->data;
		char *currusr=retrieve_profname_str(
			GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)));
		snprintf(callbackbuf, 70, "%s-%d%d%c%d%d", currusr, 9, 4, pm_sess->pm_user[0], 3,
			currusr[0]);
		set_current_pm_profile_name( currusr );
		 /* voice_launch_conference(1, currusr ,proom); */ 
		 ymsg_voice_invite(ymsg_sess, pm_sess->pm_user, callbackbuf );

		snprintf( callbackbuf, 255 ,  "%s%s   ** '%s' %s **%s%s\n",
						YAHOO_COLOR_BLUE, YAHOO_STYLE_BOLDON, 
						currusr, 
						_("has enabled voice chat."), 
						YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );

		 append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, callbackbuf );
		 focus_pm_entry(pm_sess->pm_window);
		 reset_current_pm_profile_name();
	   }  
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
}


void on_launch_py_voice(GtkMenuItem *widget, gpointer user_data) {
	snprintf(callbackbuf,300, "%s/pyvoice/pyvoiceui.py &",PACKAGE_DATA_DIR);
	my_system(callbackbuf);
}

void on_py_voice_enable(GtkMenuItem *widget, gpointer user_data) {
		if (!ymsg_sess->room) {return;}
		if (!ymsg_sess->user) {return;}
	if ( is_conference) {  
							sprintf(callbackbuf,"%s","");

								strcat(callbackbuf,"\n");
								strcat(callbackbuf,YAHOO_STYLE_BOLDON);
								strcat(callbackbuf,YAHOO_COLOR_PURPLE);
								strncat(callbackbuf,_("Voice Chat"), 60);
								strcat(callbackbuf,":  ");
								strcat(callbackbuf,YAHOO_COLOR_GREEN);
								strncat(callbackbuf, _("Enabled"), 60);
								strcat(callbackbuf,YAHOO_COLOR_BLACK);
								strcat(callbackbuf,YAHOO_STYLE_BOLDON);
								strcat(callbackbuf,"\n");
								append_to_textbox_color( chat_window, NULL, callbackbuf );

			voice_launch_conference(1, get_chat_profile_name(), ymsg_sess->room);
							  }
								 else {voice_launch(1); }
}


void  on_reject_buddy_add                    (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	char *mbuddy=NULL;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	mbuddy =gtk_object_get_data( GTK_OBJECT(button), "buddy" );
	if (mbuddy)  {
		ymsg_reject_buddy( ymsg_sess, mbuddy);

		snprintf(callbackbuf, 300 , "%s%s** %s '%s' **%s%s\n", YAHOO_STYLE_ITALICON, YAHOO_COLOR_RED, _("Denied buddy list privileges for"), mbuddy, YAHOO_STYLE_ITALICOFF, YAHOO_COLOR_BLACK);
			append_to_textbox( chat_window, NULL, callbackbuf );
		free(mbuddy);
					 }
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
}

/* added PhrozenSmoke */
void on_refresh_activate       (GtkMenuItem     *menuitem, gpointer         user_data)
{
	reset_all_online_friends_statuses( );
	update_buddy_clist();
	ymsg_refresh(ymsg_sess);
}

void  on_room_invite_accept_cb (GtkButton *button, gpointer  user_data)
{
	GtkWidget *tmp_widget;
	char *chatroom=NULL;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(button), "mywindow" );
	chatroom =gtk_object_get_data( GTK_OBJECT(button), "chatroom" );
	if (chatroom)  {
		char *enteras=NULL;
		enteras=gtk_object_get_data( GTK_OBJECT(button),PROFNAMESTR );
		if (enteras)  {
			set_current_chat_profile_name(enteras);
			free(enteras); enteras=NULL;
		}
		snprintf(callbackbuf,128, "/join %s",chatroom);
		chat_command(callbackbuf);
		free(chatroom);
					 }
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
}



void on_report_abuse(GtkMenuItem *widget, gpointer user_data) {
	snprintf( callbackbuf, 350, "%s", "http://add.yahoo.com/fast/help/chat/cgi_abuse" );
	display_url( (void *) strdup(callbackbuf) );
}

void on_yahoo_games(GtkMenuItem *widget, gpointer user_data) {
	snprintf( callbackbuf, 350, "%s",  "http://games.yahoo.com");
	display_url( (void *) strdup(callbackbuf) );
}

void on_yahoo_news_alert(GtkMenuItem *widget, gpointer user_data) {
	snprintf( callbackbuf, 350, "%s", "http://alerts.yahoo.com");
	display_url( (void *) strdup(callbackbuf) );
}

void on_yahoo_mail_act(GtkMenuItem *widget, gpointer user_data) {
	snprintf( callbackbuf, 16, "%s", "/mail" );
	chat_command( callbackbuf );
}

void on_leave_room_or_conf(GtkMenuItem *widget, gpointer user_data) {
	snprintf(callbackbuf,16,"%s","/leave");
	chat_command(callbackbuf);
}


void on_make_custom_away_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	GtkWidget *busy_icon;
	GtkWidget *idle_icon;
	char *etext=NULL;

	force_busy_icon=0;
	force_idle_icon=0;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	busy_icon = gtk_object_get_data( GTK_OBJECT(widget), "busy_icon" );
	idle_icon = gtk_object_get_data( GTK_OBJECT(widget), "idle_icon" );

	if  ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(idle_icon)) )  {force_idle_icon=1;}

	if  ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(busy_icon)) )  {force_busy_icon=1;}

	if (tmp_entry) {
		etext=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
			    }

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

	if (strlen(etext)>0) { cmd_away(etext);}
	if (etext) {free(etext);}
}

void on_make_custom_away  (GtkMenuItem *menuitem, gpointer  user_data) {
	GtkWidget *okbutton;
	GtkWidget *dhextra;
	GSList *staticon_group = NULL;
	GtkWidget *norm_icon;
	GtkWidget *busy_icon;
	GtkWidget *idle_icon;
	GtkWidget *tmpentry;		

/* added to GETTEXT here */
	okbutton=show_input_dialog(_("Away message"), "OK" ,"Cancel");
  		gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_make_custom_away_cb), NULL);
  	dhextra=gtk_object_get_data (GTK_OBJECT (okbutton), "dhextra");

	tmpentry=gtk_object_get_data (GTK_OBJECT (okbutton), "myentry");

	if (tmpentry && emulate_ymsg6) {
	  gtk_entry_set_max_length(GTK_ENTRY(tmpentry),254);	
	}

	if (last_status_msg && (strlen(last_status_msg)>3)) {
		if (tmpentry) {
				gtk_entry_set_text(GTK_ENTRY(tmpentry),last_status_msg);
				}
																		 }

	norm_icon = gtk_radio_button_new_with_mnemonic (NULL, _("Normal icon"));	
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (norm_icon), staticon_group);
	staticon_group = gtk_radio_button_group (GTK_RADIO_BUTTON (norm_icon));
	set_tooltip(norm_icon,_("Here"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (norm_icon), TRUE);

	busy_icon = gtk_radio_button_new_with_mnemonic (NULL, _("Busy icon"));
	set_tooltip(busy_icon,_("Busy"));	
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (busy_icon), staticon_group);
	staticon_group = gtk_radio_button_group (GTK_RADIO_BUTTON (busy_icon));

	idle_icon = gtk_radio_button_new_with_mnemonic (NULL, _("Idle icon"));	
	set_tooltip(idle_icon,_("Idle"));	
	gtk_radio_button_set_group (GTK_RADIO_BUTTON (idle_icon), staticon_group);
	staticon_group = gtk_radio_button_group (GTK_RADIO_BUTTON (idle_icon));

	gtk_box_pack_start (GTK_BOX (dhextra), norm_icon, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (dhextra), gtk_label_new("    "), FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (dhextra), busy_icon, TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (dhextra), idle_icon, TRUE, TRUE, 2);
	  gtk_object_set_data (GTK_OBJECT (okbutton), "busy_icon", busy_icon);
	  gtk_object_set_data (GTK_OBJECT (okbutton), "idle_icon", idle_icon);
	gtk_widget_show_all(dhextra);
}



void on_view_briefcase_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

	if (strlen(nsite)>0) {
		snprintf( callbackbuf, 350, "http://briefcase.yahoo.com/bc/%s", nsite);
		display_url( (void *) strdup(callbackbuf) );
				      }


	if (etext) {g_free(etext);}

}


void on_view_briefcase  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_view_briefcase_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}


void on_rejoin_room  (GtkMenuItem *menuitem, gpointer  user_data) {
	chat_command("/rejoin");
}



void on_send_contact_info_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

	if (strlen(nsite)>0) {
		snprintf(callbackbuf, 150, "/business-card %s", nsite);
		chat_command(callbackbuf);
				      }


	if (etext) {g_free(etext);}

}


void on_send_contact_info  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_contact_info_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	if (find_profile_name(selected_buddy)) {return;}
	if (!strcasecmp(selected_buddy, ymsg_sess->user)) {return;}
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}


void on_buddy_add_accept_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	char *nbuddy;

	nbuddy=gtk_object_get_data(GTK_OBJECT(widget), "buddy");
	if (nbuddy) {
		if ( ! find_friend( nbuddy )) { add_friend(nbuddy);}
		free(nbuddy);
			}
	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

}

void on_buddy_add_accept(GtkButton *widget, gpointer user_data) {
GtkWidget *tmp_widget;
char *nbuddy;

 nbuddy=gtk_object_get_data(GTK_OBJECT(widget), "buddy");
 if (nbuddy) {
	if (! find_friend(nbuddy)) {
		GtkWidget *okbutton;
		okbutton=show_confirm_dialog(_("Would you like to add this user to your buddy list?"),"YES","NO");
		if (!okbutton) {return;}
  		gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_add_accept_cb), NULL);
 		gtk_object_set_data (GTK_OBJECT (okbutton), "buddy", strdup(nbuddy));
										}
 	free(nbuddy);
		 }
	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}
}


/* Join a room menu callback */

void on_joinaroom_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {newsite[i]=etext[i]; }
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

	if (strlen(nsite)>0) {
		snprintf(callbackbuf, 128, "/join %s", nsite);
		set_current_chat_profile_name( get_default_profile_name());
		chat_command(callbackbuf);
				      }
	if (etext) {g_free(etext);}
}


void on_joinaroom  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

okbutton=show_input_dialog(_("Room"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_joinaroom_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}


/* webcam stuff */


void on_viewcam_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
				}

	if (strlen(nsite)>0) {
		if (strcasecmp(nsite, ymsg_sess->user )) {
			snprintf(callbackbuf, 128, "/viewcam %s", nsite);
			chat_command(callbackbuf);
																		  }
				      }
	if (etext) {g_free(etext);}
}


void on_viewcam  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

/* added to GETTEXT here */
okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_viewcam_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	if (find_profile_name(selected_buddy)) {return;}
	if (!strcasecmp(selected_buddy, ymsg_sess->user)) {return;}
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}


void on_viewcam_room  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;
	if ( ! user_selected )	{return;}

/* added to GETTEXT here */
okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_viewcam_cb), NULL);
if (user_selected)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), user_selected);
			      }
}

void on_startmycam(GtkMenuItem *widget, gpointer user_data) {
	yahoo_webcam_get_feed(get_default_profile_name());
}


void on_sendcaminvite_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {gtk_widget_destroy( tmp_widget );}
	if (strlen(nsite)>0) {
		if (strcasecmp(nsite, ymsg_sess->user )) {
		ymsg_webcam_invite( ymsg_sess, nsite); 
																		  }
							}
	if (etext) {g_free(etext);}
}

void on_sendcaminvite  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_sendcaminvite_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	if (find_profile_name(selected_buddy)) {return;}
	if (!strcasecmp(selected_buddy, ymsg_sess->user)) {return;}
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}

void on_sendcaminvite_room  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;
	if ( ! user_selected )	{return;}

okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_sendcaminvite_cb), NULL);
if (user_selected)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), user_selected);
			       }
}

void on_show_camera_on  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (!menuitem) {return;}
	if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(menuitem))) {
		 show_cam_is_on=1;
																				} else {show_cam_is_on=0;}
	if (in_a_chat) {
		snprintf(callbackbuf,16, "%s", "/rejoin");
		chat_command(callbackbuf);
		}
}

void on_display_plugin_info  (GtkMenuItem *menuitem, gpointer  user_data) {
	GtkWidget *tmp_widget=NULL;
	print_loaded_plugin_info();
	tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
	if (tmp_widget) {
		gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 0);
	}
	gdk_window_raise(chat_window->window );
}


void collect_buddy_list_selected(GtkTreeSelection *selection, gpointer data)  {
	GtkTreeModel *model;
	GtkTreeIter iter;

		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			gchar *sfound;
			char *gtemp=NULL;

			if ( selected_buddy_group ) {
				free( selected_buddy_group );
				selected_buddy_group=NULL;
			}
			if ( selected_buddy ) {
				free( selected_buddy );  
				selected_buddy=NULL;
			}

		if (budgrp_menu) {gtk_menu_popdown((GtkMenu *) budgrp_menu);}
		if (budpop_menu) {gtk_menu_popdown((GtkMenu *) budpop_menu);}

			gtk_tree_model_get(model, &iter,GYDOUBLE_TOP, &sfound, -1);	
			gtemp=strdup(sfound);
			if (!strcmp(gtemp, "GYEBuddy_TopLvl")) {  /* buddy group selected */
				gchar *gfound;
				g_free(sfound);
				free(gtemp);
				gtk_tree_model_get(model, &iter,GYDOUBLE_COL1, &gfound, -1);	
				if (strcmp(gfound,TMP_FRIEND_CB)) {
					selected_buddy_group=strdup(gfound);
				}
				g_free(gfound);
				return;
			}	else {  /* regular buddy selected */
				gchar *gfound;
				if (strcmp(gtemp, TMP_FRIEND_CB)) {
					selected_buddy_group=strdup(gtemp);
				}
				free(gtemp);
				g_free(sfound);
				gtk_tree_model_get(model, &iter,GYDOUBLE_COL1, &gfound, -1);	
				if (strcmp(gfound, TMP_FRIEND_CB)) {
					selected_buddy=strdup(gfound);
				}
				g_free(gfound);
				return;
			            }
		} 
}


void on_buddy_list_rename_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	gchar *etext=NULL;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
						}
	ymsg_group_rename( ymsg_sess, etext );
	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	if (etext) {g_free(etext);}
}

void on_buddy_list_rename (GtkMenuItem  *menuitem,   gpointer user_data)
{
GtkWidget *okbutton, *tmpentry;
	if ( ! selected_buddy_group ) {return;}
	if (strcmp(selected_buddy_group, TMP_FRIEND_CB)==0) {
		show_ok_dialog(_("You cannot rename or delete the selected buddy group."));
		return;
	}
	if (strcmp(selected_buddy_group, "Y! Mobile Messenger")==0) {
		show_ok_dialog(_("You cannot rename or delete the selected buddy group."));
		return;
	}
/* added to GETTEXT here */
okbutton=show_input_dialog("Group","Rename","Cancel");

	tmpentry=gtk_object_get_data (GTK_OBJECT (okbutton), "myentry");
	if (tmpentry) {
		gtk_entry_set_text(GTK_ENTRY(tmpentry),selected_buddy_group);
	}

  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_list_rename_cb), NULL);
}

void on_move_buddy_to_group_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	gchar *etext=NULL;
	char *selbud=NULL;

	if (!selected_buddy) {return;}
	if (!selected_buddy_group) {return;}
	if (strcmp(selected_buddy_group, TMP_FRIEND_CB)==0) {return;}

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "window" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "entry");
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
						}
	selbud=strdup(selected_buddy);

	/* Remove buddy from old group */
	ymsg_remove_buddy( ymsg_sess,selbud );

	/* Re-add buddy to new group */
	if (selected_buddy_group) {free(selected_buddy_group);}
	selected_buddy_group=strdup(etext);
	ymsg_add_buddy( ymsg_sess,selbud );

	if (tmp_widget)  {
		gtk_widget_destroy( tmp_widget );
						    }
	free(selbud);
	if (etext) {g_free(etext);}
}

void on_move_buddy_to_group (GtkMenuItem  *menuitem,   gpointer user_data) {
	GtkWidget *okbutton;
	if (!selected_buddy) {return;}
	if (!selected_buddy_group) {return;}
	if (strcmp(selected_buddy_group, TMP_FRIEND_CB)==0) {return;}
	okbutton=create_list_dialog(_("Group"), _("Group:"), selected_buddy_group, get_buddy_group_list(), TRUE, 6);
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC ( on_move_buddy_to_group_cb),
                      NULL);
}

void on_open_multi_pms (GtkMenuItem  *menuitem,   gpointer user_data) {
	use_multpm_settings=1;
	use_chat_settings=0;
	open_make_conference_window();
}


void on_gameprofile_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;

	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {gtk_widget_destroy( tmp_widget );}
	if (strlen(nsite)>0) {
		GtkWidget *tmp_widget=NULL;
		tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
		snprintf(callbackbuf, 256, "http://profiles.games.yahoo.com/games/profile2?name=%s", nsite);
		load_myyahoo_URL(callbackbuf);
		if (tmp_widget) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
		}
		gdk_window_raise(chat_window->window );
							}
	if (etext) {g_free(etext);}
}

void on_gameprofile  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_gameprofile_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			      }
}

void view_full_sized_avatar(char *who, char *avatar, int animation) {
	if (!avatar ) {return;}	
 if (! emulate_ymsg6) { return; }

	if (animation) {
		snprintf( callbackbuf, 400, "%s \"http://img.avatars.yahoo.com/users/%s.full.swf\" &", flash_player_cmd?flash_player_cmd:"gflashplayer", avatar );
		my_system(callbackbuf);
	} else {		
		char *tmppage=NULL;
		GtkWidget *tmp_widget=NULL;
		if (!who ) {return;}
		tmppage=malloc(1184);
		if (!tmppage) {return;}
		tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
		snprintf(tmppage, 1182, "<br><center><img  src=\"http://img.avatars.yahoo.com/users/%s.large.%s\"><br>%s</center><br> <br><center><a href=\"http://img.avatars.yahoo.com/users/%s.large.png\">http://img.avatars.yahoo.com/users/%s.large.png</a><br><a href=\"http://img.avatars.yahoo.com/users/%s.medium.png\">http://img.avatars.yahoo.com/users/%s.medium.png</a><br><a href=\"http://img.avatars.yahoo.com/users/%s.small.png\">http://img.avatars.yahoo.com/users/%s.small.png</a><br><a href=\"http://img.avatars.yahoo.com/users/%s.full.swf\">http://img.avatars.yahoo.com/users/%s.full.swf</a><br>", avatar,avatar_filetype, who, avatar, avatar, avatar, avatar,avatar, avatar,avatar, avatar);
		set_myyahoo_html_data(tmppage);
		free(tmppage);
		if (tmp_widget) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
			}
		gdk_window_raise(chat_window->window );
	}
}


void on_viewavatar  (GtkMenuItem *menuitem, gpointer  user_data) {
	if (!emulate_ymsg6) {return;}
	if (!selected_buddy)  {return; }
	else {
		struct yahoo_friend *FROBJECT = NULL;
		FROBJECT=yahoo_friend_find(selected_buddy);
		if (FROBJECT) {
			if (FROBJECT->avatar) {
					if (user_data) {
						view_full_sized_avatar(selected_buddy, FROBJECT->avatar, 1); 
					} else {view_full_sized_avatar(selected_buddy, FROBJECT->avatar, 0); }
			} else {show_ok_dialog(_("The user has no avatar."));}
		} else {show_ok_dialog(_("The user has no avatar."));}
	}
}


void on_viewgameroom  (GtkMenuItem *menuitem, gpointer  user_data) {
	if (!selected_buddy)  {return; }
	else {
		struct yahoo_friend *FROBJECT = NULL;
		FROBJECT=yahoo_friend_find(selected_buddy);
		if (FROBJECT) {
			if (FROBJECT->game_url) {
				if (strlen(FROBJECT->game_url)>1)  {
					snprintf( callbackbuf, 400, "%s", FROBJECT->game_url );
					display_url( (void *) strdup(callbackbuf) );
				} else {show_ok_dialog(_("The user is not in a game room."));}
			} else {show_ok_dialog(_("The user is not in a game room."));}
		} else {show_ok_dialog(_("The user is not in a game room."));}
	}
}


void on_viewbuddyinfo  (GtkMenuItem *menuitem, gpointer  user_data) {
	if (!selected_buddy)  {return; }
	else {
		struct yahoo_friend *FROBJECT = NULL;
		FROBJECT=yahoo_friend_find(selected_buddy);
		if (FROBJECT) {
			char *b_info_buf=NULL;
			b_info_buf=malloc(5120);
			if (! b_info_buf) {return;}

			snprintf(callbackbuf,85, "</font>\n[~G1y_eBreak~]<font face=\"sans\" size=\"14\">%s", YAHOO_COLOR_PMBLUE);

			snprintf( b_info_buf, 500, "<font face=\"Arial\" size=\"16\">%s[ %s ]\n%s\"%s\" ", YAHOO_COLOR_PMPURPLE, selected_buddy, YAHOO_COLOR_PMBLUE,  FROBJECT->main_stat?FROBJECT->main_stat:""  );

			if (FROBJECT->away) {
				strcat(b_info_buf," (");
				strncat(b_info_buf,_("Away"), 15);
				strcat(b_info_buf,") ");
			}		
			if (FROBJECT->idle) {
				strcat(b_info_buf," (");
				strncat(b_info_buf,_("Idle"), 15);
				strcat(b_info_buf,") ");
			}	
			strcat(b_info_buf,callbackbuf);

			if (FROBJECT->inchat) {
				strncat(b_info_buf,_("Chat"), 15);
				strcat(b_info_buf,callbackbuf);
			}
			if (FROBJECT->ingames) {
				strncat(b_info_buf,_("Games"), 15);
				strcat(b_info_buf,callbackbuf);
			}
			if (FROBJECT->webcam) {
				strncat(b_info_buf,_("Webcam"), 20);
				strcat(b_info_buf,callbackbuf);
			}
			if (FROBJECT->insms) {
				strncat(b_info_buf,_("SMS (Cell Phone)"), 25);
				strcat(b_info_buf,callbackbuf);
			}
			if (FROBJECT->launchcast) {
				strncat(b_info_buf,"LaunchCAST", 12);
				if (FROBJECT->radio_stat) {
					if (strlen(FROBJECT->radio_stat)>1) {
						strncat(b_info_buf,": ", 3);
						strncat(b_info_buf,FROBJECT->radio_stat, 45);
					}
				}
				strcat(b_info_buf,callbackbuf);
			}

			if (FROBJECT->avatar) {
				if (strlen(FROBJECT->avatar)>1) {
						strncat(b_info_buf,"Avatar", 7);
						strcat(b_info_buf,callbackbuf);
				}
			}

			strncat(b_info_buf, _("Identities"), 25);
			strcat(b_info_buf,": ");
			strncat(b_info_buf, get_buddy_identities_list(selected_buddy) , 288);
			strcat(b_info_buf,callbackbuf);
			strcat(b_info_buf,"\n</font>");

			if (prof_text) {g_free(prof_text); prof_text=NULL;}
			prof_text = g_strdup( _utf(b_info_buf) );
			free(b_info_buf);
			prof_pic=NULL;
			if (FROBJECT->avatar) {
				if (download_yavatar(FROBJECT->avatar, 1 , NULL)) {
					char filename[192];
					snprintf( filename, 190, "%s/yavatars/%s.medium.%s", GYACH_CFG_DIR, FROBJECT->avatar, avatar_filetype );
					prof_pic=filename;
						/* printf("filename: %s\n", prof_pic); fflush(stdout); */ 
				}
			}
			if ( profile_window ) {
					gtk_widget_destroy( profile_window );
					profile_window = NULL;
			}
			display_profile();
			if (profile_window) {
				gtk_window_set_title (GTK_WINDOW (profile_window), selected_buddy);
			}
		}
	}
}




void on_open_stealth_window  (GtkMenuItem *menuitem, gpointer  user_data) {
	if (!emulate_ymsg6) {return;}
	if (!selected_buddy)  {return; }

	else {
		GtkWidget *swin, *offline, *online, *permoff;
		struct yahoo_friend *FROBJECT = NULL;
		FROBJECT=yahoo_friend_find(selected_buddy);
		if (stealth_sel_buddy) {free(stealth_sel_buddy);}
		stealth_sel_buddy=strdup(selected_buddy);
		if (FROBJECT) {
			swin=create_stealth_window(selected_buddy);
			offline=gtk_object_get_data(GTK_OBJECT(swin), "offline");
			online=gtk_object_get_data(GTK_OBJECT(swin), "online");
			permoff=gtk_object_get_data(GTK_OBJECT(swin), "permoff");
			if (my_status != 12) { /* visible */
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (online), TRUE);
					gtk_widget_set_sensitive(offline, FALSE);
					if (FROBJECT->stealth==3) {
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (permoff), TRUE);
						}
				} else { /* invisible */
					gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (offline), TRUE);
					if (FROBJECT->stealth==3) {
						gtk_widget_set_sensitive(offline, FALSE);
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (permoff), TRUE);
					}
					if (FROBJECT->stealth==1) {
						gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (online), TRUE);
					}
						 }
			gtk_widget_show_all(swin);
		}
	}
}

void on_activate_stealth_window  (GtkButton *widget, gpointer  user_data) {
	GtkWidget *swin=NULL;
	GtkWidget  *offline, *online, *permoff;
	struct yahoo_friend *FROBJECT = NULL;		
	swin=gtk_object_get_data(GTK_OBJECT(widget), "mywindow");
	if (!stealth_sel_buddy)  {if (swin) {gtk_widget_destroy(swin);} return; }
	FROBJECT=yahoo_friend_find(stealth_sel_buddy);
	if (!FROBJECT)  {if (swin) {gtk_widget_destroy(swin);} return; }

	offline=gtk_object_get_data(GTK_OBJECT(swin), "offline");
	online=gtk_object_get_data(GTK_OBJECT(swin), "online");
	permoff=gtk_object_get_data(GTK_OBJECT(swin), "permoff");

	if (my_status != 12) { /* visible */
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (permoff))) {
			if (FROBJECT->stealth<3) {
				ymsg_stealth(ymsg_sess, stealth_sel_buddy, 0);
				ymsg_stealth_perm(ymsg_sess, stealth_sel_buddy, 0);
			}
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (online))) {
			if (FROBJECT->stealth==3) {
				ymsg_stealth_perm(ymsg_sess, stealth_sel_buddy, 1);
				ymsg_stealth(ymsg_sess, stealth_sel_buddy, 1);
			}
		}
	} else { /* invisible */
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (permoff))) {
			if (FROBJECT->stealth<3) {
				ymsg_stealth(ymsg_sess, stealth_sel_buddy, 0);
				ymsg_stealth_perm(ymsg_sess, stealth_sel_buddy, 0);
			}
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (online))) {
				if (FROBJECT->stealth==3) {ymsg_stealth_perm(ymsg_sess,stealth_sel_buddy,1);}
				ymsg_stealth(ymsg_sess, stealth_sel_buddy, 1);
		}
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (offline))) {
				ymsg_stealth(ymsg_sess, stealth_sel_buddy, 0);
		}

	}

	if (swin) {gtk_widget_destroy(swin);}
}

void on_send_audible_cb(GtkWidget *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	PM_SESSION *pm_sess;
	char *smiley_t=NULL;
	char *myalias=NULL;
	char *aud_text=NULL;
	char *aud_hash=NULL;
	char *aud_file=NULL;

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget),"mywindow");
	pm_sess=gtk_object_get_data(GTK_OBJECT(widget),"pms_sess");
	smiley_t=gtk_object_get_data(GTK_OBJECT(widget),"smiley_tag");

	if (pm_sess && smiley_t)  {
		focus_pm_entry(pm_sess->pm_window);
		if ( ! check_gy_audible(smiley_t) ) {
			if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
			return;
		}

		aud_text=get_gy_audible_text(smiley_t);
		aud_hash=get_gy_audible_hash(smiley_t);
		aud_file=get_gy_audible_disk_name( smiley_t );

		if ( (! aud_text) || (! aud_hash) || (! aud_file) ) {
			if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
			if (aud_file) {free(aud_file);}
			if (aud_text) {free(aud_text);}
			if (aud_hash) {free(aud_hash);}
			return;
		}
		set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );

		ymsg_audible(ymsg_sess, pm_sess->pm_user, smiley_t, aud_text, aud_hash);

		if ( chat_timestamp_pm ) {
			append_timestamp(pm_sess->pm_window,pm_sess->pm_text);
		}
		if (pm_nick_names) {myalias=get_screenname_alias(get_current_pm_profile_name());}

		snprintf( callbackbuf, 256, "%s%s%s%s%s  ",
			YAHOO_COLOR_PMBLUE, YAHOO_STYLE_BOLDON, 
			myalias?myalias:get_current_pm_profile_name(),
			YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );
		append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, callbackbuf );
		
		snprintf(callbackbuf,350,"%s/audibles/%s.gif", PACKAGE_DATA_DIR, aud_file);
		ct_append_pixmap(callbackbuf, pm_sess->pm_text);

		snprintf( callbackbuf, 500, " %s . o O ( %s ) %s\n",
			"\033[#A7708Cm", aud_text,YAHOO_COLOR_BLACK  );
		append_to_textbox_color( pm_sess->pm_window, pm_sess->pm_text, callbackbuf );

		/* Play mp3 here */
		play_audible(smiley_t);

		if (myalias) {g_free(myalias); myalias=NULL;}
		if (aud_text) {free(aud_text);}
		if (aud_hash) {free(aud_hash);}
		if (aud_file) {free(aud_file);}
		reset_current_pm_profile_name();

		if (( my_status ) &&
			( my_status != 12 ) &&
			( pm_brings_back )) {
			cmd_away( "0" );
										  }

		show_incoming_pm(pm_sess, 0);

	}

	if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
}


void on_open_audible_window(GtkWidget *button, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;

	if (!emulate_ymsg6) {return;}
	if (! enable_audibles) {return;}

	if (( pm_lptr=find_pm_session_from_widget( GTK_WIDGET(button), "pm_aud" )) != NULL) {
		pm_sess = pm_lptr->data;
		show_audibles_window(pm_sess);
	}
}


void on_activate_yprivacy(GtkWidget *button, gpointer user_data) {
	GtkWidget *swin;
	GtkWidget *gameson, *gameshs, *yonline;
	int doit=0;
	char fetchit[64]="";

	swin=gtk_object_get_data(GTK_OBJECT(button), "mywindow");
	gameson=gtk_object_get_data(GTK_OBJECT(button), "gameson");
	gameshs=gtk_object_get_data(GTK_OBJECT(button), "gameshs");
	yonline=gtk_object_get_data(GTK_OBJECT(button), "yonline");


	if (gameson) {
		doit=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gameson));
		snprintf(callbackbuf, 100,
			 "http://insider.msg.yahoo.com/ycontent/?setgp=0&enabled=%s", 
			doit?"1":"0");
		set_max_url_fetch_size(62);
		fetch_url( callbackbuf,  fetchit, ymsg_sess->cookie );
	}
	if (gameshs) {
		doit=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gameshs));
		snprintf(callbackbuf, 100,
			 "http://mtab.games.yahoo.com/messenger/prefs?action=set&show_hs=%s", 
			doit?"1":"0");
		set_max_url_fetch_size(62);
		fetch_url( callbackbuf,  fetchit, ymsg_sess->cookie );
	}
	if (yonline) {
		doit=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(yonline));
		snprintf(callbackbuf, 100,
			 "http://edit.profiles.yahoo.com/config/edit_identity?.opi=%s&.set=1", 
			doit?"0":"1");
		set_max_url_fetch_size(62);
		fetch_url(callbackbuf,  fetchit, ymsg_sess->cookie );
	}
	
	if (swin) {gtk_widget_destroy(swin);}
}

void on_open_yprivacy_window  (GtkMenuItem *menuitem, gpointer  user_data) {
	create_yprivacy_window ();
}

void on_remove_me_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_entry;
	char newsite[64]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "entry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {newsite[i]=etext[i]; }
						}
	if (strlen(nsite)>0) {
		snprintf(callbackbuf, 128, "/revoke %s", nsite);
		chat_command(callbackbuf);
		snprintf(callbackbuf, 5, "%s", "");
		gtk_entry_set_text(GTK_ENTRY(tmp_entry), callbackbuf);
				      }
	if (etext) {g_free(etext);}
}

void on_perm_offline_input_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_entry;
	char newsite[64]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;
	 if (! emulate_ymsg6) { return; }
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "entry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {newsite[i]=etext[i]; }
						}
	if (strlen(nsite)>0) {
		if (user_data)  {  /* bring online */
			ymsg_stealth_perm(ymsg_sess, nsite, 1);
			ymsg_stealth(ymsg_sess, nsite, 1);
		} else {  /* send offline */
			ymsg_stealth(ymsg_sess, nsite, 0);
			ymsg_stealth_perm(ymsg_sess, nsite, 0);
		}
		snprintf(callbackbuf, 5, "%s", "");
		gtk_entry_set_text(GTK_ENTRY(tmp_entry), callbackbuf);
				      }
	if (etext) {g_free(etext);}
}


void update_my_buddy_image(PM_SESSION *pm_sess, int is_me) {
	GdkPixbuf *imbuf=NULL;
	GdkPixbuf *imbuf2=NULL;
	int height=0;
	int width=0;
	int scaleto=100;	
	if (!show_bimages) {return;}
	 if (! emulate_ymsg6) { return; }
	if (! pm_sess->buddy_image_visible) {return;}
	if (is_me) { 		
			if (!pm_sess->bimage_me) {return;}
			if ( (! bimage_file) || (! pm_sess->buddy_image_share) ) {
				gtk_image_set_from_stock(GTK_IMAGE(pm_sess->bimage_me), GTK_STOCK_NEW, GTK_ICON_SIZE_DND);		
					gtk_widget_show_all(pm_sess->bimage_me);
				return;
			}

		}
	if (! is_me) { 
		if (!pm_sess->bimage_them) {return;}
		 if (! pm_sess->buddy_image_file) {return;} 
	}
	if (pm_sess->buddy_image_size > 2) {pm_sess->buddy_image_size=2;}
	if (pm_sess->buddy_image_size < 0) {pm_sess->buddy_image_size=0;}

	if (pm_sess->buddy_image_size==0) {scaleto=54;}
	if (pm_sess->buddy_image_size==2) {scaleto=142;}

	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);

	imbuf=gdk_pixbuf_new_from_file(is_me?callbackbuf:pm_sess->buddy_image_file, NULL);
	if (!imbuf) {return;}

	width = gdk_pixbuf_get_width( imbuf );
	height = gdk_pixbuf_get_height( imbuf );
	if (is_me) { 
		 if (pm_sess->buddy_image_share==2) {height = scaleto;}  
		else {height = ( 1.0 * height / width ) * scaleto; }
				 } else {height = ( 1.0 * height / width ) * scaleto; }

	if (height>210) {height=210;}
	width = scaleto;

	imbuf2=gdk_pixbuf_scale_simple( imbuf, width, height,
				GDK_INTERP_BILINEAR );

	gtk_image_set_from_pixbuf(GTK_IMAGE(is_me?pm_sess->bimage_me:pm_sess->bimage_them), imbuf2);

	gtk_widget_show_all(is_me?pm_sess->bimage_me:pm_sess->bimage_them);

	g_object_unref(imbuf);
	g_object_unref(imbuf2);
}
	
void on_bimage_share  (GtkMenuItem *widget, gpointer user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *my_widget=(GtkWidget *) user_data;
	if (! user_data) {return;}	
	if (!show_bimages) {return;}
	if (( pm_lptr = find_pm_session_from_widget( my_widget, "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
			pm_sess->buddy_image_share=bimage_share;
			update_my_buddy_image(pm_sess, 1);
			set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, pm_sess->buddy_image_share);
			reset_current_pm_profile_name();
		}
}

void on_bimage_share_off  (GtkMenuItem *widget, gpointer user_data)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *my_widget=(GtkWidget *) user_data;
	if (! user_data) {return;}	
	if (!show_bimages) {return;}
	if (( pm_lptr = find_pm_session_from_widget( my_widget, "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
			pm_sess->buddy_image_share=0;
			update_my_buddy_image(pm_sess, 1);
			set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
			ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, pm_sess->buddy_image_share);
			reset_current_pm_profile_name();
		}
}


void on_bimage_update_size  (GtkMenuItem *widget, gpointer user_data, int bimsize)
{
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *my_widget=(GtkWidget *) user_data;
	if (! user_data) {return;}
	if (( pm_lptr = find_pm_session_from_widget( my_widget, "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
			pm_sess->buddy_image_size=bimsize;
			update_my_buddy_image(pm_sess, 1);
			update_my_buddy_image(pm_sess, 0);
		}
}

void on_bimage_update_size1  (GtkMenuItem *widget, gpointer user_data) {
	on_bimage_update_size(widget, user_data, 0);
}
void on_bimage_update_size2  (GtkMenuItem *widget, gpointer user_data) {
	on_bimage_update_size(widget, user_data, 1);
}
void on_bimage_update_size3  (GtkMenuItem *widget, gpointer user_data) {
	on_bimage_update_size(widget, user_data, 2);
}

void on_bimage_show_cb  (GtkMenuItem *widget, gpointer user_data)
{
	if (! user_data) {return;}
	gtk_widget_show_all((GtkWidget *) user_data);
}
void on_bimage_hide_cb  (GtkMenuItem *widget, gpointer user_data)
{
	if (! user_data) {return;}
	gtk_widget_hide((GtkWidget *) user_data);
}

void on_bimage_view_my_avatar_cb  (GtkMenuItem *menuitem, gpointer  user_data) {
	if (!emulate_ymsg6) {return;}
	if (!my_avatar) {
		show_ok_dialog("You have no avatar.");
		return;
	}
	if (user_data) {
		view_full_sized_avatar(ymsg_sess->user, my_avatar, 1); 
	} else {
		view_full_sized_avatar(ymsg_sess->user, my_avatar, 0); 
	}
}


void on_bimage_view_avatar  (GtkMenuItem *menuitem, gpointer  user_data) {
	struct yahoo_friend *FROBJECT = NULL;
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *my_widget=(GtkWidget *) user_data;
	if (! user_data) {return;}
	
	if (( pm_lptr = find_pm_session_from_widget( my_widget, "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
		FROBJECT=yahoo_friend_find(pm_sess->pm_user);
		if (FROBJECT) {
			if (FROBJECT->avatar) {
				view_full_sized_avatar(pm_sess->pm_user, FROBJECT->avatar, 0); 
			} else {show_ok_dialog(_("The user has no avatar.")	);}
		} else {show_ok_dialog(_("This feature is only available for friends."));}
	}
}

void on_bimage_view_animation  (GtkMenuItem *menuitem, gpointer  user_data) {
	struct yahoo_friend *FROBJECT = NULL;
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	GtkWidget *my_widget=(GtkWidget *) user_data;
	if (! user_data) {return;}
	
	if (( pm_lptr = find_pm_session_from_widget( my_widget, "pms_entry" )) != NULL ) {
		pm_sess = pm_lptr->data;
		FROBJECT=yahoo_friend_find(pm_sess->pm_user);
		if (FROBJECT) {
			if (FROBJECT->avatar) {
				view_full_sized_avatar(pm_sess->pm_user, FROBJECT->avatar, 1); 
			} else {show_ok_dialog(_("The user has no avatar.")	);}
		} else {show_ok_dialog(_("This feature is only available for friends."));}
	}
}


void on_bimage_edit_my_photo(GtkButton *widget, gpointer user_data) {
	struct stat sbuf;
	GtkWidget *imwidg=NULL;

	 if (! emulate_ymsg6) { return; }
	imwidg=create_bimage_select_window();
	if (widget != NULL) {
	 	GtkWidget *textfocus=NULL;
		textfocus=gtk_object_get_data(GTK_OBJECT(widget),"textfocus" );
		if (textfocus != NULL) {
			GtkWidget *touched=NULL;	
			gtk_object_set_data(GTK_OBJECT(imwidg),"textfocus", textfocus);
			touched=gtk_object_get_data(GTK_OBJECT(imwidg),"okbutton");
			if (touched) {gtk_object_set_data(GTK_OBJECT(touched),"textfocus", textfocus);}
			touched=gtk_object_get_data(GTK_OBJECT(imwidg),"cancel_button");
			if (touched) {gtk_object_set_data(GTK_OBJECT(touched),"textfocus", textfocus);}
		}
	}

	imwidg=NULL;

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_check1), show_bimages?TRUE:FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_check2), bimage_friend_only?TRUE:FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_check3), show_yavatars?TRUE:FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_check4), share_blist_avatar?TRUE:FALSE);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_size_radio2),TRUE);
	if (bimage_size==2) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_size_radio3),TRUE);
	}
	if (bimage_size==0) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_size_radio1),TRUE);
	}

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_radio1),TRUE);
	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);
	if ( (bimage_share==1) && bimage_file) {
		imwidg=bimdiag_image2;
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry2), bimage_file);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_radio3),TRUE);
	}
	if ( (bimage_share==2) && bimage_file) {
		imwidg=bimdiag_image1;
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry1), bimage_file);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(bimdiag_radio2),TRUE);
	}

	if (imwidg)  {
		GdkPixbuf *imbuf=NULL;
		imbuf=gdk_pixbuf_new_from_file(callbackbuf, NULL);
		if (! imbuf) {	show_ok_dialog(_("The image could not be loaded."));  }
		else {
			int width=0;
			int height=0;
			GdkPixbuf *imbuf2=NULL;
			width = gdk_pixbuf_get_width( imbuf );
			height = gdk_pixbuf_get_height( imbuf );
			height = ( 1.0 * height / width ) * 96; 
			if (height>200) {height=200;}
			width = 96;
			imbuf2=gdk_pixbuf_scale_simple( imbuf, width, height,
				GDK_INTERP_BILINEAR );

			gtk_image_set_from_pixbuf(GTK_IMAGE(imwidg), imbuf2 );
			gtk_widget_show_all(GTK_WIDGET(imwidg));
			g_object_unref(imbuf);
			g_object_unref(imbuf2);
		}
	}

	if ( (bimage_share != 1) && my_avatar) {  /* not the avatar, but show it if available */
		snprintf( callbackbuf, 300, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, my_avatar, avatar_filetype);
		if (! stat( callbackbuf, &sbuf )) {
		GdkPixbuf *imbuf=NULL;
		imbuf=gdk_pixbuf_new_from_file(callbackbuf, NULL);
		if (! imbuf) {	show_ok_dialog(_("The image could not be loaded."));  }
		else {
			int width=0;
			int height=0;
			GdkPixbuf *imbuf2=NULL;
			width = gdk_pixbuf_get_width( imbuf );
			height = gdk_pixbuf_get_height( imbuf );
			height = ( 1.0 * height / width ) * 96; 
			if (height>200) {height=200;}
			width = 96;
			imbuf2=gdk_pixbuf_scale_simple( imbuf, width, height,
				GDK_INTERP_BILINEAR );

			gtk_image_set_from_pixbuf(GTK_IMAGE(bimdiag_image2), imbuf2 );
			gtk_widget_show_all(GTK_WIDGET(bimdiag_image2));
			g_object_unref(imbuf);
			g_object_unref(imbuf2);			
			gtk_entry_set_text(GTK_ENTRY(bimdiag_entry2), callbackbuf);
			}
		}
	}

}


void on_bimage_edit_my_photo_menu  (GtkMenuItem *menuitem, gpointer  user_data) {
	on_bimage_edit_my_photo(NULL, NULL);
}	


void on_select_buddy_image_activate(GtkButton *button, gpointer user_data) {
	struct stat sbuf;
	char *imy_file;
	GtkWidget *tmp_widget;
	      tmp_widget =gtk_object_get_data(GTK_OBJECT(button),"mywindow");
	      if (!tmp_widget) {return;}

	imy_file=strdup(gtk_file_selection_get_filename( GTK_FILE_SELECTION( tmp_widget )));
	if (strlen(imy_file)<1) {free(imy_file); return;}

	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);
	unlink(callbackbuf);

	snprintf( callbackbuf, 300, "convert -resize 96x96! -colors 128 -quality 7 \"%s\"  \"%s/.friend_icon.png\" ", imy_file, GYACH_CFG_DIR);
	my_system(callbackbuf);
	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);

	if ( stat( callbackbuf, &sbuf )) {
		show_ok_dialog(_("The image could not be converted to a 96x96 .PNG image."));
		snprintf( callbackbuf, 3, "%s", "");
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry1), callbackbuf);
		free(imy_file); 
		return;
	} else {
		GdkPixbuf *imbuf=NULL;
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry1), imy_file);
		imbuf=gdk_pixbuf_new_from_file(callbackbuf, NULL);
		if (! imbuf) {
			show_ok_dialog(_("The image could not be loaded."));
			free(imy_file); 
			snprintf( callbackbuf, 3, "%s", "");
			gtk_entry_set_text(GTK_ENTRY(bimdiag_entry1), callbackbuf);
			return;
		}
		gtk_image_set_from_pixbuf(GTK_IMAGE(bimdiag_image1), imbuf );
		gtk_widget_show_all(GTK_WIDGET(bimdiag_image1));
		g_object_unref(imbuf);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (bimdiag_radio2), TRUE);
	}

	gtk_widget_destroy(tmp_widget);
	free(imy_file);
}


void on_select_buddy_image(GtkButton *widget, gpointer user_data) {
	GtkWidget *okbutton;
	if (bimage_file) {preselected_filename(bimage_file);}	
	okbutton=create_fileselection2();

  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_select_buddy_image_activate),
                      NULL);
}

void on_goto_edit_avatar  (GtkButton *button, gpointer  user_data) {
	snprintf( callbackbuf, 200, "%s", "http://avatars.yahoo.com");
	display_url( (void *) strdup(callbackbuf) );
}

void on_goto_edit_avatar_menu (GtkMenuItem *menuitem, gpointer  user_data) {
	on_goto_edit_avatar  (NULL, NULL);
}

void on_delete_all_yavatars (GtkButton *button, gpointer  user_data) {
	delete_all_yavatars();
}
void on_delete_all_bimages (GtkButton *button, gpointer  user_data) {
	delete_all_bimages();
}

void on_refresh_bavatar (GtkButton *button, gpointer  user_data) {
	struct stat sbuf;

	if (! my_avatar) {  return;}
	delete_yavatars(my_avatar);
	download_yavatar(my_avatar, 0, NULL);
	if (! download_yavatar(my_avatar, 2, NULL)) { return; }

	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);
	unlink(callbackbuf);

	snprintf( callbackbuf, 300, "convert -colors 96 -quality 9 \"%s/yavatars/%s.large.%s\"  \"%s/.friend_icon.png\" ", GYACH_CFG_DIR, my_avatar, avatar_filetype, GYACH_CFG_DIR);
	my_system(callbackbuf);

	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);

	if ( stat( callbackbuf, &sbuf )) {
		show_ok_dialog(_("The image could not be loaded."));
		snprintf( callbackbuf, 3, "%s", "");
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry2), callbackbuf);
		return;
	} else {
		GdkPixbuf *imbuf=NULL;
		GdkPixbuf *imbuf2=NULL;
		int width=0;
		int height=0;
		imbuf=gdk_pixbuf_new_from_file(callbackbuf, NULL);
	snprintf( callbackbuf, 300, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, my_avatar, avatar_filetype);
		gtk_entry_set_text(GTK_ENTRY(bimdiag_entry2), callbackbuf);
		if (! imbuf) {
			show_ok_dialog(_("The image could not be loaded."));
			snprintf( callbackbuf, 3, "%s", "");
			gtk_entry_set_text(GTK_ENTRY(bimdiag_entry2), callbackbuf);
			return;
		}

		width = gdk_pixbuf_get_width( imbuf );
		height = gdk_pixbuf_get_height( imbuf );
		height = ( 1.0 * height / width ) * 96; 
		if (height>200) {height=200;}
		width = 96;

		imbuf2=gdk_pixbuf_scale_simple( imbuf, width, height,
				GDK_INTERP_BILINEAR );

		gtk_image_set_from_pixbuf(GTK_IMAGE(bimdiag_image2), imbuf2 );
		g_object_unref(imbuf);
		g_object_unref(imbuf2);
		update_buddy_clist();
	}
}


void convert_buddy_image() {

	snprintf( callbackbuf, 300, "%s/.friend_icon.png", GYACH_CFG_DIR);
	unlink(callbackbuf);

	if ( (bimage_share==1) && my_avatar  && bimage_file) {
		snprintf( callbackbuf, 400, "convert -colors 96 -quality 9 \"%s/yavatars/%s.large.%s\"  \"%s/.friend_icon.png\" ", GYACH_CFG_DIR, my_avatar, avatar_filetype, GYACH_CFG_DIR);
		my_system(callbackbuf);
	}

	if ( (bimage_share==2) && bimage_file) {
		snprintf( callbackbuf, 400, "convert -resize 96x96! -colors 128 -quality 7 \"%s\"  \"%s/.friend_icon.png\" ", bimage_file, GYACH_CFG_DIR);
		my_system(callbackbuf);
	}
}

void on_activate_bimage_config (GtkButton *button, gpointer  user_data) {
	GtkWidget *tmp_widget;
	int b_int=0;
	int b_size=0;
	char *avy=NULL;
	GList *this_session = pm_list;
	PM_SESSION *pm_sess;

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bimdiag_radio2))) {
		b_int=2;  /* photo */
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bimdiag_radio3))) {
		b_int=1;  /* avatar */
	}
	if (b_int==1) {
		avy=strdup(gtk_entry_get_text(GTK_ENTRY(bimdiag_entry2) ));
	}
	if (b_int==2) {
		avy=strdup(gtk_entry_get_text(GTK_ENTRY(bimdiag_entry1) ));
	}
	
	if (avy) {
		if (strlen(avy)<1) {free(avy);  return;}
		if (bimage_file) {free(bimage_file);}
		if (bimage_hash) {free(bimage_hash); bimage_hash=NULL;}
		bimage_timestamp=0;
		bimage_file=strdup(avy);
	}

	if (bimage_url) {free(bimage_url); bimage_url=NULL;}	

	bimage_share=b_int;

	show_bimages=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bimdiag_check1));
	bimage_friend_only=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bimdiag_check2));
	show_yavatars=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bimdiag_check3));
	share_blist_avatar=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(bimdiag_check4));

	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bimdiag_size_radio2))) {
		b_size=1;
	}
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (bimdiag_size_radio3))) {
		b_size=2;
	}
	bimage_size=b_size;
	ymsg_avatar_toggle(ymsg_sess,  share_blist_avatar);

	while( this_session ) {   /* any open PM sessions */
		int ichange=0;
		pm_sess = (PM_SESSION *)this_session->data;
		this_session = g_list_next( this_session );
			if (pm_sess->buddy_image_size != bimage_size) {ichange=1;}
			pm_sess->buddy_image_size=bimage_size;
			pm_sess->buddy_image_share=bimage_share;

			if ( (bimage_friend_only) && (! find_friend(pm_sess->pm_user)) && 
				(! find_temporary_friend(pm_sess->pm_user)) ) {
						pm_sess->buddy_image_share=0;
				}

			if (! pm_sess->buddy_image_share) {
				set_current_pm_profile_name(retrieve_profname_str(
					GTK_WIDGET(retrieve_profname_entry(pm_sess->pm_window)))  );
				ymsg_bimage_toggle(ymsg_sess, pm_sess->pm_user, 0);
				reset_current_pm_profile_name();
			} 
		update_my_buddy_image(pm_sess, 1);
		if (ichange ) {update_my_buddy_image(pm_sess, 0); } /* size change, update theirs */ 
	}

	if ( bimage_share && bimage_file) { upload_new_buddy_image(); }

	gtk_widget_destroy(bimdiag_window);

	tmp_widget=gtk_object_get_data(GTK_OBJECT(button), "textfocus");
	if (tmp_widget) {
		int sposs=0;
		if (gtk_object_get_data (GTK_OBJECT (tmp_widget), "is_gtk_entry") != NULL) {
			sposs=gtk_editable_get_position(GTK_EDITABLE(tmp_widget));
		}
		gtk_widget_grab_focus(GTK_WIDGET(tmp_widget));
		if (sposs > 0 ) {gtk_editable_set_position(GTK_EDITABLE(tmp_widget), sposs);}
	}

	write_config();
}


void on_pms_sendcaminvite (GtkMenuItem *widget, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "pms_menu1" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (selected_buddy)  {free(selected_buddy);}
		selected_buddy=strdup(pm_sess->pm_user);
		on_sendcaminvite  (widget, user_data);
	}
	return;
}

void on_pms_viewcam (GtkMenuItem *widget, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "pms_menu2" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (selected_buddy)  {free(selected_buddy);}
		selected_buddy=strdup(pm_sess->pm_user);
		on_viewcam(widget, user_data);
	}
	return;
}


void on_photoalbum_cb(GtkButton *widget, gpointer user_data) {
	GtkWidget *tmp_widget;
	GtkWidget *tmp_entry;
	char newsite[65]="";
	char *nsite=newsite;
	gchar *etext=NULL;
	int i;
	tmp_widget =gtk_object_get_data( GTK_OBJECT(widget), "mywindow" );
	tmp_entry = gtk_object_get_data( GTK_OBJECT(widget), "myentry" );
	if (tmp_entry) {
		etext=g_strdup(gtk_entry_get_text(GTK_ENTRY(tmp_entry)));
		for (i=0; i<strlen(etext); i++) {
			if (etext[i] != ' ') {newsite[i]=etext[i]; }
														}
						}

	if (tmp_widget)  {gtk_widget_destroy( tmp_widget );}
	if (strlen(nsite)>0) {
		snprintf( callbackbuf, 300, "http://photos.yahoo.com/bc/%s", nsite);
		display_url( (void *) strdup(callbackbuf) );
							}
	if (etext) {g_free(etext);}
}

void on_photoalbum  (GtkMenuItem *menuitem, gpointer  user_data) {
GtkWidget *okbutton;

okbutton=show_input_dialog(_("User"), "OK" ,"Cancel");
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_photoalbum_cb), NULL);
if (selected_buddy)  {
	GtkWidget *tmp_entry;
	tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_entry), selected_buddy);
			           } else {
	if (user_selected)  {
		GtkWidget *tmp_entry;
		tmp_entry = gtk_object_get_data( GTK_OBJECT(okbutton), "myentry" );
		gtk_entry_set_text(GTK_ENTRY(tmp_entry), user_selected);
		}
								}
}


void on_blist_get_yab (GtkMenuItem *widget, gpointer user_data) {
	if ( ! selected_buddy ) { return ; }
	search_open_yab(selected_buddy);
}


void on_pms_get_yab (GtkMenuItem *widget, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "yab" )) != NULL ) {
		pm_sess = pm_lptr->data;
		search_open_yab(pm_sess->pm_user);
	}
	return;
}

void on_pms_yphoto_start (GtkMenuItem *widget, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;
	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "yphoto" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (plugins_yphotos_loaded()) {
			plugins_on_yphoto_session_start(pm_sess);
		}
	}
	return;
}

void on_pms_add_friend (GtkMenuItem *widget, gpointer user_data) {
	GList *pm_lptr;
	PM_SESSION *pm_sess;

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "fadd1" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (! find_friend(pm_sess->pm_user)) {
			if (user_data) {
				snprintf(callbackbuf, 128, "/temp-friend-add %s", pm_sess->pm_user);
			} else {
				snprintf(callbackbuf, 128, "/buddy %s", pm_sess->pm_user);
			}
			chat_command(callbackbuf);
		}
	return;
	}

	if (( pm_lptr = find_pm_session_from_widget( GTK_WIDGET(widget), "fadd2" )) != NULL ) {
		pm_sess = pm_lptr->data;
		if (! find_friend(pm_sess->pm_user)) {
			if (user_data) {
				snprintf(callbackbuf, 128, "/temp-friend-add %s", pm_sess->pm_user);
			} else {
				snprintf(callbackbuf, 128, "/buddy %s", pm_sess->pm_user);
			}
			chat_command(callbackbuf);
		}
	return;
	}
}



void on_check_gyache_version_cb(GtkMenuItem *widget, gpointer user_data) {
		int url_length=0;
		char *tmppage=NULL;
		char *gynews=NULL;
		char *gyversion=NULL;
		GtkWidget *tmp_widget=NULL;
		tmppage=malloc(8192);
		if (! tmppage) {return;}
		tmp_widget=gtk_object_get_data(GTK_OBJECT(chat_window),"gynotebook");
		snprintf(callbackbuf, 150, "%s%s", GYACH_URL, "GYACHE_VERSION");
		set_max_url_fetch_size(600); /* should be under 60-70 bytes */ 
		url_length=fetch_url( callbackbuf,  tmppage, NULL );
		if (url_length>1) {gyversion=strdup(tmppage);}
		else {gyversion=strdup("Latest version could not be retrieved");}

		set_max_url_fetch_size(5250);
		snprintf(callbackbuf, 150, "%s%s", GYACH_URL, "news.php");
		sprintf(tmppage,"%s","");
		url_length=fetch_url(callbackbuf,  tmppage, NULL );
		if (url_length>1) {gynews=strdup(tmppage);}
		else {gynews=strdup("Latest news could not be retrieved");}

		/* check for missing pages */ 
		if ( strstr(gynews, "not found on this server")  || 
			strstr(gynews, "<html>") || strstr(gynews, "<HTML>") ) {
				free(gynews);
				gynews=strdup("Latest news could not be retrieved");
		}
		/* check for missing pages */ 
		if ( strstr(gyversion, "not found on this server")  || 
			strstr(gyversion, "<html>") || strstr(gyversion, "<HTML>") ) {
				free(gyversion);
				gyversion=strdup("Latest version could not be retrieved");
		}

		snprintf(tmppage, 8190, "<br>You are currently using version: <b>%s</b><br>The most CURRENT version available is: <b>%s</b><br><a href=\"%sindex.php\">Download  Gyach-E</a><br>&nbsp; <br>&nbsp; <br><b><a href=\"%sindex.php\">Gyach-E</a> News Bulletins:</b><br>%s<br>", VERSION, gyversion, GYACH_URL, GYACH_URL, gynews );
		set_myyahoo_html_data(tmppage);
		free(tmppage);
		free(gynews);
		free(gyversion);
		if (tmp_widget) {
			gtk_notebook_set_current_page(GTK_NOTEBOOK(tmp_widget), 2);
			}
		gdk_window_raise(chat_window->window );
}






