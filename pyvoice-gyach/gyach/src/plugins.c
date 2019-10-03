/* Alot of code borrowed from the Ayttm project */

/*****************************************************************************
 * plugins.c
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

/* Currently this module will be used to support a few optional 
    loadable modules either available or in the works: webcam, 
    encryption, maybe others 
*/
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <sys/types.h>
#include <dirent.h> /* Routines to read directories to find modules */
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <ltdl.h>

#include "plugin_api.h"
#include "plugins.h"


typedef enum {
	PLUGIN_NOT_LOADED,
	PLUGIN_LOADED,
	PLUGIN_CANNOT_LOAD,
	PLUGIN_NO_STATUS
} PLUGIN_STATUS;

struct gyache_PI {
	PLUGIN_INFO mypinfo;			/* Information provided by the plugin */
	lt_dlhandle Module;		/* Reference to the shared object itself */
	char *path;			/* Full Path */
	char *name;			/* File Name */
	PLUGIN_STATUS status;			/* Is the plugin loaded? */
	 char *status_desc;	/* Error dsecription */
} ;


int enable_encryption=1;

char *enc_armor_unarmor=NULL;
char *enc_armor_strg=NULL;

int current_gpgme_passphrase=0;
int MAX_GPGME_PASSPHRASE=35;

GHashTable *gyache_plugins =NULL; 
int plugin_load_successes=0;
extern char *_(char *some) ;
extern int ALTERNATIVE_LOGIN_PLUGIN;

int load_default_plugin( lt_dlhandle Module, PLUGIN_INFO *info,  char *name );
int load_encrypt_plugin( lt_dlhandle Module, PLUGIN_INFO *info,  char *name );


 PLUGIN_INFO Plugin_Cannot_Load = {PLUGIN_OTHER, 
				  "Unknown",
				  "Unknown", 
				  "Unknown", 
					"Unknown",
					"Unknown",
				  "Unknown", NULL};


/* Convert to and from nice ascii Hex strings Yahoo can send 
    easily without disconnecting us...similar to the ascii armored
    messages availabe in GPG...we want to avoid sending
    'binary' data Yahoo is likely to choke up on */

char *enc_ascii_unarmor(char *somechar) {
	char tmp_int[3];
	unsigned int scanned_r=0;
	int x_counts=0;
	int enc_counts=0;
	if (enc_armor_unarmor) {free(enc_armor_unarmor);}
	enc_armor_unarmor=malloc( (strlen(somechar)/2)+25 );
	if (!enc_armor_unarmor) {return "";}
	tmp_int[2] = '\0';
	while (x_counts < strlen(somechar)) {
			tmp_int[0] = somechar[x_counts];
			tmp_int[1] = somechar[x_counts+1];
			sscanf( tmp_int, "%X", &scanned_r);
			enc_armor_unarmor[enc_counts]=(unsigned int) (scanned_r & 0xFF) ;
			x_counts +=2;
			enc_counts++;
	}
 	enc_armor_unarmor[enc_counts]= '\0';
	return enc_armor_unarmor;
}

char *enc_ascii_armor(char *somechar) {
	char myhexy[4];
	int x_counts=0;
	if (enc_armor_strg) {free(enc_armor_strg);}
	enc_armor_strg=malloc( (strlen(somechar)*2)+25 );
	if (!enc_armor_strg) {return "";}
	sprintf(enc_armor_strg,"%s","");
	while (x_counts < strlen(somechar)) {
			snprintf(myhexy, 3, "%02X",  (unsigned int) (somechar[x_counts]  & 0xFF) );			
			strncat(enc_armor_strg, myhexy,3);
			x_counts ++;
	}
	return enc_armor_strg;
}


void set_current_gpgme_passphrase(int gpgc) {
	if ( (gpgc>-1) && (gpgc<MAX_GPGME_PASSPHRASE) ) {
		current_gpgme_passphrase=gpgc;
	} else {gpgc=0;}
}

int get_current_gpgme_passphrase() {return current_gpgme_passphrase;}

/* Grabs random integers from /dev/urandom or /dev/random */

