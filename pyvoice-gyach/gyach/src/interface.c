/*****************************************************************************
 * interface.c
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
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "callbacks.h"
#include "interface.h"

#include "images.h"
#include "util.h"
#include "users.h"
#include "friends.h"
#include "trayicon.h"   /* added, PhrozenSmoke */
#include "profname.h"
#include "webcam.h"
#include "gytreeview.h"
#include "sounds.h"

GtkWidget* MAIN_WINDOW=NULL;
GtkWidget *mail_label=NULL;
GtkStyle  *myprofile_style = NULL;
GdkColor friends_online_color;

int num_buddies=0;
int num_buddies_online=0;
GtkWidget *buddy_list_label;
GtkTooltips *TIPS=NULL;
int color_buttons_created=0;
int show_quick_access_toolbar=1;
char *color_xpm[25];

GtkWidget *quick_access=NULL;

extern int enable_addressbook;
extern GtkWidget *get_yab_panel();


GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id) {
	GtkWidget *buttbox;
	GtkWidget *dbutton;
	GtkWidget *dlab;

	dlab=gtk_label_new(button_label);
  	gtk_label_set_justify (GTK_LABEL (dlab), GTK_JUSTIFY_LEFT);
  	gtk_misc_set_alignment (GTK_MISC (dlab), 0, 0.5);

	dbutton=gtk_button_new();
	buttbox=gtk_hbox_new(FALSE, 2);
  	gtk_container_set_border_width (GTK_CONTAINER (buttbox), 1);
	gtk_box_pack_start (GTK_BOX (buttbox), GTK_WIDGET(gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON) ), FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (buttbox), GTK_WIDGET(dlab), TRUE, TRUE,2);
	gtk_container_add (GTK_CONTAINER (dbutton), buttbox); 
	gtk_widget_show_all(dbutton);
	return dbutton;
}


void show_mail_stat(char *mailcount) {
	char mcount[8]="";	
	if (!mailcount) {return;}
	snprintf(mcount,6, "[%s]",mailcount);
	gtk_label_set_text(GTK_LABEL(mail_label), mcount);
	gtk_widget_show_all(mail_label);
	trayicon_mail_count(mailcount);
}

void set_tooltip(GtkWidget *somewid, char *somechar) {
	if (!somewid) {return;}
	if (!somechar) {return;}
	if (!TIPS) {TIPS=gtk_tooltips_new();}
	if (!TIPS) {return;}
	gtk_tooltips_set_tip(TIPS,somewid,somechar,somechar);
}



GtkWidget *create_window1 (void)
{
  GtkWidget *im_menu;

  GtkWidget *window1;
  GtkWidget *vbox3;
  GtkWidget *menubar1;
  GtkWidget *connection;
  GtkWidget *connection_menu;
  GtkWidget *connect;
  GtkWidget *disconnect;
  GtkWidget *save_current_text;
  GtkWidget *log_to_file; 
  GtkWidget *packet_debugging;
  GtkWidget *quit;
  GtkWidget *rooms;
  GtkWidget *rooms_menu;
  GtkWidget *room_list;
  GtkWidget *favorites;
  GtkWidget *add_to_favorites;
  GtkWidget *status;
  GtkWidget *status_menu;
  GSList *status_group = NULL;
  GtkWidget *status_here;
  GtkWidget *status_be_right_back;
  GtkWidget *status_idleaway;
  GtkWidget *status_busy;
  GtkWidget *status_not_at_home;
  GtkWidget *status_not_at_my_desk;
  GtkWidget *status_not_in_the_office;
  GtkWidget *status_on_the_phone;
  GtkWidget *status_on_vacation;
  GtkWidget *status_out_to_lunch;
  GtkWidget *status_stepped_out;
  GtkWidget *status_invisible;
  GtkWidget *status_autoaway;
  GtkWidget *status_custom;
  GtkWidget *setup;
  GtkWidget *help_main;
  GtkWidget *help_main_menu;
  GtkWidget *help;
  GtkWidget *about;
  GtkWidget *hbox3;
  GtkWidget *chat_toolbar;
  GtkWidget *chat_room_name;
  GtkWidget *hpaned1;
  GtkWidget *hbox2;
  GtkWidget *vpaned1;
  GtkWidget *scrolledwindow3;
  GtkWidget *users;
  GtkWidget *scrolledwindow4;
  GtkWidget *aliases;
  GtkWidget *hbox1;
  GtkWidget *chat_entry;
  GtkWidget *b_send;
  GtkWidget *b_clear;
  GtkWidget *chat_status;
  GtkWidget *mail_menu;
  GtkWidget *create_room;
  GtkWidget *launch_voice;
  GtkWidget *enable_voice;
  GtkWidget *buddy_tab;
  GtkWidget *noteb;
  GtkWidget *botbox;
  GtkWidget *files_menu;
  GtkWidget *find_text;
  GtkWidget *profnames_edit;
  GtkWidget *tools_menu;
  GtkWidget *tools;
  GtkWidget *actions_menu;
  GtkWidget *actions;

  GtkWidget *gotouser;
  GtkWidget *getprofile;
  GtkWidget *start_conf;
  GtkWidget *inv_conf;
  GtkWidget *refresh;
  GtkWidget *games_menu;
  GtkWidget *abuse_menu;
  GtkWidget *leave_conf;
  GtkWidget *status_create; 
  GtkWidget *shared_menu;
  GtkWidget *rejoin_menu;
  GtkWidget *contact_info_menu;
  GtkWidget *webcam_menu;
  GtkWidget *jointheroom_menu;
  GtkWidget *perm_igg;
  GtkWidget *tmp_friend;
  GtkWidget *flood_edit;
  GtkWidget *ignore_edit;
  GList *endlist = NULL;
  GList *startlist = NULL;
  GtkWidget *fader_text_start_combo;
  GtkWidget *fader_text_end_combo;
  GtkWidget *mailimage;
  GtkWidget *smile_button;
  GtkWidget *startcam;
  GtkWidget *sendcam;
  GtkWidget *showcamison;
  GtkWidget *pluginfo, *configfader, *newsa_menu, *im_multi_menu;
   GtkWidget *game_profile, *yprivacy;
   GtkWidget *bimage, *avatar1, *avatar2, *avatar3;
  PangoFontDescription *font_desc=NULL;
   GtkWidget *photoalbum;
   GtkWidget *update_check, *ynews_alert_menu, *yahlinks;
	
  login_credentials=NULL; /* keep this here */  

  char *aliascol_headers[]={"","","",NULL};
  char *col_headers2[]={"","","","", NULL};

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  
  MAIN_WINDOW=window1;
  gtk_window_set_wmclass (GTK_WINDOW (window1), "gyachEMain", "GyachE");
  gtk_window_set_title (GTK_WINDOW (window1), "Gyach Enhanced");
  gtk_window_set_default_size (GTK_WINDOW (window1), 940, 690);
  gtk_window_set_policy (GTK_WINDOW (window1), TRUE, TRUE, FALSE);

  vbox3 = gtk_vbox_new (FALSE, 2);
  gtk_widget_ref(vbox3);
  gtk_widget_ref(window1);
  gtk_container_add (GTK_CONTAINER (window1), vbox3); 
  // gtk_widget_show (vbox3);
  gtk_container_set_border_width (GTK_CONTAINER (vbox3), 2);

  menubar1 = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox3), menubar1, FALSE, FALSE, 0);
  // gtk_widget_show (menubar1);

  connection = gtk_menu_item_new_with_label (_("Connection"));
  gtk_container_add (GTK_CONTAINER (menubar1), connection);
  // gtk_widget_show (connection);

  connection_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (connection), connection_menu);


  connect = gtk_image_menu_item_new_with_label (_("Connect"));
  gtk_container_add (GTK_CONTAINER (connection_menu), connect);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(connect), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show (connect);

  disconnect = gtk_image_menu_item_new_with_label (_("Disconnect"));
  gtk_container_add (GTK_CONTAINER (connection_menu), disconnect);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(disconnect), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_STOP, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show (disconnect);

  /* separator */
  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

  log_to_file = gtk_check_menu_item_new_with_label (_("Log to File"));
  gtk_container_add (GTK_CONTAINER (connection_menu), log_to_file);
  // gtk_widget_show (log_to_file);

  packet_debugging = gtk_check_menu_item_new_with_label (_("Packet Debugging"));
  gtk_container_add (GTK_CONTAINER (connection_menu), packet_debugging);
  // gtk_widget_show (packet_debugging);

  /* separator */
  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

