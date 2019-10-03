/*****************************************************************************
 * util.c
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
 * Copyright (C) 2000-2002 Chris Pinkham
 * cpinkham@corp.infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 * 
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 *****************************************************************************/

#include "config.h"

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#ifdef USE_GDK_PIXBUF
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>
#endif

#include "gyach.h"
#include "commands.h"
#include "gyach_int.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"

SMILEY *smileys;
char mail_user[65] = "";
char *recv_sound = NULL;
char *send_sound = NULL;
int  profile_viewer = 1;
int support_scentral_smileys=1;

int show_yavatars=1;
int enable_audibles=1;
int activate_profnames=1;
int show_bimages=1;
int pm_window_style=1;
int pmb_toolbar1=1;
int pmb_toolbar2=1;
int auto_reject_invitations=0;
int pmb_audibles=1;
int pmb_smileys=1;
int pmb_send_erase=1;
char *flash_player_cmd=NULL;
char *bimage_file=NULL;
char *bimage_url=NULL;
int bimage_timestamp=0;
char *bimage_hash=NULL;
int bimage_friend_only=0;
int bimage_share=0;
int share_blist_avatar=1;
int bimage_size=1;

int chatwin_smileys=1;
int chatwin_send_erase=1;
int show_tabs_at_bottom=0;

/* Do buddy list avatars use 'png' or 'jpg' format from yahoo's servers */
/* as of this writing, Yahoo supports 3 filetypes: swf, png, jpg ... 
	now using jpg instead of png because of smaller size, sometimes 
	a difference of 40-45kb pngs to 10kb for jpg */			
/* can also use: 
	http://img.avatars.yahoo.com/users/%s.medium.png */ 

char *avatar_filetype="jpg";


/* added: PhrozenSmoke from fader.c */
extern char *get_formatted_color(char *some_color);
extern char  *parse_fader_color_code(char *fade_color);
extern char *do_fader_text(char *from_col, char *to_col, const char *s);
extern char *do_alt_text(char *from_col, char *to_col, const char *s);
extern void check_proxy_config();
extern void show_ok_dialog(char *mymsg);  /* added: PhrozenSmoke */
extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 
extern void set_socket_timer(int sockfd, int mytime);

extern int enable_animations;

int max_url_fetch_size=2048;
char *webresult=NULL;
char *url_post_data=NULL;

/* added PhrozenSmoke */

char *smiley_tags[90]={};
char *smiley_files[90]={};

int have_smiley_file(char *str) {
	int i=0;
	while (1)  {
		if (i>87) {return 0;}
		if (smiley_files[i]==NULL) {return 0;}
		if (strcasecmp(smiley_files[i],str)==0) {return 1;}
		i++;
				 }
	return 0;
}

void map_smileys() {
	SMILEY *sm_ptr;
	int i=0;
	sm_ptr = &smileys[0];
	while( sm_ptr->sm_text ) {
		if (i>87) {break;}		
		if (!  have_smiley_file(sm_ptr->sm_file)) {
			smiley_files[i]=sm_ptr->sm_file;
			smiley_tags[i]=sm_ptr->sm_text;
			i++;
																		  }
		sm_ptr++;
		
	}
	smiley_files[i]=NULL;
	smiley_tags[i]=NULL;
}

GtkWidget *load_smiley_icon(char *str) {
	int is_audible=0;
	GdkPixbufAnimation *anim = NULL;
	char filename[192]="";

	if (! strncmp(str,"base.us", 7)) { is_audible=1;}

					snprintf(filename,190,"%s/smileys/%s.gif", PACKAGE_DATA_DIR, str);
					if (is_audible) {
						/* its an audible */
						snprintf(filename,190,"%s/audibles/%s.gif", PACKAGE_DATA_DIR, str);
					}
					/* printf("file:  %s", filename);
					fflush(stdout);  */
					anim = gdk_pixbuf_animation_new_from_file(filename,NULL);
					if (anim)  {
						GtkWidget *myimage=NULL;
						if (gdk_pixbuf_animation_is_static_image(anim) || 
							(!enable_animations) || (is_audible) )   {
								GdkPixbuf *buf = NULL;
								buf=gdk_pixbuf_animation_get_static_image(anim);
								if (is_audible) {
									GdkPixbuf *spixbuf=NULL;
									int awidth=0;
									int aheight=0;	
									awidth = gdk_pixbuf_get_width( buf );
									aheight = gdk_pixbuf_get_height( buf );
									aheight = ( 1.0 *  aheight / awidth ) * 40;
									awidth = 40;
									spixbuf = gdk_pixbuf_scale_simple( buf, awidth, aheight,
					GDK_INTERP_BILINEAR );
									if (spixbuf) {
										myimage=gtk_image_new_from_pixbuf(spixbuf);
										g_object_unref( spixbuf );
									} else {myimage=gtk_image_new_from_pixbuf(buf);}
								} else {
								myimage=gtk_image_new_from_pixbuf(buf);
										  }
								gtk_widget_show_all(myimage);
								//g_object_unref( buf );  /* creates seg-faults, dont re-enable */

															  } else {
						myimage=gtk_image_new_from_animation(anim);
						gtk_widget_show_all(myimage);
																		}

						gdk_pixbuf_animation_unref(anim);
						return myimage;
								    }
return NULL;
}


void strip_html_tags( char *str ) {
	char *to, *from;

	// DBG( 21, "strip_html_tags()\n" );

	to = str;
	from = str;

	while( *from ) {
		if ( *from == '<' ) {
			from++;
	
			if ( (!strncasecmp(from,"tr",2)) || 
				(!strncasecmp(from,"table",5)) ) {
				/* add newline char */ 
				*to = '\n';
				to++;
			}

			while(( *from ) &&
				  ( *from != '>' ) &&
				  ( *from != '<' )) {
				from++;
			}
			if ( *from )
				from++;
		} else {
			*to = *from;
			to++;
			from++;
		}
	}
	*to = *from;

	to = str;
	from = str;
	while( *from ) {
		if ( *from == '\r' ) {
			from++;
		} else if ( *from == '\t' ) { /* tabs */
			from++;
			*to = ' ';
			to++;
		} else if ( *from == '&' ) {

 			if (( *(from+1) == '#' ) &&
					   ( *(from+2) == '1' ) &&
					   ( *(from+3) == '8' ) &&
					   ( *(from+4) == '3' ) &&
					   ( *(from+5) == ';' )) {
				from += 5;
				*to = '\n';
				to++;
			}  	else if ( *(from+1) == '#' ) {
				while( ( *from ) && (*from != ';') ) {
					from++;
				}
			} else if (( *(from+1) == 'n' ) &&
					   ( *(from+2) == 'b' ) &&
					   ( *(from+3) == 's' ) &&
					   ( *(from+4) == 'p' ) &&
					   ( *(from+5) == ';' )) {
				from += 5;
				*to = ' ';
				to++;
			}
			from++;
		} else {
			*to = *from;
			to++;
			from++;
		}
	}
	*to = *from;

	to = str;
	from = str;
	while( *from ) {
		if (( *from == '\n' ) &&
			( *(from+1) == '\n' )) {
		} else {
			*to = *from;
			to++;
		}
		from++;
	}
	*to = *from;

	/* now strip "\033[30m" style entries */
	to = str;
	from = str;
	while( *from ) {
		if ( *from == '\033' ) {
			while(( *from ) &&
				( *from != 'm' )) {
				from++;
			}
		} else {
			*to = *from;
			to++;
		}
		if ( *from )
			from++;
	}
	*to = *from;
}