int grab_random_seed() {
	char readbuf[10];
	struct stat sbuf;	
	int randfd, mybytes;
	int start_rand;
	char *rand_dev="/dev/urandom";

	if ( stat( rand_dev, &sbuf )) {
		rand_dev="/dev/random";
		if ( stat( rand_dev, &sbuf )) { 
			/* Neither device file exists, return default; */
			return RAND_MAX;
		}
	}
	randfd = open( rand_dev, O_RDONLY, 0600 );
	if ( randfd == -1 ) {return RAND_MAX;}
	mybytes = read( randfd, readbuf, 9 );
	close(randfd);
	if (mybytes <9) {return RAND_MAX;}
	start_rand=readbuf[3]*(int) (readbuf[1]+readbuf[4]+readbuf[2]+readbuf[7]+readbuf[5]);	
	if (start_rand<0) {start_rand=(-1) * start_rand;}
	return start_rand;
}

int make_gpgme_passphrase_nmbr() {
	/* Plant the seed */
	srand(grab_random_seed());
	int randj=(int) ( (MAX_GPGME_PASSPHRASE*1.0) *rand()/(RAND_MAX+1.0));
	if (randj<0) {randj=0;}
	if (randj>MAX_GPGME_PASSPHRASE) {randj=MAX_GPGME_PASSPHRASE;}
	return randj;
}

void print_gyache_plugins_hash(gpointer key, gpointer value, gpointer user_data) {
	char infobuf[768]="";
	struct gyache_PI *epi=(struct gyache_PI *)value;
	char *plugname=(char *)key;

	if (!epi) {return;}
	if(epi->status!=PLUGIN_LOADED) {return;}

	snprintf(infobuf, 766, 
	"\n%s    Name: %s%s%s\n    Status: %s\n    Location: %s\n    Description: %s\n    Version: %s\n    Author: %s\n    System Requirements: %s\n    Date: %s", "\033[#8F4CB1m",  YAHOO_COLOR_BLUE, plugname, "\033[#8F4CB1m", epi->status_desc?epi->status_desc:"Unknown", epi->path?epi->path:"Unknown", epi->mypinfo.description?epi->mypinfo.description:"Unknown", epi->mypinfo.version?epi->mypinfo.version:"Unknown", epi->mypinfo.credits?epi->mypinfo.credits:"Unknown", epi->mypinfo.sys_req?epi->mypinfo.sys_req:"Unknown", epi->mypinfo.date?epi->mypinfo.date:"Unknown"
	);
	append_to_textbox_color( chat_window,NULL, infobuf );	

	snprintf(infobuf, 200, "\n____________________\n%s", YAHOO_COLOR_BLACK ); 
	append_to_textbox_color( chat_window,NULL, infobuf );
}

void print_loaded_plugin_info() {
	char plugbuf[96];

	if (!gyache_plugins) {
		snprintf(plugbuf,94,"\n%s ** %s **%s\n", "\033[#73B7DBm",
		_("No plugins loaded."), YAHOO_COLOR_BLACK);
		append_to_textbox_color( chat_window,NULL, plugbuf );
		return;
	}
	if (g_hash_table_size( gyache_plugins )<1) {
		snprintf(plugbuf,94,"\n%s ** %s **%s\n", "\033[#73B7DBm",
		_("No plugins loaded."), YAHOO_COLOR_BLACK);
		append_to_textbox_color( chat_window,NULL, plugbuf );
		return;
	}
		snprintf(plugbuf,94,"\n%s %s :%s\n", "\033[#73B7DBm",
		_("Loaded plugins"), YAHOO_COLOR_BLACK);
		append_to_textbox_color( chat_window,NULL, plugbuf );

		g_hash_table_foreach(gyache_plugins, print_gyache_plugins_hash, NULL );

		snprintf(plugbuf,12,"%s", "\n");
		append_to_textbox_color( chat_window,NULL, plugbuf );
}