/* added: PhrozenSmoke, access to 'Find...' window from menu */

  find_text = gtk_image_menu_item_new_with_label (_(" Find Text... "));
  gtk_container_add (GTK_CONTAINER (connection_menu), find_text);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(find_text), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (find_text);


  /* separator */
  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

  save_current_text = gtk_image_menu_item_new_with_label (_("Save Current Text"));
  gtk_container_add (GTK_CONTAINER (connection_menu), save_current_text);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(save_current_text), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (save_current_text); 

  /* separator */
  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

  quit = gtk_image_menu_item_new_with_label (_("Quit"));
  gtk_container_add (GTK_CONTAINER (connection_menu), quit);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(quit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all(connection);
  

  actions = gtk_menu_item_new_with_label (_("Actions"));
  gtk_container_add (GTK_CONTAINER (menubar1), actions);
  // gtk_widget_show (actions);

  actions_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (actions), actions_menu);


  /* added: PhrozenSmoke - access to PM window */
  im_menu = gtk_image_menu_item_new_with_label (_(" Instant Message "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(im_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), im_menu);


  im_multi_menu = gtk_image_menu_item_new_with_label (_(" Send Instant Message to Many... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(im_multi_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), im_multi_menu);

  /* separator */
  gtk_container_add (GTK_CONTAINER (actions_menu), gtk_menu_item_new ());

/* added: PhrozenSmoke -user profile and join user in room items */
  getprofile = gtk_image_menu_item_new_with_label (_(" Get Profile... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(getprofile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), getprofile);


 game_profile = gtk_image_menu_item_new_with_label (_(" View Game Scores... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(game_profile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), game_profile);

  contact_info_menu = gtk_image_menu_item_new_with_label(_(" Send My Contact Information... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(contact_info_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), contact_info_menu);


  /* separator */
  gtk_container_add (GTK_CONTAINER (actions_menu), gtk_menu_item_new ());


/* added: PhrozenSmoke - file sending menu item */
  files_menu = gtk_image_menu_item_new_with_label (_(" Send File to User... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(files_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), files_menu);


  shared_menu = gtk_image_menu_item_new_with_label (_(" View Shared Files... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(shared_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), shared_menu);

  photoalbum = gtk_image_menu_item_new_with_label(_(" View Photo Album... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(photoalbum), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), photoalbum);

  /* separator */
  gtk_container_add (GTK_CONTAINER (actions_menu), gtk_menu_item_new ());

  webcam_menu = gtk_image_menu_item_new_with_label (_(" View Webcam... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(webcam_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), webcam_menu);


  startcam = gtk_image_menu_item_new_with_label (_(" Start My Webcam... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(startcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), startcam);

  sendcam = gtk_image_menu_item_new_with_label (_(" Send Webcam Invitation... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(sendcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), sendcam);

#ifdef HAVE_WEBCAM
	gtk_widget_set_sensitive(webcam_menu,1);
	gtk_widget_set_sensitive(sendcam,1);
#endif

  showcamison = gtk_check_menu_item_new_with_label  (_("Show My Webcam Is On In Chat Rooms" ));
  gtk_container_add (GTK_CONTAINER (actions_menu), showcamison);


  /* separator */
  gtk_container_add (GTK_CONTAINER (actions_menu), gtk_menu_item_new ());

  refresh = gtk_image_menu_item_new_with_label (_("Refresh Buddy List"));
  gtk_container_add (GTK_CONTAINER (actions_menu), refresh);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(refresh), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (refresh);

  /* separator */
  gtk_container_add (GTK_CONTAINER (actions_menu), gtk_menu_item_new ());

  profnames_edit = gtk_image_menu_item_new_with_label (_(" Manage Profile Names... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(profnames_edit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (actions_menu), profnames_edit);


	/* added: PhrozenSmoke, a 'tools' menu */
  tools = gtk_menu_item_new_with_label (_("Tools"));
  gtk_container_add (GTK_CONTAINER (menubar1), tools);
  // gtk_widget_show (tools);

  tools_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (tools), tools_menu);


  ignore_edit = gtk_image_menu_item_new_with_label (_(" Gyach Enhanced Ignore List... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ignore_edit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), ignore_edit);

  perm_igg = gtk_image_menu_item_new_with_label (_(" Yahoo! Ignore List... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(perm_igg), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), perm_igg);

  flood_edit = gtk_image_menu_item_new_with_label (_(" Flooders/Booters... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(flood_edit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), flood_edit);


  tmp_friend = gtk_image_menu_item_new_with_label (_(" Temporary Friends... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tmp_friend), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), tmp_friend);

  /* separator */
  gtk_container_add (GTK_CONTAINER (tools_menu), gtk_menu_item_new ());
	
  yprivacy = gtk_image_menu_item_new_with_label (_(" Yahoo! Privacy Settings... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(yprivacy), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), yprivacy);



  /* separator */
  gtk_container_add (GTK_CONTAINER (tools_menu), gtk_menu_item_new ());

  bimage = gtk_image_menu_item_new_with_label (_("Edit My Display Image..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(bimage), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), bimage);

  avatar1 = gtk_image_menu_item_new_with_label (_("Edit / Create Avatar..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(avatar1), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), avatar1);


  avatar2 = gtk_image_menu_item_new_with_label (_("View full-size avatar"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(avatar2), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), avatar2);

  avatar3 = gtk_image_menu_item_new_with_label (_("View avatar animation"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(avatar3), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), avatar3);


  /* separator */
  gtk_container_add (GTK_CONTAINER (tools_menu), gtk_menu_item_new ());

  mail_menu = gtk_image_menu_item_new_with_label (_(" Yahoo! Mail "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(mail_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), mail_menu);

  games_menu = gtk_image_menu_item_new_with_label (_(" Yahoo! Games "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(games_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), games_menu);

  newsa_menu = gtk_image_menu_item_new_with_label (_(" My News Alerts "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(newsa_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), newsa_menu);

  abuse_menu = gtk_image_menu_item_new_with_label  (_(" Report Abuse... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(abuse_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (tools_menu), abuse_menu);



  // gtk_widget_show_all(tools);
  // gtk_widget_show_all(actions);

  rooms = gtk_menu_item_new_with_label (_("Rooms"));
  gtk_container_add (GTK_CONTAINER (menubar1), rooms);
  // gtk_widget_show (rooms);

  rooms_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (rooms), rooms_menu);

  room_list = gtk_image_menu_item_new_with_label (_("Room List"));
  gtk_container_add (GTK_CONTAINER (rooms_menu), room_list);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(room_list), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (room_list);

  /* separator */
  gtk_container_add (GTK_CONTAINER (rooms_menu), gtk_menu_item_new ());

  favorites = gtk_image_menu_item_new_with_label (_("Favorites"));
  gtk_container_add (GTK_CONTAINER (rooms_menu), favorites);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(favorites), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (favorites);

  add_to_favorites = gtk_image_menu_item_new_with_label (_("Add To Favorites"));
  gtk_container_add (GTK_CONTAINER (rooms_menu), add_to_favorites);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(add_to_favorites), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (add_to_favorites);

  /* separator */
  gtk_container_add (GTK_CONTAINER (rooms_menu), gtk_menu_item_new ());

  jointheroom_menu = gtk_image_menu_item_new_with_label (_(" Join Chat Room... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(jointheroom_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu), jointheroom_menu);


  gotouser = gtk_image_menu_item_new_with_label (_(" Join User In Chat Room... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gotouser), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu), gotouser);

  /* separator */
  gtk_container_add (GTK_CONTAINER (rooms_menu), gtk_menu_item_new ());

  create_room = gtk_image_menu_item_new_with_label (_("Create Room..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(create_room), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu), create_room);
  // gtk_widget_show (create_room);

  start_conf = gtk_image_menu_item_new_with_label (_("Start A Conference"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(start_conf ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu),start_conf);
  // gtk_widget_show (start_conf);

  inv_conf = gtk_image_menu_item_new_with_label (_("Invite User to Room or Conference..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(inv_conf ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu),inv_conf);
  // gtk_widget_show (inv_conf);

  /* separator */
  gtk_container_add (GTK_CONTAINER (rooms_menu), gtk_menu_item_new ());

  leave_conf = gtk_image_menu_item_new_with_label (_("Leave Room or Conference"));
  gtk_container_add (GTK_CONTAINER (rooms_menu),leave_conf);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(leave_conf), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (leave_conf);


  rejoin_menu = gtk_image_menu_item_new_with_label (_("Rejoin Current Chat Room"));
  gtk_container_add (GTK_CONTAINER (rooms_menu),rejoin_menu);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rejoin_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (rejoin_menu);


  /* voice chat stuff, Phrozensmoke */
  /* separator */
  gtk_container_add (GTK_CONTAINER (rooms_menu), gtk_menu_item_new ());


  enable_voice = gtk_image_menu_item_new_with_label (_("Enable Voice Chat"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(enable_voice), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu),enable_voice);
  // gtk_widget_show (enable_voice);

  launch_voice = gtk_image_menu_item_new_with_label (_("Launch pY! Voice Chat"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(launch_voice), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CDROM, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (rooms_menu),launch_voice);
  // gtk_widget_show (launch_voice);

  // gtk_widget_show_all(rooms);

  status = gtk_menu_item_new_with_label (_("Status"));
  gtk_container_add (GTK_CONTAINER (menubar1), status);
  // gtk_widget_show (status);

  status_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (status), status_menu);
#ifndef USE_GTK2
  status_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (status_menu));
#endif

  status_here = gtk_radio_menu_item_new_with_label (status_group, _("Here"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_here));
  gtk_container_add (GTK_CONTAINER (status_menu), status_here);
  // gtk_widget_show (status_here);

  status_be_right_back = gtk_radio_menu_item_new_with_label (status_group, _("Be Right Back"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_be_right_back));
  gtk_container_add (GTK_CONTAINER (status_menu), status_be_right_back);
  // gtk_widget_show (status_be_right_back);

  status_busy = gtk_radio_menu_item_new_with_label (status_group, _("Busy"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_busy));
  gtk_container_add (GTK_CONTAINER (status_menu), status_busy);
  // gtk_widget_show (status_busy);

  status_not_at_home = gtk_radio_menu_item_new_with_label (status_group, _("Not At Home"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_not_at_home));
  gtk_container_add (GTK_CONTAINER (status_menu), status_not_at_home);
  // gtk_widget_show (status_not_at_home);

  status_not_at_my_desk = gtk_radio_menu_item_new_with_label (status_group, _("Not At My Desk"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_not_at_my_desk));
  gtk_container_add (GTK_CONTAINER (status_menu), status_not_at_my_desk);
  // gtk_widget_show (status_not_at_my_desk);

  status_not_in_the_office = gtk_radio_menu_item_new_with_label (status_group, _("Not In The Office"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_not_in_the_office));
  gtk_container_add (GTK_CONTAINER (status_menu), status_not_in_the_office);
  // gtk_widget_show (status_not_in_the_office);

  status_on_the_phone = gtk_radio_menu_item_new_with_label (status_group, _("On The Phone"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_on_the_phone));
  gtk_container_add (GTK_CONTAINER (status_menu), status_on_the_phone);
  // gtk_widget_show (status_on_the_phone);

  status_on_vacation = gtk_radio_menu_item_new_with_label (status_group, _("On Vacation"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_on_vacation));
  gtk_container_add (GTK_CONTAINER (status_menu), status_on_vacation);
  // gtk_widget_show (status_on_vacation);

  status_out_to_lunch = gtk_radio_menu_item_new_with_label (status_group, _("Out To Lunch"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_out_to_lunch));
  gtk_container_add (GTK_CONTAINER (status_menu), status_out_to_lunch);
  // gtk_widget_show (status_out_to_lunch);

  status_stepped_out = gtk_radio_menu_item_new_with_label (status_group, _("Stepped Out"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_stepped_out));
  gtk_container_add (GTK_CONTAINER (status_menu), status_stepped_out);
  // gtk_widget_show (status_stepped_out);

  status_invisible = gtk_radio_menu_item_new_with_label (status_group, _("Invisible"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_invisible));
  gtk_container_add (GTK_CONTAINER (status_menu), status_invisible);
  // gtk_widget_show (status_invisible);

  status_autoaway = gtk_radio_menu_item_new_with_label (status_group, _("Auto-Away"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_autoaway));
  gtk_container_add (GTK_CONTAINER (status_menu), status_autoaway);
  // gtk_widget_show (status_autoaway);


  status_idleaway = gtk_radio_menu_item_new_with_label (status_group, _("Idle"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_idleaway));
  gtk_container_add (GTK_CONTAINER (status_menu), status_idleaway);
  // gtk_widget_show (status_idleaway);


  status_custom = gtk_radio_menu_item_new_with_label (status_group, _("Custom Message"));
  status_group = gtk_radio_menu_item_group (GTK_RADIO_MENU_ITEM (status_custom));
  gtk_container_add (GTK_CONTAINER (status_menu), status_custom);
  // gtk_widget_show (status_custom);


    /* separator */
  gtk_container_add (GTK_CONTAINER (status_menu), gtk_menu_item_new ());

  status_create = gtk_menu_item_new_with_label (_("New Custom Message..."));
  gtk_container_add (GTK_CONTAINER (status_menu), status_create);
  // gtk_widget_show (status_create);


  gtk_container_add (GTK_CONTAINER (status_menu), gtk_menu_item_new ());

  ynews_alert_menu = gtk_check_menu_item_new_with_label (_("Yahoo News Alerts"));
  gtk_container_add (GTK_CONTAINER (status_menu), ynews_alert_menu);


  setup = gtk_image_menu_item_new_with_label (_("Setup"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(setup), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (menubar1), setup);
  // gtk_widget_show (setup);

  help_main = gtk_image_menu_item_new_with_label (_("Help"));
  gtk_container_add (GTK_CONTAINER (menubar1), help_main);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(help_main), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show (help_main);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help_main));

  help_main_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_main), help_main_menu);
#ifndef USE_GTK2
  help_main_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (help_main_menu));
#endif

  help = gtk_image_menu_item_new_with_label (_("Help"));
  gtk_container_add (GTK_CONTAINER (help_main_menu), help);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(help), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_HELP, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (help);


    /* separator */
  gtk_container_add (GTK_CONTAINER (help_main_menu), gtk_menu_item_new ());


  about = gtk_image_menu_item_new_with_label (_("About GYach Enhanced..."));
  gtk_container_add (GTK_CONTAINER (help_main_menu), about);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(about), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (about);

  pluginfo = gtk_image_menu_item_new_with_label (_("Plugins"));
  gtk_container_add (GTK_CONTAINER (help_main_menu), pluginfo);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pluginfo), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (pluginfo);

  yahlinks = gtk_image_menu_item_new_with_label (_("Y! Links"));
  gtk_container_add (GTK_CONTAINER (help_main_menu), yahlinks);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(yahlinks), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUMP_TO, GTK_ICON_SIZE_MENU) ));

  update_check = gtk_image_menu_item_new_with_label (_("Check for New Versions and News..."));
  gtk_container_add (GTK_CONTAINER (help_main_menu), update_check);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(update_check), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU) ));
  // gtk_widget_show_all (update_check);

  hbox3 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox3, FALSE, FALSE, 0);
  // gtk_widget_show (hbox3);


  chat_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_orientation( GTK_TOOLBAR(chat_toolbar), GTK_ORIENTATION_HORIZONTAL );
  gtk_toolbar_set_style( GTK_TOOLBAR(chat_toolbar), GTK_TOOLBAR_ICONS );

  gtk_box_pack_start (GTK_BOX (hbox3), chat_toolbar, FALSE, FALSE, 2);
  // gtk_widget_show (chat_toolbar);
  gtk_container_set_border_width (GTK_CONTAINER (chat_toolbar), 2);

   /* added PhrozenSmoke, mail icon stuff */
  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new("  "), FALSE, FALSE, 0);
  mailimage=get_pm_icon(12,_("E-mail Messages"));
	if (mailimage) {
	GtkWidget *ymailbutt=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ymailbutt), mailimage);
  	set_tooltip(ymailbutt,_("E-mail Messages"));
  	// gtk_widget_show_all(ymailbutt);
  	gtk_box_pack_start (GTK_BOX (hbox3), ymailbutt, FALSE, FALSE, 0);
	gtk_button_set_relief(GTK_BUTTON(ymailbutt), GTK_RELIEF_NONE);
	  gtk_signal_connect_object(GTK_OBJECT(ymailbutt), "clicked",
				GTK_SIGNAL_FUNC(on_yahoo_mail_act),
				GTK_OBJECT(pluginfo));
	}
  /* gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new(" "), FALSE, FALSE, 0); */ 
  mail_label=gtk_label_new("[0]");
  gtk_box_pack_start (GTK_BOX (hbox3), mail_label, FALSE, FALSE, 0);
  gtk_misc_set_alignment (GTK_MISC (mail_label), 0, 0.5);
  font_desc = pango_font_description_from_string("courier 12");
  if ( font_desc ) {
	gtk_widget_modify_font  (mail_label,font_desc);
	pango_font_description_free(font_desc);
	}

  /* Appended: PhrozenSmoke, allow US to use faders and ALT tags */
  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new("       "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new(_("Fader:  ")), FALSE, FALSE, 0);
  use_chat_fader=gtk_check_button_new_with_label (_("On  "));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_chat_fader),TRUE);
  if (use_chat_fader_val<1) {
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_chat_fader),FALSE);
	}
  gtk_box_pack_start (GTK_BOX (hbox3), use_chat_fader, FALSE, FALSE, 0);
  set_tooltip(use_chat_fader,_("Fader:  "));

  configfader=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(configfader), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ) );
gtk_button_set_relief(GTK_BUTTON(configfader), GTK_RELIEF_NONE);
  gtk_box_pack_start (GTK_BOX (hbox3), configfader, FALSE, FALSE, 1);
  set_tooltip(configfader,_("Configuration: Text Effects"));

  fader_text_start_combo=gtk_combo_new();
  fader_text_start=GTK_COMBO( fader_text_start_combo)->entry;
  gtk_entry_set_max_length(GTK_ENTRY(fader_text_start),63);		 
  gtk_editable_set_editable(GTK_EDITABLE(fader_text_start),TRUE );


  gtk_entry_set_text(GTK_ENTRY(fader_text_start),"<FADE #2063f3,#a2d3f0,#b7a9d6,#9d42df,#828fc7,#b2c0d8,#3683b9>");

  startlist = g_list_append( startlist, "<FADE #a584c2,#b7a9d6,#828fc7,#b2c0d8,#3683b9>" );  
  startlist = g_list_append( startlist, "<FADE #2063f3,#a2d3f0,#b7a9d6,#9d42df,#828fc7,#b2c0d8,#3683b9>" );  
  startlist = g_list_append( startlist, "<FADE #328dfc,#c62fff,#1d9dfc,#51b5ff,#de86ff>" );  
  startlist = g_list_append( startlist, "<FADE #069ff8,#02d6f4,#aecde8,#8eb0e2,#b9bfce,#8b8ba8,#7070a2>" );  
  startlist = g_list_append( startlist, "<FADE #2853ea,#a2d3f0,#a9b2d6,#a254da,#a0a8cb,#ba94c6,#6166a6>" );  
  startlist = g_list_append( startlist, "<FADE #a246b9,#1d44e2,#b046b9,#182de7>" );
  startlist = g_list_append( startlist, "<FADE #acc2de,#375d8d,#bbcee7,#254166,#9cb2d5,#a1badb,#2a3363>" );
  startlist = g_list_append( startlist, "<FADE #1394ef,#04d8f7,#1494f6,#9b1df3,#cf4dfb>" );
  startlist = g_list_append( startlist, "<FADE #acc2de,#8127a4,#bbcee7,#277be7,#9cb2d5,#a1badb,#8628d0>" );
  startlist = g_list_append( startlist, "<FADE #b9cadc,#8d9eca,#9e7bb0,#8d9eca,#a49fc2>" );  
  startlist=g_list_append(startlist,"<FADE #b9cadc,#8d9eca,#26c6df,#8d9eca,#a49fc2,#a9b9ca>");   startlist = g_list_append( startlist, "<FADE #b9c5da,#aebfe3,#a7dcd2,#cab3d0,#b1def5>" );  
  startlist = g_list_append( startlist, "<FADE #bdc6ff,#bddaff,#bdaeff,#a4e2ff,#bdcee6>" );  
  startlist = g_list_append( startlist, "<FADE #a72a35,#ccb995,#a0834c,#5b4c2e,#ab9685,#c8bda9,#ca9b61>" );  
  startlist = g_list_append( startlist, "<FADE #cbd7e5,#d28ec6,#f60865,#c60df5,#1879ef>" ); 
  startlist = g_list_append( startlist, "<FADE #cbd7e5,#376ac6,#a588bb,#ab22bb,#920df5,#7c13d7,#252dd5>" ); 
  startlist = g_list_append( startlist, "<FADE #e9a613,#a9893d,#cb8d32,#ebd3b0,#bb9c6f>" );  
  startlist = g_list_append( startlist, "<FADE #947eb7,#a559c7,#7773db,#559ac3,#9f41bb,#b95590>" );  
  startlist = g_list_append( startlist, "<FADE #28a2ea,#28a2ea,#4230e2,#4230e2>" );
  startlist = g_list_append( startlist, "<FADE #00BBDD,#1d44e2,#a246b9>" );
  startlist = g_list_append( startlist, "<FADE #982612,#d2351a,#e8553c,#f19c8d,#f5bcb1>" );


  startlist = g_list_append( startlist, "<ALT #a584c2,#b7a9d6,#828fc7,#b2c0d8,#3683b9>" );
  startlist = g_list_append( startlist, "<ALT #ab1231,#302499,#5aaf27,#8c318c,#327d8b>" );
  startlist = g_list_append( startlist, "<ALT #00AABB,#00CCDD>" );
  startlist = g_list_append( startlist, "<ALT #a246b9,#1d44e2,#b046b9,#182de7>" );
  startlist = g_list_append( startlist, "<ALT #28a2ea,#28a2ea,#4230e2,#4230e2>" );
  startlist = g_list_append( startlist, "<ALT #d36393,#6a5fd8,#d76095,#5a5adc>" );
  startlist = g_list_append( startlist, "<ALT #00BBDD,#1d44e2,#a246b9>" );
  startlist = g_list_append( startlist, "<ALT #947eb7,#a559c7,#7773db,#559ac3,#9f41bb,#b95590>" );  
  startlist = g_list_append( startlist, "<ALT #982612,#d2351a,#e8553c,#f19c8d,#f5bcb1>" );
  startlist = g_list_append( startlist, "<ALT #a72a35,#ccb995,#a0834c,#5b4c2e,#ab9685,#c8bda9,#ca9b61>" );  
  startlist =g_list_append( startlist, "<ALT #f00f8b,#240215,#f00f8b,#240215,#f00f8b,#240215>" );

  gtk_combo_set_popdown_strings( GTK_COMBO(fader_text_start_combo), startlist );

if (fader_text_start_str) {
  	gtk_entry_set_text(GTK_ENTRY(fader_text_start),fader_text_start_str);
}

  gtk_widget_set_usize(fader_text_start, 230,-2);
  gtk_box_pack_start (GTK_BOX (hbox3), fader_text_start_combo, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new("  "), FALSE, FALSE, 0);

  fader_text_end_combo=gtk_combo_new();
  fader_text_end=GTK_COMBO( fader_text_end_combo)->entry;
  gtk_entry_set_max_length(GTK_ENTRY(fader_text_end),8);		 
  gtk_editable_set_editable(GTK_EDITABLE(fader_text_end),TRUE );
  gtk_entry_set_text(GTK_ENTRY(fader_text_end),"</FADE>");

  endlist = g_list_append( endlist, "</FADE>" );
  endlist = g_list_append( endlist, "</ALT>" );
  endlist = g_list_append( endlist, " " );
  gtk_combo_set_popdown_strings( GTK_COMBO(fader_text_end_combo), endlist );

if (fader_text_end_str) {
  	gtk_entry_set_text(GTK_ENTRY(fader_text_end),fader_text_end_str);
}

  gtk_widget_set_usize(fader_text_end, 85,-2);
  gtk_box_pack_start (GTK_BOX (hbox3),fader_text_end_combo, FALSE, FALSE, 0);

  // gtk_widget_show_all(hbox3);
  /* end appended, PhrozenSmoke */

  /* just a space-holder */
  chat_room_name = gtk_label_new (" ");
  gtk_box_pack_start (GTK_BOX (hbox3), chat_room_name, FALSE, FALSE, 0);
  // gtk_widget_show (chat_room_name);


  botbox = gtk_vbox_new (FALSE, 2);
  // gtk_widget_show (botbox);

  hpaned1 = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (botbox), hpaned1, TRUE, TRUE, 0);  
  // gtk_widget_show (hpaned1);





  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_widget_ref (hbox2);
	/* do this now */ 
  gtk_object_set_data (GTK_OBJECT (window1), "hbox2", hbox2);
  gtk_paned_pack1 (GTK_PANED (hpaned1), hbox2, TRUE, TRUE);
  // gtk_widget_show (hbox2);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 1);



  vpaned1 = gtk_vpaned_new ();
  gtk_paned_pack2 (GTK_PANED (hpaned1), vpaned1, TRUE, TRUE);
  gtk_widget_set_usize (vpaned1, 259, -2);
  // gtk_widget_show (vpaned1);
  gtk_container_set_border_width (GTK_CONTAINER (vpaned1), 2);

  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_pack1 (GTK_PANED (vpaned1), scrolledwindow3, TRUE, TRUE);
  gtk_widget_set_usize (scrolledwindow3, 320, 140);
  // gtk_widget_show (scrolledwindow3);


  col_headers2[0]=" ";
  col_headers2[1]=_("Alias");
  col_headers2[2]=_("Screen Name");
  col_headers2[3]=_("Last Comment");

   users = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_QUAD, 1, 1, 	col_headers2));
   set_basic_treeview_sorting(users, GYLIST_TYPE_QUAD);
  gtk_widget_ref (users);
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), users);
  // gtk_widget_show (users);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_SHADOW_NONE);


  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_pack2 (GTK_PANED (vpaned1), scrolledwindow4, TRUE, TRUE);
  // gtk_widget_show (scrolledwindow4);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

	aliascol_headers[0]=_("Emotes");
	aliascol_headers[1]=_("Cmd Format #1");
	aliascol_headers[2]=_("Cmd Format #2");

  aliases = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_TRIPLE, 1, 
	0, aliascol_headers));
  set_basic_treeview_sorting(aliases,GYLIST_TYPE_TRIPLE);
  gtk_container_add (GTK_CONTAINER (scrolledwindow4), aliases);
  // gtk_widget_show (aliases);


  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_SHADOW_NONE);
gtk_widget_set_usize (scrolledwindow4, -2, 120);


  hbox1 = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (botbox), hbox1, FALSE, TRUE, 0);
  /* gtk_widget_set_usize (hbox1, 600, -2);  */
  // gtk_widget_show (hbox1);


  /* smiley button */
  smile_button=get_smiley_button("smicon",0);
 gtk_button_set_relief(GTK_BUTTON(smile_button), GTK_RELIEF_NONE);
  	gtk_signal_connect (GTK_OBJECT (smile_button), "clicked",
                      GTK_SIGNAL_FUNC (show_smiley_window_chat), NULL);	
  if (chatwin_smileys) {gtk_box_pack_start (GTK_BOX (hbox1), smile_button, FALSE, FALSE, 0);}
  set_tooltip(smile_button,_("Colors & Emoticons") );

  chat_entry = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(chat_entry),500);
  gtk_box_pack_start (GTK_BOX (hbox1), chat_entry, TRUE, TRUE, 1);
  // gtk_widget_show (chat_entry);

  gtk_object_set_data(GTK_OBJECT (smile_button), "entry", chat_entry); 


  b_send=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(b_send), get_pm_icon(8,_(" Send ")) );
  gtk_container_set_border_width(GTK_CONTAINER(b_send), 1);
 gtk_button_set_relief(GTK_BUTTON(b_send), GTK_RELIEF_NONE);
  set_tooltip(b_send,_(" Send "));
  if (chatwin_send_erase) {gtk_box_pack_start (GTK_BOX (hbox1), b_send, FALSE, FALSE, 0);}
  // gtk_widget_show_all (b_send);

  b_clear=gtk_button_new();
  gtk_container_add(GTK_CONTAINER(b_clear), get_pm_icon(3,_(" Clear ")) );
  gtk_container_set_border_width(GTK_CONTAINER(b_clear), 1);
 gtk_button_set_relief(GTK_BUTTON(b_clear), GTK_RELIEF_NONE);
  set_tooltip(b_clear,_(" Clear "));
  if (chatwin_send_erase) {gtk_box_pack_start (GTK_BOX (hbox1), b_clear, FALSE, FALSE, 0);}
  // gtk_widget_show_all (b_clear);

   buddy_tab=create_buddy_list_tab ();
   noteb=create_notebook (botbox, buddy_tab);
   gtk_box_pack_start (GTK_BOX (vbox3), noteb, TRUE, TRUE, 1); 


  chat_status = gtk_statusbar_new ();
  gtk_box_pack_start (GTK_BOX (vbox3), chat_status, FALSE, TRUE, 0);
    /* must do this one now */ 
  gtk_widget_show (chat_status);
  gtk_container_set_border_width (GTK_CONTAINER (chat_status), 2);


	/* set up some gtk object references */ 
	/* needed by gyach_int.c */ 
  gtk_object_set_data (GTK_OBJECT (window1), "connect", connect);
  gtk_object_set_data (GTK_OBJECT (window1), "disconnect", disconnect);
  gtk_object_set_data (GTK_OBJECT (window1), "actions", actions);
  gtk_object_set_data (GTK_OBJECT (window1), "tools", tools);
  gtk_object_set_data (GTK_OBJECT (window1), "rooms", rooms);
  gtk_object_set_data (GTK_OBJECT (window1), "status", status);
  gtk_object_set_data (GTK_OBJECT (window1), "chat_toolbar", chat_toolbar);
  gtk_object_set_data (GTK_OBJECT (window1), "hbox2", hbox2);
  gtk_object_set_data (GTK_OBJECT (window1), "scrolledwindow3", scrolledwindow3);
   gtk_object_set_data (GTK_OBJECT (window1), "quick_access", quick_access?quick_access:tools_menu);

	/* needed by pmwindow.c */
  gtk_object_set_data (GTK_OBJECT (window1), "chat_entry", chat_entry);

	/* needed by callbacks.c */
  gtk_object_set_data (GTK_OBJECT (window1), "log_to_file", log_to_file);
  gtk_object_set_data (GTK_OBJECT (window1), "packet_debugging", packet_debugging);

	/* needed all over, especially callbacks.c  */ 
   gtk_object_set_data (GTK_OBJECT (window1), "gynotebook", noteb);

	/* needed by commands.c */ 
  gtk_object_set_data (GTK_OBJECT (window1), "status_here", status_here);
  gtk_object_set_data (GTK_OBJECT (window1), "status_be_right_back", status_be_right_back);
  gtk_object_set_data (GTK_OBJECT (window1), "status_busy", status_busy);
  gtk_object_set_data (GTK_OBJECT (window1), "status_not_at_home", status_not_at_home);
  gtk_object_set_data (GTK_OBJECT (window1), "status_not_at_my_desk", status_not_at_my_desk);
  gtk_object_set_data (GTK_OBJECT (window1), "status_not_in_the_office", status_not_in_the_office);
  gtk_object_set_data (GTK_OBJECT (window1), "status_on_the_phone", status_on_the_phone);
  gtk_object_set_data (GTK_OBJECT (window1), "status_on_vacation", status_on_vacation);
  gtk_object_set_data (GTK_OBJECT (window1), "status_out_to_lunch", status_out_to_lunch);
  gtk_object_set_data (GTK_OBJECT (window1), "status_stepped_out", status_stepped_out);
  gtk_object_set_data (GTK_OBJECT (window1), "status_invisible", status_invisible);
  gtk_object_set_data (GTK_OBJECT (window1), "status_autoaway", status_autoaway);
  gtk_object_set_data (GTK_OBJECT (window1), "status_idleaway", status_idleaway);
  gtk_object_set_data (GTK_OBJECT (window1), "status_custom", status_custom);

	/* some plugins need these */ 
  gtk_object_set_data (GTK_OBJECT (window1), "tools_menu", tools_menu);
  gtk_object_set_data (GTK_OBJECT (window1), "actions_menu", actions_menu);
  gtk_object_set_data (GTK_OBJECT (window1), "rooms_menu", rooms_menu);
  gtk_object_set_data (GTK_OBJECT (window1), "connection_menu", connection_menu);
  gtk_object_set_data (GTK_OBJECT (window1), "help_main_menu", help_main_menu);
  gtk_object_set_data (GTK_OBJECT (window1), "status_menu", status_menu);

/* needed in packet_handler.c */
   gtk_object_set_data (GTK_OBJECT (window1), "ynews_alert_menu", ynews_alert_menu);


  gtk_signal_connect (GTK_OBJECT (window1), "map",
                      GTK_SIGNAL_FUNC (on_window1_map),
                      NULL);
  gtk_signal_connect_after (GTK_OBJECT (window1), "delete_event",
                            GTK_SIGNAL_FUNC (on_chat_window_delete_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (connect), "activate",
                      GTK_SIGNAL_FUNC (on_connect_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (disconnect), "activate",
                      GTK_SIGNAL_FUNC (on_disconnect_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (find_text), "activate",
                      GTK_SIGNAL_FUNC (on_find_menu_clicked),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (save_current_text), "activate",
                      GTK_SIGNAL_FUNC (on_save_current_text_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (log_to_file), "activate",
                      GTK_SIGNAL_FUNC (on_log_to_file_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (packet_debugging), "activate",
                      GTK_SIGNAL_FUNC (on_packet_debugging_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (quit), "activate",
                      GTK_SIGNAL_FUNC (on_quit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (room_list), "activate",
                      GTK_SIGNAL_FUNC (on_room_list_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (favorites), "activate",
                      GTK_SIGNAL_FUNC (on_favorites_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (add_to_favorites), "activate",
                      GTK_SIGNAL_FUNC (on_add_to_favorites_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (webcam_menu), "activate",
                      GTK_SIGNAL_FUNC (on_viewcam),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (create_room), "activate",
                      GTK_SIGNAL_FUNC (on_create_room_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (status_here), "activate",
                      GTK_SIGNAL_FUNC (on_status_here_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_be_right_back), "activate",
                      GTK_SIGNAL_FUNC (on_status_be_right_back_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_busy), "activate",
                      GTK_SIGNAL_FUNC (on_status_busy_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_not_at_home), "activate",
                      GTK_SIGNAL_FUNC (on_status_not_at_home_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_not_at_my_desk), "activate",
                      GTK_SIGNAL_FUNC (on_status_not_at_my_desk_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_not_in_the_office), "activate",
                      GTK_SIGNAL_FUNC (on_status_not_in_the_office_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_on_the_phone), "activate",
                      GTK_SIGNAL_FUNC (on_status_on_the_phone_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_on_vacation), "activate",
                      GTK_SIGNAL_FUNC (on_status_on_vacation_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_out_to_lunch), "activate",
                      GTK_SIGNAL_FUNC (on_status_out_to_lunch_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_stepped_out), "activate",
                      GTK_SIGNAL_FUNC (on_status_stepped_out_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_invisible), "activate",
                      GTK_SIGNAL_FUNC (on_status_invisible_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_autoaway), "activate",
                      GTK_SIGNAL_FUNC (on_status_autoaway_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_idleaway), "activate",
                      GTK_SIGNAL_FUNC (on_status_idle_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_custom), "activate",
                      GTK_SIGNAL_FUNC (on_status_custom_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (status_create), "activate",
                      GTK_SIGNAL_FUNC (on_make_custom_away),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (setup), "activate",
                      GTK_SIGNAL_FUNC (on_setup_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (help), "activate",
                      GTK_SIGNAL_FUNC (on_help_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (yahlinks), "activate",
                      GTK_SIGNAL_FUNC (on_help_activate),
                      "1");
  gtk_signal_connect (GTK_OBJECT (about), "activate",
                      GTK_SIGNAL_FUNC (on_about_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (users), "map",
                      GTK_SIGNAL_FUNC (on_users_map),
                      NULL);
  g_signal_connect (users, "row-activated", (GCallback) onChatUsersRowActivated,
                      NULL);
  g_signal_connect (users, "popup-menu", (GCallback) on_chatlist_kb_popup,
                      NULL);
g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(users))), "changed", G_CALLBACK(collect_chat_list_selected), NULL);
  gtk_signal_connect (GTK_OBJECT (users), "button_press_event",
                      GTK_SIGNAL_FUNC (on_users_clicked),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (aliases), "map",
                      GTK_SIGNAL_FUNC (on_aliases_map),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (aliases), "button_press_event",
                      GTK_SIGNAL_FUNC (on_aliases_clicked),
                      NULL);
  g_signal_connect (aliases, "row-activated", (GCallback) onAliasesRowActivated,
                      NULL);
  g_signal_connect (aliases, "popup-menu", (GCallback) on_aliases_kb_popup,
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chat_entry), "activate",
                      GTK_SIGNAL_FUNC (on_chat_entry_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chat_entry), "map",
                      GTK_SIGNAL_FUNC (on_chat_entry_map),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chat_entry), "event",
                      GTK_SIGNAL_FUNC (on_chat_entry_key_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (b_send), "clicked",
                      GTK_SIGNAL_FUNC (on_b_send_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (b_clear), "clicked",
                      GTK_SIGNAL_FUNC (on_b_clear_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (chat_status), "map",
                      GTK_SIGNAL_FUNC (on_chat_status_map),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (im_menu), "activate",
                      GTK_SIGNAL_FUNC (on_blank_pm_window),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (gotouser), "activate",
                      GTK_SIGNAL_FUNC (on_open_goto_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (getprofile), "activate",
                      GTK_SIGNAL_FUNC (on_open_profile_cb),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (files_menu), "activate",
                      GTK_SIGNAL_FUNC (on_send_file_to_user_cb),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (inv_conf), "activate",
                      GTK_SIGNAL_FUNC (on_invite_to_conf),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (start_conf), "activate",
                      GTK_SIGNAL_FUNC (on_start_conference),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (refresh), "activate",
                      GTK_SIGNAL_FUNC (on_refresh_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (yprivacy), "activate",
                      GTK_SIGNAL_FUNC (on_open_yprivacy_window),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (launch_voice), "activate",
                      GTK_SIGNAL_FUNC (on_launch_py_voice),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (enable_voice), "activate",
                      GTK_SIGNAL_FUNC (on_py_voice_enable),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (im_multi_menu), "activate",
                      GTK_SIGNAL_FUNC (on_open_multi_pms),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (games_menu), "activate",
                      GTK_SIGNAL_FUNC (on_yahoo_games),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newsa_menu), "activate",
                      GTK_SIGNAL_FUNC (on_yahoo_news_alert),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (mail_menu), "activate",
                      GTK_SIGNAL_FUNC (on_yahoo_mail_act),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (abuse_menu), "activate",
                      GTK_SIGNAL_FUNC (on_report_abuse),
                      NULL);
  gtk_signal_connect (GTK_OBJECT ( leave_conf), "activate",
                      GTK_SIGNAL_FUNC (on_leave_room_or_conf),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (rejoin_menu), "activate",
                      GTK_SIGNAL_FUNC (on_rejoin_room),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (shared_menu), "activate",
                      GTK_SIGNAL_FUNC (on_view_briefcase),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (contact_info_menu), "activate",
                      GTK_SIGNAL_FUNC (on_send_contact_info),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (perm_igg), "activate",
                      GTK_SIGNAL_FUNC (open_perm_ignore),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (jointheroom_menu), "activate",
                      GTK_SIGNAL_FUNC (on_joinaroom),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (tmp_friend), "activate",
                      GTK_SIGNAL_FUNC (open_temp_friend_editor),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (flood_edit), "activate",
                      GTK_SIGNAL_FUNC (open_flooder_editor),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignore_edit), "activate",
                      GTK_SIGNAL_FUNC (open_ignore_editor),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (startcam), "activate",
                      GTK_SIGNAL_FUNC (on_startmycam),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (sendcam), "activate",
                      GTK_SIGNAL_FUNC (on_sendcaminvite),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (showcamison), "toggled",
                      GTK_SIGNAL_FUNC (on_show_camera_on ),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (game_profile), "activate",
                      GTK_SIGNAL_FUNC (on_gameprofile ),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pluginfo), "activate",
                      GTK_SIGNAL_FUNC (on_display_plugin_info),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (configfader), "clicked",
                      GTK_SIGNAL_FUNC (on_create_fadeconfigwin),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (profnames_edit), "activate",
                      GTK_SIGNAL_FUNC (create_profilename_window_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (bimage), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_edit_my_photo_menu),
                      NULL);
 gtk_signal_connect (GTK_OBJECT (avatar1), "activate",
                      GTK_SIGNAL_FUNC (on_goto_edit_avatar_menu ),
                      NULL);
 gtk_signal_connect (GTK_OBJECT (avatar2), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_view_my_avatar_cb),
                      NULL);
 gtk_signal_connect (GTK_OBJECT (avatar3), "activate",
                      GTK_SIGNAL_FUNC (on_bimage_view_my_avatar_cb),
                      avatar3);
 gtk_signal_connect (GTK_OBJECT (photoalbum), "activate",
                      GTK_SIGNAL_FUNC (on_photoalbum),
                      NULL);
 gtk_signal_connect (GTK_OBJECT (update_check), "activate",
                      GTK_SIGNAL_FUNC (on_check_gyache_version_cb),
                      NULL);


  return window1;
}


GtkWidget*
create_user_menu (void)
{
  GtkWidget *user_menu;
  GtkWidget *profile;
  GtkWidget *qprofile;
  GtkWidget *pm;
  GtkWidget *add_to_friends;
  GtkWidget *ignore;
  GtkWidget *ignoreb;
  GtkWidget *follow;
  GtkWidget *muter;
  GtkWidget *unmute;
  GtkWidget *unignore;
  GtkWidget *webcam;
  GtkWidget *sendcam;
  GtkWidget *tmpfriend;
  GtkWidget *cpname2, *cpname1, *immune;


  user_menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (user_menu), "user_menu", user_menu);
  gtk_container_set_border_width (GTK_CONTAINER (user_menu), 2);


  pm = gtk_image_menu_item_new_with_label (_(" Instant Message "));
  gtk_container_add (GTK_CONTAINER (user_menu), pm);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pm), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));

 gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());


  qprofile = gtk_image_menu_item_new_with_label (_(" Quick Profile "));
  gtk_container_add (GTK_CONTAINER (user_menu), qprofile);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(qprofile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));


  profile = gtk_image_menu_item_new_with_label (_(" Profile "));
  gtk_container_add (GTK_CONTAINER (user_menu), profile);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(profile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU) ));


 gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());

  add_to_friends = gtk_image_menu_item_new_with_label (_(" Add To Friends "));
  gtk_container_add (GTK_CONTAINER (user_menu), add_to_friends);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(add_to_friends), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));


  tmpfriend = gtk_image_menu_item_new_with_label (_(" Add As Temporary Friend "));
  gtk_container_add (GTK_CONTAINER (user_menu), tmpfriend);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tmpfriend), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));

 gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());

  follow = gtk_image_menu_item_new_with_label (_(" Follow "));
  gtk_container_add (GTK_CONTAINER (user_menu), follow);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(follow), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU) ));

  gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());

  webcam = gtk_image_menu_item_new_with_label (_(" View Webcam... "));
  gtk_container_add (GTK_CONTAINER (user_menu), webcam);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(webcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_MENU) ));

  sendcam = gtk_image_menu_item_new_with_label (_(" Send Webcam Invitation... "));
   gtk_container_add (GTK_CONTAINER (user_menu), sendcam);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(sendcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));

	/* webcam currently not complete */
  gtk_widget_set_sensitive(webcam,0);
  gtk_widget_set_sensitive(sendcam,0);

#ifdef HAVE_WEBCAM
	gtk_widget_set_sensitive(webcam,1);
	gtk_widget_set_sensitive(sendcam,1);
#endif


  gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());

  ignore = gtk_image_menu_item_new_with_label (_(" Ignore "));
  gtk_container_add (GTK_CONTAINER (user_menu), ignore);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ignore), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));

  ignoreb = gtk_image_menu_item_new_with_label (_(" Ignore & BROADCAST "));
  gtk_container_add (GTK_CONTAINER (user_menu), ignoreb);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ignoreb), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_MENU) ));

  unignore = gtk_image_menu_item_new_with_label (_(" Un-Ignore "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(unignore), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_STOP, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (user_menu), unignore);

  muter = gtk_image_menu_item_new_with_label (_(" Mute "));
  gtk_container_add (GTK_CONTAINER (user_menu), muter);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(muter), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_MENU) ));

  unmute = gtk_image_menu_item_new_with_label (_(" Un-Mute "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(unmute), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (user_menu), unmute);


  immune = gtk_image_menu_item_new_with_label (_(" Do Not Auto-Mute "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(immune), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PASTE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (user_menu), immune);

 gtk_container_add (GTK_CONTAINER (user_menu), gtk_menu_item_new ());

  cpname1 = gtk_image_menu_item_new_with_label (_("Copy name"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cpname1), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (user_menu), cpname1);

  cpname2 = gtk_image_menu_item_new_with_label (_("Copy nick name"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cpname2), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (user_menu), cpname2);


  gtk_widget_show_all(user_menu);

  gtk_signal_connect (GTK_OBJECT (cpname2), "activate",
                      GTK_SIGNAL_FUNC (on_copy_room_user_name),
                      cpname2);
  gtk_signal_connect (GTK_OBJECT (cpname1), "activate",
                      GTK_SIGNAL_FUNC (on_copy_room_user_name),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (profile), "activate",
                      GTK_SIGNAL_FUNC (on_profile_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (qprofile), "activate",
                      GTK_SIGNAL_FUNC (on_qprofile_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (pm), "activate",
                      GTK_SIGNAL_FUNC (on_pm_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (add_to_friends), "activate",
                      GTK_SIGNAL_FUNC (on_add_to_friends_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignore), "activate",
                      GTK_SIGNAL_FUNC (on_ignore_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignoreb), "activate",
                      GTK_SIGNAL_FUNC (on_ignore_broadcast_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (follow), "activate",
                      GTK_SIGNAL_FUNC (on_follow_activate),
                      NULL);

  gtk_signal_connect (GTK_OBJECT (unignore), "activate",
                      GTK_SIGNAL_FUNC (on_unignore ),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (unmute), "activate",
                      GTK_SIGNAL_FUNC (on_unmute),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (immune), "activate",
                      GTK_SIGNAL_FUNC (on_doimmunity),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (muter), "activate",
                      GTK_SIGNAL_FUNC (on_domute),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (webcam), "activate",
                      GTK_SIGNAL_FUNC (on_viewcam_room),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (tmpfriend), "activate",
                      GTK_SIGNAL_FUNC (on_add_tmp_friend),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (sendcam), "activate",
                      GTK_SIGNAL_FUNC (on_sendcaminvite_room),
                      NULL);
  return user_menu;
}


/* end added PhrozenSmoke */

GtkWidget *create_login_window (void)
{
  GtkWidget *login_window;
  GtkWidget *vbox5;
  GtkWidget *table1;
  GtkWidget *chat_password;
  GtkWidget *label4;
  GtkWidget *label5;
  GtkWidget *label6;
  GtkWidget *chat_username;
  GtkWidget *chat_room;
  GtkWidget *label44;
  GtkWidget *chat_server;
  GtkWidget *login_button;
  GtkWidget *cancel_login;
  GtkWidget *invis_button;
  GtkWidget *noroom_button;
  GtkWidget *hbox66;
 GtkWidget *logohbox;
 GtkWidget *logomain;
  GtkWidget *chat_port;
  GdkPixbuf *imbuf=NULL;
  GtkWidget *imimage=NULL;

  login_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (login_window), "gyachELogin", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW ( login_window), GTK_WIN_POS_CENTER);
  gtk_object_set_data (GTK_OBJECT (login_window), "login_window", login_window);
  gtk_window_set_title (GTK_WINDOW (login_window), _("Gyach Enhanced: Yahoo! Login"));

  logomain = gtk_hbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (login_window), logomain);
  logohbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (logomain), logohbox, FALSE, FALSE, 1);

  imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_log_logo);
  if (imbuf) {imimage=gtk_image_new_from_pixbuf(imbuf);}
  if (imimage) {  
	gtk_box_pack_start (GTK_BOX (logohbox), imimage, FALSE, FALSE, 0);
  }
  if (imbuf) {g_object_unref(imbuf); imbuf=NULL;}

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (logomain), vbox5, TRUE, TRUE, 1);
  // gtk_container_add (GTK_CONTAINER (login_window), vbox5);
  gtk_container_set_border_width (GTK_CONTAINER (vbox5), 5);
  gtk_object_set_data (GTK_OBJECT (login_window), "mainbox",vbox5);

  hbox66 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox5), hbox66, FALSE, FALSE, 3);

  imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_gyach_icon);
  if (imbuf) {imimage=gtk_image_new_from_pixbuf(imbuf);}
  if (imimage) {  
	gtk_box_pack_start (GTK_BOX (hbox66), gtk_label_new(" "), TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox66), imimage, FALSE, FALSE, 0);
  }
  if (imbuf) {g_object_unref(imbuf);}

  gtk_box_pack_start (GTK_BOX (hbox66), gtk_label_new (_("Enter Login Information: ")),
		 FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox66), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_set_spacing (GTK_BOX (hbox66), 7);

  table1 = gtk_table_new (4, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox5), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 5);
  gtk_table_set_row_spacings (GTK_TABLE (table1), 10);

  chat_password = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(chat_password),30);
  gtk_widget_set_usize (chat_password, 350, -2);
  gtk_object_set_data (GTK_OBJECT (login_window), "chat_password", chat_password);
  gtk_table_attach (GTK_TABLE (table1), chat_password, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_entry_set_visibility (GTK_ENTRY (chat_password), FALSE);

  label4 = gtk_label_new (_("Username: "));
  gtk_misc_set_alignment (GTK_MISC (label4), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table1), label4, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_padding (GTK_MISC (label4), 5, 0);

  label5 = gtk_label_new (_("Password: "));
  gtk_misc_set_alignment (GTK_MISC (label5), 0, 0.5);
  gtk_object_set_data (GTK_OBJECT (login_window), "label5", label5);
  gtk_table_attach (GTK_TABLE (table1), label5, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_padding (GTK_MISC (label5), 5, 0);

  label6 = gtk_label_new (_("Room: "));
  gtk_misc_set_alignment (GTK_MISC (label6), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table1), label6, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label6), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_padding (GTK_MISC (label6), 5, 0);

  chat_username = gtk_combo_new ();
  // gtk_widget_ref (chat_username);
  gtk_object_set_data (GTK_OBJECT (login_window), "chat_username", chat_username);
  gtk_table_attach (GTK_TABLE (table1), chat_username, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_combo_set_use_arrows_always (GTK_COMBO (chat_username), TRUE);
  gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO (chat_username)->entry), 60);

  chat_room = gtk_combo_new ();
  gtk_object_set_data (GTK_OBJECT (login_window), "chat_room", chat_room);
  gtk_table_attach (GTK_TABLE (table1), chat_room, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_combo_set_use_arrows_always (GTK_COMBO (chat_room), TRUE);
  gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO (chat_room)->entry), 60);

  label44 = gtk_label_new (_("Server: "));
  gtk_misc_set_alignment (GTK_MISC (label44), 0, 0.5);
  gtk_table_attach (GTK_TABLE (table1), label44, 0, 1, 3, 4,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (label44), GTK_JUSTIFY_RIGHT);
  gtk_misc_set_padding (GTK_MISC (label44), 5, 0);


  hbox66 = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (table1), hbox66, 1, 2, 3, 4,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  chat_server = gtk_combo_new ();
  gtk_object_set_data (GTK_OBJECT (login_window), "chat_server", chat_server);
  gtk_combo_set_use_arrows_always (GTK_COMBO (chat_server), TRUE);
  gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO (chat_server)->entry), 124);
  gtk_box_pack_start (GTK_BOX (hbox66), chat_server, TRUE, TRUE, 0);

  chat_port = gtk_combo_new ();
  gtk_object_set_data (GTK_OBJECT (login_window), "chat_port", chat_port);
  gtk_combo_set_use_arrows_always (GTK_COMBO (chat_port), TRUE);
  gtk_entry_set_max_length(GTK_ENTRY(GTK_COMBO (chat_port)->entry), 7);
  gtk_box_pack_start (GTK_BOX (hbox66), gtk_label_new(" "), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox66), chat_port, FALSE, FALSE, 0);
  gtk_widget_set_usize (chat_port, 83, -2);


  hbox66 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox5), hbox66, FALSE, FALSE, 3);

  invis_button = gtk_check_button_new_with_mnemonic (_("Login as invisible"));
  gtk_object_set_data (GTK_OBJECT (login_window), "invis_button", invis_button);
  gtk_box_pack_start (GTK_BOX (hbox66), invis_button, TRUE, TRUE, 2);
 set_tooltip(invis_button,_("Login as invisible"));

  noroom_button = gtk_check_button_new_with_mnemonic (_("No chat room"));
  gtk_object_set_data (GTK_OBJECT (login_window), "noroom_button", noroom_button);
  gtk_box_pack_start (GTK_BOX (hbox66), gtk_label_new("  "), TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox66), noroom_button, FALSE, FALSE, 2);
 set_tooltip(noroom_button,_("No chat room"));

  hbox66 = gtk_hbox_new (TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox5), hbox66, FALSE, FALSE, 3);

  login_button =  get_pixmapped_button(_(" Login "), GTK_STOCK_YES);
  gtk_box_pack_start (GTK_BOX (hbox66), login_button, FALSE, FALSE, 0);
 set_tooltip(login_button,_(" Login "));

  cancel_login = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox66), cancel_login, FALSE, FALSE, 0);
 set_tooltip(cancel_login,_(" Cancel "));

  gtk_signal_connect (GTK_OBJECT (login_button), "clicked",
                      GTK_SIGNAL_FUNC (on_login_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_login), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_login_clicked),
                      NULL);
  gtk_signal_connect_after (GTK_OBJECT (login_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_login_window_destroy_event),
                            NULL);

  return login_window;
}

