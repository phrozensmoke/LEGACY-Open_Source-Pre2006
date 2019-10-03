/*****************************************************************************
 * callbacks.h
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

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>


void
on_b_send_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_window1_map                         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_chat_entry_map                      (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_chat_entry_activate                 (GtkEditable     *editable,
                                        gpointer         user_data);


void
on_aliases_map                        (GtkWidget       *widget,
                                        gpointer         user_data);


void collect_chat_list_selected(GtkTreeSelection *selection, gpointer data);
void onChatUsersRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);
gboolean on_chatlist_kb_popup  (GtkWidget  *widget, gpointer user_data);


void
on_users_map                           (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_aliases_clicked                    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void onAliasesRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);
gboolean on_aliases_kb_popup  (GtkWidget  *widget, gpointer user_data);

void
on_connect_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_disconnect_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_room_list_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_favorites_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_to_favorites_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_profile_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pm_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_to_friends_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_ignore_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_ignore_broadcast_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);


void
on_unignore                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_unmute                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_domute                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_users_clicked                       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_pm_send_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_pm_cancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_pm_user_map                         (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pm_entry_activate                   (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_login_button_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancel_login_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancel_favs_clicked                 (GtkButton       *button,
                                        gpointer         user_data);


void
on_close_profile_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_alias_edit_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_alias_delete_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_alias_new_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_alias_send_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_alias_save_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_alias_cancel_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_b_clear_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_follow_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_chat_entry_key_press_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_save_current_text_activate          (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_log_to_file_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_here_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_be_right_back_activate       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_busy_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_not_at_home_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_not_at_my_desk_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_not_in_the_office_activate   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_on_the_phone_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_on_vacation_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_out_to_lunch_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_stepped_out_activate         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_status_invisible_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_packet_debugging_activate           (GtkCheckMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_file_ok_button_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_file_cancel_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_setup_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_setup_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancel_setup_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_status_custom_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_goto_room_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancel_room_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_room_tree_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);



void
on_setup_insert_activate                  (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_setup_delete_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_setup_edit_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_setup_list_button_press_event       (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_edit_ig_save_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_edit_ig_cancel_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_status_autoaway_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_pm_window_destroy_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_favorites_destroy_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_profile_window_destroy_event        (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_alias_window_destroy_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);


gboolean
on_setup_window_destroy_event          (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_edit_ignore_window_destroy_event    (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_room_list_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_exit_clicked                    (GtkButton *button,
                                        gpointer         user_data);

void
on_tgl_bold_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tgl_italics_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_tgl_underline_toggled               (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_pms_entry_activate                  (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_pms_close_clicked                   (GtkWidget       *button,
                                        gpointer         user_data);

void
on_pms_send_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_pms_clear_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_pm_session_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_pms_ignore_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_chat_status_map                     (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_pms_profile_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void on_room_tree_tree_expand  (GtkTreeView *treeview, GtkTreeIter *iter, GtkTreePath *treepath,  gpointer user_data);

void onRoomListActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);


gboolean on_room_tree_kb_popup (GtkWidget  *widget, gpointer user_data);

gboolean
on_favroom_list_button_press_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_goto_fav_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_chat_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

gboolean
on_pms_text_button_press_event         (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_color_custom_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_reload_room_list_clicked            (GtkButton       *button,
                                        gpointer         user_data);


gboolean
on_pms_entry_key_press_event           (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_find_button_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_clear_find_button_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_cancel_find_button_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_find_next_button_clicked            (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_find_window_delete_event            (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_search_text_activate                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_browser_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void 
on_blank_pm_window            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_pms_buzz_clicked                    (GtkButton       *button,
                                        gpointer         user_data);



/* added: PhrozenSmoke, for room creation */

gboolean
on_cancel_croom_clicked              (GtkWidget       *widget,                       
                                        gpointer         user_data);

gboolean
on_cancel_croom_clickedw             (GtkWidget       *widget,       
                                        GdkEvent        *event,                
                                        gpointer         user_data);

gboolean
on_create_croom_clicked              (GtkButton       *widget,                       
                                        gpointer         user_data);




void on_create_room_activate     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);



/* added PhrozenSmoke,buddy list callbacks */
gboolean on_select_a_buddy    (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_buddy_pm_activate                         (GtkButton     *bbutton,
                                        gpointer         user_data);


void  on_buddy_profile_activate       (GtkButton     *bbutton,
                                        gpointer         user_data);

void
on_buddy_buzz                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buddy_join_in_chat                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_buddy_invite_to_chat                    (GtkButton       *button,
                                        gpointer         user_data);

void on_display_buddy_list_popup_menu_kb(GtkWidget *widget, gpointer user_data);
void onBuddyListRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);
void collect_buddy_list_selected(GtkTreeSelection *selection, gpointer data);


