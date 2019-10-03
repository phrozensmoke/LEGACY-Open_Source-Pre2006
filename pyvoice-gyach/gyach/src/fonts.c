/*****************************************************************************
 * fonts
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

/*  The all important 'font' module, for Gtk+2 only, for loading font faces, basically 
      This module checks to see if the font specified in <font face="???"> is a font 
	available on our system (or if it is a font we have 'mapped' to one on our system) 

	This is NOT meant to exactly replicate the other user's fonts - their on Windows, 
	we're on Linux!   But, we can get a feel for the font they are using...just map it 
	to something similar.  You should have a good number of TTF fonts on your system.
	Or, you can just map everything to a 'boring' font like arial.

	NOTE: You should edit this file by hand to customize it to the font available on your 
	system.  This maps commonly used Windows fonts in Yahoo to fonts on linux/unix 

	Great place for TTF fonts: http://www.funsite24.com/fo/p/  */


#include <stdlib.h>
#include <string.h>
#include <stdio.h> 
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <malloc.h> 
#include "fonts.h"

extern void lower_str( char *str );
extern char *font_family;
extern GtkTextView *chat_text;
extern char *custom_scale_text;

	/* Holds a shorter list of recently used font faces, speedup */
GHashTable *cached_font_match_hash =NULL; 
	/* Holds list of font capabilities: Bold, Italic, Bold/Italic */
GHashTable *gypango_font_cap =NULL; 

char *guessed_font=NULL;
char *guessed_fontface=NULL;
PangoContext *ftmap=NULL;
int pangfontmapcount;
int pang_fontm_initiated=0;
int gyfonts_available=0;




/*  ****** Size control and font structs ***** */

struct gyach_pang_font {
	char *n_tag;  /* normal */
	char *b_tag;  /* bold */
	char *i_tag;  /* italic */
	char *bi_tag;  /* italic */
};


/* FONTS defined here: PhrozenSmoke, added this file:  A list of acceptable fonts to 
     use for display if a person in the chat has specified their own fonts.  We don't want 
     to try to load any random font they specified to avoid "Gtk fallback" messages on 
     gtk-2, but we also don't want to just load the font because it appears to be 
     available on our system:  Trying to use fonts that don't support the iso-8859-1
     encoding don't seem to work well, so we want to specify a subset of allowable fonts...
     you should adjust this to your system: names can be in ANY case lower/upper...
     and you should avoid listing fonts that don't support full iso-8859-1 encodings, 
     such as ascii-0 'macromedia' fonts - use font aliases, not 'full names', ADJUST THIS 
     for your system!  Remove fonts listed here that you don't have - add ones that you 
     want to support. At a minimum, you should have the common ones like "Arial", 
     "Helvetica" and "Times", It appears you need to select fonts that use your LOCALE's
     'char-set'  such as 'iso-8859-1'.  This is a good set for English and Spanish locales, 
     people speaking other languages and needing other charsets are on their own: 
     not my concern.  */

/* This structure is here to help avoid gtk 'fallbacks' on Gtk-2, just because we chose
  * to use a font does not mean it supports 'bold' or 'italic' (if requested), so we have 
  * a list of what fonts support what.  Perhaps there is some way to list fonts and font 
  * capabilities from Pango & Gtk, but I don't have the time or interest to try to learn
  * how from the crappy API docs.  So, I've hard-coded what works for ME. If somebody 
  * request a font like "Helvetica bold", and Helvetica is available, but 'bold' is not...
  * we can STILL use the Helvetica font, just as 'normal'...to me this is better behavior
  * than Gtk's stupid 'default' behavior of abandoning the requested font altogether 
  * and falling back to the boring 'Sans' font.  (Wouldn't it make more sense to try to load 
  * 'Helvetica normal' if 'Helvetica bold' could not be loaded?)  Well, that's what I'M going
  * to do.
*/

struct installed_font
{
	char *name;  /* the name of the font on our machine */
	int min_font_size;   /* minimum font size to be accepted for this font */
};

struct installed_font  installed_fonts[] ={

	/* basic fonts we all should have */
	{"Times",   12},
	{"Arial",   12},
	{"Helvetica",   12},
	{"courier",   12},
	{"fixed",   12},
	{"sans",   12},
	{"serif",   12},
	{"monospace",   12},

