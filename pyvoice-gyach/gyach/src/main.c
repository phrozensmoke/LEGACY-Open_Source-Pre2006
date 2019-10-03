/*****************************************************************************
 * main.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#  include <libintl.h>  /* added PhrozenSmoke: locale support */
#include <locale.h>  /* added PhrozenSmoke: locale support */

#include <gtk/gtk.h>


#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-loader.h>

#ifdef USE_PTHREAD_CREATE
#include <pthread.h>
#endif

#include "gyach.h"

#ifndef OS_WINDOWS
#	include <X11/Xlib.h>
#	include <gdk/gdkx.h>
#endif

#include <errno.h>

#include "aliases.h"
#include "callbacks.h"
#include "commands.h"
#include "gyach_int.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "sounds.h"  /* added, PhrozenSmoke */
#include "bootprevent.h"  /* added, PhrozenSmoke */
#include "animations.h"  /* added, PhrozenSmoke */
#include "conference.h"   /* added, PhrozenSmoke */
#include "packet_handler.h"    /* added, PhrozenSmoke */
#include "trayicon.h"   /* added, PhrozenSmoke */
#include "plugins.h"
#include "ycht.h"
#include "fonts.h"

extern void check_tag_table_validity(GtkTextTagTable *tag_table);

    /* 
   extern PM_SESSION * new_pm_session(char *forwho);
   extern void run_news_test();  
   extern void run_status_test();
   extern void on_create_fadeconfigwin (GtkButton  *button, gpointer user_data) ;
   extern void set_tuxvironment (GtkWidget *pmwidget, GtkWidget *pmwindow, char *tuxviron);
   extern void send_automated_response(char *inuser, char *inalias, char *myinmsg); */


extern char *custom_scale_text;
int in_a_chat=0;
char *yalias_name=NULL;  /* for Yahoo 'nick name' handling */


/* ------------------------------------------------------------------------ */ 

gchar *mygconvedstr=NULL;
gchar *my_locale_string=NULL;
char *_(char *some) ;

/* added, PhrozenSmoke...to help avoid server-side boots */
extern int possible_boot_attempts;
extern unsigned long sock_sleep;   /* flow control, added PhrozenSmoke, boot prevention */



char *_(char *some) 
{
/*
#ifdef USE_GTK2
	if (my_locale_string != NULL) {g_free(my_locale_string);}
	my_locale_string=g_locale_to_utf8(gettext(some),-1, NULL, NULL, NULL );
	return my_locale_string;
#endif */
	return gettext(some);
}

gchar *_utf(char *some) { /* converts from the assumed locale to UTF */
	if (mygconvedstr) {g_free(mygconvedstr); mygconvedstr=NULL;}
	if (g_utf8_validate ( some, -1, NULL))  { /* already in utf */
		mygconvedstr=g_strdup(some);
		return mygconvedstr;
	}
	mygconvedstr=g_convert(some, -1,"UTF-8","ISO8859-1",NULL,NULL,NULL);
	if (! mygconvedstr) {mygconvedstr=g_strdup(some);}
	return mygconvedstr;
}

gchar *_b2loc(char *some) {  /* converts back from UTF to locale string */
	if (my_locale_string) {g_free(my_locale_string); my_locale_string=NULL;}
	if (! g_utf8_validate ( some, -1, NULL))  {
		my_locale_string=g_strdup(some);
		return my_locale_string;
	}
	my_locale_string=g_locale_from_utf8(some, -1, NULL, NULL,NULL);
	if (! my_locale_string) {my_locale_string=g_strdup(some);}
	return my_locale_string;
}


/* added PhrozenSmoke: from fader.c */
extern void convert_fader(char *str);


int real_show_colors = 1;
static int bold = FALSE, italic = FALSE, underline = FALSE, url = FALSE;

void tv_jump_to_bottom( GtkTextView *text_view) {
	GtkTextIter end;
	GtkTextBuffer *text_buffer;
	GtkTextMark *end_mark;
	text_buffer = gtk_text_view_get_buffer( text_view );
	if (! text_buffer ) {return;}
	if (! text_view) {return;}
	end_mark = gtk_text_buffer_get_mark( text_buffer, "end_mark" );

	gtk_text_buffer_get_iter_at_mark( text_buffer, &end, end_mark );
	gtk_text_buffer_place_cursor( text_buffer, &end );		
	gtk_text_view_scroll_to_mark( text_view, end_mark, 0, FALSE, 0, 0 );
	/*  */   gtk_text_view_scroll_mark_onscreen( text_view, end_mark);
}

int tv_scroll_needed(GtkTextView *text_view) {
	GtkAdjustment *adj;
	int scroller=0;
	/* Scroll down if we aint scrolling up */
	adj = text_view->vadjustment;
	if (adj) {
		if ( adj->value == ( adj->upper - adj->lower - adj->page_size )) {scroller=1;}
		/* not right at the bottom, but very close ... */ 
		if ( adj->value > ( adj->upper - adj->lower - adj->page_size -33 )) {scroller=1;}
	}

		/* printf("targ: %f\n", (float) ( adj->upper - adj->lower - adj->page_size )); fflush(stdout); 
		printf("adj: %f\n", (float) adj->value ); fflush(stdout);  */  

	return scroller;
}

