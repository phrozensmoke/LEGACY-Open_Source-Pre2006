/*****************************************************************************
 * fader.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gytreeview.h"

char url_data[10]="";
char last_col[8]="";
char num_to_col[8]="";

GdkColor fadelist_color;
GtkTreeIter fadealt_row_selected;
GtkTreeView *colorlist=NULL;
GtkWidget *fadeconfigwin=NULL;
GtkWidget *fader_style1=NULL;
int fader_conf_using_fader=1;
int is_inserting_fade_col=0;
int total_fader_colors_config=0;
GtkWidget *fader_colorsel=NULL;
GtkWidget *fader_sampler_textb=NULL;
GtkWidget *fader_config_tag_entry=NULL;
GtkTextBuffer  *fader_sampler_textbuff=NULL;
char conffader_start_holder[65]="";
char conffader_end_holder[10]="";
char conffader_sample_holder[132]="";
char *conffader_sample_str="<font face=\"Arial\" size=\"14\">Gyach Enhanced for Linux";
extern GtkWidget *fader_text_start;
extern GtkWidget *fader_text_end;
extern GtkWidget *use_chat_fader;
extern GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id);
extern char *_(char *some);  /* added PhrozenSmoke: locale support */ 
extern void set_tooltip(GtkWidget *somewid, char *somechar);
extern void append_to_textbox_color(GtkWidget * win, GtkWidget *tb, char *text);
extern GtkWidget *get_pm_icon(int which_icon, char *err_str) ;
extern void write_config();

extern gchar *_utf(char *some);
extern gchar *_b2loc(char *some);

/* PhrozenSmoke: an adapted version of the doRainbow() 
     method from the rainbow.c 
     module from the Everybuddy/Ayttm project (GPL) - used 
     for basic <ALT tag support, right now it only supports ALT 
     for 2 colors - no more ...ALT tags are where the letters 
     alternate colors  */


/* a list of chars we have problems with when placed near terminal color escape sequences */
char danger_chars[11]={'\0',  ':','x','o','m','1','2','3','4','#',    '\0'};


		/* hold up to 12 colors */
char *fade_col_collect[] = {
			NULL, NULL,NULL,NULL,NULL,  NULL, NULL,NULL,NULL,NULL,NULL,NULL,
			NULL,NULL,NULL
								};

void clear_fade_colors() {
	int colii=0;
	char *mynone=NULL;
	while( colii<13 ) {
		if (fade_col_collect[colii]) {
			free(fade_col_collect[colii]);
			fade_col_collect[colii]=mynone;
			  								}
			colii++;
						}
}

int count_fade_colors() {
	char **cptr = fade_col_collect;
	int colii=0;
	while( *cptr ) {
			colii++;
			cptr++;
					}
return colii;
}


char *get_num_to_color(int red, int green,int blue) {
	int rr=0;
	int gg=0;
	int bb=0;
	char *colors=num_to_col;
	if ( (red<256) && (red>-1) ) {rr=red;}
	if ( (green<256) && (green>-1) ) {gg=green;}
	if ( (blue<256) && (blue>-1) ) {bb=blue;}
	snprintf(colors,7,"%02x%02x%02x",rr,gg,bb);	
	return colors;
}


/* 
  check to see if this is a char we should NOT try to fade/alt
  This can help avoid unreadable text and 'pango' text layout warnings
   now DEPRECATED */  /* 
int is_danger_char(char dd) {
	int ij=0;
	if (dd=='\0') {return 0;}
	while (1) {
		if (danger_chars[ij]=='\0') {return 0;}
		if (danger_chars[ij]==dd) {return 1;}
		ij++;
			}
return 0;
								}
*/

int is_danger_char(char dd) {return 0;}