	{"albert",    16},  /* begin fonts on my system */
	{"bastian",   16},
	{"beetlejuice",   16},
	{"westminster",   18},  /* light */
	{"boston",   14},
	{"charter",   16},
	{"comix",   14},
	{"cow_spots",   20},
	{"currency",   16},
	{"currencyoutline",   16},
	{"eurose",   16},
	{"lucidabright",   14},  /* does 'demibold' instead of real 'bold' */
	{"lucidatypewriter",   14},
	{"marlboro",   18},
	{"new",   14},
	{"news gothic mt",   14},
	{"orient",   20},
	{"palisade",   14},
	/* {"parchment",   16},  */ /* traded in parchment for Papyrus, same but bigger */
	{"scogin",   14},
	{"speedball",   16},
	{"penchant",   16},
	{"shower",   16},
	{"subwaylondon",   14},
	{"tahoma",    12},    /* doesnt support italics, causes 'fallback' warnings */
	{"technical",   14},
	{"terminal",   16},
	{"utopia",   16},
	{"vagabond",   12},
	{"vagabondshadow",   24},
	{"verdana",   12},
	{"weekendinparis",   16},
	{"wildwest",   18},
	{"lublick",   16},
	{"lucida",   14},
	{"clearlyu",   14},
	{"bedrock",   18},

	{"trebuchet ms",   12},
	{"spellbound",   16},
	{"tempus sans itc",   14},
	{"papyrus",   14},
	{"new york",   14},
	{"kino mt",   18},
	{"garamond",   12},
	{"engravers mt",   14},
	{"century gothic",   12},
	{"calisto mt",   14},
	{"book antiqua",   14},
	{"new century schoolbook",   14},
	{"blackchancery",   18},
	{"brushflairexpanded",   16},
	{"impact condensed",   16},
	{"palatino_thin",   12},
	/* {"vivacious", 0, 0}, */  /* has been traded in for more readable 'parkavenue bt' */
	{"halloween",   20},
	{"fossil",   16},

	{"baskerville old face",   18},
	{"becker latino bold italic",   16},
	{"bodoni mt bold italic",   14},
	{"bremen bd bt",   14},
	{"broadway",   14},
	{"californian fb",   14},
	{"liquid sex",   18},
	{"webdings",   16},
	{"funstuff",   18},
	{"cut_n_paste",   22},
	{"halibut",   14},
	{"trekker_3",   20},
	{"futurex",   18},
	{"shower",   18},
	{"shatter",   20},
	{"snowtopcaps",   22},
	{"freefrm721 bt bold italic",   16},

	{"bauhaus",   16},
	{"felix titling",   14},
	{"franciscan",   22},
	{"parkavenue bt",   22},
	{"pegasus",   18},
	{"scribble",   14},
	{"short hand",   18},

	/* Other fonts that need size adjustments, may be unreadable if they 
	are rendered at to small a size */

	{"french script mt",   24},
	{"diamond",   20},
	{"epitaph",   20},
	{"actionis",   16},
	{"action jackson",   16},
	{"agatha",   18},
	{"agency fb",   16},
	{"air",   16},
	{"allurescript",   20},
	{"blackadder itc",   18},
	{"black night",   16},
	{"bodini mt condensed",   16},
	{"bradley hand itc",   16},
	{"chiller",   18},
	{"colonna mt",   16},
	{"curlz mt",   16},
	{"ransom",   18},
	{"edwardian script itc",   20},
	{"eras contour",   16},
	{"footlight mt light",   14},
	{"forte",   14},
	{"gallante",   18},
	{"gigi",   16},
	{"harrington",   14},
	{"jokerman",   13},
	{"juice itc",   16},
	{"matisse itc",   16},
	{"menthol",   14},
	{"onyx",   16},
	{"perpetua",   14},
	{"poor richard",   13},
	{"rage",   16},
	{"rage italic",   16},
	{"sadelle",   14},
	{"snap itc",   13},
	{"snell bt",   16},
	{"steward",   13},
	{"viner hand itc",   14},
	{"highlight let",   26},

		/* ** These are necessary, dont remove *** */
	{NULL ,   0},
	{NULL,   0},

  }; 


/* a structure to make font aliases given to us by <font face= to fonts on our machine */

struct fonts_map
{
	char *keyword;  /* the 'keyword' to look for for this alias */
	char *mapped_font;   /* the font on our system we will map this keywoard to */
};






/*  OUR FONT MAP - add/remove/edit as needed */
/*  Just map the name to the desired font alias */

struct fonts_map lin_font_map[] ={

	{"jester", "franciscan"},
	{"callig", "penchant"},   /* any calligraphy font */
	{"chancery", "blackchancery"},	
	{"blackadder", "blackchancery"},
	{"marriage", "blackchancery"},
	{"maidenword", "penchant"},
	{"americantext", "blackchancery"},
	{"american text", "blackchancery"},
	{"cloisterblack", "blackchancery"},
	{"cloister black", "blackchancery"},
	{"wedding", "blackchancery"},
	{"old eng", "penchant"},
	{"oldeng", "penchant"},
	{"out house", "clampettsdisplay"},
	{"outhouse", "clampettsdisplay"},
	{"bamboo", "clampettsdisplay"},