void tv_append_text_with_color( GtkTextView *text_view, char *text, int len,
		int r, int g, int b, char *font ) {
	int text_length;
	GtkTextIter start;
	GtkTextIter end;
	char buf[1536];
	char tmp[128];
	GtkTextTagTable *tag_table;
	GtkTextBuffer *text_buffer;
	GdkColor chat_color;
	GtkTextIter iter;
	GtkTextMark *end_mark;
	int textlen;
	int chcount=0;

	/* printf("Colors:  %d  %d  %d\n", r,g,b); fflush(stdout); */ 

	text_buffer = gtk_text_view_get_buffer( text_view );

	chat_color.red   = r * ( 65535 / 255 );
	chat_color.green = g * ( 65535 / 255 );
	chat_color.blue  = b * ( 65535 / 255 );

	snprintf( tmp, 126, "%02X%02X%02X|%s|%s", r,g,b,
	font, underline?"u1":"u0"	);  /* create hex string 'ffccff', etc. */

	tag_table = gtk_text_buffer_get_tag_table( text_buffer );
	check_tag_table_validity(tag_table);
	if ( ! gtk_text_tag_table_lookup( tag_table, tmp )) {
		GtkTextTag *ttag;	
		ttag=gtk_text_buffer_create_tag( text_buffer, tmp, "foreground-gdk",
			&chat_color, NULL);
		if (underline) {g_object_set(ttag,"underline",PANGO_UNDERLINE_SINGLE, NULL);}
		g_object_set(ttag,"font",font, NULL);
		g_object_set(ttag,"editable",FALSE, NULL);
			/* Below, this tag 'encourages' use of iso-8859-1 charsets, suitable 
			for  English and most Euro languages, avoids most Gtk-related 
			warnings about trying to load the wrong charset for the font, so assume
			English */
		g_object_set(ttag,"language","en_US.ISO8859-1", NULL);

		/* if (strcmp(custom_scale_text,"1.0")) { */ 
			g_object_set(ttag,"scale", (double) atof(custom_scale_text), NULL);
		/* } */
	}


	gtk_text_buffer_get_end_iter( text_buffer, &iter );
	chcount += gtk_text_iter_get_offset(&iter);
	textlen=len;
	if (textlen>1530) {textlen=1530;}
	sprintf(buf,"%s","");
	strncpy( buf, text, textlen );
	buf[textlen] = '\0';
	gtk_text_buffer_insert_with_tags_by_name( text_buffer, &iter, buf, textlen,
		tmp, NULL );

	/* If PM, do logging */
	if (gtk_object_get_data(GTK_OBJECT(text_view),"logfile") != NULL ) {
		FILE *fp;
		gchar *capb2loc;
		char *dptr;
		fp=gtk_object_get_data(GTK_OBJECT(text_view),"logfile");
		end_mark = gtk_text_buffer_get_mark( text_buffer, "end_mark" );
		gtk_text_buffer_get_iter_at_mark( text_buffer, &end, end_mark );
		gtk_text_buffer_get_iter_at_offset( text_buffer, &start, chcount );
		dptr = gtk_text_buffer_get_text(text_buffer, &start, &end, 0);
		capb2loc=_b2loc(dptr);
		fwrite( dptr, 1, strlen(dptr), fp );
		fflush(fp);
		g_free(dptr);
	}

	/* For PM windows and other text buffers besides the Chat window */
		/* Limit the text in the box to about 9kb, This gives us enough of 
		a PM 'history' but stops the PM window from getting slow 
		due to too much text in the box  */
	text_length = gtk_text_buffer_get_char_count( text_buffer );
	if ( text_length > 9216) {
			gtk_text_buffer_get_iter_at_offset( text_buffer, &start, 0 );
			gtk_text_buffer_get_iter_at_offset( text_buffer, &end, (text_length-9216)+256 );
			gtk_text_buffer_delete( text_buffer, &start, &end );
		}
}


