/*****************************************************************************
 * gyach_int.c
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

#include "config.h"

#include <sys/stat.h>
#include <unistd.h>

#include "gyach.h"
#include "callbacks.h"
#include "friends.h"
#include "gyach_int.h"
#include "ignore.h"
#include "images.h"
#include "interface.h"
#include "history.h"
#include "main.h"
#include "roomlist.h"
#include "users.h"
#include "util.h"
#include "trayicon.h"   /* added, PhrozenSmoke */
#include "plugins.h"   /* added, PhrozenSmoke */
#include "gytreeview.h"
#include "fonts.h"

extern void on_setup_activate_button    (GtkButton       *button,  gpointer         user_data);
extern void enable_yab_address_book();
extern void freeze_yab_address_book();
extern char *custom_scale_text;
extern int use_buddy_list_font;
extern int show_my_status_in_chat;
extern int pm_nick_names;

extern int enable_addressbook;
extern int enable_pm_searches;

GtkWidget *color_window = NULL;
GList *yserverlist = NULL;
GList *yserverport_list = NULL;

int highlight_friends = 0;
int	indent_wrapped;
#define INDENT_DEFAULT 60

extern int show_quick_access_toolbar;
GtkWidget *top_text;
GtkWidget *vpaned;
int transparent_bg = 0;
int shading_r = 0;
int shading_g = 0;
int shading_b = 0;

/* items on menu to activate when connected */
static char *conn_menu_items[] = {
		"disconnect",	"rooms", "quick_access", 
		"status", "tools", "actions", "buddy_mainvbox",
		NULL
	};

/* items on menu to activate when DISconnected */
static char *disconn_menu_items[] = {
		"connect",
		NULL
	};

int build_toolbar( GtkWidget *chat_window ) {
	GtkWidget *tgl_bold;
	GtkWidget *tgl_italics;
	GtkWidget *tgl_underline;
	GtkWidget *bt_colors;
	GtkWidget *bt_fonts;
	GtkWidget *chat_toolbar;
	GtkWidget *imimage=NULL;
	GdkPixbuf *imbuf=NULL;
	GdkPixbuf *spixbuf=NULL;
	int tsize = 16;        /* size of toolbar buttons in pixels wide & high */

	/* PhrozenSmoke: disabled the 'exit Gyach' and packet debugging toolbar buttons, 
	    most people aren't using the packet debugging, and the toolbar for 'exit' 
            can be confused easily with the 'exit room' idea, so these are accessible from the 
            menu only now */

	chat_toolbar = lookup_widget( chat_window, "chat_toolbar" );

	/* now the bold button */
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_bold);
	imimage=gtk_image_new_from_pixbuf(imbuf);
	tgl_bold = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
		GTK_TOOLBAR_CHILD_TOGGLEBUTTON, NULL, " B ", _("Send your text in Bold"),
		NULL, imimage, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (chat_window), "tgl_bold", tgl_bold);
	gtk_widget_show (tgl_bold);
	gtk_widget_set_usize (tgl_bold, tsize, tsize);

	/* now the italic button */
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_italic);
	imimage=gtk_image_new_from_pixbuf(imbuf);
	tgl_italics = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
		GTK_TOOLBAR_CHILD_TOGGLEBUTTON, NULL, " I ", _("Send your text in italic"),
		NULL, imimage, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (chat_window), "tgl_italics", tgl_italics);
	gtk_widget_show (tgl_italics);
	gtk_widget_set_usize (tgl_italics, tsize, tsize);

	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_underline);
	imimage=gtk_image_new_from_pixbuf(imbuf);
	tgl_underline = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
		GTK_TOOLBAR_CHILD_TOGGLEBUTTON, NULL, " U ",
		_("Send your text in underline"), NULL, imimage, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (chat_window), "tgl_underline", tgl_underline);
	gtk_widget_show (tgl_underline);
	gtk_widget_set_usize (tgl_underline, tsize, tsize);

	/* give us a blank */
	// gtk_toolbar_append_space( GTK_TOOLBAR (chat_toolbar));

	/* colors button */
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_colors);
	imimage=gtk_image_new_from_pixbuf(imbuf);
	bt_colors = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
		GTK_TOOLBAR_CHILD_BUTTON, NULL, _("Colors"), _("Select Color"),
		NULL, imimage, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (chat_window), "bt_colors", bt_colors);
	gtk_widget_show (bt_colors);
	gtk_widget_set_usize (bt_colors, tsize, tsize);

	/* give us a blank */
	// gtk_toolbar_append_space( GTK_TOOLBAR (chat_toolbar));

		/* fader button on PM windows */ 
	if (gtk_object_get_data(GTK_OBJECT(chat_window), "with_fader_button") != NULL ) {
		GtkWidget *bt_fader;
		imbuf=gtk_widget_render_icon(GTK_WIDGET(chat_toolbar), GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU, "gywindow");
		spixbuf = gdk_pixbuf_scale_simple( imbuf, 14, 14,
			GDK_INTERP_BILINEAR );
		imimage=gtk_image_new_from_pixbuf(spixbuf);
		bt_fader = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
			GTK_TOOLBAR_CHILD_BUTTON, NULL, _("Configuration: Text Effects"), _("Configuration: Text Effects"),
			NULL, imimage, NULL, NULL);
		gtk_button_set_relief(GTK_BUTTON(bt_fader), GTK_RELIEF_NONE);
		gtk_widget_show (bt_fader);
		gtk_signal_connect (GTK_OBJECT (bt_fader), "clicked",
			GTK_SIGNAL_FUNC (on_create_fadeconfigwin),NULL);
	}

	imbuf=gtk_widget_render_icon(GTK_WIDGET(chat_toolbar), GTK_STOCK_BOLD, GTK_ICON_SIZE_MENU, "gywindow");
	spixbuf = gdk_pixbuf_scale_simple( imbuf, 14, 14,
		GDK_INTERP_BILINEAR );
	imimage=gtk_image_new_from_pixbuf(spixbuf);
	bt_fonts = gtk_toolbar_append_element (GTK_TOOLBAR (chat_toolbar),
		GTK_TOOLBAR_CHILD_BUTTON, NULL, _("Font"), _("Font"),
		NULL, imimage, NULL, NULL);
	gtk_object_set_data (GTK_OBJECT (chat_window), "bt_fonts", bt_fonts);
	gtk_widget_show (bt_fonts);

	gtk_button_set_relief(GTK_BUTTON(bt_fonts), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(bt_colors), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(tgl_bold), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(tgl_italics), GTK_RELIEF_NONE);
	gtk_button_set_relief(GTK_BUTTON(tgl_underline), GTK_RELIEF_NONE);

	/* setup some signals */
	gtk_signal_connect (GTK_OBJECT (tgl_bold), "toggled",
		GTK_SIGNAL_FUNC (on_tgl_bold_toggled), NULL);
	gtk_signal_connect (GTK_OBJECT (tgl_italics), "toggled",
		GTK_SIGNAL_FUNC (on_tgl_italics_toggled), NULL);
	gtk_signal_connect (GTK_OBJECT (tgl_underline), "toggled",
		GTK_SIGNAL_FUNC (on_tgl_underline_toggled), NULL);
	gtk_signal_connect (GTK_OBJECT (bt_colors), "clicked",
		GTK_SIGNAL_FUNC (on_setup_activate_button), "color");
	gtk_signal_connect (GTK_OBJECT (bt_fonts), "clicked",
		GTK_SIGNAL_FUNC (on_setup_activate_button), "font");

	return( 0 );
}