	{"bradley", "technical"},
	{"technical", "technical"},
	{"fine hand", "technical"},
	{"finehand", "technical"},
	{"architext", "technical"},

	{"woodland", "technical"},
	{"engrave", "engravers mt"},
	{"matisse", "bedrock"},
	{"calisto", "calisto mt"},
	{"kino", "kino mt"},
	{"latino", "becker latino bold italic"},
	{"palatino", "palatino_thin"},
	{"impact", "impact condensed"},
	{"Impact", "impact condensed"},
	{"heavy", "impact condensed"},  /* any 'heavy' font */
	{"book", "book antiqua"},
	{"georgia", "book antiqua"},  /* MS Georgia and the like */
	{"benguiat", "book antiqua"},
	{"photina", "book antiqua"},
	{"vivacious", "parkavenue bt"},
	{"script", "parkavenue bt"}, 
	{"cursive", "parkavenue bt"},
	{"pepita", "parkavenue bt"}, 
	{"handwrit", "parkavenue bt"}, /* any 'handwriting' font, Lucida handwriting, etc. */
	{"paradise", "parkavenue bt"},
	{"tempus", "tempus sans itc"},
	{"kristen", "tempus sans itc"},
	{"comix", "comix"},  /* any comic/comix/cartoon font */
	{"comic", "comix"},
	{"cartoon", "comix"},
	{"kids", "comix"},
	{"trebuchet", "trebuchet ms"},
	{"garamond", "garamond"},
	{"gourmand", "garamond"},
	{"britannic", "garamond"},
	{"haetten", "garamond"},
	{"tahoma", "tahoma"},

	{"halloween", "halloween"}, 
	{"wolfgang", "halloween"},
	{"blood", "halloween"},
	{"bleed", "halloween"},
	{"creepy", "halloween"},
	{"horror", "halloween"},
	{"scary", "halloween"},
	{"nightmare", "halloween"},
	{"avant", "eurose"}, 
	{"century", "century gothic"}, 
	{"news", "news gothic mt"}, 
	{"copper", "news gothic mt"},  /* any 'copper'/ Copperplate Gothic, etc. font */
	{"coppr", "news gothic mt"},
	{"gothic", "century gothic"}, 
	{"subway", "subwaylondon"},
	{"london", "subwaylondon"}, 
	{"bart", "subwaylondon"},
	{"andy", "palisade"},
	{"orient", "orient"},	
	{"asia", "orient"},
	{"china", "orient"},
	{"wildwest", "wildwest"},
	{"wanted", "wildwest"},
	{"wild west", "wildwest"},
	{"jokewood", "wildwest"},

	{"hobo", "vagabond"},
	{"bremen", "bremen bd bt"},
	{"unicorn", "brushflairexpanded"},
	{"louisia", "brushflairexpanded"},
	{"baskerville", "baskerville old face"},
	{"latin", "becker latino bold italic"},
	{"poster", "boston"},
	{"bodoni", "bodoni mt bold italic"},
	{"broadway", "broadway"},
	{"california", "californian fb"},
	{"liquid", "liquid sex"},
	{"webdings", "webdings"},  /* for idiots that like to send 'symbols' */
	{"wingdings", "webdings"},
	{"tombats", "webdings"},
	{"funstuff", "funstuff"},
	{"party", "funstuff"},
	{"abadi", "Arial"},
	{"disco", "funstuff"},
	{"festiv", "funstuff"},  /* any festive/festival font */
	{"celebrat", "funstuff"},  /* any 'celebrate' or 'celebration' font */
	{"curlz", "funstuff"},
	{"curls", "funstuff"},
	{"urban", "scribble"},  /* also any 'urban' or 'graffiti' font */
	{"ghetto", "scribble"},  /* TODO: find a good 'graffiti font */
	{"tagger", "scribble"},
	{"spray", "scribble"},
	{"grafitti", "scribble"},
	{"brooklyn", "scribble"},
	{"cut_n_paste", "cut_n_paste"},
	{"blackout", "cut_n_paste"},
	{"paste", "cut_n_paste"},
	{"ransom", "cut_n_paste"},  /* Ransom note fonts, very similar */
	{"paris", "weekendinparis"},
	{"eurostile", "bauhaus"},
	{"eurostar", "bauhaus"},
	{"batang", "new york"},  /* not sure what this one looks like */
	{"bangle", "new york"},  /* not sure what this one looks like */
	{"uppereast", "new york"},
	{"upper east", "new york"},
	{"upperwest", "new york"},
	{"upper west", "new york"},
	{"halibut", "halibut"},
	{"trekk", "trekker_3"},
	{"treker", "trekker_3"},
	{"trekie", "trekker_3"},
	{"rtrek", "trekker_3"},
	{"futur", "futurex"},  /* Any 'futuristic' font, Futurism, Futura, etc. */
	{"chisel", "marlboro"},
	{"castellar", "new york"},
	{"maiandra", "papyrus"},
	{"highlight", "vagabondshadow"},
	{"exotc", "halibut"},
	{"banjo", "halibut"},
	{"humanst", "impact"},
	{"bell mt", "new century schoolbook"},
	{"brush", "brushflairexpanded"},
	{"flatbrush", "orient"},
	{"flat brush", "orient"},