int my_system( char *command ) {
	int pid;
	int status;

	if ( ! command ) {
		return( 1 );
	}

	pid = fork();
	if ( pid == -1 ) {
		return( -1 );
	} else if ( pid == 0 ) {
		char *argv[4];
		
		argv[0] = "sh";
		argv[1] = "-c";
		argv[2] = command;
		argv[3] = 0;

			if ( capture_fp ) {	
				time_t time_llnow = time(NULL);
				fprintf(capture_fp,"\n\n[%s]  EXTERNAL PROCESS LAUNCHED in 'my_system',  Command-Line: sh -c %s\n\n", ctime(&time_llnow), command);
				fflush( capture_fp );
				}

		execv( "/bin/sh", argv );
		exit( 127 );
	} else {
		while( 1 ) {
			if ( waitpid( pid, &status, 0 ) == -1 ) {
				return( -1 );
			} else {
				return( status );
			}
		}
	}
}

/* NOTE: this function frees *arg when done, so use strdup() before calling */
void *display_url( void *arg ) {
	char *url = (char *)arg;
	char cmd[768];

	if ( ! url ) { return( NULL ); }

	if ( strlen( url ) == 0 ) {
		free( url );
		return( NULL );
	}

	/* quick sanity check */
	if (( strchr( url, ';' )) ||
		( strchr( url, '`' ))) {
		free( url );
		return( NULL );
	}

	snprintf( cmd, 766, browser_command, url );
	my_system( cmd );
	free( url );
	return( NULL );
}


void lower_str( char *str ) {
	char tmp[1280];
	char *dst = str;
	char *src = tmp;

	strncpy( tmp, str, 1278 );
	while( *src ) {		*(dst++) = tolower( *(src++) ); 	}
}


/* this list is prioritized a little, since a few smileys are the same */
/* as others except for one extra char at the end, 
    files are format 89.gif, 33.gif, etc. */

SMILEY default_smileys[] = {
	{ ":(|)", "51" },
	{ "(:|", "37" },
	{ ":)>-", "67" },
	{ "\\:d/", "69" }, { "\\:D/", "69" },	
	{ ":))", "21" },	{ ":-))", "21" },
	{ ":)", "01" },	{ "(:", "01" },	{ "(-:", "01" },	{ ":-)", "01" },
	{ ":((", "20" },	{ ":-((", "20" },
	{ ":(", "02" },	{ ":-(", "02" },
	{ ";))", "71" },
	{ ";;)", "05" },	{ ";;-)", "05" },
	{ ";)", "03" },	{ ";-)", "03" },
	{ ":D", "04" },	{ ":-D", "04" },
	{ ":-/", "07" },	{ ":-\\", "07" },
	{ ":x", "08" },	{ ":-x", "08" },	{ ":X", "08" },	{ ":-X", "08" },
	{ ":\">", "09" },
	{ ">:p", "47" }, 	{ ">:P", "47" },
	{ ":p", "10" },	{ ":-p", "10" },	{ ":P", "10" },	{ ":-P", "10" },
	{ ":-*", "11" }, { "=*", "11" },	{ ":*", "11" },	
	{ "=((", "12" },
	{ ":o)", "34" },	{ ":O)", "34" },	{ ":0)", "34" },
	{ ":-o", "13" }, { ":O", "13" },	{ ":-O", "13" },  { ":0", "13" },	{ ":-0", "13" },  
	{ "X(", "14" },	{ "X-(", "14" },
	{ ":>", "15" },	{ ":->", "15" },
	 { "b-)", "16" }, { "B-)", "16" },
	{ "#:-s", "18" }, 	{ "#:-S", "18" },
	{ ":-ss", "42" }, 	{ ":-SS", "42" },
	{ ":-s", "17" }, 	{ ":-S", "17" },
	{ ">:)", "19" },
	{ ":|", "22" },	{ ":-|", "22" },
	{ "/:)", "23" }, { "/:-)", "23" },	
	{ "=))", "24" },
	{ "o:-)", "25" }, { "0:-)", "25" },	{ "O:-)", "25" }, { "o:)", "25" }, { "0:)", "25" }, { "8-)", "25" }, { "8)", "25" },
	{ ":-B", "26" }, { ":-b", "26" },
	{ "=;", "27" },
	{ "i-)", "28" }, { "I-)", "28" },	{ "|-)", "28" }, 
	{ "8-|", "29" },
	{ "l-)", "30" },
	{ ":-&", "31" },
	{ ":-$", "32" },
	{ "[-(", "33" },
	{ "8-}", "35" },

	{ "<:-p", "36" },

	{ "=p~", "38" },
	{ ":-?", "39" },
	{ "#-o", "40" },
	{ "=D>", "41" },
	
	{ "@-)", "43" },
	{ ":^o", "44" }, { ":^0", "44" },  { ":^O", "44" },
	{ ":-w", "45" }, 	{ ":-W", "45" },
	{ ":-<", "46" },

	{ ":@)", "49" },
	{ "3:-O", "50" }, { "3:-0", "50" },	{ "3:-o", "50" },  
	{ "~:>", "52" },
	{ "@};-", "53" },
	{ "%%-", "54" },
	{ "**==", "55" },
	{ "(~~)", "56" },
	{ "~o)", "57" },
	{ "*-:)", "58" },
	{ "8-X", "59" }, { "8-x", "59" },
	{ "=:)", "60" },	{ "=:-)", "60" },
	{ ">-)", "61" },
	{ ":-l", "62" }, { ":-L", "62" },
	{ "<):)", "48" },
	{ "[-o<", "63" },
	{ "$-)", "64" },
	{ ":-\"", "65" },
	{ "b-(", "66" },	{ "B-(", "66" },
	{ "[-x", "68" }, { "[-X", "68" },  
	{ ">:D<", "06" }, 	{ ">:d<", "06" },
	{ ">:/", "70" },

	{ "o->", "72" },
	{ "o=>", "73" },
	{ "o-+", "74" },
	{ "(%)", "75" },

	{ ":-@", "76" },
	{ "^:)^", "77" },
	{ ":-j", "78" }, { ":-J", "78" },
	{ "(*)", "79" },
	{ 0, 0 }
};



SMILEY read_smileys[256];

