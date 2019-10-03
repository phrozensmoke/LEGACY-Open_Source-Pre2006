/*****************************************************************************
 * ignore.c
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

#include <gtk/gtk.h>
#include <glib.h>

#include "gyach.h"
#include "ignore.h"
#include "friends.h"
#include "bootprevent.h"
#include "webconnect.h"
#include "ycht.h"
#include "images.h"
#include "yahoochat.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "profname.h"
#include "gytreeview.h"

GList *regex_list = NULL;
GHashTable *ignore_hash = NULL;
GHashTable *mute_hash = NULL;
GHashTable *regex_hash = NULL;
GHashTable *immunity_hash = NULL;

int  disp_auto_ignored = 1;
int  ignore_on_mults = 0;
int  mute_on_mults = 0;
int  ignore_guests = 0;
int  suppress_mult = 0;
int  first_post_is_url = 0;
int  first_post_is_pm = 0;
int  url_from_nonroom_user = 0;

int quiet_mutes=0;

char iggbuf[384];

/* The GUI in this module has multiple uses... */
/*  1=regular perm. igg
     2=temporary friend
     3=flooders/booters
     4=local gyach igg list
*/

int ignore_list_type=1;  

/* Added: PhrozenSmoke - whether to 'broadcast' to the room we are ignoring a person, 
     or do a 'quiet' ignore and tell no one.  Default is to NOT  'broadcast'  */
int BROADCAST_IGNORE=0;

extern gchar **choose_pixmap(char *user);
extern GList *temporary_friend_list ;
extern GList *flooder_list;
void free_igglist_row_data();

/* added: PhrozenSmoke */

  GtkWidget *ignorewindow;
  GtkTreeView *iggclist;
  GtkTreeModel *iggfreezer;
  GtkWidget *igg_delbutton;
  GtkWidget *igg_addentry;
  GtkWidget *igg_addbutton;
  GHashTable *perm_igg_hash;
  char *selected_permigg=NULL;

 
  gboolean on_select_perm_igg    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);
void on_add_perm_igg (GtkButton *button,gpointer  user_data);
void on_del_perm_igg_cb (GtkButton *button,gpointer  user_data);
void onIgglistRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);

int igglist_visible=0;

/* end added: PhrozenSmoke */

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data_full (GTK_OBJECT (component), name, \
    gtk_widget_ref (widget), (GtkDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)



static void show_hash(gpointer key, gpointer value,
        gpointer user_data) {
	char buf[72];

	snprintf( buf, 70, "\t%s\n", (char *)key );
	append_to_textbox( chat_window, NULL, buf );
}

void ignore_add( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return;	} 

	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	if ( ! ignore_hash ) {
		ignore_hash = g_hash_table_new( g_str_hash, g_str_equal );
	}
	if ( ! ignore_hash ) {return;}
	if ( ! g_hash_table_lookup( ignore_hash, tmp_user )) {
		g_hash_table_insert( ignore_hash, g_strdup( tmp_user ), g_strdup( "1" ));
	}
}

void ignore_remove( char *user ) {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! user ) {	return;	} 
	if ( ! ignore_hash ) {return;}

	strncpy( tmp_user, user , 62);
	lower_str( tmp_user );

	if ( g_hash_table_lookup_extended( ignore_hash, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( ignore_hash, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );
		if ( value_ptr )
			g_free( value_ptr );
	}
}

int ignore_check( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return 0;} 
	if ( ! ignore_hash ) {return( 0 );	}

	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	/* check for .guest user */
	if (( ignore_guests ) &&
		( strlen( tmp_user ) > 6 ) &&
		( ! strcmp( &tmp_user[strlen(tmp_user)-6], ".guest" ))) {
		return( 1 );
	}

	if ( g_hash_table_lookup( ignore_hash, tmp_user )) {
		return( 1 );
	}

	return( 0 );
}

void regex_add( char *regex ) {
	regex_t *ai_regex;

	if ( ! regex ) {
		// DBG( 11, "regex_add( NULL )\n" );
		return;
	} else {
		// DBG( 11, "regex_add( '%s' )\n", regex );
	}

	/* maintain the list of regex */
	if ( g_list_find_custom( regex_list, regex, (GCompareFunc)gstrcmp ))
		return;

	regex_list = g_list_prepend( regex_list, g_strdup( regex ));

	if ( ! regex_hash ) {
		regex_hash = g_hash_table_new( g_str_hash, g_str_equal );
	}

	/* maintain the hash table of compiled regex structures */
	if ( ! g_hash_table_lookup( regex_hash, regex )) {
		ai_regex = malloc( sizeof( regex_t ));
		regcomp( ai_regex, regex, REG_EXTENDED | REG_ICASE | REG_NOSUB );
		g_hash_table_insert( regex_hash, g_strdup( regex ), ai_regex );
	}
}