char *do_alt_text(char *from_col, char *to_col, const char *s)
{
	char *retval;
	char *wptr;
	int pos = 0;
	char *colorsg[2]={from_col,to_col};
	char **colors;
	int colnum=0;
	int len = strlen(s);
	int url_count=0;
	int is_url=0;
	int is_tag=0;  /* for escape tags like \033|21.gif  */
	int is_smiley=0;
	int fade_block=0;  
	int fade_block_counter=0;
	int dangerous=0;

	int max_rowscolors=count_fade_colors();

	/* printf("fade count: %d\n", max_rowscolors); fflush(stdout); */

	if (max_rowscolors>0) {colors=fade_col_collect;}
	else {max_rowscolors=2; colors=colorsg;}


	/* used to facilitate speed for longer texts - on long texts (over 50 bytes) GTK will take forever to render 
	     the text if we truly 'fade' each character, so we will fade 'blocks' of characters based on the size of 
	     's' - the longer 's'...the longer the 'blocks' will be to cut down on the amount of rendering GTK 
	     will have to do for all the color changes - so the fade won't look as 'gradual' as we would like, 
	     but we get a decent 'fade' effect without slowing down the system too much  */
	if (strlen(s)>30) {fade_block=1;}
	if (strlen(s)>70) {fade_block=2;}
	if (strlen(s)> 100) {fade_block=3;}
	if (strlen(s)>130) {fade_block=4;}
	if (strlen(s)>160) {fade_block=5;}
	if (strlen(s)>185) {fade_block=6;}
	if (strlen(s)>240) {fade_block=7;}
	if (strlen(s)>275) {fade_block=8;}
	if (strlen(s)>330) {fade_block=9;}
	if (strlen(s)>370) {fade_block=10;}
	if (strlen(s)>410) {fade_block=11;}
	if (strlen(s)>450) {fade_block=12;}
	if (strlen(s)>495) {fade_block=13;}
	if (strlen(s)>540) {fade_block=14;}


	if (strlen(s)<1) {return strdup(" ");}

	wptr = retval = malloc(15 * len * sizeof(char));

	   /* printf("STRING LEN:  %d\n",strlen(s));
	     printf("STRING:  %s\n\n",s);
	     fflush(stdout);  */

	while (s[pos] != '\0') {
		dangerous=0;
		/* check for URL, we dont want to break up a URL with ALT data, then it wont 
			 get underlined, plus an ALT on a URL is a waste, since URLS are always blue */
		is_url=0;
		url_count=pos;
		is_tag=0;

		if (colnum==max_rowscolors) {colnum=0;}

		/*check for converted 'smileys'...like \033|filename.gif, also other \033
			escape tags  */
		if (( s[pos] == 033 ) ) {is_tag=1;}

		if (is_tag) {
			is_smiley=0;
			if (s[pos+1]=='|') {is_smiley=1;}
			/* escape tags always end with either 'm' or 'g', example: 
				 "\033[30m"  or "\033|21.gif", so we will simply copy 
				 the chars to the buffer until we hit a 'm', 'g', ' ', or '\0'
			*/
				while (1)  {  /* keeping going til we hit a 'm', 'g', ' ', or '\0' */
					if (s[url_count]==' ') {break;}
					if (s[url_count]=='\0') {break;}
					if (!is_smiley) {
						if (s[url_count]=='m') {url_count++; break;}
											}
					/* found '.gif' ending */
					if (is_smiley) {
						if ((s[url_count]=='i') && (s[url_count+1]=='f')) {
							url_count++; url_count++; break;}
										 }
					url_count++;
								 }

				while (pos<url_count) { /* append raw SMILEY or other \033 tag */
					wptr += snprintf(wptr, 3,"%c",s[pos]);
					pos++;
													 }

				if (is_smiley) {wptr += snprintf(wptr, 3,"%c",' ');} /* smileys dont show without it */
				else {
					wptr +=
		    			snprintf(wptr, 18,
			      		 "\033[#%sm%c", colors[colnum], ' ');
						if (is_danger_char(s[pos-1])) {wptr += snprintf(wptr,3,"%c",' ');}
						if (colnum==max_rowscolors) {colnum=0;}
						}
				continue;				

						  } /* if is_tag */


		if ( s[pos]=='h' || s[pos]=='H') {
			if ( (len-pos) >=7) {  /* still at least 7 chars left, enough for an http:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				url_data[6]=s[pos+6];
				if ( (len-pos) >=8) {
					url_data[7]=s[pos+7];
					if ( !strncasecmp( url_data, "https://", 8 )) {is_url=1;}
				}
				if ( !strncasecmp( url_data, "http://", 7 )) {is_url=1;}
										   }
														   }
		if ( s[pos]=='f' || s[pos]=='F') {
			if ( (len-pos) >=6) {  /* still at least 6 chars left, enough for an ftp:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				if ( !strncasecmp( url_data, "ftp://", 6 )) {is_url=1;}
										   }
													 	   }


		/* we can treat URLs as normal, but simply writing raw (unALTed) 
			 chars until we reach a ' ' or a '\0 */

		if (is_url)  {
					wptr +=
		    			snprintf(wptr, 18,
			      		 "\033[#%sm", "0000dd");

				while (1)  {  /* keeping going til we reach a '\0' or ' ' */
					if (s[url_count]==' ') {break;}
					if (s[url_count]=='\0') {break;}
					url_count++;
								 }
				while (pos<url_count) { /* append raw URL  */
					wptr += snprintf(wptr, 3,"%c",s[pos]);
					pos++;
													 }

		wptr +=
		    snprintf(wptr, 18,
			       "\033[#%sm%c", colors[colnum], ' ');
		colnum++; 
		if (colnum==max_rowscolors) {colnum=0;}

				continue;					
						 }				

		/* for normal text, fade it ! */
		dangerous=is_danger_char(s[pos]); 
		if (!dangerous) { if (pos>0) {dangerous=is_danger_char(s[pos-1]); } }
		if (!dangerous) { if (pos>0) {dangerous=is_danger_char(s[pos+1]); } }

		if (!dangerous) {
		wptr +=
		    snprintf(wptr, 18,
			       "\033[#%sm%c", colors[colnum], s[pos]);
		if (s[pos] != ' ') { colnum++;  }
		if (colnum==max_rowscolors) {colnum=0;}
		pos++;
							  } else  {
				wptr +=  snprintf(wptr, 5,"%c", s[pos]);
				colnum++; 
				if (colnum==max_rowscolors) {colnum=0;}
				pos++;
				continue; 
										}

		/* Gtk gets REALLY slow when we change color on every single letter, best we can do is 
			 every 2 if we don't want to sacrifice speed too much - anybody know a fix? */

		fade_block_counter=0;
		while (fade_block_counter<fade_block) {
			dangerous=0;
			if (s[pos] != '\0') { 

				if (( s[pos] == 033 ) ) { break; }  /* skip escapes and smileys */

				/* skip URLs */

		if ( s[pos]=='h' || s[pos]=='H') {
			if ( (len-pos) >=7) {  /* still at least 7 chars left, enough for an http:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				url_data[6]=s[pos+6];
				if ( (len-pos) >=8) {
					url_data[7]=s[pos+7];
					if ( !strncasecmp( url_data, "https://", 8 )) {break;}
				}
				if ( !strncasecmp( url_data, "http://", 7 )) {break;}
										   }
														   }
		if ( s[pos]=='f' || s[pos]=='F') {
			if ( (len-pos) >=6) {  /* still at least 6 chars left, enough for an ftp:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				if ( !strncasecmp( url_data, "ftp://", 6 )) {break;}
										   }
														   }


				if (pos > 0)  {  /* fixes truncated lines with 'o', 'm', ':', 'x' chars */
					wptr +=snprintf(wptr, 6,"%c",s[pos]); 		
									} else {wptr +=snprintf(wptr, 6,"%c",s[pos]); }

				pos++;
										} /* end if null byte */
			fade_block_counter++;
																		}

						 }  /* end while */

	return retval;

}



/* PhrozenSmoke: an adapted version of the doRainbow() 
     method from the rainbow.c 
     module from the Everybuddy/Ayttm project (GPL)  - 
     used for basic <FADE tag support, right now, it only supports up to 
     the first 12 colors of a FADE tag - that's probably enough  */

char *do_fader_text(char *from_col, char *to_col, const char *s)
{
	char *retval;
	char *wptr;
	int pos = 0;
	unsigned  int start_r, start_g, start_b, end_r, end_g, end_b;
	int len = strlen(s);
	char tmp_int[3];
	int url_count=0;
	int is_url=0;
	int is_tag=0;  /* for escape tags like \033|21.gif  */
	int is_smiley=0;
	int fade_block=0;  
	int fade_block_counter=0;
	int dangerous=0;
	int max_colors=2;
	int allow_switch=0;
	int color_pos=0;
	int col_arr=0; 
	char *tocol=NULL;
	char *frcol=NULL;
	char **cptr = fade_col_collect;
	int col_arr_check=(int) (((int) strlen(s)/(int) (1.0) ) * 1.0);

	if (strlen(s)<1) {clear_fade_colors(); return strdup(" ");}

	max_colors=count_fade_colors();

	/* printf("max: %d \n", max_colors);
	fflush(stdout); */

	if (max_colors>1) {
		col_arr_check=(int) (((int) strlen(s)/(int) (max_colors) ) * 1.0);
		/* printf("float: %d \n", col_arr_check);
		fflush(stdout);  */
		if (strlen(s) >= max_colors) {allow_switch=1;}
						  }

	/* DO NOT REMOVE line below, avoids division by zero errors */
	if (col_arr_check<1) {col_arr_check=1;}


	/* printf("allow: %d \n", allow_switch);
	fflush(stdout); */

	/* used to facilitate speed for longer texts - on long texts (over 50 bytes) GTK will take forever to render 
	     the text if we truly 'fade' each character, so we will fade 'blocks' of characters based on the size of 
	     's' - the longer 's'...the longer the 'blocks' will be to cut down on the amount of rendering GTK 
	     will have to do for all the color changes - so the fade won't look as 'gradual' as we would like, 
	     but we get a decent 'fade' effect without slowing down the system too much  */
	if (strlen(s)>20) {fade_block=1;}
	if (strlen(s)>30) {fade_block=2;}
	if (strlen(s)>45) {fade_block=3;}
	if (strlen(s)> 60) {fade_block=4;}
	if (strlen(s)>75) {fade_block=5;}
	if (strlen(s)>95) {fade_block=6;}
	if (strlen(s)>115) {fade_block=7;}
	if (strlen(s)>135) {fade_block=8;}
	if (strlen(s)>165) {fade_block=9;}
	if (strlen(s)>185) {fade_block=10;}
	if (strlen(s)>210) {fade_block=11;}
	if (strlen(s)>235) {fade_block=12;}
	if (strlen(s)>260) {fade_block=13;}
	if (strlen(s)>290) {fade_block=14;}
	if (strlen(s)>315) {fade_block=15;}
	if (strlen(s)>340) {fade_block=16;}
	if (strlen(s)>375) {fade_block=18;}
	if (strlen(s)>400) {fade_block=19;}
	if (strlen(s)>425) {fade_block=20;}
	if (strlen(s)>450) {fade_block=22;}
	if (strlen(s)>475) {fade_block=23;}
	if (strlen(s)>500) {fade_block=25;}
	if (strlen(s)>525) {fade_block=26;}
	if (strlen(s)>550) {fade_block=27;}
	if (strlen(s)>575) {fade_block=28;}
	if (strlen(s)>600) {fade_block=29;}


	/* make some ints from from_col */
	if (! allow_switch) {frcol=from_col;}
	else {  /* more that 2 fade colors */
		frcol=*cptr;
		cptr++;
		col_arr++;
		}
	tmp_int[2] = '\0';
	tmp_int[0] = frcol[ 0 ];
	tmp_int[1] = frcol[ 1 ];
	sscanf( tmp_int, "%x", &start_r );
	tmp_int[0] = frcol[ 2 ];
	tmp_int[1] = frcol[ 3 ];
	sscanf( tmp_int, "%x", &start_g );
	tmp_int[0] = frcol[ 4 ];
	tmp_int[1] = frcol[ 5 ];
	sscanf( tmp_int, "%x", &start_b );


	/* make some ints from to_col */
	if (! allow_switch) {tocol=to_col;}
	else {  /* more that 2 fade colors */
		tocol= fade_col_collect[1];
		//tocol=*cptr;
		//cptr++;
		//col_arr++;
		}

	tmp_int[2] = '\0';
	tmp_int[0] = tocol[ 0 ];
	tmp_int[1] = tocol[ 1 ];
	sscanf( tmp_int, "%x", &end_r );
	tmp_int[0] = tocol[ 2 ];
	tmp_int[1] = tocol[ 3 ];
	sscanf( tmp_int, "%x", &end_g );
	tmp_int[0] = tocol[4 ];
	tmp_int[1] = tocol[5 ];
	sscanf( tmp_int, "%x", &end_b );

	/* printf("cols: %s   %s \n", frcol, tocol);
	fflush(stdout); */


	if (start_r > 255 || start_r < 0) {start_r = 0;	}
	if (start_g > 255 || start_g < 0) {	start_g = 0;}
	if (start_b > 255 || start_b < 0) {start_b = 0;	}
	if (end_r > 255 || end_r < 0) {	end_r = 0;}
	if (end_g > 255 || end_g < 0) {	end_g = 0;	}
	if (end_b > 255 || end_b < 0) {	end_b = 0;	}

	

	wptr = retval = malloc(15 * len * sizeof(char));

	/* printf("STRING LEN:  %d\n",strlen(s));
	     printf("STRING:  %s\n\n",s);
	     fflush(stdout);  */

	while (s[pos] != '\0') {
		dangerous=0;
		/* check for URL, we dont want to break up a URL with fade data, then it wont 
			 get underlined, plus a fade on a URL is a waste, since URLS are always blue */
		is_url=0;
		url_count=pos;
		is_tag=0;


if (allow_switch) {
	if (col_arr<=max_colors) {
	if (color_pos>=col_arr_check) {
	  if (*cptr) {
		frcol=*cptr;
		//frcol=last_col;
		col_arr++;
		cptr++;
				}
	  if (*cptr) {
		tocol=*cptr;
				} else {tocol=frcol;}
	 color_pos=0;
	tmp_int[2] = '\0';
	tmp_int[0] = frcol[ 0 ];
	tmp_int[1] = frcol[ 1 ];
	sscanf( tmp_int, "%x", &start_r );
	tmp_int[0] = frcol[ 2 ];
	tmp_int[1] = frcol[ 3 ];
	sscanf( tmp_int, "%x", &start_g );
	tmp_int[0] = frcol[ 4 ];
	tmp_int[1] = frcol[ 5 ];
	sscanf( tmp_int, "%x", &start_b );
	tmp_int[2] = '\0';
	tmp_int[0] = tocol[ 0 ];
	tmp_int[1] = tocol[ 1 ];
	sscanf( tmp_int, "%x", &end_r );
	tmp_int[0] = tocol[ 2 ];
	tmp_int[1] = tocol[ 3 ];
	sscanf( tmp_int, "%x", &end_g );
	tmp_int[0] = tocol[4 ];
	tmp_int[1] = tocol[5 ];
	sscanf( tmp_int, "%x", &end_b );
	if (start_r > 255 || start_r < 0) {start_r = 0;	}
	if (start_g > 255 || start_g < 0) {	start_g = 0;}
	if (start_b > 255 || start_b < 0) {start_b = 0;	}
	if (end_r > 255 || end_r < 0) {	end_r = 0;}
	if (end_g > 255 || end_g < 0) {	end_g = 0;	}
	if (end_b > 255 || end_b < 0) {	end_b = 0;	}

	/* printf("swith-1 cols: %s   %s \n", frcol, tocol);
	fflush(stdout); */
											}
									  }
				} /* allow_switch */


		/*check for converted 'smileys'...like \033|filename.gif, also other \033
			escape tags  */
		if (( s[pos] == 033 ) ) {is_tag=1;}
		

		if (is_tag) {
			is_smiley=0;
			if (s[pos+1]=='|') {is_smiley=1;}
			/* escape tags always end with either 'm' or 'g', example: 
				 "\033[30m"  or "\033|21.gif", so we will simply copy 
				 the chars to the buffer until we hit a 'm', 'g', ' ', or '\0'
			*/
				while (1)  {  /* keeping going til we hit a 'm', 'g', ' ', or '\0' */
					if (s[url_count]==' ') {break;}
					if (s[url_count]=='\0') {break;}
					if (!is_smiley) {
						if (s[url_count]=='m') {url_count++; break;}
											}
					/* found '.gif' ending */
					if (is_smiley) {
						if ((s[url_count]=='i') && (s[url_count+1]=='f')) {
							url_count++; url_count++; break;}
										 }
					url_count++;
								 }

				while (pos<url_count) { /* append raw SMILEY or other \033 tag */

					wptr += snprintf(wptr, 2,"%c",s[pos]);
					pos++;
					/* color_pos++; */  /* fader fix! */
													 }

				if (is_smiley) {wptr += snprintf(wptr, 3,"%c",' ');} /* smileys dont show without it */

				else {
						snprintf(last_col, 7,"%s", get_num_to_color(
			      		 (color_pos * end_r + (col_arr_check - color_pos) * start_r) / col_arr_check,
			      		 (color_pos * end_g + (col_arr_check - color_pos) * start_g) / col_arr_check,
			      		 (color_pos * end_b + (col_arr_check - color_pos) * start_b) / col_arr_check)  );

					wptr +=  snprintf(wptr, 17, "\033[#%sm%c", last_col, ' ');

						if (is_danger_char(s[pos-1])) {wptr += snprintf(wptr, 2,"%c",' ');}
						}

				continue;				

						  } /* if is_tag */


		if ( s[pos]=='h' || s[pos]=='H') {
			if ( (len-pos) >=7) {  /* still at least 7 chars left, enough for an http:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				url_data[6]=s[pos+6];
				if ( (len-pos) >=8) {
					url_data[7]=s[pos+7];
					if ( !strncasecmp( url_data, "https://", 8 )) {is_url=1;}
				}
				if ( !strncasecmp( url_data, "http://", 7 )) {is_url=1;}
										   }
														   }
		if ( s[pos]=='f' || s[pos]=='F') {
			if ( (len-pos) >=6) {  /* still at least 6 chars left, enough for an ftp:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				if ( !strncasecmp( url_data, "ftp://", 6 )) {is_url=1;}
										   }
														   }


		/* we can treat URLs  as normal, but simply writing raw (unfaded) 
			 chars until we reach a ' ' or a '\0 */

		if (is_url)  {

					wptr +=	snprintf(wptr, 17, "\033[#%sm", "0000dd");

				while (1)  {  /* keeping going til we reach a '\0' or ' ' */
					if (s[url_count]==' ') {break;}
					if (s[url_count]=='\0') {break;}
					url_count++;
								 }
				while (pos<url_count) { /* append raw URL  */
					wptr += snprintf(wptr, 2,"%c",s[pos]);
					pos++;
					/* color_pos++; */  /* fader fix! */
													 }

						snprintf(last_col, 7,"%s", get_num_to_color(
			      		 (color_pos * end_r + (col_arr_check - color_pos) * start_r) / col_arr_check,
			      		 (color_pos * end_g + (col_arr_check - color_pos) * start_g) / col_arr_check,
			      		 (color_pos * end_b + (col_arr_check - color_pos) * start_b) / col_arr_check)  );

					wptr +=  snprintf(wptr, 17, "\033[#%sm%c", last_col, ' ');

				continue;					
						 }

		/* for normal text, fade it ! */
					dangerous=is_danger_char(s[pos]); 
					if (!dangerous) {dangerous=is_danger_char(s[pos-1]); } 
					if (!dangerous) { dangerous=is_danger_char(s[pos+1]);  }


		if (dangerous)  {
		wptr +=    snprintf(wptr, 5,   "%c", s[pos]);
		}  else  {
						snprintf(last_col, 7,"%s", get_num_to_color(
			      		 (color_pos * end_r + (col_arr_check - color_pos) * start_r) / col_arr_check,
			      		 (color_pos * end_g + (col_arr_check - color_pos) * start_g) / col_arr_check,
			      		 (color_pos * end_b + (col_arr_check - color_pos) * start_b) / col_arr_check)  );

					wptr +=  snprintf(wptr, 17, "\033[#%sm%c", last_col, s[pos]);

				   }
		pos++;
		color_pos++;

		/* Gtk gets REALLY slow when we change color on every single letter, best we can do is 
			 every 2-9 if we don't want to sacrifice speed too much - anybody know a fix? */

		fade_block_counter=0;
		while (fade_block_counter<fade_block) {


if (allow_switch) {
	if (col_arr<=max_colors) {
	if (color_pos>=col_arr_check) {
	  if (*cptr) {
		frcol=*cptr;
		//frcol=last_col;
		col_arr++;
		cptr++;
				}
	  if (*cptr) {
		tocol=*cptr;
				} else {tocol=frcol;}
	 color_pos=0;
	tmp_int[2] = '\0';
	tmp_int[0] = frcol[ 0 ];
	tmp_int[1] = frcol[ 1 ];
	sscanf( tmp_int, "%x", &start_r );
	tmp_int[0] = frcol[ 2 ];
	tmp_int[1] = frcol[ 3 ];
	sscanf( tmp_int, "%x", &start_g );
	tmp_int[0] = frcol[ 4 ];
	tmp_int[1] = frcol[ 5 ];
	sscanf( tmp_int, "%x", &start_b );
	tmp_int[2] = '\0';
	tmp_int[0] = tocol[ 0 ];
	tmp_int[1] = tocol[ 1 ];
	sscanf( tmp_int, "%x", &end_r );
	tmp_int[0] = tocol[ 2 ];
	tmp_int[1] = tocol[ 3 ];
	sscanf( tmp_int, "%x", &end_g );
	tmp_int[0] = tocol[4 ];
	tmp_int[1] = tocol[5 ];
	sscanf( tmp_int, "%x", &end_b );
	if (start_r > 255 || start_r < 0) {start_r = 0;	}
	if (start_g > 255 || start_g < 0) {	start_g = 0;}
	if (start_b > 255 || start_b < 0) {start_b = 0;	}
	if (end_r > 255 || end_r < 0) {	end_r = 0;}
	if (end_g > 255 || end_g < 0) {	end_g = 0;	}
	if (end_b > 255 || end_b < 0) {	end_b = 0;	}

	/* printf("swith-2 cols: %s   %s \n", frcol, tocol);
	fflush(stdout); */
											}
									  }
				} /* allow_switch */




			dangerous=0;
			if (s[pos] != '\0') { 

				if (( s[pos] == 033 ) ) { break; }  /* skip escapes and smileys */

				/* skip URLs */

		if ( s[pos]=='h' || s[pos]=='H') {
			if ( (len-pos) >=7) {  /* still at least 7 chars left, enough for an http:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				url_data[6]=s[pos+6];
				if ( (len-pos) >=8) {
					url_data[7]=s[pos+7];
					if ( !strncasecmp( url_data, "https://", 8 )) {break;}
				}
				if ( !strncasecmp( url_data, "http://", 7 )) {break;}
										   }
														   }
		if ( s[pos]=='f' || s[pos]=='F') {
			if ( (len-pos) >=6) {  /* still at least 6 chars left, enough for an ftp:// */
				url_data[0]=s[pos];
				url_data[1]=s[pos+1];
				url_data[2]=s[pos+2];
				url_data[3]=s[pos+3];
				url_data[4]=s[pos+4];
				url_data[5]=s[pos+5];
				if ( !strncasecmp( url_data, "ftp://", 6 )) {break;}
										   }
														   }


				if (pos > 0)  {  /* fixes truncated lines with 'o' , 'm', 'x', ':' chars */
					dangerous=is_danger_char(s[pos]); 
					if (!dangerous) {dangerous=is_danger_char(s[pos-1]); } 
					if (!dangerous) { dangerous=is_danger_char(s[pos+1]);  }

					if ( dangerous)  {
						wptr +=	 snprintf(wptr, 5,	"%c", s[pos]);
											}  else  {

						snprintf(last_col, 7,"%s", get_num_to_color(
			      		 (color_pos * end_r + (col_arr_check - color_pos) * start_r) / col_arr_check,
			      		 (color_pos * end_g + (col_arr_check - color_pos) * start_g) / col_arr_check,
			      		 (color_pos * end_b + (col_arr_check - color_pos) * start_b) / col_arr_check)  );

					wptr +=snprintf(wptr, 5,"%c",s[pos]); 

														}
							} else {
				wptr +=snprintf(wptr, 5,"%c",s[pos]); 
									}
			


				pos++;
				color_pos++;
										}
			fade_block_counter++;
																		}
									}  /* end while */

	clear_fade_colors();

	/* printf("RET:  %s\n", retval);  fflush(stdout);  */

	return retval;
}


/* used to parse strings like '#FF0000' into 'FF0000', removing any unnecessary or illegal characters - to avoid 'boot codes' with invalid HTML data */

char  *parse_fader_color_code(char *fade_color) 
{
	int counter=0;
	char cust_col[7]="";
	int fade_len=0;
	char *accept_char="ABCDEFabcdef1234567890";  /* list of acceptable HTML data */


						if (fade_color != NULL)  {
							counter=0;
							fade_len=0;
							while (fade_len<strlen(fade_color)) {
								if (counter==6) {break;}
								if (strchr(accept_char,fade_color[fade_len])  != NULL)  {
									cust_col[counter]=fade_color[fade_len];
									counter++;

																														 }
								fade_len++;
														  							 } /* while */
							if (counter==6){
								return strdup(cust_col);							
													  }  else {return strdup("000000");}

															} /* fade_color != Null */
return strdup("000000");
}


/*    added: PhrozenSmoke, this method takes an HTML color string like 
    	'ff88ff' and converts it to the 
     	the 'terminal' form needed for the text boxes... 
     	example custom color: \033[#ff88ffm, we must append 'm' 
*/

char *get_formatted_color(char *some_color) {
	int colori=0;
	char formatted[12]="";
	formatted[0]='\033';
	formatted[1]='[';
	formatted[2]='#';
		/* return a default on a bad some_color length, 
		this should never happen, but just in case */
	/*printf("color:%s[HERE]\n",some_color);
	fflush(stdout);*/
	if (strlen(some_color) != 6) {return strdup("\033[#000000m");} 
	while (colori<6){
		formatted[3+colori]=some_color[colori];
		colori++;
						  }
	formatted[9]='m';
	return strdup(formatted);
																   }




/* *******************************************************  */


/* This method should only be called after  convert_tags(str) has 
     been called, so that the only tags left 
     will be <fade> <alt> and <font> tags  - any other 
     tags left over will be skipped over here 
*/


void convert_fader(char *str)   
{
	char *to;
	char *from;
	char *ptr;
	int is_fader=0;
	int is_alt=0;
	int counter=0;

	static char fdrtmp[3072];
	static char fade_text[1696];  /* buffer for holding stuff between <fade and </fade> tags */
	static char tag_holder[256];  /* for holding tags like <font in between <fade> and </fade> */
	static char fade_buffer[384];  /* should be long enough, fader limit usually 326 */

	int fade_counter_max=1600;
	int tag_holder_counter_max=250;
	int fade_buffer_max=370;

	char *cust_col_formatted1=NULL;  /* retrieved from fader.c parse_fader_color_code() */	
	char *cust_col_formatted2=NULL;  /* retrieved from fader.c parse_fader_color_code() */	

	char *fade_token=NULL;   /* will be used for strtok() calls */
	char *fade_ptr=NULL;  /* will be used for strchr() calls */
	char *fade_color1=NULL;
	char *fade_color2=NULL;
	char *look_for1="<fade>";  /* could be changed to <alt> */
	char *look_for2="</fade>";  /* could be changed to </alt> */

	/* return if too long */
	if ( strlen( str ) > 1550 )
		return;

	if ( ! strchr( str, '<' )) {return; } /* don't bother parsing if no < is present */


	fdrtmp[0]='\0';
	fade_text[0]='\0';
	tag_holder[0]='\0';
	fade_buffer[0]='\0';

	memset( fdrtmp, 0, sizeof( fdrtmp ));
	memset( fade_text, 0, sizeof( fade_text ));
	memset( tag_holder, 0, sizeof( tag_holder ));
	memset( fade_buffer, 0, sizeof( fade_buffer ));

	to = fdrtmp;
	from=ptr=_b2loc(str);

	while ((*ptr) && (*ptr != '\0')) 
	{
		is_fader=0;
		is_alt=0;
		from=ptr;
		while ((*ptr) && (*ptr != '\0') && (*ptr != '<')) { /* grab anything before a < */
			*(to++)=*(ptr++);
					  																  }
		from=ptr;
		if (!strncasecmp( ptr, "<fade", 5 )) {is_fader=1;}
		if (!strncasecmp( ptr, "<alt", 4 )) {is_alt=1;}
		/* This below to avoid segfaults */
		if (!strncasecmp(ptr,"<fade>", 6)) {is_fader=0;}
		if (!strncasecmp(ptr,"<alt>", 5)) {is_alt=0;}


		if (is_fader || is_alt)  /* bumped into <fade or <alt tag ...parse it */
		{

				/* added: PhrozenSmoke, for support of ALT/FADE tags */
				int fade_counter=0;  /*count chars */
				int text_too_big=0;   /* signal if fade_text > 325  in length */
				char *stale_color=NULL;  /* if text over 225, we will only use one ALT/FADE color */

				int tag_holder_counter=0;
				int fade_tokens=0;
				int col_collect=0;

				clear_fade_colors();

				if (is_alt) {
				/* skip alt tags */
				from += 4;	
				look_for1="<alt>";
				look_for2="</alt>";
								}

				if (is_fader) {
				/* skip fade tags */
				from += 5;
				look_for1="<fade>"; 
				look_for2="</fade>";
									}

				ptr=from;

				/* begin added PhrozenSmoke, support ALT/FADE tags. 
					 This is PARTIAL support for ALT/FADE tags, text between the <alt and the </alt> 
					 (or <fade and </fade>)
					 tags that is less than 255 bytes in size will be 'ALTed'/Faded , text 
					 larger than that will be shown with the very first color in the ALT/Fade tag.
					 Right now, only 2 colors (the first color and last color) are supported for 
					 ALT tags  (FADE tags support up to 12 colors)...not the greatest, but enough for a
					 'basic' effect.  8-)
			    */

				fade_ptr=strchr( from, '>' );


				if (fade_ptr != NULL && strncasecmp( ptr,look_for1,strlen(look_for1) )) { 
					/* just a <alt> or <fade> segfaults, so skip it */

					counter=1+ fade_ptr-from;
					if (counter>fade_buffer_max) {counter=fade_buffer_max;}

					if (counter<fade_buffer_max) { /* avoid buffer overflow in fade_buffer */
						snprintf(fade_buffer,counter,"%s",ptr);
						counter=0;
						fade_token=strtok(fade_buffer,",");
						fade_color1=strdup(fade_token);
						fade_col_collect[col_collect]=parse_fader_color_code(fade_color1);
						col_collect++;
						fade_color2=strdup(fade_token);
						fade_tokens=0;
						fade_token=strtok(NULL,",");
						while (fade_token != NULL) {
							if (fade_color2 != NULL) {free(fade_color2);}
							fade_color2=strdup(fade_token);

							if (col_collect<12) {
							fade_col_collect[col_collect]=parse_fader_color_code(fade_color2);
							col_collect++;
													 } else {break;}
							// if (fade_tokens==4) {break;}
							fade_token=strtok(NULL,",");
							fade_tokens++;
																	 }

						if (fade_color1 != NULL && fade_color2 !=NULL)  {

								cust_col_formatted1=parse_fader_color_code(fade_color1);
								cust_col_formatted2=parse_fader_color_code(fade_color2);


								/* wind ptr forward: ignore everything between <fade and the > */
									while(( *from ) && ( *from != '>' )) 	{ from++;}
									if ( *from == '>' ) {from++;}
									ptr=from;

								/* grab everything between the > and the </fade> tags */
								/* right here, we set a limit on fade_text of 325 bytes...if its bigger than that, 
									 it will not be 'faded' or 'alted' instead, it will be shown with the last 
									 collected color - FADE and ALT text take up alot of space.  Our 'to' buffer 
									 is only 4096 bytes, so we will keep the FADE/ALT text result size down to 
									 about 2475 or less (this equates to about 300 chars unformatted)...that's 
									 PLENTY of text - 3 to 4 long sentences.  - This limit is for safety and to 
									 avoid buffer overflows from filling up the 'to' buffer with too much data, since 
									 we must same room for added formatting of smileys, URLS, etc - to be safe, 
									 we will leave 1000-1600+ bytes free in the 'to' buffer for other types of formatting. */

								while(( *ptr ) &&  (fade_counter<fade_counter_max)) 		{ 
									if (*ptr=='\0') {break;}
									/* if (*ptr=='\033') {break;} */ /* needed if </alt> tag is missing at end of line */

									if ( !strncasecmp( ptr, look_for2, strlen(look_for2)))  {
										/* we found </alt> or </fade> let's skip over it break */
										while (( *ptr) && ( *ptr != '>' )){ptr++;}
										from=ptr;
										break; 
																   							   							  }

									if (*ptr=='<')    {
										/* we got a tag like <font> in the middle <alt></alt> (or <fade></fade>) */
										/* right now, we keep any <font> data , but skip all other tags */
										/* All other HTML tags should have been handled in convert_tags except the 
											 <font tags, and this method should ONLY be called after convert_tags */
										/* Drawback: This also means if more than one <font tag is 
											 present between the <alt>
											 </alt> tags (or <fade></fade>) , only the last font saved will take 
											 affect, since we will be 
											 appending everything in tag_holder BEFORE 
											 any ALTed or FADED text */

										if ( !strncasecmp( ptr, "<font", 5 ))  {
											/* make sure <font is lower case */
											if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]='<';}
											if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]='f';}
											if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]='o';}
											if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]='n';}
											if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]='t';}
											ptr += 5;
											while (( *ptr) && ( *ptr != '>' )){
												/* keep data between <font and > tags */
												if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]=*(ptr);}
												ptr++;
														 										}
											if (( *ptr) && ( *ptr == '>' )){	
												if (tag_holder_counter<tag_holder_counter_max) {tag_holder[tag_holder_counter++]=*(ptr);}
												ptr++;
																						   }
											continue;
																								  } /* font */


												/* skipping all other tags between 
													<alt> and </alt> (or <fade></fade>)
											    */  /* Disabled - May 21, 2004 */ /*
											else {  

												if ( *ptr == '<' ) {*(to++)=*(ptr++); }
												while ((*ptr) && ( *ptr != '>' ) && ( *ptr != '<' )){*(to++)=*(ptr++); }
												if (( *ptr) && ( *ptr == '>' )){*(to++)=*(ptr++);}
													}
									
											continue; */
															   } /* if ptr==< */

									/* we got regular text within the <fade></fade> */
									if (fade_counter<fade_counter_max) {
										fade_text[fade_counter++]= *(ptr++); 
										if (fade_counter>325) {text_too_big=1;}
																		   } else {ptr++; }
																					 									} /* while */

									from=ptr;




								/* Sometimes the first and last colors for ALT/FADE 
									are the same...in this case, 
									we will treat it as 'text_too_big' and display the text 
									with one color using 
									the last collected color (cust_col_formatted2) */
								/* 
								if ( !strncasecmp(cust_col_formatted1, cust_col_formatted2,6)) {text_too_big=1;}  */  /* DISABLED, no longer needed */ 


								if (text_too_big==1)  {
									/* there was a huge amount of data between the > and the </alt>/</fade>...
										 so we will display that text as a one color string using the first 
										 collected color (cust_col_formatted1)  */

											if (strlen(tag_holder)>0) { /* append any <font tags */
												strcat( to, tag_holder );
												to += strlen(tag_holder);
																					 }
											stale_color=get_formatted_color(cust_col_formatted1);
											strcat( to, stale_color );
											to += strlen( stale_color  );
											strcat( to, fade_text );
											to += strlen( fade_text);
											free(stale_color);
																	}  else  {  /* data is small enough for full ALT treatment */
											if (strlen(tag_holder)>0) {   /* append any <font tags */
												strcat( to, tag_holder );
												to += strlen(tag_holder);
																					 }
											if (is_fader) {
												stale_color=do_fader_text(cust_col_formatted1,cust_col_formatted2,fade_text);
																} else {
												stale_color=do_alt_text(cust_col_formatted1,cust_col_formatted2,fade_text);
																		  }
											/*printf("STALE COLOR:  %s",stale_color);
											    fflush(stdout);  */											
											strcat( to, stale_color);
											to += strlen(stale_color);
											free(stale_color);
																				}  /* end else */


								free(cust_col_formatted1);	
								free(cust_col_formatted2);	

							if (counter > 0) {counter=0;}
							if (fade_color1 != NULL) {free(fade_color1); fade_color1=NULL;}
							if (fade_color2 != NULL) {free(fade_color2); fade_color2=NULL;}


															  										 } /* if fade_color not null */															
				
												 } /* if counter < 299 */

												   													} /* end if fade_ptr not null */
				 if (*from=='>') {from++; ptr=from;}
				/* end added PhrozenSmoke */




		} else { /*non-fader, append unedited, some other text outside
					 of <fade or <alt :  yada yada <b><fade ...>hey</fade> you go...  */
			if ( *ptr == '<' ) {*(to++)=*(ptr++); }
			while ((*ptr) && (*ptr != '\0') && (*ptr != '>') && ( *ptr != '<' ) ) { 
				/* grab everything before the < */
				*(to++)=*(ptr++);
					  																  	  }
			if (*from=='>') {from++; ptr=from;}
				  } /* else non-fader */

	} /* while */



	*to = '\0';	
	strncpy( str,  _utf(fdrtmp ), 3068 );
}