#ifdef TRANS_BG
gboolean configure_event_notify (GtkWidget *widget, GdkEvent *event,
		gpointer user_data) {
	if ( transparent_bg )
		ct_update_bg();

	return( FALSE );
}
#endif

GtkWidget *build_chat_window( void ) {
	GtkWidget *tmp_widget;
	GtkStyle *windowstyle;
	GdkPixmap *pm_icon;
	GdkBitmap *mask_icon;

	chat_window = create_window1();
	gtk_window_set_title( GTK_WINDOW(chat_window), "GYach Enhanced v." VERSION );

	/* set default window sizes from saved values */
	if ( remember_position )
		gtk_widget_set_uposition( chat_window, chat_window_x, chat_window_y );
	gtk_widget_set_usize( chat_window, chat_window_width, chat_window_height );
	gtk_window_set_default_size( GTK_WINDOW(chat_window), chat_window_width,
		chat_window_height );

	ct_build();
	ct_set_max_lines( buffer_size );

	tmp_widget = lookup_widget( chat_window, "hbox2" );
	gtk_widget_set_usize( tmp_widget, chat_text_width, -2 );
	tmp_widget = lookup_widget( chat_window, "scrolledwindow3" );
	gtk_widget_set_usize( tmp_widget, -2, chat_user_height );

	/* set the chat entry box font */
	/* PhrozenSmoke: Added, this is done elsewhere */

	gtk_widget_show_all( chat_window );

#ifdef TRANS_BG
	gtk_signal_connect (GTK_OBJECT (chat_window), "configure_event",
	        GTK_SIGNAL_FUNC(configure_event_notify), NULL);

	if ( transparent_bg ) {

		ct_set_transparent_shade( shading_r, shading_g, shading_b );

		ct_set_transparent( 1 );

	}
#endif

	/* attach our icon */
	windowstyle = gtk_widget_get_style( chat_window );
	pm_icon = gdk_pixmap_create_from_xpm_d( chat_window->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( chat_window->window, NULL, pm_icon, mask_icon );

	build_toolbar( chat_window );

	st_cid = gtk_statusbar_get_context_id( GTK_STATUSBAR(chat_status),
		"Gyach" );

	set_menu_connected( 0 );

	/* set any options that need the window created */
	set_config();

	/* disabled, PhrozenSmoke, do these when they are needed */ /*  
	chat_user_menu = create_user_menu();
	alias_menu = create_alias_menu();
	setup_menu = create_setup_menu();  */ 

	return( chat_window );
}


static int shown_login_window_once=0;

void create_yserverport_list() {
	yserverport_list = g_list_append( yserverport_list, "5050" );
	yserverport_list = g_list_append( yserverport_list, "8001" );
	yserverport_list = g_list_append( yserverport_list, "8002" );
	yserverport_list = g_list_append( yserverport_list, "119" );
	yserverport_list = g_list_append( yserverport_list, "80" );
	yserverport_list = g_list_append( yserverport_list, "23" );
	/* 
	yserverport_list = g_list_append( yserverport_list, "20" );
	yserverport_list = g_list_append( yserverport_list, "25" );  */ 
}

void create_yserverlist() {
	yserverlist = g_list_append( yserverlist, " " ); /* empty entry, we need this */
	yserverlist = g_list_append( yserverlist, "scs.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcna.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcnb.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcnc.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcnd.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcne.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcnf.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcng.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-dcnh.msg.yahoo.com" );

	yserverlist = g_list_append( yserverlist, "scsa.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scsc.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scsdcntest-a.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scsdcntest-c.msg.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scsdcntest-e.msg.yahoo.com" );

	/* a few by IP address, known to be reliable, a little faster, and usually
           not over-trafficked */ 

	yserverlist = g_list_append( yserverlist, "216.155.193.128" );
	yserverlist = g_list_append( yserverlist, "216.155.193.137" );
	yserverlist = g_list_append( yserverlist, "216.155.193.143" );
	yserverlist = g_list_append( yserverlist, "216.155.193.151" );
	yserverlist = g_list_append( yserverlist, "216.155.193.155" );
	yserverlist = g_list_append( yserverlist, "216.155.193.164" );
	yserverlist = g_list_append( yserverlist, "216.155.193.173" );
	yserverlist = g_list_append( yserverlist, "216.155.193.179" );
	yserverlist = g_list_append( yserverlist, "216.155.193.182" );
	yserverlist = g_list_append( yserverlist, "216.155.193.185" );
	yserverlist = g_list_append( yserverlist, "216.155.193.187" );

	/* 'web login' servers for backups when Yahoo locks us out of the
	    normal servers */

	yserverlist = g_list_append( yserverlist, "WEB LOGIN 1" );
	yserverlist = g_list_append( yserverlist, "WEB LOGIN 2" );

	yserverlist = g_list_append( yserverlist, "YCHT 1" );
	yserverlist = g_list_append( yserverlist, "YCHT 2" );
	yserverlist = g_list_append( yserverlist, "YCHT 3" );	
	yserverlist = g_list_append( yserverlist, "YCHT 4" );


	/* yserverlist = g_list_append( yserverlist, "scs.yahoo.com" ); */  /* Not good!, limited functionality */
	/* Older servers */  	/* 
	yserverlist = g_list_append( yserverlist, "scs-fooa.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-foob.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-fooc.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-food.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-fooe.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "scs-foof.yahoo.com" );
	*/

	/* BELOW: old legacy servers for historical purposes */	 /*  
	yserverlist = g_list_append( yserverlist, "cs.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "cs.chat.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "cs2.chat.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "cs3.chat.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "cs4.chat.yahoo.com" );
	yserverlist = g_list_append( yserverlist, "cs5.chat.yahoo.com" );
	*/
}

GtkWidget *build_login_window( void ) {
	GtkWidget *tmp_widget;
	char mcbuf[128]="";
	
	login_credentials=NULL;
	login_window = create_login_window();

	if ( ! password ) {	password = strdup( "" );}


	tmp_widget = lookup_widget( login_window, "chat_port" );
	if (!yserverport_list) {create_yserverport_list();}
	gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), yserverport_list );
		/* an initial port is set when the ymsg_sess object is created, so this is safe */  
	snprintf(mcbuf,6,"%d", ymsg_sess->port?ymsg_sess->port:5050);
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), mcbuf); 


	tmp_widget = lookup_widget( login_window, "chat_username" );

	if ( login_list ) {	gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), login_list ); }

	if (shown_login_window_once) {
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->user ); 
											} else {
		if (last_loginname) {gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), last_loginname); }
														}

	if ( login_list ) {
		strncpy( ymsg_sess->user, (char*)login_list->data, 62 );
	} else {
		strcpy( ymsg_sess->user, "" );
	}
	if (last_loginname) {strncpy( ymsg_sess->user,last_loginname, 62 );}

	tmp_widget = lookup_widget( login_window, "chat_password" );
	gtk_entry_set_text( GTK_ENTRY(tmp_widget), password );

	if (shown_login_window_once) {
		gtk_entry_set_text( GTK_ENTRY(tmp_widget), ymsg_sess->password ); 
											}
	strncpy( ymsg_sess->password, gtk_entry_get_text(GTK_ENTRY(tmp_widget)) , 30 );

	if ( ! favroom_list ) {
		favroom_list = g_list_append( favroom_list,
			strdup( "Linux, FreeBSD, Solaris:1" ));
		favroom_list = g_list_append( favroom_list, strdup( "En Espanol:4" ));
	}

	tmp_widget = lookup_widget( login_window, "chat_room" );
	gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ""); 
	gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), favroom_list );

	if (shown_login_window_once) {
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->req_room ); 
											}

	if ( last_chatroom) {
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), last_chatroom); 
							}

		strncpy( ymsg_sess->req_room, gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)), 62 );

	tmp_widget = lookup_widget( login_window, "chat_server" );

	/* Fixed, PhrozenSmoke, only create this once, avoids memory leaks */
	if (!yserverlist) {create_yserverlist();}

	if ( last_chatserver) {
		GList *this_yserv;
		char *mbuf=mcbuf;
		snprintf(mcbuf,126, "%s", last_chatserver);
		this_yserv=yserverlist;
		/* Remove the first list element */
		if (this_yserv) {yserverlist = g_list_remove_link(yserverlist,this_yserv);}
		yserverlist = g_list_prepend( yserverlist, mbuf );
							}

	gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ""); 
	gtk_combo_set_popdown_strings( GTK_COMBO(tmp_widget), yserverlist );

	if (shown_login_window_once) {
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), ymsg_sess->host ); 
											}

	if ( last_chatserver) {
		gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry), last_chatserver); 
							}

	strncpy( ymsg_sess->host, gtk_entry_get_text( GTK_ENTRY(GTK_COMBO(tmp_widget)->entry)), 125 );

	strncpy( ymsg_sess->room, "[NONE]", 15 );

	tmp_widget = lookup_widget( login_window, "invis_button" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), login_invisible);

	tmp_widget = lookup_widget( login_window, "noroom_button" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), login_noroom);

	shown_login_window_once=1;
	return( login_window );
}