	{"pristina", "speedball"},
	{"balloon", "albert"},
	{"westminster", "westminster"},
	{"desdemona", "westminster"}, 
	{"computer", "westminster"},
	{"digital", "westminster"},
	{"transistor", "westminster"},
	{"berlin", "eurose"},
	{"perpetua", "palatino_thin"},
	{"cataneo", "papyrus"},
	{"shelley", "parkavenue bt"},
	{"allegro", "bremen bd bt"},
	{"ashley", "bremen bd bt"}, /* Ashley crawford font */
	{"footlight", "new york"},
	{"longisland", "new york"},
	{"dauphin", "scogin"},
	{"dolphin", "scogin"},
	{"graphite", "scogin"},
	{"harrington", "funstuff"},
	{"madge", "blackchancery"},
	{"saloon", "wildwest"},
	{"symbol", "webdings"},

	{"freef", "freefrm721 bt bold italic"},  /* any freefrm/freeform font */
	{"freehand", "freefrm721 bt bold italic"},
	{"free hand", "freefrm721 bt bold italic"},
	{"lithograph", "halibut"},
	{"stonehenge", "tempus sans itc"},
	{"joker", "funstuff"},

	{"snap", "albert"},
	{"enviro", "tempus sans itc"},
	{"juice", "papyrus"},
	{"litterbox", "speedball"},
	{"kabel", "impact"},
	{"holstein", "futurex"},
	{"rockwell", "garamond"},
	{"artisti", "albert"}, /* artistik/artistic */
	{"ravie", "funstuff"}, /* Ravie font */
	{"nimrod", "times"},
	{"snell", "parkavenue bt"},
	{"roundhand", "parkavenue bt"},
	{"round hand", "parkavenue bt"},
	{"schwaben", "penchant"},
	{"spanishround", "penchant"},
	{"spanish round", "penchant"},
	{"scythe", "penchant"},
	{"thalia", "penchant"},
	{"shrewsbury", "penchant"},
	{"shlop", "halloween"},
	{"suffer", "halloween"},
	{"shamen", "orient"},
	{"squish", "halibut"},
	{"spong", "subwaylondon"},
	{"madfont", "funstuff"},
	{"wack", "funstuff"},
	{"funk", "funstuff"},
	{"minisystem", "westminster"},

	{"bauhaus", "bauhaus"},
	{"ozhand", "bauhaus"},
	{"oz hand", "bauhaus"},
	{"agency", "futurex"},  /* agency FB */
	{"felix", "felix titling"},
	{"eras", "felix titling"},
	{"barbedor", "felix titling"},  /* barbedor T */
	{"franciscan", "franciscan"},
	{"parkav", "parkavenue bt"},
	{"avenue", "parkavenue bt"},
	{"vivaldi", "parkavenue bt"},  /* any vivaldi font */
	{"english111", "parkavenue bt"},  /* any Vivace or English111 font */
	{"vivace", "parkavenue bt"},
	{"pegasus", "pegasus"},
	{"scribbl", "scribble"},
	{"noize", "scribble"},   /*  'bring tha noize'/ 'bring the noise' */
	{"noise", "scribble"},
	{"short hand", "short hand"},
	{"shorthand", "short hand"},
	{"stylus", "short hand"},  /* Stylus BT */