/* ******************************************************* */




void fader_sampler_ct_clear() {
	GtkTextIter start;
	GtkTextIter end;
	int text_length;
	text_length = gtk_text_buffer_get_char_count(fader_sampler_textbuff );
	gtk_text_buffer_get_iter_at_offset( fader_sampler_textbuff, &start, 0 );
	gtk_text_buffer_get_iter_at_offset( fader_sampler_textbuff, &end, text_length );
	gtk_text_buffer_delete( fader_sampler_textbuff, &start, &end );
	gtk_text_buffer_get_end_iter(fader_sampler_textbuff, &end );
	gtk_text_buffer_create_mark(fader_sampler_textbuff, "end_mark", &end, 0 );
}

void update_fader_sample_display() {
	static char fdrsampbuff[256]="";
	fader_sampler_ct_clear();

	if (fader_conf_using_fader)  {
		snprintf(conffader_start_holder, 8, "%s", "<FADE ");
		snprintf(conffader_end_holder, 8, "%s", "</FADE>");
	} else {
		snprintf(conffader_start_holder, 8, "%s", "<ALT ");
		snprintf(conffader_end_holder, 8, "%s", "</ALT>");
	}

	if (total_fader_colors_config>0)  {		
		GtkTreeIter iter;
		gboolean valid;
		int curr_fader_row=0;
		valid = gtk_tree_model_get_iter_first(gtk_tree_view_get_model(colorlist), &iter);
		while (valid) {
			gchar *lselcolor;
			char *lseldup=NULL;
			gtk_tree_model_get(gtk_tree_view_get_model(colorlist), &iter,GYSINGLE_COL1, &lselcolor, -1);
			/* Note, we need to do this extra strdup, because otherwise
				Gtk will segfault on the call to 'g_free' if we call strncat on
				'lselcolor' directly...not sure why tho  */
			lseldup=strdup(lselcolor);
			strncat(conffader_start_holder,lseldup, 9);
			free(lseldup);
			if (lselcolor) {g_free(lselcolor);}
			curr_fader_row++;
			if (curr_fader_row < total_fader_colors_config) {strcat(conffader_start_holder,",");}
			valid = gtk_tree_model_iter_next(gtk_tree_view_get_model(colorlist), &iter);
		}

	} else {strcat(conffader_start_holder, "#000000,#000000");}
	strcat(conffader_start_holder,  ">");

	snprintf(conffader_sample_holder, 131, "%s%s%s", conffader_start_holder, conffader_sample_str, conffader_end_holder);
	gtk_entry_set_text(GTK_ENTRY(fader_config_tag_entry), conffader_start_holder);
	snprintf(fdrsampbuff, 254, "%s", conffader_sample_holder); 
	append_to_textbox_color(fadeconfigwin, fader_sampler_textb, fdrsampbuff);
}

