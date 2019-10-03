/*****************************************************************************
 * ignore.h
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
 * cpinkham@infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 *****************************************************************************/


#ifndef _IGNORE_H_
#define _IGNORE_H_

extern GList *mute_list;
extern GList *regex_list;
extern GHashTable *ignore_hash;

extern int  disp_auto_ignored;
extern int  ignore_on_mults;
extern int  mute_on_mults;
extern int  ignore_guests;
extern int  suppress_mult;
extern int  first_post_is_url;
extern int  first_post_is_pm;
extern int  url_from_nonroom_user;

void ignore_add( char *user );
void ignore_remove( char *user );
int ignore_check( char *user );
char *auto_ignore_check( char *str );
void ignore_load();
void ignore_save();
void ignore_toggle( char *user );
int ignore_count_in_room();
int ignore_count();
void mute_add( char *user );
void mute_remove( char *user );
int mute_check( char *user );
void mute_toggle( char *user );
int mute_count_in_room();
void regex_toggle( char *regex );
int regex_match( char *regex_str, char *str );

/* added: PhrozenSmoke */
GtkWidget* create_ignorelistwindow ();
int perm_igg_check( char *user );
int add_perm_igg( char *user );
void remove_perm_igg(char *user);
void clear_perm_igg_hash( );
void update_perm_igg_clist();


#endif /* #ifndef _IGNORE_H_ */