void smileys_load() {
	SMILEY *sm_ptr;
	char filename[192];
	FILE *fp;
	struct stat sbuf;
	char buf[201];
	char *sm_text;
	char *sm_file;
	char *end;
	int override_smile=1;

	snprintf( filename, 190, "%s/.yahoorc/gyach/emoticons", getenv( "HOME" ));

	if ( stat( filename, &sbuf )) {
		/* doesn't exist so write and use default */
		fp = fopen( filename, "w" );
		if ( fp ) {
			fprintf( fp,
				"# format of file is emoticon_text<tab>emoticon_file\n# Place this file at ~/.yahoorc/gyach/emoticons\n# You probably shouldnt change the order of these too much.\n# NOTE: This file is no longer being read by Gyach-E - built-in emoticon configurations are being  used.  This file remains for historic purposes only" );
			sm_ptr = default_smileys;
			while( sm_ptr->sm_file ) {
				fprintf( fp, "%s	%s\n", sm_ptr->sm_text, sm_ptr->sm_file );
				sm_ptr++;
			}
			fclose( fp );
		}
		smileys = default_smileys;
	} else {
		/* changed here: PhrozenSmoke - do not use emoticon config file anymore
		it will screw up support for Yahoo's new smileys...the builtin configuration 
		will suit most folks just fine */

		if (override_smile) {
			smileys = default_smileys;
			return;
		}


		/* exists, so open and read */
		smileys = read_smileys;
		sm_ptr = smileys;
		fp = fopen( filename, "r" );
		if ( fp ) {
			while( fgets( buf, 200, fp )) {
				/* chop the newline */
				buf[strlen(buf)-1] = '\0';
				sm_text = skip_whitespace( buf );
				
				if ( *sm_text == '#' ) {
					continue;
				}

				end = find_whitespace( sm_text );
				if ( ! *end ) {
					/* invalid line, no filename given */
					continue;
				}
				*end = '\0';

				sm_file = skip_whitespace( end + 1 );
				end = find_whitespace( sm_file );
				*end = '\0';

				sm_ptr->sm_text = strdup( sm_text );
				sm_ptr->sm_file = strdup( sm_file );
				sm_ptr++;
			}
			fclose( fp );
			sm_ptr->sm_text = NULL;
			sm_ptr->sm_file = NULL;
		}
	}
}

int check_smiley( char *str ) {
	SMILEY *sm_ptr;

	sm_ptr = &smileys[0];
	while( sm_ptr->sm_text ) {
		if (( !strncmp( str, sm_ptr->sm_text, strlen(sm_ptr->sm_text))) &&
			(( strchr( " \n", str[strlen(sm_ptr->sm_text)] )) ||
			 ( ! str[strlen(sm_ptr->sm_text)] ))) {
			return( 1 );
		}
		sm_ptr++;
	}

	return( 0 );
}


void convert_smileys( char *str ) {
	char tmp[4096];
	char sm_code[66];
	char *from;
	char *to;
	int found;
	SMILEY *sm_ptr;
	char last_char=' ';
	int open_html_tag=0;

	// printf("STUFF:  %s\n",str); fflush(stdout);

	/* return if too long */
	if ( strlen( str ) > 3250 )
		return;

	strncpy( tmp, str, 3250 );

	from = tmp;
	to = str;
	*to = '\0';

	while( *from ) {
		found = 0;

		/* support SmileyCentral retrievable smileys, on the web */
		/* Parse SmileyCentral.com smiley codes that look something like this:
				@[smiley: Hi Ya : [15/15_1_70]]
			into a downloadable URL for an animated .gif file like this:
				http://smileys.smileycentral.com/cat/15/15_1_70.gif

			We can see the SmileyCentral.com smileys (instead of the ugly code)
			WITHOUT all the spyware that comes with the SmileyCentral.com
			software for Windoze!  The animated .gifs (there are THOUSANDS) 
			on that site, are being more commonly used in the chat rooms.
		*/
		
			if ( !strncasecmp( from, "@[smiley", 8)) {		
				if (strstr(from,"]"))  {
						char *smender=NULL;
						from +=7;
						while (*from)    {
								if (*from=='[') {
										from++;
										break;
														}
								from++;
												}
						smender=strstr(from,"]");
						if (*from  && smender)  {
							*smender='\0';

							if (support_scentral_smileys)  
							{
								snprintf( sm_code, 42, " %s", 
									"\033|scnt://smileys.smileycentral.com/cat/" );
								/* snprintf( sm_code, 42, "\033|%s", "scnt://localhost/ism/" ); */
								strncat(sm_code,from,16);
								strcat(sm_code,".gif ");
								strcat( to, sm_code );
							} else { /* user doesn't want SmileyCentral emoticons */
								snprintf( sm_code, 30, " [%s] ", "SC-Smiley" );
								strcat( to, sm_code );
									  } 
							to += strlen( sm_code );
							found=1;
							smender += 1;
							if (*smender==']') {smender += 1;}
							from=smender;
																	}
				}
			}

		sm_ptr = &smileys[0];
		while(( sm_ptr->sm_text ) &&
			( ! found )) {

		/* if ( ! ( (strstr( str, "has entered") != NULL || strstr( str, "has left") != NULL) && (strstr( str, "the room") != NULL )  ) )  { */


				/* This is a bug fix below: PhrozenSmoke */ 
		if ( ! open_html_tag)  {
			
			if ( !strncasecmp( from, sm_ptr->sm_text, strlen(sm_ptr->sm_text))) {		


			if (( !strncmp( ":0", sm_ptr->sm_text, 2)) && isdigit(last_char)) 
				 { found=0;}  else {

				found = 1;
				snprintf( sm_code, 9, "\033|%s ", sm_ptr->sm_file );
				strcat( to, sm_code );
				to += strlen( sm_code );
				from += strlen( sm_ptr->sm_text );
							   }
															} 
						} /* end if */
			sm_ptr++;
		}
		if ( ! found ) {
			*to = *from;
			last_char=from[0];
			if (last_char=='>') {open_html_tag=0;}
			if (last_char=='<') {open_html_tag=1;}
			to++;
			from++;
			*to = '\0';
		}
	}
}

char *get_last_color( char *str ) {
	char *ptr;

	ptr = str + strlen( str );
	while(( ptr > str ) &&
		( *ptr != '|' )) {
		ptr--;
	}

	if ( ptr == str ) {
		return( ptr );
	}

	if ( *ptr == '|' ) {
		*ptr = '\0';
		ptr++;
		return( ptr );
	}

	/* should never get here */
	return( YAHOO_COLOR_BLACK );
}




/*
 * attempt to highlight urls if they aren't already highlighted along with
 * turning html style tags into esc sequences.
 */