void regex_remove( char *regex ) {
	GList *this_regex;
	gpointer key_ptr;
	gpointer value_ptr;
	char *ptr;

	if ( ! regex ) {
		// DBG( 11, "regex_remove( NULL )\n" );
		return;
	} else {
		// DBG( 11, "regex_remove( '%s' )\n", regex );
	}

	/* maintain the list of regex */
	this_regex = g_list_find_custom( regex_list, regex, (GCompareFunc)gstrcmp );

	if ( this_regex ) {
		ptr = this_regex->data;
		regex_list = g_list_remove( regex_list, ptr );
		free( ptr );
	}

	/* maintain the hash table of compiled regex structures */
	if ( g_hash_table_lookup_extended( regex_hash, regex, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( regex_hash, regex );
		if ( key_ptr )
			g_free( key_ptr );
		if ( value_ptr ) {
			regfree( value_ptr );
			free( value_ptr );
		}
	}
}

int regex_check( char *regex ) {
	if ( ! regex ) {
		// DBG( 11, "regex_check( NULL )\n" );
		return( 0 );
	} else {
		// DBG( 11, "regex_check( '%s' )\n", regex );
	}

	/* quickest to check the hash table rather than the list */
	if ( g_hash_table_lookup( regex_hash, regex )) {
		return( 1 );
	}

	return( 0 );
}

char *auto_ignore_check( char *str ) {
	GList *this_regex;
	regex_t *ai_regex;
	

	if ( ! str ) {
		// DBG( 11, "auto_ignore_check( NULL )\n" );
		return( 0 );
	} else {
		// DBG( 11, "auto_ignore_check( '%s' )\n", str );
	}

	this_regex = regex_list;
	while( this_regex ) {
		ai_regex = g_hash_table_lookup( regex_hash, this_regex->data );
		if ( ! regexec( ai_regex, str, 0, NULL, 0 )) {
			return( this_regex->data );
		}

		this_regex = g_list_next( this_regex );
	}

	return( NULL );
}

void ignore_load() {
	char if_filename[192];
	char buf[129];
	FILE *fp;

	// DBG( 11, "ignore_load()\n" );

	snprintf( if_filename,  190,  "%s/ignore", GYACH_CFG_COMMON_DIR );

	fp = fopen( if_filename, "r" );

	if ( fp ) {
		/* load the new list */
		while( fgets( buf, 128, fp )) {
			if (strlen(buf)>2) {  
			/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
				/* chop the newline */
				buf[strlen(buf)-1] = '\0';
  		   		ignore_add( buf );
			}
		}

		fclose( fp );
	}

	snprintf( if_filename, 190, "%s/ignore.regex", GYACH_CFG_DIR );

	fp = fopen( if_filename, "r" );

	if ( fp ) {
		regex_list = gyach_g_list_free( regex_list );

		/* load the new list */
		while( fgets( buf, 128, fp )) {
			if (strlen(buf)>2) {  
			/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
				/* chop the newline */
				buf[strlen(buf)-1] = '\0';
				regex_add( buf );
			}
		}

		fclose( fp );
		/*
		regex_list = g_list_sort( regex_list, (GCompareFunc)gstrcmp );
		*/
	}
}

FILE *fp;

static void print_ignore_hash(gpointer key, gpointer value,
        gpointer user_data) {
	
	if (strlen((char *)key) > 1) {
	/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
	fprintf( fp, "%s\n", (char *)key );
	}
}

void ignore_save() {
	GList *this_regex;
	char if_filename[192];

	// DBG( 11, "ignore_save()\n" );

	snprintf( if_filename, 190, "%s/ignore", GYACH_CFG_COMMON_DIR );

	fp = fopen( if_filename, "w" );

	if ( fp ) {
		if ( ignore_hash ) {
			g_hash_table_foreach( ignore_hash, print_ignore_hash, NULL );
		}
		fclose( fp );
	}

	snprintf( if_filename, 190, "%s/ignore.regex", GYACH_CFG_DIR );

	fp = fopen( if_filename, "w" );

	if ( fp ) {
		this_regex = regex_list;
		while( this_regex ) {
			if (strlen((char *)this_regex->data) > 1) {
				/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
				fprintf( fp, "%s\n", (char *)this_regex->data );
			}
			this_regex = g_list_next( this_regex );
		}

		fclose( fp );
	}
}

void ignore_toggle( char *user ) {
	GtkTreeModel *model;
	int r;
	int i;
	char dbuf[160] ;
	GdkPixbuf *imbuf=NULL;

	char *yalias=NULL;
	yalias=get_screenname_alias(user);

	// DBG( 11, "ignore_toggle( '%s' )\n", user );

	r = find_user_row( user );

	if ( user[0] ) {
		if ( ignore_check( user )) {
			if ( mute_check( user )) {
				mute_remove( user );
				if ( r >= 0 ) {mutey_count_in_room--;}
			}
			ignore_remove( user );
			if ( r >= 0 ) {
				if ( mute_check( user )) {
					igg_count_in_room--;
					imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
				} else {
				imbuf=gdk_pixbuf_new_from_xpm_data((const char**)choose_pixmap(user));
				}
			}		


			snprintf( dbuf, 158, " *** %s %s %s ( %s ) %s ***\n", _("no longer ignoring"), 
				YAHOO_COLOR_RED, user, yalias, YAHOO_COLOR_BLACK );
			set_last_comment( user, "[no longer ignoring]" );


		} else {
			if (ymsg_sess)  {  /* added: PhrozenSmoke, we don't ignore ourselves */
				if (ymsg_sess->user) {
					if ( ! strcasecmp( user, ymsg_sess->user))	{return;}
												   }				
									 }
			if (find_profile_name(user)) {return;}

			ignore_add( user );
			if ( r >= 0 ) {
				igg_count_in_room++;
				imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
			}

			if (BROADCAST_IGNORE==1) {
				char bufr[384];
			snprintf( bufr, 381, "%s%s* [ Whoop Dat Trick 1.0.8 - IGNORING: '%s' ( %s ) ] *%s    %s*CLICK*%s  >:p  Acceptable levels of ignorance have been exceeded:  :-@ . %sYou are #%d in my iggy bin.  =D>  %sSee ya!  ;)) " ,YAHOO_STYLE_BOLDON,YAHOO_COLOR_RED,user,yalias,YAHOO_STYLE_BOLDOFF, YAHOO_COLOR_PURPLE, YAHOO_COLOR_BLUE, YAHOO_COLOR_GREEN, g_hash_table_size( ignore_hash ), YAHOO_COLOR_ORANGE);

			ymsg_comment(ymsg_sess,bufr);
			strcat(bufr,"\n");
			append_to_textbox( chat_window, NULL, bufr );

															  } /* end if BROADCAST_IGNORE */

			snprintf( dbuf, 158, " *** %s %s %s  ( %s ) %s ***\n", _("ignoring"), 
				YAHOO_COLOR_RED, user, yalias, YAHOO_COLOR_BLACK ); 
				/* this just shows to screen, not to room */

		}
		append_to_textbox( chat_window, NULL, dbuf );
	} else {
		strncpy( dbuf, "*** Ignoring the following users:\n" , 149);
		append_to_textbox( chat_window, NULL, dbuf );
		if ( ignore_hash ) {
			g_hash_table_foreach( ignore_hash, show_hash, NULL );
		}
		i = g_hash_table_size( ignore_hash );
		if ( i ) {
			snprintf( dbuf, 149, "*** Ignoring %d users ***\n", i );
		} else {
			strncpy( dbuf, "\tNo Users Ignored\n" , 149);
		}
		append_to_textbox( chat_window, NULL, dbuf );
	}

	ignore_save();

	if (yalias) {g_free(yalias); yalias=NULL;}

	if ( r < 0 ) {	return;  }

		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return;}

	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_PIX, imbuf,
				 -1);
	if (imbuf) {g_object_unref(imbuf);}
	set_status_room_counts();
}

