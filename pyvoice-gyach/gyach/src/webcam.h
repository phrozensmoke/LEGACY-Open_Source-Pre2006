/* Mainly a wrapper for launching the external web cam 
    viewer gyache-webcam, but handles the necessary 
    Ymsg negotiations inside Gyach */

/*****************************************************************************
 * webcam.h
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
 * VERY preliminary code for handling webcams - much of this code is 
 * borrowed from the Ayttm/libyahoo2 projects
 *****************************************************************************/
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#include "gyach.h"
#include "yahoochat.h"
#include "util.h"

#define HAVE_WEBCAM 1

extern int ymsg_get_webcam( YMSG_SESSION *session, char *who);
extern int ymsg_webcam_invite( YMSG_SESSION *session, char *who);
extern int ymsg_webcam_invite_accept( YMSG_SESSION *session, char *who);
extern int ymsg_webcam_invite_reject( YMSG_SESSION *session, char *who);
extern void yahoo_webcam_invite_msg(char *who);

extern void yahoo_webcam_get_feed(char *who);
extern void yahoo_process_webcam_key(char *key);


