/*****************************************************************************
 * setupwindow.c
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
	all main Configuration Window UI functions
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
#include "util.h"
#include "gytreeview.h"

extern GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id);
extern void on_enable_chat_spam_filter_toggled  (GtkToggleButton *togglebutton, gpointer  user_data);



#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  gtk_object_set_data_full (GTK_OBJECT (component), name, \
    gtk_widget_ref (widget), (GtkDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)


GList *vccamlist = NULL;
GList *fontscalelist=NULL;
GList *ftservlist=NULL;


GtkWidget *stoolbutt(const gchar *stock_id) {
	GtkWidget *butt=NULL;
	butt=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(butt), GTK_WIDGET(
		gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON) 
	));
	gtk_button_set_relief(GTK_BUTTON(butt), GTK_RELIEF_HALF);
	return butt;
}

GtkWidget *stoolbars(GtkTooltips *MYTIPS, GtkWidget *widget) {
	GtkWidget *toolbox, *toolbutt;

toolbox=gtk_hbox_new(FALSE,0);
gtk_container_set_border_width (GTK_CONTAINER (toolbox), 2);
toolbutt=stoolbutt(GTK_STOCK_ADD);
gtk_tooltips_set_tip(MYTIPS,toolbutt,_("Insert"),_("Insert"));
gtk_box_pack_start (GTK_BOX (toolbox), toolbutt, FALSE, FALSE, 2);
  gtk_signal_connect_object(GTK_OBJECT(toolbutt), "clicked",
				GTK_SIGNAL_FUNC(on_setuplist_tree_kb_popup),
				GTK_OBJECT(widget));
g_signal_connect_after (toolbutt,  "clicked", (GCallback)  on_setup_insert_activate, NULL);

toolbutt=stoolbutt(GTK_STOCK_FIND_AND_REPLACE);
gtk_tooltips_set_tip(MYTIPS,toolbutt,_("Edit"),_("Edit"));
gtk_box_pack_start (GTK_BOX (toolbox), toolbutt, FALSE, FALSE, 2);
  gtk_signal_connect_object(GTK_OBJECT(toolbutt), "clicked",
				GTK_SIGNAL_FUNC(on_setuplist_tree_kb_popup),
				GTK_OBJECT(widget));
g_signal_connect_after (toolbutt,  "clicked", (GCallback)  on_setup_edit_activate, NULL);

toolbutt=stoolbutt(GTK_STOCK_REMOVE);
gtk_tooltips_set_tip(MYTIPS,toolbutt,_("Delete"),_("Delete"));
gtk_box_pack_start (GTK_BOX (toolbox), toolbutt, FALSE, FALSE, 2);
  gtk_signal_connect_object(GTK_OBJECT(toolbutt), "clicked",
				GTK_SIGNAL_FUNC(on_setuplist_tree_kb_popup),
				GTK_OBJECT(widget));
g_signal_connect_after (toolbutt,  "clicked", (GCallback)  on_setup_delete_activate, NULL);
return toolbox;
}



GtkWidget *create_setup_window (void)
{
  GtkWidget *myslabel;
  GtkWidget *setup_window;
  GtkWidget *vbox1;
  GtkWidget *notebook1;
  GtkWidget *hbox2;
  GtkWidget *vbox8;
  GtkWidget *show_html;
  GtkWidget *py_voice_helper;
  GtkWidget *vbox10;
  GtkWidget *show_emoticons;
  GtkWidget *support_scentral_smileys;
  GtkWidget *allow_animations;
  GtkWidget *hbox25;
  GtkObject *max_simult_animations_adj;
  GtkWidget *max_simult_animations;
  GtkWidget *esd_sound_events;
  GtkWidget *show_avatars;
  GtkWidget *hbox26;
  GtkWidget *send_avatar;
  GtkWidget *vbox11;
  GtkWidget *vbox14;
  GtkWidget *table3;
  GtkWidget *show_fonts;
  GtkWidget *show_colors;
  GtkWidget *blended_colors;
  GtkWidget *enable_webcam_features;
  GtkWidget *enter_leave_timestamp;
  GtkWidget *show_enters;
  GtkWidget *show_statuses;
  GtkWidget *vbox13;
  GtkWidget *hbox30;
  GtkWidget *use_history_limit;
  GtkObject *history_limit_adj;
  GtkWidget *history_limit;
  GtkWidget *hbox29;
  GtkWidget *filter_command;
  GtkWidget *vbox12;
  GtkWidget *pm_brings_back;
  GtkWidget *hbox27;
  GtkWidget *auto_away;
  GtkObject *auto_away_time_adj;
  GtkWidget *auto_away_time;
  GtkWidget *hbox28;
  GtkWidget *custom_away_message;
  GtkWidget *notebook2;
  GtkWidget *hbox22;
  GtkWidget *vbox2;
  GtkWidget *color_black;
  GSList *color_black_group = NULL;
  GSList *profile_viewer_group = NULL;
  GtkWidget *color_red;
  GtkWidget *color_blue;
  GtkWidget *color_green;
  GtkWidget *color_yellow;
  GtkWidget *color_purple;
  GtkWidget *color_cyan;
  GtkWidget *color_orange;
  GtkWidget *color_gray;
  GtkWidget *color_custom;
  GtkWidget *style_bold;
  GtkWidget *style_underline;
  GtkWidget *style_italic;
  GtkWidget *colorselection;
  GtkWidget *fontselection1;
  GtkWidget *hbox15;
  GtkWidget *vbox18;
  GtkWidget *chat_timestamp;
  GtkWidget *force_lowercase;
  GtkWidget *hbox33;
  GtkWidget *limit_lfs;
  GtkObject *limit_lfs_count_adj;
  GtkWidget *limit_lfs_count;
  GtkWidget *hbox34;
  GtkWidget *limit_buffer_size;
  GtkObject *buffer_size_adj;
  GtkWidget *buffer_size;
  GtkWidget *indent_wrapped;
  GtkWidget *transparent_bg;
  GtkWidget *hbox35;
  GtkObject *shading_r_adj;
  GtkWidget *shading_r;
  GtkObject *shading_g_adj;
  GtkWidget *shading_g;
  GtkObject *shading_b_adj;
  GtkWidget *shading_b;
  GtkWidget *highlight_friends;
  GtkWidget *hbox16;
  GtkWidget *vbox3;
  GtkWidget *vbox15;
  GtkWidget *auto_reply_when_away;
  GtkWidget *hbox31;
  GtkWidget *auto_reply_msg;
  GtkWidget *chat_timestamp_pm;
  GtkWidget *pm_in_sep_windows;
  GtkWidget *auto_raise_pm;
  GtkWidget *esd_sound_events_pm;
  GtkWidget *hbox17;
  GtkWidget *vbox5;
  GtkWidget *vbox7;
  GtkWidget *no_random_friend_add;
  GtkWidget *no_sent_files;
  GtkWidget *basic_boot_protect;
  GtkWidget *preempt_boot_protection;
  GtkWidget *vbox6;
  GtkWidget *pm_from_friends;
  GtkWidget *pm_from_users;
  GtkWidget *pm_from_all;
  GtkWidget *vbox4;
  GtkWidget *disp_auto_ignored;
  GtkWidget *suppress_mult;
  GtkWidget *ignore_guests;
  GtkWidget *table1;
  GtkWidget *hbox23;
  GtkWidget *mute_on_mults;
  GtkObject *mute_on_mults_count_adj;
  GtkWidget *mute_on_mults_count;
  GtkWidget *hbox24;
  GtkWidget *ignore_on_mults;
  GtkObject *ignore_on_mults_count_adj;
  GtkWidget *ignore_on_mults_count;
  GtkWidget *table2;
  GtkWidget *first_post_is_url;
  GtkWidget *first_post_is_pm;
  GtkWidget *url_from_nonroom_user;
  GtkWidget *never_ignore_friend;
  GtkWidget *hbox18;
  GtkWidget *vbox20;
  GtkWidget *vbox21;
  GtkWidget *auto_login;
  GtkWidget *auto_reconnect;
  GtkWidget *hbox37;
  GtkWidget *use_ping_time;
  GtkObject *ping_minutes_adj;
  GtkWidget *ping_minutes;
  GtkWidget *use_proxy;
  GtkWidget *hbox38;
  GtkWidget *proxy_host;
  GtkWidget *hbox39;
  GtkObject *proxy_port_adj;
  GtkWidget *proxy_port;
  GtkWidget *vbox23;
  GtkWidget *auto_close_roomlist;
  GtkWidget *show_adult;
  GtkWidget *cache_room_list;
  GtkWidget *vbox19;
  GtkWidget *vbox24;
  GtkWidget *profile_viewer;
  GtkWidget *profile_viewer2;
  GtkWidget *profile_viewer3;
  GtkWidget *packet_debugging;
  GtkWidget *browser_command;
  GtkWidget *hbox36;
  GtkWidget *browser_netscape;
  GtkWidget *browser_mozilla;
  GtkWidget *browser_opera;
  GtkWidget *browser_konquerer;
  GtkWidget *vbox22;
  GtkWidget *remember_password;
  GtkWidget *remember_position;
  GtkWidget *vbox16;
  GtkWidget *hbox32;
  GtkWidget *scrolledwindow1;
  GtkWidget *login_list;
  GtkWidget *scrolledwindow2;
  GtkWidget *favroom_list;
  GtkWidget *scrolledwindow3;
  GtkWidget *friend_list;
  GtkWidget *vbox17;
  GtkWidget *hpaned1;
  GtkWidget *scrolledwindow5;
  GtkWidget *ignore_list;
  GtkWidget *scrolledwindow4;
  GtkWidget *regex_list;
  GtkWidget *hbox1;
  GtkWidget *save_setup;
  GtkWidget *cancel_setup;
  GtkWidget *vbox98;
  GtkWidget *highlight_friends_chat;
  GtkWidget *highlight_me_chat;
  GtkWidget *locale_status;
  GtkWidget *vbox303;
  GtkWidget *hbox414;
  GtkWidget *hbox415;
  GtkWidget *hbox416;
  GtkWidget *hbox417;
  GtkWidget *hbox418;
  GtkWidget *hbox419;
  GtkWidget *contact_first;
  GtkWidget *contact_last;
  GtkWidget *contact_email;
  GtkWidget *contact_home;
  GtkWidget *contact_work;
  GtkWidget *contact_mobile;
GtkWidget *mute_noage;
GtkWidget *mute_minors;
GtkWidget *mute_nogender;
GtkWidget *mute_males;
GtkWidget *mute_females;
GtkWidget *enable_tuxvironments;
 GtkWidget *enable_chat_spam_filter;
 GtkWidget *vboxspam6;
 GtkWidget *vboxeffect2;
 GtkWidget *enable_trayicon;
GtkWidget *fadeconfig;
GtkWidget *webcam_viewer_combo;
GtkWidget *webcam_viewer_entry;
GtkWidget *wcambox;
GtkWidget *wcamlabel;
GtkWidget *camdevbox, *camdevlab, *webcam_device;
GtkWidget *enable_encryption;

GtkWidget *popup_buzz;
GtkWidget *popup_buddy_on;
GtkWidget *popup_buddy_off;
GtkWidget *popup_new_mail;
GtkWidget *popup_news_bulletin;
GtkWidget *vboxpopup;

	GtkWidget *infoframe, *pmright,  *pmbuttons, *cprefbox1, *cprefbox2;
	GtkWidget *playbox2, *playbox1, *playlab1, *playlab2;
	GtkWidget *show_yavatars;
	GtkWidget *emulate_ymsg6;
	GtkWidget *enable_audibles;
	GtkWidget *activate_profnames;
	GtkWidget *show_bimages;
	GtkWidget *auto_reject_invitations;
	GtkWidget *pmb_audibles;
	GtkWidget *pmb_smileys;
	GtkWidget *pmb_send_erase;
	GtkWidget *flash_player_cmd;
	GtkWidget *mp3_player;
	GtkWidget *pm_window_style, *pmb_toolbar1, *pmb_toolbar2, *pmbuttons2;

GtkWidget *use_buddy_list_font, *show_quick_access_toolbar;
GtkWidget *fscalebox, *fscalelabel, *fonts_scale_combo, *fonts_scale_entry;
GtkWidget *pm_nick_names, *show_my_status_in_chat;
GtkWidget *chatwin_smileys, *chatwin_send_erase, *show_tabs_at_bottom1;
GtkWidget *enable_pm_searches, *enable_addressbook; 
GtkWidget *allow_pm_buzzes; 
GtkWidget *file_transfer_server;
GtkWidget *fontselection2;

GtkTooltips *MYTIPS=NULL;
char *col_headers[]={"",NULL};

  MYTIPS=gtk_tooltips_new();

  setup_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (setup_window), _("Setup"));
  gtk_window_set_position (GTK_WINDOW (setup_window), GTK_WIN_POS_CENTER);
  gtk_window_set_wmclass (GTK_WINDOW (setup_window), "gyachESetup", "GyachE");

  vbox1 = gtk_vbox_new (FALSE, 8);
  gtk_container_add (GTK_CONTAINER (setup_window), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 4);

  notebook1 = gtk_notebook_new ();
  gtk_box_pack_start (GTK_BOX (vbox1), notebook1, TRUE, TRUE, 0);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook1), TRUE);
  gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (notebook1), 4);
  gtk_notebook_set_tab_vborder (GTK_NOTEBOOK (notebook1), 1);

  hbox2 = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox2);
  gtk_notebook_set_tab_label_packing (GTK_NOTEBOOK (notebook1), hbox2,
                                      TRUE, TRUE, GTK_PACK_START);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 3);

  vbox8 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), vbox8, TRUE, TRUE, 0);


  vbox10 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox8), vbox10, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Graphics & Multimedia:"));
  gtk_box_pack_start (GTK_BOX (vbox10), myslabel, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  show_html = gtk_check_button_new_with_label (_("Support html tags"));
  gtk_box_pack_start (GTK_BOX (vbox10), show_html, FALSE, FALSE, 0);


  fscalebox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox10), fscalebox, FALSE, FALSE, 0);

  fscalelabel = gtk_label_new (_("Scale Text:"));
  gtk_box_pack_start (GTK_BOX (fscalebox), fscalelabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (fscalelabel), GTK_JUSTIFY_LEFT);

  fonts_scale_combo=gtk_combo_new();
  fonts_scale_entry=GTK_COMBO(fonts_scale_combo)->entry;
  gtk_editable_set_editable(GTK_EDITABLE(fonts_scale_entry),FALSE );
  gtk_box_pack_start (GTK_BOX (fscalebox), fonts_scale_combo, FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (fscalebox), gtk_label_new (""), TRUE, TRUE, 0);

  if (! fontscalelist) {
  fontscalelist = g_list_append( fontscalelist, "0.25" );
  fontscalelist = g_list_append( fontscalelist, "0.3" );
  fontscalelist = g_list_append( fontscalelist, "0.4" );
  fontscalelist = g_list_append( fontscalelist, "0.5" );
  fontscalelist = g_list_append( fontscalelist, "0.6" );
  fontscalelist = g_list_append( fontscalelist, "0.75" );
  fontscalelist = g_list_append( fontscalelist, "0.8" );
  fontscalelist = g_list_append( fontscalelist, "0.85" );
  fontscalelist = g_list_append( fontscalelist, "0.9" );
  fontscalelist = g_list_append( fontscalelist, "0.95" );
  fontscalelist = g_list_append( fontscalelist, "1.0" );
  fontscalelist = g_list_append( fontscalelist, "1.1" );
  fontscalelist = g_list_append( fontscalelist, "1.25" );
  fontscalelist = g_list_append( fontscalelist, "1.3" );
  fontscalelist = g_list_append( fontscalelist, "1.4" );
  fontscalelist = g_list_append( fontscalelist, "1.5" );
  fontscalelist = g_list_append( fontscalelist, "1.6" );
  fontscalelist = g_list_append( fontscalelist, "1.75" );
  fontscalelist = g_list_append( fontscalelist, "1.8" );
  fontscalelist = g_list_append( fontscalelist, "1.9" );
  fontscalelist = g_list_append( fontscalelist, "2.0" );
  fontscalelist = g_list_append( fontscalelist, "2.25" );
  fontscalelist = g_list_append( fontscalelist, "2.5" );
	}
  gtk_combo_set_popdown_strings( GTK_COMBO(fonts_scale_combo), fontscalelist );
  gtk_entry_set_text(GTK_ENTRY(fonts_scale_entry),"1.0" );


  py_voice_helper = gtk_check_button_new_with_label (_("Support pY! Voice voice\nchat features"));
  gtk_box_pack_start (GTK_BOX (vbox10), py_voice_helper, FALSE, FALSE, 0);

  show_emoticons = gtk_check_button_new_with_label (_("Show smileys"));
  gtk_box_pack_start (GTK_BOX (vbox10), show_emoticons, FALSE, FALSE, 0);

  support_scentral_smileys = gtk_check_button_new_with_label (_("Show SmileyCentral smileys (web)"));
  gtk_box_pack_start (GTK_BOX (vbox10), support_scentral_smileys, FALSE, FALSE, 0);

  allow_animations = gtk_check_button_new_with_label (_("Show animated smileys"));
  gtk_box_pack_start (GTK_BOX (vbox10), allow_animations, FALSE, FALSE, 0);

  hbox25 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox10), hbox25, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Maximum simultaneous\nanimated smileys"));
  gtk_box_pack_start (GTK_BOX (hbox25), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  gtk_box_pack_start (GTK_BOX (hbox25), gtk_label_new ("    "), FALSE, FALSE, 0);

  max_simult_animations_adj = gtk_adjustment_new (4, 1, 15, 1, 10, 10);
  max_simult_animations = gtk_spin_button_new (GTK_ADJUSTMENT (max_simult_animations_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox25), max_simult_animations, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (hbox25), gtk_label_new (""), TRUE, TRUE, 0);

  esd_sound_events = gtk_check_button_new_with_label (_("Enable sound events"));
  gtk_box_pack_start (GTK_BOX (vbox10), esd_sound_events, FALSE, FALSE, 0);

  show_yavatars = gtk_check_button_new_with_label (_("Show buddy list avatars"));
  gtk_box_pack_start (GTK_BOX (vbox10), show_yavatars, FALSE, FALSE, 0);



  enable_webcam_features = gtk_check_button_new_with_label (_("Enable Webcam Features"));
  gtk_box_pack_start (GTK_BOX (vbox10), enable_webcam_features, FALSE, FALSE, 0);

  wcambox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox10), wcambox, FALSE, FALSE, 0);

  wcamlabel = gtk_label_new (_("Webcam Viewer:"));
  gtk_box_pack_start (GTK_BOX (wcambox), wcamlabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wcamlabel), GTK_JUSTIFY_LEFT);

  webcam_viewer_combo=gtk_combo_new();
  webcam_viewer_entry=GTK_COMBO(webcam_viewer_combo)->entry;
  gtk_entry_set_max_length(GTK_ENTRY(webcam_viewer_entry),25);		 
  gtk_editable_set_editable(GTK_EDITABLE(webcam_viewer_entry),FALSE );
  gtk_box_pack_start (GTK_BOX (wcambox), webcam_viewer_combo, TRUE, TRUE, 3);

  if (! vccamlist) {
  vccamlist = g_list_append( vccamlist, "Gyach-E Webcam" );
  vccamlist = g_list_append( vccamlist, "pY WebCam" );  
	}
  gtk_combo_set_popdown_strings( GTK_COMBO(webcam_viewer_combo), vccamlist );
  gtk_entry_set_text(GTK_ENTRY(webcam_viewer_entry),"Gyach-E Webcam" );

  camdevbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox10), camdevbox, TRUE, TRUE, 2);

  camdevlab = gtk_label_new (_("Webcam Device:"));
  gtk_box_pack_start (GTK_BOX (camdevbox), camdevlab, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (camdevlab), GTK_JUSTIFY_LEFT);

  webcam_device = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(webcam_device),13);
  gtk_box_pack_start (GTK_BOX (camdevbox), webcam_device, TRUE, TRUE, 3);


  enable_trayicon = gtk_check_button_new_with_label (_("Enable Tray Icon"));
  gtk_box_pack_start (GTK_BOX (vbox10), enable_trayicon, FALSE, FALSE, 0);

  enable_addressbook = gtk_check_button_new_with_label (_("Enable Addressbook"));
  gtk_box_pack_start (GTK_BOX (vbox10), enable_addressbook, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox8), gtk_label_new (""), TRUE, TRUE, 0);

  vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), vbox11, TRUE, TRUE, 0);


  vbox14 = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox11), vbox14, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Show the following from other users:"));
  gtk_box_pack_start (GTK_BOX (vbox14), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  table3 = gtk_table_new (3, 2, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox14), table3, TRUE, TRUE, 0);
  gtk_table_set_row_spacings (GTK_TABLE (table3), 2);
  gtk_table_set_col_spacings (GTK_TABLE (table3), 18);

  show_fonts = gtk_check_button_new_with_label (_("Fonts"));
  gtk_table_attach (GTK_TABLE (table3), show_fonts, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  show_colors = gtk_check_button_new_with_label (_("Colors"));
  gtk_table_attach (GTK_TABLE (table3), show_colors, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  blended_colors = gtk_check_button_new_with_label (_("Faders"));
  gtk_table_attach (GTK_TABLE (table3), blended_colors, 0, 1, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  show_statuses = gtk_check_button_new_with_label (_("Status changes"));
  gtk_table_attach (GTK_TABLE (table3), show_statuses, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  show_enters = gtk_check_button_new_with_label (_("Entering/leaving"));
  gtk_table_attach (GTK_TABLE (table3), show_enters, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  enter_leave_timestamp = gtk_check_button_new_with_label (_("Entering/leaving Timestamps"));
  gtk_table_attach (GTK_TABLE (table3), enter_leave_timestamp, 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);


  gtk_box_pack_start (GTK_BOX (vbox11),  gtk_label_new (""), TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Miscellaneous Options:"));
  gtk_box_pack_start (GTK_BOX (vbox11), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  show_quick_access_toolbar= gtk_check_button_new_with_label (_("Show the 'quick access' toolbar (requires restart)"));
  gtk_box_pack_start (GTK_BOX (vbox11), show_quick_access_toolbar, FALSE, FALSE, 0);

  show_tabs_at_bottom1= gtk_check_button_new_with_label (_("Tabs on the bottom (requires restart)"));
  gtk_box_pack_start (GTK_BOX (vbox11), show_tabs_at_bottom1, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox11), gtk_label_new (""), TRUE, TRUE, 0);


  vbox12 = gtk_vbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox11), vbox12, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Away Status:"));
  gtk_box_pack_start (GTK_BOX (vbox12), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  pm_brings_back = gtk_check_button_new_with_label (_("Sending PM or chat comment\nwhen away brings you back"));
  gtk_box_pack_start (GTK_BOX (vbox12), pm_brings_back, FALSE, FALSE, 0);

  show_my_status_in_chat = gtk_check_button_new_with_label (_("Broadcast my status changes in chats / conferences"));
  gtk_box_pack_start (GTK_BOX (vbox12), show_my_status_in_chat, FALSE, FALSE, 0);

 locale_status = gtk_check_button_new_with_label (_("Send and receive status messages\nin the same language as my LOCALE"));
  gtk_box_pack_start (GTK_BOX (vbox12), locale_status, FALSE, FALSE, 0);

  hbox27 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox12), hbox27, TRUE, TRUE, 0);

  auto_away = gtk_check_button_new_with_label (_("Auto-away timer  (in minutes)"));
  gtk_box_pack_start (GTK_BOX (hbox27), auto_away, TRUE, TRUE, 0);

  auto_away_time_adj = gtk_adjustment_new (30, 0, 100, 1, 10, 10);
  auto_away_time = gtk_spin_button_new (GTK_ADJUSTMENT (auto_away_time_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox27), auto_away_time, FALSE, FALSE, 0);

  hbox28 = gtk_hbox_new (FALSE, 7);
  gtk_box_pack_start (GTK_BOX (vbox12), hbox28, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Custom Away Msg.: "));
  gtk_box_pack_start (GTK_BOX (hbox28), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  custom_away_message = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(custom_away_message),155);
  gtk_box_pack_start (GTK_BOX (hbox28), custom_away_message, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (custom_away_message), _("Be right back"));

  myslabel = gtk_label_new (_("General"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

	if ( show_tabs_at_bottom) {
		gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook1), GTK_POS_BOTTOM);
	}

  notebook2 = gtk_notebook_new ();
  gtk_container_add (GTK_CONTAINER (notebook1), notebook2);
  gtk_container_set_border_width (GTK_CONTAINER (notebook2), 1);
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook2), GTK_POS_LEFT);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook2), TRUE);
  gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (notebook2), 4);

  hbox22 = gtk_hbox_new (FALSE, 16);
  gtk_container_add (GTK_CONTAINER (notebook2), hbox22);
  gtk_container_set_border_width (GTK_CONTAINER (hbox22), 3);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox22), vbox2, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Color"));
  gtk_box_pack_start (GTK_BOX (vbox2), myslabel, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  color_black = gtk_radio_button_new_with_mnemonic (NULL, _("Black"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_black, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_black), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_black));

  color_red = gtk_radio_button_new_with_mnemonic (NULL, _("Red"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_red, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_red), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_red));

  color_blue = gtk_radio_button_new_with_mnemonic (NULL, _("Blue"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_blue, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_blue), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_blue));


  color_green = gtk_radio_button_new_with_mnemonic (NULL, _("Green"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_green, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_green), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_green));

  color_yellow = gtk_radio_button_new_with_mnemonic (NULL, _("Yellow"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_yellow, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_yellow), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_yellow));

  color_purple = gtk_radio_button_new_with_mnemonic (NULL, _("Purple"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_purple, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_purple), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_purple));

  color_cyan = gtk_radio_button_new_with_mnemonic (NULL, _("Cyan"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_cyan, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_cyan), color_black_group);
 color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_cyan));

  color_orange = gtk_radio_button_new_with_mnemonic (NULL, _("Orange"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_orange, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_orange), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_orange));

  color_gray = gtk_radio_button_new_with_mnemonic (NULL, _("Gray"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_gray, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_gray), color_black_group);
  color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_gray));

  color_custom = gtk_radio_button_new_with_mnemonic (NULL, _("Custom"));
  gtk_box_pack_start (GTK_BOX (vbox2), color_custom, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (color_custom), color_black_group);
 color_black_group = gtk_radio_button_group (GTK_RADIO_BUTTON (color_custom));

  gtk_box_pack_start (GTK_BOX (vbox2), gtk_label_new(""), TRUE, TRUE, 0);


  colorselection = gtk_color_selection_new ();
  gtk_box_pack_start (GTK_BOX (hbox22), colorselection, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (colorselection), 3);
  gtk_color_selection_set_has_opacity_control (GTK_COLOR_SELECTION (colorselection), FALSE);
  gtk_color_selection_set_has_palette (GTK_COLOR_SELECTION (colorselection), TRUE);

  fadeconfig=get_pixmapped_button(_("Configuration: Text Effects"), GTK_STOCK_SELECT_COLOR);
  gtk_box_pack_start (GTK_BOX (colorselection), fadeconfig, FALSE, FALSE, 5);
	gtk_signal_connect (GTK_OBJECT (fadeconfig), "clicked",
		GTK_SIGNAL_FUNC (on_create_fadeconfigwin), NULL);

  myslabel = gtk_label_new (_("Color"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 0), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);




  fontselection1 = gtk_font_selection_new ();

  hbox22 = gtk_hbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox22), 3);
  gtk_box_pack_start (GTK_BOX (fontselection1), hbox22, FALSE, FALSE, 3);

  myslabel=gtk_label_new(_("This font is used for the chat and PM\nmessages you send to other users."));
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox22),myslabel , FALSE, FALSE, 3);

  gtk_box_pack_start (GTK_BOX (hbox22), gtk_label_new("  "), TRUE, TRUE, 2);

  vboxeffect2=gtk_vbox_new (FALSE, 0);  
  gtk_box_pack_start (GTK_BOX (hbox22), vboxeffect2 , FALSE, FALSE, 2);
  myslabel = gtk_label_new (_("Effects"));
  gtk_box_pack_start (GTK_BOX (vboxeffect2), myslabel, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  style_bold = gtk_check_button_new_with_label (_("Bold"));
  gtk_box_pack_start (GTK_BOX (vboxeffect2), style_bold, FALSE, FALSE, 0);

  style_underline = gtk_check_button_new_with_label (_("Underline"));
  gtk_box_pack_start (GTK_BOX (vboxeffect2), style_underline, FALSE, FALSE, 0);

  style_italic = gtk_check_button_new_with_label (_("Italics"));
  gtk_box_pack_start (GTK_BOX (vboxeffect2), style_italic, FALSE, FALSE, 0);


  gtk_container_add (GTK_CONTAINER (notebook2), fontselection1);
  gtk_container_set_border_width (GTK_CONTAINER (fontselection1), 7);
  gtk_font_selection_set_preview_text (GTK_FONT_SELECTION (fontselection1),
                                       _("abcdefghijk ABCDEFGHIJK GYach Enhanced 1234567890"));

  myslabel = gtk_label_new (_("Message\nFont"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 1), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);





  fontselection2 = gtk_font_selection_new ();

  hbox22 = gtk_vbox_new (FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox22), 3);
  gtk_box_pack_start (GTK_BOX (fontselection2), hbox22, FALSE, FALSE, 3);

  myslabel=gtk_label_new(_("This font is used for timestamps, informational messages,\nand as the default font for incoming messages."));
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (hbox22),myslabel , FALSE, FALSE, 3);

  gtk_box_pack_start (GTK_BOX (hbox22), gtk_hseparator_new(), FALSE, FALSE, 1);

  use_buddy_list_font = gtk_check_button_new_with_label (_("Apply this font to the buddy list and address book\n(Disabling requires the program to be restarted.)"));
  gtk_box_pack_start (GTK_BOX (hbox22), use_buddy_list_font, FALSE, FALSE, 3);


  gtk_container_add (GTK_CONTAINER (notebook2), fontselection2);
  gtk_container_set_border_width (GTK_CONTAINER (fontselection2), 7);
  gtk_font_selection_set_preview_text (GTK_FONT_SELECTION (fontselection2),
                                       _("abcdefghijk ABCDEFGHIJK GYach Enhanced 1234567890"));

  myslabel = gtk_label_new (_("Display\nFont"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook2), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook2), 2), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);




  myslabel = gtk_label_new (_("Color &\nFont"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  hbox15 = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox15);
  gtk_container_set_border_width (GTK_CONTAINER (hbox15), 3);
  gtk_box_set_spacing(GTK_BOX (hbox15) , 4);

  vbox18 = gtk_vbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (hbox15), vbox18, FALSE, FALSE, 3);

  myslabel = gtk_label_new (_("Chat Text Display:"));
  gtk_box_pack_start (GTK_BOX (vbox18), myslabel, FALSE, FALSE, 4);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  chat_timestamp = gtk_check_button_new_with_label (_("Display timestamp with messages"));
  gtk_box_pack_start (GTK_BOX (vbox18), chat_timestamp, FALSE, FALSE, 0);

  force_lowercase = gtk_check_button_new_with_label (_("Force text to lowercase"));
  gtk_box_pack_start (GTK_BOX (vbox18), force_lowercase, FALSE, FALSE, 0);

  hbox33 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox18), hbox33, FALSE, FALSE,  0);

  limit_lfs = gtk_check_button_new_with_label (_("Maximum line feeds per message"));
  gtk_box_pack_start (GTK_BOX (hbox33), limit_lfs, TRUE, TRUE, 0);

  limit_lfs_count_adj = gtk_adjustment_new (6, 0, 16, 1, 10, 10);
  limit_lfs_count = gtk_spin_button_new (GTK_ADJUSTMENT (limit_lfs_count_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox33), limit_lfs_count, FALSE, FALSE, 0);

  hbox34 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox18), hbox34, FALSE, FALSE,  0);

  limit_buffer_size = gtk_check_button_new_with_label (_("Limit text box size (in Kilobytes)"));
  gtk_box_pack_start (GTK_BOX (hbox34), limit_buffer_size, TRUE, TRUE, 0);

  buffer_size_adj = gtk_adjustment_new (10, 0, 50, 1, 10, 10);
  buffer_size = gtk_spin_button_new (GTK_ADJUSTMENT (buffer_size_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox34), buffer_size, FALSE, FALSE, 0);

  indent_wrapped = gtk_check_button_new_with_label (_("Indent wrapped lines"));
  gtk_box_pack_start (GTK_BOX (vbox18), indent_wrapped, FALSE, FALSE, 0);

  transparent_bg = gtk_check_button_new_with_label (_("Transparent Background (EXPERIMENTAL)"));
  gtk_box_pack_start (GTK_BOX (vbox18), transparent_bg, FALSE, FALSE, 0);

  hbox35 = gtk_hbox_new (FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vbox18), hbox35, TRUE, TRUE,  0);

  myslabel = gtk_label_new (_("Transparent Shading\n(R,G,B)"));
  gtk_box_pack_start (GTK_BOX (hbox35), myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  shading_r_adj = gtk_adjustment_new (0, 0, 255, 1, 10, 10);
  shading_r = gtk_spin_button_new (GTK_ADJUSTMENT (shading_r_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox35), shading_r, FALSE, FALSE, 0);

  shading_g_adj = gtk_adjustment_new (0, 0, 255, 1, 10, 10);
  shading_g = gtk_spin_button_new (GTK_ADJUSTMENT (shading_g_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox35), shading_g, FALSE, FALSE, 0);

  shading_b_adj = gtk_adjustment_new (0, 0, 255, 1, 10, 10);
  shading_b = gtk_spin_button_new (GTK_ADJUSTMENT (shading_b_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox35), shading_b, FALSE, FALSE, 0);

  highlight_friends = gtk_check_button_new_with_label (_("Highlight Friends names when they comment"));
  gtk_box_pack_start (GTK_BOX (vbox18), highlight_friends, FALSE, FALSE, 0);


  highlight_friends_chat = gtk_check_button_new_with_label (_("Highlight the names of my friends\nin the list of chat room members"));
  gtk_box_pack_start (GTK_BOX (vbox18), highlight_friends_chat, FALSE, FALSE, 0);

  highlight_me_chat = gtk_check_button_new_with_label (_("Highlight my name in the list of\nchat room members"));
  gtk_box_pack_start (GTK_BOX (vbox18), highlight_me_chat, FALSE, FALSE, 0);


  show_avatars = gtk_check_button_new_with_label (_("Show Avatars"));
  gtk_box_pack_start (GTK_BOX (vbox18), show_avatars, FALSE, FALSE, 0);

  hbox26 = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox18), hbox26, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Send Avatar:"));
  gtk_box_pack_start (GTK_BOX (hbox26), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  send_avatar = gtk_entry_new ();
  gtk_entry_set_max_length(GTK_ENTRY(send_avatar),32);
  gtk_box_pack_start (GTK_BOX (hbox26), send_avatar, TRUE, TRUE, 3);

  gtk_tooltips_set_tip(MYTIPS,show_avatars,_("CheetaChat Avatars"),_("CheetaChat Avatars"));
  gtk_tooltips_set_tip(MYTIPS,send_avatar,_("CheetaChat Avatars"),_("CheetaChat Avatars"));

  gtk_box_pack_start (GTK_BOX (vbox18), gtk_label_new (""), TRUE, TRUE, 0);


  vbox98 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox15), gtk_label_new(" "), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox15), vbox98, TRUE, TRUE, 3);



  myslabel = gtk_label_new (_("In chat rooms, try to mute all of the\nfollowing users (You must leave and\nre-enter the room) :"));
  gtk_box_pack_start (GTK_BOX (vbox98), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  cprefbox1 = gtk_hbox_new (FALSE, 2);
  cprefbox2 = gtk_hbox_new (FALSE, 2);

  gtk_box_pack_start (GTK_BOX (vbox98), cprefbox1, FALSE, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox98), cprefbox2, FALSE, FALSE, 1);

  mute_males = gtk_check_button_new_with_label (_("Males"));
  gtk_box_pack_start (GTK_BOX (cprefbox1), mute_males, TRUE, TRUE, 0);

  mute_females = gtk_check_button_new_with_label (_("Females"));
  gtk_box_pack_start (GTK_BOX (cprefbox1), mute_females, TRUE, TRUE, 0);

  mute_nogender = gtk_check_button_new_with_label (_("Unknown Gender"));
  gtk_box_pack_start (GTK_BOX (cprefbox1), mute_nogender, TRUE, TRUE, 0);

  mute_minors = gtk_check_button_new_with_label (_("Users Under 18"));
  gtk_box_pack_start (GTK_BOX (cprefbox2), mute_minors, TRUE, TRUE, 0);

  mute_noage = gtk_check_button_new_with_label (_("Unknown Age"));
  gtk_box_pack_start (GTK_BOX (cprefbox2), mute_noage, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox98), gtk_label_new(""), TRUE, TRUE, 0);


  vbox13 = gtk_vbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox98), vbox13, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Buttons shown on the chat window:"));
  gtk_box_pack_start (GTK_BOX (vbox13), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox13), hbox31, FALSE, FALSE, 0);

  chatwin_smileys = gtk_check_button_new_with_label (_("Smileys"));
  gtk_box_pack_start (GTK_BOX (hbox31), chatwin_smileys, TRUE, TRUE, 0);
  chatwin_send_erase = gtk_check_button_new_with_label (_("Send / Erase"));
  gtk_box_pack_start (GTK_BOX (hbox31), chatwin_send_erase, TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (vbox13), gtk_label_new(_("(requires restart)")), FALSE, FALSE, 0);


  gtk_box_pack_start (GTK_BOX (vbox13), gtk_label_new(""), TRUE, TRUE, 0);


  myslabel = gtk_label_new (_("Outgoing Messages:"));
  gtk_box_pack_start (GTK_BOX (vbox13), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  hbox30 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox13), hbox30, TRUE, TRUE, 0);

  use_history_limit = gtk_check_button_new_with_label (_("Enable command history (lines)"));
  gtk_box_pack_start (GTK_BOX (hbox30), use_history_limit, TRUE, TRUE, 0);

  history_limit_adj = gtk_adjustment_new (50, 0, 200, 1, 10, 10);
  history_limit = gtk_spin_button_new (GTK_ADJUSTMENT (history_limit_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox30), history_limit, FALSE, FALSE, 0);

	/* The 'filter' command stuff is disable for now-below....
		I personally dont use it and it's wasting space */

  hbox29 = gtk_hbox_new (FALSE, 7);
  /* gtk_box_pack_start (GTK_BOX (vbox13), hbox29, TRUE, TRUE, 0); */

  myslabel = gtk_label_new (_("Filter Command:"));
  gtk_box_pack_start (GTK_BOX (hbox29), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  filter_command = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(filter_command),128);
  gtk_box_pack_start (GTK_BOX (hbox29), filter_command, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox98), gtk_label_new(""), TRUE, TRUE, 0);

  vbox23 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox98), vbox23, FALSE, FALSE, 0);


  myslabel = gtk_label_new (_("Room List Options:"));
  gtk_box_pack_start (GTK_BOX (vbox23), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  auto_close_roomlist = gtk_check_button_new_with_label (_("Auto-close room list\nwindow when joining"));
  gtk_box_pack_start (GTK_BOX (vbox23), auto_close_roomlist, FALSE, FALSE, 0);

  show_adult = gtk_check_button_new_with_label (_("List adult rooms in room list"));
  gtk_box_pack_start (GTK_BOX (vbox23), show_adult, FALSE, FALSE, 0);

  cache_room_list = gtk_check_button_new_with_label (_("Cache room list"));
  gtk_box_pack_start (GTK_BOX (vbox23), cache_room_list, FALSE, FALSE, 0);





  myslabel = gtk_label_new (_("Chat &\nConferences"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  hbox16 = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox16);
  gtk_container_set_border_width (GTK_CONTAINER (hbox16), 3);

  vbox3 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox16), vbox3, TRUE, TRUE, 0);

  pmright = gtk_vbox_new (FALSE, 2);

  pmbuttons = gtk_hbox_new (FALSE, 2);
  pmb_audibles = gtk_check_button_new_with_label (_("Audibles"));
  gtk_box_pack_start (GTK_BOX (pmbuttons), pmb_audibles, TRUE, TRUE, 0);
  pmb_smileys = gtk_check_button_new_with_label (_("Smileys"));
  gtk_box_pack_start (GTK_BOX (pmbuttons), pmb_smileys, TRUE, TRUE, 0);
  pmb_send_erase = gtk_check_button_new_with_label (_("Send / Erase"));
  gtk_box_pack_start (GTK_BOX (pmbuttons), pmb_send_erase, TRUE, TRUE, 0);

  pmbuttons2 = gtk_hbox_new (FALSE, 2);
  pmb_toolbar1 = gtk_check_button_new_with_label (_("Top Toolbar"));
  gtk_box_pack_start (GTK_BOX (pmbuttons2), pmb_toolbar1, TRUE, TRUE, 0);
  pmb_toolbar2 = gtk_check_button_new_with_label (_("Bottom Toolbar"));
  gtk_box_pack_start (GTK_BOX (pmbuttons2), pmb_toolbar2, TRUE, TRUE, 0);

  vbox303 = gtk_vbox_new (FALSE, 2);
  infoframe = gtk_frame_new (_("Contact Information:"));
  gtk_frame_set_shadow_type (GTK_FRAME (infoframe), GTK_SHADOW_IN );
  gtk_container_add (GTK_CONTAINER (infoframe), vbox303 );
  gtk_container_set_border_width (GTK_CONTAINER (vbox303), 5);

  gtk_box_pack_start (GTK_BOX (pmright), gtk_label_new (_("Buttons shown on PM windows:")), FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (pmright), pmbuttons, FALSE, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (pmright), pmbuttons2, FALSE, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (pmright), gtk_label_new (_("(Changes affect new PM windows.)")), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (pmright), gtk_label_new (""), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (pmright), infoframe, FALSE, FALSE, 1);
  gtk_box_pack_start (GTK_BOX (hbox16), pmright, FALSE, FALSE, 2);

  myslabel = gtk_label_new (_("Private Message Options:"));
  gtk_box_pack_start (GTK_BOX (vbox3), myslabel, FALSE, FALSE, 6);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  vbox15 = gtk_vbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox3), vbox15, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (vbox15), 6);

  auto_reply_when_away = gtk_check_button_new_with_label (_("Automatically reply to private messages\nwhen I'm away"));
  gtk_box_pack_start (GTK_BOX (vbox15), auto_reply_when_away, FALSE, FALSE, 0);

  hbox31 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox15), hbox31, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Message:"));
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  auto_reply_msg = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(auto_reply_msg),230);
  gtk_box_pack_start (GTK_BOX (hbox31), auto_reply_msg, TRUE, TRUE, 0);
  gtk_entry_set_text (GTK_ENTRY (auto_reply_msg), _("Sorry, I'm away right now, but I'll respond as soon as I get back."));

  chat_timestamp_pm = gtk_check_button_new_with_label (_("Display timestamp with PM messages"));
  gtk_box_pack_start (GTK_BOX (vbox3), chat_timestamp_pm, FALSE, FALSE, 0);

  esd_sound_events_pm = gtk_check_button_new_with_label (_("Enable PM sound events"));
  gtk_box_pack_start (GTK_BOX (vbox3), esd_sound_events_pm, FALSE, FALSE, 0);

  allow_pm_buzzes = gtk_check_button_new_with_label (_("Allow users to 'buzz' me"));
  gtk_box_pack_start (GTK_BOX (vbox3), allow_pm_buzzes, FALSE, FALSE, 0);

  enable_audibles= gtk_check_button_new_with_label (_("Enable Audibles"));
  gtk_box_pack_start (GTK_BOX (vbox3), enable_audibles, FALSE, FALSE, 0);

  enable_pm_searches= gtk_check_button_new_with_label (_("Enable Web Searches"));
  gtk_box_pack_start (GTK_BOX (vbox3), enable_pm_searches, FALSE, FALSE, 0);

  pm_nick_names = gtk_check_button_new_with_label (_("When possible, show chat 'nicknames' instead\nof screen names in messages"));
  gtk_box_pack_start (GTK_BOX (vbox3), pm_nick_names, FALSE, FALSE, 0);


  pm_in_sep_windows = gtk_check_button_new_with_label (_("Open separate windows for PM chats"));
  gtk_box_pack_start (GTK_BOX (vbox3), pm_in_sep_windows, FALSE, FALSE, 0);

  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox31, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);

  auto_raise_pm = gtk_check_button_new_with_label (_("Auto-raise PM window when message received"));
  gtk_box_pack_start (GTK_BOX (hbox31), auto_raise_pm, FALSE, FALSE, 0);

  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox31, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);

  show_bimages= gtk_check_button_new_with_label (_("Show Buddy Images"));
  gtk_box_pack_start (GTK_BOX (hbox31), show_bimages, FALSE, FALSE, 0);


  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox31, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);

  pm_window_style= gtk_check_button_new_with_label (_("PM windows have scrollable typing area"));
  gtk_box_pack_start (GTK_BOX (hbox31), pm_window_style, FALSE, FALSE, 0);


  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox31, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);

  enable_tuxvironments= gtk_check_button_new_with_label (_("Enable TUXVironments"));
  gtk_box_pack_start (GTK_BOX (hbox31), enable_tuxvironments, FALSE, FALSE, 0);

  hbox31 = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox3), hbox31, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox31), gtk_label_new("   "), FALSE, FALSE, 0);

  enable_encryption= gtk_check_button_new_with_label (_("Allow PM Encryption (plugin)"));
  gtk_box_pack_start (GTK_BOX (hbox31), enable_encryption, FALSE, FALSE, 0);


  /* gtk_box_pack_start (GTK_BOX (vbox3), gtk_label_new (""), TRUE, TRUE, 7); */ 


  myslabel = gtk_label_new (_("You can send your personal\ninformation for other users to save\nin their Yahoo! addressbook."));
  gtk_box_pack_start (GTK_BOX (vbox303), myslabel, FALSE, FALSE, 6);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);


  hbox414 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303), hbox414, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("First Name:"));
  gtk_box_pack_start (GTK_BOX (hbox414), myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_first = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(contact_first),24);
  gtk_box_pack_start (GTK_BOX (hbox414), contact_first, TRUE, TRUE, 0);

  hbox415 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303), hbox415, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Last Name:"));
  gtk_box_pack_start (GTK_BOX (hbox415), myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_last = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(contact_last),32);
  gtk_box_pack_start (GTK_BOX (hbox415), contact_last, TRUE, TRUE, 0);

  hbox416 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303), hbox416, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("E-mail:"));
  gtk_box_pack_start (GTK_BOX (hbox416),myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_email = gtk_entry_new();
gtk_entry_set_max_length(GTK_ENTRY(contact_email),60);
  gtk_box_pack_start (GTK_BOX (hbox416), contact_email, TRUE, TRUE, 0);

  hbox417 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303), hbox417, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Home #:"));
  gtk_box_pack_start (GTK_BOX (hbox417),myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_home = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(contact_home),20);
  gtk_box_pack_start (GTK_BOX (hbox417), contact_home, TRUE, TRUE, 0);

  hbox418 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303),hbox418, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Work #:"));
  gtk_box_pack_start (GTK_BOX (hbox418),myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_work = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(contact_work),20);
  gtk_box_pack_start (GTK_BOX (hbox418), contact_work, TRUE, TRUE, 0);

  hbox419 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox303),hbox419, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Mobile #:"));
  gtk_box_pack_start (GTK_BOX (hbox419),myslabel, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  contact_mobile = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(contact_mobile),20);
  gtk_box_pack_start (GTK_BOX (hbox419), contact_mobile, TRUE, TRUE, 0);



  myslabel = gtk_label_new (_("Messages &\nPersonal"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  hbox17 = gtk_hbox_new (FALSE, 12);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox17);
  gtk_container_set_border_width (GTK_CONTAINER (hbox17), 3);

  vbox5 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox17), vbox5, TRUE, TRUE, 0);

  vbox7 = gtk_vbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox5), vbox7, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Privacy and Security:"));
  gtk_box_pack_start (GTK_BOX (vbox7), myslabel, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  no_random_friend_add = gtk_check_button_new_with_label (_("Do not allow anyone who\nis not a friend to add me\nto their friend's list"));
  gtk_box_pack_start (GTK_BOX (vbox7), no_random_friend_add, FALSE, FALSE, 0);

  no_sent_files = gtk_check_button_new_with_label (_("Do not accept any sent files"));
  gtk_box_pack_start (GTK_BOX (vbox7), no_sent_files, FALSE, FALSE, 0);

  auto_reject_invitations = gtk_check_button_new_with_label (_("Automatically reject all invitations"));
  gtk_box_pack_start (GTK_BOX (vbox7), auto_reject_invitations, FALSE, FALSE, 0);

  basic_boot_protect = gtk_check_button_new_with_label (_("Enable basic boot prevention"));
  gtk_box_pack_start (GTK_BOX (vbox7), basic_boot_protect, FALSE, FALSE, 0);

  preempt_boot_protection = gtk_check_button_new_with_label (_("Advanced boot prevention -\npre-emptive strikes against\nusers trying to boot me"));
  gtk_box_pack_start (GTK_BOX (vbox7), preempt_boot_protection, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vbox5), gtk_label_new (""), TRUE, TRUE, 0);


  vbox6 = gtk_vbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox5), vbox6, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Accept Private Messages,\nInvitations, and Files From:"));
  gtk_box_pack_start (GTK_BOX (vbox6), myslabel, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  pm_from_friends = gtk_check_button_new_with_label (_("Friends/Buddies"));
  gtk_box_pack_start (GTK_BOX (vbox6), pm_from_friends, FALSE, FALSE, 0);

  pm_from_users = gtk_check_button_new_with_label (_("Users in current room"));
  gtk_box_pack_start (GTK_BOX (vbox6), pm_from_users, FALSE, FALSE, 0);

  pm_from_all = gtk_check_button_new_with_label (_("All Yahoo! users"));
  gtk_box_pack_start (GTK_BOX (vbox6), pm_from_all, FALSE, FALSE, 0);

  vbox4 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox17), vbox4, TRUE, TRUE, 0);


  vboxspam6 = gtk_vbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox4), vboxspam6, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Spam Protection:"));
  gtk_box_pack_start (GTK_BOX (vboxspam6), myslabel, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  ignore_guests = gtk_check_button_new_with_label (_("Ignore guest & @sbcglobal.net users"));
  gtk_box_pack_start (GTK_BOX (vboxspam6), ignore_guests, FALSE, FALSE, 0);


  enable_chat_spam_filter = gtk_check_button_new_with_label (_("Enable spam protection (PM & Chat)"));
  gtk_box_pack_start (GTK_BOX (vboxspam6), enable_chat_spam_filter, FALSE, FALSE, 0);

  gtk_box_pack_start (GTK_BOX (vboxspam6), gtk_hseparator_new(), TRUE, TRUE, 0);


  myslabel = gtk_label_new (_("Auto-Ignore/Auto-Mute Rules:"));
  gtk_box_pack_start (GTK_BOX (vbox4), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);


  never_ignore_friend = gtk_check_button_new_with_label (_("Never auto-ignore/auto-mute a friend"));
  gtk_box_pack_start (GTK_BOX (vbox4), never_ignore_friend, FALSE, FALSE, 0);

  disp_auto_ignored = gtk_check_button_new_with_label (_("Display messages before auto-ignoring"));
  gtk_box_pack_start (GTK_BOX (vbox4), disp_auto_ignored, FALSE, FALSE, 0);

  suppress_mult = gtk_check_button_new_with_label (_("Suppress multiple consecutive posts"));
  gtk_box_pack_start (GTK_BOX (vbox4), suppress_mult, FALSE, FALSE, 0);


  gtk_box_pack_start (GTK_BOX (vbox4), gtk_label_new (""), FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Auto-Ignore/Auto-Mute Triggers:"));
  gtk_box_pack_start (GTK_BOX (vbox4), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  table1 = gtk_table_new (2, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox4), table1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table1), 1);
  gtk_table_set_col_spacings (GTK_TABLE (table1), 8);

  myslabel = gtk_label_new (_("Auto-Ignore"));
  gtk_table_attach (GTK_TABLE (table1), myslabel, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  myslabel = gtk_label_new (_("Auto-Mute"));
  gtk_table_attach (GTK_TABLE (table1), myslabel, 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  hbox23 = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (table1), hbox23, 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 0, 0);

  mute_on_mults = gtk_check_button_new_with_label ("");
  gtk_box_pack_start (GTK_BOX (hbox23), mute_on_mults, TRUE, FALSE, 0);

  mute_on_mults_count_adj = gtk_adjustment_new (5, 0, 100, 1, 10, 10);
  mute_on_mults_count = gtk_spin_button_new (GTK_ADJUSTMENT (mute_on_mults_count_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox23), mute_on_mults_count, FALSE, FALSE, 0);

  hbox24 = gtk_hbox_new (FALSE, 0);
  gtk_table_attach (GTK_TABLE (table1), hbox24, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (GTK_FILL), 0, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox24), 4);

  ignore_on_mults = gtk_check_button_new_with_label ("");
  gtk_box_pack_start (GTK_BOX (hbox24), ignore_on_mults, TRUE, FALSE, 0);

  ignore_on_mults_count_adj = gtk_adjustment_new (6, 0, 100, 1, 10, 10);
  ignore_on_mults_count = gtk_spin_button_new (GTK_ADJUSTMENT (ignore_on_mults_count_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox24), ignore_on_mults_count, FALSE, FALSE, 0);

  gtk_table_attach (GTK_TABLE (table1), gtk_label_new (""), 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  myslabel = gtk_label_new (_("Duplicate chat\nmessages, invitations,\nand file transfers in a row"));
  gtk_table_attach (GTK_TABLE (table1), myslabel, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  table2 = gtk_table_new (3, 3, FALSE);
  gtk_box_pack_start (GTK_BOX (vbox4), table2, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (table2), 1);
  gtk_table_set_col_spacings (GTK_TABLE (table2), 8);

  first_post_is_url = gtk_check_button_new_with_label ("");
  gtk_table_attach (GTK_TABLE (table2), first_post_is_url, 2, 3, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (table2), gtk_label_new (""), 1, 2, 0, 1,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  first_post_is_pm = gtk_check_button_new_with_label ("");
  gtk_table_attach (GTK_TABLE (table2), first_post_is_pm, 2, 3, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  url_from_nonroom_user = gtk_check_button_new_with_label ("");
  gtk_table_attach (GTK_TABLE (table2), url_from_nonroom_user, 2, 3, 2, 3,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (table2), gtk_label_new (""), 1, 2, 2, 3,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  gtk_table_attach (GTK_TABLE (table2), gtk_label_new (""), 1, 2, 1, 2,
                    (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);

  myslabel = gtk_label_new (_("URL from user not in room"));
  gtk_table_attach (GTK_TABLE (table2), myslabel, 0, 1, 2, 3,
                    (GtkAttachOptions) (0),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  myslabel = gtk_label_new (_("First Post is a PM"));
  gtk_table_attach (GTK_TABLE (table2), myslabel, 0, 1, 1, 2,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  myslabel = gtk_label_new (_("First Post is a URL"));
  gtk_table_attach (GTK_TABLE (table2), myslabel, 0, 1, 0, 1,
                    (GtkAttachOptions) (GTK_FILL),
                    (GtkAttachOptions) (0), 0, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);



  myslabel = gtk_label_new (_("Protection"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 4), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  hbox18 = gtk_hbox_new (FALSE, 18);
  gtk_container_add (GTK_CONTAINER (notebook1), hbox18);
  gtk_container_set_border_width (GTK_CONTAINER (hbox18), 3);

  vbox20 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox18), vbox20, TRUE, TRUE, 0);

  vbox21 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox20), vbox21, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Connection Options:"));
  gtk_box_pack_start (GTK_BOX (vbox21), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  packet_debugging = gtk_check_button_new_with_label (_("Packet debugging"));
  gtk_box_pack_start (GTK_BOX (vbox21), packet_debugging, FALSE, FALSE, 0);

  auto_login = gtk_check_button_new_with_label (_("Auto-login at startup"));
  gtk_box_pack_start (GTK_BOX (vbox21), auto_login, FALSE, FALSE, 0);

  auto_reconnect = gtk_check_button_new_with_label (_("Auto-reconnect if disconnected"));
  gtk_box_pack_start (GTK_BOX (vbox21), auto_reconnect, FALSE, FALSE, 0);

  hbox37 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox21), hbox37, TRUE, TRUE, 0);

  use_ping_time = gtk_check_button_new_with_label (_("Ping keepalive (time in minutes)"));
  gtk_box_pack_start (GTK_BOX (hbox37), use_ping_time, FALSE, FALSE, 0);

  ping_minutes_adj = gtk_adjustment_new (5, 0, 10000, 1, 10, 10);
  ping_minutes = gtk_spin_button_new (GTK_ADJUSTMENT (ping_minutes_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox37), ping_minutes, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox37), gtk_label_new(" "), TRUE, TRUE, 0);

  use_proxy = gtk_check_button_new_with_label (_("Connect via Proxy"));
  gtk_box_pack_start (GTK_BOX (vbox21), use_proxy, FALSE, FALSE, 0);

  hbox38 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox21), hbox38, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Proxy Server: "));
  gtk_box_pack_start (GTK_BOX (hbox38), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  proxy_host = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (hbox38), proxy_host, TRUE, TRUE, 0);

  hbox39 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox21), hbox39, TRUE, TRUE, 0);

  myslabel = gtk_label_new (_("Proxy Port: "));
  gtk_box_pack_start (GTK_BOX (hbox39), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  proxy_port_adj = gtk_adjustment_new (80, 0, 65534, 1, 10, 10);
  proxy_port = gtk_spin_button_new (GTK_ADJUSTMENT (proxy_port_adj), 1, 0);
  gtk_box_pack_start (GTK_BOX (hbox39), proxy_port, FALSE, FALSE, 0);
  gtk_widget_set_usize(proxy_port, 65,-2);

  gtk_box_pack_start (GTK_BOX (hbox39), gtk_label_new (""), TRUE, TRUE, 0);


  activate_profnames = gtk_check_button_new_with_label (_("Activate my profile names"));
  gtk_box_pack_start (GTK_BOX (vbox21), activate_profnames, FALSE, FALSE, 0);

  emulate_ymsg6 = gtk_check_button_new_with_label (_("Enable Messenger-6 features"));
  gtk_box_pack_start (GTK_BOX (vbox21), emulate_ymsg6, FALSE, FALSE, 0);


  wcambox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (vbox21), wcambox, FALSE, FALSE, 0);

  wcamlabel = gtk_label_new (_("File Server:"));
  gtk_box_pack_start (GTK_BOX (wcambox), wcamlabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (wcamlabel), GTK_JUSTIFY_LEFT);

  webcam_viewer_combo=gtk_combo_new();
  file_transfer_server=GTK_COMBO(webcam_viewer_combo)->entry;
  gtk_entry_set_max_length(GTK_ENTRY(file_transfer_server),72);		 
  gtk_editable_set_editable(GTK_EDITABLE(file_transfer_server),FALSE );
  gtk_box_pack_start (GTK_BOX (wcambox), webcam_viewer_combo, TRUE, TRUE, 3);

  gtk_tooltips_set_tip(MYTIPS,file_transfer_server,_("The file server is used when you send files to other users or share a buddy image in a PM window."),_("The file server is used when you send files to other users or share a buddy image in a PM window.") );

  if (! ftservlist) {
  ftservlist = g_list_append( ftservlist, "filetransfer.msg.yahoo.com" );
  ftservlist = g_list_append( ftservlist, "ft1.msg.vip.dcn.yahoo.com" );  
  ftservlist = g_list_append( ftservlist, "ft1.msg.dcn.yahoo.com" );  
  ftservlist = g_list_append( ftservlist, "ft2.msg.dcn.yahoo.com" );  
  ftservlist = g_list_append( ftservlist, "ft3.msg.dcn.yahoo.com" );  
  ftservlist = g_list_append( ftservlist, "ft4.msg.dcn.yahoo.com" );  
 /*  ftservlist = g_list_append( ftservlist, "ft1.msg.yahoo.com" );  */ 
	}
  gtk_combo_set_popdown_strings( GTK_COMBO(webcam_viewer_combo), ftservlist );


  gtk_box_pack_start (GTK_BOX (vbox20), gtk_label_new (""), TRUE, TRUE, 0);

  vbox22 = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox20), vbox22, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Save Settings Options:"));
  gtk_box_pack_start (GTK_BOX (vbox22), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  remember_password = gtk_check_button_new_with_label (_("Save password in \nconfiguration file"));
  gtk_box_pack_start (GTK_BOX (vbox22), remember_password, FALSE, FALSE, 0);

  remember_position = gtk_check_button_new_with_label (_("Remember main window\nposition & size"));
  gtk_box_pack_start (GTK_BOX (vbox22), remember_position, FALSE, FALSE, 0);


  vbox19 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox18), vbox19, TRUE, TRUE, 0);

  vbox24 = gtk_vbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (vbox19), vbox24, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Miscellaneous Options:"));
  gtk_box_pack_start (GTK_BOX (vbox24), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);


  profile_viewer = gtk_radio_button_new_with_mnemonic (NULL, _("Use builtin profile viewer (My Yahoo)"));
  gtk_box_pack_start (GTK_BOX (vbox24), profile_viewer, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (profile_viewer), profile_viewer_group);
  profile_viewer_group = gtk_radio_button_group (GTK_RADIO_BUTTON (profile_viewer));

  profile_viewer2 = gtk_radio_button_new_with_mnemonic (NULL, _("Use builtin profile viewer (Window)"));
  gtk_box_pack_start (GTK_BOX (vbox24), profile_viewer2, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (profile_viewer2), profile_viewer_group);
  profile_viewer_group = gtk_radio_button_group (GTK_RADIO_BUTTON (profile_viewer2));

  profile_viewer3 = gtk_radio_button_new_with_mnemonic (NULL, _("Use my web browser to view profiles"));
  gtk_box_pack_start (GTK_BOX (vbox24), profile_viewer3, FALSE, FALSE, 0);
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (profile_viewer3), profile_viewer_group);
  profile_viewer_group = gtk_radio_button_group (GTK_RADIO_BUTTON (profile_viewer3));



  myslabel = gtk_label_new (_("Default Browser Command\n(use %s for URL):")); 
  gtk_box_pack_start (GTK_BOX (vbox24), myslabel, FALSE, FALSE, 3);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  browser_command = gtk_entry_new ();