void set_fadecolor_for_row(char *frcol)  {
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	unsigned int colornum;
	char tmp_int[3];
	char myfadecol[10];
	char *myfadey=NULL;

	selection=gtk_tree_view_get_selection(colorlist);
	if (! gtk_tree_selection_get_selected(selection, &model, &fadealt_row_selected)) {
		if (!is_inserting_fade_col) {return;}
	}
				tmp_int[2] = '\0';
				tmp_int[0] = frcol[ 0 ];
				tmp_int[1] = frcol[ 1 ];
				sscanf( tmp_int, "%x", &colornum);
				fadelist_color.red=colornum * ( 65535 / 255 );
				tmp_int[0] = frcol[2];
				tmp_int[1] = frcol[3];
				sscanf( tmp_int, "%x", &colornum);
				fadelist_color.green=colornum * ( 65535 / 255 );
				tmp_int[0] = frcol[4];
				tmp_int[1] = frcol[5];
				sscanf( tmp_int, "%x", &colornum);
				fadelist_color.blue=colornum * ( 65535 / 255 );

	/* printf("got colors: %d, %d, %d\n", fadelist_color.red, fadelist_color.green, fadelist_color.blue); fflush(stdout); */

	snprintf(myfadecol, 9, "#%s", frcol);
	myfadey=strdup(myfadecol);

	if (is_inserting_fade_col) {
		if (total_fader_colors_config>6) {return;}
		if (gtk_tree_selection_get_selected(selection, &model, &fadealt_row_selected)) {
			gtk_list_store_insert_after(GTK_LIST_STORE(model), &iter, &fadealt_row_selected);
													 } else {
			gtk_list_store_append(GTK_LIST_STORE(model), &iter);
																}
		total_fader_colors_config++;

			gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			GYSINGLE_PIX_TOGGLE, FALSE, 
			GYSINGLE_PIX, NULL,
			GYSINGLE_COL1, myfadey,
			GYSINGLE_COLOR, &fadelist_color, GYSINGLE_TOP, "",  -1);

	}  else {
			gtk_list_store_set(GTK_LIST_STORE(model), &fadealt_row_selected, 
			GYSINGLE_PIX_TOGGLE, FALSE, 
			GYSINGLE_PIX, NULL,
			GYSINGLE_COL1, myfadey,
			GYSINGLE_COLOR, &fadelist_color, GYSINGLE_TOP, "",-1);
			}
	free(myfadey);
}