int ignore_count_in_room() {
	return igg_count_in_room;
}

int ignore_count() {
	if ( ! ignore_hash ) {
		return( 0 );
	} else {
		return( g_hash_table_size( ignore_hash ));
	}
}

void mute_add( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return;} 
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	if ( ! mute_hash ) {
		mute_hash = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! mute_hash ) {return;}
    	if ( ! g_hash_table_lookup( mute_hash, tmp_user )) {
		g_hash_table_insert( mute_hash, g_strdup( tmp_user ), g_strdup( "1" ));
	}
}

void mute_remove( char *user ) {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! user ) {	return;} 
	if ( ! mute_hash ) {return;	}

	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	if ( g_hash_table_lookup_extended( mute_hash, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( mute_hash, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );
		if ( value_ptr )
			g_free( value_ptr );
	}
}

int mute_check( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return 0;} 
	if ( ! mute_hash ) {return( 0 );}
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );

	if ( g_hash_table_lookup( mute_hash, tmp_user )) {
		return( 1 );
	}
	return( 0 );
}

void mute_toggle( char *user ) {
	GtkTreeModel *model;
	int r;
	int i;
	char dbuf[160] ;
	GdkPixbuf *imbuf=NULL;
	char *yalias=NULL;
	yalias=get_screenname_alias(user);

	// DBG( 11, "mute_toggle( '%s' )\n", user );

	r = find_user_row( user );

	if ( user[0] ) {
		if ( mute_check( user )) {
			mute_remove( user );			
			if ( r >= 0 ) {
				mutey_count_in_room--;
				if ( ignore_check( user )) {
					imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
				} else {
					imbuf=gdk_pixbuf_new_from_xpm_data((const char**)choose_pixmap(user));
				}
			}
			snprintf( dbuf,158, " *** %s %s %s ( %s ) %s ***\n", _("no longer muting"), 
				YAHOO_COLOR_BLUE, user, yalias, YAHOO_COLOR_BLACK );
			set_last_comment( user, "[no longer muting]" );
		} else {
			if (ymsg_sess)  {  /* added: PhrozenSmoke, we don't mute ourselves */
				if (ymsg_sess->user) {
					if ( ! strcasecmp( user, ymsg_sess->user))	{return;}
												   }				
									 }
			if (find_profile_name(user)) {return;}

			mute_add( user );			
			if ( r >= 0 ) {
				mutey_count_in_room++;
				imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
			}
			snprintf( dbuf, 158, " *** %s %s%s ( %s ) %s ***\n", _("muting"), 
				YAHOO_COLOR_BLUE, user, yalias, YAHOO_COLOR_BLACK );
		}
		if (!quiet_mutes) { append_to_textbox( chat_window, NULL, dbuf ); }
	} else {
		strncpy( dbuf, "*** Muting the following users:\n" , 149);
		append_to_textbox( chat_window, NULL, dbuf );
		if ( mute_hash ) {
			g_hash_table_foreach( mute_hash, show_hash, NULL );
		}
		i = g_hash_table_size( mute_hash );
		if ( i ) {
			snprintf( dbuf, 149, "*** Muting %d users ***\n", i );
		} else {
			strncpy( dbuf, "\tNo Users Muted\n", 149 );
		}
		append_to_textbox( chat_window, NULL, dbuf );
	}

	if (yalias) {g_free(yalias); yalias=NULL;}

	if ( r < 0 ) {	return;  }

		model=gtk_tree_view_get_model(chat_users);
		if (!model) {model=chat_user_model;}
		if (!model) {return;}

	gtk_list_store_set(GTK_LIST_STORE(model), 
				&chat_user_iter, 
				GYQUAD_PIX, imbuf,
				 -1);
	if (imbuf) {g_object_unref(imbuf);}

	set_status_room_counts();
}

