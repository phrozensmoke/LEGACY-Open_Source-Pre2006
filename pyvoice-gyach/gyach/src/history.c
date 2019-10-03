/*****************************************************************************
 * history.c
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

#include "config.h"

#include <glib.h>
#include <string.h>

#include "gyach.h"
#include "history.h"
#include "main.h"
#include "util.h"

int history_limit = 50;
int history_size = 0;
int history_current = -1;

GList *history_list = NULL;

void history_add( char *str ) {
	GList *this_link;
	static char last[513] = "";

	// DBG( 11, "history_add( '%s' )\n", str );

	if ( ! history_limit ) 
		return;

	history_current = history_size;

	/* don't add the same entry twice in a row */
	if ( ! strcmp( last, str ))
		return;

	strncpy( last, str, 511 );

	history_size++;

	while ( history_size > ( history_limit + 1 )) {
		history_size--;
		this_link = history_list;
		history_list = g_list_remove_link( history_list, this_link );
		gyach_g_list_free( this_link );
		strcpy( history_list->data, "" );
	}

	history_current = history_size;
	history_list = g_list_append( history_list, strdup( str ));
}

char *history_next() {
	GList *this_item;

	// DBG( 11, "history_next()\n" );

	if ( ! history_size )
		return( "" );

	history_current++;

	if ( history_current >= history_size )
		history_current = 0;

	this_item = g_list_nth( history_list, history_current );

	return( this_item->data );
}

char *history_prev() {
	GList *this_item;

	// DBG( 11, "history_prev()\n" );

	if ( ! history_size )
		return( "" );

	history_current--;

	if ( history_current < 0 )
		history_current = history_size - 1;

	this_item = g_list_nth( history_list, history_current );

	return( this_item->data );
}

void history_print() {
	GList *this_item;
	char buf[513];

	// DBG( 11, "history_print()\n" );

	strncpy( buf, "*** Command History ***\n", 50 );
	append_to_textbox( chat_window, NULL, buf );

	this_item = history_list;
	while( this_item ) {
		if ( strcmp( this_item->data, "" )) {
			snprintf( buf, 512, "%s\n", (char *)this_item->data );
			append_to_textbox( chat_window, NULL, buf );
		}
		this_item = g_list_next( this_item );
	}
	strncpy( buf, "*** End History ***\n", 50 );
	append_to_textbox( chat_window, NULL, buf );
}

void history_save() {
	GList *this_item;
	char filename[192];
	FILE *fp;

	snprintf( filename, 190, "%s/history", GYACH_CFG_DIR );

	fp = fopen( filename, "w" );

	if ( fp ) {
		this_item = history_list;
		while( this_item ) {
			if (strlen((char *)this_item->data) > 1) {
				/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
				fprintf( fp, "%s\n", (char *)this_item->data );
			}

			this_item = g_list_next( this_item );
		}
		
		fclose( fp );
	}
}

void history_load() {
	char filename[192];
	char buf[129];
	FILE *fp;

	// DBG( 11, "history_load()\n" );

	snprintf( filename, 190, "%s/history", GYACH_CFG_DIR );

	fp = fopen( filename, "r" );

	if ( fp ) {
		/* clear the list if there already is one */
		history_list = gyach_g_list_free( history_list );
		history_size = 0;
		history_current = -1;

		/* load the new list */
		while( fgets( buf, 128, fp )) {
			if (strlen(buf)>2) {  
				/* 11.27.2005, patch problems with Gyach-E choking on empty strings */
				while( strchr( "\r\n", buf[strlen(buf)-1] )) {
					buf[strlen(buf)-1] = '\0';
				}
				history_add( buf );
			} 
		}
		fclose( fp );
	}
}