void setup_fader_color() {
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	char *frcol=NULL;
	gchar *lselcolor;

	selection=gtk_tree_view_get_selection(colorlist);
	if (! gtk_tree_selection_get_selected(selection, &model, &fadealt_row_selected)){return;}
	gtk_tree_model_get(gtk_tree_view_get_model(colorlist), &fadealt_row_selected,GYSINGLE_COL1, &lselcolor, -1);
	frcol=parse_fader_color_code(lselcolor);
	set_fadecolor_for_row(frcol);
	g_free(lselcolor);
	free(frcol);
}

void on_fader_del_cb (GtkButton  *button, gpointer user_data) {
	GtkTreeSelection *selection;
	GtkTreeModel *model;

	selection=gtk_tree_view_get_selection(colorlist);
	if (! gtk_tree_selection_get_selected(selection, &model, &fadealt_row_selected)){return;}
	gtk_list_store_remove(GTK_LIST_STORE(model), &fadealt_row_selected);
	total_fader_colors_config--;
	update_fader_sample_display();
}

void fade_color_closed_cb  (GtkButton  *button, gpointer user_data) {
	gtk_widget_destroy(fader_colorsel);
}

void update_fade_color_cb  (GtkButton  *button, gpointer user_data) {
	gtk_color_selection_get_current_color ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->colorsel) , &fadelist_color);	
	set_fadecolor_for_row(
get_num_to_color(fadelist_color.red / ( 65535 / 255 ), fadelist_color.green / ( 65535 / 255 ), fadelist_color.blue / ( 65535 / 255 ))
);
	update_fader_sample_display();
	gtk_widget_destroy(fader_colorsel);
}

