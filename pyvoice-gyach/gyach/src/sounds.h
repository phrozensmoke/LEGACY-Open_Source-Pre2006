/*****************************************************************************
 * sounds.h
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
 * Erica Andrews, PhrozenSmoke ['at'] yahoo.com
 * added 8.31.2003, support for playing sound events on the ESound daemon
 *****************************************************************************/

#ifndef _SOUNDS_H_
#define _SOUNDS_H_


#define SOUND_EVENT_BUDDY_ON 0
#define  SOUND_EVENT_BUDDY_OFF 1
#define  SOUND_EVENT_MAIL 2
#define  SOUND_EVENT_PM 3  /* PM, instant message */
#define  SOUND_EVENT_BUZZ 4  /* we've been buzzed */
#define  SOUND_EVENT_OTHER 5  /* any other sound event, invites, file transfers, important notices, etc. */
#define SOUND_EVENT_REJECT 6  /* any time we 'reject' something, invite, webcam, pm, etc. */

typedef struct gy_audible {
	char *aud_file;
	char *aud_text;
	char *aud_hash;
	char *aud_disk_name;
} GYAUDIBLE;

GYAUDIBLE *gyache_audibles;

extern char *mp3_player;
char *play_audible(char *aud);

char *get_gy_audible_text( char *str );
char *get_gy_audible_hash( char *str );
char *get_gy_audible_disk_name( char *str );
int check_gy_audible( char *str );
void init_audibles();

extern void play_sound_event(int sound_value);


#endif