void on_pms_sendfile_clicked           (GtkButton       *button,
                                        gpointer         user_data);
void on_pms_goto_clicked           (GtkButton       *button,
                                        gpointer         user_data);


void on_add_this_buddy(GtkButton *widget, gpointer user_data);
void on_click_add_a_buddy(GtkButton *widget, gpointer user_data);
void on_click_del_this_buddy(GtkButton *widget, gpointer user_data);
void on_click_del_a_buddy(GtkButton *widget, gpointer user_data);
void on_send_buddy_file(GtkButton *widget, gpointer user_data);
void on_send_user_file_cb(GtkButton *widget, gpointer user_data) ;
void on_send_file_to_user_cb(GtkMenuItem *widget, gpointer user_data);
void on_find_menu_clicked(GtkMenuItem *mitem, gpointer user_data);

void on_open_profile_cb(GtkMenuItem *widget, gpointer user_data);
void on_open_goto_cb(GtkMenuItem *widget, gpointer user_data);

void  on_conf_decline_cb                    (GtkButton       *button,
                                        gpointer         user_data);
void  on_conf_accept_cb                    (GtkButton       *button,
                                        gpointer         user_data);
void on_invite_to_conf(GtkMenuItem *widget, gpointer user_data);
void on_start_conference(GtkMenuItem *widget, gpointer user_data);

void  on_activate_voice_conf                    (GtkButton       *button,
                                        gpointer         user_data);

void  on_accept_voice_conf                    (GtkButton       *button,
                                        gpointer         user_data);

void on_py_voice_enable(GtkMenuItem *widget, gpointer user_data);
void on_launch_py_voice(GtkMenuItem *widget, gpointer user_data);

void  on_reject_buddy_add                    (GtkButton       *button,
                                        gpointer         user_data);

void on_refresh_activate       (GtkMenuItem     *menuitem, gpointer         user_data);

void  on_room_invite_accept_cb                    (GtkButton       *button,
                                        gpointer         user_data);

void on_view_briefcase  (GtkMenuItem *menuitem, gpointer  user_data);
void on_make_custom_away  (GtkMenuItem *menuitem, gpointer  user_data);

void onFavoriteRoomRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data);
void on_favoriteroom_tree_kb_popup(GtkWidget *widgy, gpointer user_data);
void on_setuplist_tree_kb_popup(GtkWidget *widgy, gpointer user_data);
void on_buddy_list_rename (GtkMenuItem  *menuitem,   gpointer user_data);
void on_move_buddy_to_group (GtkMenuItem  *menuitem,   gpointer user_data);
void on_open_multi_pms (GtkMenuItem  *menuitem,   gpointer user_data);
void on_activate_yprivacy(GtkWidget *button, gpointer user_data);
void on_open_yprivacy_window  (GtkMenuItem *menuitem, gpointer  user_data);

void on_activate_stealth_window  (GtkButton *widget, gpointer  user_data);
void on_open_stealth_window  (GtkMenuItem *menuitem, gpointer  user_data);
void on_gameprofile  (GtkMenuItem *menuitem, gpointer  user_data);
void on_viewgameroom  (GtkMenuItem *menuitem, gpointer  user_data);
void on_viewavatar  (GtkMenuItem *menuitem, gpointer  user_data);
void on_viewbuddyinfo  (GtkMenuItem *menuitem, gpointer  user_data);
void on_send_audible_cb(GtkWidget *widget, gpointer user_data);

extern void preselected_filename(char *filename);
extern GtkWidget *create_budgrp_popup_menu();
extern GtkWidget *create_list_dialog(char *wintitle, char *header, char *ldefault, GList *comblist, int editable, int icon);
extern void jump_to_chat_tab();
extern void set_myyahoo_html_data(char *myhtmldata);
extern GtkWidget* create_stealth_window (char *who);
extern void show_audibles_window(gpointer pms_sess);
extern GList *get_buddy_group_list();
extern int pm_nick_names;