void set_config() {
	GtkWidget *tmp_widget;

	// DBG( 1, "set_config()\n" );

	tmp_widget = lookup_widget( chat_window, "tgl_bold" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tmp_widget), use_bold );

	tmp_widget = lookup_widget( chat_window, "tgl_italics" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tmp_widget), use_italics );

	tmp_widget = lookup_widget( chat_window, "tgl_underline" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(tmp_widget),
		use_underline );

	tmp_widget = lookup_widget( chat_window, "packet_debugging" );
	gtk_check_menu_item_set_active( GTK_CHECK_MENU_ITEM(tmp_widget),
		ymsg_sess->debug_packets );

	ct_set_max_lines( buffer_size );

	if ( indent_wrapped ) {
		ct_set_indent( INDENT_DEFAULT );
	} else {
		ct_set_indent( 0 );
	}

#ifdef USE_GTK2
	ct_set_transparent( transparent_bg );
	ct_set_transparent_shade( shading_r, shading_g, shading_b );
#endif
}


int set_menu_connected( int connected ) {
	GtkWidget *tmp_widget;
	char **on_items;
	char **off_items;
	int i;
	

	if ( connected ) {
		on_items = conn_menu_items;
		off_items = disconn_menu_items;
		trayicon_set_connected(connected);
		if (login_invisible) {trayicon_set_status(TRAY_ICON_INVISIBLE);}
		else {trayicon_set_status(TRAY_ICON_ONLINE_AVAILABLE);}
		enable_yab_address_book();
	} else {
		off_items = conn_menu_items;
		on_items = disconn_menu_items;
		trayicon_set_connected(connected);
		freeze_yab_address_book();
	}

	i = 0;
	while( on_items[i] ) {
		tmp_widget = lookup_widget( chat_window, on_items[i] );
		gtk_widget_set_sensitive( tmp_widget, 1 );
		i++;
	}

	i = 0;
	while( off_items[i] ) {
		tmp_widget = lookup_widget( chat_window, off_items[i] );
		gtk_widget_set_sensitive( tmp_widget, 0 );
		i++;
	}

	return( 0 );
}

static void fill_in_ignore_list(gpointer key, gpointer value, gpointer user_data) {
	GtkTreeIter iter;
	char *answer=NULL;
	GdkPixbuf *imbuf=NULL;
	GtkTreeModel *model = GTK_TREE_MODEL(user_data);

	answer=strdup((char *)key);
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_ignore);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, answer,
			GYSINGLE_COLOR, NULL, GYSINGLE_TOP, "",  -1);
	if (imbuf) {g_object_unref(imbuf);}
	free(answer);
}

void set_setup_options() {
	GtkWidget *tmp_widget;
	GList *this_item = NULL;
	GtkTreeModel *model;
	GtkTreeIter iter;
	char *answer=NULL;
	GdkPixbuf *imbuf=NULL;
	gdouble cur_color[4];

	// DBG( 11, "set_setup_options()\n" );

	/* show the correct settings */
	tmp_widget = lookup_widget( setup_window, "auto_login" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_login );

	tmp_widget = lookup_widget( setup_window, "blended_colors" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_blended_colors );

	tmp_widget = lookup_widget( setup_window, "packet_debugging" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), ymsg_sess->debug_packets );

	if ( auto_away ) {
		tmp_widget = lookup_widget( setup_window, "auto_away" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "auto_away_time" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)auto_away );
	} else {
		tmp_widget = lookup_widget( setup_window, "auto_away" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );
		 }

	tmp_widget = lookup_widget( setup_window, "custom_away_message" );
	if ( custom_away_message )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), custom_away_message );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );

	tmp_widget = lookup_widget( setup_window, "browser_command" );
	if ( browser_command )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), browser_command );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );

	tmp_widget = lookup_widget( setup_window, "filter_command" );
	if ( filter_command )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), filter_command );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );
	

	tmp_widget = lookup_widget( setup_window, "pm_brings_back" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_brings_back );

	tmp_widget = lookup_widget( setup_window, "disp_auto_ignored" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), disp_auto_ignored );

	tmp_widget = lookup_widget( setup_window, "remember_position" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), remember_position );

	tmp_widget = lookup_widget( setup_window, "remember_password" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), remember_password );

	tmp_widget = lookup_widget( setup_window, "auto_raise_pm" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_raise_pm );

	tmp_widget = lookup_widget( setup_window, "highlight_friends" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), highlight_friends );

	tmp_widget = lookup_widget( setup_window, "auto_close_roomlist" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_close_roomlist );

	tmp_widget = lookup_widget( setup_window, "cache_room_list" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), cache_room_list );

	tmp_widget = lookup_widget( setup_window, "chat_timestamp" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), chat_timestamp );

	tmp_widget = lookup_widget( setup_window, "indent_wrapped" );