/* Module splitting: PhrozenSmoke, 
    'create_favorites' moved to roomui.c
*/



GtkWidget* create_profile_window (void)
{
  GtkWidget *profile_window;
  GtkWidget *layout1;
  GtkWidget *close_profile;
  GtkWidget *profile_view_tv;
  GtkWidget *user_picture;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *scrolledwindow1;
  GtkWidget *picbox;
  GtkTextBuffer *profile_view_tb;
  GtkWidget *infoframe;

  profile_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (profile_window), "gyachEProfile", "GyachE"); 
    gtk_window_set_position (GTK_WINDOW (profile_window), GTK_WIN_POS_CENTER);
  gtk_object_set_data (GTK_OBJECT (profile_window), "profile_window", profile_window);
  gtk_container_set_border_width (GTK_CONTAINER (profile_window), 2);
  gtk_window_set_title (GTK_WINDOW (profile_window), _("Yahoo! User Profile"));

  layout1=gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (layout1), 2);
  gtk_container_add (GTK_CONTAINER (profile_window), layout1);

  hbox1=gtk_hbox_new (FALSE, 8);  /* holds profile and pic */
  gtk_box_pack_start (GTK_BOX (layout1), hbox1, TRUE, TRUE, 2);

   profile_view_tv = gtk_text_view_new ();	
   gtk_text_view_set_editable(GTK_TEXT_VIEW(profile_view_tv), FALSE);
   gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(profile_view_tv), GTK_WRAP_WORD );
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW(profile_view_tv), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( profile_view_tv), 5);
   profile_view_tb=gtk_text_view_get_buffer( GTK_TEXT_VIEW(profile_view_tv));
     gtk_object_set_data (GTK_OBJECT (profile_view_tv), "textbuffer", 
		profile_view_tb );
   gtk_object_set_data (GTK_OBJECT (profile_window), "textbuffer", 
		profile_view_tb );
    gtk_object_set_data (GTK_OBJECT (profile_window), "profile_info", profile_view_tv);

	/* setup the callback for url highlighting and clicking */
	gtk_signal_connect (GTK_OBJECT( profile_view_tv), "button_press_event",
                      GTK_SIGNAL_FUNC (on_pms_text_button_press_event),
                      NULL);

	gtk_signal_connect_after (GTK_OBJECT (profile_view_tv),
		"motion_notify_event", GTK_SIGNAL_FUNC(chat_motion_notify), NULL);

	GTK_WIDGET_UNSET_FLAGS( GTK_WIDGET(profile_view_tv), GTK_CAN_FOCUS);

	gtk_widget_set_events( GTK_WIDGET(profile_view_tv),
		GDK_POINTER_MOTION_MASK
		| GDK_POINTER_MOTION_HINT_MASK );

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 2);
  gtk_widget_set_usize (scrolledwindow1, 400, 300);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET(profile_view_tv));
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_OUT);


  user_picture = get_pm_icon(15, "");
  gtk_object_set_data (GTK_OBJECT (profile_window), "user_picture", user_picture); 

  infoframe = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (infoframe), GTK_SHADOW_ETCHED_IN );
  gtk_container_add (GTK_CONTAINER (infoframe), user_picture );
	/* This causes gtk console warnings below */ 
  /* gtk_container_set_border_width (GTK_CONTAINER (user_picture), 3); */ 

  picbox= gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (picbox), infoframe, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox1), picbox, FALSE, FALSE, 4);

  hbox2=gtk_hbox_new (FALSE, 0);   /* holds 'Close' button */
  gtk_box_pack_start (GTK_BOX (picbox), hbox2, FALSE, FALSE, 2); 
  close_profile = get_pixmapped_button(_(" Close "), GTK_STOCK_CANCEL);
    gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new("  "), TRUE, TRUE, 0);
    gtk_box_pack_start (GTK_BOX (hbox2), close_profile, FALSE, FALSE, 2);
    gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new("  "), TRUE, TRUE, 0);
 set_tooltip(close_profile,_(" Close "));

  gtk_widget_show_all(layout1);

  gtk_signal_connect_after (GTK_OBJECT (profile_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_profile_window_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (close_profile), "clicked",
                      GTK_SIGNAL_FUNC (on_close_profile_clicked),
                      NULL);

  return profile_window;
}