void convert_tags( char *str ) {
	char tmp[4096];
	char *from;
	char *to;
	char *ptr;
	char *ptr2;
	char *ptr3;
	int len = 0;
	char tmp2[512];
	char colors[2048] = YAHOO_COLOR_BLACK;
	char prev_color[25] = YAHOO_COLOR_BLACK;
	char *cptr;

	/* PhrozenSmoke test code, ignore this */

	 /*printf("\nTAG:  %s\n",str);
	     fflush(stdout);  */


	// DBG( 21, "convert_tags( '%s' )\n", str );

	/* return if too long */
	if ( strlen( str ) > 3200 )
		return;

	/* convert html tags to ESC sequences */
	if ( strchr( str, '<' )) {
		memset( tmp, 0, sizeof( tmp ));

		from = str;
		to = tmp;

		ptr = strchr( from, '<' );
		while( ptr ) {
			len = ptr - from;
			strncpy( to, from, len );
			to += len;
			*to = '\0';

			from += len;


			if ( !strncasecmp( from, "<black>", 7 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_BLACK );
					to += strlen( YAHOO_COLOR_BLACK );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_BLACK, 23 );
				}
				from += 7;
			} else if ( !strncasecmp( from, "<blue>", 6 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_BLUE );
					to += strlen( YAHOO_COLOR_BLUE );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_BLUE, 23 );
				}
				from += 6;
			} else if ( !strncasecmp( from, "<cyan>", 6 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_CYAN );
					to += strlen( YAHOO_COLOR_CYAN );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_CYAN, 23 );
				}
				from += 6;
			} else if ( !strncasecmp( from, "<gray>", 6 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_GRAY );
					to += strlen( YAHOO_COLOR_GRAY );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_GRAY, 23 );
				}
				from += 6;
			} else if ( !strncasecmp( from, "<green>", 7 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_GREEN );
					to += strlen( YAHOO_COLOR_GREEN );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_GREEN, 23 );
				}
				from += 7;
			} else if ( !strncasecmp( from, "<pink>", 6 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_PINK );
					to += strlen( YAHOO_COLOR_PINK );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_PINK , 23);
				}
				from += 6;
			} else if ( !strncasecmp( from, "<purple>", 8 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_PURPLE );
					to += strlen( YAHOO_COLOR_PURPLE );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_PURPLE, 23 );
				}
				from += 8;
			} else if ( !strncasecmp( from, "<orange>", 8 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_ORANGE );
					to += strlen( YAHOO_COLOR_ORANGE );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_ORANGE, 23 );
				}
				from += 8;
			} else if ( !strncasecmp( from, "<red>", 5 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_RED );
					to += strlen( YAHOO_COLOR_RED );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_RED, 23 );
				}
				from += 5;
			} else if ( !strncasecmp( from, "<yellow>", 8 )) {
				if ( show_html ) {
					strcat( to, YAHOO_COLOR_YELLOW );
					to += strlen( YAHOO_COLOR_YELLOW );
					strcat( colors, "|" );
					strcat( colors, prev_color );
					strncpy( prev_color, YAHOO_COLOR_YELLOW, 23);
				}
				from += 8;
			} else if (( !strncasecmp( from, "</black>", 8 )) ||
				( !strncasecmp( from, "</blue>", 7 )) ||
				( !strncasecmp( from, "</cyan>", 7 )) ||
				( !strncasecmp( from, "</gray>", 7 )) ||
				( !strncasecmp( from, "</green>", 8 )) ||
				( !strncasecmp( from, "</pink>", 7 )) ||
				( !strncasecmp( from, "</purple>", 9 )) ||
				( !strncasecmp( from, "</orange>", 9 )) ||
				( !strncasecmp( from, "</red>", 6 )) ||
				( !strncasecmp( from, "</yellow>", 9 ))) {
				if ( show_html ) {
					cptr = get_last_color( colors );
					strcat( to, cptr );
					to += strlen( cptr );
				}
				while( *from != '>' ) {
					from++;
				}
				from++;
			} else if ( !strncasecmp( from, "<i>", 3 )) {
				if ( show_html ) {
					strcat( to, YAHOO_STYLE_ITALICON );
					to += strlen( YAHOO_STYLE_ITALICON );
				}
				from += 3;
			} else if ( !strncasecmp( from, "</i>", 4 )) {
				if ( show_html ) {
					strcat( to, YAHOO_STYLE_ITALICOFF );
					to += strlen( YAHOO_STYLE_ITALICOFF );
				}
				from += 4;
			} else if ( !strncasecmp( from, "<b>", 3 )) {
				if ( show_html ) {
					strcat( to, YAHOO_STYLE_BOLDON );
					to += strlen( YAHOO_STYLE_BOLDON );
				}
				from += 3;
			} else if ( !strncasecmp( from, "</b>", 4 )) {
				if ( show_html ) {
					strcat( to, YAHOO_STYLE_BOLDOFF );
					to += strlen( YAHOO_STYLE_BOLDOFF );
				}
				from += 4;
			} else if ( !strncasecmp( from, "<font", 5 )) {
				/* EXPERIMENTAL - skip beginning font tags */
				/*
				from += 5;
				while(( *from ) && ( *from != '>' )) {
					from++;
				}
				from++;
				*/
				/* this basically insures that "<font" is in lowercase */
				from += 5;
				*(to++) = '<';
				*(to++) = 'f';
				*(to++) = 'o';
				*(to++) = 'n';
				*(to++) = 't';
			} else if ( !strncasecmp( from, "</font>", 7 )) {
				/* skip ending font tags */
				from += 7;
			} else if ( !strncasecmp( from, "<alt", 4 )) {

				if (show_blended_colors) {
					/* PhrozenSmoke: changed here, SAVE, dont skip this tag */
					ptr=from;
					while ((*ptr) && (*ptr != '>')) {
						*(to++)=*(ptr++); 
												   				  }
					if (*ptr == '>') {*(to++)=*(ptr++); }

														  } else {
					while ((*ptr) && (*ptr != '>')) {ptr++;}
					if (*ptr == '>') {ptr++; }
																	 }

					from=ptr;


			} else if ( !strncasecmp( from, "</alt>", 6 )) {

				if (show_blended_colors) {
					/* PhrozenSmoke: changed here, SAVE, dont skip this tag */
					ptr=from;
					while ((*ptr) && (*ptr != '>')) {
						*(to++)=*(ptr++); 
												   				  }
					if (*ptr == '>') {*(to++)=*(ptr++); }

														  } else {
					while ((*ptr) && (*ptr != '>')) {ptr++;}
					if (*ptr == '>') {ptr++; }
																	 }

					from=ptr;

				/* skip ending alt tags */
				   /* from += 6; */  /* disabled: PhrozenSmoke */
			} else if ( !strncasecmp( from, "<url=", 5 )) {
				/* skip beginning url tags */
				ptr = from + 5;
				while(( *ptr ) && ( *ptr != '>' )) {
					*(to++) = *(ptr++);
				}
				from = ptr;
				if ( *ptr )
					from++;
				*(to++) = ' ';
				*(to++) = '(';
			} else if ( !strncasecmp( from, "</url>", 6 )) {
				/* skip ending url tags */
				*(to++) = ')';
				from += 6;
			} else if (( !strncasecmp( from, "<snd", 4 )) ||
					   ( !strncasecmp( from, "<sound", 6 ))) {
				/* skip beginning snd tags */
				if ( from[2] == 'n' ) {
					ptr = from + 4;
				} else {
					ptr = from + 6;
				}

				if (( *ptr ) &&
					( *ptr == '=' )) {
					ptr++;
				}
				while(( *ptr ) && ( *ptr != '>' )) {
					ptr++;
				}
				if (( *ptr ) &&
					( *(from+4) == '=' )) {
					/* remember this sound so we can play it */
					if ( recv_sound ) {
						free( recv_sound );
						recv_sound = NULL;
					}
					if (( ptr - from ) < sizeof( tmp )) {
						strncpy( tmp2, from + 5, ptr - from - 5 );
						tmp2[ptr-from] = '\0';
						recv_sound = strdup( tmp2 );
					}
				}
				from = ptr;
				if ( *ptr )
					from++;

			} else if ( !strncasecmp( from, "</snd>", 6 )) {
				/* skip ending snd tags */
				from += 6;
			} else if ( !strncasecmp( from, "<fade", 5 )) {

				if (show_blended_colors) {
					/* PhrozenSmoke: changed here, SAVE, dont skip this tag */
					ptr=from;
					while ((*ptr) && (*ptr != '>')) {
						*(to++)=*(ptr++); 
												   				  }
					if (*ptr == '>') {*(to++)=*(ptr++); }

														  } else {
					while ((*ptr) && (*ptr != '>')) {ptr++;}
					if (*ptr == '>') {ptr++; }
																	 }

					from=ptr;

			} else if ( !strncasecmp( from, "</fade>", 7 )) {

				if (show_blended_colors) {
					/* PhrozenSmoke: changed here, SAVE, dont skip this tag */
					ptr=from;
					while ((*ptr) && (*ptr != '>')) {
						*(to++)=*(ptr++); 
												   				  }
					if (*ptr == '>') {*(to++)=*(ptr++); }

														  } else {
					while ((*ptr) && (*ptr != '>')) {ptr++;}
					if (*ptr == '>') {ptr++; }
																	 }

					from=ptr;

				/* skip ending fade tags */
					/* from += 7; */ /* disabled: PhrozenSmoke */
			} else {
				 *(to++) = *(from++); 
			}

			ptr = strchr( from, '<' );
		}

		strcat( to, from );
		to += strlen( from );
		*to = '\0';
		strcpy( str, tmp );

		if ( ! strchr( str, '\n' ))
			strcat( str, "\n" );
	}

	/* return if no urls */
	if (( ! strstr( str, "http://" )) && ( ! strstr( str, "https://" )) && 
		( ! strstr( str, "ftp://" ))) {
		return;
	}

	/* return if already marked */
	if ( strstr( str, YAHOO_STYLE_URLON ))
		return;

	/* return if not showing html */
	if ( ! show_html )
		return;

	memset( tmp, 0, sizeof( tmp ));

	from = str;
	to = tmp;

	/* search for ftp:// and http:// */
	ptr2 = strstr( str, "http://" );
		if (!ptr2) {ptr2=strstr( str, "https://" );}
	ptr3 = strstr( str, "ftp://" );

	if ( ptr2 && ptr3 ) {
		ptr = ptr2 < ptr3 ? ptr2 : ptr3;
	} else if ( ptr2 ) {
		ptr = ptr2;
	} else if ( ptr3 ) {
		ptr = ptr3;
	} else {
		ptr = NULL;
	}

	while( ptr ) {
		len = ptr - from;
		strncpy( to, from, len );
		to += len;
		*to = '\0';

		from += len;

		strcat( to, YAHOO_STYLE_URLON );
		to += strlen( YAHOO_STYLE_URLON );

		while(( *from ) && ( ! isspace( *from ))) {
			*(to++) = *(from++);
		}
		*to = '\0';

		strcat( to, YAHOO_STYLE_URLOFF );
		to += strlen( YAHOO_STYLE_URLOFF );

		/* search for ftp:// and http:// */
		ptr2 = strstr( from, "http://" );
		if (!ptr2) {ptr2=strstr( from, "https://" );}
		ptr3 = strstr( from, "ftp://" );

		if ( ptr2 && ptr3 ) {
			ptr = ptr2 < ptr3 ? ptr2 : ptr3;
		} else if ( ptr2 ) {
			ptr = ptr2;
		} else if ( ptr3 ) {
			ptr = ptr3;
		} else {
			ptr = NULL;
		}
	}

	strcat( to, from );
	to += strlen( from );
	*to = '\0';
	strcpy( str, tmp );
}