void register_plugin(char *plugname, struct gyache_PI *gyache, PLUGIN_STATUS status )
{
	char tmp_plugname[48];
	strncpy( tmp_plugname, plugname, 46 );

	if ( ! gyache_plugins) {
		gyache_plugins = g_hash_table_new( g_str_hash, g_str_equal );
	}

	if ( ! gyache_plugins) {return; }
        if (!plugname) {return;}
        if (!gyache) {return;}

	else {
		char buff[160]="";
		g_hash_table_insert(gyache_plugins, g_strdup(tmp_plugname), gyache);
		snprintf(buff, 158,  " %s%s:  '%s' \n%s",_("Plugin Loaded"), "\033[#8F4CB1m", tmp_plugname,YAHOO_COLOR_BLACK);
		if(status==PLUGIN_LOADED) {
			plugin_load_successes++;
			append_to_textbox_color( chat_window,NULL, buff );
														  }
		}
}

struct gyache_PI *plugin_find( char *plugname)
{
	struct gyache_PI *mypi=NULL;  
	char tmp_plugname[56];
	if (! gyache_plugins) {return NULL; }
        if (!plugname) {return NULL;}
	strncpy( tmp_plugname, plugname, 54 );
	mypi = g_hash_table_lookup(gyache_plugins, tmp_plugname);
	return mypi;
}


/* Will add/update info about a plugin */
 void	SetPluginInfo( PLUGIN_INFO *pi,  char *fpath, lt_dlhandle Module,
	PLUGIN_STATUS status,  char *status_desc )
{
	struct gyache_PI *epi=NULL;

	if (!fpath) {return;}

	if(!pi) {pi=&Plugin_Cannot_Load; pi->module_name=strdup(fpath);}

	epi=plugin_find(pi->module_name);
	if (! epi) {
		epi=g_new0(struct gyache_PI, 1);
		register_plugin(pi->module_name, epi, status);
	}

	else if(epi->status!=PLUGIN_LOADED) {
		free(epi->name);
		free(epi->path);
		free(epi->mypinfo.module_name); 
		free(epi->mypinfo.description);
		free(epi->mypinfo.version);
		free(epi->mypinfo.credits);
		free(epi->mypinfo.date);
		free(epi->mypinfo.sys_req);
	}
	else 	/* A plugin is already succesfully load */
		return;
	epi->status=status;
	epi->path=strdup(fpath);
	epi->status_desc=status_desc;
	epi->mypinfo.type=pi->type;
	epi->mypinfo.module_name=strdup(pi->module_name);
	epi->mypinfo.description=strdup(pi->description);
	epi->mypinfo.version=strdup(pi->version);
	epi->mypinfo.date=strdup(pi->date);
	epi->mypinfo.sys_req=strdup(pi->sys_req);
	epi->mypinfo.credits=strdup(pi->credits);
	epi->mypinfo.init=pi->init;
	epi->name=strdup(pi->module_name);
	epi->Module=Module;
	
	if (status == PLUGIN_CANNOT_LOAD) {
		char buff[768];
		snprintf(buff, 766, "%s** %s '%s' %s:\n%s\n%s: %s\n**%s", 
		YAHOO_COLOR_BLUE, _("Plugin"), 	pi->module_name, _("could not be loaded because of an error"), status_desc, _("Location"),  fpath, YAHOO_COLOR_BLACK);
		append_to_textbox_color( chat_window,NULL, buff );

	}
}




/* Find names which end in .so */
 int select_module_entry( struct dirent *dent)
{
	int len=0;
	char *ext;
	len=strlen(dent->d_name);
	if(len<4)
	   return(0);
	ext=(char *)dent->d_name;
	ext+=(len-3);
	if(!strncmp(ext, ".so", 3)) {   return(1); }
	return(0);
}


