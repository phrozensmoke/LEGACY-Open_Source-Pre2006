/*****************************************************************************
 * roomlist.h
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


#ifndef _ROOMLIST_H_
#define _ROOMLIST_H_

#define YAHOO_ROOM_LIST_URL "http://chat.yahoo.com/c/roomlist.html"

extern GtkWidget * room_window;

extern int  auto_close_roomlist;
extern int  cache_room_list;
extern int  show_adult;

extern char *room_selected;

typedef struct room_entry {
	int top_level;
	char *room_name;
	char *room_id;
	int chatters;
	int populated;
	GtkTreePath *temp_child;
} ROOM_ENTRY;

void populate_child_room_list( char *number, GtkTreeView *room_tree,
    GtkTreePath *parent_node );
void populate_room_list(int tree_type);

#endif /* #ifndef _ROOMLIST_H_ */
