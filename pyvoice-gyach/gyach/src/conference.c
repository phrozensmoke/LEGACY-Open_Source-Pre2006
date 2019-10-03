/*****************************************************************************
 * conference.c
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


#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif


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
#include "gyach.h"
#include "sounds.h"
#include "bootprevent.h"
#include "packet_handler.h"
#include "conference.h"
#include "profname.h"
#include "gytreeview.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)


  GtkTreeView *invite_list;
  GtkTreeView *friends_list;
  GtkTreeModel *conffreezer;
  GtkWidget *invite_button;
  GtkWidget *msg_entry;
  GtkWidget *add_entry;
  GtkWidget *host_entry;
  GtkWidget *add_button;
  GtkWidget *remove_button;
  GtkWidget *addn_button;
  GtkWidget *conf_cancel_button;
  GtkWidget *conf_win=NULL;
  char *selected_invite_friend=NULL;
  char *selected_invite=NULL;

  int use_chat_settings=0;
  int use_multpm_settings=0;
  int conference_packet_type=0;

  static char conference_member_data[2048]; /* changed */ 
  static  gchar confibuff[1280];

  char *conference_member_list=NULL;

 GHashTable *conference_members=NULL;
 GHashTable *conference_invites=NULL;

extern GtkWidget *chat_window;
extern GList *temporary_friend_list;
extern void append_timestamp(GtkWidget *somewidget, GtkWidget *somewidget2);
extern void jump_to_chat_tab();
extern void append_char_pixmap_text(const char **pixy, GtkWidget *textw);
extern void append_to_open_pms(char *who, char *mytext, int with_timestamp);
extern void set_voice_conference_cookie(char *cook);
extern void comm_block_notify(char *who, char *who_alias, char *msg1, char *msg2, int  preemptive_strike, int sound_event);

char *conference_invite_room=NULL;
char *conference_invite_list=NULL;

char *conference_invite_host=NULL;
char *conference_host=NULL;
char *conference_room=NULL;
char *conference_who=NULL;
char *conference_msg=NULL;
char *conference_members_str=NULL;
char *conference_error=NULL;
char *last_conference=NULL;
char *conference_invite_voice=NULL;
int i_started_conference=0;
char *last_decline=NULL;  /* used to monitor attempts to 'boot' using conf declines */
char *last_decline_m=NULL;  /* used to monitor attempts to 'boot' using conf declines */
char *conference_me=NULL;
int is_conference=0;
char *conf_sep1=NULL;
char *conf_sep2=NULL;
char *conference_header=NULL;
void start_conf_voice_chat();


GtkWidget* create_make_conference (void)
{
  GtkWidget *make_conference;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label1=NULL;
  GtkWidget *label3=NULL;
  GtkWidget *table1;
  GtkWidget *label8;
  GtkWidget *label10;
  GtkWidget *vbox3;
  GtkWidget *scrolledwindow1;
  GtkWidget *label9;
  GtkWidget *vbox2;
  GtkWidget *scrolledwindow2;
  GtkWidget *hbox4;
  GtkWidget *hbox2;
  GtkWidget *label4;
  GtkWidget *hbox3;
  char *col_headers[]={"",NULL};

 if (! in_a_chat) {use_chat_settings=0;}

  make_conference = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (make_conference), _("Create a Conference"));
 if (use_chat_settings) {
  gtk_window_set_title (GTK_WINDOW (make_conference), _("Invite to Chat Room"));
				 }

 if (use_multpm_settings) {
  gtk_window_set_title (GTK_WINDOW (make_conference), _(" Instant Message "));
				 }

  gtk_window_set_wmclass (GTK_WINDOW (make_conference), "gyachEConference", "GyachE");
  gtk_window_set_position (GTK_WINDOW (make_conference), GTK_WIN_POS_CENTER);
  /* gtk_window_set_modal (GTK_WINDOW (make_conference), TRUE); */ 

  vbox1 = gtk_vbox_new (FALSE, 3);
  gtk_container_add (GTK_CONTAINER (make_conference), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 6);

  hbox1 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

  if (use_chat_settings) {
	label1 = gtk_label_new (_("Room:"));
  	gtk_widget_show (label1);
  	gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);
  	gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  	host_entry = gtk_entry_new ();
	gtk_entry_set_max_length(GTK_ENTRY(host_entry),65);
  	gtk_widget_show (host_entry);
  	gtk_box_pack_start (GTK_BOX (hbox1), host_entry, FALSE, FALSE, 2);
  	gtk_editable_set_editable(GTK_EDITABLE (host_entry), FALSE);
				  } else {
  label1 =get_profile_name_component(1);
  host_entry=GTK_WIDGET(retrieve_profname_entry(label1));
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 2);
  if (is_conference) {gtk_widget_set_sensitive(label1,0); gtk_widget_set_sensitive(host_entry,0);}
  gtk_object_set_data(GTK_OBJECT(make_conference),"profcombo" ,gtk_object_get_data(GTK_OBJECT(label1), "profcombo") );
  gtk_widget_show_all(label1);
							}


  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new (""), TRUE, TRUE, 0);

	if (use_chat_settings) {
  label3 = gtk_label_new (_("Select friends from the list on the left and click \"Add\" to invite them to the\nchat room. To add somebody who is not on the list, type his or her Yahoo!\nID in the box and click \"Add\"."));
	} else 	if (use_multpm_settings) {
  label3 = gtk_label_new (_("You can send short instant messages to up to 8 users at once.\nSelect friends from the list on the left and click \"Add\" to add them to the\nlist of recipients. To add somebody who is not on the list, type his or her Yahoo!\nID in the box and click \"Add\"."));
		} else {
  label3 = gtk_label_new (_("Select friends from the list on the left and click \"Add\" to invite them to your\nconference. To add somebody who is not on the list, type his or her Yahoo!\nID in the box and click \"Add\"."));
			  }
  gtk_box_pack_start (GTK_BOX (vbox1), label3, FALSE, FALSE, 5);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label3), 0, 0.5);

  table1 = gtk_table_new (2, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox1), table1, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 4);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 4);

  label8 = gtk_label_new (_("Invitation List"));
  if (use_multpm_settings) {gtk_label_set_text(GTK_LABEL(label8), _("Recipients"));}
  gtk_table_attach (GTK_TABLE (table1), label8, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label8), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label8), 0, 0.5);

  label10 = gtk_label_new (_("List of Friends Online"));
  gtk_table_attach (GTK_TABLE (table1), label10, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label10), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label10), 0, 0.5);

  vbox3 = gtk_vbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (table1), vbox3, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3), 4);

  gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new (""), FALSE, FALSE, 0);

  add_button =get_pixmapped_button(_("Add >>"), GTK_STOCK_ADD);
  set_tooltip(add_button,_("Add"));
  gtk_box_pack_start (GTK_BOX (vbox3), add_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new (""), TRUE, TRUE, 0);

  remove_button=get_pixmapped_button(_("<< Remove"), GTK_STOCK_REMOVE);
    set_tooltip(remove_button,_("Delete"));
  gtk_box_pack_start (GTK_BOX (vbox3), remove_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new (""), FALSE, FALSE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_table_attach (GTK_TABLE (table1), scrolledwindow1, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_widget_set_usize (scrolledwindow1, -2, 200);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_ETCHED_IN);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  friends_list = create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 0, 
	1, col_headers);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET(friends_list));

  label9 = gtk_label_new ("");
  gtk_table_attach (GTK_TABLE (table1), label9, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label9), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label9), 0, 0.5);

  vbox2 = gtk_vbox_new (FALSE, 4);
  gtk_table_attach (GTK_TABLE (table1), vbox2, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (vbox2), scrolledwindow2, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolledwindow2, 200, -2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_ETCHED_IN);

  invite_list = create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 0, 
	1, col_headers);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), GTK_WIDGET(invite_list));

  hbox4 = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox2), hbox4, FALSE, FALSE, 0);

  add_entry = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(add_entry),62);
    set_tooltip(add_entry,_("User"));
  gtk_box_pack_start (GTK_BOX (hbox4), add_entry, TRUE, TRUE, 0);

  addn_button =get_pixmapped_button(_("Add"), GTK_STOCK_ADD);
    set_tooltip(addn_button,_("Add"));
  gtk_box_pack_start (GTK_BOX (hbox4), addn_button, FALSE, FALSE, 0);

  hbox2 = gtk_hbox_new (FALSE, 7);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);

  label4 = gtk_label_new (_("Message:"));
  gtk_box_pack_start (GTK_BOX (hbox2), label4, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

  msg_entry = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(msg_entry),118);
  if (use_multpm_settings) {gtk_entry_set_max_length(GTK_ENTRY(msg_entry),400);}
  gtk_box_pack_start (GTK_BOX (hbox2), msg_entry, TRUE, TRUE, 0);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox3, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox3), 4);

  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new (""), TRUE, TRUE, 0);

	if (use_multpm_settings) {
 		 invite_button =get_pixmapped_button(_("Send"), GTK_STOCK_APPLY);
  		set_tooltip(invite_button,_("Send"));
	} else  {
 		 invite_button =get_pixmapped_button(_("Invite"), GTK_STOCK_APPLY);
  		set_tooltip(invite_button,_("Invite"));
	}
  gtk_box_pack_start (GTK_BOX (hbox3), invite_button, FALSE, FALSE, 0);
	 if (! use_chat_settings) { gtk_object_set_data(GTK_OBJECT(invite_button),PROFNAMESTR,host_entry );
	}

  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new (""), TRUE, TRUE, 0);

  conf_cancel_button = get_pixmapped_button(_("Cancel"), GTK_STOCK_CANCEL);
    set_tooltip(conf_cancel_button,_("Cancel"));
  gtk_box_pack_start (GTK_BOX (hbox3), conf_cancel_button, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new (""), TRUE, TRUE, 0);


  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (make_conference, make_conference, "make_conference");
  GLADE_HOOKUP_OBJECT (make_conference, label1, "label1");
  GLADE_HOOKUP_OBJECT (make_conference, host_entry, "host_entry");
  GLADE_HOOKUP_OBJECT (make_conference, add_button, "add_button");
  GLADE_HOOKUP_OBJECT (make_conference, remove_button, "remove_button");
  GLADE_HOOKUP_OBJECT (make_conference, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (make_conference, friends_list, "friends_list");
  GLADE_HOOKUP_OBJECT (make_conference, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT (make_conference, invite_list, "invite_list");
  GLADE_HOOKUP_OBJECT (make_conference, add_entry, "add_entry");
  GLADE_HOOKUP_OBJECT (make_conference, addn_button, "addn_button");
  GLADE_HOOKUP_OBJECT (make_conference, msg_entry, "msg_entry");
  GLADE_HOOKUP_OBJECT (make_conference, invite_button, "invite_button");
  GLADE_HOOKUP_OBJECT (make_conference, conf_cancel_button, "conf_cancel_button");

  return make_conference;
}


void remove_conference_member(char *user)  {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! conference_members ) { return; }
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	if ( g_hash_table_lookup_extended( conference_members, tmp_user, &key_ptr,
			&value_ptr )) {
		chatter_list_remove( tmp_user );			
		g_hash_table_remove( conference_members, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );  /* seg-fault watch ?  */
		if ( value_ptr )
			g_free( value_ptr );   /* seg-fault watch ?  */
									}
															  }