int load_module_full_path(  char *inFullPath )
{
	lt_dlhandle		Module;
	PLUGIN_INFO		*plugin_info = NULL;
	struct gyache_PI	*epi = NULL;

	
	if ( inFullPath == NULL) { return (-1); }
	
		/* printf( "Opening module: %s\n", inFullPath ); */
	Module = lt_dlopen( inFullPath );
		/* printf( "Module: %p\n", Module ); */

	/* Find out if this plugin is already loaded */
	if ( Module == NULL )
	{
		char *stuff=strdup(lt_dlerror() );
		/* Only update status on a plugin that is not already loaded */
		SetPluginInfo( NULL, inFullPath, NULL, PLUGIN_CANNOT_LOAD, stuff );
		free(stuff);
		return( -1 );
	}
	
	plugin_info = (PLUGIN_INFO *)lt_dlsym( Module, "plugin_info" );
	if ( !plugin_info )
	{
		lt_dlclose( Module );
		/* Only update status on a plugin that is not already loaded */
		SetPluginInfo( NULL, inFullPath, NULL, PLUGIN_CANNOT_LOAD, _("Cannot resolve symbol \"plugin_info\".") );
		return( -1 );
	}
	

	epi = plugin_find( plugin_info->module_name );
	if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
	{
		lt_dlclose( Module );
		/* printf( "Module already loaded: %s\n", inFullPath ); */
		return( -1 );
	}
	
	switch ( plugin_info->type )
	{
		case PLUGIN_ENCRYPT:
			load_encrypt_plugin( Module, plugin_info, inFullPath );
			break;
			
		default:
			load_default_plugin( Module, plugin_info, inFullPath );
			break;
	}
	
	return( 0 );
}



int	load_module(  char *path,  char *name )
{
	char full_path[256]="";
	if (path == NULL ) {return 0;}
	if (name == NULL) {return 0;}	
	snprintf( full_path, 255, "%s/%s", path, name );	
	return( load_module_full_path( full_path ) );
}


