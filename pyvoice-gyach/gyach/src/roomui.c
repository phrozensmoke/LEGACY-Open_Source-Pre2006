/*****************************************************************************
 * roomui.c
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
/*
	exported by PhrozenSmoke from interface.c, to reduce monstrous size of 
	interface.c (can cause 'out of memory' errors while compiling)...
	all room list, create a room, and room 'favorites' UI functions
*/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"
#include <malloc.h>
#include "util.h"
#include "profname.h"
#include "gytreeview.h"

extern void set_tooltip(GtkWidget *somewid, char *somechar);
extern GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id);
extern void on_addfavroom_insert_activate  (GtkMenuItem  *menuitem, gpointer  user_data);
extern char *room_list_country_code;

  GList *roomintllist = NULL;

GtkWidget *get_intl_room_bar(GtkWidget *room_list)  {
	GtkWidget *hbox12, *intlcombo, *reload_room_list;

  hbox12 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox12), gtk_label_new(_("Location")), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox12), gtk_label_new(":  "), FALSE, FALSE, 0);

	if (!roomintllist) {
	roomintllist = g_list_append( roomintllist, "us" );
	roomintllist = g_list_append( roomintllist, "ar" );
	roomintllist = g_list_append( roomintllist, "au" );
	roomintllist = g_list_append( roomintllist, "br" );
	roomintllist = g_list_append( roomintllist, "ca" );
	roomintllist = g_list_append( roomintllist, "cf" );
	roomintllist = g_list_append( roomintllist, "cn" );
	roomintllist = g_list_append( roomintllist, "ct" );
	roomintllist = g_list_append( roomintllist, "de" );
	roomintllist = g_list_append( roomintllist, "dk" );
	roomintllist = g_list_append( roomintllist, "e1" );
	roomintllist = g_list_append( roomintllist, "es" );
	roomintllist = g_list_append( roomintllist, "fr" );
	roomintllist = g_list_append( roomintllist, "hk" );
	roomintllist = g_list_append( roomintllist, "in" );
	roomintllist = g_list_append( roomintllist, "kr" );
	roomintllist = g_list_append( roomintllist, "it" );
	roomintllist = g_list_append( roomintllist, "mx" );
	roomintllist = g_list_append( roomintllist, "no" );
	roomintllist = g_list_append( roomintllist, "se" );
	roomintllist = g_list_append( roomintllist, "sg" );
	roomintllist = g_list_append( roomintllist, "tw" );
	roomintllist = g_list_append( roomintllist, "uk" );
	}

  intlcombo=gtk_combo_new();
  set_tooltip(intlcombo,_("Location"));
  gtk_object_set_data (GTK_OBJECT (room_list), "intlcombo",intlcombo);
  gtk_combo_set_popdown_strings( GTK_COMBO(intlcombo), roomintllist );
  gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(intlcombo)->entry),FALSE );
  gtk_box_pack_start (GTK_BOX (hbox12), intlcombo, FALSE, FALSE, 2);

  if (room_list_country_code) {
	char *intls=strdup(room_list_country_code);
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(intlcombo)->entry), intls);
	free(intls);
								 } else {
	char *intls=strdup("us");
	gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(intlcombo)->entry), intls);
	free(intls);
										  }

  reload_room_list = get_pixmapped_button(_(" Reload "), GTK_STOCK_REFRESH);
  set_tooltip(reload_room_list,_(" Reload "));
  gtk_object_set_data (GTK_OBJECT (room_list), "reload_room_list",reload_room_list);
  gtk_box_pack_start (GTK_BOX (hbox12), reload_room_list, TRUE, TRUE, 3);

  gtk_widget_show_all (hbox12);
  return hbox12;
}