void append_to_textbox(GtkWidget *window, GtkWidget *textbox, char *intext)
{
	int i = 0;
	char face[75] = "", size[10] = "", my_font_face[75]="";
	int tmp_color;
	char *tmp_ptr;
	char *tmp_ptr2;
	char buf[384];
	char text[3072];
	int scroll = FALSE;
	unsigned int r, g, b;
	unsigned int or, og, ob;
	char *sound = NULL;
	struct stat sbuf;
	int tmp_int;
	char sm_file[64];
#ifdef USE_PTHREAD_CREATE
	pthread_t prof_thread;
#endif
	char user_font[112];

	// DBG( 21, "append_to_textbox( %p, %p, '%s' )\n",window, textbox, text );

	*face = *size = '\0';

	/*    printf("STUFF:  %s\n",text);
	fflush(stdout); */ 

	/* This automatically falls back to Sans if all else fails WITHOUT console warnings */ 
	if (! display_font_family) {display_font_family=strdup("Sans");}
	if (! display_font_name) {display_font_name=strdup("Sans 12");}
	if (! font_family) {font_family=strdup("Sans");}
	if (! font_name) {font_name=strdup("Sans 12");}

	strncpy( face, map_font_family(display_font_family),73 );

		/* printf("FACE: %s\n", face);  fflush(stdout); */

	/*  bold = use_bold;
	    italic = use_italics;  */ 

	if ( strstr( display_font_name, " Bold" )) {	bold = 1;} 
	else {	bold = 0;	}
	if ( strstr( display_font_name, " Italic" )) {italic = 1;} 
	else {	italic = 0;}  

	/* added: PhrozenSmoke...set some reasonable font size limits */
	if (display_font_size>32) {display_font_size=32;}
	if (display_font_size<12) {display_font_size=12;}
	snprintf( size, 4, "%d", display_font_size );

	sprintf(text,"%s","");
	strncpy(text,intext,2047); /* save formatting room, fader, etc. avoid buffer over flow */

		if ( capture_fp ) {	 /* logging */
			gchar *mylocb=_b2loc(text);
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\nTEXT BUFFER ADDITION  [%s]: \n\n%s\n",ctime(&time_llnow), mylocb);
			fflush( capture_fp );
			}


	/* changed here: PhrozenSmoke : call '"."convert_smileys' BEFORE 'convert_tags' */  
	 /* main chat window only */ /*
	if (( show_emoticons ) &&
		( ! textbox )) { 
		convert_smileys( text );
	}  */  

	if (show_emoticons ) {convert_smileys( text );	} 

	/* printf("STUFF-smile:  %s\n",text);
	fflush(stdout); */ 

	if ( force_lowercase )
		lower_str( text );

	strip_junk( text );

	/* printf("STUFF-strip:  %s\n",text);
	fflush(stdout);  */ 

	convert_tags( text );

	/* printf("STUFF-convert:  %s\n",text);
	fflush(stdout);  */ 

	if ( show_blended_colors ) {
	convert_fader(text);  /* added: PhrozenSmoke, <fade and <alt tag support, in fader.c */
						}

	/* printf("STUFF-fader:  %s\n",text);
	fflush(stdout);  */ 

	if ( recv_sound ) {
		sound = recv_sound;
		recv_sound = NULL;

		snprintf( buf, 382, "%s/sounds/%s.wav", GYACH_CFG_DIR, sound );

		free( sound );
		sound = strdup( buf );

		if ( ! stat( buf, &sbuf )) {
#ifdef USE_PTHREAD_CREATE
			pthread_create( &prof_thread, NULL, play_sound, (void *)sound );
#else
			play_sound( (void *)sound );
#endif
		} else {
			snprintf( buf, 382, PACKAGE_DATA_DIR "/sounds/%s.wav", sound );
			if ( ! stat( buf, &sbuf )) {
#ifdef USE_PTHREAD_CREATE
				pthread_create( &prof_thread, NULL, play_sound, (void *)sound );
#else
				play_sound( (void *)sound );
#endif
			}
		}
	}

	if (! textbox ) {
		if ( ct_scroll_at_bottom() ) {scroll = TRUE;}
	} else {
		if (tv_scroll_needed(GTK_TEXT_VIEW(textbox) )) {scroll = TRUE;	}
		if (! strchr(intext,'\n')) {scroll = FALSE;}
	}

	/* Reset all attributes - currently just color */
	r = g = b = 0;
	or = og = ob = 0;

	/* freeze before printing to speed up display, especially on blended */
	/* colors or for people who switch colors every letter */
	if ( ! textbox )
		ct_freeze();

	i = 0;
	while (text[i] && i < strlen(text))
	{
		if (!strncmp(&text[i], YAHOO_FONT_TAG_START,
				strlen(YAHOO_FONT_TAG_START))) {
			int closeFound = FALSE;

			i += strlen(YAHOO_FONT_TAG_START);
			while (!closeFound)
			{
/* changed; PhrozenSmoke - parse font face if we are using Gtk+2 */
				if (( show_fonts ) &&
					( !strncmp(&text[i], YAHOO_FONT_FACE_START,
						strlen(YAHOO_FONT_FACE_START))))
				{
					int k = 0;


					i += strlen(YAHOO_FONT_FACE_START);
					while (text && i < strlen(text) && text[i] != '"')
					{
						my_font_face[k] = text[i];
						++i;
						++k;
						my_font_face[k] = '\0';
						if (k>72) {break;}
					}
					while (text && i < strlen(text)
						&& NULL != strchr("\" \t", text[i]))
					{
						++i;
					}
					if (my_font_face[0] ) {
						char *font_temp;
						font_temp=map_font_family(my_font_face);
						strncpy( face, font_temp, 73); 
						/* printf("FONT HANDLED:  %s  [%s]\n",my_font_face,face);
							    fflush(stdout);  */ 
													   }

				}

				if (!strncmp(&text[i], YAHOO_FONT_SIZE_START,
						strlen(YAHOO_FONT_SIZE_START)))
				{
					int k = 0;

					i += strlen(YAHOO_FONT_SIZE_START);
					while (text && i < strlen(text) && text[i] != '"')
					{
						size[k] = text[i];
						++i;
						++k;
						size[k] = '\0';
						if (k>4) {break;}
					}
					while (text && i < strlen(text)
						&& NULL != strchr("\" \t", text[i]))
					{
						++i;
					}
					if ( ! size[0] ) {
						strncpy( size, "14", 4);
					}
				}

/* changed; PhrozenSmoke - parse font face if we are using Gtk+2 */
/* The variable PARSE_USER_FONTS is unused, why does original author have it here ?
     Added a second time to be more accomodating of fonts specified in reverse order - 
     <font size="12" face="Times"> instead of <font face="Times" size="12"> to 
     help clients that may have specified fonts in a 'non-official' way.  */

				if (( show_fonts ) &&
					( !strncmp(&text[i], YAHOO_FONT_FACE_START,
						strlen(YAHOO_FONT_FACE_START))))
				{
					int k = 0;


					i += strlen(YAHOO_FONT_FACE_START);
					while (text && i < strlen(text) && text[i] != '"')
					{
						my_font_face[k] = text[i];
						++i;
						++k;
						my_font_face[k] = '\0';
						if (k>72) {break;}
					}
					while (text && i < strlen(text)
						&& NULL != strchr("\" \t", text[i]))
					{
						++i;
					}

					if (my_font_face[0] ) {
						char *font_temp;
						font_temp=map_font_family(my_font_face);
						strncpy( face, font_temp, 73); 
						/*  printf("FONT HANDLED:  %s  [%s]\n",my_font_face,face);
							     fflush(stdout);  */
													   }

				}


				if (text && '>' == text[i])
				{
					++i;
					closeFound = TRUE;
					continue;
				}

				i++;
			}

			continue;
#ifdef TRY_WITHOUT_THIS
		} else if ( text[i] == '<' ) {
			/* shouldn't get in here except when user puts a literal < */
			i++;
#endif
		} else if (( text[i] == 033 ) && ( text[i+1] == '|' )) {
			int smlcpylen;
			/* we found one of our converted smileys */
			i += 2;	/* skip over the escape and | */

			tmp_ptr = &text[i];
			tmp_ptr2 = find_whitespace( tmp_ptr );

			smlcpylen=tmp_ptr2 - tmp_ptr;
			if (smlcpylen>62) {smlcpylen=62;}
			strncpy( sm_file, tmp_ptr, smlcpylen );
			sm_file[smlcpylen] = '\0';

			/* printf("sm_file %s\n", sm_file); fflush(stdout); */ 

			tmp_int = atoi( &text[i] );

			i += tmp_ptr2 - tmp_ptr;

			if ( ! ct_can_do_pixmaps()) {
				continue;
			}

			
			if (!strncasecmp("scnt://", sm_file, 7) ) { 
				/* a smiley on the web, smiley central */
				snprintf( buf, 382,   "%s", sm_file );
				ct_append_pixmap( buf ,textbox );
				} else {
			snprintf( buf, 382,   "%s/smileys/%s.gif", GYACH_CFG_DIR, sm_file );

			if ( ! stat( buf, &sbuf )) {
				ct_append_pixmap( buf ,textbox );
			} else {
				snprintf( buf,382,   PACKAGE_DATA_DIR "/smileys/%s.gif", sm_file );
				if ( ! stat( buf, &sbuf )) {
					ct_append_pixmap( buf ,textbox );
					 }
							}
			}

		} else if (( text[i] == 033 ) && ( text[i+1] == '[' )) {
			i += 2;  /* skip over the escape and [ */
			if ( text[i+1] == ';' ) {  /* we have a bold/not bold */
				if ( text[i] == '0' ) {	bold = FALSE;} 
				else {	bold = TRUE;}
				i += 2;
			}

			if ( text[i] == '#' ) { /* added: PhrozenSmoke allow custom colors like '\033[#54C076m' */
				char tmp_int[3]; /* added PhrozenSmoke */
					or = r; og = g; ob = b;
					tmp_int[2] = '\0';
					tmp_int[0] = text[ i + 1 ];
					tmp_int[1] = text[ i + 2 ];
					sscanf( tmp_int, "%x", &r );
					tmp_int[0] = text[ i + 3 ];
					tmp_int[1] = text[ i + 4 ];
					sscanf( tmp_int, "%x", &g );
					tmp_int[0] = text[ i + 5 ];
					tmp_int[1] = text[ i + 6 ];
					sscanf( tmp_int, "%x", &b );
					i +=8;
					continue;
										  }


			else if ( text[i] == 'x' ) {
				/* turn something OFF */
				i++;
				switch ( text[i] ) {
					case '1': bold = FALSE ;
					          break;
					case '2': italic = FALSE;
					          break;
					case '4': underline = FALSE;
					          break;
					case 'l': url = FALSE ;
							  r = or; g = og; b = ob;
							  break;

				}
				i += 2;
				continue;

			} else {
				tmp_color = atoi( &text[i] );
				switch ( tmp_color ) {
					case 0: if (( text[i] == 'l' ) && ( text[i+1] == 'm' )) {
					        	url = TRUE ;
								or = r; og = g; ob = b;
								r = 0; g = 0; b = 0xd9;
							  i += 2;
							  continue;
							} else if ( text[i+1] == 'm' ) {
							  italic = FALSE;
							  bold = FALSE;
							  underline = FALSE;
							  url = FALSE;

							  /* black */
							  or = r; og = g; ob = b;
							  r = 0; g = 0; b = 0;
							  i += 2;
							  continue;
							}
						    break;
					case 1: bold = TRUE ;
							i += 2;
							continue;
					        break;
					case 2: italic = TRUE;
							i += 2;
							continue;
					        break;
					case 4: underline = TRUE;
							i += 2;
							continue;
					        break;
					case 30: /* black */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0; g = 0; b = 0;
							}
							i += 3;
							break;
					case 31: /* blue */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0; g = 0; b = 0xd9;
							}
							i += 3;
						    break;
					case 32: /* cyan */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0; g = 0xb8; b = 0xc8;
							}
							i += 3;
						    break;
					case 33: /* gray */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0x9a; g = 0xa0; b = 0xb3;
							}
							i += 3;
						    break;
					case 34: /* green */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0x4f; g = 0xbc ; b = 0x6f;
							}
							i += 3;
						    break;
					case 35: /* pink */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0xff; g = 0xaf; b = 0xaf;
							}
							i += 3;
						    break;
					case 36: /* purple */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0xb2; g = 0; b = 0xba;
							}
							i += 3;
						    break;
					case 37: /* yellow */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0xdd; g = 0xd9 ; b = 0x27;
							}
							i += 3;
						    break;
					case 38: /* red */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0xc8 ; g = 0x14; b = 0x47;
							}
							i += 3;
						    break;
					case 39: /* olive */
							if ( real_show_colors ) {
								or = r; og = g; ob = b;
								r = 0x70; g = 0x99 ; b = 0x66;
							}
							i += 3;
						    break;
				}
			}
			continue;
		} else if ( text[i] == 033 ) {
			i++;
		}

		if (i >= strlen(text))
		{
			break;
		}

		if ( text[i] == 033 ) {
			continue;
		}

		if ( ! text[i] ) {
			continue;
		}

		/* print out as much as we can */
		tmp_ptr = strchr( &text[i], 033 );
		tmp_ptr2 = strstr( &text[i], "<font" );
		if ( tmp_ptr || tmp_ptr2 ) {
			if (( tmp_ptr ) && ( tmp_ptr2 )) {
				if ( tmp_ptr < tmp_ptr2 ) {
					tmp_color = tmp_ptr - &text[i];
				} else {
					tmp_color = tmp_ptr2 - &text[i];
				}
			} else {
				if ( tmp_ptr ) {
					tmp_color = tmp_ptr - &text[i];
				} else {
					tmp_color = tmp_ptr2 - &text[i];
				}
			}
		} else {
			tmp_color = strlen( &text[i] );
		}

		if ( textbox ) {  /* PMs and other text buffers besides chat window */

			if ( show_fonts ) {

				/* added, PhrozenSmoke: avoid bold/italic related fallbacks, if 
					 a font can't do bold/italics, we will just load 'normal', bold and/or
					 italics will be disabled if they are not supported for this particular 
					font, but this will still allow us to USE this font rather than falling 
					back to a boring 'Sans' font and getting ugly Gtk warnings. */
				
				strncpy( user_font, face, 45 );
				strcat( user_font, ", " );
				/* Check bold/italic capabilities if requested */
				strncat( user_font, get_pango_font_style_string(face, bold, italic), 40);
				strcat( user_font, " " );

				/* added: PhrozenSmoke, set some reasonable font limits to avoid 
					 'billboards' and microscopic text */
				snprintf(size,4, "%d", get_minimum_font_size(face, atoi(size)));

				strncat( user_font, size, 4 );

			} else {
				strncpy( user_font, display_font_name?display_font_name:"Sans 12", 92 );
			}

			if (url) { /* URL underlining in PMs */
				int orgunder=underline;
				underline=TRUE;
				tv_append_text_with_color( GTK_TEXT_VIEW(textbox),
				&text[i], tmp_color, r, g, b, user_font );
				underline=orgunder;
			} else {
				tv_append_text_with_color( GTK_TEXT_VIEW(textbox),
				&text[i], tmp_color, r, g, b, user_font );
					 }

		} else {  /* The Chat window's text buffer */			

			if ( show_fonts ) {

				/* added, PhrozenSmoke: avoid bold/italic related fallbacks, if 
					 a font can't do bold/italics, we will just load 'normal', bold and/or
					 italics will be disabled if they are not supported for this particular 
					font, but this will still allow us to USE this font rather than falling 
					back to a boring 'Sans' font and getting ugly Gtk warnings. */

				strncpy( user_font, face, 45 );
				strcat( user_font, ", " );
				/* Check bold/italic capabilities if requested */
				strncat( user_font, get_pango_font_style_string(face, bold, italic), 40);
				strcat( user_font, " " );
				/* added: PhrozenSmoke, set some reasonable font limits to avoid 
					 'billboards' and microscopic text */
				snprintf(size,4, "%d", get_minimum_font_size(face, atoi(size)));
				strncat( user_font, size, 4 );

				/* printf("got font name:  %s\n", user_font);   fflush(stdout); */ 

				ct_set_font( user_font );
			} else {
				ct_set_font( display_font_name?display_font_name:"Sans 12" );
			}

			tmp_ptr = strstr( text, ":\n" );
			if ( tmp_ptr > ( text + tmp_color )) {
				tmp_ptr = NULL;
			}
			/* attempt to display /exec-ed stuff in fixed width font */
			if (( tmp_ptr != NULL ) &&
				( tmp_ptr != ( text + strlen( text ) - 2 ))) {
				ct_append_fixed( &text[i], tmp_color );
			} else {
				tmp_ptr = strstr( text, "-->\n" );
				if (( tmp_ptr != NULL ) &&
					( tmp_ptr != ( text + strlen( text ) - 4 ))) {
					ct_append_fixed( &text[i], tmp_color );
				} else {
					if ( url || underline ) {ct_set_underline( 1 );}
					ct_set_color( r, g, b );
					ct_append( &text[i], tmp_color );
				}
			}
		}


		i += tmp_color;
	}

	if ( ! textbox )
		ct_thaw();

	if( scroll ) {		
		if (textbox) {
			tv_jump_to_bottom( GTK_TEXT_VIEW(textbox)); 
		}  else {ct_scroll_to_bottom();}
	}		

	// DBG( 22, "RETURN append_to_textbox()\n" );
}