int load_default_plugin( lt_dlhandle Module, PLUGIN_INFO *info,  char *name ) {
	if ((! strcmp(info->module_name,"AltLogin"))  )  {

	/* 	Use an alternative login plugin when Yahoo locks us out, only available
		on my system and not currently distributed because it is probably 
		a violation of the GPL license to do so - not quite legal */

		char loadmsg[255]="";
		int (*yahprocss)(char *myusername, char *mypasswd, char *myseed);
		int (*plugin_init)();

		plugin_init=lt_dlsym(Module, "my_init");
		if (plugin_init) { 	
			if (! plugin_init())  {
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("An error occurred initiating the plugin."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
			}
						} else {
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("No function named \"my_init\" defined in module."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
								}

		yahprocss=lt_dlsym(Module, "yahoo_process_auth_alternate");
		if ( !yahprocss )
		{
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("Cannot resolve symbol \"yahoo_process_auth_alternate\"."), info->sys_req
				);
			lt_dlclose( Module );
			SetPluginInfo( NULL, name, Module, PLUGIN_CANNOT_LOAD,  loadmsg);
			return( -1 );
		}

			SetPluginInfo( info, name, Module, PLUGIN_LOADED, _("Successfully loaded.") );
			ALTERNATIVE_LOGIN_PLUGIN=1;
			return( 1 );
	}


	if ((! strcmp(info->module_name,"GyachE-XMMS"))  || 
		(! strcmp(info->module_name,"GyachE-Photos"))  )  {
		/* 	XMMS status plugin */
		char loadmsg[255]="";
		int (*plugin_init)();

		
		/* printf("plug-1a\n"); fflush(stdout); */

		plugin_init=lt_dlsym(Module, "my_init"); 
			/* printf("ERRORS: [%s] %s\n", info->module_name, lt_dlerror()); fflush(stdout); */
		if (plugin_init != NULL ) { 	
				/* printf("ERRORS: %s\n", lt_dlerror()); fflush(stdout);
				printf("plug-1\n"); fflush(stdout); */
			if (! plugin_init())  {
				/* printf("plug-2\n"); fflush(stdout); */ 

				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("An error occurred initiating the plugin."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
			}
							} else {
				/* printf("plug-3\n"); fflush(stdout); */ 

				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("No function named \"my_init\" defined in module."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
								}

			SetPluginInfo( info, name, Module, PLUGIN_LOADED, _("Successfully loaded.") );
			return( 1 );
	}  /* end xmms plugin */



		lt_dlclose( Module );
		SetPluginInfo(NULL, name, NULL, PLUGIN_CANNOT_LOAD, _("Unknown and unhandled plugin type.") );
		return( 0 );
	/* return 0; */
}


int load_encrypt_plugin( lt_dlhandle Module, PLUGIN_INFO *info,  char *name ) {

	/* printf("load encrypt\n"); fflush(stdout); */ 

	if ((! strcmp(info->module_name,"Blowfish-Internal")) 
		||  (! strcmp(info->module_name,"MCrypt"))
		||  (! strcmp(info->module_name,"Gpgme")) )  {
		
		char loadmsg[255]="";
		char  *(*encrypt_message)(char *inuser, char *inmsg, int encrypt_type);
		char  *(*decrypt_message)(char *inuser, char *inmsg, int encrypt_type);
		int (*plugin_init)();
		int run_encrypt_test=0;

		plugin_init=lt_dlsym(Module, "my_init");
		if (plugin_init) { 	
			if (! plugin_init())  {
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("An error occurred initiating the plugin."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
			}
						} else {
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("No function named \"my_init\" defined in module."), info->sys_req
				);
				lt_dlclose( Module );
				SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
								}

		encrypt_message=lt_dlsym(Module, "encrypt_message");
		decrypt_message=lt_dlsym(Module, "decrypt_message");

		if ( !encrypt_message )
		{
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("Cannot resolve symbol \"encrypt_message\"."), info->sys_req
				);
			lt_dlclose( Module );
			SetPluginInfo( NULL, name, Module, PLUGIN_CANNOT_LOAD,  loadmsg);
			return( -1 );
		}

		if ( !decrypt_message )
		{
				snprintf(loadmsg, 254, "\n%s\nSystem Requirements: %s\n",
					_("Cannot resolve symbol \"decrypt_message\"."), info->sys_req
				);
			lt_dlclose( Module );
			SetPluginInfo( NULL, name, Module, PLUGIN_CANNOT_LOAD, loadmsg );
			return( -1 );
		}

			if (run_encrypt_test) {
			char *test1="This is the Gyach Enhanced Encryption test string. I dont wanna grow up, I'm a toys r us kid.";
			char *test2=NULL;
			test2=enc_ascii_armor(encrypt_message("gPhrozenSmoke", test1, ENC_TYPE_MCRYPT_TF)  );
			printf("Encrypted: %s\n", test2 );
			test1=decrypt_message("gPhrozenSmoke", enc_ascii_unarmor(test2), ENC_TYPE_MCRYPT_TF);
			printf("Decrypted: %s\n", test1 );
			}

			SetPluginInfo( info, name, Module, PLUGIN_LOADED, _("Successfully loaded.") );
			return( 1 );
	}  /* end internal blowfish and mcrypt plugins plugin */


		lt_dlclose( Module );
		SetPluginInfo( NULL, name, NULL, PLUGIN_CANNOT_LOAD, _("Unknown and unhandled encryption plugin type.") );
		return( 0 );
}


/* This is really a modules loader now */
void load_plugin_modules(  )
{
	char gbuff[256]="";
	char *modules_path=NULL;
	int success=0;
	struct dirent *dp;
	DIR *dirp;

	snprintf(gbuff, 192, "%s/plugins", PACKAGE_DATA_DIR);
	modules_path=g_strdup(gbuff);

	lt_dlinit();
	lt_dlsetsearchpath(modules_path);

		if((dirp = opendir(modules_path)) == NULL)
		{
			return ;
		}
		while((dp = readdir(dirp)) != NULL)
		{
			if( dp == NULL )
			{
				continue;
			}
			else if( select_module_entry( dp ) )
			{
				success = load_module(modules_path, dp->d_name);
			}
		}
		if( plugin_load_successes == 0 )
		{
			snprintf( gbuff, 254, "\n%s No plugins were loaded from module path '%s'.\n\n%s", "\033[#73B7DBm", modules_path, YAHOO_COLOR_BLACK );
			append_to_textbox_color( chat_window,NULL, gbuff ); 
		}
		else
		{			
			snprintf(gbuff, 254,  "%s Loaded %d plugins from '%s'.\n\n%s", "\033[#73B7DBm",  plugin_load_successes, modules_path, YAHOO_COLOR_BLACK);
			append_to_textbox_color( chat_window,NULL, gbuff );
			/* print_loaded_plugin_info(); */
		}
		closedir(dirp);

	g_free(modules_path);

}


char *get_encryption_description(int encrypt_type) {

	switch (encrypt_type) {
		case ENC_TYPE_BF_INTERNAL:
			return ENC_STR_BF_INTERNAL;
			break;

		case ENC_TYPE_GPGME:
			return ENC_STR_GPGME;
			break;

		case ENC_TYPE_MCRYPT_XTEA:
			return ENC_STR_MCRYPT_XTEA;
			break;
		case ENC_TYPE_MCRYPT_BF:
			return ENC_STR_MCRYPT_BF;
			break;
		case ENC_TYPE_MCRYPT_TF:
			return ENC_STR_MCRYPT_TF;
			break;
		case ENC_TYPE_MCRYPT_TD:
			return ENC_STR_MCRYPT_TD;
			break;
		case ENC_TYPE_MCRYPT_LOKI97:
			return ENC_STR_MCRYPT_LOKI97;
			break;
		case ENC_TYPE_MCRYPT_RC6:
			return ENC_STR_MCRYPT_RC6;
			break;
		case ENC_TYPE_MCRYPT_CAST128:
			return ENC_STR_MCRYPT_CAST128;
			break;
		case ENC_TYPE_MCRYPT_CAST256:
			return ENC_STR_MCRYPT_CAST256;
			break;
		case ENC_TYPE_MCRYPT_GOST:
			return ENC_STR_MCRYPT_GOST;
			break;
		case ENC_TYPE_MCRYPT_RIJNDAEL:
			return ENC_STR_MCRYPT_RIJNDAEL;
			break;
		case ENC_TYPE_MCRYPT_ARCFOUR:
			return ENC_STR_MCRYPT_ARCFOUR;
			break;
		case ENC_TYPE_MCRYPT_SERPENT:
			return ENC_STR_MCRYPT_SERPENT;
			break;
		case ENC_TYPE_MCRYPT_SAFERPLUS:
			return ENC_STR_MCRYPT_SAFERPLUS;
			break;
		default:
			break;
	}

	return ENC_STR_UNKNOWN;
}

int get_encryption_from_description(char  *edesc) {
  if (!strcasecmp(edesc, ENC_STR_BF_INTERNAL)) {return ENC_TYPE_BF_INTERNAL;}
  if (!strcasecmp(edesc, ENC_STR_GPGME)) {return ENC_TYPE_GPGME;}

  if (!strcasecmp(edesc, ENC_STR_MCRYPT_XTEA)) {return ENC_TYPE_MCRYPT_XTEA;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_BF)) {return ENC_TYPE_MCRYPT_BF;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_TF)) {return ENC_TYPE_MCRYPT_TF;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_TD)) {return ENC_TYPE_MCRYPT_TD;}  
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_LOKI97)) {return ENC_TYPE_MCRYPT_LOKI97;}  
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_RC6)) {return ENC_TYPE_MCRYPT_RC6;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_GOST)) {return ENC_TYPE_MCRYPT_GOST;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_CAST128)) {return ENC_TYPE_MCRYPT_CAST128;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_CAST256)) {return ENC_TYPE_MCRYPT_CAST256;}

  if (!strcasecmp(edesc, ENC_STR_MCRYPT_RIJNDAEL)) {return ENC_TYPE_MCRYPT_RIJNDAEL;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_ARCFOUR)) {return ENC_TYPE_MCRYPT_ARCFOUR;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_SERPENT)) {return ENC_TYPE_MCRYPT_SERPENT;}
  if (!strcasecmp(edesc, ENC_STR_MCRYPT_SAFERPLUS)) {return ENC_TYPE_MCRYPT_SAFERPLUS;}

	return ENC_TYPE_UNKNOWN;
}