GtkWidget* create_room_list (void)
{
  GtkWidget *room_list, *profwidget;
  GtkWidget *vbox10;
  GtkWidget *scrolledwindow6;
  GtkWidget *room_tree;
  /* GtkWidget *loading_status; */ 
  GtkWidget *hbox11;
  GtkWidget *goto_room;
  GtkWidget *cancel_room;
  GtkWidget *reload_room_list;
  GtkWidget *hbox12;
  char *col_headers[]={"",NULL};

  room_list = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (room_list), "gyachERooms", "GyachE");
  gtk_window_set_position (GTK_WINDOW (room_list), GTK_WIN_POS_CENTER);
  gtk_object_set_data (GTK_OBJECT (room_list), "room_list", room_list);
  gtk_window_set_title (GTK_WINDOW (room_list), _("Yahoo! Room List"));
  gtk_window_set_default_size (GTK_WINDOW (room_list), 515, 350);

  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (room_list), vbox10);
  gtk_widget_show (vbox10);
  gtk_container_set_border_width(GTK_CONTAINER(vbox10),2);

  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow6);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "scrolledwindow6", scrolledwindow6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox10), scrolledwindow6, TRUE, TRUE, 2);
  gtk_widget_show (scrolledwindow6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_SHADOW_ETCHED_IN);

  room_tree = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_TREE,GYLIST_TYPE_ROOM, 0, 1, col_headers));
  gtk_widget_ref (room_tree);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "room_tree", room_tree,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), room_tree);
  gtk_widget_show (room_tree);

  /* loading_status = gtk_label_new (_("Loading Room List..."));
  gtk_widget_ref (loading_status);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "loading_status", loading_status,
                            (GtkDestroyNotify) gtk_widget_unref);
    gtk_box_pack_start (GTK_BOX (vbox10), loading_status, FALSE, FALSE, 3);
  gtk_widget_show (loading_status); */

  hbox12=get_intl_room_bar(room_list);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox12, FALSE, FALSE, 2);
  reload_room_list=gtk_object_get_data(GTK_OBJECT(room_list),"reload_room_list");

  profwidget=get_profile_name_component(1);
  gtk_box_pack_start (GTK_BOX (vbox10), profwidget, FALSE, FALSE, 2);

  hbox11 = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox11, FALSE, TRUE, 2);
  gtk_widget_show (hbox11);
  gtk_container_set_border_width(GTK_CONTAINER(hbox11),3);
  gtk_container_set_border_width(GTK_CONTAINER(hbox12),5);

  goto_room = get_pixmapped_button(_(" Join Room "), GTK_STOCK_APPLY);
  gtk_object_set_data(GTK_OBJECT(goto_room),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(room_tree),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(room_list),"profcombo" ,gtk_object_get_data(GTK_OBJECT(profwidget), "profcombo") );

  set_tooltip(goto_room,_(" Join Room "));
  gtk_widget_ref (goto_room);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "goto_room", goto_room,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox11), goto_room, TRUE, TRUE, 0);
  gtk_widget_show (goto_room);

  gtk_box_pack_start (GTK_BOX (hbox11), gtk_label_new(" "), TRUE, TRUE, 0);

  cancel_room = get_pixmapped_button(_(" Close "), GTK_STOCK_CANCEL);
set_tooltip(cancel_room,_(" Close "));
  gtk_widget_ref (cancel_room);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "cancel_room", cancel_room,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox11), cancel_room, TRUE, TRUE, 0);
  gtk_widget_show (cancel_room);

  gtk_signal_connect_after (GTK_OBJECT (room_list), "delete_event",
                            GTK_SIGNAL_FUNC (on_room_list_delete_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (room_tree), "button_press_event",
                      GTK_SIGNAL_FUNC (on_room_tree_button_press_event),
                      NULL);
  g_signal_connect (room_tree, "row-activated", (GCallback) onRoomListActivated,
                      NULL);
  g_signal_connect (room_tree, "popup-menu", (GCallback) on_room_tree_kb_popup, NULL);
  g_signal_connect_after (room_tree,  "row-expanded",
                            (GCallback)  on_room_tree_tree_expand,
                            NULL);
  gtk_signal_connect (GTK_OBJECT (goto_room), "clicked",
                      GTK_SIGNAL_FUNC (on_goto_room_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (reload_room_list), "clicked",
                      GTK_SIGNAL_FUNC (on_reload_room_list_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_room), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_room_clicked),
                      NULL);

  return room_list;
}






/* ********** ADDED: PhrozenSmoke  ****************************  */
/*  UI Code for creating a Yahoo! Room */