/*
 * strip junk (like "<snd" directives out of text
 */
void strip_junk( char *str ) {
	// DBG( 21, "strip_junk( '%s' )\n", str );

	/* fixme */
}

void capture_text_to_file( char *filename ) {

	// DBG( 11, "capture_text_to_file( '%s' )\n", filename );

	ct_capture_to_file( filename );
}


void set_max_url_fetch_size(int mysize) {  /* control buffers on URL downloads */
	max_url_fetch_size=mysize-2;
	if (max_url_fetch_size >100352) {max_url_fetch_size=100352;}
}

void unset_url_post_data() {
	if (url_post_data) {free(url_post_data); url_post_data=NULL;}
}

void set_url_post_data(char *pp) {
	if (url_post_data) {free(url_post_data); url_post_data=NULL;}
	if (pp) {url_post_data=strdup(pp);}
}
void clear_fetched_url_data() {
	if (webresult) {free(webresult); webresult=NULL;}
}

/* fetch a URL into a predefined buffer */

int fetch_url( char *image_url, char *buf, char *cookie ) {
	int sock = -1;
	struct sockaddr_in sa;
	struct hostent *hinfo;
	char url[1024];
	char tmp[512];
	char *ptr;
	int nr = 0, tr = 0;
	char host[56] = "";

	if (webresult) {free(webresult); webresult=NULL;}

	if (!webresult) {
		webresult=malloc(max_url_fetch_size+2);
		if (!webresult) {return 0;}
	}

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  FETCH URL:\n   URL: %s\n   Cookie: %s\n\n", ctime(&time_llnow), image_url, cookie?cookie:"None");
			fflush( capture_fp );
			}

	if ( strncmp( image_url, "http://", 7 )) {
		if ( strncmp( image_url, "scnt://", 7 )) {return( 0 ); }
	}

	webresult[0] = '\0';
	buf[0] = '\0';

	strncpy( host, image_url + 7, 54 );
	host[54] = '\0';
	ptr = strchr( host, '/' );
	*ptr = '\0';

	check_proxy_config();

	set_socket_timer(sock, -1);
	set_socket_timer(sock, 12);

	if ( use_proxy ) {
		ptr = image_url;
		hinfo = gethostbyname( proxy_host );
	} else {
		ptr = strchr( image_url + 7, '/' );
		hinfo = gethostbyname( host );
	}

	set_socket_timer(sock, -1);

	if ( ! hinfo || ( hinfo->h_addrtype != AF_INET )) {
		if (use_proxy)  { /* added: PhrozenSmoke */
		snprintf(tmp, 150, "%s:\n%s", _("Could not connect to server"), proxy_host );
		show_ok_dialog(tmp);
								} else {
		snprintf(tmp, 150, "%s:\n%s", _("Could not connect to server"), host );
		show_ok_dialog(tmp);
										   }
		return( 0 );
	}

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
		snprintf(tmp,150,"%s:\n%s", _("Could not connect to server"), proxy_host );
		show_ok_dialog(tmp);
								} else {
		snprintf(tmp, 150,"%s:\n%s", _("Could not connect to server"), host );
		show_ok_dialog(tmp);
										   }

		/* if ( DBG_LEVEL > 10 ) {
			printf( "error in socket()\n" );
		} */ 
		return( 0 );
	} else {
		set_socket_timer(sock, 8);
		if ( connect( sock, (struct sockaddr*)&sa, sizeof(sa)) == -1 ) {

		if (use_proxy)  { /* added: PhrozenSmoke */
		snprintf(tmp,150, "%s:\n%s", _("Could not connect to server"), proxy_host );
		show_ok_dialog(tmp);
								} else {
		snprintf(tmp,150, "%s:\n%s", _("Could not connect to server"), host );
		show_ok_dialog(tmp);
										   }


			if ( DBG_LEVEL > 10 ) {
				printf( "error in connect()\n" );
			}
			set_socket_timer(sock, -1);
			close(sock);
			return( 0 );
		}
	}

	snprintf( url, 940,
		"%s %s HTTP/%s\r\n"
		"Accept: application/xml,application/xhtml+xml,text/html,text/plain,*/*\r\n"
		"Accept: text/plain\r\n"
		/* "User-Agent: Gyach/%s\r\n" */     /* changed: PhrozenSmoke */
		"User-Agent: %s\r\n"      /* changed: PhrozenSmoke */
		"Host: %s\r\n",

		url_post_data?"POST":"GET",
		ptr, 
		url_post_data?"1.1":"1.0",
		GYACH_USER_AGENT, host );     /* changed: PhrozenSmoke */

		if (strstr(image_url,"/share/get") || strstr(image_url,"/share/put") ) { /* photos imv */ 
			strncat(url, "x-flash-version: 7,0,19,0\r\n", 28);
			if (strstr(image_url,"/share/put")) {
				strncat(url, "Content-Type: application/x-www-form-urlencoded\r\n", 50);
			}
		}

	set_socket_timer(sock, -1);
	set_socket_timer(sock, 10);
	write( sock, url, strlen( url ));

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  FETCH URL: \n   URL: %s\n   Sent Data Request:\n%s\n\n", ctime(&time_llnow), image_url, url);
			fflush( capture_fp );
			}

	sprintf(url,"%s","");  /* dont remove */
	if ( cookie ) {
		strcat( url, "Cookie: " );

		if (strstr(host, "ab.login.yahoo.com")) {
			/* reactivation URLs */ 
			snprintf(url, 1012, "%s",  "Cookie: B=9a4k50l00fmia&b=2; Q=q1=AACAAAAAAAAAbw--&q2=QJMimQ--; F=a=.Z6jMDgsvaBtGShSqwPBTq0Ky1SSv3GwNnlt3BTHmMAZ2FO0YmBtuOWyfWKR&b=B0ZL; LYC=l_v=0&l_lv=7&l_s=zwz03zv2v1rzu5wv11z4z5xu34xuty4w&l_lc=0_0_32_0_-1&l_um=0_1_1_0_0; U=mt=CkOmvJ2MhYs818jJ_hTEskZoqphBfV.t2yWA&ux=0XuyAB&un=4ruau026ct429; PROMO=sessionID=; C=mg=1");    

		} 	else {strncat( url, cookie, 900 );}
		strcat( url, "\r\n" );
	}


	if (! url_post_data) { strcat( url, "\r\n" );	}

	//printf("URL STUFF: %s",url);  fflush(stdout);

	write( sock, url, strlen( url ));

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  FETCH-URL COOKIES:\n%s\n\n", ctime(&time_llnow), url);
			fflush( capture_fp );
			}

	if (url_post_data) {
		set_socket_timer(sock, -1);
		set_socket_timer(sock, 30);
		snprintf(url, 128,  "Content-Length: %d\r\n\r\n", strlen(url_post_data));
		write( sock, url, strlen( url ));
		write( sock, url_post_data, strlen(url_post_data));
		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  FETCH-URL POST:\n%s\n%s\n\n", ctime(&time_llnow), url, url_post_data);
			fflush( capture_fp );
			}
	}

	webresult[0] = '\0';
	tmp[0]='\0';

	tr = 0;
	if (max_url_fetch_size<518) { 	set_socket_timer(sock, -1); close( sock ); return 0;}
	set_socket_timer(sock, -1);

	if (strstr(image_url, "/a/list?.ver" )) {set_socket_timer(sock, 25);}
	else {set_socket_timer(sock, 12); }

	nr = read( sock, tmp, 512 );
	while(( nr > 0 ) &&
		( tr < (max_url_fetch_size-513 ) )) {
		// int in_allow=max_url_fetch_size-tr-1;
		process_gtk_events();
		tmp[nr] = '\0';
		memcpy( webresult + tr, tmp, nr );
		tr += nr;
		set_socket_timer(sock, -1);
		set_socket_timer(sock, 12);
		nr = read( sock, tmp, 512 );
	}

	close( sock );
	set_socket_timer(sock, -1);
	if (tr<0) {tr=0;}
	webresult[tr] = '\0';

	if ( (tr<1) || (! strstr( webresult, "\r\n\r\n" )) )  {
		sprintf(buf,"%s","");
		return 0;
	}

	ptr = strstr( webresult, "\r\n\r\n" );
	if (ptr) {ptr += 4;}

	tr -= ( webresult - ptr );
	memcpy( buf, ptr, tr );

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  FETCH URL-Retrieved Data: \n   URL: %s\n   Got Data Size: %d\n   Got Data:\n%s\n\n", ctime(&time_llnow), image_url, tr, buf);
			fflush( capture_fp );
			}

	return( tr );
}