int encryption_type_available(int encrypt_type) {
	struct gyache_PI	*epi = NULL;

	switch (encrypt_type) {
		case 0:
		case -1:
			return 0;
			break;

		case ENC_TYPE_BF_INTERNAL:
			epi = plugin_find( "Blowfish-Internal");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED ) { return 1;}
			else {return 0;}
			break;

		case ENC_TYPE_GPGME:  /* Not yet created, so return false for now */
			/*     */
			epi = plugin_find( "Gpgme");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED ) { return 1;}
			else {return 0;} 
			return 0;
			break;

		case ENC_TYPE_MCRYPT_XTEA:
		case ENC_TYPE_MCRYPT_BF:
		case ENC_TYPE_MCRYPT_TF:
		case ENC_TYPE_MCRYPT_TD:
		case ENC_TYPE_MCRYPT_LOKI97:
		case ENC_TYPE_MCRYPT_RC6:
		case ENC_TYPE_MCRYPT_GOST:
		case ENC_TYPE_MCRYPT_CAST128:
		case ENC_TYPE_MCRYPT_CAST256:
		case ENC_TYPE_MCRYPT_RIJNDAEL:
		case ENC_TYPE_MCRYPT_ARCFOUR:
		case ENC_TYPE_MCRYPT_SERPENT:
		case ENC_TYPE_MCRYPT_SAFERPLUS:
			epi = plugin_find( "MCrypt");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED ) { return 1;}
			else {return 0;}
			break;

		default:
			return 0;
			break;
	}		
	return 0;
}