void create_fade_colorsel() {
	fader_colorsel=gtk_color_selection_dialog_new(_("Configuration: Text Effects"));
	gtk_window_set_wmclass (GTK_WINDOW (fader_colorsel), "gyachEDialog", "GyachE"); 
  	gtk_window_set_position (GTK_WINDOW (fader_colorsel), GTK_WIN_POS_MOUSE);
  	gtk_window_set_modal (GTK_WINDOW (fader_colorsel), TRUE);
	gtk_color_selection_set_has_opacity_control ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->colorsel) , FALSE);
	gtk_color_selection_set_has_palette  ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->colorsel) , TRUE);
	gtk_color_selection_set_current_color ( GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->colorsel) , &fadelist_color);
  		gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->cancel_button),  "clicked",
                      GTK_SIGNAL_FUNC (fade_color_closed_cb),
                      NULL);
  		gtk_signal_connect (GTK_OBJECT (GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->ok_button), "clicked",
                      GTK_SIGNAL_FUNC (update_fade_color_cb),
                      NULL);
	gtk_widget_show(fader_colorsel);
	gtk_widget_hide(GTK_WIDGET (GTK_COLOR_SELECTION_DIALOG(fader_colorsel)->help_button));
}

void on_fader_insert_cb (GtkButton  *button, gpointer user_data) {
	if (total_fader_colors_config>6) {return;}
	is_inserting_fade_col=1;
	create_fade_colorsel();
}
void on_fader_edit_cb (GtkButton  *button, gpointer user_data) {
	GtkTreeModel *model;
	GtkTreeSelection *selection;

	selection=gtk_tree_view_get_selection(colorlist);
	if (! gtk_tree_selection_get_selected(selection, &model, &fadealt_row_selected)) {return;}
	is_inserting_fade_col=0;
	setup_fader_color();
	create_fade_colorsel();
}


void onFaderRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		GtkTreeModel *model;

		model=gtk_tree_view_get_model(tv);
		if (gtk_tree_model_get_iter(model, &fadealt_row_selected, tp)) {
			gchar *name;
			gtk_tree_model_get(model, &fadealt_row_selected,  GYSINGLE_COL1, &name, -1);
			is_inserting_fade_col=0;
			setup_fader_color();
			create_fade_colorsel();
			g_free(name);
		}
}




void on_fader_style_toggled (GtkWidget  *button, gpointer user_data) {
	if ( gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (fader_style1))) {
	fader_conf_using_fader=1;
	} else {fader_conf_using_fader=0;}
	update_fader_sample_display();
}

gboolean on_fadeconfigwin_destroy_event  (GtkWidget *widget, GdkEvent *event,                      gpointer user_data) {
	if ( fadeconfigwin) {gtk_widget_destroy( fadeconfigwin );}
	fadeconfigwin= NULL;
	return( TRUE );
}
gboolean on_fadeconfigwin_destroy_cb (GtkButton  *button, gpointer user_data) {
	return on_fadeconfigwin_destroy_event  (NULL, NULL, NULL);
}

gboolean on_fadeconfigwin_ok_cb (GtkButton  *button, gpointer user_data) {
	update_fader_sample_display();
	gtk_entry_set_text(GTK_ENTRY(fader_text_start), conffader_start_holder);
	gtk_entry_set_text(GTK_ENTRY(fader_text_end), conffader_end_holder);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(use_chat_fader),TRUE);
	write_config();
	return on_fadeconfigwin_destroy_event  (NULL, NULL, NULL);
}

void parse_incoming_fader_sample() {
	char *frcol=NULL;	
	char *confsth=NULL;
	char *confenh=NULL;
	char *myfade1=strdup(gtk_entry_get_text(GTK_ENTRY(fader_text_start)));
	snprintf(conffader_start_holder, 63, "%s", myfade1);
	free(myfade1);
	if (!strcmp(conffader_start_holder,"")) {return;}
	if (strncasecmp(conffader_start_holder,"<FADE ", 6) &&
	strncasecmp(conffader_start_holder,"<ALT ", 5)) {
		return ;
	}
	if (! strncasecmp(conffader_start_holder,"<FADE ", 6)) {fader_conf_using_fader=1;}
	else {fader_conf_using_fader=0;}
	is_inserting_fade_col=1;
	confsth=strchr(conffader_start_holder,'#');
	while (confsth)  {
		confenh=strchr(confsth,',');
		if (!confenh) {confenh=strchr(confsth,' ');}
		if (!confenh) {confenh=strchr(confsth,'>');}
		if (confenh)  {
			*confenh='\0';
			confsth++;
			frcol=parse_fader_color_code(confsth);
			set_fadecolor_for_row(frcol);
			free(frcol);
			confenh++;
			if (total_fader_colors_config>6) {return;}
		}
		confsth=confenh;
		if (confsth) {confsth=strchr(confenh,'#');}
	}
}