void subst_escs( char *str ) {
	char tmp[1536];
	char *from = tmp;
	char *to = str;

	// DBG( 21, "subst_escs( '%s' )\n", str );

	if ( ! strchr( str, '\\' ))
		return;

	strncpy( tmp, str, 1534 );

	while( *from ) {
		if (( *from == '\\' ) &&
			( *(from+1) == '\\' ) &&
			( *(from+2) == 'n' )) {
			*(to++) = '\\';
			*(to++) = 'n';
			from += 3;
		} else if (( *from == '\\' ) &&
			( *(from+1) == 'n' )) {
			*(to++) = '\n';
			from += 2;
		} else if (( *from == '\\' ) &&
			( *(from+1) == 't' )) {
			*(to++) = '\t';
			from += 2;
		} else {
			*(to++) = *(from++);
		}
	}
	*to = '\0';
}

char *filter_text( char *txt ) {
	static char buffilt[513] = "";
	char data[513];
	char *ptr;
	char new_cmd[768];
	FILE *fp;

	// DBG( 11, "filter_text( '%s' )\n", txt );

	/* zero out buffilt every time */
	buffilt[0] = '\0';

	if ( ! filter_command ) {
		strncpy( buffilt, txt , 511);
		return( buffilt );
	}

	/* sanity check to make sure our echo doesn't hose up */
	ptr = txt;
	while( *ptr ) {
		if ( *ptr == '"' )
			*ptr = '\'';
		ptr++;
	}
	snprintf( new_cmd, 766, "echo \"%s\" | %s", txt, filter_command );

	fp = popen( new_cmd, "r" );

	if ( fp ) {
		while(( strlen( buffilt ) < 512 ) &&
			  ( fgets( data, 512-strlen(buffilt), fp ))) {
			if (( strlen( buffilt ) + strlen( data )) < 512 )
				strcat( buffilt, data );
		}
		pclose( fp );

		ptr = buffilt + strlen( buffilt );
		ptr--;
		while(( *ptr == '\n' ) || ( *ptr == '\r' )) {
			*ptr = '\0';
			ptr--;
		}

		if ( strlen( buffilt ) == 511 ) {
			strcpy( buffilt+507, "...." );
		}
	} else {
		strncpy( buffilt, txt, 511 );
	}

	return( buffilt );
}