gtk_entry_set_max_length(GTK_ENTRY(browser_command),128);
  gtk_box_pack_start (GTK_BOX (vbox24), browser_command, FALSE, FALSE, 0);
  gtk_entry_set_text (GTK_ENTRY (browser_command), "opera -remote 'openURL(%s,new-page)' &");

  hbox36 = gtk_hbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox24), hbox36, TRUE, TRUE, 0);

  browser_netscape = gtk_button_new_with_mnemonic (_("Netscape"));
  gtk_box_pack_start (GTK_BOX (hbox36), browser_netscape, TRUE, TRUE, 0);

  browser_mozilla = gtk_button_new_with_mnemonic (_("Mozilla"));
  gtk_box_pack_start (GTK_BOX (hbox36), browser_mozilla, TRUE, TRUE, 0);

  browser_opera = gtk_button_new_with_mnemonic (_("Opera"));
  gtk_box_pack_start (GTK_BOX (hbox36), browser_opera, TRUE, TRUE, 0);

  browser_konquerer = gtk_button_new_with_mnemonic (_("Konquerer"));
  gtk_box_pack_start (GTK_BOX (hbox36), browser_konquerer, TRUE, TRUE, 0);



  playbox1 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox19), playbox1, TRUE, TRUE, 0);
  playlab1 = gtk_label_new (_("Flash Player Command: "));
  gtk_box_pack_start (GTK_BOX (playbox1), playlab1, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (playlab1), GTK_JUSTIFY_LEFT);
  flash_player_cmd = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (playbox1), flash_player_cmd, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(flash_player_cmd), 50);

  playbox2 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox19), playbox2, TRUE, TRUE, 0);
  playlab2 = gtk_label_new (_("MP3 Player Command: "));
  gtk_box_pack_start (GTK_BOX (playbox2), playlab2, TRUE, TRUE, 0);
  gtk_label_set_justify (GTK_LABEL (playlab2), GTK_JUSTIFY_LEFT);
  mp3_player = gtk_entry_new ();
  gtk_box_pack_start (GTK_BOX (playbox2), mp3_player, TRUE, TRUE, 0);
  gtk_entry_set_max_length(GTK_ENTRY(mp3_player), 64);

  gtk_tooltips_set_tip(MYTIPS,mp3_player,_("The MP3 player command allows you to listen to such things as Audibles. You should use a command-line MP3 player such as 'mplayer' or 'mpg123'."),_("The MP3 player command allows you to listen to such things as Audibles. You should use a command-line MP3 player such as 'mplayer' or 'mpg123'."));

  gtk_tooltips_set_tip(MYTIPS,flash_player_cmd,_("The Flash player command allows you to view  such things as avatar animations. Suggested commands are 'gflashplayer' and 'swf_play'."),_("The Flash player command allows you to view  such things as avatar animations. Suggested commands are 'gflashplayer' and 'swf_play'."));


  gtk_box_pack_start (GTK_BOX (vbox19), gtk_label_new (""), TRUE, TRUE, 0);

  vboxpopup = gtk_vbox_new (FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox19), vboxpopup, FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("Show A Popup Window When:"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), myslabel, FALSE, FALSE, 2);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  gtk_misc_set_alignment (GTK_MISC (myslabel), 0, 0.5);

  popup_buddy_on = gtk_check_button_new_with_label (_("A buddy comes online"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), popup_buddy_on, FALSE, FALSE, 0);

  popup_buddy_off = gtk_check_button_new_with_label (_("A buddy goes offline"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), popup_buddy_off, FALSE, FALSE, 0);

  popup_buzz = gtk_check_button_new_with_label (_("A user buzzes me"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), popup_buzz, FALSE, FALSE, 0);

  popup_new_mail = gtk_check_button_new_with_label (_("New mail arrives"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), popup_new_mail, FALSE, FALSE, 0);

  popup_news_bulletin= gtk_check_button_new_with_label (_("News bulletins arrive"));
  gtk_box_pack_start (GTK_BOX (vboxpopup), popup_news_bulletin, FALSE, FALSE, 0);


  myslabel = gtk_label_new (_("Options"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 5), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  vbox16 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox16);

  hbox32 = gtk_hbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (vbox16), hbox32, TRUE, TRUE, 0);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox32), scrolledwindow1, TRUE, TRUE, 0);

  col_headers[0]=_("Yahoo Logins");
  login_list =  GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST, 
	GYLIST_TYPE_SINGLE, 1, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), login_list);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_ETCHED_IN);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox32), scrolledwindow2, TRUE, TRUE, 0);

  col_headers[0]=_("Favorite Rooms");
  favroom_list =  GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST, 
	GYLIST_TYPE_SINGLE, 1, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), favroom_list);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_SHADOW_ETCHED_IN);


  scrolledwindow3 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox32), scrolledwindow3, TRUE, TRUE, 0);

  col_headers[0]=_("Friend");
  friend_list =  GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST, 
	GYLIST_TYPE_SINGLE, 1, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow3), friend_list);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow3), GTK_SHADOW_ETCHED_IN);

  hbox32 = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox16), hbox32, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox32), stoolbars(MYTIPS, login_list), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), stoolbars(MYTIPS, favroom_list), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), stoolbars(MYTIPS, friend_list ), FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("* right-click mouse on list for edit menu *"));
  gtk_box_pack_start (GTK_BOX (vbox16), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  myslabel = gtk_label_new (_("Accounts,\nRooms,\nFriends"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 6), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  vbox17 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (notebook1), vbox17);

  hpaned1 = gtk_hpaned_new ();
  gtk_box_pack_start (GTK_BOX (vbox17), hpaned1, TRUE, TRUE, 0);
  /* gtk_paned_set_gutter_size (GTK_PANED (hpaned1), 10); */ 
  gtk_paned_set_position (GTK_PANED (hpaned1), 360);

  scrolledwindow5 = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_pack1 (GTK_PANED (hpaned1), scrolledwindow5, TRUE, TRUE);

  col_headers[0]=_("Ignored Users");
  ignore_list =  GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST, 
	GYLIST_TYPE_SINGLE, 1, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow5),  ignore_list);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow5), GTK_SHADOW_ETCHED_IN);

  scrolledwindow4 = gtk_scrolled_window_new (NULL, NULL);
  gtk_paned_pack2 (GTK_PANED (hpaned1), scrolledwindow4, TRUE, TRUE);

  col_headers[0]=_("Regex to Auto-ignore");
  regex_list =  GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST, 
	GYLIST_TYPE_SINGLE, 1, 1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow4),  regex_list);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow4), GTK_SHADOW_ETCHED_IN);


  hbox32 = gtk_hbox_new (FALSE, 1);
  gtk_box_pack_start (GTK_BOX (vbox17), hbox32, FALSE, FALSE, 2);
  gtk_box_pack_start (GTK_BOX (hbox32), stoolbars(MYTIPS, ignore_list), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), gtk_label_new(" "), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox32), stoolbars(MYTIPS, regex_list), FALSE, FALSE, 0);

  myslabel = gtk_label_new (_("* right-click mouse on list for edit menu *"));
  gtk_box_pack_start (GTK_BOX (vbox17), myslabel, FALSE, FALSE, 0);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  myslabel = gtk_label_new (_("Ignored\nUsers"));
  gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 7), myslabel);
  gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);

  hbox1 = gtk_hbox_new (FALSE, 20);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, TRUE, 0);

  save_setup =get_pixmapped_button(_(" Save "), GTK_STOCK_SAVE);
  gtk_box_pack_start (GTK_BOX (hbox1), save_setup, TRUE, TRUE, 0);

  gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("   "), TRUE, TRUE, 0);

  cancel_setup = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox1), cancel_setup, TRUE, TRUE, 0);

 

  /* Store pointers to all widgets, for use by lookup_widget(). */
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, setup_window, "setup_window");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, notebook1, "notebook1");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_html, "show_html");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, py_voice_helper, "py_voice_helper");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_emoticons, "show_emoticons");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, support_scentral_smileys, "support_scentral_smileys");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, allow_animations, "allow_animations");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, max_simult_animations, "max_simult_animations");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, esd_sound_events, "esd_sound_events");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_avatars, "show_avatars");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, send_avatar, "send_avatar");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_fonts, "show_fonts");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_colors, "show_colors");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, blended_colors, "blended_colors");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,  enter_leave_timestamp, "enter_leave_timestamp");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,  enable_webcam_features, "enable_webcam_features");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_enters, "show_enters");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_statuses, "show_statuses");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, use_history_limit, "use_history_limit");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, history_limit, "history_limit");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, filter_command, "filter_command");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_brings_back, "pm_brings_back");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_away, "auto_away");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_away_time, "auto_away_time");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, custom_away_message, "custom_away_message");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, notebook2, "notebook2");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_black, "color_black");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_red, "color_red");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_blue, "color_blue");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_green, "color_green");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_yellow, "color_yellow");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_purple, "color_purple");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_cyan, "color_cyan");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_orange, "color_orange");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_gray, "color_gray");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, color_custom, "color_custom");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, style_bold, "style_bold");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, style_underline, "style_underline");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, style_italic, "style_italic");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, colorselection, "colorselection");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, fontselection1, "fontselection1");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, chat_timestamp, "chat_timestamp");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, force_lowercase, "force_lowercase");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, limit_lfs, "limit_lfs");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, limit_lfs_count, "limit_lfs_count");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, limit_buffer_size, "limit_buffer_size");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, buffer_size, "buffer_size");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, indent_wrapped, "indent_wrapped");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, transparent_bg, "transparent_bg");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, shading_r, "shading_r");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, shading_g, "shading_g");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, shading_b, "shading_b");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, highlight_friends, "highlight_friends");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_reply_when_away, "auto_reply_when_away");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_reply_msg, "auto_reply_msg");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, chat_timestamp_pm, "chat_timestamp_pm");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_in_sep_windows, "pm_in_sep_windows");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_raise_pm, "auto_raise_pm");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, esd_sound_events_pm, "esd_sound_events_pm");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, no_random_friend_add, "no_random_friend_add");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, no_sent_files, "no_sent_files");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, basic_boot_protect, "basic_boot_protect");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, preempt_boot_protection, "preempt_boot_protection");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_from_friends, "pm_from_friends");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_from_users, "pm_from_users");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_from_all, "pm_from_all");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, disp_auto_ignored, "disp_auto_ignored");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, suppress_mult, "suppress_mult");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_on_mults, "mute_on_mults");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_on_mults_count, "mute_on_mults_count");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, ignore_on_mults, "ignore_on_mults");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, ignore_on_mults_count, "ignore_on_mults_count");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, first_post_is_url, "first_post_is_url");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, first_post_is_pm, "first_post_is_pm");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, url_from_nonroom_user, "url_from_nonroom_user");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, never_ignore_friend, "never_ignore_friend");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_login, "auto_login");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_reconnect, "auto_reconnect");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, use_ping_time, "use_ping_time");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, ping_minutes, "ping_minutes");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, use_proxy, "use_proxy");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, proxy_host, "proxy_host");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, proxy_port, "proxy_port");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_close_roomlist, "auto_close_roomlist");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_adult, "show_adult");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, cache_room_list, "cache_room_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, profile_viewer, "profile_viewer");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, profile_viewer2, "profile_viewer2");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, profile_viewer3, "profile_viewer3");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, packet_debugging, "packet_debugging");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, browser_command, "browser_command");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, browser_netscape, "browser_netscape");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, browser_mozilla, "browser_mozilla");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, browser_opera, "browser_opera");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, browser_konquerer, "browser_konquerer");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, remember_password, "remember_password");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, remember_position, "remember_position");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, scrolledwindow1, "scrolledwindow1");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, login_list, "login_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, scrolledwindow2, "scrolledwindow2");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, favroom_list, "favroom_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, scrolledwindow3, "scrolledwindow3");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, friend_list, "friend_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, hpaned1, "hpaned1");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, scrolledwindow5, "scrolledwindow5");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, ignore_list, "ignore_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, scrolledwindow4, "scrolledwindow4");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, regex_list, "regex_list");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, save_setup, "save_setup");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, cancel_setup, "cancel_setup");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, highlight_friends_chat, "highlight_friends_chat");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, highlight_me_chat, "highlight_me_chat");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, locale_status, "locale_status");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_first, "contact_first");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_last, "contact_last");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_email, "contact_email");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_home, "contact_home");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_work, "contact_work");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, contact_mobile, "contact_mobile");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_noage, "mute_noage");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_minors, "mute_minors");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_nogender, "mute_nogender");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_males, "mute_males");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mute_females, "mute_females");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_tuxvironments, "enable_tuxvironments");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_chat_spam_filter, "enable_chat_spam_filter");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_trayicon, "enable_trayicon");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, webcam_viewer_entry, "webcam_viewer_app");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, fonts_scale_entry, "custom_scale_text");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, webcam_device, "webcam_device");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_encryption, "enable_encryption");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, popup_buzz, "popup_buzz");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, popup_buddy_on, "popup_buddy_on");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, popup_buddy_off, "popup_buddy_off");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, popup_new_mail, "popup_new_mail");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, popup_news_bulletin, "popup_news_bulletin");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,use_buddy_list_font, "use_buddy_list_font");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,show_quick_access_toolbar, "show_quick_access_toolbar");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,show_my_status_in_chat, "show_my_status_in_chat");
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window,pm_nick_names, "pm_nick_names");
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_yavatars, "show_yavatars"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_audibles, "enable_audibles"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, activate_profnames, "activate_profnames"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_bimages, "show_bimages"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, auto_reject_invitations, "auto_reject_invitations"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pmb_audibles, "pmb_audibles"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pmb_smileys, "pmb_smileys"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pmb_send_erase, "pmb_send_erase"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, flash_player_cmd, "flash_player_cmd"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, mp3_player, "mp3_player"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, emulate_ymsg6, "emulate_ymsg6"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pm_window_style, "pm_window_style"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pmb_toolbar1, "pmb_toolbar1"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, pmb_toolbar2, "pmb_toolbar2"); 

	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, chatwin_smileys, "chatwin_smileys"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, chatwin_send_erase, "chatwin_send_erase"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, show_tabs_at_bottom1, "show_tabs_at_bottom"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_pm_searches, "enable_pm_searches"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, enable_addressbook, "enable_addressbook"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, allow_pm_buzzes, "allow_pm_buzzes"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, file_transfer_server, "file_transfer_server"); 
	  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, fontselection2, "fontselection2"); 

  gtk_signal_connect_after (GTK_OBJECT (setup_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_setup_window_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (browser_netscape), "clicked",
                      GTK_SIGNAL_FUNC (on_browser_clicked),
                      "netscape");
  gtk_signal_connect (GTK_OBJECT (browser_mozilla), "clicked",
                      GTK_SIGNAL_FUNC (on_browser_clicked),
                      "mozilla");
  gtk_signal_connect (GTK_OBJECT (browser_opera), "clicked",
                      GTK_SIGNAL_FUNC (on_browser_clicked),
                      "opera");
  gtk_signal_connect (GTK_OBJECT (browser_konquerer), "clicked",
                      GTK_SIGNAL_FUNC (on_browser_clicked),
                      "konquerer");
  gtk_signal_connect_after (GTK_OBJECT (color_custom), "toggled",
                            GTK_SIGNAL_FUNC (on_color_custom_toggled),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (login_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_setup_list_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (favroom_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_setup_list_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (friend_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_setup_list_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignore_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_setup_list_button_press_event),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (regex_list), "button_press_event",
                      GTK_SIGNAL_FUNC (on_setup_list_button_press_event),
                      NULL);

  g_signal_connect(regex_list, "popup-menu", (GCallback) on_setuplist_tree_kb_popup,NULL);
  g_signal_connect(friend_list, "popup-menu", (GCallback) on_setuplist_tree_kb_popup,NULL);
  g_signal_connect(ignore_list, "popup-menu", (GCallback) on_setuplist_tree_kb_popup,NULL);
  g_signal_connect(login_list, "popup-menu", (GCallback) on_setuplist_tree_kb_popup,NULL);
  g_signal_connect(favroom_list, "popup-menu", (GCallback) on_setuplist_tree_kb_popup,NULL);

  gtk_signal_connect (GTK_OBJECT (save_setup), "clicked",
                      GTK_SIGNAL_FUNC (on_save_setup_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancel_setup), "clicked",
                      GTK_SIGNAL_FUNC (on_cancel_setup_clicked),
                      NULL);

  /* Must do this one now */
  GLADE_HOOKUP_OBJECT_NO_REF (setup_window, ignore_guests, "ignore_guests");

  gtk_signal_connect (GTK_OBJECT (enable_chat_spam_filter), "toggled", GTK_SIGNAL_FUNC (on_enable_chat_spam_filter_toggled), NULL);

  return setup_window;
}










GtkWidget*
create_setup_menu (void)
{
  GtkWidget *setup_menu;
  GtkWidget *setup_insert;
  GtkWidget *setup_delete;
  GtkWidget *setup_edit;

  setup_menu = gtk_menu_new ();
  gtk_object_set_data (GTK_OBJECT (setup_menu), "setup_menu", setup_menu);

  setup_insert = gtk_image_menu_item_new_with_label (_("Insert"));
  gtk_widget_ref (setup_insert);
  gtk_object_set_data_full (GTK_OBJECT (setup_menu), "setup_insert", setup_insert,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (setup_menu), setup_insert);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(setup_insert), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU) ));

  setup_delete = gtk_image_menu_item_new_with_label (_("Delete"));
  gtk_widget_ref (setup_delete);
  gtk_object_set_data_full (GTK_OBJECT (setup_menu), "setup_delete", setup_delete,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (setup_menu), setup_delete);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(setup_delete), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DELETE, GTK_ICON_SIZE_MENU) ));

  setup_edit = gtk_image_menu_item_new_with_label (_("Edit"));
  gtk_widget_ref (setup_edit);
  gtk_object_set_data_full (GTK_OBJECT (setup_menu), "setup_edit", setup_edit,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (setup_menu), setup_edit);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(setup_edit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_FIND_AND_REPLACE, GTK_ICON_SIZE_MENU) ));

  gtk_signal_connect (GTK_OBJECT (setup_insert), "activate",
                      GTK_SIGNAL_FUNC (on_setup_insert_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (setup_delete), "activate",
                      GTK_SIGNAL_FUNC (on_setup_delete_activate),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (setup_edit), "activate",
                      GTK_SIGNAL_FUNC (on_setup_edit_activate),
                      NULL);

  return setup_menu;
}