void acknowledge_encryption(PM_SESSION *pms, int encrypt_type) {
	if (encrypt_type==0) { /* turn encryption off */
		pms->encryption_type=0;
		pms->encrypted_myway=0;
		ymsg_encryption_notify( ymsg_sess, pms->pm_user, 0);
		return ;
	}

	if (encrypt_type==-1) { /* turn encryption off, its not available or not wanted */
		pms->encryption_type=0;
		pms->encrypted_myway=0;
		ymsg_encryption_notify( ymsg_sess, pms->pm_user, -1);
		return ;
	}

	if (!enable_encryption) {
		pms->encryption_type=0;
		pms->encrypted_myway=0;
		ymsg_encryption_notify( ymsg_sess, pms->pm_user, 0);
		return ;
	}

	if ( encryption_type_available(encrypt_type)) {
		pms->encryption_type=encrypt_type;
		pms->encrypted_myway=1;
		ymsg_encryption_notify( ymsg_sess, pms->pm_user, encrypt_type);
	} else {
		pms->encryption_type=0;
		pms->encrypted_myway=0;
		ymsg_encryption_notify( ymsg_sess, pms->pm_user, -1);
	}
	
}

char *gyache_encrypt_message (char *who, char *msg, int enct) {

	if (!encryption_type_available(enct)) {return msg;}
	if (enct<1) {return msg;}
	if (!who)  {return msg;}
	if (!msg)  {return msg;}
	if (strlen(msg)<1) {return msg;}

	/* not yet supported, so return */
	if (enct==ENC_TYPE_GPGME)  {
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "Gpgme");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*encrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					encrypt_message=lt_dlsym(epi->Module, "encrypt_message");
					encdstr=encrypt_message(who, msg,enct);
					return encdstr;
				} else {return msg;}

	} 

	if (enct==ENC_TYPE_BF_INTERNAL)  {
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "Blowfish-Internal");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*encrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					encrypt_message=lt_dlsym(epi->Module, "encrypt_message");
					encdstr=encrypt_message(who, msg,enct);
					return enc_ascii_armor(encdstr);
				} else {return msg;}

	} /* end internal blowfish */

	else  {  /* MCrypt */
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "MCrypt");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*encrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					encrypt_message=lt_dlsym(epi->Module, "encrypt_message");
					encdstr=encrypt_message(who, msg,enct);
						
					/* 
					if (capture_fp) {
						fprintf(capture_fp, "Encrypt Msg-MCrypt: %s\n[%s]\n\n", msg, encdstr);
						fprintf(capture_fp, "Encrypted size: %d\n", strlen(encdstr)  );
						fprintf(capture_fp, "Encoded size: %d\n", strlen(enc_ascii_armor(encdstr))  );
						fflush(capture_fp);
					}  */

					return enc_ascii_armor(encdstr);
				} else {return msg;}

	} /* end Mcrypt */

	return msg;
}