char *replace_args( char *str, char *args ) {
	static char result[1025] = "";
	char zargs[1025];
	char tmp[1025];
	char search[4];
	char *arg;
	char *ptr;
	char *end;
	char *ptr2;
	int x;

	// DBG( 11, "replace_args( '%s', '%s' )\n", str, args );

	strncpy( result, str , 1024);
	strncpy( zargs, args, 1024 );

	if (( ! strchr( result, '$' )) &&
		( ! strstr( result, "%s" ))) {
		return( result );
	}

	arg = zargs;
	for( x = 0; x < 10; x++ ) {
		/* find next arg */
		arg = skip_whitespace( arg );
		end=strchr(arg,'\0');
		/* end = find_whitespace( arg );  */
		*end = '\0';

		snprintf( search, 3, "$%d", x );
		ptr2 = result;
		while(( ptr = strstr( ptr2, search )) != NULL ) {
			/* skip if escaped */
			if (( ptr != result ) &&
				( *(ptr-1) == '\\' )) {
				ptr2++;
				continue;
			}

			strncpy( tmp, ptr + 2, 1022 );

			/* append arg if we have room */
			if (( ptr - result + strlen( arg ) + 1 ) < sizeof( result )) 
				strcpy( ptr, arg );

			/* append rest of string after arg if we have room */
			strncat( ptr, tmp, sizeof( result ) - strlen( result ));
			ptr2 = ptr;
		}

		arg = end + 1;
	}

	ptr2 = result;
	while(( ptr = strstr( ptr2, "$*" )) != NULL ) {
		/* skip if escaped */
		if (( ptr != result ) &&
			( *(ptr-1) == '\\' )) {
			ptr2++;
			continue;
		}

		strncpy( tmp, ptr + 2, 1024 );

		/* append arg if we have room */
		if (( ptr - result + strlen( args ) + 1 ) < sizeof( result )) 
			strcpy( ptr, args );

		/* append rest of string after arg if we have room */
		strncat( ptr, tmp, sizeof( result ) - strlen( result ));
		ptr2 = ptr;
	}

	ptr2 = result;
	while(( ptr = strstr( ptr2, "%s" )) != NULL ) {
		/* skip if escaped */
		if (( ptr != result ) &&
			( *(ptr-1) == '\\' )) {
			ptr2++;
			continue;
		}

		strncpy( tmp, ptr + 2 , 1024);

		/* append arg if we have room */
		if (( ptr - result + strlen( args ) + 1 ) < sizeof( result )) 
			strcpy( ptr, args );

		/* append rest of string after arg if we have room */
		strncat( ptr, tmp, sizeof( result ) - strlen( result ));
		ptr2 = ptr;
	}

	return( result );
}



void *fetch_avatar_all( void *arg, int is_yavatar, int ava_size, int bimage, char *burl ) {
	int allocsize=7168;
	char *buf=NULL;   /* avatars are usually under 2-3kb */	
	char *avatar = (char *)arg;
	char filename[192];
	char dirbuf[192];
	char url[240];
	int length=0;
	FILE *fp;
	struct stat sbuf;
	
	if (bimage) { /* a buddy image */
			char av_clean[40]="";
			snprintf( url, 190, "%s", burl );  /* will be a full URL */ 
			snprintf(av_clean,35, "%s", avatar);
			clean_bimage_name(av_clean);
			snprintf( dirbuf, 190, "%s/bimages", GYACH_CFG_DIR );
			snprintf( filename, 190, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, av_clean);
			allocsize=48672;
	} else if (is_yavatar) {

		/* Yahoo's new avatars for buddy lists, etc */
		if (ava_size>3) {return( NULL );} 
		snprintf( dirbuf, 190, "%s/yavatars", GYACH_CFG_DIR );
		if (ava_size==3) {
			snprintf( url, 238, "http://img.avatars.yahoo.com/users/%s.full.swf?src=ymsgr&intl=us&os=win&ver=%s", avatar, 
				version_emulation );
			snprintf( filename, 254, "%s/yavatars/%s.full.swf", GYACH_CFG_DIR, avatar);
			allocsize=56864;
			}
		if (ava_size==2) {
			snprintf( url, 238, "http://img.avatars.yahoo.com/users/%s.large.%s?src=ymsgr&intl=us&os=win&ver=%s", avatar, 
				avatar_filetype, version_emulation );
			snprintf( filename, 190, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, avatar, 
				avatar_filetype );
			allocsize=48672;
			}
		if (ava_size==1) {
			snprintf( url, 238, "http://img.avatars.yahoo.com/users/%s.medium.%s?src=ymsgr&intl=us&os=win&ver=%s", avatar, avatar_filetype, version_emulation );
			snprintf( filename, 190, "%s/yavatars/%s.medium.%s", GYACH_CFG_DIR, avatar, 
				avatar_filetype );
			allocsize=14236;
			}
		if (ava_size==0) {
			snprintf( url, 238, "http://img.avatars.yahoo.com/users/%s.small.%s?src=ymsgr&intl=us&os=win&ver=%s", avatar, 
				avatar_filetype, version_emulation );
			snprintf( filename, 190, "%s/yavatars/%s.small.%s", GYACH_CFG_DIR, avatar, 
				avatar_filetype );
			allocsize=6144;
			}

	} else {
		/* Cheeta Chat avatars */
		char tmp_avatar[72] = "";
		strncpy( tmp_avatar, avatar, 70 );
		lower_str( tmp_avatar );		
		snprintf( filename, 190,  "%s/avatars/%s.jpg", GYACH_CFG_DIR, tmp_avatar );
		/* also avatars.cheetachat.com  - same IP address */
		snprintf( url, 190, "http://avserv.cheetachat.com/avatar/%s.jpg", avatar );
		snprintf( dirbuf, 190, "%s/avatars", GYACH_CFG_DIR );	
	}

	free( avatar );
	
	/* Make any needed directories */ 
	if ( stat( dirbuf, &sbuf )) {mkdir( dirbuf, 0700 );  }

	buf=malloc(allocsize);
	if (!buf) {return( NULL );}
	sprintf(buf, "%s", "");
		// printf("alloced size: %d\n", allocsize); fflush(stdout);
	set_max_url_fetch_size(allocsize-1);
	length = fetch_url( url, buf, NULL );

		// printf("url length: %d\n", length); fflush(stdout);

	if ( length ) {
		if ( strstr( buf, "document has moved" )) {
			char *start, *end;

			if (is_yavatar) {free(buf); return( NULL );}
			start = strstr( buf, "com/avatar/" );
			if (start) {
				start += 11;
				end = strchr( start, '.' );
				if (end) {
					*end = '\0';
					free(buf);
					return fetch_avatar( (void *)strdup(start));
							}
						}
		} else if ( strstr( buf, "was not found" )) {
			free(buf);
			clear_fetched_url_data();
			return( NULL );
		} else {
			fp = fopen( filename, "w" );
			if ( fp ) {
				fwrite( buf, length, 1, fp );
				fclose( fp );
			}
		}
	}

	free(buf);
	clear_fetched_url_data();
	return( NULL );
}


void *fetch_avatar( void *arg ) {
	return fetch_avatar_all(arg, 0, 0, 0, NULL);
}

void *fetch_yavatar( void *arg, int ava_size, char *burl ) {
	if ( (! show_yavatars) && (ava_size == 0) ) {return 0; }
	if ( (! show_bimages) && (ava_size == 4) ) {return( NULL );}

	if (ava_size != 4) {  /* avatar */
				// printf("dl-2\n"); fflush(stdout);
		return fetch_avatar_all(arg, 1, ava_size, 0, NULL);
	}
	return fetch_avatar_all(arg, 0, 0, 1, burl);  /* buddy image */
}