GtkWidget* create_room_maker (char *mname)
{
  GtkWidget *room_list;
  GtkWidget *vbox10;
  GtkWidget *scrolledwindow6;
  GtkWidget *room_tree;
  /* GtkWidget *loading_status; */
  GtkWidget *hbox11;  /* buttons */
  GtkWidget *reload_room_list;
  GtkWidget *catbox; 
  GtkWidget *catvbox; 

  GtkWidget *roombox; 
  GtkWidget *roomentry; 

  GtkWidget *messbox; 
  GtkWidget *messentry; 
  GtkWidget *hbox12;
  GtkWidget *achbox; 
  GtkWidget *acvbox1; 
  GtkWidget *acvbox2; 
  GtkWidget *access1; 
  GtkWidget *access2; 
  GtkWidget *access3; 
  GtkWidget *access4; 
  GSList *acgroup=NULL;
  GtkWidget *profwidget;
  GtkWidget *create_room;
  GtkWidget *cancel_room;
  char *col_headers[]={"",NULL};

  room_list = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (room_list), GTK_WIN_POS_CENTER);
  gtk_window_set_wmclass (GTK_WINDOW (room_list), "gyachECrRooms", "GyachE");
  gtk_object_set_data (GTK_OBJECT (room_list), "croom_list", room_list);
  gtk_object_set_data (GTK_OBJECT (room_list),"mywindow", room_list );
  gtk_window_set_title (GTK_WINDOW (room_list), _("Create Chat Room"));
  gtk_window_set_default_size (GTK_WINDOW (room_list), 415, 475);

  vbox10 = gtk_vbox_new (FALSE, 0);  /* Main vbox */
  gtk_container_add (GTK_CONTAINER (room_list), vbox10);
  gtk_widget_show (vbox10);

  profwidget=get_profile_name_component(1);
  gtk_box_pack_start (GTK_BOX (vbox10), profwidget, FALSE, FALSE, 2);


  /*  GtkWidget *catbox;    */
  catbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), catbox, TRUE, TRUE, 2);


  catvbox = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (catbox), catvbox, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (catvbox), gtk_label_new(_("Create in Category:")), FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (catvbox), gtk_label_new(" "), TRUE, TRUE, 6);
  gtk_container_set_border_width(GTK_CONTAINER(catvbox),  2);


  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow6);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "scrolledwindow6", scrolledwindow6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (catbox), scrolledwindow6, TRUE, TRUE, 1);
  gtk_widget_show (scrolledwindow6);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_SHADOW_ETCHED_IN);

  room_tree = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_TREE,GYLIST_TYPE_ROOM, 0, 1, col_headers));
  gtk_widget_ref (room_tree);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "croom_tree", room_tree,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), room_tree);
  gtk_widget_show (room_tree);

  hbox12=get_intl_room_bar(room_list);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox12, FALSE, FALSE, 2);
  reload_room_list=gtk_object_get_data(GTK_OBJECT(room_list),"reload_room_list");
  gtk_object_set_data(GTK_OBJECT(reload_room_list), "mywindow", room_list);

  /* loading_status = gtk_label_new ("  ");
  gtk_widget_ref (loading_status);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "cloading_status", loading_status,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (catbox), loading_status, FALSE, FALSE, 0);
  gtk_widget_show (loading_status); */

   roomentry=gtk_entry_new();
   gtk_entry_set_max_length(GTK_ENTRY(roomentry),30);

  roombox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), roombox, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (roombox), gtk_label_new(_("Room Name:")), FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (roombox),roomentry, TRUE, TRUE, 2);


   messentry=gtk_entry_new();
   gtk_entry_set_max_length(GTK_ENTRY(messentry),52);

  messbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), messbox, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (messbox), gtk_label_new(_("Room Topic:")), FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (messbox),messentry, TRUE, TRUE, 2);


  achbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), achbox, FALSE, FALSE, 2);
  acvbox1 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (achbox), acvbox1, FALSE, FALSE, 2);
  acvbox2 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (achbox), acvbox2, FALSE, FALSE, 2);

  gtk_box_pack_start (GTK_BOX (acvbox1), gtk_label_new(_("Access:")), FALSE, FALSE, 6);
  gtk_box_pack_start (GTK_BOX (acvbox1), gtk_label_new(" "), TRUE, TRUE, 2);

  access1=gtk_radio_button_new_with_label(acgroup,_("Public  (Room will appear in Room list for everyone to join)"));
  set_tooltip(access1,_("Public  (Room will appear in Room list for everyone to join)"));
  acgroup=gtk_radio_button_group(GTK_RADIO_BUTTON(access1) );
  access2=gtk_radio_button_new_with_label(acgroup,_("Private  (Room will not appear in Room list)"));
  set_tooltip(access2,_("Private  (Room will not appear in Room list)"));
  access3=gtk_check_button_new_with_label(_("Users can join the room only if I invite them"));
  set_tooltip(access3,_("Users can join the room only if I invite them"));
  access4=gtk_check_button_new_with_label(_("Enable Voice in Room"));
  set_tooltip(access4,_(" Voice Chat "));
  gtk_box_pack_start (GTK_BOX (acvbox2), access1, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (acvbox2), access2, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (acvbox2), access3, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (acvbox2), access4, FALSE, FALSE, 2);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(access1), TRUE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(access4), TRUE);

  gtk_container_set_border_width(GTK_CONTAINER(acvbox1),  2);
  gtk_container_set_border_width(GTK_CONTAINER(achbox),  3);


  /* button box */
  hbox11 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox11, FALSE, FALSE, 5);
  gtk_widget_show (hbox11);


  gtk_box_pack_start (GTK_BOX (hbox11), gtk_label_new(" "), TRUE, TRUE, 2);

  create_room = get_pixmapped_button(_(" Create Room "), GTK_STOCK_APPLY);