	{"miami", "broadway"},  /* miami beach, miami skyline, etc. */
	{"penguin", "funstuff"},  /* penguin, penguin attack, etc. */
	{"gigi", "funstuff"},  /* GiGi, etc. */
	{"vag", "vagabond"},
	{"garmnd", "garamond"},  /* ClassGarmnd/ClassGamnd = Garamond */
	{"gamnd", "garamond"},
	{"bible", "tempus sans itc"},
	{"symphony", "tempus sans itc"},
	{"goudy", "engravers mt"},  /* any 'goudy' script */
	{"algerian", "engravers mt"},  /* any 'algerian' script */
	{"souvenir", "palisade"},  /* any 'souvenir' script */
	{"intima", "papyrus"},  /* any 'intimacy'/'intimate' font */
	{"parchment", "papyrus"}, /* Parchment Itc */
	{"poor", "baskerville old face"},  /* Richard/poor richard */
	{"richard", "baskerville old face"},
	{"cuckoo", "franciscan"},
	{"krazy", "franciscan"},
	{"crazy", "franciscan"},
	{"crazy", "franciscan"},
	{"tech", "futurex"},  /* any Techno/other Tech font */
	{"cyber", "futurex"},  /* any 'cyber' font */
	{"snow", "snowtopcaps"},  /* any 'snow' or 'ice' font font */
	{"icicle", "snowtopcaps"},
	{"pooh", "albert"}, /* Pooh, Pooh Bear, etc. */
	{"mickey", "albert"}, /* Mickey, Mickey Mouse, etc. */
	{"simsun", "albert"}, 
	{"simpson", "albert"}, 
	{"modern", "bauhaus"},  /* any other 'modern' font */
	{"advertiser", "bauhaus"},  /* Advertiser, not sure about this one */
	{"zapf", "felix titling"},  /* ZapfEllipt, etc. */
	{"aucoin", "felix titling"},
	{"collage", "cut_n_paste"},  /* 'Collage' style fonts */
	{"boca", "broadway"},  /* Boca Raton, etc. - not sure about this one */
	{"arctic", "short hand"},
	{"bastian", "bastian"},
	{"raised", "bastian"},
	{"3d", "bastian"},
	{"3-d", "bastian"},
	{"mepp", "bastian"},  /* Mepp display shadow */
	{"falkville", "bastian"},  /* Falkville 3d */
	{"shadow", "bastian"},
	{"rising", "bastian"},  /* any 'rising' font, Phoenix Rising, etc. */
	{"adventure", "franciscan"},
	{"balthazar", "franciscan"},
	{"beast", "franciscan"},
	{"babelfish", "scribble"},
	{"hydrogen", "westminster"},  /* hydrogen whiskey, etc. */
	{"clip", "vagabondshadow"},  /* any Page Clips or Paper clips font */
	{"gaelic", "penchant"},  /* any Gaelic/Celtic font */
	{"celt", "penchant"},
	{"edgewater", "shatter"},  /* Edgewater, 'broken', and 'cracked' fonts */
	{"edge water", "shatter"},
	{"broke", "shatter"},
	{"crack", "shatter"},
	{"shatter", "shatter"},
		/* 'Fiber'-ish fonts, Carbon Phyber, etc. */
	{"phyber", "shatter"},
	{"fiber", "shatter"},
	{"ice", "snowtopcaps"},

	/* THE BASICS - we try the basics last */
	{"fixed", "fixed"},   /* fixed FixedSys, etc. */
	{"lucidabright", "lucidabright"},  /* any Lucida Bright */
	{"lucida bright", "lucidabright"},
	{"typew", "lucidatypewriter"},  /* any 'typewriter' font, Lucida Typewriter, etc. */
	{"type w", "lucidatypewriter"}, 
	{"typist", "lucidatypewriter"}, 
	{"lucida", "lucida"},  /* any other Lucida font */
	{"helvetica", "Helvetica"},  /* any Helvetica font */
	{"arial", "Arial"},  /* any Arial font */
	{"mono", "monospace"},  /* any 'mono spaced' font */
	{"times new", "times"},   /* Times New, and Times New Roman, etc. */
	{"roman", "times"},   /* Any other 'Roman' font, etc. */
	{"courier", "courier"},  /* any courier */
	{"serif", "serif"},
	{"system", "serif"},  /* any font called 'system' */

		/* ** These are necessary, dont remove *** */
	{NULL ,   NULL},
	{NULL,   NULL},

};


/* END FONT MAP */

struct gyach_pang_font *gyach_pang_font_new()
{
	struct gyach_pang_font *ret;
	char *mynone=NULL;
	ret = g_new0(struct gyach_pang_font, 1);
	ret->n_tag=mynone;
	ret->b_tag=mynone;
	ret->bi_tag=mynone;
	ret->i_tag=mynone;
	return ret;
}

struct gyach_pang_font *gyach_pang_font_find(char *bud)
{
	struct gyach_pang_font *f = NULL;
	if ( ! gypango_font_cap) {return f;}
	else {
		char tmp_user[80];
		strncpy( tmp_user, bud, 78 );
		lower_str(tmp_user);
		f = g_hash_table_lookup(gypango_font_cap, tmp_user);
	}
	return f;
}