#ifdef USE_GTK2
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), indent_wrapped );
#else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#endif

	tmp_widget = lookup_widget( setup_window, "chat_timestamp_pm" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), chat_timestamp_pm );

	tmp_widget = lookup_widget( setup_window, "auto_reply_when_away" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_reply_when_away );

	tmp_widget = lookup_widget( setup_window, "auto_reply_msg" );
	if ( auto_reply_msg )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), auto_reply_msg );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );

	tmp_widget = lookup_widget( setup_window, "custom_scale_text" );
	if ( custom_scale_text)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), custom_scale_text );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "1.0" );


	tmp_widget = lookup_widget( setup_window, "webcam_viewer_app" );
	if ( webcam_viewer_app)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), webcam_viewer_app );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "Gyach-E Webcam" );

	tmp_widget = lookup_widget( setup_window, "webcam_device" );
	if ( webcam_device)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), webcam_device );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "/dev/video0" );


	tmp_widget = lookup_widget( setup_window, "contact_first" );
	if ( contact_first)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_first );

	tmp_widget = lookup_widget( setup_window, "contact_last" );
	if ( contact_last)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_last );

	tmp_widget = lookup_widget( setup_window, "contact_email" );
	if ( contact_email)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_email );

	tmp_widget = lookup_widget( setup_window, "contact_home" );
	if ( contact_home)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_home );

	tmp_widget = lookup_widget( setup_window, "contact_work" );
	if ( contact_work)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_work );

	tmp_widget = lookup_widget( setup_window, "contact_mobile" );
	if ( contact_mobile)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), contact_mobile );


	tmp_widget = lookup_widget( setup_window, "file_transfer_server" );
	if ( file_transfer_server)
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), file_transfer_server );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "filetransfer.msg.yahoo.com" );


	if ( limit_lfs ) {
		tmp_widget = lookup_widget( setup_window, "limit_lfs" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "limit_lfs_count" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)limit_lfs );
	} else {
		tmp_widget = lookup_widget( setup_window, "limit_lfs" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );
		  }

	tmp_widget = lookup_widget( setup_window, "transparent_bg" );
#ifdef USE_GTK2
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), transparent_bg );
#else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#endif

#ifdef TRANS_BG
	if ( ! transparent_bg ) {
		shading_r = shading_g = shading_b = 0;
	}
#endif

	tmp_widget = lookup_widget( setup_window, "shading_r" );
#ifdef USE_GTK2
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
	            (float)shading_r );
#else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#endif

	tmp_widget = lookup_widget( setup_window, "shading_g" );
#ifdef USE_GTK2
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
	            (float)shading_g );
#else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#endif

	tmp_widget = lookup_widget( setup_window, "shading_b" );
#ifdef USE_GTK2
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
	            (float)shading_b );
