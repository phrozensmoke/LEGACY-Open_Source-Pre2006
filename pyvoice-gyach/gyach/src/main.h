/*****************************************************************************
 * main.h
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

#define ATTR_RESET  "\033[0m"
#define YAHOO_COLOR_BLACK "\033[30m" 
#define YAHOO_COLOR_BLUE "\033[#0000D9m"   /* was \033[31m */
#define YAHOO_COLOR_CYAN "\033[#00B8C8m"  /* was \033[32m */
#define YAHOO_COLOR_GRAY "\033[#9AA0B3m"   /* was \033[33m, too close to black */
#define YAHOO_COLOR_GREEN "\033[#4FBC6Fm"  /* was \033[34m */
#define YAHOO_COLOR_ORANGE "\033[#E6B42Cm"  /* was \033[37m , looks yellow not orange */
#define YAHOO_COLOR_PINK "\033[#FFAFAFm"   /* was \033[35m */
#define YAHOO_COLOR_PURPLE "\033[#B200BAm"     /* was \033[35m */
#define YAHOO_COLOR_RED "\033[#C81447m"  /* was \033[38m */

/* Added, PhrozenSmoke - this needed to be standardized */
#define YAHOO_COLOR_YELLOW "\033[#DDD927m"  
/* Added, PhrozenSmoke - a couple others */
#define YAHOO_COLOR_OLIVE "\033[#709966m"  
#define YAHOO_COLOR_PMBLUE "\033[#3149BFm"  
#define YAHOO_COLOR_PMPURPLE "\033[#A137BEm"  

#define YAHOO_FONT_FACE_START "face=\""
#define YAHOO_FONT_SIZE_START "size=\""
#define YAHOO_FONT_TAG_START "<font"
#define YAHOO_STYLE_BOLDOFF "\033[x1m"
#define YAHOO_STYLE_BOLDON "\033[1m"
#define YAHOO_STYLE_ITALICOFF "\033[x2m"
#define YAHOO_STYLE_ITALICON "\033[2m"
#define YAHOO_STYLE_UNDERLINEOFF "\033[x4m"
#define YAHOO_STYLE_UNDERLINEON "\033[4m"
#define YAHOO_STYLE_URLON "\033[lm"
#define YAHOO_STYLE_URLOFF "\033[xlm"

GtkWidget *build_login_window( void );
int split( char *str, char *connector );
void show_yahoo_packet();
void append_to_textbox(GtkWidget *window, GtkWidget *textbox, char *text);
void append_to_textbox_color(GtkWidget * win, GtkWidget *tb, char *text);
void chat_command( char *cmd );
void chatter_list_status( char *user, gchar **status_image, char *status );

#ifndef USE_GTK2
GdkFont *getFont(int bold, int italic, int underline,
    const char *family, const char *sizeS);
#endif