GtkWidget* create_alias_menu (void)
{
  GtkWidget *alias_menu;
#ifndef USE_GTK2
  GtkAccelGroup *alias_menu_accels;
#endif
  GtkWidget *alias_send;
  GtkWidget *alias_edit;
  GtkWidget *alias_delete;
  GtkWidget *alias_new;

  alias_menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (alias_menu), "alias_menu", alias_menu);
#ifndef USE_GTK2
  alias_menu_accels = gtk_menu_ensure_uline_accel_group (GTK_MENU (alias_menu));
#endif

  alias_send = gtk_image_menu_item_new_with_label (_("Send"));
  // gtk_widget_ref (alias_send);
  gtk_object_set_data (GTK_OBJECT (alias_menu), "alias_send", alias_send);
  gtk_container_add (GTK_CONTAINER (alias_menu), alias_send);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(alias_send), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OK, GTK_ICON_SIZE_MENU) ));
  gtk_widget_show_all (alias_send);

  alias_edit = gtk_image_menu_item_new_with_label (_("Edit..."));
  // gtk_widget_ref (alias_edit);
  gtk_object_set_data (GTK_OBJECT (alias_menu), "alias_edit", alias_edit);
  gtk_container_add (GTK_CONTAINER (alias_menu), alias_edit);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(alias_edit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));
  gtk_widget_show_all (alias_edit);

  alias_delete = gtk_image_menu_item_new_with_label (_("Delete..."));
  // gtk_widget_ref (alias_delete);
  gtk_object_set_data (GTK_OBJECT (alias_menu), "alias_delete", alias_delete);
  gtk_container_add (GTK_CONTAINER (alias_menu), alias_delete);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(alias_delete), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU) ));
  gtk_widget_show_all (alias_delete);

  alias_new = gtk_image_menu_item_new_with_label (_("New..."));
  // gtk_widget_ref (alias_new);
  gtk_object_set_data (GTK_OBJECT (alias_menu), "alias_new", alias_new);
  gtk_container_add (GTK_CONTAINER (alias_menu), alias_new);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(alias_new), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));
  gtk_widget_show_all (alias_new);

  gtk_signal_connect (GTK_OBJECT (alias_send), "activate",
                      GTK_SIGNAL_FUNC (on_alias_send_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (alias_edit), "activate",
                      GTK_SIGNAL_FUNC (on_alias_edit_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (alias_delete), "activate",
                      GTK_SIGNAL_FUNC (on_alias_delete_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (alias_new), "activate",
                      GTK_SIGNAL_FUNC (on_alias_new_activate),
                      NULL);

  return alias_menu;
}

GtkWidget* create_alias_window (void)
{
  GtkWidget *alias_window;
  GtkWidget *vbox7;
  GtkWidget *label49;
  GtkWidget *table3;
  GtkWidget *label46;
  GtkWidget *label47;
  GtkWidget *label48;
  GtkWidget *cmd_value1;
  GtkWidget *cmd_value2;
  GtkWidget *alias_name;
  GtkWidget *hbox8;
  GtkWidget *alias_save;
  GtkWidget *alias_cancel;

  alias_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
gtk_window_set_position (GTK_WINDOW (alias_window), GTK_WIN_POS_MOUSE);
  gtk_object_set_data (GTK_OBJECT (alias_window), "alias_window", alias_window);
  gtk_container_set_border_width (GTK_CONTAINER (alias_window), 2);
  gtk_window_set_title (GTK_WINDOW (alias_window), _("Edit Emote"));

  vbox7 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(vbox7),5);
  // gtk_widget_ref (vbox7);
  gtk_object_set_data (GTK_OBJECT (alias_window), "vbox7", vbox7);
  gtk_container_add (GTK_CONTAINER (alias_window), vbox7);

  	gtk_box_pack_start (GTK_BOX (vbox7), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_DND)), FALSE, FALSE , 3);

  gtk_widget_show_all (vbox7);

  label49 = gtk_label_new ("");
  // gtk_widget_ref (label49);
  gtk_object_set_data (GTK_OBJECT (alias_window), "label49", label49);
  gtk_box_pack_start (GTK_BOX (vbox7), label49, FALSE, FALSE, 0);
  gtk_widget_show (label49);

  table3 = gtk_table_new (3, 2, FALSE);
  // gtk_widget_ref (table3);
  gtk_object_set_data (GTK_OBJECT (alias_window), "table3", table3);
  gtk_box_pack_start (GTK_BOX (vbox7), table3, TRUE, TRUE, 0);
  gtk_widget_show (table3);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 5);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 5);

  label46 = gtk_label_new (_("Emote Name:"));
  // gtk_widget_ref (label46);
  gtk_object_set_data (GTK_OBJECT (alias_window), "label46", label46);
  gtk_table_attach (GTK_TABLE (table3), label46, 0, 1, 0, 1,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (label46);
  gtk_label_set_justify (GTK_LABEL (label46), GTK_JUSTIFY_RIGHT);

  label47 = gtk_label_new (_("Cmd Version 1:"));
  // gtk_widget_ref (label47);
  gtk_object_set_data (GTK_OBJECT (alias_window), "label47", label47);
  gtk_table_attach (GTK_TABLE (table3), label47, 0, 1, 1, 2,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (label47);
  gtk_label_set_justify (GTK_LABEL (label47), GTK_JUSTIFY_RIGHT);

  label48 = gtk_label_new (_("Cmd Version 2:"));
  // gtk_widget_ref (label48);
  gtk_object_set_data (GTK_OBJECT (alias_window), "label48", label48);
  gtk_table_attach (GTK_TABLE (table3), label48, 0, 1, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (label48);

  cmd_value1 = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(cmd_value1),512);

   gtk_widget_set_usize (cmd_value1, 335, -2);  

  // gtk_widget_ref (cmd_value1);
  gtk_object_set_data (GTK_OBJECT (alias_window), "cmd_value1", cmd_value1);
  gtk_table_attach (GTK_TABLE (table3), cmd_value1, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (cmd_value1);

  cmd_value2 = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(cmd_value2),512);
  // gtk_widget_ref (cmd_value2);
  gtk_object_set_data (GTK_OBJECT (alias_window), "cmd_value2", cmd_value2);
  gtk_table_attach (GTK_TABLE (table3), cmd_value2, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (cmd_value2);

  alias_name = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(alias_name),15);
  // gtk_widget_ref (alias_name);
  gtk_object_set_data (GTK_OBJECT (alias_window), "alias_name", alias_name);
  gtk_table_attach (GTK_TABLE (table3), alias_name, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_widget_show (alias_name);

  gtk_box_pack_start (GTK_BOX (vbox7), gtk_label_new(" "), TRUE, TRUE, 0);

  hbox8 = gtk_hbox_new (TRUE, 0);
  // gtk_widget_ref (hbox8);
  gtk_object_set_data (GTK_OBJECT (alias_window), "hbox8", hbox8);
  gtk_box_pack_start (GTK_BOX (vbox7), hbox8, FALSE, FALSE, 0);
  gtk_widget_show (hbox8);
  gtk_container_set_border_width (GTK_CONTAINER (hbox8), 10);

  alias_save = get_pixmapped_button(_(" Save "), GTK_STOCK_SAVE);
  // gtk_widget_ref (alias_save);
  gtk_object_set_data (GTK_OBJECT (alias_window), "alias_save", alias_save);
  gtk_box_pack_start (GTK_BOX (hbox8), alias_save, FALSE, FALSE, 0);
  gtk_widget_show (alias_save);
 set_tooltip(alias_save,_(" Save "));

  alias_cancel = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  // gtk_widget_ref (alias_cancel);
  gtk_object_set_data (GTK_OBJECT (alias_window), "alias_cancel", alias_cancel);
  gtk_box_pack_start (GTK_BOX (hbox8), alias_cancel, FALSE, FALSE, 0);
  gtk_widget_show (alias_cancel);
	 set_tooltip(alias_cancel,_(" Cancel "));

  gtk_signal_connect_after (GTK_OBJECT (alias_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_alias_window_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (alias_save), "clicked",
                      GTK_SIGNAL_FUNC (on_alias_save_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (alias_cancel), "clicked",
                      GTK_SIGNAL_FUNC (on_alias_cancel_clicked),
                      NULL);

  return alias_window;
}


GtkWidget* create_fileselection (void)
{
  GtkWidget *fileselection;
  GtkWidget *file_ok_button;
  GtkWidget *file_cancel_button;

  fileselection = gtk_file_selection_new (_("File name"));
  gtk_window_set_wmclass (GTK_WINDOW (fileselection), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (fileselection), "fileselection", fileselection);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection), 10);

  file_ok_button = GTK_FILE_SELECTION (fileselection)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "file_ok_button", file_ok_button);
  gtk_widget_show (file_ok_button);
  GTK_WIDGET_SET_FLAGS (file_ok_button, GTK_CAN_DEFAULT);

  file_cancel_button = GTK_FILE_SELECTION (fileselection)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "file_cancel_button", file_cancel_button);
  gtk_widget_show (file_cancel_button);
  GTK_WIDGET_SET_FLAGS (file_cancel_button, GTK_CAN_DEFAULT);

  gtk_signal_connect (GTK_OBJECT (file_ok_button), "clicked",
                      GTK_SIGNAL_FUNC (on_file_ok_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (file_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (on_file_cancel_button_clicked),
                      NULL);

  return fileselection;
}

 /* Module splitting, PhrozenSmoke:
  'create_room_list' moved to roomui.c module
  */


/* Module splitting, PhrozenSmoke:
	'create_edit_ignore_window', 'create_setup_window', and 'create_setup_menu' moved to setupwindow.c
*/

/* 
 create_pm_session - moved to pmwindow.c, PhrozenSmoke 
*/


/* completely re-written: PhrozenSmoke, I REALLY wish the original author had 
     NOT used fixed positions, since he seemed to assume everybody uses the same 
     font size...let's make it USABLE on all systems, trading in 'gtklayout' objects 
     for hbox/vbox  */

GtkWidget* create_find_window (void)
{
  GtkWidget *find_window;
  GtkWidget *layout12;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *hbox3;
  GtkWidget *search_text;
  GtkWidget *label91;
  GtkWidget *case_sensitive;
  GtkWidget *find_button;
  GtkWidget *clear_find_button;
  GtkWidget *cancel_find_button;
  GtkWidget *find_next_button;

  find_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (find_window), "gyachEDialog", "GyachE"); 
gtk_window_set_position (GTK_WINDOW (find_window), GTK_WIN_POS_MOUSE);
  gtk_object_set_data (GTK_OBJECT (find_window), "find_window", find_window);
  /*  gtk_widget_set_usize (find_window, 380, 140);  */
  gtk_window_set_title (GTK_WINDOW (find_window), _("Gyach Enhanced: Find"));
  /* gtk_window_set_default_size (GTK_WINDOW (find_window), 380, 140);  */
  gtk_window_set_policy (GTK_WINDOW (find_window), FALSE, FALSE, FALSE);

  layout12 = gtk_vbox_new(FALSE, 1);
  gtk_container_set_border_width(GTK_CONTAINER(layout12),5);
  // gtk_widget_ref (layout12);
  gtk_object_set_data (GTK_OBJECT (find_window), "layout12", layout12);
  gtk_container_add (GTK_CONTAINER (find_window), layout12);
  gtk_widget_show (layout12);

  	gtk_box_pack_start (GTK_BOX (layout12), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND, GTK_ICON_SIZE_DND)), FALSE, FALSE , 3);

  hbox1 = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_start(GTK_BOX(layout12), hbox1, FALSE, FALSE,3);
  hbox2 = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_start(GTK_BOX(layout12), hbox2, FALSE, FALSE,3);
  gtk_box_pack_start(GTK_BOX(layout12), gtk_hseparator_new (), TRUE, TRUE,4);
  hbox3 = gtk_hbox_new(FALSE, 1);
  gtk_box_pack_start(GTK_BOX(layout12), hbox3, FALSE, FALSE,3);
  gtk_container_set_border_width(GTK_CONTAINER(hbox3),4);

  search_text = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(search_text),40);
  // gtk_widget_ref (search_text);
  gtk_object_set_data (GTK_OBJECT (find_window), "search_text", search_text);
  gtk_widget_set_usize (search_text, 230, -2);
  gtk_widget_show (search_text);

  label91 = gtk_label_new (_("Find:"));
  gtk_label_set_justify (GTK_LABEL (label91), GTK_JUSTIFY_RIGHT);
  gtk_box_pack_start(GTK_BOX(hbox1), label91, FALSE, FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox1), search_text, TRUE, TRUE,3);

  case_sensitive = gtk_check_button_new_with_label (_("Case Sensitive"));
  set_tooltip(case_sensitive,_("Case Sensitive"));
  // gtk_widget_ref (case_sensitive);
  gtk_object_set_data (GTK_OBJECT (find_window), "case_sensitive", case_sensitive);
  gtk_widget_show (case_sensitive);
  gtk_box_pack_start(GTK_BOX(hbox2),  case_sensitive, FALSE, FALSE,3);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_label_new(" "), TRUE, TRUE,3);

  find_button = get_pixmapped_button(_(" Find "), GTK_STOCK_ZOOM_IN);
  set_tooltip(find_button,_(" Find "));
  // gtk_widget_ref (find_button);
  gtk_object_set_data (GTK_OBJECT (find_window), "find_button", find_button);
  gtk_widget_show (find_button);

  clear_find_button =  get_pixmapped_button(_(" Clear "), GTK_STOCK_CLEAR);
  set_tooltip(clear_find_button,_(" Clear "));
  // gtk_widget_ref (clear_find_button);
  gtk_object_set_data (GTK_OBJECT (find_window), "clear_find_button", clear_find_button);
  gtk_widget_show (clear_find_button);

  cancel_find_button = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  set_tooltip(cancel_find_button,_(" Cancel "));
  // gtk_widget_ref (cancel_find_button);
  gtk_object_set_data (GTK_OBJECT (find_window), "cancel_find_button", cancel_find_button);
  gtk_widget_show (cancel_find_button);

  find_next_button = get_pixmapped_button(_(" Next "), GTK_STOCK_GO_FORWARD);
  set_tooltip(find_next_button,_(" Next "));
  // gtk_widget_ref (find_next_button);
  gtk_object_set_data (GTK_OBJECT (find_window), "find_next_button", find_next_button);
  gtk_widget_show (find_next_button);

  gtk_box_pack_start(GTK_BOX(hbox3), find_button, TRUE, TRUE,3);
  gtk_box_pack_start(GTK_BOX(hbox3), find_next_button, TRUE, TRUE,3);
  gtk_box_pack_start(GTK_BOX(hbox3), gtk_label_new("   "), TRUE, TRUE,3);
  gtk_box_pack_start(GTK_BOX(hbox3), clear_find_button, TRUE, TRUE,3);
  gtk_box_pack_start(GTK_BOX(hbox3), gtk_label_new("   "), TRUE, TRUE,3);
  gtk_box_pack_start(GTK_BOX(hbox3), cancel_find_button, TRUE, TRUE,3);

  gtk_signal_connect (GTK_OBJECT (find_window), "delete_event",
                      GTK_SIGNAL_FUNC (on_find_window_delete_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (search_text), "activate",
                      GTK_SIGNAL_FUNC (on_search_text_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (find_button), "clicked",
                      GTK_SIGNAL_FUNC (on_find_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (clear_find_button), "clicked",
                      GTK_SIGNAL_FUNC (on_clear_find_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_find_button), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_find_button_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (find_next_button), "clicked",
                      GTK_SIGNAL_FUNC (on_find_next_button_clicked),
                      NULL);

  return find_window;
}


 /* Module splitting, PhrozenSmoke:
  'create_room_maker' moved to roomui.c module
  */

 GtkWidget *get_stocki_button(const gchar *stock_id, char *tip) {
	GtkWidget *im_menu;
	im_menu = gtk_button_new();
	gtk_container_add(GTK_CONTAINER(im_menu), GTK_WIDGET(gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_MENU) ));
	if (tip) {set_tooltip(im_menu,tip); }
	gtk_button_set_relief(GTK_BUTTON(im_menu), GTK_RELIEF_NONE); 
	return im_menu;
}


/* PhrozenSmoke: UI code for buddylist */

GtkWidget* create_buddy_list_tab ()
{
  GtkWidget *mainvbox;
  GtkWidget *hbox1;
  GtkWidget *scrolledwindow6;

  GtkWidget *pmbutton;
  GtkWidget *joinbutton;
  GtkWidget *invitebutton;
  GtkWidget *cambutton;
  GtkWidget *profilebutton;
  GtkWidget *addbutton;
  GtkWidget *delbutton;
  GtkWidget *sendbutton, *refreshbutton;
  GtkWidget *contbutton, *movebutton, *infobutton, *dummyw, *hbox2;

  GtkTreeSelection *selection;
  char *col_headers[]={"","",NULL};

  mainvbox = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(mainvbox),  4);
  // gtk_widget_ref (mainvbox);
  gtk_object_set_data (GTK_OBJECT (MAIN_WINDOW), "buddy_mainvbox", mainvbox);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width(GTK_CONTAINER(hbox1),  4);
  gtk_box_pack_start (GTK_BOX (mainvbox), hbox1, FALSE, FALSE, 0);

  hbox2=gtk_hbox_new (FALSE, 0);;

   scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (mainvbox), scrolledwindow6, TRUE, TRUE, 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);

  col_headers[0]=_("Buddy");
  col_headers[1]=_("Status");
  bd_tree=GTK_WIDGET(create_gy_treeview(GYTV_TYPE_TREE,GYLIST_TYPE_DOUBLE, 0, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), bd_tree);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_SHADOW_NONE);


	pmbutton=get_stocki_button(GTK_STOCK_NEW,_("Instant Message"));	  	
	joinbutton=get_stocki_button(GTK_STOCK_JUMP_TO,_("Join User In Room"));	 	  	 	
	invitebutton=get_stocki_button(GTK_STOCK_CONVERT,_("Invite User To Room"));	  	
	cambutton=get_stocki_button(GTK_STOCK_ZOOM_100,_(" View Webcam... "));		  	  
	profilebutton=get_stocki_button(GTK_STOCK_DIALOG_INFO,_("Profile"));	 	  	
	addbutton=get_stocki_button(GTK_STOCK_ADD,_("Add Buddy..."));	  	
	delbutton=get_stocki_button(GTK_STOCK_REMOVE,_("Remove Buddy"));	  		  	
	sendbutton=get_stocki_button(GTK_STOCK_SAVE,_("Send File..."));	 	  	
	contbutton=get_stocki_button(GTK_STOCK_JUSTIFY_LEFT,_("Edit Contact Information..."));	 
	movebutton=get_stocki_button(GTK_STOCK_REDO,_("Move to group..."));	 
	infobutton=get_stocki_button(GTK_STOCK_DIALOG_QUESTION,_("Info"));	 
	refreshbutton=get_stocki_button(GTK_STOCK_UNDO,_("Refresh Buddy List"));	

	gtk_box_set_spacing (GTK_BOX (hbox1), 2);
	gtk_box_set_spacing (GTK_BOX (hbox2), 2);


  gtk_box_pack_start (GTK_BOX (hbox1), hbox2, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("  "), FALSE, FALSE, 0); /* space */ 
  buddy_list_label=gtk_label_new("                 ");
  gtk_box_pack_start (GTK_BOX (hbox1), buddy_list_label, TRUE, TRUE, 2);

  gtk_box_pack_start (GTK_BOX (hbox2), pmbutton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_vseparator_new(), FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (hbox2), infobutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), profilebutton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_vseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), contbutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), cambutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), sendbutton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_vseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), addbutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), delbutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), movebutton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_vseparator_new(), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), joinbutton, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), invitebutton, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), gtk_vseparator_new(), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox2), refreshbutton, FALSE, FALSE, 0);

	dummyw=gtk_separator_menu_item_new ();

	/* do these by proxy */ 
  gtk_signal_connect_object(GTK_OBJECT(movebutton), "clicked",
				GTK_SIGNAL_FUNC(on_move_buddy_to_group),
				GTK_OBJECT(dummyw));
  gtk_signal_connect_object(GTK_OBJECT(contbutton), "clicked",
				GTK_SIGNAL_FUNC(on_blist_get_yab),
				GTK_OBJECT(dummyw));
  gtk_signal_connect_object(GTK_OBJECT(infobutton), "clicked",
				GTK_SIGNAL_FUNC(on_viewbuddyinfo),
				GTK_OBJECT(dummyw));
  gtk_signal_connect_object(GTK_OBJECT(cambutton), "clicked",
				GTK_SIGNAL_FUNC(on_viewcam),
				GTK_OBJECT(dummyw));
  gtk_signal_connect_object(GTK_OBJECT(refreshbutton), "clicked",
				GTK_SIGNAL_FUNC(on_refresh_activate),
				GTK_OBJECT(dummyw));

  gtk_signal_connect (GTK_OBJECT (pmbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_pm_activate), NULL);

  gtk_signal_connect (GTK_OBJECT (joinbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_join_in_chat ), NULL);

  gtk_signal_connect (GTK_OBJECT (invitebutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_invite_to_chat), NULL);

  gtk_signal_connect (GTK_OBJECT (profilebutton), "clicked",
                      GTK_SIGNAL_FUNC (on_buddy_profile_activate), NULL);

  gtk_signal_connect (GTK_OBJECT (addbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_click_add_a_buddy), NULL);

  gtk_signal_connect (GTK_OBJECT (delbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_click_del_a_buddy), NULL);

  gtk_signal_connect (GTK_OBJECT (sendbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_buddy_file), NULL);


  gtk_signal_connect (GTK_OBJECT (bd_tree), "button_press_event",
                      GTK_SIGNAL_FUNC (on_select_a_buddy  ),
                      NULL);	
  selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(bd_tree));
 g_signal_connect (bd_tree, "row-activated", (GCallback) onBuddyListRowActivated, NULL);
  g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(collect_buddy_list_selected), NULL); 
 g_signal_connect (bd_tree, "popup-menu", (GCallback) on_display_buddy_list_popup_menu_kb, NULL);
  set_basic_treeview_sorting(bd_tree, GYLIST_TYPE_DOUBLE);

  gtk_widget_show_all(mainvbox);
  return mainvbox;
}