struct gyach_pang_font *add_gypango_font(char *fntmatch)
{
	struct gyach_pang_font *f = NULL;	
	if (!fntmatch) {return f;}
	if (!gypango_font_cap) {gypango_font_cap = g_hash_table_new(g_str_hash, g_str_equal);}
	if ( ! gypango_font_cap) {return f;}

	if (gyach_pang_font_find(fntmatch)) {return f;}  /* already in there */

	else {
		char tmp_user[80];
		strncpy( tmp_user, fntmatch, 78 );
		lower_str(tmp_user);
		f=gyach_pang_font_new();
		g_hash_table_insert(gypango_font_cap, g_strdup(tmp_user), f);
	}
	return f;
}


char *find_cached_font_match(char *bud)
{
	char *f = NULL; 
	if (!bud) {return f;}
	if (!cached_font_match_hash) {return f;}
	f = g_hash_table_lookup(cached_font_match_hash, bud);
	return f;
}

int clear_font_match_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
	g_free(key);
	g_free(value);
	/* printf ("freed\n"); fflush(stdout); */
 	return 1;

} 

void add_cached_font_match(char *fntreq, char *fntmatch)
{
	if (!fntreq) {return;}
	if (!fntmatch) {return;}
	if ( ! cached_font_match_hash) {
		cached_font_match_hash = g_hash_table_new(g_str_hash, g_str_equal);
	}
	if ( ! cached_font_match_hash ) {return;}
	if (find_cached_font_match(fntreq)) {return;}  /* already in there */

	if (g_hash_table_size(cached_font_match_hash )>35) {
		/* don't let cached history get above 35 fonts */
		g_hash_table_foreach_remove(cached_font_match_hash,clear_font_match_hash_cb,NULL);
	}
	g_hash_table_insert(cached_font_match_hash, g_strdup(fntreq), g_strdup(fntmatch));
}

	/* Find best match we can for bold/italic/none settings */

char *get_pango_font_style_string(char *my_font_face, int bold, int italic) {
	struct gyach_pang_font *f=NULL;
	if (guessed_fontface) {free(guessed_fontface); guessed_fontface=NULL;}
	f=gyach_pang_font_find(my_font_face);
	if (f)  {
		if ((!bold) && (!italic))  {  /* 'Normal' */
			guessed_fontface=strdup(f->n_tag);
		}
		if ((bold) && (italic))  {  /* 'Bold/Italic' */
			guessed_fontface=strdup(f->bi_tag);
		}
		if ((bold) && (! italic))  {  /* 'Bold' */
			guessed_fontface=strdup(f->b_tag);
		}
		if ((! bold) && ( italic))  {  /* 'Italic' */
			guessed_fontface=strdup(f->i_tag);
		}
	}
	if (!guessed_fontface) {guessed_fontface=strdup("");}
	/* printf("guessed font style:  %s\n", guessed_fontface);
	fflush(stdout); */
	return guessed_fontface;
}


char *get_pango_font_cap_string(PangoFontFamily *fptr, int bold, int italic) {
	int facenum;
	PangoFontFace **fface;
	PangoFontFace **faceptr;
	int facei;
	char facep[48];

	if (guessed_fontface) {free(guessed_fontface); guessed_fontface=NULL;}

	pango_font_family_list_faces(fptr, &fface, &facenum);
	faceptr=fface;
	for (facei=0; facei<facenum; facei++) {
				snprintf(facep,46, "%s", pango_font_face_get_face_name(*faceptr));
				/* If we have no face set, set the first we get
					in case we find no other match */
				if (!guessed_fontface) { 
					int whereat=facenum-1;  /* last element */
					/* Don't put 'Normal' in the font string, Pango seems to choke on it */
					if ((!strcasecmp(facep,"medium")) || (!strcasecmp(facep,"normal")) 
					|| (!strcasecmp(facep,"regular ")) || (facei==whereat) ) {
						if (!strcasecmp(facep,"normal")) {guessed_fontface=strdup("");}
						else {guessed_fontface=strdup(facep);}
					}
				}

				if ((!bold) && (!italic))  {  /* 'Normal' */
					if ((!strcasecmp(facep,"medium")) || (!strcasecmp(facep,"normal"))) {
						if (guessed_fontface) {free(guessed_fontface);}

						/* Don't put 'Normal' in the font string, Pango seems to choke on it */
						if (!strcasecmp(facep,"normal")) {guessed_fontface=strdup("");}
						else {guessed_fontface=strdup(facep); }
						g_free(fface);
						return guessed_fontface;
					}
				}

				if (bold && italic)  {  /* bold and italic */
					if ((strstr(facep,"Bold Italic")) || (strstr(facep,"Bold Oblique")) || 
					(strstr(facep,"Heavy Italic")) || (strstr(facep,"Black Italic"))) {
						if (guessed_fontface) {free(guessed_fontface);}
						guessed_fontface=strdup(facep);
						g_free(fface);
						return guessed_fontface;
					}
				}

				if (bold)  {  /* bold and fallback for bold/italic */
					if ((strstr(facep,"Bold")) || (strstr(facep,"Heavy")) || 
					(strstr(facep,"Black")) ) {
						if (guessed_fontface) {free(guessed_fontface);}
						guessed_fontface=strdup(facep);
						if (!italic && (!strchr(guessed_fontface,' ')) ) {
							g_free(fface);
							return guessed_fontface;
							}
					}
				}

				if (italic)  {  /* italic and fallback for bold/italic */
					if ((strstr(facep,"Italic")) || (strstr(facep,"Oblique")) ) {
						if (guessed_fontface) {free(guessed_fontface);}
						guessed_fontface=strdup(facep);
						if (!bold && (!strchr(guessed_fontface,' ')) ) {
							g_free(fface);
							return guessed_fontface;
							}
					}
				}
				
				faceptr++;
		}  /* for */

	g_free(fface);
	if (!guessed_fontface) {guessed_fontface=strdup("");}
	return guessed_fontface;
}



