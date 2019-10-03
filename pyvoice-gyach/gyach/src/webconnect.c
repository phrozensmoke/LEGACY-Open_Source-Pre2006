/*****************************************************************************
 * webconnect.c - alot of this borrowed from Gaim project
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

#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "gyach.h"
#include "util.h"
#include "yahoochat.h"
#include "packet_handler.h" 
#include "md5.h"
#include "webconnect.h"
#include "ycht.h"
#include "profname.h"
#include "callbacks.h"
#include "friends.h"

char *web_cookie=NULL;
char *unlock_secword=NULL;
char *unlock_secdata=NULL;
int using_web_login=0;

void load_ycht_buddy_list( YMSG_SESSION *session);

/* This is a module for logging in using the 'web' login servers like 
    "wcs2.msg.dcn.yahoo.com"
*/

const char *gyach_url_encode(const char *str)
{
	static char bufencode[3072];
	guint i, j = 0;

	g_return_val_if_fail(str != NULL, NULL);

	for (i = 0; i < strlen(str); i++) {
		if (isalnum(str[i]))
			bufencode[j++] = str[i];
		else {
			sprintf(bufencode + j, "%%%02x", (unsigned char)str[i]);
			j += 3;
		}
		if (j>3040) {break;} /* buffer overflow protection */ 
	}

	bufencode[j] = '\0';

	return bufencode;
}


static void yahoo_login_page_hash_iter(const char *key, const char *val, GString *url)
{
	if (!strcmp(key, "passwd"))
		return;
	url = g_string_append_c(url, '&');
	url = g_string_append(url, key);
	url = g_string_append_c(url, '=');
	if (!strcmp(key, ".save") || !strcmp(key, ".js"))
		url = g_string_append_c(url, '1');
	else if (!strcmp(key, ".challenge"))
		url = g_string_append(url, val);
	else
		 url = g_string_append(url, gyach_url_encode(val));
		/* url = g_string_append(url, val); */
}

static GHashTable *yahoo_login_page_hash(const char *buf, size_t len)
{
	GHashTable *hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
	const char *c = buf;
	char *d;
	char name[64], value[64];
	while ((c < (buf + len)) && (c = strstr(c, "<input "))) {
		c = strstr(c, "name=\"") + strlen("name=\"");
		for (d = name; *c!='"'; c++, d++) 
			*d = *c;
		*d = '\0';
		d = strstr(c, "value=\"") + strlen("value=\"");
		if (strchr(c, '>') < d)
			break;
		for (c = d, d = value; *c!='"'; c++, d++)
			*d = *c;
		*d = '\0';
		g_hash_table_insert(hash, g_strdup(name), g_strdup(value));
	}
	return hash;
}