int add_conference_member( char *user ) {
	char tmp_user[64];
	char tmp_alias[64];
	char *ptr = NULL;

	if ( ! conference_members ) {
		conference_members = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! conference_members ) {return( 0 );}

	strncpy( tmp_user, user , 62);
	lower_str( tmp_user );
	strncpy( tmp_alias, user , 62);

	if (( ptr = g_hash_table_lookup( conference_members, tmp_user )) != NULL ) {
		g_hash_table_insert( conference_members, g_strdup(tmp_user), g_strdup(tmp_alias));
		g_free( ptr );
		chatter_list_add( tmp_user );
	} else {
		g_hash_table_insert(conference_members, g_strdup(tmp_user), g_strdup(tmp_alias));
		chatter_list_add( tmp_user );
	}

	/* g_warning("New alias set %s  %s \n",user,useralias); */
	return( 1 );
}


int clear_conference_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
		/* printf("free-confhash\n"); fflush(stdout); */
	g_free(key);
	g_free(value);
 	return 1;

}

void clear_conference_hash() {
if (!conference_members) {return;}
g_hash_table_foreach_remove(conference_members ,clear_conference_hash_cb,NULL);
}



void remove_conference_invite(char *user)  {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! conference_invites ) { return; }
	strncpy( tmp_user, user , 62);
	lower_str( tmp_user );

	if ( g_hash_table_lookup_extended( conference_invites, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( conference_invites, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );  /* seg-fault watch ?  */
		if ( value_ptr )
			g_free( value_ptr );   /* seg-fault watch ?  */
									}
															  }

int add_conference_invite( char *user ) {
	char tmp_user[64];
	char tmp_alias[64];
	char *ptr = NULL;

	if ( ! conference_invites ) {
		conference_invites = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! conference_invites ) {return( 0 );}

	if (use_multpm_settings) {
		if (g_hash_table_size(conference_invites )>7) {return( 0 );}
	}

	strncpy( tmp_user, user , 62);
	lower_str( tmp_user );
	strncpy( tmp_alias, user , 62);

	if (( ptr = g_hash_table_lookup( conference_invites, tmp_user )) != NULL ) {
		g_hash_table_insert( conference_invites, g_strdup(tmp_user), g_strdup(tmp_alias));
		g_free( ptr );
	} else {
		g_hash_table_insert(conference_invites, g_strdup(tmp_user), g_strdup(tmp_alias));
	}

	/* g_warning("New alias set %s  %s \n",user,useralias); */
	return( 1 );
}

int clear_conference_invite_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
		/* printf("free-confinvhash\n"); fflush(stdout); */ 
	g_free(key);
	g_free(value);
 	return 1;

}

void clear_conference_invite_hash() {
if (!conference_invites) {return;}
g_hash_table_foreach_remove(conference_invites ,clear_conference_invite_hash_cb,NULL);
}


void append_buddy_to_conf_clist(gpointer key, gpointer value,
        gpointer user_data) {
	GtkTreeIter iter;
   	char user[64];
	char *rd;	
	GdkPixbuf *imbuf=NULL;
	int is_online=1;

	if (!key) {return; }
	if (!value) {return;}
	snprintf(user,62, "%s",(char *)key);
	rd=strdup(key);

	if (find_online_friend(user))  { /* online friends up top */
		gtk_list_store_append(GTK_LIST_STORE(conffreezer), &iter);
										}  else {
		gtk_list_store_prepend(GTK_LIST_STORE(conffreezer), &iter);
		is_online=0;
												  }

	if (find_temporary_friend(user)) {
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_pm_buzz);
	} else {
		if (is_online) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_online);
						}   else {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_offline);
									}
	}

	gtk_list_store_set(GTK_LIST_STORE(conffreezer), &iter, 
			GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, rd,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "", -1);
	if (imbuf) {g_object_unref(imbuf);}
	free(rd);
}