void append_to_textbox_color(GtkWidget *win, GtkWidget *tb, char *text) {
	char tmp[40] = "";
	int old_colors = real_show_colors;
	int old_fonts = show_fonts;
	/* int old_lower = force_lowercase; */

	// DBG( 21, "append_to_textbox_color( %p, %p, '%s' )\n", win, tb, text );

	real_show_colors = 1;
	show_fonts = 1;
	/* force_lowercase = 0; */ 

	/* reset things to normal */
	if ( bold )
		strncpy( tmp, YAHOO_STYLE_BOLDOFF, 32 );
		
	if ( italic )
		strncpy( tmp, YAHOO_STYLE_ITALICOFF, 32 );

	if ( underline )
		strncpy( tmp, YAHOO_STYLE_UNDERLINEOFF, 32 );

	if ( url )
		strncpy( tmp, YAHOO_STYLE_URLOFF, 32 );

	if ( tmp[0] )
		append_to_textbox( win, tb, tmp );

	/* now send our text */
	append_to_textbox( win, tb, text );

	real_show_colors = old_colors;
	show_fonts = old_fonts;
	/* force_lowercase = old_lower; */ 

	// DBG( 22, "RETURN append_to_textbox_color()\n" );
}

/* ------------------------------------------------------------------------ */ 
/* send a chat command 
 * Note: it modifies the string in place 
 * return FALSE if we should quit 
 */ 