set_tooltip(create_room,_(" Create Room "));
  gtk_object_set_data(GTK_OBJECT(create_room),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(room_list),"profcombo" ,gtk_object_get_data(GTK_OBJECT(profwidget), "profcombo") );
  gtk_widget_ref (create_room);
  gtk_object_set_data_full (GTK_OBJECT (room_list), "ccreate_room", create_room,
                            (GtkDestroyNotify) gtk_widget_unref);

  gtk_object_set_data (GTK_OBJECT (create_room),"mywindow", room_list );
  gtk_object_set_data (GTK_OBJECT (create_room),"acccess1", access1);
  gtk_object_set_data (GTK_OBJECT (create_room),"acccess2", access2);
  gtk_object_set_data (GTK_OBJECT (create_room),"acccess3", access3);
  gtk_object_set_data (GTK_OBJECT (create_room),"acccess4", access4);
  gtk_object_set_data (GTK_OBJECT (create_room),"roomentry", roomentry);
  gtk_object_set_data (GTK_OBJECT (create_room),"messentry", messentry);
  gtk_object_set_data (GTK_OBJECT (create_room),"room_tree", room_tree);


  gtk_box_pack_start (GTK_BOX (hbox11), create_room, FALSE, FALSE, 3);
  gtk_widget_show (create_room);


  cancel_room =get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
set_tooltip(cancel_room,_(" Cancel "));
  gtk_object_set_data (GTK_OBJECT (cancel_room),"mywindow", room_list );
  gtk_box_pack_start (GTK_BOX (hbox11), cancel_room, FALSE, FALSE, 3);
  gtk_widget_show (cancel_room);


  gtk_container_set_border_width(GTK_CONTAINER(vbox10),  6);

  gtk_signal_connect_after (GTK_OBJECT (room_list), "delete_event",
                            GTK_SIGNAL_FUNC (on_cancel_croom_clickedw),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_room), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_croom_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (create_room), "clicked",
                      GTK_SIGNAL_FUNC (on_create_croom_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (reload_room_list), "clicked",
                      GTK_SIGNAL_FUNC (on_refresh_intl_croom_clicked),
                      NULL);


	/* call some methods here for loading list of room cats */

  gtk_widget_show_all (room_list);
  return room_list;
}




/* end UI Code for creating Yahoo Room */