/* end code for buddy list */

/* added: PhrozenSmoke, UI change: use notebook for chat room and simple buddy list  */

GtkWidget *get_note_tab(char *label, char **picon) {
	GtkWidget *mainvbox, *clabel;
	GdkPixbuf *imbuf=NULL;
	GtkWidget *imimage=NULL;
	GdkPixbuf *spixbuf=NULL;
	mainvbox = gtk_hbox_new (FALSE, 0);
	clabel=gtk_label_new(label);
	gtk_label_set_justify (GTK_LABEL (clabel), GTK_JUSTIFY_LEFT);
  	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)picon);
	if (imbuf) {spixbuf = gdk_pixbuf_scale_simple( imbuf, 16, 16,
					GDK_INTERP_BILINEAR ); }
  	if (spixbuf) {imimage=gtk_image_new_from_pixbuf(spixbuf);}
	if (imimage) {gtk_box_pack_start(GTK_BOX(mainvbox),  imimage, FALSE, FALSE,0);}
	gtk_box_pack_start(GTK_BOX(mainvbox),  clabel, FALSE, FALSE,0);
	gtk_box_set_spacing (GTK_BOX (mainvbox), 4);
	if (imbuf) {g_object_unref(imbuf);}
	if (spixbuf) {g_object_unref(spixbuf);}
	gtk_widget_show_all(mainvbox);
	return mainvbox;
}


GtkWidget *create_quick_access_bar() {
 	GtkWidget *mainy;
	GtkWidget *dummymenu, *im_menu;
	dummymenu=gtk_menu_item_new ();

 	mainy = gtk_hbox_new (FALSE, 0);
	gtk_box_set_spacing (GTK_BOX (mainy), 1);

	im_menu = get_stocki_button(GTK_STOCK_DIALOG_ERROR, _("Status"));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_make_custom_away),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(mainy), gtk_vseparator_new(), FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_NEW, _(" Instant Message "));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_blank_pm_window),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);


	im_menu = get_stocki_button(GTK_STOCK_DIALOG_INFO, _(" Get Profile... "));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_open_profile_cb),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(mainy), gtk_vseparator_new(), FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_FIND_AND_REPLACE, _("Room List"));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_room_list_activate),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_OPEN, _("Favorites"));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_favorites_activate),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_JUMP_TO, _(" Join Chat Room... "));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_joinaroom),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_CONVERT, _("Invite User to Room or Conference..."));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_invite_to_conf),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	gtk_box_pack_start(GTK_BOX(mainy), gtk_vseparator_new(), FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_YES,_("Enable Voice Chat"));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_py_voice_enable),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);

	im_menu = get_stocki_button(GTK_STOCK_CDROM,_("Launch pY! Voice Chat"));
	gtk_signal_connect_object(GTK_OBJECT(im_menu), "clicked",
				GTK_SIGNAL_FUNC(on_launch_py_voice),
				GTK_OBJECT(dummymenu));
	gtk_box_pack_start(GTK_BOX(mainy), im_menu, FALSE, FALSE, 0);



	gtk_widget_show_all(mainy);
	return mainy;
}


GtkWidget* create_notebook (GtkWidget *chatframe, GtkWidget *buddyframe)
{
  GtkWidget *notebook;

	notebook=gtk_notebook_new();
 	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook),TRUE);
 	gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook),TRUE);
	g_object_set(notebook,"show-border", FALSE, NULL);
	g_object_set(notebook,"tab-hborder", 2, NULL);
	g_object_set(notebook,"tab-vborder", 1, NULL);
	g_object_set(notebook,"homogeneous", TRUE, NULL);

	gtk_notebook_append_page(GTK_NOTEBOOK(notebook),buddyframe,
	get_note_tab(_("Buddies"), pixmap_status_here_male)  );


	if (show_quick_access_toolbar && (! enable_addressbook) ) {
		GtkWidget *widgy;
		GtkWidget *subwidgy;
		g_object_set(notebook,"homogeneous", FALSE, NULL);
 		widgy = gtk_hbox_new (FALSE, 0);
		gtk_box_set_spacing (GTK_BOX (widgy), 1);
		gtk_box_pack_start(GTK_BOX(widgy), get_note_tab(_("My Yahoo!"), pixmap_pm_profile) , FALSE, FALSE, 0);
		gtk_box_pack_start(GTK_BOX(widgy), gtk_label_new("    "), FALSE, FALSE, 0);
		subwidgy=create_quick_access_bar();
		gtk_box_pack_start(GTK_BOX(widgy),subwidgy , FALSE, FALSE, 0);
		gtk_widget_show_all(widgy);
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),get_my_yahoo_panel(),
		widgy  );
		quick_access=subwidgy;
	} else {
		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),get_my_yahoo_panel(),
		get_note_tab(_("My Yahoo!"), pixmap_pm_profile)   );
		if (! enable_addressbook) { quick_access=gtk_label_new(""); }
		}


	if ( enable_addressbook) {
		if (show_quick_access_toolbar ) {
			GtkWidget *widgy;
			GtkWidget *subwidgy;
			g_object_set(notebook,"homogeneous", FALSE, NULL);
 			widgy = gtk_hbox_new (FALSE, 0);
			gtk_box_set_spacing (GTK_BOX (widgy), 1);
			gtk_box_pack_start(GTK_BOX(widgy), get_note_tab(_("Contacts"), pixmap_stealth_on) , FALSE, FALSE, 0);
			gtk_box_pack_start(GTK_BOX(widgy), gtk_label_new("    "), FALSE, FALSE, 0);
			subwidgy=create_quick_access_bar();
			gtk_box_pack_start(GTK_BOX(widgy),subwidgy , FALSE, FALSE, 0);
			gtk_widget_show_all(widgy);
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),get_yab_panel(),
				widgy  );
			quick_access=subwidgy;
		} else {
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),get_yab_panel(),
			get_note_tab(_("Contacts"), pixmap_stealth_on)   );
			quick_access=gtk_label_new("");
		}
	}





	gtk_notebook_prepend_page(GTK_NOTEBOOK(notebook),chatframe,
	get_note_tab(_("Chat"), pixmap_pm_join)  );

	if ( show_tabs_at_bottom) {
		gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_BOTTOM);
	}

	 gtk_widget_show_all(notebook); 
	/* the following line must be here or we get GTK-Critical messages */
 	gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook),0);
	return notebook;

}