void chat_command( char *cmd ) 
{ 
	char *txt = ""; 
	char *args;
	char buf[1280] = "";
	char fnttmp[160] = "";

	sprintf(buf,"%s","");
	sprintf(fnttmp,"%s","");

	// DBG( 21, "chat_command( '%s' )\n", cmd );

		if ( capture_fp ) {	
			time_t time_llnow = time(NULL);
			fprintf(capture_fp,"\n\n[%s]  CHAT COMMAND EXECUTED: \n'%s'\n\n", ctime(&time_llnow), cmd);
			fflush( capture_fp );
			}

	cmd = skip_whitespace( cmd );

	auto_away_time = time(NULL) + ( auto_away * 60 );

	if (!  strncmp(cmd, "s: ", 3)) { handle_y_search_results(chat_window, NULL, cmd); return; }

	switch(cmd[0]) { 
		case '\0': 
			return;
			break; 
		case ':':
			if ( cmd[1] == '!' ) {
				/* emote a ! or !! */
				if ( cmd[2] == '!' ) {
					snprintf( buf, 512, "/execlocal_e %s", cmd+3 );
				} else {
					snprintf( buf, 512,  "/exec_e %s", cmd+2 );
				}
				try_command( buf );
				return;
			} else {
				if ( check_smiley( cmd )) {
					/* it's a smiley */
					txt = cmd;
					subst_escs( txt );
				/* disabled, PhrozenSmoke - this is to fix a bug where 
				   showing a line that starts with 2 smileys that start with ":"
				   caused a segfault 
				   example: sending ":-X :-X" caused a segfault
				   'convert_smileys' was being called TWICE, causing 
				  seg-faults on smileys starting with ":"
				 */

					/*  convert_smileys( txt ); */

				} else {
					/* it's not */
					txt = cmd + 1;
					subst_escs( txt );
					snprintf( buf, 655,  "/emote %s", txt );
					try_command( buf );
					return;
				}
			}

			break;
		case '!':
			if ( cmd[1] == '!' ) {
				snprintf( buf, 512, "/execlocal %s", cmd+2 );
			} else {
				snprintf( buf, 512,  "/exec %s", cmd+1 );
			}
			try_command( buf );
			return;
			break;

		case '/': 
			if ( try_command( cmd )) {return; }

			/* added: PhrozenSmoke, support smileys that start with "/"  */
			/* This patches the 'unknown command' warning seen when using 
				smileys like /:-)   and  /:)  */

			if ( check_smiley( cmd )) { 
					txt = cmd;
					subst_escs( txt );
					break; 
			}

			args = find_whitespace( cmd + 1);
			if ( *args ) {
				*args = '\0';
				args++;
				args = skip_whitespace( args );
			}

			if ( check_alias( cmd + 1, args ))
				return;

			snprintf( buf, 512, "   *** Gyach Enhanced: unknown command '%s' ***\n", cmd );
			append_to_textbox( chat_window, NULL, buf );
			return;
        	break; 
 
		default: 
			if (( cmd[0] == '\\' ) &&
				(( cmd[1] == '/' ) ||
				 ( cmd[1] == ':' ) ||
				 ( cmd[1] == '!' ))) {

			/* added: PhrozenSmoke, support smileys that start with "\"  */
			/*  This patches sometimes mutated, wrong, or incorrectly 
				 displayed smileys, one example was: \:D/   */

				if ( check_smiley( cmd )) { txt = cmd; }
				else { txt = cmd +1; }
			} else {
				txt = cmd; 
			}

			subst_escs( txt );
			break;
    } 

	if ( use_color && 
	  (! gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) ) {
		if ( ! strcmp( use_color, "black" )) {
			strcat( buf, YAHOO_COLOR_BLACK );
		} else if ( ! strcmp( use_color, "blue" )) {
			strcat( buf, YAHOO_COLOR_BLUE );
		} else if ( ! strcmp( use_color, "cyan" )) {
			strcat( buf, YAHOO_COLOR_CYAN );
		} else if ( ! strcmp( use_color, "gray" )) {
			strcat( buf, YAHOO_COLOR_GRAY );
		} else if ( ! strcmp( use_color, "green" )) {
			strcat( buf, YAHOO_COLOR_GREEN );
		} else if ( ! strcmp( use_color, "pink" )) {
			strcat( buf, YAHOO_COLOR_PINK );
		} else if ( ! strcmp( use_color, "purple" )) {
			strcat( buf, YAHOO_COLOR_PURPLE );
		} else if ( ! strcmp( use_color, "yellow" )) {
			strcat( buf, YAHOO_COLOR_YELLOW );
		} else if ( ! strcmp( use_color, "orange" )) {
			strcat( buf, YAHOO_COLOR_ORANGE );
		} else if ( ! strcmp( use_color, "red" )) {
			strcat( buf, YAHOO_COLOR_RED );
		} else if ( ! strcmp( use_color, "custom" )) {
#ifdef TRANS_BG
			if (( trans_bg ) &&
				( custom_color_red > 0xa0 ) &&
				( custom_color_green > 0xa0 ) &&
				( custom_color_blue > 0xa0 )) {
				strcat( buf, YAHOO_COLOR_BLACK );
			} else {
#endif
			snprintf( buf, 18, "\033[#%02x%02x%02xm",
				custom_color_red, custom_color_green, custom_color_blue );
#ifdef TRANS_BG
			}
#endif
		}
	}

	/* changed: PhrozenSmoke - do not append this font tag if we already have 
	     have a <font tag in the text, i.e. we specified on in the 'Send' field - mainly 
	     for developer testing, but useful for people that want to 'hack' in their own 
	     fonts on the fly  */

	if (strstr(cmd,YAHOO_FONT_TAG_START)==NULL) {
		/* only append if 'cmd' contains no 'font' tag */
		snprintf( fnttmp, 135, "%s %s%s\" %s%d\">",
			YAHOO_FONT_TAG_START, YAHOO_FONT_FACE_START, 
			font_family?font_family:"Sans",
			YAHOO_FONT_SIZE_START, font_size );

		strncat( buf, fnttmp, 140 );
																				 }

	if ( use_bold )
		strcat( buf, YAHOO_STYLE_BOLDON );

	if ( use_italics )
		strcat( buf, YAHOO_STYLE_ITALICON );

	if ( use_underline )
		strcat( buf, YAHOO_STYLE_UNDERLINEON );

	if ( filter_command ) {
		strncat( buf, filter_text( txt ), 640);
	} else {
		strncat( buf, txt, 640 );
	}

    if ( *buf ) {
		if( send_avatar ) {
			strcat( buf, AVATAR_START );
			strncat( buf, send_avatar, 35 );
			strcat( buf, AVATAR_SEND_END );
			if ( show_avatars ) {
				display_avatar( ymsg_sess->user, send_avatar );
			}

		}

		/* changed: PhrozenSmoke, to support sending Fader tags */
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(use_chat_fader))) 
			{  
				char fadebuf[1536]="";
				char *fade1=NULL;
				char *fade2=NULL;
			 /* fader use requested */
			if (strlen(gtk_entry_get_text(GTK_ENTRY(fader_text_start)))>0) {
				fade1=strdup(gtk_entry_get_text(GTK_ENTRY(fader_text_start))) ;
				strncat(fadebuf,fade1, 180);
																													 }
			strncat(fadebuf,buf, 1280);
			if (strlen(gtk_entry_get_text(GTK_ENTRY(fader_text_end)))>0) {
				fade2=strdup(gtk_entry_get_text(GTK_ENTRY(fader_text_end)));
				strncat(fadebuf,fade2, 25 );
																													 }
			gyach_comment( ymsg_sess, fadebuf);
			if (fade1 != NULL) {free(fade1);}
			if (fade2 != NULL) {free(fade2);}

			} else  {  /* send with no fader */
				strcat(buf," "); /* forces showing of smileys */
				gyach_comment( ymsg_sess, buf );
						}
	}
} 
 