void update_buddy_conf_clist()  {
	if (buddy_status || temporary_friend_list) {
			conffreezer=freeze_treeview(GTK_WIDGET(friends_list));
			gy_empty_model(conffreezer, GYTV_TYPE_LIST);

		if (buddy_status) {
			g_hash_table_foreach(buddy_status, append_buddy_to_conf_clist, NULL );
		}
		if (temporary_friend_list) {
			GList *this_friend;
			this_friend = temporary_friend_list;
			while( this_friend ) {	
				append_buddy_to_conf_clist(this_friend->data, "",NULL);
				this_friend = g_list_next( this_friend );
				}
			}
	unfreeze_treeview(GTK_WIDGET(friends_list), conffreezer);
	}
}


void append_invite_to_conf_clist(gpointer key, gpointer value,
        gpointer user_data) {
	GtkTreeIter iter;
   	char user[64];
	char *rd;	
	GdkPixbuf *imbuf=NULL;

	if (!key) {return; }
	if (!value) {return;}

	snprintf(user,62, "%s",(char *)key);
	rd=strdup(key);
	gtk_list_store_append(GTK_LIST_STORE(conffreezer), &iter);
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_online);
	gtk_list_store_set(GTK_LIST_STORE(conffreezer), &iter, 
			GYSINGLE_PIX_TOGGLE, TRUE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, rd,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "", -1);
	g_object_unref(imbuf);
	free(rd);
}




void update_invite_conf_clist()  {
		if (conference_invites) {
			conffreezer=freeze_treeview(GTK_WIDGET(invite_list));
			gy_empty_model(conffreezer, GYTV_TYPE_LIST);
			g_hash_table_foreach(conference_invites, append_invite_to_conf_clist, NULL );
			unfreeze_treeview(GTK_WIDGET(invite_list), conffreezer);
		}
}


void collect_buddy_selected()  {
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		GtkTreeIter iter;
		if (!friends_list) {
			if ( selected_invite_friend ) {
				free( selected_invite_friend );  
				selected_invite_friend=NULL;
			}
		return;
		}
		selection=gtk_tree_view_get_selection(friends_list);
		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			gchar *sfound;
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &sfound, -1);		
			if ( selected_invite_friend ) {	free( selected_invite_friend);  }
			selected_invite_friend=strdup(sfound);
			g_free(sfound);
		} else {
			if ( selected_invite_friend ) {
				free( selected_invite_friend );  
				selected_invite_friend=NULL;
			}
		}
}


void collect_invite_selected()  {
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		GtkTreeIter iter;
		if (!invite_list) {
			if ( selected_invite ) {
				free( selected_invite);  
				selected_invite=NULL;
			}
		return;
		}
		selection=gtk_tree_view_get_selection(invite_list);
		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			gchar *sfound;
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &sfound, -1);		
			if ( selected_invite ) {	free( selected_invite);  }
			selected_invite=strdup(sfound);
			g_free(sfound);
		} else {
			if ( selected_invite) {
				free(selected_invite);  
				selected_invite=NULL;
			}
		}
}


void remove_invite_from_list ()
{
	collect_invite_selected();
	if ( selected_invite  ) {
		remove_conference_invite(selected_invite);
		free(selected_invite); selected_invite=NULL;
		update_invite_conf_clist();
							  }
}

void  remove_invite_from_list_cb       (GtkButton     *bbutton,
                                        gpointer         user_data)
{
	remove_invite_from_list ();
}

void onConfInviteRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		remove_invite_from_list ();
}


void add_invite_from_list ()
{
	collect_buddy_selected();
	if ( selected_invite_friend  ) {

	if ( (! strcasecmp( selected_invite_friend, ymsg_sess->user )) || 
		find_profile_name(selected_invite_friend) ) {
			free(selected_invite_friend); 
			selected_invite_friend=NULL; 
			return;
	} 

		add_conference_invite(selected_invite_friend);
		free(selected_invite_friend); selected_invite_friend=NULL;
		update_invite_conf_clist();
							  }
}

void  add_invite_from_list_cb       (GtkButton     *bbutton,
                                        gpointer         user_data)
{
	add_invite_from_list ();
}

void onConfBuddyRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		add_invite_from_list ();
}


void add_invite_from_text (GtkButton     *bbutton,
                                        gpointer         user_data)
{
	char *randinvite;
	randinvite=strdup(gtk_entry_get_text(GTK_ENTRY(add_entry)));

	if ( (! strcasecmp( randinvite, ymsg_sess->user )) || 
		find_profile_name(randinvite) ) {
	free(randinvite); randinvite=NULL; return;
	} 

	if ( strlen(randinvite)>0 ) {
		add_conference_invite(randinvite);
		free(randinvite); randinvite=NULL;
		update_invite_conf_clist();
		gtk_entry_set_text(GTK_ENTRY(add_entry),"");
							  } else {free(randinvite);}
}



void destroy_conf_resources() {
	if (conf_win) {gtk_widget_destroy(GTK_WIDGET(conf_win));}
	conf_win=NULL;
	if (selected_invite_friend) { free(selected_invite_friend); selected_invite_friend=NULL; }
	if (selected_invite) { free(selected_invite_friend); selected_invite=NULL; }
}


gboolean
on_conf_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
destroy_conf_resources();
return FALSE;
}

void close_conf_window (GtkButton     *bbutton,
                                        gpointer         user_data)
{
destroy_conf_resources();
}



void send_conf_addinvite(gpointer key, gpointer value,
        gpointer user_data) { 
		char *tname;
		tname=(char *)key;
		ymsg_conference_addinvite(ymsg_sess,tname,conference_header);
				     }

void send_chat_invites(gpointer key, gpointer value,
        gpointer user_data) { 
		char *tname;
		tname=(char *)key;
		ymsg_invite_with_mesg( ymsg_sess, tname, ymsg_sess->room, conference_header );
				     }

void send_multi_pms(gpointer key, gpointer value,
        gpointer user_data) { 
		char *tname;
		tname=(char *)key;
		ymsg_pm( ymsg_sess, tname, conference_header );
				     }


