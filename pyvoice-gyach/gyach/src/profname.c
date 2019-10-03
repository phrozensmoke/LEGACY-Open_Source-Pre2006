/*****************************************************************************
 * profname.c
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
 * Some code for managing Yahoo 'profile' names.
 *****************************************************************************/
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include "gyach.h"
#include "util.h"
#include "users.h"
#include "yahoochat.h"
#include "profname.h"

extern GtkWidget *get_pm_icon(int which_icon, char *err_str) ;
extern int ymsg_idact( YMSG_SESSION *session, char *yahid);
extern int ymsg_iddeact( YMSG_SESSION *session, char *yahid);
extern char *ymsg_field( char *key );
extern int ymsg_reject_buddy_cb( YMSG_SESSION *session, char *who );

extern GtkWidget *room_maker_window;
extern GtkWidget *room_window;
extern GtkWidget *favorites_window;
extern GtkWidget *conf_win;
extern GtkWidget *pm_window;

gchar profeditwin_text[352];
GList *yprofnames=NULL;
GList *yprofnames_combo=NULL;
int yprofnamecount=0;
char *default_profile_name=NULL;
char *primary_screen_name=NULL;
char *chat_profile_name=NULL;
char  *current_chat_profile_name=NULL;
char *current_pm_profile_name=NULL;
char *retrieve_profname=NULL;
char *logged_in_profile_name=NULL;  /* Name we logged on, ymsg_sess-> user */
char *profile_replyto_name=NULL;
GtkWidget *profeditwin=NULL;
extern int immed_rejoin;

char *current_reject_profile_name=NULL;

void reset_profile_name_config() {
	if (chat_profile_name) {free(chat_profile_name);}
	chat_profile_name=NULL;
	if (current_chat_profile_name) {free(current_chat_profile_name);}
	current_chat_profile_name=NULL;
	if (current_pm_profile_name) {free(current_pm_profile_name);}
	current_pm_profile_name=NULL;
	clear_profile_names();
	add_profile_name( ymsg_sess->user);
	set_primary_screen_name( ymsg_sess->user);
	set_default_profile_name( ymsg_sess->user);
	if (logged_in_profile_name) {free(logged_in_profile_name);}
	logged_in_profile_name=strdup(ymsg_sess->user);
	lower_str(logged_in_profile_name);
	reset_profile_replyto_name();
	if (current_reject_profile_name) {free(current_reject_profile_name); }
	current_reject_profile_name=NULL;
}

void set_primary_screen_name(char *somesn) {
	if (primary_screen_name) {free(primary_screen_name);}
	primary_screen_name=strdup(somesn);
	lower_str(primary_screen_name);
	add_profile_name(primary_screen_name);
}
char *get_primary_screen_name() {
	if (!primary_screen_name) {primary_screen_name=strdup(ymsg_sess->user);}
	return primary_screen_name;
}

void set_default_profile_name(char *somesn) {
	if (!find_profile_name(somesn)) {return;}
	if (default_profile_name) {free(default_profile_name);}
	default_profile_name=strdup(somesn);
	lower_str(default_profile_name);
}
char *get_default_profile_name() {
	if (!default_profile_name) {default_profile_name=strdup(ymsg_sess->user);}
	return default_profile_name;
}

void set_chat_profile_name(char *somesn) {  /* Who are we currently in the room as */
	if (!find_profile_name(somesn)) {return;}
	if (chat_profile_name) {free(chat_profile_name);}
	chat_profile_name=strdup(somesn);
	lower_str(chat_profile_name);
}
char *get_chat_profile_name() {
	if (!chat_profile_name)  {return get_default_profile_name();}
	return chat_profile_name;
}

void set_current_chat_profile_name(char *somesn) {
	  /* Who are we currently ENTERING the room as */
	if (!find_profile_name(somesn)) {return;}
	if (current_chat_profile_name) {free(current_chat_profile_name);}
	current_chat_profile_name=strdup(somesn);
	lower_str(current_chat_profile_name);
}
char *get_current_chat_profile_name() {
	if (!current_chat_profile_name) {return get_default_profile_name();}
	return current_chat_profile_name;
}