extern void on_leave_room_or_conf(GtkMenuItem *widget, gpointer user_data);
extern void on_yahoo_games(GtkMenuItem *widget, gpointer user_data);
extern void on_report_abuse(GtkMenuItem *widget, gpointer user_data);
extern void on_yahoo_mail_act(GtkMenuItem *widget, gpointer user_data);
extern void on_qprofile_activate(GtkMenuItem *widget, gpointer user_data);
extern void on_rejoin_room  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_send_contact_info  (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_perm_ignore (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_temp_friend_editor (GtkMenuItem *menuitem, gpointer  user_data); 
extern void open_flooder_editor (GtkMenuItem *menuitem, gpointer  user_data);
extern void open_ignore_editor (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_status_idle_activate (GtkMenuItem *menuitem, gpointer user_data);
extern void on_viewcam_room  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_viewcam  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_startmycam  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_joinaroom  (GtkMenuItem *menuitem, gpointer  user_data);
extern GtkWidget *get_my_yahoo_panel();
extern GtkWidget *get_pm_icon(int which_icon, char *err_str) ;
extern void on_add_tmp_friend  (GtkMenuItem   *menuitem, gpointer user_data);
extern void on_sendcaminvite_room  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_sendcaminvite  (GtkMenuItem *menuitem, gpointer  user_data);
extern gboolean on_login_window_destroy_event  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data) ;
extern void on_show_camera_on  (GtkCheckMenuItem *menuitem, gpointer user_data);
extern void on_display_plugin_info  (GtkMenuItem *menuitem, gpointer  user_data);

extern void voice_launch_conference(int with_warning, char *yuser, char *yroom) ;
extern void voice_launch(int with_warning);
extern void show_yahoo_packet();
extern char *get_quick_profile(char *user);
extern char *get_screenname_alias(char *user);
extern void set_room_list_country_code(char *intlstr);
extern void append_timestamp(GtkWidget *somewidget, GtkWidget *somewidget2);
extern void focus_pm_entry(GtkWidget *pmwin);
extern GtkWidget* create_addfavroom_menu ();
extern GtkWidget *create_budlist_popup_menu();
extern void set_text_entry_styles();
extern void update_buddy_clist();
extern void load_myyahoo_URL(char *turl);

extern void on_copy_room_user_name (GtkMenuItem *menuitem, gpointer nick_user_data);
extern void on_photoalbum  (GtkMenuItem *menuitem, gpointer  user_data);
extern void on_blist_get_yab (GtkMenuItem *widget, gpointer user_data);
extern void on_check_gyache_version_cb(GtkMenuItem *widget, gpointer user_data);

extern void open_tuxvironment_selection(GtkWidget *button, gpointer user_data);
extern void on_open_audible_window(GtkWidget *button, gpointer user_data);
extern void on_pms_sendcaminvite (GtkMenuItem *widget, gpointer user_data);
extern void on_pms_viewcam (GtkMenuItem *widget, gpointer user_data);
extern void on_pms_add_friend (GtkMenuItem *widget, gpointer user_data);
extern void on_pms_get_yab (GtkMenuItem *widget, gpointer user_data);
extern void on_pms_yphoto_start (GtkMenuItem *widget, gpointer user_data);


extern char *last_status_msg;
extern void set_tooltip(GtkWidget *somewid, char *somechar);
extern int login_invisible;
extern int login_noroom;
extern int use_chat_settings;
extern int use_multpm_settings;
extern int in_a_chat;
extern int ignore_ymsg_online;
extern char *last_chatroom ;
extern char *last_chatserver ;
extern int force_busy_icon;
extern int force_idle_icon;
/* added: PhrozenSmoke - from ignore.c */
extern int BROADCAST_IGNORE;

extern char *my_avatar;

extern void on_create_fadeconfigwin (GtkButton  *button, gpointer user_data);
void on_yahoo_news_alert(GtkMenuItem *widget, gpointer user_data);
void on_remove_me_cb(GtkButton *widget, gpointer user_data);
void on_perm_offline_input_cb(GtkButton *widget, gpointer user_data);

void on_bimage_view_animation  (GtkMenuItem *menuitem, gpointer  user_data);
void on_bimage_view_avatar  (GtkMenuItem *menuitem, gpointer  user_data);
void on_bimage_hide_cb  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_show_cb  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_update_size1  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_update_size2  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_update_size3  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_share  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_share_off  (GtkMenuItem *widget, gpointer user_data);
void on_bimage_edit_my_photo(GtkButton *widget, gpointer user_data);
void on_activate_bimage_config (GtkButton *button, gpointer  user_data);
void on_refresh_bavatar (GtkButton *button, gpointer  user_data);
void on_delete_all_yavatars (GtkButton *button, gpointer  user_data);
void on_goto_edit_avatar  (GtkButton *button, gpointer  user_data);
void on_delete_all_bimages (GtkButton *button, gpointer  user_data);
void on_select_buddy_image(GtkButton *widget, gpointer user_data);

gboolean on_refresh_intl_croom_clicked (GtkWidget *widget, gpointer user_data);
void on_bimage_view_my_avatar_cb  (GtkMenuItem *menuitem, gpointer  user_data);
void on_bimage_edit_my_photo_menu  (GtkMenuItem *menuitem, gpointer  user_data);
void on_goto_edit_avatar_menu (GtkMenuItem *menuitem, gpointer  user_data);

void on_buddy_add_accept(GtkButton *widget, gpointer user_data);

void on_doimmunity (GtkMenuItem *menuitem,  gpointer  user_data);


extern void handle_y_search_results(GtkWidget *window, GtkWidget *textbox, char *intext);
extern void search_open_yab(char *user);
extern int upload_new_buddy_image();
extern void set_voice_conference_cookie(char *cook);


#endif


