/*****************************************************************************
 * voice_chat.c  - Voice chat module for use with Py! Voice Chat
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
 *****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


FILE *aliasfp;

/*  PhrozenSmoke: some variables needed for voice chat */
char current_chat_serial[50]="";
char current_chat_cookie[120]="";
char current_chat_user[75]="";
char current_chat_name[75]="";
int voice_enabled=0;

static char vchat_data[512];

char *voice_conference_cookie=NULL;

extern GHashTable *screenname_alias;

extern void show_ok_dialog(char *mymsg);  /* added: PhrozenSmoke */
extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 

char *voice_start_file;  /* file holds command to start voice chat */
char *voice_room_file;   /* file holds voice chat info (room members) */

int allow_py_voice_helper=1;

void set_voice_conference_cookie(char *cook) {
	if (voice_conference_cookie) {free(voice_conference_cookie);}
	voice_conference_cookie=strdup(cook);
}


void print_aliases_hash(gpointer key, gpointer value,
        gpointer user_data) {
	fprintf( aliasfp, "%s[pY!]%s\n", (char *)key, (char *)value );
}

void voice_aliases_save() {
	char *user="user";
	char if_filename[72];
	
	if (!allow_py_voice_helper) {return;}

	if (getenv("USER")) {user=getenv("USER");}
	snprintf(if_filename, 70, "/tmp/pyvoice_aliases_%s", user);
	aliasfp = fopen( if_filename, "w" );

	if ( aliasfp ) {
		if (screenname_alias) {
			g_hash_table_foreach(screenname_alias, print_aliases_hash, NULL );
		}
		fclose( aliasfp );
	}
}

int voice_aliases_update(guint data) {voice_aliases_save(); return 1;}
void start_alias_timeout() {gtk_timeout_add( 9000,
				(void *)voice_aliases_update, NULL );}


char *get_voice_start_file() {
	char *user="user";
	char storage[72];
	if (voice_start_file) {return voice_start_file;}
	if (getenv("USER")) {user=getenv("USER");}
	snprintf(storage, 70, "/tmp/pyvoice_chat_start_%s", user);
	voice_start_file=strdup(storage);
	return voice_start_file;
}


char *get_voice_room_file() {
	char *user="user";
	char storage[72];
	if (voice_room_file) {return voice_room_file;}
	if (getenv("USER")) {user=getenv("USER");}
	snprintf(storage, 70, "/tmp/pyvoice_chat_room_%s", user);
	voice_room_file=strdup(storage);
	return voice_room_file;
}



void voice_launch(int with_warning)   /* for regular chat rooms */
{
	char *vdata=NULL;
	FILE *vchat_file;

	if (!allow_py_voice_helper) {return;}

	if (! voice_enabled) {
		if (with_warning)  {
			char *mss=strdup(_("Voice chat is not available in this room."));
			show_ok_dialog(mss);
			free(mss);
									   }
		return;
							    }


	vchat_file = fopen(get_voice_start_file(), "wb");
	memset( vchat_data, 0, sizeof( vchat_data ));
	if (! vchat_file) {
		char *mss=NULL;
		snprintf(vchat_data,500, "%s:\n%s", _("Voice chat: could not open file for writing"), get_voice_start_file() );
		mss=strdup(vchat_data);
		show_ok_dialog(mss);
		free(mss);
		return;
						 }

			snprintf(vchat_data,510, "%s\n%s\n%s\n%s", 
			current_chat_user, current_chat_name, 
			current_chat_cookie, current_chat_serial);
		vdata=strdup(vchat_data);
		fwrite(vdata, 1, strlen(vdata), vchat_file);
		free(vdata);
		fflush(vchat_file);
		fclose(vchat_file);

		if (with_warning)  {
			char *mss=strdup(_("Voice chat has been enabled.  Start pY! Voice Chat."));
			show_ok_dialog(mss);
			free(mss);
									   }

}


void voice_launch_conference(int with_warning, char *yuser, char *yroom)  {
	 /* for private conferences and PMs */
	char *vdata=NULL;
	FILE *vchat_file;
	
	if (!allow_py_voice_helper) {return;}

	if (!yuser) {return;}
	if (!yroom) {return;}
	if (! voice_conference_cookie) {return;}

	vchat_file = fopen(get_voice_start_file(), "wb");
	memset( vchat_data, 0, sizeof( vchat_data ));
	if (! vchat_file) {
		char *mss=NULL;
		snprintf(vchat_data, 500, "%s:\n%s", _("Voice chat: could not open file for writing"), get_voice_start_file() );
		mss=strdup(vchat_data);
		show_ok_dialog(mss);
		free(mss);
		return;
						 }

			snprintf(vchat_data,510, "%s\n%s\n%s\n%s", 
			yuser, yroom, 
			voice_conference_cookie,"[PYCONFERENCE]");
		vdata=strdup(vchat_data);
		fwrite(vdata, 1, strlen(vdata), vchat_file);
		free(vdata);
		fflush(vchat_file);
		fclose(vchat_file);

		if (with_warning)  {
			char *mss=strdup(_("Voice chat has been enabled.  Start pY! Voice Chat."));
			show_ok_dialog(mss);
			free(mss);
									   }

}