void append_buddy_to_clist(gpointer key, gpointer value,
        gpointer user_data) {

	int is_tmp_friend=0;
   	char user[64]="";
	char grpinfo[72];
	char stat[304]="";
	char mainstat[256]="";
	char game[96]="";
	char idle[16]="";
	char launchstat[32]="";
	gchar *rd;
	GdkPixbuf *imbuf=NULL;
	GdkPixbuf *imbuf2=NULL;
	GdkPixbuf *simbuf2=NULL;
	GdkPixbuf *bufgame=NULL;
	GdkPixbuf *bufsms=NULL;
	GdkPixbuf *bufcam=NULL;
	GdkPixbuf *buflc=NULL;
	GdkPixbuf *bufchat=NULL;
	GdkPixbuf *bufstealth=NULL;
	int f_online=0;
	GtkTreeIter iter;
	GtkTreeIter pariter;
	GtkTreePath *tp;
	GtkTreeIter mobiter;
	GtkTreeIter mobpariter;
	GtkTreePath *mobtp;
	GtkTreeModel *model;
	model =(GtkTreeModel *)user_data;
	struct yahoo_friend *FRIEND_OBJECT=value;
	gchar *mobile_grp;
	int add_mobile=0;

	if (!key) {return; }
	if (!value) {return;}
	if (! model) {return;}
	friends_online_color.red   = 0x92 * ( 65535 / 255 );
	friends_online_color.green = 0x4b * ( 65535 / 255 );
	friends_online_color.blue  = 0xc1 * ( 65535 / 255 );

	if (!FRIEND_OBJECT->buddy_group) {
		if (FRIEND_OBJECT->mobile_list) {
			FRIEND_OBJECT->buddy_group=g_strdup("Y! Mobile Messenger");
		} else { FRIEND_OBJECT->buddy_group=g_strdup("Buddies");}
	}
	tp=find_buddy_group( FRIEND_OBJECT->buddy_group, model);
	if (!tp) {return;}
	if (!gtk_tree_model_get_iter(model, &pariter, tp  )) {return;}
	gtk_tree_store_append(GTK_TREE_STORE(model), &iter, &pariter);

	mobile_grp=g_strdup("Y! Mobile Messenger");
 
	if (strcmp(FRIEND_OBJECT->buddy_group, mobile_grp) && FRIEND_OBJECT->mobile_list) {
		mobtp=find_buddy_group( mobile_grp, model);
		if (mobtp) {
			if (gtk_tree_model_get_iter(model, &mobpariter, mobtp  )) {
				gtk_tree_store_append(GTK_TREE_STORE(model), &mobiter, &mobpariter);
				add_mobile=1;
			}
		}
	}


	if (strcmp(FRIEND_OBJECT->buddy_group,"~[Temporary Friends]~")==0) {
		is_tmp_friend=1;
		friends_online_color.red   = 0xa8 * ( 65535 / 255 );
		friends_online_color.green = 0x3f * ( 65535 / 255 );
		friends_online_color.blue  = 0x3f * ( 65535 / 255 );

	}	


	if (is_tmp_friend || find_online_friend(key))  {
		int whichicon=0;
		f_online=1;
		if (! is_tmp_friend) {num_buddies_online++;}
		snprintf(user, 62, " %s",(char *)key);
		if (FRIEND_OBJECT->away) {whichicon=1;}
		if (FRIEND_OBJECT->idle) {whichicon=2;}
		if (is_tmp_friend) {whichicon=3;}	



		if (FRIEND_OBJECT->avatar) {
			if (download_yavatar(FRIEND_OBJECT->avatar, 0, NULL)) {
				char ava_file[256];
				snprintf(ava_file,254,"%s/yavatars/%s.small.%s", 
					GYACH_CFG_DIR, FRIEND_OBJECT->avatar, avatar_filetype );
				imbuf2=gdk_pixbuf_new_from_file(ava_file, NULL);

			}
		}


		if (imbuf2) {
				simbuf2=gdk_pixbuf_scale_simple( imbuf2, 23, 23,
				GDK_INTERP_BILINEAR );
				/* printf("simbuf created:  %s\n", simbuf2?"True":"False"); fflush(stdout); */
		}

		if (whichicon==1) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_away);
									}
		else if (whichicon==2) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_idle);
					 					 }
		else if (whichicon==3) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_pm_buzz);
					 					 }
		else {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_online);
				}
						} else  {  /* buddy offline */
	snprintf(user,62, "[%s]",(char *)key);
	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_buddy_offline);
							   }
	rd=g_strdup(key);

	sprintf(stat,"%s", " "); 
	snprintf(mainstat,254, "%s", FRIEND_OBJECT->main_stat);
	if (strlen(FRIEND_OBJECT->main_stat)>65) {
		snprintf(mainstat,60, "%s", FRIEND_OBJECT->main_stat);
		strcat(mainstat,"\n");
		strncat(mainstat,FRIEND_OBJECT->main_stat+59, 60);
		if (strlen(FRIEND_OBJECT->main_stat)>120) {
			strcat(mainstat,"\n");
			strncat(mainstat,FRIEND_OBJECT->main_stat+119, 64);
		}
		if (strlen(FRIEND_OBJECT->main_stat)>184) {
			strcat(mainstat,"\n");
			strncat(mainstat,FRIEND_OBJECT->main_stat+183, 66);
		}
	}

	snprintf(idle,15, "%s", FRIEND_OBJECT->idle_stat);
	snprintf(game,95, "%s", FRIEND_OBJECT->game_stat);
	snprintf(launchstat,31, "%s", FRIEND_OBJECT->radio_stat);

	if (strlen(mainstat)>0)  {snprintf(stat,254, "%s",mainstat); }

	if (strlen(idle)>0)  {
		strcat(stat," [");
		if (FRIEND_OBJECT->idle) {
			strncat(stat,_("Idle"), 15);
			strcat(stat,": ");
		}
		strncat(stat,idle, 16);
		strcat(stat,"] ");
						   }

	if (FRIEND_OBJECT->stealth>0) {
		if (FRIEND_OBJECT->stealth==1) {
			bufstealth=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_on);
		} else {
			bufstealth=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_off);
		}
											   } 

	if (FRIEND_OBJECT->inchat) {
			bufchat=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_chat);
											   } 
	if (FRIEND_OBJECT->insms) {
			bufsms=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_sms);
											   } 
	if (FRIEND_OBJECT->ingames) {
			bufgame=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_games);
											   } 
	if (FRIEND_OBJECT->launchcast) {
			buflc=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_launchcast);
											   } 
	if (FRIEND_OBJECT->webcam) {
			bufcam=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_webcam);
											   } 

	num_buddies++;

			gtk_tree_store_set(GTK_TREE_STORE(model), 
				&iter, 
				GYDOUBLE_AVA, simbuf2,
				/* GYDOUBLE_AVA_TOGGLE, ( simbuf2 && show_yavatars) ?TRUE:FALSE,  */
				GYDOUBLE_AVA_TOGGLE, show_yavatars ?TRUE:FALSE, 
				GYDOUBLE_PIX, imbuf,
				GYDOUBLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
				GYDOUBLE_LABEL, user,
				GYDOUBLE_TOP, FRIEND_OBJECT->buddy_group,
				GYDOUBLE_COLOR, f_online?&friends_online_color:NULL, 

				GYDOUBLE_STEALTH_PIX,  bufstealth,
				GYDOUBLE_STEALTH_TOGGLE,  bufstealth?TRUE:FALSE, 
				GYDOUBLE_SMS_PIX,  bufsms,
				GYDOUBLE_SMS_TOGGLE,  bufsms?TRUE:FALSE, 
				GYDOUBLE_CHAT_PIX,  bufchat,
				GYDOUBLE_CHAT_TOGGLE,  bufchat?TRUE:FALSE, 
				GYDOUBLE_WEBCAM_PIX,  bufcam,
				GYDOUBLE_WEBCAM_TOGGLE,  bufcam?TRUE:FALSE, 
				GYDOUBLE_GAMES_PIX,  bufgame,
				GYDOUBLE_GAMES_TOGGLE,  bufgame?TRUE:FALSE, 
				GYDOUBLE_LC_PIX,  buflc,
				GYDOUBLE_LC_TOGGLE,  buflc?TRUE:FALSE, 
				GYDOUBLE_COL1, rd,
				GYDOUBLE_COL2, stat,
				GYDOUBLE_GAMES_COL,  game,
				GYDOUBLE_LC_COL,  launchstat,
				 -1);

	snprintf(grpinfo, 70, "%s  (%d)", FRIEND_OBJECT->buddy_group,
	gtk_tree_model_iter_n_children(model, &pariter) );
	gtk_tree_store_set(GTK_TREE_STORE(model), 
		&pariter, 
		GYDOUBLE_LABEL, grpinfo,  -1);

	if (add_mobile) {

			gtk_tree_store_set(GTK_TREE_STORE(model), 
				&mobiter, 
				GYDOUBLE_AVA, simbuf2,
				/* GYDOUBLE_AVA_TOGGLE, ( simbuf2 && show_yavatars) ?TRUE:FALSE,  */
				GYDOUBLE_AVA_TOGGLE, show_yavatars ?TRUE:FALSE, 
				GYDOUBLE_PIX, imbuf,
				GYDOUBLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
				GYDOUBLE_LABEL, user,
				GYDOUBLE_TOP, mobile_grp,
				GYDOUBLE_COLOR, f_online?&friends_online_color:NULL, 

				GYDOUBLE_STEALTH_PIX,  bufstealth,
				GYDOUBLE_STEALTH_TOGGLE,  bufstealth?TRUE:FALSE, 
				GYDOUBLE_SMS_PIX,  bufsms,
				GYDOUBLE_SMS_TOGGLE,  bufsms?TRUE:FALSE, 
				GYDOUBLE_CHAT_PIX,  bufchat,
				GYDOUBLE_CHAT_TOGGLE,  bufchat?TRUE:FALSE, 
				GYDOUBLE_WEBCAM_PIX,  bufcam,
				GYDOUBLE_WEBCAM_TOGGLE,  bufcam?TRUE:FALSE, 
				GYDOUBLE_GAMES_PIX,  bufgame,
				GYDOUBLE_GAMES_TOGGLE,  bufgame?TRUE:FALSE, 
				GYDOUBLE_LC_PIX,  buflc,
				GYDOUBLE_LC_TOGGLE,  buflc?TRUE:FALSE, 
				GYDOUBLE_COL1, rd,
				GYDOUBLE_COL2, stat,
				GYDOUBLE_GAMES_COL,  game,
				GYDOUBLE_LC_COL,  launchstat,
				 -1);

	snprintf(grpinfo, 70, "%s  (%d)", mobile_grp,
	gtk_tree_model_iter_n_children(model, &mobpariter) );
	gtk_tree_store_set(GTK_TREE_STORE(model), 
		&mobpariter, 
		GYDOUBLE_LABEL, grpinfo,  -1);
	}

	if (imbuf) {g_object_unref(imbuf);}
	if (simbuf2) {g_object_unref(simbuf2);}
	if (imbuf2) {g_object_unref(imbuf2);}
	if (bufchat) {g_object_unref(bufchat);}
	if (buflc) {g_object_unref(buflc);}
	if (bufcam) {g_object_unref(bufcam);}
	if (bufsms) {g_object_unref(bufsms);}
	if (bufgame) {g_object_unref(bufgame);}
	if (bufstealth) {g_object_unref(bufstealth);}
	g_free(rd);
	g_free(mobile_grp);
}



void update_buddy_clist()  {
		if (buddy_status) {
			GtkTreeModel *model;
			char budcount[56];			
			num_buddies_online=0;
			num_buddies=0;
			reset_buddy_groups();		
			model=freeze_treeview(GTK_WIDGET(bd_tree));
			gtk_tree_store_clear(GTK_TREE_STORE(model));
			g_hash_table_foreach(buddy_status, append_buddy_to_clist, model );
			unfreeze_treeview(GTK_WIDGET(bd_tree), model);
			snprintf(budcount,54, "%s : %d          %s : %d", 
				_("Friends"), num_buddies, _("Online"), num_buddies_online
			);
			gtk_label_set_text(GTK_LABEL(buddy_list_label), budcount);
			trayicon_buddy_count(num_buddies, num_buddies_online);
			gtk_widget_show_all (GTK_WIDGET(bd_tree));			
			gtk_tree_view_expand_all (GTK_TREE_VIEW(bd_tree));
	if (selected_buddy_group)  {free(selected_buddy_group); selected_buddy_group=NULL;}
	if (selected_buddy)  {free(selected_buddy); selected_buddy=NULL;}
		}

}



/* added: PhrozenSmoke, some all-purpose dialogs */

gboolean on_close_ok_dialog            (GtkWidget       *widget,                       
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;
	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "textfocus");
	if (tmp_widget) {
		int sposs=0;
		if (gtk_object_get_data (GTK_OBJECT (tmp_widget), "is_gtk_entry") != NULL) {
			sposs=gtk_editable_get_position(GTK_EDITABLE(tmp_widget));
		}
		gtk_widget_grab_focus(GTK_WIDGET(tmp_widget));
		if (sposs > 0 ) {gtk_editable_set_position(GTK_EDITABLE(tmp_widget), sposs);}
	}

	tmp_widget=NULL;
	tmp_widget = lookup_widget( widget, "mywindow" );
	if (tmp_widget)  {gtk_widget_destroy( tmp_widget ); }

	if (user_data) {
		PM_SESSION *pms=user_data;
		if (pms->pm_window) {focus_pm_entry(pms->pm_window);}
	}

	return( TRUE );
}


gboolean on_close_ok_dialogw             (GtkWidget       *widget,       
                                        GdkEvent        *event,                
                                        gpointer         user_data)
{
	return on_close_ok_dialog(widget,user_data);
}


void show_ok_dialog(char *mymsg)  {

	GtkWidget *dwindow;
	GtkWidget *dvbox;
	GtkWidget *dlbox;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  INFO-DIALOG MESSAGE: \n'%s'\n\n", ctime(&time_llnow), _(mymsg));
			fflush( capture_fp );
			}

	if (dialogs_opening_too_fast()) {
		if (num_dialogs_too_fast<11) {
			char tmsgbuf[200]="";
			snprintf(tmsgbuf, 193, "** Info Message: %s", _(mymsg));
			strcat( tmsgbuf," **\n");
			append_to_textbox_color( chat_window, NULL, tmsgbuf);
		}
		return;
	}

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (dwindow), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Message"));
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);
  //gtk_window_set_modal (GTK_WINDOW (dwindow), TRUE);
  /* gtk_window_set_default_size(GTK_WINDOW (dwindow), 268,105);  */

  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);  

	dlabel=gtk_label_new("");
	gtk_label_set_text(GTK_LABEL(dlabel),_(mymsg)); /* for gettext support, gchar */
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
	gtk_label_set_selectable (GTK_LABEL (dlabel), 1);

	dbutton=get_pixmapped_button(_("OK"), GTK_STOCK_YES);
	set_tooltip(dbutton,_("OK"));
 	 gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);

	dlbox = gtk_hbox_new (FALSE, 2);
  	gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);

  	gtk_box_pack_start (GTK_BOX (dlbox), dlabel, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);

	gtk_box_pack_start (GTK_BOX (dvbox), dbutton, FALSE, FALSE, 8);

  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog), NULL);

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

gtk_widget_show_all(dwindow);

}




GtkWidget *show_confirm_dialog_config(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel, int with_callback)  {

	GtkWidget *dwindow;
	GtkWidget *dlbox;
	GtkWidget *dvbox;
	GtkWidget *dhbox;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;
	GtkWidget *cbutton;

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  CONFIRMATION-DIALOG MESSAGE: \n'%s'\n\n", ctime(&time_llnow), _(mymsg));
			fflush( capture_fp );
			}

	if (dialogs_opening_too_fast()) {
		if (num_dialogs_too_fast<11) {
			char tmsgbuf[200]="";
			snprintf(tmsgbuf, 193, "** Question Message: %s", _(mymsg));
			strcat( tmsgbuf," **\n");
			append_to_textbox_color( chat_window, NULL, tmsgbuf);
		}
		return NULL;
	}

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (dwindow), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Question"));
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);
 // gtk_window_set_modal (GTK_WINDOW (dwindow), TRUE);
  /* gtk_window_set_default_size(GTK_WINDOW (dwindow), 350,105);  */

  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);

	dlabel=gtk_label_new("");
	gtk_label_set_text(GTK_LABEL(dlabel),_(mymsg)); /* for gettext support, gchar */
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
	gtk_label_set_selectable (GTK_LABEL (dlabel), 1);


	dlbox = gtk_hbox_new (FALSE, 2);
  	gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);

  	gtk_box_pack_start (GTK_BOX (dlbox), dlabel, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);


  dhbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (dvbox), dhbox, FALSE, FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dhbox), 6);

	dbutton=get_pixmapped_button(_(okbuttonlabel), GTK_STOCK_YES);
	set_tooltip(dbutton,_(okbuttonlabel));
  gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);
	cbutton=get_pixmapped_button(_(cancelbuttonlabel), GTK_STOCK_CANCEL);
	set_tooltip(cbutton,_(cancelbuttonlabel));
  gtk_object_set_data (GTK_OBJECT (cbutton), "mywindow", dwindow);
  gtk_object_set_data (GTK_OBJECT (dbutton), "cancel", cbutton);

  gtk_box_pack_start (GTK_BOX (dhbox), dbutton, FALSE, FALSE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), gtk_label_new(" "), TRUE, TRUE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), cbutton, FALSE, FALSE,2);

  if (with_callback) {
  gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog), NULL);
						}

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

gtk_widget_show_all(dwindow);
return dbutton;

}




GtkWidget *show_confirm_dialog(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel)  {
return show_confirm_dialog_config(mymsg,okbuttonlabel, cancelbuttonlabel, 1);
}



GtkWidget *show_input_dialog(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel)  {

	GtkWidget *dwindow;
	GtkWidget *dvbox;
	GtkWidget *dtbox;
	GtkWidget *dentry;
	GtkWidget *dhbox;
	GtkWidget *dhextra;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;
	GtkWidget *cbutton;

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (dwindow), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_window_set_title (GTK_WINDOW (dwindow), "GYach Enhanced");
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);
  //gtk_window_set_modal (GTK_WINDOW (dwindow), TRUE);
  /* gtk_window_set_default_size(GTK_WINDOW (dwindow), 300,105);  */

  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);

  dtbox = gtk_hbox_new (FALSE, 2);

  	gtk_box_pack_start (GTK_BOX (dvbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_FILL, GTK_ICON_SIZE_DND)), FALSE, FALSE , 3);

  gtk_box_pack_start (GTK_BOX (dvbox), dtbox, FALSE, FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dtbox), 5);


	dlabel=gtk_label_new("");
	gtk_label_set_text(GTK_LABEL(dlabel),_(mymsg)); /* for gettext support, gchar */
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
  gtk_box_pack_start (GTK_BOX (dtbox), dlabel, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (dtbox), gtk_label_new(":  "), FALSE, FALSE, 1);
	dentry=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(dentry),62);
	gtk_widget_set_usize(dentry, 255, -2);
  gtk_box_pack_start (GTK_BOX (dtbox), dentry, TRUE, TRUE, 2);


  dhextra = gtk_hbox_new (FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dhextra), 3);
  gtk_box_pack_start (GTK_BOX (dvbox), dhextra, FALSE, FALSE, 2);


  dhbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (dvbox), dhbox, FALSE, FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (dhbox), 3);

	dbutton=get_pixmapped_button(_(okbuttonlabel), GTK_STOCK_YES);
	set_tooltip(dbutton,_(okbuttonlabel));
  gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);
  gtk_object_set_data (GTK_OBJECT (dbutton), "myentry", dentry);
  gtk_object_set_data (GTK_OBJECT (dbutton), "dhextra", dhextra);
	cbutton=get_pixmapped_button(_(cancelbuttonlabel), GTK_STOCK_CANCEL);
	set_tooltip(cbutton,_(cancelbuttonlabel));
  gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);

  gtk_box_pack_start (GTK_BOX (dhbox), dbutton, FALSE, FALSE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), gtk_label_new(" "), TRUE, TRUE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), cbutton, FALSE, FALSE,2);

  gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog), NULL);

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

gtk_widget_show_all(dwindow);
return dbutton;

}


GtkWidget *create_fileselection2 ()
{
  GtkWidget *fileselection;
  GtkWidget *file_ok_button;
  GtkWidget *file_cancel_button;

  fileselection = gtk_file_selection_new (_("Select A File"));

	/*  gtk_box_pack_start (GTK_BOX (GTK_FILE_SELECTION(fileselection)->action_area), gtk_label_new("HERE"), FALSE, FALSE,2); */


  gtk_window_set_wmclass (GTK_WINDOW (fileselection), "gyachEDialog", "GyachE"); 
  gtk_object_set_data (GTK_OBJECT (fileselection), "mywindow", fileselection);
  gtk_container_set_border_width (GTK_CONTAINER (fileselection), 10);
  gtk_window_set_position (GTK_WINDOW (fileselection), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fileselection), TRUE);

  file_ok_button = GTK_FILE_SELECTION (fileselection)->ok_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "file_ok_button", file_ok_button);
  gtk_object_set_data (GTK_OBJECT (file_ok_button), "mywindow", fileselection);
  gtk_widget_show (file_ok_button);
  GTK_WIDGET_SET_FLAGS (file_ok_button, GTK_CAN_DEFAULT);

  file_cancel_button = GTK_FILE_SELECTION (fileselection)->cancel_button;
  gtk_object_set_data (GTK_OBJECT (fileselection), "file_cancel_button", file_cancel_button);
  gtk_object_set_data (GTK_OBJECT (file_cancel_button), "mywindow", fileselection);
  gtk_widget_show (file_cancel_button);
  GTK_WIDGET_SET_FLAGS (file_cancel_button, GTK_CAN_DEFAULT);


  gtk_signal_connect (GTK_OBJECT (file_cancel_button), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (fileselection), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

  if (last_file_selected != NULL) {gtk_file_selection_set_filename(GTK_FILE_SELECTION (fileselection),  last_file_selected );}
  gtk_widget_show_all(fileselection);
  return file_ok_button;
}



GtkWidget *get_smiley_button(char *filename, int allow_fail) {
	GtkWidget *button;
	GtkWidget *bicon;

	bicon=load_smiley_icon(filename);
	if (bicon) {
		button=gtk_button_new();
		gtk_container_add(GTK_CONTAINER(button), bicon);
		gtk_container_set_border_width(GTK_CONTAINER(button), 2);
				} else {
					if (! allow_fail) {button=gtk_button_new_with_label("8-)");}
					else {return NULL;}
						  }
	if (allow_fail) {gtk_button_set_relief(GTK_BUTTON(button),GTK_RELIEF_NONE);}
	gtk_widget_show_all(button);
	return button;
}


void append_smiley_to_box(GtkWidget *widget, gpointer user_data) {
	gint sipos;
	GtkWidget *tmp_widget;
	GtkWidget *tentry;
	char *is_entry=NULL;
	char *smiley_t=NULL;

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget),"mywindow");
	tentry=gtk_object_get_data(GTK_OBJECT(widget),"textwidget");
	is_entry=gtk_object_get_data(GTK_OBJECT(widget),"is_entry");
	smiley_t=gtk_object_get_data(GTK_OBJECT(widget),"smiley_tag");

	if (tentry && smiley_t)  {
		if (is_entry) {
		sipos=gtk_editable_get_position(GTK_EDITABLE(tentry));
		gtk_editable_insert_text(GTK_EDITABLE(tentry),smiley_t, strlen(smiley_t), &sipos );
		gtk_widget_grab_focus(GTK_WIDGET(tentry));
		gtk_editable_set_position(GTK_EDITABLE(tentry), sipos);
							 }   else {  /* textview - textbuffer */		
		GtkTextBuffer *tbe=NULL;
		tbe=gtk_object_get_data(GTK_OBJECT(tentry), "textbuffer");
		gtk_text_buffer_insert_at_cursor(tbe, smiley_t,strlen(smiley_t));
		gtk_widget_grab_focus(GTK_WIDGET(tentry));
										  }
														 }

	if (tmp_widget) {
		if (gtk_object_get_data(GTK_OBJECT(widget),"mainW")) {
			gtk_widget_hide( tmp_widget );
													     } else {
			on_close_ok_dialog(tmp_widget,NULL);
														 	}
						  }
}


gboolean on_close_ok_dialog2            (GtkWidget       *widget,                       
                                        gpointer         user_data)
{
	GtkWidget *tmp_widget;

	tmp_widget = lookup_widget( widget, "mywindow" );
	if (tmp_widget)  {
		gtk_widget_hide( tmp_widget );
						    }
	return( TRUE );
}


char *sm_colors_buttons[] = {
	"black","red",	"blue",	"green",
	"purple","pink","cyan",	"orange",
	"yellow","gray","i","b",NULL
};
char *sm_colors_store[] = { /* mirrors above, holds strduped strings */
	NULL,NULL,NULL,	NULL,
	NULL,NULL,NULL,	NULL,
	NULL,NULL,NULL,NULL,NULL
};