void init_conferences(GtkButton     *bbutton, gpointer         user_data) {
	if (conference_header) {free(conference_header); conference_header=NULL;}
	conference_header=strdup(gtk_entry_get_text(GTK_ENTRY(msg_entry)));


if (use_multpm_settings) {
		if (strlen(conference_header)<1) {return;}
		if (conference_invites) {
		set_current_pm_profile_name(retrieve_profname_str(host_entry));
		destroy_conf_resources();
		g_hash_table_foreach(conference_invites, send_multi_pms, NULL );
		clear_conference_invite_hash();
		show_ok_dialog(_("Messages sent."));
		if (conference_header) {free(conference_header); conference_header=NULL;}
					 					 } else {
		destroy_conf_resources();
		if (conference_header) {free(conference_header); conference_header=NULL;}
												  }
	use_multpm_settings=0;
	return; 
				}


if (use_chat_settings) {
	if (in_a_chat) {
		if (conference_invites) {
		destroy_conf_resources();
		g_hash_table_foreach(conference_invites, send_chat_invites, NULL );
		clear_conference_invite_hash();
		show_ok_dialog(_("Invitation sent."));
		if (conference_header) {free(conference_header); conference_header=NULL;}
					 					 } else {
		destroy_conf_resources();
		if (conference_header) {free(conference_header); conference_header=NULL;}
												  }
					} else {
		destroy_conf_resources();
		if (conference_header) {free(conference_header); conference_header=NULL;}
		 					  }
	use_chat_settings=0;
	return; 
				}

if (is_conference) {
	if (conference_invites) {	
	set_current_chat_profile_name(retrieve_profname_str(host_entry));
	destroy_conf_resources();
	g_hash_table_foreach(conference_invites, send_conf_addinvite, NULL );
	clear_conference_invite_hash();
	show_ok_dialog(_("Invitation sent."));
	if (conference_header) {free(conference_header); conference_header=NULL;}
					  }
	
				  }  else  {
	char *mee=strdup(get_current_chat_profile_name());
	ymsg_leave_chat( ymsg_sess );
	chatter_list_populate(mee,1);
	add_conference_member(mee);
	free(mee);
	set_current_chat_profile_name(retrieve_profname_str(host_entry));
	ymsg_conference_invite(ymsg_sess,get_current_chat_profile_name(),conference_header);
	is_conference=1;
	in_a_chat=0;
	if (last_conference) {free(last_conference);}
	last_conference=strdup(ymsg_sess->room);
	destroy_conf_resources();

	snprintf( confibuff, 20, "%s", "\n     ");
	append_to_textbox_color( chat_window, NULL, confibuff );
	append_char_pixmap_text((const char**)pixmap_pm_join, NULL);	
	snprintf( confibuff,511, "\n     %s%s%s %s'%s' %s%s\n\n",
		YAHOO_STYLE_BOLDON, YAHOO_COLOR_PURPLE,
		_("You are now in the conference named"), 
		YAHOO_COLOR_RED, ymsg_sess->room,
		YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );  
	append_to_textbox_color( chat_window, NULL, confibuff );
	jump_to_chat_tab();

		gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
		snprintf( confibuff,255, "%s: %s", _("Conference Room"), ymsg_sess->room);
		gtk_statusbar_push( GTK_STATUSBAR(chat_status),
		st_cid, confibuff );
	/* start_conf_voice_chat(); */ 
	set_chat_profile_name(get_current_chat_profile_name());
	show_ok_dialog(_("Invitation sent."));
	if (conference_header) {free(conference_header); conference_header=NULL;}
					     }

}



void open_make_conference_window() {
	char *roomer=NULL;
	if (conf_win) {return;}
	conf_win=create_make_conference();
	if (conference_invites) {clear_conference_invite_hash(); }

	if ( ! conference_invites ) {
		conference_invites = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! conference_invites ) {
		return;
	}


	if ( ! conference_members ) {
		conference_members = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! conference_members ) {
		return;
	}

	update_buddy_conf_clist();
	
	if (use_chat_settings) {roomer=strdup(ymsg_sess->room);}
	else if (use_multpm_settings) {roomer=strdup(get_default_profile_name());}
	else {
		if (is_conference) {roomer=strdup(get_chat_profile_name());}
		else {roomer=strdup(get_default_profile_name());}
		}
	gtk_entry_set_text(GTK_ENTRY(host_entry), roomer);


	if (!use_chat_settings) { 
		if (!use_multpm_settings) {
			gtk_entry_set_text(GTK_ENTRY(msg_entry),"You are invited to join my conference.");
												}
					} else {
		snprintf(confibuff,108, "%s: '%s'", "You are invited to join the room", ymsg_sess->room);
		gtk_entry_set_text(GTK_ENTRY(msg_entry),confibuff);
						  }

  gtk_signal_connect (GTK_OBJECT (add_button), "clicked",
                      GTK_SIGNAL_FUNC (add_invite_from_list_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (addn_button), "clicked",
                      GTK_SIGNAL_FUNC (add_invite_from_text),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (remove_button), "clicked",
                      GTK_SIGNAL_FUNC (remove_invite_from_list_cb),
                      NULL);
  g_signal_connect (invite_list, "row-activated", (GCallback) onConfInviteRowActivated, NULL);
  g_signal_connect (friends_list, "row-activated", (GCallback) onConfBuddyRowActivated, NULL);

  gtk_signal_connect_after (GTK_OBJECT (conf_win), "delete_event",
                            GTK_SIGNAL_FUNC (on_conf_window_delete_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (conf_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (close_conf_window),
                      NULL);
  gtk_signal_connect (GTK_OBJECT ( invite_button), "clicked",
                      GTK_SIGNAL_FUNC (init_conferences),
                      NULL);
	set_basic_treeview_sorting(GTK_WIDGET(invite_list),GYLIST_TYPE_SINGLE);
	set_basic_treeview_sorting(GTK_WIDGET(friends_list),GYLIST_TYPE_SINGLE);
	gtk_widget_show_all(conf_win);
	if (roomer) {free(roomer);}
}


void open_make_conference_window_with_name(char *myname) {
	open_make_conference_window();
	gtk_entry_set_text(GTK_ENTRY(add_entry),strdup(myname));
	add_invite_from_text(GTK_BUTTON(addn_button),"");
}



void append_conf_member_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {

	if (strlen(conference_member_data)>1875) {return;} /* avoid buffer overflow */

	if (conf_sep2 != NULL) {  /* for ymsg formatting */
		strncat(conference_member_data,conf_sep2, 15); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 15); }
					  }
	strncat(conference_member_data,(char *)key, 68);
	if (conf_sep1 != NULL) {strncat(conference_member_data,conf_sep1, 15); }
}


char  *get_conference_member_list(char *separator1, char *separator2) {
sprintf(conference_member_data,"%s","");
	if (conf_sep1) {free(conf_sep1);}
	if (conf_sep2) {free(conf_sep2);}
	conf_sep1=NULL;
	conf_sep2=NULL;
	if (separator1 != NULL) {conf_sep1=strdup(separator1); }
	if (separator2 != NULL) {conf_sep2=strdup(separator2);  }
g_hash_table_foreach(conference_members ,append_conf_member_hash_cb, NULL );
if (conference_member_list) {free(conference_member_list);}
if (strlen(conference_member_data)<2) {
	if (conf_sep2==NULL) {
		snprintf(conference_member_data,100, "%s", get_current_chat_profile_name());
					 } else {
		/* for ymsg formatting */
		strncat(conference_member_data,conf_sep2, 25); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 25); }
		strncat(conference_member_data,get_current_chat_profile_name(), 100); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 25); }
						  }
							  }
conference_member_list=strdup(conference_member_data);
return conference_member_list;
}


void append_conf_invite_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {

	if (strlen(conference_member_data)>1875) {return;} /* avoid buffer overflow */
	if (conf_sep2 != NULL) {  /* for ymsg formatting */
		strncat(conference_member_data,conf_sep2, 15); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 15); }
					  }
	strncat(conference_member_data,(char *)key, 68);
		/* TEST */  //add_conference_member(strdup((char *)key));
	if (conf_sep1 != NULL) {strncat(conference_member_data,conf_sep1, 15); }
}


char  *get_conference_invite_list(char *separator1, char *separator2) {
sprintf(conference_member_data,"%s","");
	if (conf_sep1) {free(conf_sep1);}
	if (conf_sep2) {free(conf_sep2);}
	conf_sep1=NULL;
	conf_sep2=NULL;
	if (separator1 != NULL) {conf_sep1=strdup(separator1); }
	if (separator2 != NULL) {conf_sep2=strdup(separator2);  }
g_hash_table_foreach(conference_invites ,append_conf_invite_hash_cb, NULL );
if (conference_member_list) {free(conference_member_list);}
if (strlen(conference_member_data)<2) {
	if (conf_sep2==NULL) {
		snprintf(conference_member_data,100, "%s", get_current_chat_profile_name());
					 } else {
		/* for ymsg formatting */
		strncat(conference_member_data,conf_sep2, 25); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 25); }
		strncat(conference_member_data,get_current_chat_profile_name(), 100); 
		if (conf_sep1 != NULL) { strncat(conference_member_data,conf_sep1, 25); }
						  }
							  }
conference_member_list=strdup(conference_member_data);
//printf("members-invite: %s\n",conference_member_list); fflush(stdout);
return conference_member_list;
}