int mute_count_in_room() {
	return mutey_count_in_room;
}

void regex_toggle( char *regex ) {
	char buf[128] = "";

	// DBG( 11, "regex_toggle( '%s' )\n", regex );

	if ( regex[0] ) {
		if ( regex_check( regex )) {
			regex_remove( regex );
			snprintf( buf,126, "*** no longer auto-ignoring on regex '%s%s%s' ***\n",
				YAHOO_COLOR_BLUE, regex, YAHOO_COLOR_BLACK );
		} else {
			regex_add( regex );
			snprintf( buf,126, "*** auto-ignoring on regex '%s%s%s' ***\n",
				YAHOO_COLOR_BLUE, regex, YAHOO_COLOR_BLACK );
		}
		append_to_textbox( chat_window, NULL, buf );
	}

	ignore_save();
}


int regex_match( char *regex_str, char *str ) {
	regex_t regex;
	int result = 0;

	regcomp( &regex, regex_str, REG_EXTENDED | REG_ICASE | REG_NOSUB );
	if ( ! regexec( &regex, str, 0, NULL, 0 ))
		result = 1;

	regfree( &regex );

	return( result );
}



/* PhrozenSmoke:  a GUI for handling Yahoo's permenant ignore list */

gboolean on_close_ignorewin_dialogw      (GtkWidget       *widget,       
                                        GdkEvent        *event,                
                                        gpointer         user_data)
{
 igglist_visible=0;
 free_igglist_row_data(); 
 gtk_widget_destroy(widget);
return 1;
}  