GtkWidget *append_pmcol_button( GtkWidget *chbox, char **pm ) {
	GtkWidget *nbutton;
	GtkWidget *imimage=NULL;
	GdkPixbuf *imbuf=NULL;

	nbutton = gtk_button_new();
  	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pm);
  	imimage=gtk_image_new_from_pixbuf(imbuf);
	if (! imimage) {imimage=gtk_label_new(" ");}
	gtk_container_add( GTK_CONTAINER( nbutton ), imimage );
	gtk_button_set_relief(GTK_BUTTON(nbutton), GTK_RELIEF_NONE);
	gtk_widget_set_usize( nbutton, 26, 26 );
	return( nbutton );
}

GtkWidget *show_smiley_window(GtkWidget *textwidget, int is_entry, int mainW) {
	char color[20];
	char **pm_color = color_xpm;
	char **cptr = sm_colors_buttons;
	int colorstore=0;
	GtkWidget *smile_window;
	GtkWidget *vbox;
	GtkWidget *cbutton;
	GtkWidget *hbox;
	GtkWidget *iconbutton;
	int colcount=0;
	int i=0;
	int havewide=0;
 	char entry_val[4]="";

	snprintf(entry_val, 3, "%d",is_entry);
  	if (!mainW) {smile_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);}
	else {smile_window = gtk_window_new (GTK_WINDOW_POPUP);}
  	gtk_object_set_data (GTK_OBJECT (smile_window), "mywindow", smile_window);
	gtk_window_set_wmclass (GTK_WINDOW (smile_window), "gyachECaritas", "GyachE");
  	gtk_window_set_title (GTK_WINDOW (smile_window), _("Colors & Emoticons") );
  	gtk_window_set_policy (GTK_WINDOW (smile_window), TRUE, TRUE, FALSE);
  	gtk_window_set_position (GTK_WINDOW (smile_window), GTK_WIN_POS_MOUSE);
  	gtk_window_set_modal (GTK_WINDOW (smile_window), TRUE);

	if ( ! color_buttons_created ) {
		/* create the buttons here */
		color_xpm[0] = strdup( "21 21 1 1" );
		color_xpm[1] = color;
		for( i = 0; i < 21 ; i++ ) {
			color_xpm[i+2] = strdup( "xxxxxxxxxxxxxxxxxxxxx" );
		}
	color_buttons_created=1;
	} else {color_xpm[1] = color;}
	i=0;
	vbox=gtk_vbox_new(FALSE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
	gtk_container_add (GTK_CONTAINER (smile_window), vbox); 

		hbox=gtk_hbox_new(FALSE,0);
		gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);

	while (i<85)  {       /* smileys */
		if (i>84) {break;}
		if (smiley_files[i]==NULL) {  break;}
		/* Smiley 06.gif is really wide and awkward, so skip it and add it last */
		if (!strcmp(smiley_files[i],"06")) {i++; havewide=1; continue;}

		iconbutton=get_smiley_button(smiley_files[i], 1);
		if (iconbutton) {
			colcount++;
			set_tooltip(iconbutton,smiley_tags[i]);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "mywindow", smile_window);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "textwidget", textwidget);
			if (is_entry) {
					gtk_object_set_data (GTK_OBJECT (iconbutton), "is_entry", entry_val); 
								}
			if (mainW) {gtk_object_set_data (GTK_OBJECT (iconbutton), "mainW", "ok");}
			gtk_object_set_data (GTK_OBJECT (iconbutton), "smiley_tag", smiley_tags[i]);
			gtk_box_pack_start (GTK_BOX (hbox),iconbutton , TRUE, TRUE, 1);
  			gtk_signal_connect (GTK_OBJECT (iconbutton), "clicked",
                      GTK_SIGNAL_FUNC (append_smiley_to_box), NULL);
								} 
		i++;
		
		if (colcount==10) {  
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
			hbox=gtk_hbox_new(FALSE,0);
			gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
			colcount=0;
					    			}
				 }

		if (havewide) {
		iconbutton=get_smiley_button("06", 1);
		if (iconbutton) {
			colcount++;
			set_tooltip(iconbutton,">:D<");
			gtk_object_set_data (GTK_OBJECT (iconbutton), "mywindow", smile_window);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "textwidget", textwidget);
			if (is_entry) {
					gtk_object_set_data (GTK_OBJECT (iconbutton), "is_entry", entry_val); 
								}
			if (mainW) {gtk_object_set_data (GTK_OBJECT (iconbutton), "mainW", "ok");}
			gtk_object_set_data (GTK_OBJECT (iconbutton), "smiley_tag", ">:D<");
			gtk_box_pack_start (GTK_BOX (hbox),iconbutton , TRUE, TRUE, 1);
  			gtk_signal_connect (GTK_OBJECT (iconbutton), "clicked",
                      GTK_SIGNAL_FUNC (append_smiley_to_box), NULL);
								} 
		if (colcount==10) {  
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
			hbox=gtk_hbox_new(FALSE,0);
			gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
			colcount=0;
					    			}
		}

			if (colcount>1) {  
				gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
				hbox=gtk_hbox_new(FALSE,0);
				gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
				colcount=0;
					    			  }

		while( *cptr ) {  /* colors */
			char ccolbuf[16]="";
			char *colptr=NULL;
			if (! strcmp(*cptr,"i")) {
				iconbutton = append_pmcol_button( hbox, pixmap_italic );
				if (!sm_colors_store[colorstore]) {
					snprintf(ccolbuf,15,"%s", "<i></i>");
					sm_colors_store[colorstore]=strdup(ccolbuf);
				}
				colptr=sm_colors_store[colorstore];
			} else if (! strcmp(*cptr,"b")) { 
				iconbutton = append_pmcol_button( hbox, pixmap_bold );
				if (!sm_colors_store[colorstore]) {
					snprintf(ccolbuf,15,"%s", "<b></b>" );
					sm_colors_store[colorstore]=strdup(ccolbuf);
				}
				colptr=sm_colors_store[colorstore];
			} else {
				snprintf( color, 19, "x	c %s", *cptr );
				iconbutton = append_pmcol_button( hbox, pm_color );
				if (!sm_colors_store[colorstore]) {
					snprintf(ccolbuf,15,"<%s>", *cptr );
					sm_colors_store[colorstore]=strdup(ccolbuf);
					}
				colptr=sm_colors_store[colorstore];
				}
			set_tooltip(iconbutton,colptr);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "mywindow", smile_window);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "textwidget", textwidget);
			if (is_entry) {
					gtk_object_set_data (GTK_OBJECT (iconbutton), "is_entry", entry_val); 
								} 
			if (mainW) {gtk_object_set_data (GTK_OBJECT (iconbutton), "mainW", "ok");}
			gtk_object_set_data (GTK_OBJECT (iconbutton), "smiley_tag", colptr );
			gtk_box_pack_start (GTK_BOX (hbox),iconbutton , FALSE, FALSE, 1);
  			gtk_signal_connect (GTK_OBJECT (iconbutton), "clicked",
                      GTK_SIGNAL_FUNC (append_smiley_to_box), NULL);
			cptr++;
			colcount++;
			colorstore++;
			if (colcount==12) {  
				gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
				hbox=gtk_hbox_new(FALSE,0);
				gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
				colcount=0;
					    			  }
		}

	if (colcount<13) {  
			gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new(""), TRUE, TRUE, 0);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
						  }

	cbutton=get_pixmapped_button(_("Cancel"), GTK_STOCK_CANCEL);
	set_tooltip(cbutton,_("Cancel"));
	gtk_box_pack_start (GTK_BOX (vbox), cbutton, FALSE, FALSE, 4);
  	gtk_object_set_data (GTK_OBJECT (cbutton), "mywindow", smile_window);
	gtk_object_set_data (GTK_OBJECT (cbutton), "textfocus", textwidget);
	gtk_object_set_data (GTK_OBJECT (smile_window), "textfocus", textwidget);

	if (!mainW) {
  	gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog), NULL);
  	gtk_signal_connect (GTK_OBJECT (smile_window), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);
			  } else {
  	gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog2), NULL);
				   }
	gtk_widget_show_all(smile_window);
	return smile_window;

}





void show_smiley_window_chat(GtkWidget *button, gpointer user_data) {
	GtkWidget *tentry;
	GtkWidget *smiles;
	tentry=gtk_object_get_data(GTK_OBJECT(button),"entry");
	if (tentry) {
		smiles=show_smiley_window(tentry,1,0);

				}
}

void show_smiley_window_pm(GtkWidget *button, gpointer user_data) {
	GtkWidget *tentry;
	GtkWidget *smiles;
	tentry=gtk_object_get_data(GTK_OBJECT(button),"entry");
	if (tentry) {
		GtkWidget *pmsmile;
		pmsmile=gtk_object_get_data (GTK_OBJECT (button), "pmsmile");
		if (pmsmile) {
			gtk_widget_show_all(pmsmile);
			return;
				  } else {
			if (gtk_object_get_data(GTK_OBJECT(tentry), "textbuffer") != NULL ) {
				smiles=show_smiley_window(tentry,0,0);
			 } else {  smiles=show_smiley_window(tentry,1,0); }
					    }
				}
}


GtkWidget *create_budlist_popup_menu() {
	GtkWidget *budlist_menu, *dummybutton, *buzz_friend;
	GtkWidget *im_menu, *getprofile, *files_menu, *shared_menu;
	GtkWidget *contact_info_menu, *webcam_menu, *sendcam;
	GtkWidget *gotouser, *inv_conf, *refresh, *moveto;
	GtkWidget *vis1,  *gameprofile, *viewava, *gameuser;
	GtkWidget *budinfo, *viewava2, *yab;
	budlist_menu = gtk_menu_new ();
	dummybutton=gtk_button_new();

  im_menu = gtk_image_menu_item_new_with_label (_("Instant Message"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(im_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), im_menu);

  budinfo = gtk_image_menu_item_new_with_label (_("Info"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(budinfo), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), budinfo);

  getprofile = gtk_image_menu_item_new_with_label (_("Profile"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(getprofile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), getprofile);

  gameprofile = gtk_image_menu_item_new_with_label (_(" View Game Scores... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gameprofile), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), gameprofile);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  viewava = gtk_image_menu_item_new_with_label (_("View full-size avatar"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(viewava), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), viewava);

  viewava2 = gtk_image_menu_item_new_with_label (_("View avatar animation"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(viewava2), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_COPY, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), viewava2);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  buzz_friend = gtk_image_menu_item_new_with_label (_("Buzz"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(buzz_friend), 
  GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), buzz_friend);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

/* added: PhrozenSmoke - file sending menu item */
  files_menu = gtk_image_menu_item_new_with_label (_(" Send File to User... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(files_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), files_menu);

  shared_menu = gtk_image_menu_item_new_with_label (_(" View Shared Files... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(shared_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), shared_menu);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  yab = gtk_image_menu_item_new_with_label(_("Edit Contact Information..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(yab), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_LEFT, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), yab);

  contact_info_menu = gtk_image_menu_item_new_with_label(_(" Send My Contact Information... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(contact_info_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_JUSTIFY_CENTER, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), contact_info_menu);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  webcam_menu = gtk_image_menu_item_new_with_label (_(" View Webcam... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(webcam_menu), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_100, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), webcam_menu);

  sendcam = gtk_image_menu_item_new_with_label (_(" Send Webcam Invitation... "));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(sendcam), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DND, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), sendcam);

#ifdef HAVE_WEBCAM
	gtk_widget_set_sensitive(webcam_menu,1);
	gtk_widget_set_sensitive(sendcam,1);
#endif

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  gotouser = gtk_image_menu_item_new_with_label (_("Join User In Room"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gotouser), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), gotouser);

  gameuser = gtk_image_menu_item_new_with_label (_("Join User In Game Room"));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(gameuser), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), gameuser);

  inv_conf = gtk_image_menu_item_new_with_label (_("Invite User to Room or Conference..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(inv_conf ), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu),inv_conf);

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  refresh = gtk_image_menu_item_new_with_label (_("Refresh Buddy List"));
  gtk_container_add (GTK_CONTAINER (budlist_menu), refresh);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(refresh), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU) ));

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());
  moveto = gtk_image_menu_item_new_with_label (_("Move to group..."));
  gtk_container_add (GTK_CONTAINER (budlist_menu), moveto);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(moveto), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_REDO, GTK_ICON_SIZE_MENU) ));

  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());


  vis1 = gtk_menu_item_new_with_label (_("Stealth Settings..."));
  gtk_container_add (GTK_CONTAINER (budlist_menu), vis1);

  gtk_signal_connect (GTK_OBJECT (vis1), "activate",
                      GTK_SIGNAL_FUNC (on_open_stealth_window),
                      " ");

  gtk_signal_connect (GTK_OBJECT (budinfo), "activate",
                      GTK_SIGNAL_FUNC (on_viewbuddyinfo),
                      " ");

  gtk_signal_connect (GTK_OBJECT (yab), "activate",
                      GTK_SIGNAL_FUNC (on_blist_get_yab ),
                     NULL);

  gtk_signal_connect (GTK_OBJECT (viewava), "activate",
                      GTK_SIGNAL_FUNC (on_viewavatar),
                     NULL);

  gtk_signal_connect (GTK_OBJECT (viewava2), "activate",
                      GTK_SIGNAL_FUNC (on_viewavatar),
                      viewava2);

  gtk_signal_connect (GTK_OBJECT (gameuser), "activate",
                      GTK_SIGNAL_FUNC (on_viewgameroom),
                      " ");

  gtk_signal_connect (GTK_OBJECT (gameprofile), "activate",
                      GTK_SIGNAL_FUNC (on_gameprofile),
                      " ");
  gtk_signal_connect (GTK_OBJECT (moveto), "activate",
                      GTK_SIGNAL_FUNC (on_move_buddy_to_group),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (shared_menu), "activate",
                      GTK_SIGNAL_FUNC (on_view_briefcase),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (contact_info_menu), "activate",
                      GTK_SIGNAL_FUNC (on_send_contact_info),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (refresh), "activate",
                      GTK_SIGNAL_FUNC (on_refresh_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (sendcam), "activate",
                      GTK_SIGNAL_FUNC (on_sendcaminvite),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (webcam_menu), "activate",
                      GTK_SIGNAL_FUNC (on_viewcam),
                      NULL);
  /* Do these 'by proxy' */
  gtk_signal_connect_object(GTK_OBJECT(im_menu), "activate",
				GTK_SIGNAL_FUNC(on_buddy_pm_activate),
				GTK_OBJECT(dummybutton));
  gtk_signal_connect_object(GTK_OBJECT(getprofile), "activate",
				GTK_SIGNAL_FUNC(on_buddy_profile_activate),
				GTK_OBJECT(dummybutton));
  gtk_signal_connect_object(GTK_OBJECT(files_menu), "activate",
				GTK_SIGNAL_FUNC(on_send_buddy_file),
				GTK_OBJECT(dummybutton));
  gtk_signal_connect_object(GTK_OBJECT(gotouser), "activate",
				GTK_SIGNAL_FUNC(on_buddy_join_in_chat),
				GTK_OBJECT(dummybutton));
  gtk_signal_connect_object(GTK_OBJECT(inv_conf), "activate",
				GTK_SIGNAL_FUNC(on_buddy_invite_to_chat),
				GTK_OBJECT(dummybutton));
  gtk_signal_connect_object(GTK_OBJECT(buzz_friend), "activate",
				GTK_SIGNAL_FUNC(on_buddy_buzz),
				GTK_OBJECT(dummybutton));

gtk_widget_show_all(budlist_menu);
return budlist_menu;
}


GtkWidget *create_budgrp_popup_menu() {
	GtkWidget *budlist_menu, *rnamegrp, *refresh;
	budlist_menu = gtk_menu_new ();

  rnamegrp = gtk_image_menu_item_new_with_label (_("Rename..."));
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rnamegrp), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_SELECT_FONT, GTK_ICON_SIZE_MENU) ));
  gtk_container_add (GTK_CONTAINER (budlist_menu), rnamegrp);
  /* separator */
  gtk_container_add (GTK_CONTAINER (budlist_menu), gtk_menu_item_new ());

  refresh = gtk_image_menu_item_new_with_label (_("Refresh Buddy List"));
  gtk_container_add (GTK_CONTAINER (budlist_menu), refresh);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(refresh), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU) ));

  gtk_signal_connect (GTK_OBJECT (rnamegrp), "activate",
                      GTK_SIGNAL_FUNC (on_buddy_list_rename),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (refresh), "activate",
                      GTK_SIGNAL_FUNC (on_refresh_activate),
                      NULL);
gtk_widget_show_all(budlist_menu);
return budlist_menu;
}



int hide_popup_timeout(GtkWidget *mypopup) {
	if (mypopup) {gtk_widget_destroy(GTK_WIDGET(mypopup));}
	return 0;
}

void show_popup_dialog(char *mytitle, char *mymsg, int myicon)  {
	char phead[165];
	char pmsg[276];
	GtkWidget *dwindow;
	GtkWidget *dvbox;
	GtkWidget *dlbox;
	GtkWidget *dlabel;	
	GtkWidget *dhead;	

	if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  POPUP-DIALOG MESSAGE: \n%s '%s'\n\n", ctime(&time_llnow), mytitle, mymsg);
			fflush( capture_fp );
	}

	if (dialogs_opening_too_fast()) {
		if (num_dialogs_too_fast<11) {
			char tmsgbuf[200]="";
			snprintf(tmsgbuf, 193, "** Info Message: %s", _(mymsg));
			strcat( tmsgbuf," **\n");
			append_to_textbox_color( chat_window, NULL, tmsgbuf);
		}
		return;
	}

   snprintf(phead, 150, "<span foreground=\"#0000AA\" size=\"larger\"><b>%s", mytitle);
   strcat(phead,"</b></span>");
   snprintf(pmsg, 264, "<span foreground=\"#000000\" size=\"smaller\">%s", mymsg);
   strcat(pmsg,"</span>");
  dwindow = gtk_window_new (GTK_WINDOW_POPUP);
  gtk_window_set_wmclass (GTK_WINDOW (dwindow), "gyachEPopup", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Message"));
  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);  
	dlabel=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(dlabel),pmsg); 
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
	dhead=gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(dhead),phead); 
	gtk_label_set_line_wrap(GTK_LABEL(dhead),1);
	gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
	dlbox = gtk_hbox_new (FALSE, 2); /* icon */
  	gtk_box_set_spacing (GTK_BOX (dlbox), 3);
  	gtk_box_set_spacing (GTK_BOX (dvbox), 6);
  	gtk_box_pack_start (GTK_BOX (dlbox), get_pm_icon(myicon, " ") , FALSE, FALSE , 2);
  	gtk_box_pack_start (GTK_BOX (dlbox), dhead, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 2);  
  	gtk_box_pack_start (GTK_BOX (dvbox), dlabel, TRUE, TRUE, 1);

gtk_widget_set_uposition(dwindow, gdk_screen_width()-150, gdk_screen_height()-150);
gtk_widget_show_all(dwindow);
gtk_widget_set_uposition(dwindow, gdk_screen_width()-dwindow->allocation.width-4, gdk_screen_height()-dwindow->allocation.height-74);
g_timeout_add( 7000, (void *)hide_popup_timeout, dwindow );
}