/* changed, PhrozenSmoke, all packet handling is in its own module, packet_handler.c */
/* show_yahoo_packet has moved to packet_handler.c */

void show_yahoo_packet() {
	handle_yahoo_packet();
}


/* for testing - IGNORE */ /* 
static PM_SESSION *pm_sess3;

void show_test_pm() {	
	pm_sess3=new_pm_session(strdup("TestUser"));
	gtk_widget_show_all( pm_sess3->pm_window );
}   */
 

void SIGPIPE_Handler (int signo) {
   if (signo == SIGPIPE) {	
   handle_dead_connection (1,0, 0, _("You have been disconnected from Yahoo!: Broken pipe."));
	}
}

void ymsg_session_reset() {
	/* intiate and clear new session */ 
	ymsg_sess->sock = -1; 
	ymsg_sess->suppress_dup_packets = 0;
	ymsg_sess->session_id = -1;
	ymsg_sess->quit = 0;
	ymsg_sess->debug_packets = 0;
	ymsg_sess->proxy_port = 80;
	ymsg_sess->port = YMSG_CHAT_PORT;

	/* reset cookies and strings */
	ymsg_sess->cookie[0] = '\0';
	ymsg_sess->user[0] = '\0';
	ymsg_sess->password[0] = '\0';
	ymsg_sess->room[0] = '\0';
	ymsg_sess->req_room[0] = '\0';
	ymsg_sess->host[0] = '\0';
	ymsg_sess->proxy_host[0] = '\0';
	ymsg_sess->error_msg[0] = '\0';
}