#else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#endif

	tmp_widget = lookup_widget( setup_window, "use_buddy_list_font" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), use_buddy_list_font );

	tmp_widget = lookup_widget( setup_window, "style_bold" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), use_bold );

	tmp_widget = lookup_widget( setup_window, "style_underline" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), use_underline );

	tmp_widget = lookup_widget( setup_window, "style_italic" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), use_italics );

	if ( buffer_size ) {
		tmp_widget = lookup_widget( setup_window, "limit_buffer_size" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "buffer_size" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)buffer_size );
	} else {
		tmp_widget = lookup_widget( setup_window, "limit_buffer_size" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );
		  }

	if ( ping_minutes ) {
		tmp_widget = lookup_widget( setup_window, "use_ping_time" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "ping_minutes" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)ping_minutes );
	}  else {
		tmp_widget = lookup_widget( setup_window, "use_ping_time" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0);

		  }

	if ( history_limit ) {
		tmp_widget = lookup_widget( setup_window, "use_history_limit" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "history_limit" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)history_limit );
	} else {
		tmp_widget = lookup_widget( setup_window, "history_limit" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)20 );

		tmp_widget = lookup_widget( setup_window, "use_history_limit" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );

	}

	tmp_widget = lookup_widget( setup_window, "allow_pm_buzzes" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), allow_pm_buzzes );

	tmp_widget = lookup_widget( setup_window, "force_lowercase" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), force_lowercase );

	tmp_widget = lookup_widget( setup_window, "suppress_mult" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), suppress_mult );

	tmp_widget = lookup_widget( setup_window, "ignore_guests" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), ignore_guests );

	tmp_widget = lookup_widget( setup_window, "auto_reconnect" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_reconnect );

	tmp_widget = lookup_widget( setup_window, "show_colors" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_colors );

	tmp_widget = lookup_widget( setup_window, "show_quick_access_toolbar" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_quick_access_toolbar );

	tmp_widget = lookup_widget( setup_window, "show_fonts" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_fonts );

	tmp_widget = lookup_widget( setup_window, "show_html" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_html );

	tmp_widget = lookup_widget( setup_window, "show_statuses" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_statuses );

	tmp_widget = lookup_widget( setup_window, "show_enters" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_enters );

	if (profile_viewer==2) {
		tmp_widget = lookup_widget( setup_window, "profile_viewer2" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),TRUE );
	}
	else if (profile_viewer==1) {
		tmp_widget = lookup_widget( setup_window, "profile_viewer" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),TRUE );
	} else {
		tmp_widget = lookup_widget( setup_window, "profile_viewer3" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),TRUE );
	}

	if ( ignore_on_mults ) {
		tmp_widget = lookup_widget( setup_window, "ignore_on_mults" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "ignore_on_mults_count" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)ignore_on_mults );
	} else {
		tmp_widget = lookup_widget( setup_window, "ignore_on_mults" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );

		tmp_widget = lookup_widget( setup_window, "ignore_on_mults_count" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			0.0 );
	}

	if ( mute_on_mults ) {
		tmp_widget = lookup_widget( setup_window, "mute_on_mults" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 1 );

		tmp_widget = lookup_widget( setup_window, "mute_on_mults_count" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)mute_on_mults );
	} else {
		tmp_widget = lookup_widget( setup_window, "mute_on_mults" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), 0 );

		tmp_widget = lookup_widget( setup_window, "mute_on_mults_count" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			0.0 );
	}

	tmp_widget = lookup_widget( setup_window, "first_post_is_url" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), first_post_is_url );

	tmp_widget = lookup_widget( setup_window, "first_post_is_pm" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), first_post_is_pm );

	tmp_widget = lookup_widget( setup_window, "url_from_nonroom_user" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), url_from_nonroom_user );

	tmp_widget = lookup_widget( setup_window, "pm_in_sep_windows" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_in_sep_windows );

	tmp_widget = lookup_widget( setup_window, "pm_from_friends" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_from_friends );

	tmp_widget = lookup_widget( setup_window, "pm_from_users" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_from_users );

	tmp_widget = lookup_widget( setup_window, "pm_from_all" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_from_all );

	tmp_widget = lookup_widget( setup_window, "enable_encryption" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_encryption );

	tmp_widget = lookup_widget( setup_window, "locale_status" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), locale_status);

	tmp_widget = lookup_widget( setup_window, "show_my_status_in_chat" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_my_status_in_chat);

	tmp_widget = lookup_widget( setup_window, "pm_nick_names" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_nick_names);

	tmp_widget = lookup_widget( setup_window, "show_adult" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_adult );


	tmp_widget = lookup_widget( setup_window, "enter_leave_timestamp" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enter_leave_timestamp );

	tmp_widget = lookup_widget( setup_window, "enable_webcam_features" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_webcam_features );

	tmp_widget = lookup_widget( setup_window, "enable_tuxvironments" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enable_tuxvironments );


	tmp_widget = lookup_widget( setup_window, "enable_chat_spam_filter" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enable_chat_spam_filter);

	if (enable_chat_spam_filter) {
		tmp_widget = lookup_widget( setup_window, "ignore_guests" );
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_chat_spam_filter);
		gtk_widget_set_sensitive( tmp_widget, 0 );
		}

	tmp_widget = lookup_widget( setup_window, "enable_trayicon" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_trayicon );

	tmp_widget = lookup_widget( setup_window, "esd_sound_events" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_sound_events );

	tmp_widget = lookup_widget( setup_window, "esd_sound_events_pm" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_sound_events_pm );

	tmp_widget = lookup_widget( setup_window, "highlight_friends_chat" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), highlight_friends_in_chat);

	tmp_widget = lookup_widget( setup_window, "highlight_me_chat" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), highlight_me_in_chat );

	tmp_widget = lookup_widget( setup_window, "mute_noage" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), mute_noage );
	tmp_widget = lookup_widget( setup_window, "mute_minors" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), mute_minors );
	tmp_widget = lookup_widget( setup_window, "mute_nogender" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), mute_nogender );
	tmp_widget = lookup_widget( setup_window, "mute_males" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), mute_males );
	tmp_widget = lookup_widget( setup_window, "mute_females" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), mute_females );


	tmp_widget = lookup_widget( setup_window, "py_voice_helper" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),allow_py_voice_helper );

	tmp_widget = lookup_widget( setup_window, "allow_animations" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enable_animations );

	tmp_widget = lookup_widget( setup_window, "basic_boot_protect" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enable_basic_protection);

	tmp_widget = lookup_widget( setup_window, "preempt_boot_protection" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),enable_preemptive_strike);

	tmp_widget = lookup_widget( setup_window, "never_ignore_friend" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), never_ignore_a_friend);

	tmp_widget = lookup_widget( setup_window, "no_sent_files" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),allow_no_sent_files);

	tmp_widget = lookup_widget( setup_window, "no_random_friend_add" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),disallow_random_friend_add);

		tmp_widget = lookup_widget( setup_window, "max_simult_animations" );
		gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
			(float)max_sim_animations );

	tmp_widget = lookup_widget( setup_window, "popup_buzz" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),popup_buzz);
	tmp_widget = lookup_widget( setup_window, "popup_buddy_on" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),popup_buddy_on);
	tmp_widget = lookup_widget( setup_window, "popup_buddy_off" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),popup_buddy_off);
	tmp_widget = lookup_widget( setup_window, "popup_new_mail" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),popup_new_mail);
	tmp_widget = lookup_widget( setup_window, "popup_news_bulletin" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),popup_news_bulletin);


	  tmp_widget = lookup_widget( setup_window, "show_yavatars"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_yavatars );
	  tmp_widget = lookup_widget( setup_window,  "enable_audibles"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_audibles );
	  tmp_widget = lookup_widget( setup_window,  "activate_profnames"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), activate_profnames);
	  tmp_widget = lookup_widget( setup_window, "show_bimages"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_bimages );
	  tmp_widget = lookup_widget( setup_window,  "auto_reject_invitations"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), auto_reject_invitations );
	  tmp_widget = lookup_widget( setup_window, "pmb_audibles"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pmb_audibles );
	  tmp_widget = lookup_widget( setup_window, "pmb_smileys"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pmb_smileys);
	  tmp_widget = lookup_widget( setup_window,  "pmb_send_erase"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pmb_send_erase );

	  tmp_widget = lookup_widget( setup_window, "pmb_toolbar1"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pmb_toolbar1);
	  tmp_widget = lookup_widget( setup_window, "pmb_toolbar2"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pmb_toolbar2);
	  tmp_widget = lookup_widget( setup_window, "pm_window_style"); 
	  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), pm_window_style);

	  tmp_widget = lookup_widget( setup_window, "emulate_ymsg6"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), emulate_ymsg6 );

	  tmp_widget = lookup_widget( setup_window, "enable_addressbook"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_addressbook );

	  tmp_widget = lookup_widget( setup_window, "enable_pm_searches"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), enable_pm_searches );

	  tmp_widget = lookup_widget( setup_window, "chatwin_smileys"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), chatwin_smileys );
	  tmp_widget = lookup_widget( setup_window, "chatwin_send_erase"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget),  chatwin_send_erase );
	  tmp_widget = lookup_widget( setup_window, "show_tabs_at_bottom"); 
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_tabs_at_bottom );

	  tmp_widget = lookup_widget( setup_window,  "flash_player_cmd"); 
	  if (flash_player_cmd) {gtk_entry_set_text(GTK_ENTRY(tmp_widget), flash_player_cmd ); }

	  tmp_widget = lookup_widget( setup_window, "mp3_player"); 
	  if (mp3_player) {gtk_entry_set_text(GTK_ENTRY(tmp_widget), mp3_player); }


	tmp_widget = lookup_widget( setup_window, "show_avatars" );
#ifdef OS_WINDOWS
#	ifdef USE_GTK2
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_avatars );
#	else
	gtk_widget_set_sensitive( tmp_widget, 0 );