GtkWidget*
create_edit_ignore_window (void)
{
  GtkWidget *edit_ignore_window;
  GtkWidget *vbox11;
  GtkWidget *edit_ignore_label;
  GtkWidget *edit_ignore_text;
  GtkWidget *hbox12;
  GtkWidget *edit_ig_save;
  GtkWidget *edit_ig_cancel;

  edit_ignore_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (edit_ignore_window), "edit_ignore_window", edit_ignore_window);
  gtk_container_set_border_width (GTK_CONTAINER (edit_ignore_window), 5);
  gtk_window_set_title (GTK_WINDOW (edit_ignore_window), _("Edit"));
  gtk_window_set_default_size (GTK_WINDOW (edit_ignore_window), 350, -1);
  gtk_window_set_modal (GTK_WINDOW (edit_ignore_window), TRUE);

  vbox11 = gtk_vbox_new (FALSE, 0);
  gtk_widget_ref (vbox11);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "vbox11", vbox11,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_container_add (GTK_CONTAINER (edit_ignore_window), vbox11);

  edit_ignore_label = gtk_label_new (_("Edit Ignore Item"));
  gtk_widget_ref (edit_ignore_label);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "edit_ignore_label", edit_ignore_label,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox11), edit_ignore_label, FALSE, FALSE, 3);

  edit_ignore_text = gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(edit_ignore_text),124);
  gtk_widget_ref (edit_ignore_text);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "edit_ignore_text", edit_ignore_text,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox11), edit_ignore_text, FALSE, FALSE, 5);

  hbox12 = gtk_hbox_new (TRUE, 0);
  gtk_widget_ref (hbox12);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "hbox12", hbox12,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (vbox11), hbox12, FALSE, TRUE, 3);
  gtk_container_set_border_width (GTK_CONTAINER (hbox12), 5);

  edit_ig_save = get_pixmapped_button(_(" Save "), GTK_STOCK_SAVE);
  gtk_widget_ref (edit_ig_save);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "edit_ig_save", edit_ig_save,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox12), edit_ig_save, FALSE, FALSE, 0);

  edit_ig_cancel = get_pixmapped_button(_(" Cancel "), GTK_STOCK_CANCEL);
  gtk_widget_ref (edit_ig_cancel);
  gtk_object_set_data_full (GTK_OBJECT (edit_ignore_window), "edit_ig_cancel", edit_ig_cancel,
                            (GtkDestroyNotify) gtk_widget_unref);
  gtk_box_pack_start (GTK_BOX (hbox12), edit_ig_cancel, FALSE, FALSE, 0);

  gtk_signal_connect_after (GTK_OBJECT (edit_ignore_window), "delete_event",
                            GTK_SIGNAL_FUNC (on_edit_ignore_window_destroy_event),
                            NULL);
  gtk_signal_connect (GTK_OBJECT (edit_ig_save), "clicked",
                      GTK_SIGNAL_FUNC (on_edit_ig_save_clicked),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (edit_ig_cancel), "clicked",
                      GTK_SIGNAL_FUNC (on_edit_ig_cancel_clicked),
                      NULL);



  return edit_ignore_window;
}







