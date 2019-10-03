/*****************************************************************************
 * packet_handler.h
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
 *****************************************************************************/

#ifndef _PACKAGEHANDLER_H_
#define _PACKAGEHANDLER_H_

/* added PhrozenSmoke: some variables needed for voice chat */
extern char current_chat_serial[50];
extern char current_chat_cookie[120];
extern char current_chat_user[75];
extern char current_chat_name[75];

extern int in_a_chat;
extern int real_show_colors;
extern int enable_pm_searches;
extern char *yalias_name;  /* for Yahoo 'nick name' handling */

void handle_logoff();
void check_connection_timedout();


/* added PhrozenSMoke:  from users.c */
extern void parse_screenname_aliases( char *packet, int clear_it );

extern void start_alias_timeout();    /* from voice_chat.c */

extern void handle_yahoo_packet();
extern void clear_offline_messages();
extern void send_automated_response(char *inuser, char *inalias, char *myinmsg);
extern void clear_chat_list_members();
extern int get_pm_perms(char *who);

#endif /* #ifndef _PACKAGEHANDLER_H_ */