#	endif
#else
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_avatars );
#endif

	tmp_widget = lookup_widget( setup_window, "send_avatar" );
	if ( send_avatar )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), send_avatar );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );

	tmp_widget = lookup_widget( setup_window, "show_emoticons" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), show_emoticons );

	tmp_widget = lookup_widget( setup_window, "support_scentral_smileys" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), support_scentral_smileys );


	tmp_widget = lookup_widget( setup_window, "use_proxy" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), use_proxy );

	tmp_widget = lookup_widget( setup_window, "proxy_host" );
	if ( proxy_host )
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), proxy_host );
	else
		gtk_entry_set_text(GTK_ENTRY(tmp_widget), "" );

	tmp_widget = lookup_widget( setup_window, "proxy_port" );
	gtk_spin_button_set_value( GTK_SPIN_BUTTON( tmp_widget ),
		(float)proxy_port );

	cur_color[0] = custom_color_red / 255.0;
	cur_color[1] = custom_color_green / 255.0;
	cur_color[2] = custom_color_blue / 255.0;
	cur_color[3] = 0;
	tmp_widget = lookup_widget( setup_window, "colorselection" );
	gtk_color_selection_set_color( GTK_COLOR_SELECTION(tmp_widget),
		cur_color );

	tmp_widget = lookup_widget( setup_window, "fontselection1" );
	gtk_font_selection_set_font_name(
		GTK_FONT_SELECTION(tmp_widget), font_name );

	tmp_widget = lookup_widget( setup_window, "fontselection2" );
	if (tmp_widget && display_font_name) {
		gtk_font_selection_set_font_name(
			GTK_FONT_SELECTION(tmp_widget), display_font_name );
	}

	/* setup current color */
	if ( ! use_color )
		use_color = strdup( "black" );

	tmp_widget = lookup_widget( setup_window, "color_black" );
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), FALSE);

	tmp_widget = lookup_widget( setup_window, "colorselection" );

	gtk_widget_set_sensitive( tmp_widget, 0 );

	tmp_widget = NULL;
	if ( ! strcmp( use_color, "black" )) {
		tmp_widget = lookup_widget( setup_window, "color_black" );
	} else if ( ! strcmp( use_color, "red" )) {
		tmp_widget = lookup_widget( setup_window, "color_red" );
	} else if ( ! strcmp( use_color, "blue" )) {
		tmp_widget = lookup_widget( setup_window, "color_blue" );
	} else if ( ! strcmp( use_color, "green" )) {
		tmp_widget = lookup_widget( setup_window, "color_green" );
	} else if ( ! strcmp( use_color, "yellow" )) {
		tmp_widget = lookup_widget( setup_window, "color_yellow" );
	} else if ( ! strcmp( use_color, "purple" )) {
		tmp_widget = lookup_widget( setup_window, "color_purple" );
	} else if ( ! strcmp( use_color, "cyan" )) {
		tmp_widget = lookup_widget( setup_window, "color_cyan" );
	} else if ( ! strcmp( use_color, "orange" )) {
		tmp_widget = lookup_widget( setup_window, "color_orange" );
	} else if ( ! strcmp( use_color, "gray" )) {
		tmp_widget = lookup_widget( setup_window, "color_gray" );
	} else if ( ! strcmp( use_color, "custom" )) {
		tmp_widget = lookup_widget( setup_window, "colorselection" );
		gtk_widget_set_sensitive( tmp_widget, 1 );
		tmp_widget = lookup_widget( setup_window, "color_custom" );
	} else {
		tmp_widget = lookup_widget( setup_window, "color_black" );
	}
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp_widget), TRUE);

	/* loop through and set lists */
	/* Yahoo! login list */
	tmp_widget = lookup_widget( setup_window, "login_list" );
	model=freeze_treeview(tmp_widget );
	this_item = login_list;
	while( this_item ) {	
		answer = strdup(this_item->data);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here);
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

	/* then the favroom list */
	tmp_widget = lookup_widget( setup_window, "favroom_list" );
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



	/* then the regex list */
	tmp_widget = lookup_widget( setup_window, "regex_list" );
	model=freeze_treeview(tmp_widget );
	this_item = regex_list;
	while( this_item ) {	
		answer = strdup(this_item->data);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_mute);
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



	/* then the ignore list */
	tmp_widget = lookup_widget( setup_window, "ignore_list" );
	model=freeze_treeview(tmp_widget );
	if ( ignore_hash ) {g_hash_table_foreach( ignore_hash, fill_in_ignore_list, model );}
	unfreeze_treeview(tmp_widget, model);
	set_basic_treeview_sorting(tmp_widget, GYLIST_TYPE_SINGLE);



	/* now the friend list */
	tmp_widget = lookup_widget( setup_window, "friend_list" );
	model=freeze_treeview(tmp_widget );
	this_item = friend_list;
	while( this_item ) {	
		answer = strdup(this_item->data);
		gtk_list_store_append(GTK_LIST_STORE(model), &iter);
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here_male);
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


}

void get_setup_options() {
	gboolean valid;
	GtkTreeIter iter;
	GtkTreeModel *model;
	char *answer=NULL;
	gchar *answerin;
	GtkWidget *tmp_widget;
	char tmp_filename[256];
	char *ptr;
	FILE *fp;
	gdouble cur_color[4];

	// DBG( 11, "get_setup_options()\n" );

	/* gather & save the settings */
	tmp_widget = lookup_widget( setup_window, "auto_login" );
	auto_login = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "blended_colors" );
	show_blended_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "packet_debugging" );
	ymsg_sess->debug_packets = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "auto_away" );
	auto_away = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( auto_away ) {
		tmp_widget = lookup_widget( setup_window, "auto_away_time" );
		auto_away = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "custom_away_message" );
	if ( custom_away_message ) {
		free( custom_away_message );
		custom_away_message = NULL;
	}
	custom_away_message = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	tmp_widget = lookup_widget( setup_window, "browser_command" );
	if ( browser_command ) {
		free( browser_command );
		browser_command = NULL;
	}
	browser_command = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	if ( filter_command ) {
		free( filter_command );
		filter_command = NULL;
	}

	tmp_widget = lookup_widget( setup_window, "filter_command" );
	filter_command = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));
	if ( ! strcmp( filter_command, "" )) {
		free( filter_command );
		filter_command = NULL;
	}


	if ( webcam_viewer_app) {
		free(webcam_viewer_app);
		webcam_viewer_app = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "webcam_viewer_app" );
	webcam_viewer_app = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));


	if ( webcam_device) {
		free(webcam_device);
		webcam_device = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "webcam_device" );
	webcam_device = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	if ( custom_scale_text) {
		free(custom_scale_text);
		custom_scale_text = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "custom_scale_text" );
	custom_scale_text = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));


	if ( contact_first) {
		free(contact_first);
		contact_first = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_first" );
	contact_first = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	if ( contact_last) {
		free(contact_last);
		contact_last = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_last" );
	contact_last = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	if ( contact_email) {
		free(contact_email);
		contact_email = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_email" );
	contact_email = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	if ( contact_home) {
		free(contact_home);
		contact_home = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_home" );
	contact_home = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));


	if ( contact_work) {
		free(contact_work);
		contact_work = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_work" );
	contact_work = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));


	if ( contact_mobile) {
		free(contact_mobile);
		contact_mobile = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "contact_mobile" );
	contact_mobile = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));



	
	tmp_widget = lookup_widget( setup_window, "pm_brings_back" );
	pm_brings_back = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "disp_auto_ignored" );
	disp_auto_ignored = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "remember_position" );
	remember_position = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "remember_password" );
	remember_password = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "limit_lfs" );
	limit_lfs = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( limit_lfs ) {
		tmp_widget = lookup_widget( setup_window, "limit_lfs_count" );
		limit_lfs = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "transparent_bg" );
#ifdef TRANS_BG
	transparent_bg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( transparent_bg ) {
		tmp_widget = lookup_widget( setup_window, "shading_r" );
		shading_r = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));

		tmp_widget = lookup_widget( setup_window, "shading_g" );
		shading_g = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));

		tmp_widget = lookup_widget( setup_window, "shading_b" );
		shading_b = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	} else {
		shading_r = shading_g = shading_b = 0;
	}