GtkWidget* create_fadeconfigwin ()
{
GSList *color_fader_group = NULL;
  GtkWidget *vbox1;
  GtkWidget *label1;
  GtkWidget *hbox1;
  GtkWidget *vbox2;
  GtkWidget *insertbutt;
  GtkWidget *changebutt;
  GtkWidget *delbutt;
  GtkWidget *scrolledwindow1;
  GtkWidget *hbox2;
  GtkWidget *okbutt;
  GtkWidget *cancelbutt, *fader_style2;
  char famsg[35]="";
  char *col_headers[]={"",NULL};

  fadeconfigwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (fadeconfigwin), _("Configuration: Text Effects"));
  gtk_window_set_wmclass (GTK_WINDOW (fadeconfigwin), "gyachEFader", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW (fadeconfigwin), GTK_WIN_POS_CENTER);

  vbox1 = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (fadeconfigwin), vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (vbox1), 5);

  gtk_box_pack_start (GTK_BOX (vbox1), gtk_label_new (_("Configuration: Text Effects")), FALSE, FALSE, 2);

  snprintf(famsg, 28, "%s", _("Maximum Colors"));
  strcat(famsg, ": 7");
  label1=gtk_label_new(famsg);
  gtk_box_pack_start (GTK_BOX (vbox1), label1, FALSE, FALSE, 1);

  hbox1 = gtk_hbox_new (FALSE, 8);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  vbox2 = gtk_vbox_new (FALSE, 5);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, FALSE, FALSE, 0);

  snprintf(famsg, 20, "%s", _("Add"));
  strcat(famsg, "...");
  insertbutt = get_pixmapped_button(famsg, GTK_STOCK_ADD);
  gtk_box_pack_start (GTK_BOX (vbox2), insertbutt, FALSE, FALSE, 0);
  set_tooltip(insertbutt,famsg);

  changebutt = get_pixmapped_button(_("Edit..."), GTK_STOCK_SELECT_COLOR);
  gtk_box_pack_start (GTK_BOX (vbox2), changebutt, FALSE, FALSE, 0);
  set_tooltip(changebutt,_("Edit..."));

  delbutt = get_pixmapped_button(_("Delete"), GTK_STOCK_REMOVE);
  gtk_box_pack_start (GTK_BOX (vbox2), delbutt, FALSE, FALSE, 0);
  set_tooltip(delbutt,_("Delete"));

  fader_style1 = gtk_radio_button_new_with_mnemonic (NULL, "FADE");
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (fader_style1), color_fader_group);
  color_fader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (fader_style1));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fader_style1), TRUE);

  fader_style2 = gtk_radio_button_new_with_mnemonic (NULL, "ALT");
  gtk_radio_button_set_group (GTK_RADIO_BUTTON (fader_style2), color_fader_group);
  color_fader_group = gtk_radio_button_group (GTK_RADIO_BUTTON (fader_style2));

  gtk_box_pack_start (GTK_BOX (vbox2), gtk_hseparator_new(), FALSE, FALSE, 3);
  gtk_box_pack_start (GTK_BOX (vbox2), gtk_label_new(_("Style:")), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), fader_style1, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), fader_style2, FALSE, FALSE, 0);

  gtk_signal_connect (GTK_OBJECT (fader_style1), "toggled",
                      GTK_SIGNAL_FUNC (on_fader_style_toggled),
                      NULL);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (hbox1), scrolledwindow1, TRUE, TRUE, 0);
  gtk_widget_set_usize (scrolledwindow1, 125, -2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  colorlist = create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 0, 
	0, col_headers);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), GTK_WIDGET(colorlist));
gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_SHADOW_OUT);

   fader_sampler_textb = gtk_text_view_new ();	
   gtk_text_view_set_editable(GTK_TEXT_VIEW(fader_sampler_textb), FALSE);
   fader_sampler_textbuff=gtk_text_view_get_buffer( GTK_TEXT_VIEW(fader_sampler_textb));
   gtk_object_set_data (GTK_OBJECT (fader_sampler_textb), "textbuffer", 
		fader_sampler_textbuff );
   gtk_widget_set_usize (fader_sampler_textb, 285, 62);
  gtk_box_pack_start (GTK_BOX (vbox1), fader_sampler_textb, TRUE, TRUE, 2);
  gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(fader_sampler_textb) , GTK_WRAP_WORD );
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW( fader_sampler_textb), 5);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW( fader_sampler_textb ), 5);

  fader_config_tag_entry=gtk_entry_new();
  gtk_editable_set_editable(GTK_EDITABLE(fader_config_tag_entry), FALSE);
  gtk_box_pack_start (GTK_BOX (vbox1), fader_config_tag_entry, FALSE, FALSE, 2);

  hbox2 = gtk_hbox_new (TRUE, 7);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox2, FALSE, FALSE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (hbox2), 5);

  okbutt = get_pixmapped_button(_("OK"), GTK_STOCK_OK);
  gtk_box_pack_start (GTK_BOX (hbox2), okbutt, TRUE, TRUE, 0);
  set_tooltip(okbutt,_("OK"));

  cancelbutt = get_pixmapped_button(_("Cancel"), GTK_STOCK_CANCEL);
  gtk_box_pack_start (GTK_BOX (hbox2), cancelbutt, TRUE, TRUE, 0);
  set_tooltip(cancelbutt,_("Cancel"));

  gtk_signal_connect (GTK_OBJECT (insertbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_fader_insert_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (changebutt), "clicked",
                      GTK_SIGNAL_FUNC (on_fader_edit_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (delbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_fader_del_cb),
                      NULL);
  g_signal_connect (colorlist, "row-activated", (GCallback) onFaderRowActivated,
                      NULL);
  gtk_signal_connect (GTK_OBJECT (cancelbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_fadeconfigwin_destroy_cb),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (okbutt), "clicked",
                      GTK_SIGNAL_FUNC (on_fadeconfigwin_ok_cb),
                      NULL);
  gtk_signal_connect_after (GTK_OBJECT (fadeconfigwin), "delete_event",
                            GTK_SIGNAL_FUNC (on_fadeconfigwin_destroy_event),
                            NULL);

	fadelist_color.red   = 0x00 * ( 65535 / 255 );
	fadelist_color.green = 0x00 * ( 65535 / 255 );
	fadelist_color.blue  = 0x00 * ( 65535 / 255 );
	gtk_tree_model_get_iter_first(gtk_tree_view_get_model(colorlist), &fadealt_row_selected);
	is_inserting_fade_col=1;
	total_fader_colors_config=0;
	fader_conf_using_fader=1;

  gtk_widget_show_all(fadeconfigwin);
  parse_incoming_fader_sample();
  if (!fader_conf_using_fader) {
  	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (fader_style2), TRUE);
  }  else { update_fader_sample_display(); }
  return fadeconfigwin;
}

void on_create_fadeconfigwin (GtkButton  *button, gpointer user_data) {
        if (fadeconfigwin) {return;}
	create_fadeconfigwin ();
}