GtkWidget* create_ignorelistwindow ()
{
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *hbox1;
  GtkWidget *scrolledwindow1;
  GtkWidget *vbox2;
  GtkWidget *label3;
  GtkWidget *vbox3;
  char usname[25]="";
  char *col_headers[]={"",NULL};

  snprintf(usname,24,"%s:",_("User"));

  ignorewindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (ignorewindow), "gyachEIgnore", "GyachE");
  gtk_window_set_title (GTK_WINDOW (ignorewindow), _("Yahoo Ignore List"));

  if (ignore_list_type==2) 	{ 
	gtk_window_set_title (GTK_WINDOW (ignorewindow), _("Temporary Friends List"));
							  }
  if (ignore_list_type==3) 	{ 
	gtk_window_set_title (GTK_WINDOW (ignorewindow), _("Flooder/Booter List"));
							  }
  if (ignore_list_type==4) 	{ 
	gtk_window_set_title (GTK_WINDOW (ignorewindow), _("GYach Enhanced Ignore List"));
							  }
  gtk_window_set_position (GTK_WINDOW (ignorewindow), GTK_WIN_POS_CENTER);
	/* IMPORTANT: Do not have more than one of these windows 
	open at once! */
  gtk_window_set_modal (GTK_WINDOW (ignorewindow), TRUE);

  vbox1 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (ignorewindow), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 4);

  if (ignore_list_type==2) 	{ 
  label1 = gtk_label_new (_("Edit your list of temporary friends.\nAdding a user as a temporary friend allows\nthem to send you messages, invitations, and files,\nwithout having to open your PMs to everyone,\nand without having to add them to your real buddy list."));
							  }
  else if (ignore_list_type==3) 	{ 
  label1 = gtk_label_new (_("Edit the list of flooders/booters.\nA user is designated as a Flooder\nif it appears they are trying to harass you\nor boot you, and significant steps are taken\nto reduce their ability to communicate\nwith you. You may remove users from this list."));
							  			}
  else if (ignore_list_type==4) 	{ 
  label1 = gtk_label_new (_("Edit Gyach Enhanced's list of ignored users.\nThis list is stored locally on your computer."));
							  			}
  else {  /* igg list type 1 */
  label1 = gtk_label_new (_("Edit your list of permenantly ignored users.\nThis list is stored on Yahoo's servers,\nseparate from Gyach Enhanced's local ignore list."));
	}
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (label1), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (label1), 0, 0.5);

  hbox1 = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolledwindow1, 275, -2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_ETCHED_IN);

  iggclist = create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 0, 
	1, col_headers);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET(iggclist));

  vbox2 = gtk_vbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  igg_delbutton = get_pixmapped_button(_("Delete"), GTK_STOCK_REMOVE);
  gtk_box_pack_start (GTK_BOX (vbox2), igg_delbutton, FALSE, FALSE, 0);
  set_tooltip(igg_delbutton,_("Delete"));

  label3 = gtk_label_new ("");
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (vbox2), label3, TRUE, TRUE, 0);
  gtk_widget_set_usize (label3, -2, 140);
  gtk_label_set_justify (GTK_LABEL (label3), GTK_JUSTIFY_LEFT);

  vbox3 = gtk_vbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox2), vbox3, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new(usname), FALSE, FALSE, 0);

  igg_addentry = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(igg_addentry),62);
  gtk_box_pack_start (GTK_BOX (vbox3), igg_addentry, FALSE, FALSE, 0);
  set_tooltip(igg_addentry,_("User"));
  if (ignore_list_type==3) 	{ gtk_widget_set_sensitive(igg_addentry, 0);}

  igg_addbutton = get_pixmapped_button(_("Add"), GTK_STOCK_ADD);
  set_tooltip(igg_addbutton,_("Add"));
  gtk_box_pack_start (GTK_BOX (vbox3), igg_addbutton, FALSE, FALSE, 0);
  if (ignore_list_type==3) 	{ gtk_widget_set_sensitive(igg_addbutton, 0);}

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (ignorewindow, ignorewindow, "ignorewindow");
  GLADE_HOOKUP_OBJECT (ignorewindow, vbox1, "vbox1");
  GLADE_HOOKUP_OBJECT (ignorewindow, label1, "label1");
  GLADE_HOOKUP_OBJECT (ignorewindow, hbox1, "hbox1");
  GLADE_HOOKUP_OBJECT (ignorewindow, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT (ignorewindow, GTK_WIDGET(iggclist), "iggclist");
  GLADE_HOOKUP_OBJECT (ignorewindow, vbox2, "vbox2");
  GLADE_HOOKUP_OBJECT (ignorewindow, igg_delbutton, "igg_delbutton");
  GLADE_HOOKUP_OBJECT (ignorewindow, label3, "label3");
  GLADE_HOOKUP_OBJECT (ignorewindow, vbox3, "vbox3");
  GLADE_HOOKUP_OBJECT (ignorewindow, igg_addentry, "igg_addentry");
  GLADE_HOOKUP_OBJECT (ignorewindow, igg_addbutton, "igg_addbutton");
  gtk_signal_connect (GTK_OBJECT (igg_delbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_del_perm_igg_cb), NULL);
  g_signal_connect (iggclist, "row-activated", (GCallback) onIgglistRowActivated,
                      NULL);
  gtk_signal_connect (GTK_OBJECT (igg_addbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_add_perm_igg), NULL);
gtk_signal_connect (GTK_OBJECT (ignorewindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ignorewin_dialogw), NULL);

  set_basic_treeview_sorting(GTK_WIDGET(iggclist),GYLIST_TYPE_SINGLE);
  gtk_widget_show_all(ignorewindow);
  igglist_visible=1;
  return ignorewindow;
}