void set_current_pm_profile_name(char *somesn) {
	if (!find_profile_name(somesn)) {return;}
	if (current_pm_profile_name) {free(current_pm_profile_name);}
	current_pm_profile_name=strdup(somesn);
	lower_str(current_pm_profile_name);
}
char *get_current_pm_profile_name() {
	if (!current_pm_profile_name) {return get_default_profile_name();}
	return current_pm_profile_name;
}
void reset_current_pm_profile_name() {
	if (current_pm_profile_name) {free(current_pm_profile_name);}
	current_pm_profile_name=NULL;
}

void reset_profile_replyto_name() {
	if (profile_replyto_name) {free(profile_replyto_name);}
	profile_replyto_name=NULL;
}

int find_profile_name( char *profnm ) {
	GList *this_profnm;
	if (!yprofnames) {return 0;}
	if (!profnm) {return 0;}
	if (strlen(profnm)<1) {return 0;}
	this_profnm = yprofnames;
	while( this_profnm ) {
		if ( ! strcasecmp( this_profnm->data, profnm )) {
			return( 1 );
		}
		this_profnm = g_list_next( this_profnm );
	}
	return( 0 );
}

void clear_profile_names() {
	GList *this_profnm;

	if (yprofnames) {
	this_profnm = yprofnames;
	while( this_profnm ) {
			yprofnames = g_list_remove_link( yprofnames, this_profnm );
			this_profnm=gyach_g_list_free( this_profnm );
			this_profnm = g_list_next( this_profnm );
		}
	}

	if (yprofnames_combo) {
	this_profnm = yprofnames_combo;
	while( this_profnm ) {
			yprofnames = g_list_remove_link( yprofnames, this_profnm );
			this_profnm=gyach_g_list_free( this_profnm );
			this_profnm = g_list_next( this_profnm );
		}
	}
	yprofnames=NULL;
	yprofnames_combo=NULL;
	yprofnamecount=0;
}

void add_profile_name( char *profnm ) {
	if (!profnm) {return;}
	if (strlen(profnm)<1) {return;}
	if ( ! find_profile_name( profnm )) {
		char *adder=NULL;
		adder=strdup(profnm);
		lower_str(adder);
		yprofnames = g_list_append( yprofnames, adder);
		yprofnamecount++;
	}
}

int get_profile_name_count() {return yprofnamecount;}

void create_profile_name_combolist() {
	/* Copy into the list everything but the primary screenname */
	GList *this_profnm;
	if (!yprofnames) {return;}
	this_profnm = yprofnames;
	while( this_profnm ) {
	   if (strcasecmp( this_profnm->data, get_primary_screen_name() )) {
		yprofnames_combo=g_list_append( yprofnames_combo, strdup( this_profnm->data ));	
	  } 
	this_profnm = g_list_next( this_profnm );
	}
}


void activate_all_profile_names() {
	/* We need to do this on Y! Messenger 6 emulation, 
	activate all other profile names upon sign on */
	GList *this_profnm;
	create_profile_name_combolist();  /* create the sub-list without primary name */
	if (!yprofnames_combo) {return;}
	this_profnm = yprofnames_combo;
	while( this_profnm ) {
		 if (strcasecmp( this_profnm->data, get_primary_screen_name() ) && 
			strcasecmp(this_profnm->data, ymsg_sess->user) ) {
	  		ymsg_idact(ymsg_sess, this_profnm->data);
		}
		this_profnm = g_list_next( this_profnm );
	}
}


void deny_buddy_all_profile_names(char *who) {
	GList *this_profnm;
	if (! who) {return;}
	if (strlen(who)<1) {return;}
		/* dont reject ourselves */ 
	if (find_profile_name(who) || (! strcasecmp(ymsg_sess->user, who)) ) {return;}

	/* reject primary name first */

		if (current_reject_profile_name) {free(current_reject_profile_name); }		
		current_reject_profile_name=strdup(get_primary_screen_name());
	  	ymsg_reject_buddy_cb(ymsg_sess, who);
		if (current_reject_profile_name) {free(current_reject_profile_name); }
		current_reject_profile_name=NULL;

		usleep( 15000 );

	create_profile_name_combolist();  /* create the sub-list without primary name */
	if (!yprofnames_combo) {return;}
	this_profnm = yprofnames_combo;
	while( this_profnm ) {
		/* Iterate through our profile names and send a 
			deny-a-buddy msg from each one */ 
		if (current_reject_profile_name) {free(current_reject_profile_name); }
		current_reject_profile_name=strdup(this_profnm->data);
	  	ymsg_reject_buddy_cb(ymsg_sess, who);
		if (current_reject_profile_name) {free(current_reject_profile_name); }
		current_reject_profile_name=NULL;
		this_profnm = g_list_next( this_profnm );
		usleep( 15000 );
	}

}