int yahoo_web_login( YMSG_SESSION *session ) {
	int sock = -1;
	struct sockaddr_in sa;
	struct hostent *hinfo;
	// static char webconnresult[49152];  /* was 60001, but I think thats a bit much */
	char *webconnresult=NULL;
	char tmp[512];
	char buf2[1500], *ig = webconnresult, *r = buf2;
	int nr = 0, tr = 0;
	char host[18] = "login.yahoo.com";
	int url_length=0;
	char *sn = strdup(session->user); /* free these ! */
	char *pass = strdup(session->password);
	GHashTable *hash = NULL;
	char *auth=NULL;
	GString *url = g_string_new("GET /config/login?login=");
	// GString *url = g_string_new("GET /config/verify?login=");
	char md5[33], *hashp = md5, *chal;
	int i;
	int o=0;
	md5_byte_t mwebconnresult[16];
	md5_state_t ctx;

	webconnresult=malloc(58367); 
	ig = webconnresult;
	if (! webconnresult)  {
			snprintf(session->error_msg, 124, "%s", _("Not enough memory."));
			free(sn);
			free(pass);
			g_string_free(url, FALSE);
			return (0);
	}


	/* Try a DNS lookup so that this method catches any 
	errors before fetch_url does, this helps avoid 2 dialog 
	boxes with error messages */ 


	snprintf(host,17, "%s", "login.yahoo.com");
	check_proxy_config();

	set_socket_timer(sock, -1);
	set_socket_timer(sock, 10);

	if ( use_proxy ) {
		hinfo = gethostbyname( proxy_host );
	} else {
		hinfo = gethostbyname( host );
	}

	set_socket_timer(sock, -1);

	if ( ! hinfo || ( hinfo->h_addrtype != AF_INET )) {
		if (use_proxy)  { /* added: PhrozenSmoke */
		snprintf(session->error_msg,126,"%s: %s",_("Could not connect to server"),proxy_host );
								} else {
		snprintf(session->error_msg,126,"%s: %s", _("Could not connect to server"), host );
										   }
		free(sn);
		free(pass);
		free(webconnresult);
		g_string_free(url, FALSE);
		clear_fetched_url_data();
		return( 0 );
	}


	set_max_url_fetch_size(57500);
	sprintf(webconnresult,"%s", "");
	/* 
	url_length = fetch_url( "http://login.yahoo.com/config/verify?.src=pg", webconnresult, NULL);
	 */
	url_length = fetch_url( WEBMESSENGER_URL, webconnresult, NULL);


	if (url_length<1) {
			snprintf(session->error_msg, 124, "%s", _("Could not connect to server"));
			free(sn);
			free(pass);
			free(webconnresult);
			g_string_free(url, FALSE);
			clear_fetched_url_data();
			return (0);
						 }

	webconnresult[url_length] = '\0';


	memset( &sa, 0, sizeof(sa));
	memmove((void*)&sa.sin_addr.s_addr, hinfo->h_addr, hinfo->h_length );

	sa.sin_family = AF_INET;

	if ( use_proxy ) {
		sa.sin_port = htons( proxy_port );
	} else {
		sa.sin_port = htons( 80 );
	}

	if (( sock = socket( AF_INET, SOCK_STREAM, 6 )) == -1 ) {

		if (use_proxy)  { /* added: PhrozenSmoke */
		snprintf(session->error_msg,126,"%s: %s",_("Could not connect to server"),proxy_host );
								} else {
		snprintf(session->error_msg,126,"%s: %s", _("Could not connect to server"), host );
										   }

		free(sn);
		free(pass);
		free(webconnresult);
		g_string_free(url, FALSE);
		clear_fetched_url_data();
		return( 0 );
	} else {
		set_socket_timer(sock,8);
		if ( connect( sock, (struct sockaddr*)&sa, sizeof(sa)) == -1 ) {

		if (use_proxy)  { /* added: PhrozenSmoke */
		snprintf(session->error_msg,126,"%s: %s",_("Could not connect to server"),proxy_host );
								} else {
		snprintf(session->error_msg,126,"%s: %s", _("Could not connect to server"), host );
										   }

			set_socket_timer(sock,-1);
			close(sock);
			free(sn);
			free(pass);
			free(webconnresult);
			g_string_free(url, FALSE);
			clear_fetched_url_data();
			return( 0 );
		}
	}

	set_socket_timer(sock,-1);
	hash=yahoo_login_page_hash(webconnresult, url_length);

	url = g_string_append(url, sn);
	url = g_string_append(url, "&passwd=");

	md5_init(&ctx);
	md5_append(&ctx, pass, strlen(pass));
	md5_finish(&ctx, mwebconnresult);
	for (i = 0; i < 16; ++i) {
		g_snprintf(hashp, 3, "%02x", mwebconnresult[i]);
		hashp += 2;
	}
	chal = g_strconcat(md5, g_hash_table_lookup(hash, ".challenge"), NULL);
	md5_init(&ctx);
	md5_append(&ctx, chal, strlen(chal));
	md5_finish(&ctx, mwebconnresult);
	hashp = md5;
	for (i = 0; i < 16; ++i) {
		g_snprintf(hashp, 3, "%02x", mwebconnresult[i]);
		hashp += 2;
	}

	g_free(chal);
	
	url = g_string_append(url, md5);

	if (unlock_secword && unlock_secdata) {
		url = g_string_append(url, "&.secdata=");
		url = g_string_append(url, unlock_secdata);
		url = g_string_append(url, "&.secword=");
		url = g_string_append(url, unlock_secword);
	}
	if (unlock_secdata) {free(unlock_secdata); unlock_secdata=NULL;}
	if (unlock_secword) {free(unlock_secword); unlock_secword=NULL;}

	g_hash_table_foreach(hash, (GHFunc)yahoo_login_page_hash_iter, url);
	
	url = g_string_append(url, "&.hash=1&.md5=1 HTTP/1.1\r\n"
				  "Referer: http://login.yahoo.com\r\n"
				  "Accept-Language: en\r\n");
	url=g_string_append(url, "User-Agent: ");
	url=g_string_append(url, GYACH_USER_AGENT);
	url=g_string_append(url, "\r\n");
	url=g_string_append(url, "Host: login.yahoo.com\r\n\r\n");

	g_hash_table_destroy(hash);

	auth=g_string_free(url, FALSE);

	/* printf("URL  %s\n", auth); fflush(stdout); */

	set_socket_timer(sock,10);
	write( sock, auth,  strlen(auth));

			if ( capture_fp) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp, "\n%s\nWEB-LOGIN DATA SEND:\nHost: login.yahoo.com\nLength: %d\nData: %s\n\n", ctime(&time_llnow), strlen(auth), auth);
				fflush( capture_fp );
			}



	webconnresult[0] = '\0';

	tr = 0;
	set_socket_timer(sock,-1);
	set_socket_timer(sock,35);
	nr = read( sock, tmp, 512 );
	while(( nr > 0 ) &&
		( tr < 57500 )) {
		process_gtk_events();
		tmp[nr] = '\0';
		memcpy( webconnresult + tr, tmp, nr );
		tr += nr;
		nr = read( sock, tmp, 512 );
	}

	close( sock );
	set_socket_timer(sock,-1);
	webconnresult[tr] = '\0';

	/* inspect the HTML data we got back to check for bad logins or locked accounts */
	if ( strstr(ig,"Invalid Password")  || strstr(ig,"invalid password") ||
	    strstr(ig,"verify your password") || strstr(ig,"Try the following hints")
	     )  {
			snprintf(session->error_msg, 124, "%s",  _("INCORRECT PASSWORD") );
			free(sn);
			free(pass);
			free(webconnresult);
			clear_fetched_url_data();
			return (0);
		}

	if ( strstr(ig,"Account Locked")  || strstr(ig,"Account locked") ||
	    strstr(ig,"account locked") || strstr(ig,"is locked")
	     )  {
			snprintf(session->error_msg, 126, "%s", _("YOUR YAHOO! ACCOUNT HAS BEEN LOCKED BECAUSE OF TOO MANY FAILED LOGIN ATTEMPTS") );
			free(sn);
			free(pass);
			free(webconnresult);
			clear_fetched_url_data();
			return (0);
		}


	/* dig for cookie */
	o=0;  /* count chars written to cookie buffer */ 

	while ((ig = strstr(ig, "Set-Cookie: ")) ) {
		ig += strlen("Set-Cookie: "); 
		for (;*ig != ';'; r++, ig++) { 
			if (*ig == '\0') {break;}
			*r = *ig;
			o++;
			if (o>1492) {break;} /* protect against buffer overflow */ 
		}
		*r=';';
		r++;
		*r=' ';
		r++;		
	}
	/* Get rid of that "; " */
	  *(r-2) = '\0';  

	if (web_cookie) {free(web_cookie);}
	web_cookie = strdup(buf2);
	if (strlen(web_cookie)<10) {  /* something went wrong */
		snprintf(session->error_msg, 124, "%s", _("INCORRECT PASSWORD"));
		free(web_cookie);
		web_cookie=NULL;
		free(sn);
		free(pass);
		free(webconnresult);
		clear_fetched_url_data();
		return (0);
									}

	/* printf("Cookie:  %s\n", web_cookie); fflush(stdout);  */

	strncpy( session->cookie, web_cookie, 1022);

	free(sn);
	free(pass);
	free(webconnresult);
	clear_fetched_url_data();
	/* g_string_free(url, FALSE); */  /* causes segfault */
	return( 1);
}