int main( int argc, char **argv ) {
	struct stat gtkrc_stat;
	char buf[256]="";
/* #ifndef OS_WINDOWS  */
	struct timeval tv;
	fd_set set;
	int ret;
/* #endif  */


	memset( ymsg_sess ,0 ,sizeof(YMSG_SESSION)); 
	ymsg_session_reset();  /* very important */ 

	/* Above: Changed, PhrozenSmoke...suppression of 
	consecutive duplicate packets is now the job of the bootprevent.c
	module...because turning off consecutive duplicate packets 
	unnecessarily means that we can miss things like PMs and
	chat comments if another users says the same thing twice...
	This has created problems for me, where people were answering 
	me (typing the same response over and over), and I wasn't 
	getting their messages after the first time because their 
	innocent response was treated as a 'duplicate' packet */

	setlocale(LC_ALL, "");
	bindtextdomain (PACKAGE, LOCALEDIR);  
	 bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain (PACKAGE);

	gtk_set_locale();

	gtk_init( &argc, &argv );

	gyach_init();

	/* an app-specific theme can be stored in ~/.yahoorc/gyach/gtkrc  */  
	snprintf( buf, 254,  "%s/gtkrc", GYACH_CFG_DIR );
	if ( ! stat( buf, &gtkrc_stat )) {gtk_rc_parse( buf );}

	chat_window = build_chat_window();
	set_text_entry_styles();

	/* gtk_widget_show_all( chat_window ); */ /* this is already called in gyach_int.c */

	if (chat_entry) {gtk_widget_grab_focus(GTK_WIDGET(chat_entry));}

  if (signal(SIGPIPE, SIGPIPE_Handler) == SIG_ERR) {
	snprintf(buf, 190, "\n  %s** %s ** %s\n\n",YAHOO_COLOR_BLUE,  "WARNING: The SIGPIPE signal cannot be caught. This application may freeze or crash if the connection to Yahoo! dies unexpectedly.", YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, buf );
  }

	login_window = build_login_window();

	if ( ! auto_login ) {
		gtk_widget_show_all( login_window );
	} else {
		login_to_yahoo_chat();
	}

	trayicon_create();

	/* added: PhrozenSmoke */
	start_alias_timeout();


	/* basic welcome message, PhrozenSmoke */
	snprintf(buf, 254, "%s%sGyach Enhanced %s \n%s%s%s\n", YAHOO_STYLE_ITALICON, "\033[#8F4CB1m",VERSION, GYACH_URL, YAHOO_STYLE_ITALICOFF , YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, buf );

#ifdef SUPPORT_SOUND_EVENTS
	snprintf(buf, 254, "%sSound event support (via ESound) is available in this version using PyESound 0.1, Copyright (c) 2003-2006 Erica Andrews.\n\n%s", "\033[#73B7DBm",  YAHOO_COLOR_BLACK);
	append_to_textbox_color( chat_window, NULL, buf );
#endif

	
	/* FOR TESTING  */  /*        
	add_friend(strdup("PhrozenSmoke"));
	set_screenname_alias( strdup("phrozensmoke"), strdup("Phrozen Smoke"));
	chatter_list_add(strdup("phrozensmoke"));
	chatter_list_add(strdup("vjrvnsjk_vnbqljhl"));  
	set_buddy_status(strdup("phrozensmoke"),strdup("hey there"));
	add_online_friend(strdup("phrozensmoke"));
	add_online_friend(strdup("Smoke2"));
	set_buddy_status(strdup("Smoke2"),strdup("hey there"));
	update_buddy_clist();  
	set_menu_connected(1);      */

	/* FOR TESTING  */ /* spam bot muting tests */   /*   
   printf("%s:  %d\n\n", "tatiana_jmper_grl2002", is_spam_name("tatiana_jmper_grl2002")); fflush(stdout); 
   printf("%s:  %d\n\n", "vjrvnsjk_vnbqljhl", is_spam_name("vjrvnsjk_vnbqljhl")); fflush(stdout); 
   printf("%s:  %d\n\n",  "dionne_ymawpdcx965", is_spam_name("dionne_ymawpdcx965"));  fflush(stdout);  
   printf("%s:  %d\n\n", "bethany_larajpnw283", is_spam_name("bethany_larajpnw283")); fflush(stdout);  

 chatter_list_populate(strdup("tatiana_jmper_grl2002,vjrvnsjk_vnbqljhl,dmuteqno_qfwfwnku,gynlxhvw_mxhiivwh,wolf_6767,shah_sheh_sheh_eg,uhfljlbl_eifaycns29,hankyhanky,terr_card,PhrozenSmoke,evbptufd_tbaldadi,lkyjnbjb_cwhkfgch"),1);    */

	gdk_threads_enter();	
	load_plugin_modules( );


/* Re-enabled regular socket polling - PhrozenSmoke:
    traced several memory leaks to the GDK 'input_add' thing...this works
    just as well and seems to use less memory */


/*  
#ifndef OS_WINDOWS
*/


	while( ! ymsg_sess->quit ) {
		while( gtk_events_pending()) {
			gtk_main_iteration();
		}

		/* Check for a dead login attempt that just hangs */
		check_connection_timedout();

		/* check for input on any YCHT sockets */
		if ( ycht_is_running() ) {
			FD_ZERO( &set );
			ret=0;
			FD_SET( ycht_sock, &set );
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			ret = select( ycht_sock+ 1, &set, NULL, NULL, &tv );
			if (ret) {
			if (  ycht_recv_data( )) {
				handle_ycht_packet();
				if (yalias_name) {g_free(yalias_name); yalias_name=NULL;}
				if (mygconvedstr) {g_free(mygconvedstr); mygconvedstr=NULL;}
				}
			}
		}

		/* now check for input on our socket */
		FD_ZERO( &set );
		ret=0;
		if ( ymsg_sess->sock != -1 ) {
			FD_SET( ymsg_sess->sock, &set );
		}

			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			ret = select( ymsg_sess->sock + 1, &set, NULL, NULL, &tv );


		if ( ret ) {
			if ( ymsg_recv_data( ymsg_sess )) {
				show_yahoo_packet();
				if (yalias_name) {g_free(yalias_name); yalias_name=NULL;}
				if (mygconvedstr) {g_free(mygconvedstr); mygconvedstr=NULL;}

				/* set auto-away if needed */
				/* added to main event loop, moved from packet_handler.c; PhrozenSmoke */
				if (( ! my_status ) &&
					( auto_away ) &&
					( time(NULL) > auto_away_time )) {
						/* fixed: PhrozenSmoke, only do this if we aren't invisible at the time */
					if (my_status != 12) { cmd_away( "11" );  }
				}

				usleep(sock_sleep);   /* flow control, added PhrozenSmoke */
			}
		}
	}


/* 
#else
	gtk_main();
#endif
*/

	return( 0 );
}

