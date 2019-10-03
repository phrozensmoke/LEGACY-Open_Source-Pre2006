/* Alot of code borrowed from the Ayttm project */

/*****************************************************************************
 * plugin_api.h
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
 * VERY preliminary code for a plugin API, mainly encryptiong plugins
 *****************************************************************************/

/* Currently this module will be used to support a few optional 
    loadable modules either available or in the works:  
    encryption, maybe others 
*/

#include "config.h"

#include "gyach.h"
#include "friends.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"


#ifndef __PLUGIN_API_H__
#define __PLUGIN_API_H__


/* Plugin */

typedef enum {
	PLUGIN_ENCRYPT = 1,  /* encryption */
	PLUGIN_OTHER
} PLUGIN_TYPE;


#define ENC_TYPE_UNKNOWN 0  /* Unknown Encryption */
#define ENC_TYPE_BF_INTERNAL 25  /* Blowfish Internal */
#define ENC_TYPE_MCRYPT_XTEA 26    /* XTea via MCrypt */
#define ENC_TYPE_MCRYPT_BF 27    /* Blowfish via MCrypt */
#define ENC_TYPE_MCRYPT_TF 28    /* Twofish via MCrypt */
#define ENC_TYPE_MCRYPT_TD 29    /* Tripledes via MCrypt */
#define ENC_TYPE_MCRYPT_LOKI97 30    /* LOKI97 via MCrypt */
#define ENC_TYPE_MCRYPT_RC6 31    /* RC6 via MCrypt */
#define ENC_TYPE_MCRYPT_GOST 32    /* Gost via MCrypt */
#define ENC_TYPE_MCRYPT_CAST128 33    /* Cast-128 via MCrypt */
#define ENC_TYPE_MCRYPT_CAST256 34    /* Cast-256 via MCrypt */
#define ENC_TYPE_MCRYPT_RIJNDAEL 35    /* Rijndael-256 via MCrypt */
#define ENC_TYPE_MCRYPT_ARCFOUR 37    /* Arcfour/RC4 via MCrypt */
#define ENC_TYPE_MCRYPT_SERPENT 38    /* Serpent via MCrypt */
#define ENC_TYPE_MCRYPT_SAFERPLUS 39    /* Safer+ via MCrypt */

#define ENC_TYPE_GPGME 41    /* GPGMe, coming soon  */

#define ENC_STR_UNKNOWN "Unknown Encryption Method"  /* Unknown Encryption */
#define ENC_STR_BF_INTERNAL "GyachE Blowfish (Internal)"  /* Blowfish Internal */
#define ENC_STR_MCRYPT_XTEA "Mcrypt Xtea" 
#define ENC_STR_MCRYPT_BF "Mcrypt Blowfish" 
#define ENC_STR_MCRYPT_TF "Mcrypt Twofish" 
#define ENC_STR_MCRYPT_TD "Mcrypt Tripledes" 
#define ENC_STR_MCRYPT_LOKI97 "Mcrypt Loki97" 
#define ENC_STR_MCRYPT_RC6 "Mcrypt RC6" 
#define ENC_STR_MCRYPT_GOST "Mcrypt Gost" 
#define ENC_STR_MCRYPT_CAST128 "Mcrypt Cast-128" 
#define ENC_STR_MCRYPT_CAST256 "Mcrypt Cast-256" 
#define ENC_STR_MCRYPT_RIJNDAEL "Mcrypt Rijndael-256" 
#define ENC_STR_MCRYPT_ARCFOUR "Mcrypt RC4 / ArcFour" 
#define ENC_STR_MCRYPT_SERPENT  "Mcrypt Serpent" 
#define ENC_STR_MCRYPT_SAFERPLUS   "Mcrypt Safer+" 

#define ENC_STR_GPGME "GPG / GPGMe"     /* GPGMe, coming soon  */


typedef int (*gyache_plugin_func)();

typedef struct {
	PLUGIN_TYPE type;
	char *module_name;
	char *description;
	char *version;
	char *date;
	char *sys_req;  /* system requirements */
	char *credits;
	gyache_plugin_func init;
} PLUGIN_INFO;

extern int current_gpgme_passphrase;
extern void set_current_gpgme_passphrase(int gpgc);
extern int get_current_gpgme_passphrase();

#endif