int download_yavatar(char *avatar, int ava_size, char *burl) {
	char filename[192];
	struct stat sbuf;

	if ( (! show_yavatars) && (ava_size == 0) ) {return 0; }
	if ( (! show_bimages) && (ava_size == 4) )  {return 0; }
	if (!avatar) {return 0;}

	if (ava_size==2) {
		snprintf( filename, 190, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, avatar, 
			avatar_filetype );	
	} 
	else if (ava_size==3) {  /* shockwave animation */
		snprintf( filename, 190, "%s/yavatars/%s.full.swf", GYACH_CFG_DIR, avatar );	
	} 
	else if (ava_size==4) {  /* buddy image */
		snprintf( filename, 190, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, avatar );	
	} 
	else if (ava_size==1) {
		/* around 3-5 kb */
		snprintf( filename, 190, "%s/yavatars/%s.medium.%s", GYACH_CFG_DIR, avatar, 
			avatar_filetype );	
	} 
	else  {
		snprintf( filename, 190, "%s/yavatars/%s.small.%s", GYACH_CFG_DIR, avatar, 
			avatar_filetype );	
	} 

	if ( stat( filename, &sbuf ) || (ava_size==4) ) {
		/* doesn't exist, so download for displaying */
				// printf("dl-1\n"); fflush(stdout);
		fetch_yavatar(strdup(avatar), ava_size, burl);	
	} else {return 1;}

	if ( stat( filename, &sbuf )) {
		/* couldnt be downloaded */
		return 0;	
	} else {return 1;}
	return 0;
}

int bimage_exists(char *ava_key) {
	char filename[192];
	struct stat sbuf;
	if (!ava_key) {return 0;}
	snprintf( filename, 190, "%s/bimages/bimage-%s.png", GYACH_CFG_DIR, ava_key );
	if (  stat( filename, &sbuf )) {return 0;}
return 1;
}

void clean_bimage_name(char *bkey) {
	int stopper=0;
	if (!bkey) {return;}
	while (stopper< (strlen(bkey)-1) ) {
		if (bkey[stopper]=='\0') {break;}
		if (! isdigit(bkey[stopper]) ) {bkey[stopper]='B';}
		stopper++;
		}
}

void delete_yavatars(char *ava_key) {
	char filename[192];
	struct stat sbuf;
	if (!ava_key) {return;}
	snprintf( filename, 190, "%s/yavatars/%s.small.%s", GYACH_CFG_DIR, ava_key, 
		avatar_filetype);
	if ( ! stat( filename, &sbuf )) {unlink(filename);	}
	snprintf( filename, 190, "%s/yavatars/%s.medium.%s", GYACH_CFG_DIR, ava_key, 
		avatar_filetype);
	if ( ! stat( filename, &sbuf )) {unlink(filename);	}
	snprintf( filename, 190, "%s/yavatars/%s.large.%s", GYACH_CFG_DIR, ava_key, 
		avatar_filetype);
	if ( ! stat( filename, &sbuf )) {unlink(filename);	}
	snprintf( filename, 190, "%s/yavatars/%s.full.swf", GYACH_CFG_DIR, ava_key );
	if ( ! stat( filename, &sbuf )) {unlink(filename);	}
}

void delete_all_yavatars() {
	char filename[192];
	snprintf( filename, 190, "rm -f %s/yavatars/*.small.%s", GYACH_CFG_DIR, 
		avatar_filetype);
	my_system(filename);
	snprintf( filename, 190, "rm -f %s/yavatars/*.medium.%s", GYACH_CFG_DIR,
		avatar_filetype);
	my_system(filename);
	snprintf( filename, 190, "rm -f %s/yavatars/*.large.%s", GYACH_CFG_DIR, 
		avatar_filetype);
	my_system(filename);
	snprintf( filename, 190, "rm -f %s/yavatars/*.full.swf", GYACH_CFG_DIR);
	my_system(filename);
}

void delete_all_bimages() {
	char filename[192];
	snprintf( filename, 190, "rm -f %s/bimages/bimage-*.png", GYACH_CFG_DIR );
	my_system(filename);
}


void display_avatar( char *user, char *avatar ) {
	char filename[192];
	char ava_part[72];
	struct stat sbuf;
	pthread_t av_thread;

	if ( ! strcmp( avatar, "random" )) {return;	}

	/* Changed, PhrozenSmoke: The orignal author was lowering 
	the whole filename, which could cause problems if 'GYACH_CFG_DIR' was not
	all lowercase, i.e. the avatar not being found and repeated 'fetching' of
	the avatar.  */

	strncpy(ava_part, avatar,70);
	lower_str( ava_part );
	snprintf( filename, 190, "%s/avatars/%s.jpg", GYACH_CFG_DIR, ava_part );	

	if ( stat( filename, &sbuf )) {
		/* doesn't exist, so download for displaying later */
		pthread_create( &av_thread, NULL, fetch_avatar, (void *)strdup(avatar));
	} else {
		/* exists, so display */
		if ( ct_can_do_pixmaps()) {
			ct_append_pixmap( filename,NULL );
		} 
	}
}


void *play_sound( void *arg ) {
	char *sound = (char *)arg;
	char cmd[384];

	snprintf( cmd, 380, "wavplay %s > /dev/null 2>&1", sound );

	system( cmd );
	free( sound );
	return( NULL );
}


gint gstrcmp( gpointer a, gpointer b ) {
	if ( a && b ) {
		// DBG( 31, "gstrcmp( '%s', '%s' )\n", (char *)a, (char *)b );
	} else if ( a ) {
		// DBG( 31, "gstrcmp( '%s', NULL )\n", (char *)a );
	} else if ( b ) {
		// DBG( 31, "gstrcmp( NULL, '%s' )\n", (char *)b );
	} else {
		// DBG( 31, "gstrcmp( NULL, NULL )\n" );
	}
	fflush( stderr );

	if (( ! a ) &&
		( ! b )) {
		// DBG( 32, "RETURN gstrcmp() == 0\n" );
		return( 0 );
	}

	if ( ! a ) {
		// DBG( 32, "RETURN gstrcmp() == -1\n" );
		return( -1 );
	}

	if ( ! b ) {
		// DBG( 32, "RETURN gstrcmp() == 1\n" );
		return( 1 );
	}

	// DBG( 32, "RETURN gstrcmp() == %d\n", strcasecmp( (char *)a, (char *)b ));

	return( strcasecmp( (char *)a, (char *)b ));
}


GList *gyach_g_list_free( GList *list ) {
	GList *node;

	if ( ! list )
		return( NULL );

	node = g_list_first( list );
	while( node ) {
		free( node->data );
		node = g_list_next( node );
	}
	g_list_free( list );

	return( NULL );
}

GList *gyach_g_list_copy( GList *list ) {
	GList *result;
	GList *node;

	result = g_list_copy( list );
	node = g_list_first( result );
	while( node ) {
		node->data = strdup( node->data );
		node = g_list_next( node );
	}

	return( result );
}

GList *gyach_g_list_merge( GList *main, GList *sub ) {
	GList *this_item;
	GList *found_item;

	this_item = sub;
	while( this_item ) {
		found_item = g_list_find_custom( main, this_item->data,
			(GCompareFunc)gstrcmp );

		if ( ! found_item ) {
			g_list_append( main, strdup( this_item->data ));
		}

		this_item = g_list_next( this_item );
	}
	gyach_g_list_free( sub );
	return( main );
}


/* This was moved in from the no-longer-used
    support.c module */

GtkWidget *lookup_widget (GtkWidget  *widget, const gchar *widget_name)
{
  GtkWidget *parent, *found_widget;

  for (;;)
    {
      if (GTK_IS_MENU (widget))
        parent = gtk_menu_get_attach_widget (GTK_MENU (widget));
      else
        parent = widget->parent;
      if (parent == NULL)
        break;
      widget = parent;
    }

  found_widget = (GtkWidget*) gtk_object_get_data (GTK_OBJECT (widget),
                                                   widget_name);
  if (!found_widget)
    g_warning ("Widget not found: %s", widget_name);
  return found_widget;
}