int perm_igg_check( char *user ) {
	char tmp_user[64];
	if ( ! user ) {return( 0 );	} 
	if ( ! perm_igg_hash ) {return( 0 );}
	strncpy( tmp_user, user,62 );
	lower_str( tmp_user );
	if ( g_hash_table_lookup( perm_igg_hash, tmp_user )) {
		return( 1 );
	}
	return( 0 );
}



int add_perm_igg( char *user ) {
	char tmp_user[64];
	char tmp_stat[4];
	char *ptr = NULL;

	if ( ! perm_igg_hash ) {
		perm_igg_hash = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! perm_igg_hash ) {return( 0 );}

	strncpy( tmp_user, user, 62 );
	lower_str(tmp_user);
	strcpy( tmp_stat, "" );

	if (( ptr = g_hash_table_lookup( perm_igg_hash, tmp_user )) != NULL ) {
		g_hash_table_insert( perm_igg_hash, g_strdup(tmp_user), g_strdup(tmp_stat));
		g_free( ptr );
	} else {
		g_hash_table_insert(perm_igg_hash, g_strdup(tmp_user), g_strdup(tmp_stat));
	}
	return( 1 );
}


void remove_perm_igg(char *user)  {
	char tmp_user[64];
	gpointer key_ptr;
	gpointer value_ptr;
	if ( ! perm_igg_hash ) { return; }
	strncpy( tmp_user, user , 62);
	lower_str( tmp_user );

	if ( g_hash_table_lookup_extended( perm_igg_hash, tmp_user, &key_ptr,
			&value_ptr )) {
		g_hash_table_remove( perm_igg_hash, tmp_user );
		if ( key_ptr )
			g_free( key_ptr );  /* seg-fault watch ?  */
		if ( value_ptr )
			g_free( value_ptr );   /* seg-fault watch ?  */
									}
															  }

int clear_perm_igg_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
	g_free(key);
	g_free(value);
 	return 1;
}

void clear_perm_igg_hash( ) {
	if ( ! perm_igg_hash ) { return; }
	g_hash_table_foreach_remove(perm_igg_hash,clear_perm_igg_hash_cb,NULL);
}

/* Free up all the g_strdup'ed strings used as 
    row data in igglist CLIST, mem-leak fix */

void free_igglist_row_data() {
	gy_empty_model(gtk_tree_view_get_model(iggclist), GYTV_TYPE_LIST);
}

void append_to_iggclist(gpointer key, gpointer value, gpointer user_data) {
   	char user[72];
	char *rd;
	GtkTreeIter iter;
	GdkPixbuf *imbuf=NULL;

	gtk_list_store_append(GTK_LIST_STORE(iggfreezer), &iter);

	snprintf(user,70 ,"%s",(char *)key);
	rd=strdup(user);
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_online);
	gtk_list_store_set(GTK_LIST_STORE(iggfreezer), &iter, 
			GYSINGLE_PIX_TOGGLE, TRUE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, rd,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "", -1);
	g_object_unref(imbuf);
	free(rd);
}