GtkWidget* create_stealth_window (char *who)
{
  GSList *stealth_group = NULL;
  GtkWidget *vbox1, *stealthconfwin, *dlabel;
  GtkWidget *hbox2, *imimage1, *picbox;
  GtkWidget *okbutt, *cancelbutt;
  GtkWidget *online, *offline, *permoff;
  char smessage[128];

  stealthconfwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (stealthconfwin), _("Stealth Settings"));
  gtk_window_set_wmclass (GTK_WINDOW (stealthconfwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW (stealthconfwin), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (stealthconfwin), TRUE);

  snprintf(smessage, 126, "'%s' %s:", who, _("sees me as"));

  vbox1 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (stealthconfwin), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  dlabel=gtk_label_new (smessage);
  gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox1), dlabel, FALSE, FALSE, 4);

  online = gtk_radio_button_new (NULL);
  picbox=gtk_hbox_new (FALSE, 2);
  imimage1=gtk_image_new();
  gtk_image_set_from_pixbuf(GTK_IMAGE(imimage1), gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_on));
  gtk_box_pack_start (GTK_BOX (picbox), imimage1, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(_("Online")), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(""), TRUE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER (online), picbox);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (online), stealth_group);
  stealth_group = gtk_radio_button_group (GTK_RADIO_BUTTON (online));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (online), TRUE);
  
  offline = gtk_radio_button_new (NULL);
  picbox=gtk_hbox_new (FALSE, 2);
  imimage1=gtk_image_new();
  gtk_image_set_from_pixbuf(GTK_IMAGE(imimage1), gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_off));
  gtk_box_pack_start (GTK_BOX (picbox), imimage1, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(_("Offline")), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(""), TRUE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER (offline), picbox);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (offline), stealth_group);
  stealth_group = gtk_radio_button_group (GTK_RADIO_BUTTON (offline));

  permoff = gtk_radio_button_new (NULL);
  picbox=gtk_hbox_new (FALSE, 2);
  imimage1=gtk_image_new();
  gtk_image_set_from_pixbuf(GTK_IMAGE(imimage1), gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_off));
  gtk_box_pack_start (GTK_BOX (picbox), imimage1, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(_("Permanently Offline")), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (picbox), gtk_label_new(""), TRUE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER (permoff), picbox);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (permoff), stealth_group);
  stealth_group = gtk_radio_button_group (GTK_RADIO_BUTTON (permoff));

  gtk_box_pack_start (GTK_BOX (vbox1), online, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), offline, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), permoff, FALSE, FALSE, 0);

  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

  gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new("              "), TRUE, TRUE, 0);

  okbutt = get_pixmapped_button(_("OK"), GTK_STOCK_YES);
  gtk_box_pack_start (GTK_BOX (hbox2), okbutt, FALSE, FALSE, 2);
  set_tooltip(okbutt,_("OK"));

  gtk_object_set_data (GTK_OBJECT (stealthconfwin), "online", online);
  gtk_object_set_data (GTK_OBJECT (stealthconfwin), "offline", offline);
  gtk_object_set_data (GTK_OBJECT (stealthconfwin), "permoff", permoff);
  gtk_object_set_data (GTK_OBJECT (okbutt), "mywindow", stealthconfwin);
  gtk_object_set_data (GTK_OBJECT (okbutt), "online", online);
  gtk_object_set_data (GTK_OBJECT (okbutt), "offline", offline);
  gtk_object_set_data (GTK_OBJECT (okbutt), "permoff", permoff);

  cancelbutt = get_pixmapped_button(_("Cancel"), GTK_STOCK_NO);
  gtk_box_pack_start (GTK_BOX (hbox2), cancelbutt, FALSE, FALSE, 2);
  set_tooltip(cancelbutt,_("Cancel"));
  gtk_object_set_data (GTK_OBJECT (cancelbutt), "mywindow", stealthconfwin);
  gtk_object_set_data (GTK_OBJECT (stealthconfwin), "mywindow", stealthconfwin);

  gtk_signal_connect (GTK_OBJECT (cancelbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_activate_stealth_window),
                      NULL);

  gtk_widget_show_all(stealthconfwin);
  return stealthconfwin;
}


void show_audibles_window(gpointer pms_sess) {
	GtkWidget *audibles_window;
	GtkWidget *vbox;
	GtkWidget *cbutton;
	GtkWidget *hbox;
	GtkWidget *iconbutton;
	GYAUDIBLE *sm_ptr;
	char toolts[256];
	int colcount=0;
	int i=0;
	

	init_audibles();
	sm_ptr = &gyache_audibles[0];

  	audibles_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  	gtk_object_set_data (GTK_OBJECT (audibles_window), "mywindow", audibles_window);
	gtk_window_set_wmclass (GTK_WINDOW (audibles_window), "gyachECaritas", "GyachE");
  	gtk_window_set_title (GTK_WINDOW (audibles_window), _("Audibles") );
  	gtk_window_set_policy (GTK_WINDOW (audibles_window), TRUE, TRUE, FALSE);
  	gtk_window_set_position (GTK_WINDOW (audibles_window), GTK_WIN_POS_MOUSE);
  	gtk_window_set_modal (GTK_WINDOW (audibles_window), TRUE);

	i=0;
	vbox=gtk_vbox_new(FALSE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (vbox), 3);
	gtk_container_add (GTK_CONTAINER (audibles_window), vbox); 

	hbox=gtk_hbox_new(FALSE,0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);

	while (sm_ptr->aud_file )  {       /* smileys */
		iconbutton=get_smiley_button(sm_ptr->aud_disk_name , 0);
		if (iconbutton) {
			colcount++;
			snprintf(toolts, 254, "\"%s\"\n\n[%s]", sm_ptr->aud_text?sm_ptr->aud_text:sm_ptr->aud_file, sm_ptr->aud_disk_name+8);
			set_tooltip(iconbutton,toolts );
			gtk_object_set_data (GTK_OBJECT (iconbutton), "mywindow", audibles_window);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "pms_sess", pms_sess);
			gtk_object_set_data (GTK_OBJECT (iconbutton), "smiley_tag", sm_ptr->aud_file );
			gtk_box_pack_start (GTK_BOX (hbox),iconbutton , TRUE, TRUE, 1);
  			gtk_signal_connect (GTK_OBJECT (iconbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_send_audible_cb), NULL);
								} 
		i++;
		
		if (colcount==8) {  
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, TRUE, 0);
			hbox=gtk_hbox_new(FALSE,0);
			gtk_container_set_border_width (GTK_CONTAINER (hbox), 1);
			colcount=0;
					    			}
		sm_ptr++;
				 }

	if (colcount<8) {  
			gtk_box_pack_start (GTK_BOX (hbox), gtk_label_new(""), TRUE, TRUE, 0);
			gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
						  }

	cbutton=get_pixmapped_button(_("Cancel"), GTK_STOCK_CANCEL);
	set_tooltip(cbutton,_("Cancel"));
	gtk_box_pack_start (GTK_BOX (vbox), cbutton, FALSE, FALSE, 4);
  	gtk_object_set_data (GTK_OBJECT (cbutton), "mywindow", audibles_window);

  	gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog ), pms_sess);
  	gtk_signal_connect (GTK_OBJECT (audibles_window), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), pms_sess);

	gtk_widget_show_all(audibles_window);
}


void show_credentials_box(GtkWidget *window, GdkPixbuf *imbuf) {
	GtkWidget *login_box=NULL;
	GtkWidget *cred_box, *cred_image, *dlabel;
	login_box=gtk_object_get_data (GTK_OBJECT (window), "mainbox");
	login_credentials=NULL;
	if (!login_box) {return;}
	if (!imbuf) {return;}

	cred_box=gtk_vbox_new(FALSE,0);
  	gtk_container_set_border_width (GTK_CONTAINER (cred_box), 3);
	gtk_box_pack_start (GTK_BOX (cred_box), gtk_hseparator_new(), FALSE, FALSE, 2);

  	dlabel=gtk_label_new (_("Your account has been temporarily locked due to too many failed login attempts. Please type the word shown below to unlock your account:"));
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);
  	gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  	gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  	gtk_box_pack_start (GTK_BOX (cred_box), dlabel, FALSE, FALSE, 4);

	login_credentials=gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(login_credentials), 20);
  	gtk_box_pack_start (GTK_BOX (cred_box), login_credentials, FALSE, FALSE, 2);

	cred_image=gtk_image_new_from_pixbuf(imbuf);
  	gtk_box_pack_start (GTK_BOX (cred_box), cred_image, FALSE, FALSE, 2);
	gtk_widget_show_all(cred_box);
	gtk_box_pack_start (GTK_BOX (login_box), cred_box, FALSE, FALSE, 2);
}


GtkWidget* create_yprivacy_window ()
{
  GtkWidget *vbox1, *yprivacywin, *dlabel;
  GtkWidget *hbox2;
  GtkWidget *okbutt, *cancelbutt;
  GtkWidget *gameson, *gameshs, *yonline;

  GtkWidget *acttext, *actbutton, *actbox;

  yprivacywin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (yprivacywin), _("Yahoo! Privacy Settings"));
  gtk_window_set_wmclass (GTK_WINDOW (yprivacywin), "gyachEDialog", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW (yprivacywin), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (yprivacywin), TRUE);

  vbox1 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (yprivacywin), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  dlabel=gtk_label_new (_("Yahoo! Privacy Settings"));
  gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox1), dlabel, FALSE, FALSE, 4);


  gameson=gtk_check_button_new_with_label (_("Allow users to see when I am in Games."));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gameson),FALSE);

  gameshs=gtk_check_button_new_with_label (_("Allow users to see my Game high scores."));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gameshs),FALSE);

  yonline=gtk_check_button_new_with_label (_("Allow Yahoo! websites to show when I'm online."));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(yonline),FALSE);

  gtk_box_pack_start (GTK_BOX (vbox1), gameson, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), gameshs, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox1), yonline, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 2);
  dlabel=gtk_label_new (_("Remove my name from this user's buddy list:"));
  gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox1), dlabel, FALSE, FALSE, 2);
  actbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), actbox, FALSE, FALSE, 2);
  acttext=gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(acttext), 60);
  gtk_box_pack_start (GTK_BOX (actbox), acttext, TRUE, TRUE, 2);
  actbutton=get_pixmapped_button(_("OK"), GTK_STOCK_OK);
  set_tooltip(actbutton,_("OK"));
  gtk_box_pack_start (GTK_BOX (actbox), actbutton, FALSE, FALSE, 2);
  gtk_object_set_data (GTK_OBJECT (actbutton), "entry", acttext);
  gtk_signal_connect (GTK_OBJECT (actbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_remove_me_cb),
                      NULL);

  gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 2);
  dlabel=gtk_label_new (_("I am permanently offline to this user:"));
  gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox1), dlabel, FALSE, FALSE, 2);
  actbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), actbox, FALSE, FALSE, 2);
  acttext=gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(acttext), 60);
  gtk_box_pack_start (GTK_BOX (actbox), acttext, TRUE, TRUE, 2);
  actbutton=get_pixmapped_button(_("OK"), GTK_STOCK_OK);
  set_tooltip(actbutton,_("OK"));
  gtk_box_pack_start (GTK_BOX (actbox), actbutton, FALSE, FALSE, 2);
  gtk_object_set_data (GTK_OBJECT (actbutton), "entry", acttext);
  gtk_signal_connect (GTK_OBJECT (actbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_perm_offline_input_cb),
                      NULL);

  gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 2);
  dlabel=gtk_label_new (_("I am NO longer permanently offline to this user:"));
  gtk_misc_set_alignment (GTK_MISC (dlabel), 0, 0.5);
  gtk_label_set_justify (GTK_LABEL (dlabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox1), dlabel, FALSE, FALSE, 2);
  actbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), actbox, FALSE, FALSE, 2);
  acttext=gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(acttext), 60);
  gtk_box_pack_start (GTK_BOX (actbox), acttext, TRUE, TRUE, 2);
  actbutton=get_pixmapped_button(_("OK"), GTK_STOCK_OK);
  set_tooltip(actbutton,_("OK"));
  gtk_box_pack_start (GTK_BOX (actbox), actbutton, FALSE, FALSE, 2);
  gtk_object_set_data (GTK_OBJECT (actbutton), "entry", acttext);
  gtk_signal_connect (GTK_OBJECT (actbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_perm_offline_input_cb),
                      acttext);
 
  hbox2 = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox1), gtk_label_new(" "), FALSE, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 5);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 2);

  okbutt = get_pixmapped_button(_("OK"), GTK_STOCK_YES);
  gtk_box_pack_start (GTK_BOX (hbox2), okbutt, FALSE, FALSE, 2);
  set_tooltip(okbutt,_("OK"));

  gtk_object_set_data (GTK_OBJECT (okbutt), "mywindow", yprivacywin);
  gtk_object_set_data (GTK_OBJECT (okbutt), "gameson", gameson);
  gtk_object_set_data (GTK_OBJECT (okbutt), "gameshs", gameshs);
  gtk_object_set_data (GTK_OBJECT (okbutt), "yonline", yonline);

  gtk_object_set_data (GTK_OBJECT (yprivacywin), "mywindow", yprivacywin);

  gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new("   "), TRUE, TRUE, 0);

  cancelbutt = get_pixmapped_button(_("Cancel"), GTK_STOCK_NO);
  gtk_box_pack_start (GTK_BOX (hbox2), cancelbutt, FALSE, FALSE, 2);
  set_tooltip(cancelbutt,_("Cancel"));
  gtk_object_set_data (GTK_OBJECT (cancelbutt), "mywindow", yprivacywin);

  gtk_signal_connect (GTK_OBJECT (cancelbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_activate_yprivacy),
                      NULL);

  gtk_widget_show_all(yprivacywin);
  return yprivacywin;
}



GtkWidget  *create_bimage_select_window ()
{
  GSList *bimage_group = NULL;
  GSList *bimage_size_group = NULL;
  GtkWidget *vbox1, *vbox2, *vbox3;
  GtkWidget *hbox1, *hbox2, *hbox3, *hbox4, *hbox5;
  GtkWidget *okbutt;

  bimdiag_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data(GTK_OBJECT(bimdiag_window),"mywindow", bimdiag_window);
  gtk_window_set_title (GTK_WINDOW (bimdiag_window), _("My Display Image"));
  gtk_window_set_wmclass (GTK_WINDOW (bimdiag_window), "gyachEDialog", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW (bimdiag_window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (bimdiag_window), TRUE);
	vbox1 = gtk_vbox_new (FALSE, 2);
	vbox2 = gtk_vbox_new (FALSE, 2);
	vbox3 = gtk_vbox_new (FALSE, 2);
	hbox1 = gtk_hbox_new (FALSE, 2);
	hbox2 = gtk_hbox_new (FALSE, 2);
	hbox3 = gtk_hbox_new (FALSE, 2);
	hbox4 = gtk_hbox_new (FALSE, 2);
	hbox5 = gtk_hbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (bimdiag_window), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

	bimdiag_radio1 = gtk_radio_button_new_with_label (bimage_group, _("Don't share a display image"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_radio1), bimage_group);
  	bimage_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_radio1));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_radio1, FALSE, FALSE, 2);

	bimdiag_radio2 = gtk_radio_button_new_with_label (bimage_group, _("Share my photo:"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_radio2), bimage_group);
  	bimage_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_radio2));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_radio2, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(""), TRUE, TRUE, 3);
  	bimdiag_image1=GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_DIALOG) );
	gtk_box_pack_start (GTK_BOX (hbox1), bimdiag_image1, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("  "), FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, FALSE, 3);
	okbutt=get_pixmapped_button(_("Browse..."), GTK_STOCK_OPEN); /* signal */
	set_tooltip(okbutt,_("Browse..."));
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_select_buddy_image),
                      NULL);
	gtk_box_pack_start (GTK_BOX (vbox2), okbutt, FALSE, FALSE, 3);
	bimdiag_entry1=gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(bimdiag_entry1), FALSE);
	gtk_box_pack_start (GTK_BOX (vbox2), bimdiag_entry1, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox2), gtk_label_new(""), TRUE, TRUE, 0);


	bimdiag_radio3 = gtk_radio_button_new_with_label (bimage_group, _("Share my avatar"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_radio3), bimage_group);
  	bimage_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_radio3));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_radio3, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new(""), TRUE, TRUE, 3);
  	bimdiag_image2=GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_DIALOG) );
	gtk_box_pack_start (GTK_BOX (hbox2), bimdiag_image2, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (hbox2), gtk_label_new("  "), FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (hbox2), vbox3, FALSE, FALSE, 3);
	okbutt=get_pixmapped_button(_("Refresh View"), GTK_STOCK_REFRESH); /* signal */
	set_tooltip(okbutt,_("Refresh View"));
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_refresh_bavatar),
                      NULL);
	gtk_box_pack_start (GTK_BOX (vbox3), okbutt, FALSE, FALSE, 3);
	okbutt=get_pixmapped_button(_("Edit / Create Avatar..."), GTK_STOCK_SELECT_COLOR); /* signal */
		set_tooltip(okbutt,_("Edit / Create Avatar..."));
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_goto_edit_avatar),
                      NULL);
	gtk_box_pack_start (GTK_BOX (vbox3), okbutt, FALSE, FALSE, 3);
	bimdiag_entry2=gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(bimdiag_entry2), FALSE);
	gtk_box_pack_start (GTK_BOX (vbox3), bimdiag_entry2, FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new(""), TRUE, TRUE, 3);

	
	bimdiag_check1=gtk_check_button_new_with_label(_("Show display images in IM windows"));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_check1, FALSE, FALSE, 2);
	bimdiag_check2=gtk_check_button_new_with_label(_("Only friends may view and share display images"));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_check2, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new(_("Display Image Size:")), FALSE, FALSE, 2);
	bimdiag_size_radio1=gtk_radio_button_new_with_label (bimage_size_group, _("Small"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_size_radio1), bimage_size_group);
  	bimage_size_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_size_radio1));
	bimdiag_size_radio2=gtk_radio_button_new_with_label (bimage_size_group,_("Medium"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_size_radio2), bimage_size_group);
  	bimage_size_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_size_radio2));
	bimdiag_size_radio3=gtk_radio_button_new_with_label (bimage_size_group, _("Large"));
  	gtk_radio_button_set_group (GTK_RADIO_BUTTON (bimdiag_size_radio3), bimage_size_group);
  	bimage_size_group = gtk_radio_button_group (GTK_RADIO_BUTTON (bimdiag_size_radio3));

	gtk_box_pack_start (GTK_BOX (hbox3), bimdiag_size_radio1, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox3), bimdiag_size_radio2, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox3), bimdiag_size_radio3, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox3), gtk_label_new(""), TRUE, TRUE, 2);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox3, FALSE, FALSE, 2);


	gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 3);

	bimdiag_check3=gtk_check_button_new_with_label(_("Show avatars on my buddy list"));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_check3, FALSE, FALSE, 2);
	bimdiag_check4=gtk_check_button_new_with_label(_("Show my avatar on friends' buddy lists"));
	gtk_box_pack_start (GTK_BOX (vbox1), bimdiag_check4, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 3);

	gtk_box_pack_start (GTK_BOX (vbox1), hbox4, FALSE, FALSE, 2);
	okbutt=get_pixmapped_button(_("Delete All Avatars"), GTK_STOCK_CLOSE); /* signal */
		set_tooltip(okbutt,_("Delete All Avatars"));
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_delete_all_yavatars),
                      NULL);
	gtk_box_pack_start (GTK_BOX (hbox4), okbutt, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox4), gtk_label_new(""), TRUE, TRUE, 2);
	okbutt=get_pixmapped_button(_("Delete All Buddy Images"), GTK_STOCK_CLOSE); /* signal */
		set_tooltip(okbutt,_("Delete All Buddy Images"));
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_delete_all_bimages),
                      NULL);
	gtk_box_pack_start (GTK_BOX (hbox4), okbutt, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (vbox1), gtk_hseparator_new(), FALSE, FALSE, 3);
	gtk_box_pack_start (GTK_BOX (vbox1), hbox5, FALSE, FALSE, 3);

	okbutt=get_pixmapped_button(_("OK"), GTK_STOCK_YES); /* signal */
		set_tooltip(okbutt,_("OK"));
 	gtk_object_set_data(GTK_OBJECT(bimdiag_window),"okbutton" , okbutt);
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_activate_bimage_config),
                      NULL);
	gtk_box_pack_start (GTK_BOX (hbox5), okbutt, FALSE, FALSE, 2);
	gtk_box_pack_start (GTK_BOX (hbox5), gtk_label_new(""), TRUE, TRUE, 2);
	okbutt=get_pixmapped_button(_("Cancel"), GTK_STOCK_NO); /* signal */
		set_tooltip(okbutt,_("Cancel"));
	gtk_object_set_data(GTK_OBJECT(bimdiag_window),"cancel_button" , okbutt);
	gtk_box_pack_start (GTK_BOX (hbox5), okbutt, FALSE, FALSE, 2);
	gtk_object_set_data(GTK_OBJECT(okbutt),"mywindow", bimdiag_window);
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialog),
                      NULL);

	gtk_object_set_data(GTK_OBJECT(bimdiag_window),"mywindow", bimdiag_window);
  	gtk_signal_connect (GTK_OBJECT (bimdiag_window), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

	
	gtk_widget_show_all(bimdiag_window);
	return bimdiag_window;
}



