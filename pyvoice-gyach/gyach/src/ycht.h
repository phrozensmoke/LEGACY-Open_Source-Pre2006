/*****************************************************************************
 * ycht.h
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
 * Phrozensmoke ['at'] yahoo.com
 *****************************************************************************/

#ifndef _YCHT_H_
#define _YCHT_H_

int ycht_sock;

int ycht_only;

int ycht_is_running();
int ycht_recv_data( );
int ycht_ping( );
int ycht_join( );
int ycht_goto(char *who );
int ycht_online( );
int ycht_logout(int quiet);
int ycht_comment(  char *comment, int comtype );
int ycht_pm( char *remote_user, char *msg);
void handle_ycht_packet();
void ycht_set_host(char *myhost);

#endif