void update_perm_igg_clist() {
        if (! igglist_visible) {return; }
	if (ignore_list_type==1) {
		if ( ! perm_igg_hash ) {free_igglist_row_data(); return;}
								  }

	if (ignore_list_type==2) {
		if (! temporary_friend_list) {free_igglist_row_data();  return;}
								   } 

	if (ignore_list_type==3) {
		if (! flooder_list) {free_igglist_row_data();  return;}
								   } 

	if (ignore_list_type==4) {
		if ( ! ignore_hash ) {free_igglist_row_data(); return;}
								  }

			if ( selected_permigg )   {
				free( selected_permigg );
				selected_permigg=NULL;
						            }
			free_igglist_row_data(); 
			iggfreezer=freeze_treeview(GTK_WIDGET(iggclist));
			if (ignore_list_type==1) {
				g_hash_table_foreach(perm_igg_hash, append_to_iggclist, NULL );
												}
			if (ignore_list_type==4) {
				g_hash_table_foreach(ignore_hash, append_to_iggclist, NULL );
												}
			if (ignore_list_type==2) 	{
				GList *this_friend;
				char *blank=NULL;
				this_friend = temporary_friend_list;
				while( this_friend ) {
					append_to_iggclist(this_friend->data, blank, blank);
					this_friend = g_list_next( this_friend );
											}
											   	 }

			if (ignore_list_type==3) 	{
				GList *this_friend;
				char *blank=NULL;
				this_friend = flooder_list;
				while( this_friend ) {
					append_to_iggclist(this_friend->data, blank, blank);
					this_friend = g_list_next( this_friend );
											}
											   	 }
			unfreeze_treeview(GTK_WIDGET(iggclist), iggfreezer);
}




int check_perm_igg_remove_timeout(char *myname) {
	if (myname) {
		if ( (! using_web_login) && (! ycht_is_running()) ) {
			if (perm_igg_check(myname) ) {
				snprintf(iggbuf, 382, "'%s' %s %s %s", myname,
				_("could not be removed from the ignore list."),
				_("Gyach-E received no response from Yahoo's servers."),
				_("Yahoo's servers may be down or malfunctioning. Please try again later.")
				);
				show_ok_dialog(iggbuf);
			}
		}		
		free(myname);
	}
	return 0;
}
void start_check_perm_igg_remove_timeout(char *myname) {gtk_timeout_add( 8000,
				(void *)check_perm_igg_remove_timeout, myname );}


int check_perm_igg_add_timeout(char *myname) {
	if (myname) {
		if ( (! using_web_login) && (! ycht_is_running()) && (! is_flooder(myname)) ) {
			if (! perm_igg_check(myname) ) {
				snprintf(iggbuf, 382, "'%s' %s %s %s", myname,
				_("could not be added to the ignore list."),
				_("Gyach-E received no response from Yahoo's servers."),
				_("Yahoo's servers may be down or malfunctioning. Please try again later.")
				);
				show_ok_dialog(iggbuf);
			}
		}		
		free(myname);
	}
	return 0;
}
void start_check_perm_igg_add_timeout(char *myname) {gtk_timeout_add( 8000,
				(void *)check_perm_igg_add_timeout, myname );}


void on_del_perm_igg ()
{
	GtkWidget *tmp_widget;
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *iggnm;

	selection=gtk_tree_view_get_selection(iggclist);
	if (! gtk_tree_selection_get_selected(selection, &model, &iter)) {return;}
	if (selected_permigg) { free(selected_permigg); selected_permigg=NULL;}
	gtk_tree_model_get(gtk_tree_view_get_model(iggclist), &iter,GYSINGLE_COL1, &iggnm, -1);
	selected_permigg=strdup(iggnm);
	g_free(iggnm);

	tmp_widget = lookup_widget( ignorewindow, "igg_addentry" );
	gtk_entry_set_text(GTK_ENTRY(tmp_widget),selected_permigg);
	if (ignore_list_type==1) { 
		if ( using_web_login || ycht_is_running() ) {
			show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
			if (selected_permigg) { free(selected_permigg); selected_permigg=NULL;}
			return;
		}

		start_check_perm_igg_remove_timeout(strdup(selected_permigg));
		ymsg_perm_ignore(ymsg_sess,selected_permigg, 1); 
	 }
	if (ignore_list_type==2) {
		remove_temporary_friend( selected_permigg);
		update_perm_igg_clist();
								   }
	if (ignore_list_type==3) {
		remove_flooder( selected_permigg);
		update_perm_igg_clist();
								   }
	if (ignore_list_type==4) {
		ignore_toggle( selected_permigg); 
		update_perm_igg_clist();
								   }
	if (selected_permigg) { free(selected_permigg); selected_permigg=NULL;}
}