char *gyache_decrypt_message (char *who, char *msg, int enct) {

	if (!encryption_type_available(enct)) {return msg;}
	if (enct<1) {return msg;}
	if (!who)  {return msg;}
	if (!msg)  {return msg;}
	if (strlen(msg)<1) {return msg;}

	/* not yet supported, so return */
	if (enct==ENC_TYPE_GPGME)  {
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "Gpgme");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*decrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					decrypt_message=lt_dlsym(epi->Module, "decrypt_message");
					encdstr=decrypt_message(who, msg,enct);
					return encdstr;
				} else {return msg;}

	} 

	if (enct==ENC_TYPE_BF_INTERNAL)  {
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "Blowfish-Internal");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*decrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					decrypt_message=lt_dlsym(epi->Module, "decrypt_message");
					encdstr=decrypt_message(who, enc_ascii_unarmor(msg),enct);
					return encdstr;
				} else {return msg;}

	} /* end internal blowfish */

	else  {  /* MCrypt */
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "MCrypt");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					char  *(*decrypt_message)(char *inuser, char *inmsg, int encrypt_type);
					char *encdstr=NULL;
					decrypt_message=lt_dlsym(epi->Module, "decrypt_message");
					encdstr=decrypt_message(who, enc_ascii_unarmor(msg) ,enct);
					return encdstr;
				} else {return msg;}

	} /* end Mcrypt */

	return msg;
}

void  yahoo_process_auth_altplug(char *myusername, char *mypasswd, char *myseed) {
			struct gyache_PI	*epi = NULL;
			epi = plugin_find( "AltLogin");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					int (*yahprocss)(char *myusername, char *mypasswd, char *myseed);
					yahprocss=lt_dlsym(epi->Module, "yahoo_process_auth_alternate");
					yahprocss(myusername, mypasswd, myseed);
				} 
}


void plugins_cleanup_disconnect() {
	struct gyache_PI	*epi = NULL;

			epi = plugin_find( "GyachE-Photos");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					void  (*yphoto_cleanup)();
					yphoto_cleanup=lt_dlsym(epi->Module, "yphoto_cleanup");
					if (yphoto_cleanup) {yphoto_cleanup();}
				} 
}

void plugins_yphoto_handle_incoming(char *from, char *msg14, int ptype13) {
	struct gyache_PI	*epi = NULL;

			epi = plugin_find( "GyachE-Photos");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					void  (*yphoto_handle_incoming)(char *from, char *msg14, int ptype13);
					yphoto_handle_incoming=lt_dlsym(epi->Module, "yphoto_handle_incoming");
					if (yphoto_handle_incoming) {yphoto_handle_incoming(from, msg14, ptype13);}
				} 
}

void plugins_on_yphoto_session_start(PM_SESSION *pms) {
	struct gyache_PI	*epi = NULL;

			epi = plugin_find( "GyachE-Photos");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					void  (*on_yphoto_session_start)(PM_SESSION *pms);
					on_yphoto_session_start=lt_dlsym(epi->Module, "on_yphoto_session_start");
					if (on_yphoto_session_start) {on_yphoto_session_start(pms);}
				} 
}

void plugins_remove_yphoto_album(char *user) {
	struct gyache_PI	*epi = NULL;

			epi = plugin_find( "GyachE-Photos");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					void  (*remove_yphoto_album)(char *user);
					remove_yphoto_album=lt_dlsym(epi->Module, "remove_yphoto_album");
					if (remove_yphoto_album) {remove_yphoto_album(user);}
				} 
}

int plugins_yphotos_loaded() {
	int retval=0;
	struct gyache_PI	*epi = NULL;

			epi = plugin_find( "GyachE-Photos");
			if ( (epi != NULL) && epi->status == PLUGIN_LOADED )
				{ 
					retval=1;
				}
	return retval;
}