char *get_conference_host()  {
	if (conference_host) {free(conference_host); conference_host=NULL;}
	if ( strcmp( ymsg_field( "50" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "50" ));
		conference_host=strdup(conference_member_data);
		return conference_host;
															 }

	/* if (conference_packet_type==YMSG_CONFADDINVITE)  {
		if ( strcmp( ymsg_field( "51" ), "" ))  { 
			snprintf(conference_member_data, 96, "%s", ymsg_field( "51" ));
			conference_host=strdup(conference_member_data);
			return conference_host;
															 	}
	} */

	return NULL;
}


char *get_conference_who()  {
	if (conference_who) {free(conference_who); conference_who=NULL;}

	if ( strcmp( ymsg_field( "53" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "53" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }
	if ( strcmp( ymsg_field( "54" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "54" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }
	if ( strcmp( ymsg_field( "55" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "55" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }
	if ( strcmp( ymsg_field( "56" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "56" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }
	if ( strcmp( ymsg_field( "3" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "3" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }
	if ( strcmp( ymsg_field( "1" ), "" ))  { 
		snprintf(conference_member_data, 96, "%s", ymsg_field( "1" ));
		conference_who=strdup(conference_member_data);
		return conference_who;
															 }

	return NULL;
}


char *get_conference_msg()  {
	if (conference_msg) {free(conference_msg); conference_msg=NULL;}
	if ( strcmp( ymsg_field( "16" ), "" ))  { 
		snprintf(conference_member_data, limit_lfs?575:650, "%s", ymsg_field( "16" ));
		conference_msg=strdup(conference_member_data);
		return conference_msg;
															 }
	if ( strcmp( ymsg_field( "58" ), "" ))  { 
		snprintf(conference_member_data, limit_lfs?575:650, "%s", ymsg_field( "58" ));
		conference_msg=strdup(conference_member_data);
		return conference_msg;
															 }
	if ( strcmp( ymsg_field( "14" ), "" ))  { 
		snprintf(conference_member_data, limit_lfs?575:650, "%s", ymsg_field( "14" ));
		conference_msg=strdup(conference_member_data);
		return conference_msg;
															 }
	return NULL;
}

char *get_conference_room()  {
	if (conference_room) {free(conference_room); conference_room=NULL;}
	if ( strcmp( ymsg_field( "57" ), "" ))  { 
		snprintf(conference_member_data, 125, "%s", ymsg_field( "57" ));
		conference_room=strdup(conference_member_data);
		return conference_room;
															 }
	if ( strcmp( ymsg_field( "234" ), "" ))  { 
		snprintf(conference_member_data, 125, "%s", ymsg_field( "234" ));
		conference_room=strdup(conference_member_data);
		return conference_room;
															 }
	return NULL;
}


char *get_conference_error()  {
	if (conference_error) {free(conference_error); conference_error=NULL;}
	if ( strcmp( ymsg_field( "16" ), "" ))  { 
		snprintf(conference_member_data, 350, "%s", ymsg_field( "16" ));
		conference_error=strdup(conference_member_data);
		return conference_error;
															 }
	return NULL;
}



char *get_conference_members()  {
	if (conference_members_str) {free(conference_members_str); conference_members_str=NULL;}
	if ( strcmp( ymsg_field( "53" ), "" ))  { 
		conference_members_str=strdup(ymsg_field( "53" ));
		return conference_members_str;
															 }
	if ( strcmp( ymsg_field( "52" ), "" ))  { 
		conference_members_str=strdup(ymsg_field( "52" ));
		return conference_members_str;
															 }
	if ( strcmp( ymsg_field( "50" ), "" ))  { 
		conference_members_str=strdup(ymsg_field( "50" ));
		return conference_members_str;
															 }
	return NULL;
}





void populate_conference_list(char *plist) {
	int i=0;
	char *end;
	char *ptr;
	int last;
	char buf[80]="";

					ptr = plist;
					last = 0;
					while( ptr ) 	{
						if (i>147) {break;}
						end = strchr( ptr, ',' );
						if ( end ) {
							*end = '\0';
						} else {
							last = 1;
								  }
						strncpy(buf,ptr,78);
						add_conference_member(buf );
						
						i++;
						if ( last ) {
							break;
						} else {
							ptr = end + 1;
								  }
										}

}


void start_conf_voice_chat() {
	if (!ymsg_sess->room) {return;}

	sprintf(confibuff,"%s", "");
	strcat(confibuff,"\n");
	strcat(confibuff,YAHOO_STYLE_BOLDON);
	strcat(confibuff,YAHOO_COLOR_PURPLE);
	strncat(confibuff,_("Voice Chat"), 40);
	strcat(confibuff,":  ");
	strcat(confibuff,YAHOO_COLOR_GREEN);
	strncat(confibuff, _("Enabled"), 25);
	strcat(confibuff, "  ");
	append_char_pixmap_text((const char**)pixmap_pm_voice, NULL);
	strcat(confibuff,YAHOO_COLOR_BLACK);
	strcat(confibuff,YAHOO_STYLE_BOLDON);
	strcat(confibuff,"\n\n");
	append_to_textbox_color( chat_window, NULL, confibuff );
	voice_enabled=1;
	voice_launch_conference(0, get_chat_profile_name(), ymsg_sess->room);

}


int is_valid_conference() {  /* Is this packet for the conference we're in? */
	char *theirrom=NULL;		
	if (!is_conference) {return 0;}
	if (!last_conference) {return 0;}
	theirrom=get_conference_room();
	if (!theirrom) {return 0;}
	if (strcasecmp(last_conference,theirrom)) {return 0;} 
return 1;
}




void conference_message()  {
	int ccount;
	char *who=NULL;
	char *msg=NULL;
	char *yalias=NULL;

	who=get_conference_who();
	msg=get_conference_msg();
	if (!who) {return;}
	if (!msg) {return;}


				if (!is_conference)  {
					/* This is a boot attempt, receiving conference messages when we are not 
						in a conference */
					if ( strcasecmp( who, get_chat_profile_name())) { 
						log_possible_flood_attack(who, 9,"conference messages when not in conference.");
						return;
																				}
											}

				if ( is_flooder( who )) {return;}

				if (find_user_row(who)<0) {
					add_conference_member(who);
								    					}			

				set_last_comment( who, "Regular Conference Message" );

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nCONFERENCE ROOM COMMENT:\nRoom: %s\nUser: %s\nComment: %s\n\n", ctime(&time_llnow), ymsg_sess->room, who, msg);
				fflush( capture_fp );
			}

				ccount=set_last_comment( who, msg );
				if ( ignore_check( who ) || mute_check( who )) {
					return;
				}

				if (( suppress_mult ) &&
					( ccount > 2 )) {
						if ( (!never_ignore_a_friend) || 
						( (!find_friend(who)) && (!find_temporary_friend(who)) )   )  {	return; }
				}

				if ( strcasecmp( who, get_chat_profile_name() )) { /* other users' comments */

				if ( chat_timestamp ) {
					append_timestamp(chat_window, NULL);
													}

					/* setting total byte limit to 650, to avoid buffer 
						overflow and dangerous 'flooding'
						from people sending large paragraphs - 
						this is more than enough space for 
						standard comments, sometimes flooders 
						have tried to send up to 800 bytes, so we'll limit the likelihood
						that they can lag our screens or boot/crash us. 		
		 			*/

					yalias=get_screenname_alias(who);

					snprintf( confibuff, 175, "%s%s%s%s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_friend(who) || find_temporary_friend(who) ) )
							? "\033[#8F4CB1m" : "\033[#8A4D9Bm",
						yalias, YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );  

	if ( limit_lfs ) {  /* control line feeds if requested */
		char *lf;
		int lf_count = 0;
		/* Remove tabs and return chars */
		lf = strchr( msg, '\r' );
		while( lf ) {*lf = ' ';  lf = strchr( lf + 1, '\r' );}
		lf = strchr( msg, '\t' );
		while( lf ) {*lf = ' ';  lf = strchr( lf + 1, '\t' );}

		lf = strchr( msg, '\n' );
		while( lf ) {
			lf_count++;
			lf = strchr( lf + 1, '\n' );
		}
	if ( lf_count > limit_lfs ) {
		lf = strchr( msg, '\n' );
		while( lf ) {
			*lf = ' ';
			lf = strchr( lf + 1, '\n' );
		}
	}
				}

					strcat( confibuff, ":  " );
					/* below, size of this is controlled in get_conference_msg */
					strcat(confibuff, _utf(msg));
					strcat(confibuff,"  \n");  /* leave the space */

				append_to_textbox_color( chat_window, NULL, confibuff );
				g_free(yalias); yalias=NULL;													
																					}

}


void i_left_the_conference() {
	int stillinconf=0;
	if (last_conference && is_conference) {stillinconf=1;}
	char *inconf=get_conference_room();
	if (! stillinconf) {
   		if (!in_a_chat) {clear_chat_list_members(); strncpy( ymsg_sess->room, "[NONE]", 62);}
		is_conference=0;
	} 
	/* show user that we aren't in chat  anymore */
	snprintf( confibuff, 256, "%s%s  [** %s '%s' **] %s%s\n\n",
		"\033[#2E8AB2m", YAHOO_STYLE_BOLDON,
		_("You have left the conference."), inconf?inconf:"??",
		YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK );
	append_to_textbox_color( chat_window, NULL, confibuff );

	if (!stillinconf) {
		gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
		gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,
		_("Not currently connected to conference.") );	
	}
}



void conference_user_leave()  {
	char *who=NULL;
	char *yalias=NULL;

	who=get_conference_who();

	if (!who) {return;}

				if ((!is_conference) || (!is_valid_conference()))  			{
					/* This is a boot attempt, user 'has left' message when we are not 
						in a conference */
					if ( strcasecmp( who, get_chat_profile_name() )) { 
						log_possible_flood_attack(who, 3,"'user has left the conference.' messages.");
						return;
																				}
																							}


				if ( strcasecmp( who, get_chat_profile_name() )) { /* other user left */

				remove_conference_member(who);	
				if ( is_flooder( who )) {return;}

				set_last_comment( who, "Left Conference" );

				if ( ignore_check( who ) || mute_check(who) ) {
					return;
				}

				if ( enter_leave_timestamp && chat_timestamp) {
					append_timestamp(chat_window, NULL);
																					}

					yalias=get_screenname_alias(who);

				if (!strcmp(who,yalias)) {
					snprintf( confibuff, 200, "     ** %s%s%s%s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_friend(who) || find_temporary_friend(who) ) )
							? "\033[#B23E95m" : "\033[#33A073m",
						who, YAHOO_COLOR_BLACK,  YAHOO_STYLE_BOLDOFF );  
												 } else {
					snprintf( confibuff, 200,  "     ** %s%s%s  ( %s ) %s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_friend(who) || find_temporary_friend(who) ) )
							? "\033[#B23E95m" : "\033[#33A073m",yalias, 
						who, YAHOO_COLOR_BLACK,  YAHOO_STYLE_BOLDOFF );  
														  }

					strcat( confibuff, "  " );
					strncat(confibuff, _("has left the conference."), 125);
					strcat(confibuff," **\n");

				g_free(yalias); yalias=NULL;					
				append_to_textbox_color( chat_window, NULL, confibuff );			
				append_to_open_pms(who, confibuff, chat_timestamp_pm);

		}  else {  /* we left */
			i_left_the_conference();
			  }

}




void conference_user_enter()  {
	char *who=NULL;
	char *yalias=NULL;

	who=get_conference_who();

	if (!who) {return;}

				if ((!is_conference) || (!is_valid_conference()))  			{
					/* This is a boot attempt, user 'has entered' message when we are not 
						in a conference */
					if ( strcasecmp( who, get_chat_profile_name() )) { 
						log_possible_flood_attack(who, 3,"'user has joined the conference' messages.");
						return;
																				}
																							}


				if ( strcasecmp( who, get_chat_profile_name() )) { /* other user enters */
				if ( is_flooder( who )) {return;}

				add_conference_member(who);	
				set_last_comment( who, "Entered Conference" );

				if ( ignore_check( who ) || mute_check( who )) {
					return;
				}

				if ( enter_leave_timestamp && chat_timestamp) {
					append_timestamp(chat_window, NULL);
																					}

					yalias=get_screenname_alias(who);

				if (!strcmp(who,yalias)) {
					snprintf( confibuff, 200, "     ** %s%s%s%s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_friend(who) || find_temporary_friend(who) ) )
							? "\033[#5791B1m" :"\033[#B59239m" ,
						who, YAHOO_COLOR_BLACK,  YAHOO_STYLE_BOLDOFF );  
													} else  {
					snprintf( confibuff,200,  "     ** %s%s%s  ( %s ) %s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && (find_friend(who) || find_temporary_friend(who) ) )
							? "\033[#5791B1m" :"\033[#B59239m" , yalias, 
						who, YAHOO_COLOR_BLACK,  YAHOO_STYLE_BOLDOFF );  
															  }


					strcat( confibuff, "  " );
					strncat(confibuff, _("has joined the conference."), 125);
					strcat(confibuff," **\n");


				append_to_textbox_color( chat_window, NULL, confibuff );	
				append_to_open_pms(who, confibuff, chat_timestamp_pm);
				g_free(yalias); yalias=NULL;		
			
		
				if (i_started_conference)  {
					char *room=NULL;
					if (last_conference) {free(last_conference); last_conference=NULL;}
					ymsg_conference_join(ymsg_sess);
					set_chat_profile_name(get_current_chat_profile_name());
					i_started_conference=0;
					room=get_conference_room();
					if (room) {
						is_conference=1;
						in_a_chat=0;
						if (last_conference) {free(last_conference);}
						last_conference=strdup(room);
									}

														  }																					

															}  else  {  /* we just joined */

	char *members=NULL;
	char *room=NULL;
	room=get_conference_room();
	members=get_conference_members();
	if (!members) {members=get_conference_who();}

		if (room) {
					is_conference=1;
					in_a_chat=0;
					if (last_conference) {free(last_conference);}
					last_conference=strdup(room);
					set_chat_profile_name(get_current_chat_profile_name());
					snprintf( confibuff, 20, "%s", "\n     ");
					append_to_textbox_color( chat_window, NULL, confibuff );
					append_char_pixmap_text((const char**)pixmap_pm_join, NULL);	
					snprintf( confibuff, 500, "\n     %s%s%s %s'%s'%s%s\n\n",
					YAHOO_STYLE_BOLDON, YAHOO_COLOR_PURPLE,
					_("You are now in the conference named"), room,
							YAHOO_COLOR_RED, 
							  YAHOO_COLOR_BLACK, YAHOO_STYLE_BOLDOFF );  
				append_to_textbox_color( chat_window, NULL, confibuff );
				jump_to_chat_tab();

						gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
						snprintf( confibuff,255, "%s: %s", _("Conference Room"), room);
						gtk_statusbar_push( GTK_STATUSBAR(chat_status),
							st_cid, confibuff );

			if (members)  {
					snprintf( confibuff, 150, "     %s%s%s:%s%s   ",
					YAHOO_STYLE_BOLDON, YAHOO_COLOR_BLUE,
					_("Members in this conference"), YAHOO_COLOR_BLACK, 								YAHOO_STYLE_BOLDOFF );  
					if (conference_invite_host)  	{
						strncat(confibuff, "[", 3);
						strncat(confibuff, conference_invite_host, 60);
						strncat(confibuff, "]  ", 5);
						}
					strncat(confibuff, members, 750);
					strcat(confibuff,"\n\n");
				append_to_textbox_color( chat_window, NULL, confibuff );				

				chatter_list_populate(strdup(members),0);				
				populate_conference_list(members);
				add_conference_member(who);
				if (conference_invite_host)  	{
					add_conference_member(conference_invite_host);
					free(conference_invite_host); conference_invite_host=NULL;
										}
									}
				start_conf_voice_chat();

						}

																								 }


}


void conference_user_decline()  {
	char *who=NULL;
	char *msg=NULL;

	who=get_conference_who();
	msg=get_conference_msg();

	if (!who) {return;}

				if ( is_flooder( who )) {return;}

				if ( ignore_check( who ) ) {
						if ( (!never_ignore_a_friend) || 
						( (!find_friend(who)) && (!find_temporary_friend(who)) )   )  {	
					send_automated_response(who, who, "this user is not accepting conference decline messages from you, and it is very likely that this user never invited you to any conferences to begin with.");
					return;
						}
				}

	/* here, we have a little attempt to avoid 'conference boots', where 
	   people send you 'no thanks' messages to conferences you never 
	   invited them too...they send enough of these, Yahoo boots you...but
	   first GYach will freeze - we want to avoid showing 'decline' messages
	   from people sending repeated decline messages, and people sending
	   messages that look like we are decling ourselves */

	if ( ! strcasecmp( who, get_chat_profile_name() )) {
		boot_attempt("Someone Pretending To Be You", "conference declines");
		return;  /* somebody pretending to be us */
															 }


	if (msg)  {
		/* If they are sending really long messages, they are trying to boot us. */
		if (strlen(msg)>175) {
			log_possible_flood_attack(who, 5,"large conference declines");
			return;					}
			 }

	if (last_decline_m && msg)  {
		if (! strncasecmp(msg,last_decline_m,18))  { 
			/* we already showed conf decline msg similar to this one, 
				they maybe be sending the same msg over and over 
			    trying to boot us.  */
			log_possible_flood_attack(who, 4,"conference declines");
			return ;
														  		} else  { 
			/* save the msg for future reference */
			free(last_decline_m);
			last_decline_m=strdup(msg);
																	}
										} else  {
							if (msg) {last_decline_m=strdup(msg);}
													}



	if (last_decline)  {
		if (! strncasecmp(who,last_decline,7))  { 
			/* we already showed conf decline for this person, 
				if they are sending repeated ones, they are probably
				trying to boot us  */
			log_possible_flood_attack(who, 4,"repeated conference declines");
			return ;
														  		} else  { 
			/* save the name for future reference */
			free(last_decline);
			last_decline=strdup(who);
																	}
						} else  {last_decline=strdup(who);}


				if ( strcasecmp( who, get_chat_profile_name() )) { /* other users' comments */

				if ( chat_timestamp ) {
					append_timestamp(chat_window, NULL);
													}

					snprintf( confibuff, 200,  "     ** %s%s%s%s%s",
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && find_friend(who))
							? YAHOO_COLOR_RED : YAHOO_COLOR_BLUE,
						who, YAHOO_COLOR_BLACK, 								  YAHOO_STYLE_BOLDOFF );  

					strcat( confibuff, "  " );
					strncat(confibuff, _("has declined to join the conference."), 100);
					if (msg)  {
						strcat(confibuff,"\n");
						strncat(confibuff,_("Message"), 25);
						strcat(confibuff,":  '");
						strncat(confibuff, _utf(msg) ,85);  /* boot, buffer overflow protection */
						strcat(confibuff,"' ");
									}
					strcat(confibuff," **\n");

				if (!is_flooder(who)) {
					append_to_textbox_color( chat_window, NULL, confibuff );
					append_to_open_pms(who, confibuff, chat_timestamp_pm);
					play_sound_event(SOUND_EVENT_REJECT);
											 }
											
																					}


}

void accept_invite()  {
	char *host=NULL;
	host=conference_invite_host;

				if (!conference_invite_room) {return;}
				if (!conference_invite_list) {return;}
				i_started_conference=0;
				if (conference_me) {free(conference_me);}
				conference_me=strdup(get_current_chat_profile_name());
				strncpy(ymsg_sess->room, conference_invite_room, 62);
				clear_conference_hash();
				populate_conference_list(conference_invite_list);
				add_conference_member(conference_me);			
				if (host) {add_conference_member(host);  }			
				ymsg_conference_join(ymsg_sess);	
				if (last_conference) {free(last_conference);}
				last_conference=strdup(conference_invite_room);	
				is_conference=1;			
				set_chat_profile_name(conference_me);
				chatter_list_populate(strdup(conference_invite_list),1);					

				if (conference_invite_voice) {set_voice_conference_cookie(conference_invite_voice); }
}



void decline_invite(char *reason)  {
	char *blankmsg=NULL;
				if (!conference_invite_room) {return;}
				if (!conference_invite_list) {return;}
if (!reason) {
	ymsg_conference_decline( ymsg_sess, conference_invite_room, conference_invite_list, blankmsg);
		  return;
			}
if (strlen(reason)==0) {
	ymsg_conference_decline( ymsg_sess, conference_invite_room, conference_invite_list, blankmsg);
		  return;
							 }
	ymsg_conference_decline( ymsg_sess, conference_invite_room, conference_invite_list, reason);
}


void set_conference_invite(char *room, char *users, char *host, char *voice) {

	if (!room) {return;}
	if (!users) {return;}
	if (!host) {return;}
				if (conference_invite_room) {free(conference_invite_room);}
				conference_invite_room=strdup(room);
				if (conference_invite_list) {free(conference_invite_list);}
				conference_invite_list=strdup(users);
				if (conference_invite_host) {free(conference_invite_host);}
				conference_invite_host=strdup(host);

				if (conference_invite_voice) {free(conference_invite_voice);}
				conference_invite_voice=NULL;
				if (voice) {conference_invite_voice=strdup(voice); }
}




void conference_invite()  {
	char *blankmsg=NULL;
	char *host=NULL;
	char *msg=NULL;
	char *room=NULL;
	char *membss=NULL;
	char *voice=NULL;
	GtkWidget *okbutton;
	GtkWidget *cbutton;

	membss=get_conference_members();
	host=get_conference_host();
	msg=get_conference_msg();
	room=get_conference_room();

	if (!host) {return;}
	if (!room) {return;}
	if (!membss) {return;}

		if (is_flooder(host)) {return ;}

				if ( ignore_check( host ) ) {				
						if ( (!never_ignore_a_friend) || 
						( (!find_friend(host)) && (!find_temporary_friend(host)) )   )  {	
							collect_profile_replyto_name();
							ymsg_conference_decline( ymsg_sess, room,host, blankmsg);
							reset_profile_replyto_name();
							if (!find_friend(host)) { preemptive_strike(host); }						
						return;
						}
				}

	sprintf(conference_member_data,"%s", "");

	if (strlen(membss)==0)  {
		strncpy(conference_member_data,host, 100);
										 } else {
											strncpy(conference_member_data,membss, 355);
											strcat(conference_member_data,",");
											strncat(conference_member_data,host,40);
													}


				if ( strcasecmp( host, get_chat_profile_name())) { /* other users' comments */

				int accept_pm=0;


				/* for privacy, treat  invites like PMs
					whatever restrictions are applied to PMs will be applied
					to invitations ...from all, from friends, from people in the room 
				 */
				accept_pm=get_pm_perms(host);
				if (auto_reject_invitations) {accept_pm=0;}

				if (! accept_pm)  {
					comm_block_notify(host, host, 
						_("Ignored conference invitation from"),  
						"this user is not currently accepting  conference invitations from you. This user will not see the invitation you just sent." , 
						1, 1);
					collect_profile_replyto_name();
					ymsg_conference_decline( ymsg_sess, room,host, blankmsg);
					reset_profile_replyto_name();
					return ;
											 }


				/* Clear boot attempt log if truly safe to do so */
				if (accept_pm==2) {set_last_comment( host, "Conference Invite" );}			
				
				if ( strcmp( ymsg_field( "233" ), "" ))  { 
					voice=strdup(ymsg_field( "233" )); /* voice chat key */ 
				}

				set_conference_invite(room, conference_member_data, host, voice);
				if (voice) {free(voice);}

				if ( chat_timestamp ) {
					append_timestamp(chat_window, NULL);
													}

	  		snprintf(confibuff,6, "%s", "  ");
	  		append_to_textbox( chat_window, NULL, confibuff );
	  		append_char_pixmap_text((const char**)pixmap_pm_join, NULL);	

					snprintf( confibuff, 150, "     %s** %s%s%s%s%s",
						YAHOO_COLOR_PMPURPLE, 
						YAHOO_STYLE_BOLDON, 
						( highlight_friends && find_friend(host))
							? YAHOO_COLOR_RED : YAHOO_COLOR_BLUE,
						host, YAHOO_COLOR_PURPLE, 								  YAHOO_STYLE_BOLDOFF );  

					strcat( confibuff, "  " );
					strncat(confibuff, _("has invited you to the conference"), 105);
						strcat(confibuff," '");
						strncat(confibuff,_utf(room), 75);
						strcat(confibuff,"' ");
						strcat(confibuff,"**\n");
						strcat(confibuff,YAHOO_COLOR_PMBLUE);
						strncat(confibuff,_("Users"), 15);
						strcat(confibuff,":  ");
						strncat(confibuff,conference_member_data, 410);
						strcat(confibuff,"\n");
					if (msg)  {
						strcat(confibuff,"\n");
						strncat(confibuff,_("Message"), 25);
						strcat(confibuff,":  '");
						strncat(confibuff, _utf(msg), 85);
						strcat(confibuff,"' \n");
									}
					strcat(confibuff," \n");


				append_to_textbox_color( chat_window, NULL, confibuff );
				append_to_open_pms(host, confibuff, chat_timestamp_pm);										

				/* for testing, remove me */
				/* accept_invite(); */


				/* added to GETTEXT here */
				snprintf(confibuff, 375,  "%s:  '%s'  %s: %s", _("You have been invited to the conference"), _utf(room), _("by"), host);
				if (msg)  {
				strcat(confibuff,"\n\n");
				strncat(confibuff,_("Message"), 25);
				strcat(confibuff,":  ");
				strncat(confibuff, _utf(msg), 85);
								}

				okbutton=show_confirm_dialog_config(confibuff,"Join Conference","Decline Invitation",0);
				if (!okbutton) {return;}
  				gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_conf_accept_cb), NULL);
				gtk_object_set_data( GTK_OBJECT(okbutton), PROFNAMESTR, strdup(get_profile_replyto_name()) );

				cbutton=gtk_object_get_data( GTK_OBJECT(okbutton), "cancel" );
				if (cbutton) {
  					gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      		GTK_SIGNAL_FUNC (on_conf_decline_cb), NULL);
					gtk_object_set_data( GTK_OBJECT(okbutton), PROFNAMESTR, strdup(get_profile_replyto_name()) );
						 }
				play_sound_event(SOUND_EVENT_OTHER);
																					}


	else {
		/* invitations we send bounce back to us and have our 
		voice chat key in them */ 
		if (find_profile_name(host)) {
			if ( strcmp( ymsg_field( "233" ), "" ))  { 
				if (conference_invite_voice) {free(conference_invite_voice);}
				conference_invite_voice=strdup(ymsg_field( "233" )); 
				set_voice_conference_cookie(conference_invite_voice); 
				start_conf_voice_chat();
			}
		}
	}

}  /* end method */ 





void handle_conference_packet(char *pdata, int packet_type, int packet_size)  {
	char *who=NULL;
	char *errmsg=NULL;

	errmsg=get_conference_error();
	conference_packet_type=packet_type;

	if ( (packet_type==YMSG_CONFADDINVITE) || (packet_type==YMSG_CONFINVITE)) {
		who=get_conference_host();
	}

	if (!who) {who=get_conference_who();}


	 /* printf("CONF-PACKET:  [%d]   %s \n", packet_type, pdata);
		fflush(stdout);   */

	if (errmsg)  {
		if (strstr(errmsg,"confbpstayon")) {return;}
		if (packet_type != YMSG_CONFDECLINE) {
			snprintf(confibuff, 350, "     %s%s** %s:  %s **%s%s\n",
			YAHOO_STYLE_BOLDON, YAHOO_COLOR_RED, _("Conference Error"),
			errmsg, YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window, NULL, confibuff );
			return;
																	}
		return;
					}

	if (!who) {return;}
	
	switch( packet_type) {
		case YMSG_GYE_CONFEXIT:
			i_left_the_conference();
			break;

		case YMSG_CONFMSG:
			if ((!is_conference) || (!is_valid_conference())) {		
					push_packet_suppression ();					
					if (enable_basic_protection) {
							/* The worst boot packet of all that I've seen...
								a conference message BLAST when we aren't in a conference */
							/* We'll try to shut them down immediately and hope for the best */

							ymsg_typing( ymsg_sess, who, 1 );
							log_possible_flood_attack(who, 12, "Illegal Conference Message");
							if (sock_sleep<115000) {sock_sleep=115000;} 
							ymsg_conference_msg_echo( ymsg_sess);	/* this helps, SOMEtimes */
							// printf("basic boot protect\n"); fflush(stdout);
															}		
					return;
									   										}			
			conference_message();
			break;

		case YMSG_CONFLOGOFF:
			if (!is_conference) {
					push_packet_suppression ();
					if (is_boot_attempt(who, "large conference 'user has left' messages", packet_size)) {return;}
					else {log_possible_flood_attack(who, 3,"Illegal Conference Leave Messages");}
					return;
									   }			
			conference_user_leave();
			break;

		case YMSG_CONFLOGON:
			if (!is_conference) {
					push_packet_suppression ();
					if (is_boot_attempt(who, "large conference 'user has entered' messages", packet_size)) {return;}
					else {log_possible_flood_attack(who, 3,"Illegal Conference Enter Messages");}
					return;
									   }			
			conference_user_enter();
			break;

		case YMSG_CONFDECLINE:
			push_packet_suppression ();	
			if (is_boot_attempt(who, "large conference declines", packet_size)) {return;}
			else {log_possible_flood_attack(who, 4,"Conference Decline Messages");}
			if (enable_basic_protection) {
				emergency_keep_alive(who);
				if (sock_sleep<65000) {sock_sleep=is_flooder(who)?80000:65000;} 
															}	
			conference_user_decline();
			break;

		case YMSG_CONFADDINVITE:
		case YMSG_CONFINVITE:
			push_packet_suppression ();	
			if (is_boot_attempt(who, "large conference invites", packet_size)) {return;}
			else {log_possible_flood_attack(who, 5,"Conference Invite Messages");}
			if (enable_basic_protection) {
				emergency_keep_alive(who);
				if (sock_sleep<80000) {sock_sleep=is_flooder(who)?95000:80000;} 
															}	
			conference_invite();
			break;

	}

}