/* What is the minimum font size we should accept for this font  */

int get_minimum_font_size(char *font_name, int suggest_size)
{
	struct installed_font ifont;
	int suggested=suggest_size;	
	int f_modified=0;
	int available_font=0;

	while (1) {
	   ifont=installed_fonts[available_font];
	  if (! ifont.name) { break;} /* end of array, so quit */ 

		/* This only checks for min font sizes for 'recommended' 
		fonts in the 'installed_fonts' array.  */

	   /* to be safe, look for exact matches */
		if ( !strcasecmp(font_name,ifont.name) ) 
			{
			if (suggested<ifont.min_font_size) {
				suggested=ifont.min_font_size;	
				f_modified=1;			
																	}
			}
		available_font++;

	}

	/* script fonts, avoid microscopic and 'billboard' sized fonts */
	if (! f_modified) {
		if (strstr(font_name,"script") || strstr(font_name,"Script") || 
		strstr(font_name,"cursive") || strstr(font_name,"Cursive")  ) {
			if ( (suggested<24) && (! f_modified) ) {suggested=24;}
		}

		/* fonts with 'shadow' effects need to be larger */ 
		if (strstr(font_name,"shadow") || strstr(font_name,"Shadow") || 
		strstr(font_name,"outline") || strstr(font_name,"Outline")  ) {
			if ( suggested<18 ) {suggested=18;}
		}

		if (strstr(font_name,"highlight") || strstr(font_name,"HighLight") ||
		strstr(font_name,"Highlight") ) {
			if ( suggested<24 ) {suggested=24;}
		}
	}


	/* 
	if (custom_scale_text) {
		if ( (atof(custom_scale_text) >1.0) && (atof(custom_scale_text) <1.5) ) {
			suggested++;
		}
	} */ 

	/* unknown fonts, avoid microscopic and 'billboard' sized fonts */
	if (  (! f_modified) && (suggested<12) ) {suggested=12;}
	if (  (! f_modified) && (suggested>32) ) {suggested= 32;}

	return suggested;  
} 


void init_gyachfont_cap() {
	if (!pang_fontm_initiated) {		
		PangoFontFamily **pangfam;
		PangoFontFamily **fptr;		
		int fonti;
		ftmap=gtk_widget_get_pango_context(GTK_WIDGET(chat_text));
		/* Line below is deprecated in newer versions of Pango, dont re-enable */
		/*  ftmap=pango_ft2_font_map_for_display(); */
		pango_context_list_families(ftmap, &pangfam, &pangfontmapcount);

		fptr=pangfam;
		for (fonti=0; fonti<pangfontmapcount; fonti++) {
			/* record in hashtable */
			char tmp_user[80];
			struct gyach_pang_font *f=NULL;
			snprintf(tmp_user,78,"%s", pango_font_family_get_name(*fptr));
			f=add_gypango_font(tmp_user);
			if (!f) {fptr++; continue;}
			/* discover and record font capabilities */
			f->bi_tag=strdup(get_pango_font_cap_string(*fptr, 1, 1));
			f->b_tag=strdup(get_pango_font_cap_string(*fptr, 1, 0));
			f->i_tag=strdup(get_pango_font_cap_string(*fptr, 0, 1));
			f->n_tag=strdup(get_pango_font_cap_string(*fptr, 0, 0));
			fptr++;
		} 	/* for */
		pang_fontm_initiated=1;
		gyfonts_available=g_hash_table_size(gypango_font_cap );

		/* Free this big ass Pango font cache stuff */
		g_free(pangfam);
		/* This frees the 'ftmap' object */
			/* disabled because this seems to LEAK, instead of free it
			according to valgrind */
		/* pango_x_shutdown_display (GDK_DISPLAY());  */
	}
}