GList *build_profile_names_list() {
  	if (!yprofnames_combo) {create_profile_name_combolist();}
  	if (!yprofnames_combo) {
		yprofnames_combo=g_list_append( yprofnames_combo, strdup(""));
		}
	if (!yprofnames) {	add_profile_name( ymsg_sess->user);	}
	return yprofnames;
}

GtkWidget *get_profile_name_component(int with_primary) {
  GtkWidget *idbox=NULL; 
  GtkWidget *tcombo, *tentry, *tlabel;

  if (with_primary) {idbox = gtk_hbox_new (FALSE, 0);}
  else {idbox = gtk_vbox_new (FALSE, 0);}
  tlabel=gtk_label_new(with_primary?_("My Yahoo! ID:"):_("Alternate IDs:"));
  gtk_box_pack_start (GTK_BOX (idbox), tlabel, FALSE, FALSE, 1);
  if (with_primary) {
	gtk_box_pack_start (GTK_BOX (idbox), gtk_label_new("  "), FALSE, FALSE, 1);
  }

  tcombo=gtk_combo_new();
  tentry=GTK_COMBO(tcombo)->entry;
  gtk_editable_set_editable(GTK_EDITABLE(tentry),FALSE );
  gtk_box_pack_start (GTK_BOX (idbox),tcombo, with_primary?TRUE:FALSE, with_primary?TRUE:FALSE, 2);
	
   build_profile_names_list();

  gtk_combo_set_popdown_strings( GTK_COMBO(tcombo), with_primary?yprofnames:yprofnames_combo);

  if (with_primary) {
  	gtk_box_pack_start (GTK_BOX (idbox), gtk_label_new(" "), TRUE, TRUE, 1);
	gtk_entry_set_text(GTK_ENTRY(tentry),get_default_profile_name());
  } else {gtk_entry_set_text(GTK_ENTRY(tentry),yprofnames_combo->data);}
  gtk_container_set_border_width(GTK_CONTAINER(idbox),  3);
  gtk_widget_set_sensitive(tcombo , get_profile_name_count()>1?TRUE:FALSE);
  gtk_object_set_data (GTK_OBJECT (idbox), PROFNAMESTR,tentry);
  gtk_object_set_data (GTK_OBJECT (idbox), "profcombo",tcombo);
  gtk_object_set_data (GTK_OBJECT (idbox), "label",tlabel);  /* needed for PM window */ 
  gtk_widget_show_all(idbox);
  return idbox;
}

char *retrieve_profname_str (GtkWidget *widgy) {
	if (!retrieve_profname) {free(retrieve_profname); retrieve_profname=NULL;} 
	if (!widgy) {
		retrieve_profname=strdup(get_default_profile_name());}
	else {retrieve_profname=strdup(gtk_entry_get_text(GTK_ENTRY(widgy)));}
        return retrieve_profname;
}

GtkEntry *retrieve_profname_entry (GtkWidget *widgy) {
	GtkWidget *tmpwid=NULL;
	tmpwid=gtk_object_get_data(GTK_OBJECT(widgy),PROFNAMESTR);
	return GTK_ENTRY(tmpwid);
}

void on_profnameedit_activate (GtkButton *button, gpointer user_data) {
	if (my_status == 12) {return;}
        ymsg_idact(ymsg_sess, retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry (GTK_WIDGET(button)))));
}

void on_profnameedit_deactivate (GtkButton *button, gpointer user_data) {
	if (my_status == 12) {return;}
        ymsg_iddeact(ymsg_sess, retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry (GTK_WIDGET(button)))));
}
void on_profnameedit_setdefault (GtkButton *button, gpointer user_data) {
	set_default_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry (GTK_WIDGET(button))))); 
	set_current_chat_profile_name( get_default_profile_name());
	gtk_widget_destroy(profeditwin);
	profeditwin=NULL;
}