void yahoo_web_login_finish(YMSG_SESSION *session) {
	session->session_id=0;
	clear_offline_messages();
	reset_profile_name_config();
	update_existing_profile_name_widgets();
	if (ycht_only) {
		load_ycht_buddy_list( session);
		ycht_join( );
	} else {	
		ymsg_web_login( session, web_cookie	);
		session->cookie[0]='\0';
	}
}


void set_account_unlocker_image(char *my_image) {
	if (unlock_secdata) {free(unlock_secdata); unlock_secdata=NULL;}
	if ( ! my_image) {return ;}
	if ( strlen(my_image)<1) {return ;}
	unlock_secdata=strdup(my_image);
}

int unlock_yahoo_account(YMSG_SESSION *session, char *mysecret_word) {
	if (! unlock_secdata) {return 0;}
	if (! mysecret_word) {return 0;}
	if ( strlen(mysecret_word)<1) {return 0;}
	if (unlock_secword) {free(unlock_secword); unlock_secword=NULL;}
	unlock_secword=strdup(mysecret_word);
	return yahoo_web_login(ymsg_sess);
}


void load_ycht_buddy_list(YMSG_SESSION *session ) {
	char friends_url[48];
	char *starter=NULL;
	char *stopper=NULL;
	char *webconnresult=NULL;
	char *friends_data=NULL;
	int url_length=0;

	char *finder=":del_friend(unescape('";
	char *find_end="')";

	webconnresult=malloc(81920); 
	if (! webconnresult)  {return; 	}
	set_max_url_fetch_size(81000);
	sprintf(webconnresult,"%s", "");
	snprintf(friends_url, 46, "%s", "http://messenger.yahoo.com/edit/?.src=pg");

	friends_data=malloc(8192);
	if (! friends_data) {free(webconnresult); return; }

	sprintf(friends_data,"%s", "");
	starter=strstr(session->cookie, "Y=");
	if (starter) {
		snprintf(friends_data, 300, "%s", starter);
		stopper=strchr(friends_data,';');
		if (stopper) {*stopper='\0';}
	}

	starter=strstr(session->cookie, "T=");
	if (starter) {
		if (friends_data[0]) {strcat(friends_data, "; ");}
		stopper=strchr(starter,';');
		if (stopper) {*stopper='\0';}
		strncat(friends_data, starter, 300);
		if (stopper) {*stopper=';';}
	}


	url_length = fetch_url( friends_url, webconnresult, friends_data);

	if (url_length<1) {free(friends_data); free(webconnresult); return; }
	if (! strstr(webconnresult,finder)) {free(friends_data); free(webconnresult); return; }

	snprintf(friends_data, 18, "%s", "Buddies:");
	starter=strstr(webconnresult,finder);
	while (starter) {
			starter += strlen(finder);
			stopper=strstr(starter,find_end);
			if (stopper) {
				*stopper='\0';
				if (strlen(starter)>1) {
					if (strcmp(friends_data,"")) {strcat(friends_data, ",");}
					strncat(friends_data,starter, 64);
					if (! strcasecmp(session->user,starter)) {add_online_friend(starter);}
				}
				stopper++;
				starter=stopper;
				starter=strstr(stopper,finder);
			} else {starter=NULL;}
			if (strlen(friends_data)>8100) {break;}			
	}
	free(webconnresult); 
	if (strlen(friends_data)>10) {
		populate_friend_list(friends_data);
		update_buddy_clist();
		show_friends();
	}
	free(friends_data); 
}


