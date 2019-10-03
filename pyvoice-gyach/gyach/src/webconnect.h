/*****************************************************************************
 * webconnect.h
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

#ifndef _WEBCONNECT_H_
#define _WEBCONNECT_H_

#include "gyach.h"
#include "util.h"
#include "yahoochat.h"
#include "packet_handler.h" 

#define YAHOO_STATUS_WEBLOGIN  0x5a55aa55
#define YAHOO_SERVICE_WEBLOGIN  0x0226
#define WEBMESSENGER_URL "http://login.yahoo.com/config/login?.src=pg"

extern int using_web_login;

extern int yahoo_web_login( YMSG_SESSION *session );
extern void yahoo_web_login_finish(YMSG_SESSION *session);
const char *gyach_url_encode(const char *str);
extern char *yahoo_xmldecode(const char *instr);


void set_account_unlocker_image(char *my_image);

#endif