void on_profnameedit_login (GtkButton *button, gpointer user_data) {
	snprintf(profeditwin_text, 350, "http://edit.profiles.yahoo.com/config/edit_identity?.src=pg&.done=http://messenger.yahoo.com/&.l=%s&.redir_from=MESSENGER&.redir_from=REGISTRATION", ymsg_sess->user);
	display_url( (void *) strdup(profeditwin_text) );
}

gboolean on_profeditwin_window_delete_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
if (profeditwin) {gtk_widget_destroy(profeditwin);}
profeditwin=NULL;
return FALSE;
}

void create_profilename_window() {	
	GtkWidget *vbox_main, *label1, *label2, *row_box, *label13;
        GtkWidget *okbutton, *activbutt, *deactivbutt, *editbutt;
	GtkWidget *tcombo1, *tcombo2, *tentry1, *tentry2;

  profeditwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (profeditwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (profeditwin), _("Edit My Profiles"));
  gtk_window_set_position (GTK_WINDOW (profeditwin), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (profeditwin), TRUE);
  vbox_main = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 4);
  gtk_container_add (GTK_CONTAINER (profeditwin), vbox_main);
 
  gtk_box_pack_start (GTK_BOX (vbox_main), get_pm_icon(6, ""), FALSE, FALSE,3);
  snprintf(profeditwin_text, 350, "%s '%s'. %s '%s'. %s", _("Your primary Yahoo ID is"),  get_primary_screen_name(), _("You are logged in as"), ymsg_sess->user,_("You can disable any of your alternate IDs by selecting the ID and clicking 'Deactivate ID'.  Disabling an ID will turn it offline."));
  label1=gtk_label_new(profeditwin_text);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);	
  gtk_label_set_line_wrap(GTK_LABEL(label1),1);
  gtk_box_pack_start (GTK_BOX (vbox_main), label1, FALSE, FALSE, 3);

  label13=gtk_label_new(_("You cannot activate or deactivate IDs while you are invisible."));
  gtk_label_set_justify (GTK_LABEL (label13), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label13), 0, 0.5);	
  gtk_label_set_line_wrap(GTK_LABEL(label13),1);
  gtk_box_pack_start (GTK_BOX (vbox_main), label13, FALSE, FALSE, 2);

  tcombo1=get_profile_name_component(0);
  tentry1=gtk_object_get_data(GTK_OBJECT(tcombo1),PROFNAMESTR);
  tcombo2=get_profile_name_component(1);
  tentry2=gtk_object_get_data(GTK_OBJECT(tcombo2),PROFNAMESTR);

  gtk_box_pack_start (GTK_BOX (vbox_main), tcombo1, FALSE, FALSE, 2);
  row_box= gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox_main), row_box, FALSE, FALSE, 2);
  activbutt = get_pixmapped_button(_("Activate ID"), GTK_STOCK_YES);
  set_tooltip(activbutt,_("Activate ID"));
  gtk_box_pack_start (GTK_BOX (row_box), activbutt, TRUE, TRUE, 2);
  deactivbutt = get_pixmapped_button(_("Deactivate ID"), GTK_STOCK_NO);
  set_tooltip(deactivbutt,_("Deactivate ID"));
  gtk_box_pack_start (GTK_BOX (row_box), deactivbutt, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (vbox_main), gtk_label_new(" "), FALSE, FALSE, 2);
  editbutt = get_pixmapped_button(_("Create or Edit Profiles"), GTK_STOCK_FIND_AND_REPLACE);
  set_tooltip(editbutt,_("Create/Edit Profiles"));
  gtk_box_pack_start (GTK_BOX (vbox_main), editbutt, FALSE, FALSE, 2);
	
  if (get_profile_name_count()<2) {
	gtk_widget_set_sensitive(activbutt, 0);
	gtk_widget_set_sensitive(deactivbutt, 0);
  }
  gtk_box_pack_start (GTK_BOX (vbox_main), gtk_hseparator_new(), TRUE, TRUE, 4);

  snprintf(profeditwin_text, 350, "%s %s: %d", _("Default ID to be used for messages, chats, conferences, files, and invitations."), _("Names available"), get_profile_name_count());
  label2=gtk_label_new(profeditwin_text);
  gtk_label_set_justify (GTK_LABEL (label2), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label2), 0, 0.5);	
  gtk_label_set_line_wrap(GTK_LABEL(label2),1);
  gtk_box_pack_start (GTK_BOX (vbox_main), label2, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox_main), tcombo2, FALSE, FALSE, 2);
  okbutton = get_pixmapped_button(_("OK"), GTK_STOCK_APPLY);