#endif

	tmp_widget = lookup_widget( setup_window, "use_buddy_list_font" );
	use_buddy_list_font = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "auto_raise_pm" );
	auto_raise_pm = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "highlight_friends" );
	highlight_friends = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "auto_close_roomlist" );
	auto_close_roomlist = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "cache_room_list" );
	cache_room_list = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "chat_timestamp" );
	chat_timestamp = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "pm_nick_names" );
	pm_nick_names = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_my_status_in_chat" );
	show_my_status_in_chat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "indent_wrapped" );
	indent_wrapped = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_quick_access_toolbar" );
	show_quick_access_toolbar = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "chat_timestamp_pm" );
	chat_timestamp_pm = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "auto_reply_when_away" );
	auto_reply_when_away = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( auto_reply_msg ) {
		free( auto_reply_msg );
		auto_reply_msg = NULL;
	}

	if ( auto_reply_when_away ) {
		tmp_widget = lookup_widget( setup_window, "auto_reply_msg" );
		auto_reply_msg = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));
	}


	if ( file_transfer_server ) {
		free( file_transfer_server);
		file_transfer_server = NULL;
	}
	tmp_widget = lookup_widget( setup_window, "file_transfer_server" );
	file_transfer_server= strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	tmp_widget = lookup_widget( setup_window, "allow_pm_buzzes" );
	allow_pm_buzzes = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));


	tmp_widget = lookup_widget( setup_window, "style_bold" );
	use_bold = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "style_underline" );
	use_underline = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "style_italic" );
	use_italics = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "limit_buffer_size" );
	buffer_size = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( buffer_size ) {
		tmp_widget = lookup_widget( setup_window, "buffer_size" );
		buffer_size = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "use_history_limit" );
	history_limit = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( history_limit ) {
		tmp_widget = lookup_widget( setup_window, "history_limit" );
		history_limit = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "use_ping_time" );
	ping_minutes = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( ping_minutes ) {
		tmp_widget = lookup_widget( setup_window, "ping_minutes" );
		ping_minutes = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "force_lowercase" );
	force_lowercase = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "suppress_mult" );
	suppress_mult = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "ignore_guests" );
	ignore_guests = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "auto_reconnect" );
	auto_reconnect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_colors" );
	show_colors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_fonts" );
	show_fonts = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_html" );
	show_html = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_statuses" );
	show_statuses = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_enters" );
	show_enters = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_pm_searches" );
	enable_pm_searches = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_addressbook" );
	enable_addressbook = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	  tmp_widget = lookup_widget( setup_window, "show_yavatars"); 
	  show_yavatars = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "enable_audibles"); 
	  enable_audibles = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "activate_profnames"); 
	  activate_profnames = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window, "show_bimages"); 
	  show_bimages = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "auto_reject_invitations"); 
	  auto_reject_invitations = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window, "pmb_audibles"); 
	  pmb_audibles = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window, "pmb_smileys"); 
	  pmb_smileys = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "pmb_send_erase"); 
	  pmb_send_erase = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	  tmp_widget = lookup_widget( setup_window,  "pm_window_style"); 
	  pm_window_style = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "pmb_toolbar1"); 
	  pmb_toolbar1 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "pmb_toolbar2"); 
	  pmb_toolbar2 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	  tmp_widget = lookup_widget( setup_window,  "show_tabs_at_bottom"); 
	  show_tabs_at_bottom = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "chatwin_send_erase"); 
	  chatwin_send_erase = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	  tmp_widget = lookup_widget( setup_window,  "chatwin_smileys"); 
	  chatwin_smileys = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	  tmp_widget = lookup_widget( setup_window,  "flash_player_cmd"); 
	  	if ( flash_player_cmd) {
		free(flash_player_cmd);
		flash_player_cmd = NULL;
		}
	  flash_player_cmd = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	  tmp_widget = lookup_widget( setup_window, "mp3_player"); 
	  	if ( mp3_player) {
		free(mp3_player);
		mp3_player = NULL;
		}
	mp3_player = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	  tmp_widget = lookup_widget( setup_window, "emulate_ymsg6"); 
	 emulate_ymsg6 = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	profile_viewer = 0;

	tmp_widget = lookup_widget( setup_window, "profile_viewer" );
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget))) {
		profile_viewer = 1;
	}

	tmp_widget = lookup_widget( setup_window, "profile_viewer2" );
	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget))) {
		profile_viewer = 2;
	}
	tmp_widget = lookup_widget( setup_window, "ignore_on_mults" );
	ignore_on_mults = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( ignore_on_mults ) {
		tmp_widget = lookup_widget( setup_window, "ignore_on_mults_count" );
		ignore_on_mults = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "mute_on_mults" );
	mute_on_mults = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	if ( mute_on_mults ) {
		tmp_widget = lookup_widget( setup_window, "mute_on_mults_count" );
		mute_on_mults = gtk_spin_button_get_value_as_int(
			GTK_SPIN_BUTTON( tmp_widget ));
	}

	tmp_widget = lookup_widget( setup_window, "first_post_is_url" );
	first_post_is_url = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "first_post_is_pm" );
	first_post_is_pm = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "url_from_nonroom_user" );
	url_from_nonroom_user = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "pm_in_sep_windows" );
	pm_in_sep_windows = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "pm_from_friends" );
	pm_from_friends = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "pm_from_users" );
	pm_from_users = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "pm_from_all" );
	pm_from_all = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_encryption" );
	enable_encryption = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "show_adult" );
	show_adult = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "locale_status" );
	locale_status = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_trayicon" );
	enable_trayicon = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	if (enable_trayicon) {trayicon_create ();}
	else {trayicon_destroy ();}


	tmp_widget = lookup_widget( setup_window, "esd_sound_events" );
	enable_sound_events = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "esd_sound_events_pm" );
	enable_sound_events_pm = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_webcam_features" );
	enable_webcam_features = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enter_leave_timestamp" );
	enter_leave_timestamp = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_tuxvironments" );
	enable_tuxvironments = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "popup_buzz" );
	popup_buzz = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "popup_buddy_on" );
	popup_buddy_on = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "popup_buddy_off" );
	popup_buddy_off = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "popup_new_mail" );
	popup_new_mail = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "popup_news_bulletin" );
	popup_news_bulletin = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "enable_chat_spam_filter" );
	enable_chat_spam_filter = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "mute_noage" );
	mute_noage = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "mute_minors" );
	mute_minors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "mute_nogender" );
	mute_nogender = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "mute_males" );
	mute_males = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "mute_females" );
	mute_females = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "py_voice_helper" );
	allow_py_voice_helper  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "allow_animations" );
	enable_animations = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "basic_boot_protect" );
	enable_basic_protection = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "preempt_boot_protection" );
	enable_preemptive_strike = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "never_ignore_friend" );
	never_ignore_a_friend = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "no_sent_files" );
	allow_no_sent_files = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "no_random_friend_add" );
	disallow_random_friend_add = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "highlight_friends_chat" );
	highlight_friends_in_chat = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	tmp_widget = lookup_widget( setup_window, "highlight_me_chat" );
	highlight_me_in_chat= gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));


	tmp_widget = lookup_widget( setup_window, "max_simult_animations" );
	max_sim_animations  = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON( tmp_widget ));


	tmp_widget = lookup_widget( setup_window, "show_avatars" );
	show_avatars = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "send_avatar" );
	if ( send_avatar ) {
		free( send_avatar );
		send_avatar = NULL;
	}
	send_avatar = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));
	if ( ! strlen( send_avatar )) {
		free( send_avatar );
		send_avatar = NULL;
	}

	tmp_widget = lookup_widget( setup_window, "show_emoticons" );
	show_emoticons = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "support_scentral_smileys" );
	support_scentral_smileys = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "use_proxy" );
	use_proxy = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));

	tmp_widget = lookup_widget( setup_window, "proxy_host" );
	if ( proxy_host ) {
		free( proxy_host );
		proxy_host = NULL;
	}
	proxy_host = strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget)));

	tmp_widget = lookup_widget( setup_window, "proxy_port" );
	proxy_port = gtk_spin_button_get_value_as_int(
		GTK_SPIN_BUTTON( tmp_widget ));

	tmp_widget = lookup_widget( setup_window, "colorselection" );
	gtk_color_selection_get_color( GTK_COLOR_SELECTION(tmp_widget),
		cur_color );
	custom_color_red = cur_color[0] * 255;
	custom_color_green = cur_color[1] * 255;
	custom_color_blue = cur_color[2] * 255;

	tmp_widget = lookup_widget( setup_window, "fontselection1" );
	if (font_name) { free( font_name ); }
	if ( font_family ) {free( font_family ); }
	font_family = NULL;
	font_name = strdup( gtk_font_selection_get_font_name(
		GTK_FONT_SELECTION(tmp_widget)));
	font_family = strdup( font_name );

	ptr=strrchr(font_family, ' ');

	/* This was messy! */  /* 
	ptr = font_family + strlen( font_family ) - 1;
	while( *ptr != ' ' ) {ptr--;}
	ptr++;
	*/ 


	font_size = atoi( ptr?ptr+1:"12" );
	if (ptr) {*ptr = '\0';}

	if ( strstr( font_name, " Bold" )) {	use_bold = 1;} 
	else {	use_bold = 0;	}

	if ( strstr( font_name, " Italic" )) {use_italics = 1;} 
	else {	use_italics = 0;}  

	ptr=strstr( font_family, " Italic" );
	if (ptr) {*ptr = '\0';}
	ptr=strstr( font_family, " Bold" );
	if (ptr) {*ptr = '\0';}
	ptr=strstr( font_family, "," );
	if (ptr) {*ptr = '\0';}
	ptr=strstr( font_family, " Medium" );
	if (ptr) {*ptr = '\0';}

	ptr=map_font_family(font_family);
	free(font_family);
	font_family=strdup(ptr);

	/* begin - OLD CODE */  /*  
	ptr = font_family;
	while(( *ptr ) && ( *ptr != ' ' )) {
		ptr++;
	}
	*ptr = '\0';
	if ( strstr( font_name, "Bold" )) {
		use_bold = 1;
	} else {
		use_bold = 0;
	}

	if ( strstr( font_name, "Italic" )) {
		use_italics = 1;
	} else {
		use_italics = 0;
	}  

	*/ /* end- OLD CODE */ 


	tmp_widget = lookup_widget( setup_window, "fontselection2" );
	if (tmp_widget ) {
		if (display_font_name) { free( display_font_name ); }
		if ( display_font_family ) {free( display_font_family ); }
		display_font_family = NULL;
		display_font_name = strdup( gtk_font_selection_get_font_name(
			GTK_FONT_SELECTION(tmp_widget)));
		display_font_family = strdup( display_font_name );

		ptr=strrchr(display_font_family, ' ');
		display_font_size = atoi( ptr?ptr+1:"12" );
		if (ptr) {*ptr = '\0';}
		ptr=strstr( display_font_family, " Italic" );
		if (ptr) {*ptr = '\0';}
		ptr=strstr( display_font_family, " Bold" );
		if (ptr) {*ptr = '\0';}
		ptr=strstr( display_font_family, "," );
		if (ptr) {*ptr = '\0';}
		ptr=strstr( display_font_family, " Medium" );
		if (ptr) {*ptr = '\0';}
		ptr=map_font_family(display_font_family);
		free(display_font_family);
		display_font_family=strdup(ptr);
	}




	/* get the user's color */
	free( use_color );
	use_color = NULL;
	if (( tmp_widget = lookup_widget( setup_window, "color_black" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "black" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_red" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "red" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_blue" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "blue" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_green" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "green" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_yellow" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "yellow" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_purple" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "purple" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_cyan" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "cyan" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_orange" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "orange" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_gray" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "gray" );
	} else if (( tmp_widget = lookup_widget( setup_window, "color_custom" )) &&
		( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget)) )) {
		use_color = strdup( "custom" );
	}

	if ( ! use_color )
		use_color = strdup( "black" );

	auto_away_time = time(NULL) + ( auto_away * 60 );

	/* now get and save the friends, regex, and ignore lists */
	tmp_widget = lookup_widget( setup_window, "login_list" );
		/* empty current login list */
	login_list = gyach_g_list_free( login_list );
		/* fetch new login list */

		model=gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget));
		valid = gtk_tree_model_get_iter_first(model, &iter);
		while(valid) {			
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &answerin, -1);			
			answer=strdup(answerin);
				login_list = g_list_append( login_list, strdup( answer));
			free(answer);
			g_free(answerin);
			valid = gtk_tree_model_iter_next(model, &iter);
		}


	tmp_widget = lookup_widget( setup_window, "favroom_list" );
	favroom_list = gyach_g_list_free( favroom_list );

	/* fetch new favroom list */
		model=gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget));
		valid = gtk_tree_model_get_iter_first(model, &iter);
		while(valid) {			
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &answerin, -1);			
			answer=strdup(answerin);
				favroom_list = g_list_append( favroom_list, strdup( answer ));
			free(answer);
			g_free(answerin);
			valid = gtk_tree_model_iter_next(model, &iter);
		}


	tmp_widget = lookup_widget( setup_window, "regex_list" );
	snprintf( tmp_filename, 254, "%s/ignore.regex", GYACH_CFG_DIR );
	fp = fopen( tmp_filename, "wb" );
		model=gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget));
		valid = gtk_tree_model_get_iter_first(model, &iter);

	if ( fp ) {
		while(valid) {			
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &answerin, -1);			
			answer=strdup(answerin);
				fprintf( fp, "%s\n", answer );
			free(answer);
			g_free(answerin);
			valid = gtk_tree_model_iter_next(model, &iter);
		}
		fclose( fp );
	}

	tmp_widget = lookup_widget( setup_window, "ignore_list" );

	snprintf( tmp_filename, 254,  "%s/ignore", GYACH_CFG_COMMON_DIR );
	fp = fopen( tmp_filename, "wb" );
		model=gtk_tree_view_get_model(GTK_TREE_VIEW(tmp_widget));
		valid = gtk_tree_model_get_iter_first(model, &iter);

	if ( fp ) {
		while(valid) {			
			gtk_tree_model_get(model, &iter,GYSINGLE_COL1, &answerin, -1);			
			answer=strdup(answerin);
				fprintf( fp, "%s\n", answer );
			free(answer);
			g_free(answerin);
			valid = gtk_tree_model_iter_next(model, &iter);
		}
		fclose( fp );
	}

	/* now load the files we just saved */
	ignore_load();

	/* set the options we just saved */
	set_config();
}


