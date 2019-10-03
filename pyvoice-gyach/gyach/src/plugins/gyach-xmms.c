/*****************************************************************************
 * gyache-xmms.c, Plugin to use XMMS current song as status message.
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
 * Copyright (C) 2003-2006, Erica Andrews (Phrozensmoke ['a'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 *****************************************************************************/

#include "../plugin_api.h"
#include "../commands.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmsctrl.h>

int xmms_show_status_toggled=0;
int started_xmms_status_timeout=0;
char *last_xmms_song_status=NULL;
char xmms_send_status[224]="";

void reset_xmms_status_msg() {
   if (last_xmms_song_status) {free(last_xmms_song_status); last_xmms_song_status=NULL;}	
}

int adjust_xmms_status(guint data) {
	if (!xmms_show_status_toggled) {return 1;}

	/* We have our status set to some type of 'busy', so dont change */
	if ( (my_status>0) && (my_status<10) ) {reset_xmms_status_msg(); return 1;}

	if (my_status != 12) { /* as long as we aren't invisible */
		gchar *currsong=xmms_remote_get_playlist_title(0, xmms_remote_get_playlist_pos(0));
		if (!currsong) { /* No song playing */
			if (!last_xmms_song_status) {return 1;}
			if (!strcmp(last_xmms_song_status,"")) {return 1;}  /* Status already cleared */
			free(last_xmms_song_status);
			last_xmms_song_status=strdup("");
			cmd_away( "0" );
			return 1;
		} else { /* a song is playing */
			if (last_xmms_song_status) {
				if (!strcmp(last_xmms_song_status,currsong)) {return 1;}  /* Same song */
				free(last_xmms_song_status);
			}			
			last_xmms_song_status=strdup(currsong);
			snprintf(xmms_send_status, emulate_ymsg6?220:61, "XMMS: '%s", currsong);
			strcat(xmms_send_status,  "'");
			show_radio_is_on=1;
			cmd_away(xmms_send_status);
			show_radio_is_on=0;
			return 1;
		}
	} else {reset_xmms_status_msg();}
	return 1;
}


void start_xmms_status_timeout() {gtk_timeout_add( 6500,
				(void *)adjust_xmms_status, NULL ); started_xmms_status_timeout=1;}

void on_xmms_show_status_toggled  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (!menuitem) {return;}
	if (GTK_CHECK_MENU_ITEM(menuitem)->active) {
		 xmms_show_status_toggled=1;
		 adjust_xmms_status(1); /* update immediately */
		 if (! started_xmms_status_timeout) {start_xmms_status_timeout();}
																				} else {
					xmms_show_status_toggled=0;
					reset_xmms_status_msg(); 
					if (my_status==10) {cmd_away( "0" );}
																						  }
}



int my_init();

PLUGIN_INFO plugin_info = {
	PLUGIN_OTHER, 
	"GyachE-XMMS",
	"A plugin to show the current XMMS song as your status message when you are not 'Busy' or Invisible.", 
	"0.4", 
	"11/28/2004",
	"XMMS 1.2.7 or better",
	"Erica Andrews [PhrozenSmoke ('at') yahoo.com]", my_init};



int my_init() {
 	GtkWidget *mystatmenu;
	mystatmenu=gtk_object_get_data (GTK_OBJECT (chat_window), "status_menu");
	if (mystatmenu)  {
		GtkWidget *xmmseparator;
		GtkWidget *xmms_status_check_button;
  		xmmseparator = gtk_menu_item_new ();
  		gtk_container_add (GTK_CONTAINER (mystatmenu), xmmseparator);
  		gtk_widget_show (xmmseparator);
  		gtk_widget_set_sensitive (xmmseparator, FALSE);

		xmms_status_check_button=gtk_check_menu_item_new_with_label (_("XMMS: What's Playing?"));
  		gtk_container_add (GTK_CONTAINER (mystatmenu), xmms_status_check_button);
  		gtk_widget_show (xmms_status_check_button);

  gtk_signal_connect (GTK_OBJECT (xmms_status_check_button), "toggled",
                      GTK_SIGNAL_FUNC (on_xmms_show_status_toggled),
                      NULL);
		return 1;
	}
return 0;
}