set_tooltip(okbutton,_("OK"));
  gtk_box_pack_start (GTK_BOX (vbox_main), okbutton, FALSE, FALSE, 2);
  gtk_object_set_data(GTK_OBJECT(okbutton),PROFNAMESTR, tentry2);
  gtk_object_set_data(GTK_OBJECT(activbutt), PROFNAMESTR, tentry1);
  gtk_object_set_data(GTK_OBJECT(deactivbutt), PROFNAMESTR, tentry1);
  gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_profnameedit_setdefault),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (activbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_profnameedit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (deactivbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_profnameedit_deactivate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (editbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_profnameedit_login),
                      NULL);
  gtk_signal_connect_after (GTK_OBJECT (profeditwin), "delete_event",
                            GTK_SIGNAL_FUNC (on_profeditwin_window_delete_event),
                            NULL);
  gtk_widget_show_all(profeditwin);

}

void create_profilename_window_cb(GtkMenuItem *menuitem, gpointer  user_data) {
 create_profilename_window();
}

void update_existing_profile_name_widgets() {
	PM_SESSION *pm_sess;
	GtkWidget *tmp_widget=NULL;
	GList *this_session = pm_list;

	while( this_session ) {
		pm_sess = (PM_SESSION *)this_session->data;
		tmp_widget=gtk_object_get_data( GTK_OBJECT(pm_sess->pm_window), "profcombo");
		if (tmp_widget) {
			gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), 
			build_profile_names_list());
			gtk_widget_set_sensitive(tmp_widget , get_profile_name_count()>1?TRUE:FALSE);
			gtk_entry_set_text( 
			GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user);
		}
		this_session = g_list_next( this_session );
	}

	if (room_maker_window) {
		tmp_widget=gtk_object_get_data( GTK_OBJECT(room_maker_window), "profcombo");
		if (tmp_widget) {
			gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), 
			build_profile_names_list());
			gtk_widget_set_sensitive(tmp_widget , get_profile_name_count()>1?TRUE:FALSE);
			gtk_entry_set_text( 
			GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user);
		}
	}
	if (room_window) {
		tmp_widget=gtk_object_get_data( GTK_OBJECT(room_window), "profcombo");
		if (tmp_widget) {
			gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), 
			build_profile_names_list());
			gtk_widget_set_sensitive(tmp_widget , get_profile_name_count()>1?TRUE:FALSE);
			gtk_entry_set_text( 
			GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user);
		}
	}
	if (favorites_window) {
		tmp_widget=gtk_object_get_data( GTK_OBJECT(favorites_window), "profcombo");
		if (tmp_widget) {
			gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), 
			build_profile_names_list());
			gtk_widget_set_sensitive(tmp_widget , get_profile_name_count()>1?TRUE:FALSE);
			gtk_entry_set_text( 
			GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user);
		}
	}
	if (conf_win) {
		tmp_widget=gtk_object_get_data( GTK_OBJECT(conf_win), "profcombo");
		if (tmp_widget) {
			gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), 
			build_profile_names_list());
			gtk_widget_set_sensitive(tmp_widget , get_profile_name_count()>1?TRUE:FALSE);
			gtk_entry_set_text( 
			GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user);
		}
	}
	if (profeditwin) {gtk_widget_destroy(profeditwin);}
	profeditwin=NULL;
	if (pm_window) {gtk_widget_destroy(pm_window);}
	pm_window=NULL;
}


void parse_profilenames(char *somefield) {
	char smfield[480];
	char nwprofnm[64];
	char *smfptr=NULL;	
	char *starter=NULL;
	snprintf(smfield,478,"%s",somefield);
	if (strlen(smfield)<3) {return;}
	starter=smfield;
	smfptr=strchr(smfield,',');
	if (!smfptr) {add_profile_name(smfield); return;} /* one name */
	while (smfptr) {
		if (smfptr) {
			*smfptr='\0';
			snprintf(nwprofnm,62,"%s",starter);
			add_profile_name(nwprofnm);
			   /* printf("added: %s\n", nwprofnm);
				fflush(stdout); */ 
			smfptr++; starter=smfptr;
		}
		if (! strchr(smfptr,',')) { /* last one */
			snprintf(nwprofnm,62,"%s",starter);
			add_profile_name(nwprofnm);
			  /* printf("added: %s\n", nwprofnm);
				fflush(stdout); */ 
			break;
		} else {smfptr=strchr(smfptr,',');}
	}

}