GtkWidget* create_favorites (void)
{
  GtkWidget *favorites;
  GtkWidget *vbox6;
  GtkWidget *table2;
  GtkWidget *scrolledwindow11;
  GtkWidget *fav_room_list;
  GtkWidget *hbox7;
  GtkWidget *goto_fav;
  GtkWidget *cancel_favs;
  GtkWidget *profwidget;
  char *col_headers[]={"",NULL};

  favorites = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position (GTK_WINDOW (favorites), GTK_WIN_POS_CENTER);
  gtk_window_set_wmclass (GTK_WINDOW (favorites), "gyachEFavorites", "GyachE");
  gtk_object_set_data (GTK_OBJECT (favorites), "favorites", favorites);
  gtk_window_set_title (GTK_WINDOW (favorites), _("Favorite Yahoo! Rooms"));

  vbox6 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox6);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "vbox6", vbox6,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (favorites), vbox6);
  gtk_widget_show (vbox6);

  table2 = gtk_table_new (1, 1, FALSE);
  gtk_widget_ref (table2);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "table2", table2,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox6), table2, FALSE, TRUE, 0);
  gtk_widget_show (table2);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table2), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 5);

  scrolledwindow11 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_ref (scrolledwindow11);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "scrolledwindow11", scrolledwindow11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_table_attach (GTK_TABLE (table2), scrolledwindow11, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);
  gtk_widget_set_usize (scrolledwindow11, 200, 328);
  gtk_widget_show (scrolledwindow11);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow11), GTK_SHADOW_ETCHED_IN);

  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow11), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  col_headers[0]=_("Favorite Rooms");
  fav_room_list = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 1, 
	1, col_headers));
  gtk_widget_ref (fav_room_list);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "fav_room_list", fav_room_list,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (scrolledwindow11), fav_room_list);
  gtk_widget_set_usize (fav_room_list, 300, 328);
  gtk_widget_show (fav_room_list);

  profwidget=get_profile_name_component(1);
  gtk_box_pack_start (GTK_BOX (vbox6), profwidget, TRUE, FALSE, 2);

  hbox7 = gtk_hbox_new (TRUE, 0);
  gtk_widget_ref (hbox7);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "hbox7", hbox7,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox6), hbox7, TRUE, FALSE, 0);
  gtk_widget_show (hbox7);
  gtk_container_set_border_width (GTK_CONTAINER (hbox7), 5);

  goto_fav = get_pixmapped_button(_(" Join Room "), GTK_STOCK_APPLY);
set_tooltip(goto_fav,_(" Join Room "));
  gtk_object_set_data(GTK_OBJECT(goto_fav),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(favorites),"profcombo" ,gtk_object_get_data(GTK_OBJECT(profwidget), "profcombo") );
  gtk_widget_ref (goto_fav);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "goto_fav", goto_fav,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox7), goto_fav, FALSE, FALSE, 0);
  gtk_widget_show (goto_fav);

  cancel_favs = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
set_tooltip( cancel_favs,_(" Cancel "));
  gtk_widget_ref (cancel_favs);
  gtk_object_set_data_full (GTK_OBJECT (favorites), "cancel_favs", cancel_favs,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox7), cancel_favs, FALSE, FALSE, 0);
  gtk_widget_show (cancel_favs);

  gtk_object_set_data(GTK_OBJECT(goto_fav),PROFNAMESTR , retrieve_profname_entry (profwidget) );
  gtk_object_set_data(GTK_OBJECT(fav_room_list),PROFNAMESTR , retrieve_profname_entry (profwidget) );

  gtk_signal_connect_after (GTK_OBJECT (favorites), "delete_event",
                            GTK_SIGNAL_FUNC (on_favorites_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (fav_room_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_favroom_list_button_press_event),
                      NULL);
  g_signal_connect (fav_room_list, "popup-menu", (GCallback) on_favoriteroom_tree_kb_popup, NULL);
  g_signal_connect (fav_room_list, "row-activated", (GCallback) onFavoriteRoomRowActivated,
                      NULL);
  gtk_signal_connect (GTK_OBJECT (goto_fav), "clicked",
                      GTK_SIGNAL_FUNC (on_goto_fav_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_favs), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_favs_clicked),
                      NULL);
   gtk_window_set_default_size(GTK_WINDOW (favorites), 410, -1);
  return favorites;
}


GtkWidget* create_addfavroom_menu ()
{
  GtkWidget *setup_menu;
  GtkWidget *setup_insert;

  setup_menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (setup_menu), "setup_menu", setup_menu);

  setup_insert = gtk_image_menu_item_new_with_label (_("Add To Favorites"));
  gtk_widget_ref (setup_insert);
  gtk_object_set_data_full (GTK_OBJECT (setup_menu), "setup_insert", setup_insert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (setup_menu), setup_insert);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(setup_insert), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));

  gtk_signal_connect (GTK_OBJECT (setup_insert), "activate",
                      GTK_SIGNAL_FUNC (on_addfavroom_insert_activate),
                      NULL);
  gtk_widget_show_all(setup_menu);
  return setup_menu;
}



