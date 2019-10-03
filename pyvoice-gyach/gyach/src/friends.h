/*****************************************************************************
 * friends.h
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


#ifndef _FRIENDS_H_
#define _FRIENDS_H_


struct yahoo_friend {
	int idle;
	int away;
	int inchat;
	int insms;
	int ingames;
	int stealth;  /* 0=default, 1=online, 2=offline, 3=perm offline */
	int launchcast;
	int webcam;
	int mobile_list;
	char *main_stat;
	char *game_stat;
	char *game_url;
	char *radio_stat; /* launchcast status */
	char *idle_stat;
	char *buddy_group;
	char *avatar;
	int identity_id;
	char *buddy_image_hash;
};

extern GList *friend_list;
extern GList *ofriend_list;

void populate_friend_list( char *friends );
void *fetch_friend_list( void *arg );
int remove_friend( char *friend );
int add_friend( char *friend );
int find_friend( char *friend );
int remove_online_friend( char *friend );
int add_online_friend( char *friend );
int find_online_friend( char *friend );
int show_friends( void );
int build_online_friends_list( void );
int find_temporary_friend( char *friend );
int remove_temporary_friend( char *friend );
int add_temporary_friend( char *friend );
int set_buddy_status( char *user, char *userstatus );
void parse_initial_statuses( char *packet );
int set_buddy_status_full( char *user, char *userstatus, int allow_invisible );
void remove_all_online_friends( ) ;
void remove_buddy_status(char *user) ;
void  reset_all_online_friends_statuses( );
int friend_is_invisible(char *user);
void reset_all_online_friends_statuses();
char *get_launchast_genre(char *incoming);
char *get_buddy_identities_list(char *who);
void rename_buddy_group(char *ogroup, char *ngroup);
void append_new_friend_to_list(char *friend_to_add);
void remove_old_friend_from_list(char *friend_to_rem);
struct yahoo_friend *create_or_find_yahoo_friend(char *bud);
struct yahoo_friend *yahoo_friend_find(char *bud);

#endif /* #ifndef _FRIENDS_H_ */
