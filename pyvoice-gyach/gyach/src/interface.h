/*****************************************************************************
 * interface.h
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

#ifndef _INTERFACE_H_
#define _INTERFACE_H_

  GtkWidget *bd_tree;

GtkWidget *bimdiag_radio1;
GtkWidget *bimdiag_radio2;
GtkWidget *bimdiag_radio3;
GtkWidget *bimdiag_entry1;
GtkWidget *bimdiag_entry2;
GtkWidget *bimdiag_check1;
GtkWidget *bimdiag_check2;
GtkWidget *bimdiag_check3;
GtkWidget *bimdiag_check4;
GtkWidget *bimdiag_size_radio1;
GtkWidget *bimdiag_size_radio2;
GtkWidget *bimdiag_size_radio3;
GtkWidget *bimdiag_window;
GtkWidget *bimdiag_image1;
GtkWidget *bimdiag_image2;
GtkWidget* create_bimage_select_window ();

GtkWidget* create_window1 (void);
GtkWidget* create_user_menu (void);
GtkWidget* create_pm_window (void);
GtkWidget* create_login_window (void);
GtkWidget* create_favorites (void);
GtkWidget* create_profile_window (void);
GtkWidget* create_alias_menu (void);
GtkWidget* create_alias_window (void);
GtkWidget* create_fileselection (void);
GtkWidget* create_setup_window (void);
GtkWidget* create_room_list (void);
GtkWidget* create_setup_menu (void);
GtkWidget* create_edit_ignore_window (void);
GtkWidget* create_pm_session (void);
GtkWidget *blank_pm_window (char *some_sn);  /* added: PhrozenSmoke */
GtkWidget* create_find_window (void);
/* added: PhrozenSmoke - from interface.c, fader support */
GtkWidget *use_chat_fader;
GtkWidget *fader_text_start;
GtkWidget *fader_text_end;
GtkWidget *login_credentials;
/* added: PhrozenSmoke - call back for making a Yahoo room */
GtkWidget* create_room_maker (char *mname);

GtkWidget* create_buddy_list_tab ();
GtkWidget* create_notebook (GtkWidget *chatframe, GtkWidget *buddyframe);

/* added PhrozenSmoke: common dialogs */
void show_ok_dialog(char *mymsg);
GtkWidget *show_confirm_dialog(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel);
GtkWidget *show_input_dialog(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel);
GtkWidget *create_fileselection2 ();

void show_credentials_box(GtkWidget *window, GdkPixbuf *imbuf);
GtkWidget* create_yprivacy_window ();


GtkWidget *get_smiley_button(char *filename, int allow_fail);
void show_smiley_window_chat(GtkWidget *button, gpointer user_data);
void show_smiley_window_pm(GtkWidget *button, gpointer user_data);

extern GtkWidget *chat_window;
extern char *last_file_selected;
extern char *selected_buddy_group;
extern char *selected_buddy;
extern GtkWidget *load_smiley_icon(char *str);
extern void append_to_textbox_color(GtkWidget * win, GtkWidget *tb, char *text);

extern char *fader_text_end_str;
extern char *fader_text_start_str;
extern int use_chat_fader_val;
extern int num_dialogs_too_fast;
extern char *_(char *some);  /* added PhrozenSmoke: locale support */
extern GHashTable *buddy_status;
extern int find_online_friend( char *friend );
extern int dialogs_opening_too_fast();
extern GtkTreePath *find_buddy_group(char *bud, GtkTreeModel *model);
extern void reset_buddy_groups();



#endif