void on_del_perm_igg_cb (GtkButton *button,gpointer  user_data)
{
	on_del_perm_igg();
}

void onIgglistRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		on_del_perm_igg();
}


/* Fix Me: Possible memory leaks here from newigg not being freed
    in all instances below */ 

void on_add_perm_igg (GtkButton *button,gpointer  user_data)
{
	char *newigg=NULL;
	GtkWidget *tmp_widget;
	tmp_widget = lookup_widget( ignorewindow, "igg_addentry" );
	newigg=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));
	if (strlen(newigg)<1) {free(newigg); return;}

	if ( (ignore_list_type==1) || (ignore_list_type==4)) {
		/* Never ignore ourselves */
		if (! strcasecmp(newigg, ymsg_sess->user)) {free(newigg); return;}
		if (find_profile_name( newigg)) {free(newigg); return;}
	}

	if (ignore_list_type==1) { 
		if ( using_web_login || ycht_is_running() ) {
			show_ok_dialog(_("This feature is not available when connected using the WEBCONNECT or YCHT servers."));
			if (newigg) { free(newigg);}
			return;
		}
		start_check_perm_igg_add_timeout(strdup(newigg));
		ymsg_perm_ignore(ymsg_sess,newigg, 0); 
		free(newigg);
	}
	if (ignore_list_type==2) {
		add_temporary_friend(newigg);
		update_perm_igg_clist();
								   }
	if (ignore_list_type==3) {  /* we should never get here */
		add_flooder(newigg);
		update_perm_igg_clist();
								   }
	if (ignore_list_type==4) { 
		if (! ignore_check(newigg)) {
			ignore_toggle(newigg);
									 			}
		update_perm_igg_clist();
								   }
	gtk_entry_set_text(GTK_ENTRY(tmp_widget),"");
}

void open_perm_ignore (GtkMenuItem *menuitem, gpointer  user_data)
{
ignore_list_type=1;
create_ignorelistwindow ();
update_perm_igg_clist();
}

void open_temp_friend_editor (GtkMenuItem *menuitem, gpointer  user_data)
{
ignore_list_type=2;
create_ignorelistwindow ();
update_perm_igg_clist();
}

void open_flooder_editor (GtkMenuItem *menuitem, gpointer  user_data)
{
ignore_list_type=3;
create_ignorelistwindow ();
update_perm_igg_clist();
}

void open_ignore_editor (GtkMenuItem *menuitem, gpointer  user_data)
{
ignore_list_type=4;
create_ignorelistwindow ();
update_perm_igg_clist();
}


void immunity_add( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return;} 
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );
	if ( ! immunity_hash ) {
		immunity_hash = g_hash_table_new( g_str_hash, g_str_equal );
	}
	if ( ! immunity_hash ) {return;}
	if ( mute_check( tmp_user)) { mute_toggle( tmp_user); }
    	if ( ! g_hash_table_lookup( immunity_hash, tmp_user )) {
		char dbuf[160];
		char *yalias=NULL;
		yalias=get_screenname_alias(user);
		g_hash_table_insert( immunity_hash, g_strdup( tmp_user ), g_strdup( "1" ));
		snprintf( dbuf, 158, "  *** %s %s  ( %s ) %s %s ***\n", 
				YAHOO_COLOR_ORANGE, user, yalias, 
				_("is now immune from auto-muting."), 
				YAHOO_COLOR_BLACK ); 
		append_to_textbox( chat_window, NULL, dbuf );
		g_free(yalias);
	}
}

int immunity_check( char *user ) {
	char tmp_user[64];
	if ( ! user ) {	return 0;} 
	if ( ! immunity_hash ) {return( 0 );}
	strncpy( tmp_user, user, 62 );
	lower_str( tmp_user );
	if ( g_hash_table_lookup( immunity_hash, tmp_user )) {	return( 1 );}
	return( 0 );
}





