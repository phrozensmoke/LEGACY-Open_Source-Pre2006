/*****************************************************************************
 * plugins.h
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
 * Code for handling loading/usage of various plugins, mainly encryption.
 *****************************************************************************/

#include "plugin_api.h"

#ifndef _PLUGINS_H_
#define _PLUGINS_H_

/* for the ymsg Notify packets... */
#define ENCRYPTION_OFF "GYEKRYPTOFF"
#define ENCRYPTION_UNAVAIL "GYEKRYPTUNAVAIL"
#define ENCRYPTION_START_TAG "GYEKRYPTSTRT"
#define ENCRYPTION_START_GPGME "GYEKRYPTGPGME"

extern int enable_encryption;

extern void load_plugin_modules(  );
extern char *get_encryption_description(int encrypt_type);
extern int get_encryption_from_description(char  *edesc);
extern int encryption_type_available(int encrypt_type);
extern int ymsg_encryption_notify( YMSG_SESSION *session, char *who, int which);
extern void acknowledge_encryption(PM_SESSION *pms, int encrypt_type);
extern char *gyache_decrypt_message (char *who, char *msg, int enct);
extern char *gyache_encrypt_message (char *who, char *msg, int enct);
extern void set_current_gpgme_passphrase(int gpgc);
extern int get_current_gpgme_passphrase();
extern int ymsg_encryption_gpgme_notify( YMSG_SESSION *session, char *who, int which);
extern int make_gpgme_passphrase_nmbr();
extern void print_loaded_plugin_info();

extern void  yahoo_process_auth_altplug(char *myusername, char *mypasswd, char *myseed);

void plugins_remove_yphoto_album(char *user);
void plugins_on_yphoto_session_start(PM_SESSION *pms);
void plugins_yphoto_handle_incoming(char *from, char *msg14, int ptype13);
void plugins_cleanup_disconnect();
int plugins_yphotos_loaded();


#endif 


