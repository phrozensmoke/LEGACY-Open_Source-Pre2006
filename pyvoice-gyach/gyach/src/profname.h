/*****************************************************************************
 * profname.h
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
#ifndef _PROFNAME_H_
#define _PROFNAME_H_

#ifndef PROFNAMESTR
#define PROFNAMESTR "profname"
#endif

void set_primary_screen_name(char *somesn);
char *get_primary_screen_name();
void set_default_profile_name(char *somesn);
char *get_default_profile_name();
void set_chat_profile_name(char *somesn);
char *get_chat_profile_name();
void set_current_chat_profile_name(char *somesn);
char *get_current_chat_profile_name();
void set_current_pm_profile_name(char *somesn);
char *get_current_pm_profile_name();
void reset_current_pm_profile_name();

int find_profile_name( char *profnm );
void add_profile_name( char *profnm );
void clear_profile_names();
int get_profile_name_count();
void reset_profile_name_config();
GtkWidget *get_profile_name_component(int with_primary);
void create_profilename_window();
void create_profilename_window_cb(GtkMenuItem *menuitem, gpointer  user_data);
char *retrieve_profname_str (GtkWidget *widgy);
GtkEntry *retrieve_profname_entry (GtkWidget *widgy);
void parse_profilenames(char *somefield);
char *select_profile_name(int packet_type, int inpm);
void reset_profile_replyto_name();
void collect_profile_replyto_name();
char *get_profile_replyto_name();
void update_existing_profile_name_widgets();
void activate_all_profile_names();

#endif /* #ifndef _PROFNAME_H_ */