/*  HERE IS WHERE WE DO THE ACTUAL FONT MAPPING, map_font_family */

void  gypang_fontsearch_cb(gpointer key, gpointer value,
        gpointer user_data) {
	if (!guessed_font) {
		if (strstr( (char *)key, (char *)user_data )) {
			guessed_font=strdup((char *)key);
		}
	}
} 

char *lookup_pango_font_family(char *my_font_face, int p_exact) 
{
	static char font_hh[80]="";

	if (guessed_font) {free(guessed_font); guessed_font=NULL;}
	if (!pang_fontm_initiated) {init_gyachfont_cap();}
	if ( ! gypango_font_cap) {return guessed_font;}

			if (p_exact)  {
				if (gyach_pang_font_find(my_font_face)) {
					guessed_font=strdup(my_font_face);
					return guessed_font;
					}
				}	else {  /* non-exact matches */
				char *searchstr=NULL;
				snprintf(font_hh, 78, "%s",my_font_face);
				lower_str(font_hh);
				searchstr=g_strdup(font_hh);
				g_hash_table_foreach(gypango_font_cap,gypang_fontsearch_cb,searchstr);
				g_free(searchstr);
				return guessed_font;
				}		/* else */

	return guessed_font;
}


char *recommend_font_family(char *my_font_face) 
{
	static char font_gg[64]="";
	int cc;
	struct fonts_map ff;

	sprintf(font_gg,"%s","");

	for (cc=0; cc<500; cc++) {
		ff=lin_font_map[cc];

		if (! ff.keyword) {break;} /* end of arrays, so quit */ 

		if (strstr(my_font_face, ff.keyword) !=  NULL) {
			/* found possible match */
			if (ff.mapped_font != NULL) {
				strncpy(font_gg, ff.mapped_font, 62);
				break;
														 }
																				  }
																	}

	if (guessed_font) {free(guessed_font); guessed_font=NULL;}
	if ( strlen(font_gg)>1) {guessed_font=strdup(font_gg);}
	return guessed_font;
}

char *map_font_family(char *my_font_face)  {
	char *sugg_font=NULL;
	static char font_guess[64]="";

	if (guessed_font) {free(guessed_font); guessed_font=NULL;}
	if (! my_font_face) {guessed_font=strdup("Sans"); return guessed_font;}

	sprintf(font_guess,"%s","");
	strncpy(font_guess,my_font_face, 62);
	lower_str(font_guess);

	/* First, see if this is a font that's been requested recently, 
	look through a shorter 'cache' list to avoid unnecessary cycles
	through possibly huge font lists on some systems */

	sugg_font=find_cached_font_match(font_guess);
	if (sugg_font) {return sugg_font;}    /* it's cached, return it */

	/* Second, see if Pango has any 'loose' matches in the list of 
	fonts installed on this system, or any exact matches */

	sugg_font=lookup_pango_font_family(font_guess, 1);  /* check exact first */
	  /* Pango got something we can use */
	if (sugg_font) {add_cached_font_match(font_guess, sugg_font); return sugg_font;}

	/* Third, See if we can recommend a close match that exists 
	in Pango's font list */

	sugg_font=recommend_font_family(font_guess);
	if (sugg_font) {
		/* Got a suggested font, see if Pango has it installed on the sys */
		snprintf(font_guess,62, "%s",sugg_font);
		lower_str(font_guess);
		/* We'll accept exact matches only, as we want to be 
			completely sure this is something Pango can load */
		sugg_font=lookup_pango_font_family(font_guess, 1);
		if (sugg_font) {add_cached_font_match(font_guess, sugg_font); return sugg_font;}
	}

	sugg_font=lookup_pango_font_family(font_guess, 0);
	  /* Pango got something we can use, a very 'loose' match */
	if (sugg_font) {add_cached_font_match(font_guess, sugg_font); return sugg_font;}

	/* Finally, Fallback on a default  */
	if (guessed_font) {free(guessed_font); guessed_font=NULL;}

	/* Warning: This could get stale if we change our font! */
	if (font_family) {guessed_font=strdup(font_family);}
	else {guessed_font=strdup("Sans");}
	add_cached_font_match(font_guess, guessed_font);
	return guessed_font;
}