char *select_profile_name(int packet_type, int inpm) {
	if (packet_type==YMSG_REJECTBUDDY) {
		if (current_reject_profile_name) {return current_reject_profile_name;}
		else {return ymsg_sess->user;}		
	}

	if (profile_replyto_name) {	return profile_replyto_name;}
		/* which profile name to send for which outgoing packet */

	switch (packet_type) {
	case YMSG_CONFINVITE:
	case YMSG_CONFLOGON:
	case YMSG_CONFDECLINE:
	case YMSG_CONFADDINVITE:
	case YMSG_JOIN:
	case YMSG_GOTO:
	case YMSG_CREATE_ROOM:
		return get_current_chat_profile_name();
		break;
	case YMSG_ONLINE:
		if (immed_rejoin) {return get_chat_profile_name();}
		else {return get_current_chat_profile_name();}
		break;
	case YMSG_COMMENT:
	case YMSG_LOGOUT:
	case YMSG_PING:
	case YMSG_EXIT:
	case YMSG_CONFLOGOFF:
	case YMSG_CONFMSG:
	case YMSG_INVITE:
		return get_chat_profile_name();
		break;
	case YMSG_FILETRANSFER:
	case YMSG_WEBCAM:
		return get_default_profile_name();
		break;
	case YMSG_PM:
	case YMSG_VOICECHAT:
	case YMSG_P2PASK:
	case YMSG_GAME_INVITE:
	case YMSG_AUDIBLE:
		return get_current_pm_profile_name();
		break;
	case YMSG_P2P:
	case YMSG_NOTIFY:
		if (inpm) {return get_current_pm_profile_name();}
		else {return get_default_profile_name();}
		break;

	default:
		return logged_in_profile_name;
		break;
	} /*end switch */
	
	return logged_in_profile_name;
}


void collect_profile_replyto_name() {
		/* Which one of our profile names was the last packet directed to?  */
		/* This helps us send responses from the right profile name, hopefully
		to avoid giving away other profile names if we don't want to */
	char *incptr=NULL;
	char incprof[64];
	snprintf(incprof, 62, "%s", "");

	switch (ymsg_sess->pkt.type) {
		case YMSG_PM:
		case YMSG_PM_RECV:
		case YMSG_GAMEMSG:
		case YMSG_VOICECHAT:
		case YMSG_NOTIFY:
		case YMSG_P2P:
		case YMSG_P2PASK:
		case YMSG_FILETRANSFER:
		case YMSG_SYSMSG:
		case YMSG_GAME_INVITE:
		case YMSG_AUDIBLE:
		case YMSG_BIMAGE:
		case YMSG_BIMAGE_TOGGLE:
		case YMSG_BIMAGE_UPDATED:
			if (strcmp(ymsg_field( "5" ),"")) {snprintf(incprof, 62, "%s", ymsg_field( "5" ));}
			break;
		case YMSG_CONFINVITE:
		case YMSG_CONFADDINVITE:
		case YMSG_CONFDECLINE:
		case YMSG_NEW_CONTACT:
			if (strcmp(ymsg_field( "1" ),"")) {snprintf(incprof, 62, "%s", ymsg_field( "1" ));}
			break;
		case YMSG_INVITE:
			if (strcmp(ymsg_field( "118" ),"")) {snprintf(incprof, 62, "%s", ymsg_field( "118" ));}
			break;
		default:
			break;
	}
	incptr=strchr(incprof, ',');
	if (incptr) {*incptr='\0';}
	if (profile_replyto_name) {free(profile_replyto_name);}
	if ( (strlen(incprof)>1) && (find_profile_name(incprof))) {profile_replyto_name=strdup(incprof);}
	else {profile_replyto_name=strdup(default_profile_name);}
}

char *get_profile_replyto_name() {
	if (! profile_replyto_name) {collect_profile_replyto_name();}
	return profile_replyto_name;
}




