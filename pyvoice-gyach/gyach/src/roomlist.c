/*****************************************************************************
 * roomlist.c
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

#include "gyach.h"
#include "main.h"
#include "roomlist.h"
#include "images.h"
#include "util.h"
#include "gytreeview.h"

#include <stdio.h>
#include <expat.h>
#include <string.h>

# define y_new		g_new
# define y_renew	g_renew

extern GtkWidget *room_maker_window;

GtkWidget * room_window = NULL;

int  auto_close_roomlist = 1;
int  cache_room_list = 1;
int  show_adult = 1;

char *room_selected = NULL;
char *room_list_country_code=NULL;
char *create_room_list_country_code=NULL;

int category_level=0;
int ttree_type=0;
GtkTreeView *troom_tree;
GtkTreeIter catnodes[24];
/*  GtkLabel *troom_label;  */
ROOM_ENTRY *troom_item;
GdkColor roomscolor;
char *dl_holder=NULL;
int got_first_room_iter=0;

GtkTreePath *troomnode=NULL;
GtkTreePath *tusernode=NULL;
GtkTreePath *tyahnode=NULL;
GtkTreePath *tparentnode=NULL;
char *troomname=NULL;
char *troomid=NULL;

char *category_cache=NULL;

/* Added, PhrozenSmoke: We are now using the XML based room loading 
    features of Yahoo, which means we can't simple filter out adult rooms 
    by changing the URLs we download, here's some basic filtering to restore 
    the option to filter out adult rooms */

char *adult_filter[] = {
			  /* The 'romance' and 'adult' category holds most of the adult rooms */
			 "adult",  "bdsm",  "fetish", "gay", "sex", "bi-sex", "lesbian", "straight", "str8", 
			"married but", "role play", "roleplay",
			 "webcam",  "cams",  "camz",  "web-cam", "web cam", "penis", " ass", 
			"shit", "bitch", "fuck", "fck", "shyt",  "fukk", "jack", "assh", "anal", "cum", "dick", " butt", 
			"cock",	"pussy","puzzy",	"horny","oral",	"blowj","jerk", "blow j", "bastard", 
			"tramp", "slut", "whore", "hoez", "hoes", "cunt", "biotch", "biatch", "bytch", "inch", 
			"kkk", "nigga", "nigger", "nikka", "masturb", "masterb", "nude", "naked", "nekkid", 
			 "transgen", "transves", "vagina", " ballz", " balls", "incest", " rape", " rapin", "suck",
			"breast", "tits", "titties", "nipple", "testicl"," lick", "clit", "tongue", "orgasm",  "hump",
			"ejac", "foreplay", "69", "viagra","hardon", "hard-on", "lolita", "xxx", "porn",  "molest",
			 "pedo",  "fag",  "dyke",  "dike",  "fisting",  "erect",  "mistress",  "slave",  "stripper",
			 "strap-on", "dildo", "vibrat",  "molest",  "strapon",  "hardcore",  "submiss","underwear",
			 "hooker",  "escort",  "prostitut",  "kink",  "sleaz",  "sleez",   "freak", "nudist", "panty", 
			 "j/o", "m4m", "curious",  "swallow", "sxy", "moan", "coochi", "panties", "lust", "nudity", 
			NULL
						};

char filter_holder[56]="";


/* room category list is cached here in case we want to create a room, but Yahoo's server's
   are malfunctioning, a quick list of usable categories so we can create a room */

char *backup="<content time=\"1064921901\"> \n<chatCategories>\n<category id=\"1600000002\" name=\"Business &amp; Finance\">\n</category>\n<category id=\"1600004725\" name=\"Computers &amp; Internet\">\n</category>\n<category id=\"1600008033\" name=\"Cultures &amp; Community\">\n<category id=\"1600043325\" name=\"By Language\">\n</category>\n<category id=\"1600013556\" name=\"Countries and Cultures\">\n</category>\n<category id=\"1600307381\" name=\"Crime\">\n</category>\n<category id=\"1600041694\" name=\"Food and Drink\">\n</category>\n<category id=\"1600136382\" name=\"Groups\">\n<category id=\"1600014766\" name=\"Lesbians, Gays, and Bisexuals\">\n</category>\n<category id=\"1600008562\" name=\"Teen\">\n</category>\n</category>\n<category id=\"1600063985\" name=\"Issues and Causes\">\n</category>\n</category>\n<category id=\"1600016068\" name=\"Entertainment &amp; Arts\">\n<category id=\"1600020948\" name=\"Movies\">\n</category>\n<category id=\"1600035291\" name=\"Television\">\n</category>\n</category>\n<category id=\"1600038063\" name=\"Family &amp; Home\">\n</category>\n<category id=\"1600052895\" name=\"Games\">\n</category>\n<category id=\"1600059353\" name=\"Government &amp; Politics\">\n</category>\n<category id=\"1600060813\" name=\"Health &amp; Wellness\">\n</category>\n<category id=\"1600062280\" name=\"Hobbies &amp; Crafts\">\n</category>\n<category id=\"1600417569\" name=\"Music\">\n</category>\n<category id=\"1600064623\" name=\"Recreation &amp; Sports\">\n<category id=\"1600064625\" name=\"Automotive\">\n</category>\n<category id=\"1600066644\" name=\"Baseball\">\n</category>\n<category id=\"1600066992\" name=\"Basketball\">\n<category id=\"1600067052\" name=\"National Basketball Association (NBA)\">\n</category>\n<category id=\"1600067245\" name=\"Women's National Basketball Association (WNBA)\">\n</category>\n</category>\n<category id=\"1600067622\" name=\"Football (American)\">\n</category>\n<category id=\"1600184035\" name=\"Hockey\">\n</category>\n<category id=\"1600070156\" name=\"Wrestling\">\n</category>\n</category>\n<category id=\"1600043463\" name=\"Regional\">\n<category id=\"1600043929\" name=\"Asia\">\n</category>\n<category id=\"1600043471\" name=\"Australia\">\n</category>\n<category id=\"1600043504\" name=\"Canada\">\n</category>\n<category id=\"1600043564\" name=\"China\">\n</category>\n<category id=\"1600043628\" name=\"India\">\n</category>\n<category id=\"1600043695\" name=\"Indonesia\">\n</category>\n<category id=\"1600043699\" name=\"Ireland\">\n</category>\n<category id=\"1600043731\" name=\"Malaysia\">\n</category>\n<category id=\"1600043773\" name=\"New Zealand\">\n</category>\n<category id=\"1600043794\" name=\"Philippines\">\n</category>\n<category id=\"1600043818\" name=\"Singapore\">\n</category>\n<category id=\"1600043852\" name=\"Thailand\">\n</category>\n<category id=\"1600043956\" name=\"United States\">\n</category>\n<category id=\"1600043863\" name=\"United Kingdom\">\n</category>\n<category id=\"1600014318\" name=\"Vietnam\">\n</category>\n</category>\n<category id=\"1600073831\" name=\"Religion &amp; Beliefs\">\n</category>\n<category id=\"1600083763\" name=\"Romance\">\n<category id=\"1600083764\" name=\"Adult\">\n<category id=\"1600083801\" name=\"BDSM\">\n</category>\n<category id=\"1600085416\" name=\"Fetishes\">\n</category>\n<category id=\"1600084215\" name=\"Gay Male\">\n</category>\n<category id=\"1600084740\" name=\"Lesbian\">\n</category>\n<category id=\"1600084752\" name=\"Married But Looking\">\n</category>\n<category id=\"1600085693\" name=\"Masturbation\">\n</category>\n<category id=\"1600086224\" name=\"Role Playing\">\n</category>\n<category id=\"1600084891\" name=\"Straight\">\n</category>\n</category>\n<category id=\"1600089124\" name=\"By Location\">\n</category>\n<category id=\"1600047754\" name=\"Friends\">\n</category>\n</category>\n<category id=\"1600077623\" name=\"Schools &amp; Education\">\n</category>\n<category id=\"1600082641\" name=\"Science\">\n</category>\n<category id=\"20377088\" name=\"Voice Chat\">\n</category>\n</chatCategories>\n</content>\n";


/* Added, PhrozenSmoke: We are now using the XML based room loading 
    features of Yahoo, which means we can't simple filter out adult rooms 
    by changing the URLs we download, here's some basic filtering to restore 
    the option to filter out adult rooms */

int is_adult_cat_or_room(char *name) {
	if (show_adult) {return 0;}
	else {
	char **cptr = adult_filter;
	snprintf(filter_holder, 54, "%s", name);
	lower_str(filter_holder);		
		while( *cptr ) {
			if ( strstr(filter_holder,*cptr )) {  return 1; }
			cptr++;
							}
		}
	return 0;
}




/* borrowed from libyahoo2 project */

char *yahoo_xmldecode(const char *instr)
{
	int ipos=0, bpos=0, epos=0;
	char *str = NULL;
	char entity[4]={0,0,0,0};
	char *entitymap[5][2]={
		{"amp;",  "&"}, 
		{"quot;", "\""},
		{"lt;",   "<"}, 
		{"gt;",   ">"}, 
		{"nbsp;", " "}
	};
	unsigned dec;
	int len = strlen(instr);

	/* Speedup, PhrozenSmoke, if it has no amphersand, dont waste the time parsing */
	if (!strchr(instr, '&')) {return strdup(instr);}

	if(!(str = y_new(char, len+1) ))  {	return strdup(""); }

	while(instr[ipos]) {
		while(instr[ipos] && instr[ipos]!='&')
			if(instr[ipos]=='+') {
				str[bpos++]=' ';
				ipos++;
			} else
				str[bpos++] = instr[ipos++];
		if(!instr[ipos] || !instr[ipos+1])
			break;
		ipos++;

		if(instr[ipos] == '#') {
			ipos++;
			epos=0;
			while(instr[ipos] != ';') {
				entity[epos++]=instr[ipos++];
											  }
			sscanf(entity, "%u", &dec);
			str[bpos++] = (char)dec;
			ipos++;
		} else {
			int i;
			int convwrit=0;			
			for (i=0; i<5; i++)  {
				if(!strncmp(instr+ipos, entitymap[i][0], 
					       strlen(entitymap[i][0]))) {
				       	str[bpos++] = entitymap[i][1][0];
					ipos += strlen(entitymap[i][0]);
					convwrit=1;
					break;
				}
										}
			if (convwrit != 1) {str[bpos++]='&'; }  /* nothing found, write skipped ampherand */ 
		}
	}
	str[bpos]='\0';

	/* free extra alloc'ed mem. */
	len = strlen(str);
	str = y_renew(char, str, len+1);

	return (str);
}





/* Below is some XML expat parsing code borrowed partially from the libquran project (GPL) */

int callback_enc(void *enc_data, const XML_Char *name, XML_Encoding *info) {
		return 0;
}


/**
 * Internal function: This callback function is used when parsing the xml data.
 * Called when the parser finds an XML start tag.
 *
 * \param *user_data user data
 * \param *name tag name
 * \param **attrs attributes
 *
 * \sa callback_cdata(), callback_end()
 **/
void callback_start_cat(void *user_data, const XML_Char *name, const XML_Char **attrs) {
	char name_p[88]="";
	char *name_ptr=NULL;
	ROOM_ENTRY *uroom_item;
	GtkTreeStore *treestore;
	GtkTreeModel *model;
	GtkTreeIter niter;
	GtkTreeIter niterb;
	GdkPixbuf *imbuf=NULL;

	/* saving translators names */
	if (strncmp((char*)name,"category",8)==0) {
		if (!attrs) {return;}
		if (! *attrs) {return;}

		/* check for adult rooms/categories, this only affects us if 'show_adult' is false */
		if (! show_adult) {
			char *adult_check=NULL;
			XML_Char **cptr=(XML_Char **) attrs;
			while (cptr && *cptr) {
				if (strncmp(cptr[0],"name",4)==0) {
						if (adult_check) {free(adult_check);}						
						adult_check=strdup(cptr[1]);
						if (is_adult_cat_or_room(adult_check)) {category_level++; free(adult_check); return;}
															 	  }
				cptr += 2;
											}
				if (adult_check) {free(adult_check);}
								}

		troom_item = (ROOM_ENTRY *)malloc( sizeof( ROOM_ENTRY ));
		troom_item->populated = 0;
		troom_item->top_level=0;
		troom_item->temp_child = NULL;
		troom_item->room_name=NULL;
		troom_item->room_id=NULL;

		while (attrs && *attrs) {
			if (strncmp(attrs[0],"id",2)==0) {
				troom_item->room_id = strdup( attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"name",4)==0) {
				troom_item->room_name= strdup( attrs[1]);
														}
			//printf("cat parsed\n"); fflush(stdout);
			attrs += 2;
										}

		

		if (!troom_item->room_name) {free(troom_item); return;}
		if (!troom_item->room_id) {free(troom_item); return;}
		category_level++;
		model=gtk_tree_view_get_model(troom_tree);
		treestore=GTK_TREE_STORE(model);
		g_object_ref(model);
		g_object_ref(treestore);
		if (got_first_room_iter) {niter=catnodes[category_level];	}


				/* printf(" --- CHILD  incoming %d  \n", category_level); fflush(stdout); */ 

		if (got_first_room_iter && (category_level>1)) {
		niterb=catnodes[category_level-1];
		gtk_tree_model_get(model, &niterb,GYROOM_DATA, &uroom_item, -1);
		if (( uroom_item ) &&
			( uroom_item->temp_child )) {
				GtkTreeIter iter;
				if (gtk_tree_model_get_iter(model, &iter, 
					uroom_item->temp_child  ) ) {

				/* printf(" --- remove CHILD \n"); fflush(stdout); */ 
			gtk_tree_store_remove(treestore, &iter);	
			uroom_item->populated = 0;
			gtk_tree_path_free(uroom_item->temp_child);
			uroom_item->temp_child = NULL;
					  }
		}
									} /* got first iter */

		snprintf(name_p,86, "%s",troom_item->room_name );		
		name_ptr=strdup(name_p);

				/* printf(" --- CHILD addd\n"); fflush(stdout);  */

	if ( got_first_room_iter && (category_level>1) ) {
		niterb=catnodes[category_level-1];
		gtk_tree_store_append(treestore, &niter, &niterb);  /* Force to the end of the rows */
		catnodes[category_level]=niter;
			/* printf("withparent\n"); fflush(stdout);  */
		} else {
		gtk_tree_store_insert(treestore, &niter, NULL, 9000);  /* Force to the end of the rows */
		catnodes[category_level]=niter;
				 }

		if (ttree_type==0) {
		troom_item->top_level=1;
									 } else {
		troom_item->top_level=0;
												}


		/* printf(" --- CHILD  adder %d  %d \n", category_level, troom_item->top_level); fflush(stdout);  */


		if (ttree_type==0) {
			GtkTreeIter iter;
				/* printf(" --- CHILD append\n"); fflush(stdout);*/ 
			gtk_tree_store_append(treestore, &iter, &niter);  /* Force to the end of the rows */		
			troom_item->temp_child =gtk_tree_model_get_path(model, &iter);
			roomscolor.red=0; roomscolor.green=23000; roomscolor.blue=24000;
			gtk_tree_store_set(treestore, 
				&iter, 
				GYROOM_PIX_TOGGLE, FALSE, 
				GYROOM_PIX, NULL,
				GYROOM_COL1, dl_holder,
				GYROOM_COLOR, &roomscolor,  -1);
				}

	imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here);
	gtk_tree_store_set(treestore, 
			&niter, 
			GYROOM_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYROOM_PIX, imbuf,
			GYROOM_COL1, name_ptr,
			GYROOM_COLOR, NULL, GYROOM_DATA, troom_item, -1);
	got_first_room_iter=1;
	if (imbuf) {g_object_unref(imbuf);}

		free(name_ptr); name_ptr=NULL;

		g_object_unref(model);
		g_object_unref(treestore);
	}
	if (name_ptr) {free(name_ptr);}
}

/**
 * Internal function: This callback function is used when parsing the xml data.
 * Called when the parser finds an XML CData.
 *
 * \param *user_data user data
 * \param *data data string
 * \param len length of the data string
 *
 * \sa callback_start(), callback_end()
 **/

void callback_cdata(void *user_data, const XML_Char *data, int len) {
	/* Yahoo chatroom lists have no cdata, return */
	return;
}

/**
 * Internal function: This callback function is used when parsing the xml data.
 * Called when the parser finds an XML end tag.
 *
 * \param *user_data user data
 * \param *name tag name
 *
 * \sa callback_start(), callback_cdata()
 **/

void callback_end_cat(void *user_data, const XML_Char *name) {	
	if (strncmp((char*)name,"category",8)==0) {
		category_level--;
		if (category_level<0) {category_level=0;}
	}
	

}



void callback_end_rooms(void *user_data, const XML_Char *name) {	
	return;
}


void callback_start_rooms(void *user_data, const XML_Char *name, const XML_Char **attrs) {
	GtkTreeIter pariter;
	GtkTreeIter iter;
	char name_p[160]="";
	char *rtopic=NULL;
	char *rname=NULL;
	char *name_ptr=NULL;
        int roomtype=0;  /* 0=yahoo, 1=user */

	if (strncmp((char*)name,"room",4)==0) {

		if (!attrs) {return;}
		if (! *attrs) {return;}


		/* check for adult rooms/categories, this only affects us if 'show_adult' is false */
		if (! show_adult) {
			char *adult_check=NULL;
			XML_Char **cptr=(XML_Char **) attrs;
			while (cptr && *cptr) {
				if (strncmp(cptr[0],"name",4)==0) {
						if (adult_check) {free(adult_check);}						
						adult_check=strdup(cptr[1]);
						if (is_adult_cat_or_room(adult_check)) {free(adult_check); return;}
															 	  }
				cptr += 2;
											}
				if (adult_check) {free(adult_check);}
								}


		if (troomname) {free(troomname); troomname=NULL;}
		if (troomid) {free(troomid); troomid=NULL;}

		troom_item = (ROOM_ENTRY *)malloc( sizeof( ROOM_ENTRY ));
		troom_item->populated = 1;
		troom_item->top_level=0;
		troom_item->temp_child = NULL;
		troom_item->room_name=NULL;
		troom_item->room_id=NULL;

		/* Below: I don't think these calls to 'yahoo_xmldecode' are 
		really necessary as it seems Expat handles decoding of 
		weirdo tags by itself...I guess more to be thorough for now */

		while (attrs && *attrs) {
			if (strncmp(attrs[0],"id",2)==0) {
				troom_item->room_id = strdup( attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"name",4)==0) {
				rname= yahoo_xmldecode( attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"topic",5)==0) {
				rtopic= yahoo_xmldecode( attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"type",4)==0) {
				if (strncmp(attrs[1],"user",4)==0) { roomtype=1;}
														}

			// printf("room parsed\n"); fflush(stdout);
			attrs += 2;
										}

					

		if (!rname) {free(troom_item); return;}
		if (!troom_item->room_id) {free(troom_item); return;}
		troom_item->room_name=strdup(rname);
		troomname=strdup(rname);
		if (!rtopic) {rtopic=g_strdup(" -No Topic- ");}
		troomid=strdup(troom_item->room_id);

		if (name_ptr) {free(name_ptr);}
		snprintf(name_p,157, "%s   (\"%s\")",troomname, rtopic );	
		strip_html_tags(name_p);	
		name_ptr=strdup(name_p);
		if (troomnode) {gtk_tree_path_free(troomnode);}
		if (roomtype==1) { 
			gtk_tree_model_get_iter(gtk_tree_view_get_model(troom_tree), &pariter, tusernode);
		} else {
			gtk_tree_model_get_iter(gtk_tree_view_get_model(troom_tree), &pariter, tyahnode);
		}


	gtk_tree_store_append(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), &iter, &pariter);  /* Force to the end of the rows */
	troomnode = gtk_tree_model_get_path(gtk_tree_view_get_model(troom_tree), &iter);
	gtk_tree_store_set(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), 
			&iter, 
			GYROOM_PIX_TOGGLE, FALSE, 
			GYROOM_PIX, NULL,
			GYROOM_COL1, name_ptr,
			GYROOM_COLOR, NULL, GYROOM_DATA, troom_item, -1);

		g_free(rname);
		g_free(rtopic);
		if (name_ptr) {free(name_ptr); name_ptr=NULL;}
	}


	if (strncmp((char*)name,"lobby",5)==0) {
		char myname[88]="";
		char *itsname=myname;
		char pusers[8]="";
		char pvoice[8]="";
		char pcam[8]="";

		if (!attrs) {return;}
		if (! *attrs) {return;}
		if (!troomnode) {return;}
		if (!troomname) {return;}

		troom_item = (ROOM_ENTRY *)malloc( sizeof( ROOM_ENTRY ));
		troom_item->populated = 1;
		troom_item->top_level=0;
		troom_item->temp_child = NULL;
		troom_item->room_name=NULL;

		while (attrs && *attrs) {
			if (strncmp(attrs[0],"count",5)==0) {
				snprintf(itsname,86, "%s:%s",troomname,attrs[1]);
				troom_item->room_name=strdup(itsname);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"users",4)==0) {
				snprintf(pusers,6, "U:%s",attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"voices",6)==0) {
				snprintf(pvoice,6, "V:%s",attrs[1]);
				attrs += 2; continue;
														}
			if (strncmp(attrs[0],"webcams",7)==0) {
				snprintf(pcam,6, "C:%s",attrs[1]);
														}
			attrs += 2;
										}

		if (!troom_item->room_name) {free(troom_item); return;}
		troom_item->room_id=strdup(troomid);
		if (!troom_item->room_id) {free(troom_item); return;}

		if (name_ptr) {free(name_ptr);}
		snprintf(name_p,157, "%s     [%s  %s  %s]",troom_item->room_name, pusers,pvoice,pcam );		
		name_ptr=strdup(name_p);
		gtk_tree_model_get_iter(gtk_tree_view_get_model(troom_tree), &pariter, troomnode);
		gtk_tree_store_prepend(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), &iter, &pariter);  /* Force to the end of the rows */
		gtk_tree_store_set(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), 
			&iter, 
			GYROOM_PIX_TOGGLE, FALSE, 
			GYROOM_PIX, NULL,
			GYROOM_COL1, name_ptr,
			GYROOM_COLOR, NULL, GYROOM_DATA, troom_item, -1);

		if (name_ptr) {free(name_ptr); name_ptr=NULL;}
	}

	if (name_ptr) {free(name_ptr);}
}




int parse_xml_rooms(char *mybuff, int bytes_read, int is_category) {

	XML_Parser p;
	void *buff;
	p = XML_ParserCreate(NULL);      /* XML_ParserCreate( "iso-8859-1"); */
	if (is_category) {XML_SetElementHandler(p, callback_start_cat, callback_end_cat);}
	else {XML_SetElementHandler(p, callback_start_rooms, callback_end_rooms);}
	XML_SetCharacterDataHandler(p, callback_cdata);
	XML_SetUnknownEncodingHandler(p, callback_enc, NULL);
	XML_SetUserData(p, "");

		buff = XML_GetBuffer(p, bytes_read+3);		
		if (buff == NULL) {
			return -1;
		}

		strncpy((char *)buff,mybuff,bytes_read+1);

		if (! XML_ParseBuffer(p, bytes_read, bytes_read == 0)) {
			return 1;
		}

	XML_ParserFree(p);	
	return 1;

}



void set_room_list_country_code(char *intlstr) {
  if (room_list_country_code) {free(room_list_country_code);}
	room_list_country_code=strdup(intlstr);
}


void populate_child_room_list( char *number, GtkTreeView *room_tree,
    GtkTreePath *parent_node )
{
	
	char *roombuf=NULL;
	GtkTreeIter iter;
	GtkTreeIter parentiter;
	char catlist[88] = "http://insider.msg.yahoo.com/ycontent/?chatroom_";
	int url_length;	
	char mymsg[192]="";
	char name_p[56]="";  
	char *name_ptr=NULL;
	GdkPixbuf *imbuf=NULL;
	
	// DBG( 11, "populate_child_room_list()\n" );

	 	/* changed, seg-fault fix: 65536, was not always enough */
	if (!roombuf) {roombuf=malloc(80896);}
	if (!roombuf) {	return;	}

	snprintf(catlist,76, "http://insider.msg.yahoo.com/ycontent/?chatroom_%s=0", number);

		if (room_list_country_code)  {
			if (strstr(room_list_country_code,"us")) {
				strncat(catlist, "&intl=us", 8);
			}
		}

	ttree_type=0;   /* regular room list, not room creation list */

	/* troom_label = GTK_LABEL( lookup_widget( GTK_WIDGET(room_tree), 
					       "loading_status" ));
	gtk_label_set_text( troom_label, _("Loading Room List...") );  */

	process_gtk_events();

	set_max_url_fetch_size(80896);
	sprintf(roombuf,"%s","");
	url_length = fetch_url( catlist, roombuf, ymsg_sess->cookie ); 
	clear_fetched_url_data();

	if ((url_length ) &&
		(!strstr( roombuf, "lobby" )))  {
		snprintf(mymsg, 190, "%s:\n%s", _("Could not load the room list"), catlist);
		show_ok_dialog(mymsg);
		/* gtk_label_set_text( troom_label, " " ); */
		free(roombuf); roombuf=NULL;		
		return;
												}

	strncpy( name_p, _("User Rooms"), 54 );
	if (name_ptr) {free(name_ptr);}
	name_ptr=strdup(name_p);

			if (gtk_tree_model_get_iter(gtk_tree_view_get_model(troom_tree), &parentiter, 
					parent_node  )) {

			gtk_tree_store_append(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), &iter, &parentiter);  /* Force to the end of the rows */

			troom_item = (ROOM_ENTRY *)malloc( sizeof( ROOM_ENTRY ));
			troom_item->room_name = strdup( name_ptr );
			troom_item->room_id = strdup( number );
			troom_item->populated = 1;
			troom_item->top_level=1;
			troom_item->temp_child = NULL;

			roomscolor.red=0; roomscolor.green=0; roomscolor.blue=26000;
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here_male);
	gtk_tree_store_set(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), 
			&iter, 
			GYROOM_PIX_TOGGLE, TRUE, 
			GYROOM_PIX, imbuf,
			GYROOM_COL1, name_ptr,
			GYROOM_COLOR, &roomscolor, GYROOM_DATA, troom_item, -1);

		if (tusernode) {gtk_tree_path_free(tusernode);}
		tusernode= gtk_tree_model_get_path(gtk_tree_view_get_model(troom_tree), &iter);
		g_object_unref(imbuf);
		}


	strncpy( name_p, _("Yahoo Rooms"), 54 );
	if (name_ptr) {free(name_ptr);}
	name_ptr=strdup(name_p);

			if (gtk_tree_model_get_iter(gtk_tree_view_get_model(troom_tree), &parentiter, 
					parent_node  )) {

			gtk_tree_store_append(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), &iter, &parentiter);  /* Force to the end of the rows */

			troom_item = (ROOM_ENTRY *)malloc( sizeof( ROOM_ENTRY ));
			troom_item->room_name = strdup( name_ptr );
			troom_item->room_id = strdup( number );
			troom_item->populated = 1;
			troom_item->top_level=1;
			troom_item->temp_child = NULL;

			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here_female);
			roomscolor.red=25000; roomscolor.green=0; roomscolor.blue=0;
	gtk_tree_store_set(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), 
			&iter, 
			GYROOM_PIX_TOGGLE, TRUE, 
			GYROOM_PIX, imbuf,
			GYROOM_COL1, name_ptr,
			GYROOM_COLOR, &roomscolor, GYROOM_DATA, troom_item, -1);
		g_object_unref(imbuf);

		if (tyahnode) {gtk_tree_path_free(tyahnode);}
		tyahnode= gtk_tree_model_get_path(gtk_tree_view_get_model(troom_tree), &iter);
		}

	if (name_ptr) {free(name_ptr); name_ptr=NULL;}

	if (parse_xml_rooms(roombuf,url_length ,0) != 1) {
		snprintf(mymsg,190, "%s:\n%s", _("Could not load the room list"), catlist);
		show_ok_dialog(mymsg);
		/* gtk_label_set_text( troom_label, " " ); */
		process_gtk_events(); /* added */
		free(roombuf); roombuf=NULL;
		return;
	}

	/* added: PhrozenSmoke */
	/* gtk_label_set_text( troom_label, " " );  */ 
	process_gtk_events(); /* added */
	free(roombuf); roombuf=NULL;
	if (name_ptr) {free(name_ptr); name_ptr=NULL;}
}



void populate_room_list(int tree_type) {
	
	/* Notes: Foreign chat lists */
	/*  	mx.chat.yahoo.com/c/roomlist/catfeed.html
	  	mx.chat.yahoo.com/c/roomlist/catfeed.html
  		es.chat.yahoo.com/c/roomlist/catfeed.html
	  	fr.chat.yahoo.com/c/roomlist/catfeed.html
  		dk.chat.yahoo.com/c/roomlist/catfeed.html
	  	in.chat.yahoo.com/c/roomlist/catfeed.html
  		uk.chat.yahoo.com/c/roomlist/catfeed.html
  		au.chat.yahoo.com/c/roomlist/catfeed.html
		others: sg, ca, lt, de, dk, no, ar, asia, cn, kr, hk, tw
	*/

	char *catebuf=NULL;

	char catlist[72] = "http://insider.msg.yahoo.com/ycontent/?chatcat=0&intl=us"; 
	int url_length;
	char mymsg[192];

 	/* the category list is usually under 5kb, extra space to be safe */
	if (!catebuf) {catebuf=malloc(10242);} 
	if (!catebuf) {	return;	}

	/* this should never be freed after it is allocated */
	if (!category_cache) {category_cache=malloc(10241);}
	if (!category_cache) {	return;	}

	category_level=0;

	/* back up for 'create a room' if room list fails to load - category names rarely ever change */
	// http://insider.msg.yahoo.com/ycontent/?chatcat=0&intl=mx

	// DBG( 11, "populate_room_list()\n" );

	if (dl_holder) {free(dl_holder);}
	dl_holder = strdup( _("---- downloading list ----") );
	ttree_type=tree_type;

	if (room_list_country_code)  {

		if ( tree_type ==1)  {
			if (create_room_list_country_code) {
				/* the create a room list, compare and delete cache if necessary */ 
				if (strcasecmp(create_room_list_country_code, room_list_country_code)) {
					sprintf(category_cache,"%s", "");					
				} 
				free(create_room_list_country_code);
			} 
			create_room_list_country_code=strdup(room_list_country_code);
		}

		if (!strstr(room_list_country_code,"us")) {
			 /* internationalization for room list */
			snprintf(catlist, 70, "http://insider.msg.yahoo.com/ycontent/?chatcat=0&intl=%s",room_list_country_code );		

				  														 }	
										 }

	//printf("catlist:  %s\n",catlist);  fflush(stdout);


	if (tree_type==0)  {  /* regular room list */
	troom_tree = GTK_TREE_VIEW(lookup_widget( room_window, "room_tree" ));

	/* troom_label =
		GTK_LABEL( lookup_widget( GTK_WIDGET(troom_tree), "loading_status" )); */

							} else {
	troom_tree = GTK_TREE_VIEW(lookup_widget( room_maker_window, "croom_tree" ));

	/* troom_label =
		GTK_LABEL( lookup_widget( GTK_WIDGET(troom_tree), "cloading_status" ));  */
									   }

	/* gtk_label_set_text( troom_label, _("Retrieving Room List...") ); */

	process_gtk_events();

	/* use cache if available when creating a room */
	if ( (strlen(category_cache)>15) && (tree_type==1)) {
			sprintf(catebuf,"%s", "");
			snprintf(catebuf,10240, "%s", category_cache);
			url_length=strlen(backup);
																			} else {
			sprintf(catebuf,"%s","");
			set_max_url_fetch_size(10239);
			url_length = fetch_url( catlist, catebuf, ymsg_sess->cookie );
			clear_fetched_url_data();
													  								 }


	if ( (! url_length) || (!strstr( catebuf, "category" ))) {   /* use backups for 'create a room' if needed */
		 if (tree_type==1) {  
			sprintf(catebuf,"%s", "");
			snprintf(catebuf,10240, "%s", backup);
			url_length=strlen(backup);
									 } 

																	}  else  {

				/* cache the category list for faster loading next time we create a room */
				if (tree_type==1 ) {
					if (strlen(category_cache)<16) {
			    		sprintf(category_cache,"%s", "");
						snprintf(category_cache, 10240, "%s", catebuf);
																}
											}
																				}

	if ((url_length ) &&
		(!strstr( catebuf, "category" )))  {
		snprintf(mymsg,190, "%s:\n%s", _("Could not load the room list"), catlist);
		show_ok_dialog(mymsg);
		/* gtk_label_set_text( troom_label, " " ); */
		free(catebuf); catebuf=NULL;		
		return;

												}

	/* gtk_label_set_text( troom_label, _("Loading Room List...") ); */ 
	process_gtk_events();
	//catnodes[0]=NULL;
	// gtk_tree_store_append(GTK_TREE_STORE(gtk_tree_view_get_model(troom_tree)), &catnodes[0]);
	got_first_room_iter=0;


	if (parse_xml_rooms(catebuf,url_length , 1) != 1) {
		snprintf(mymsg, 190, "%s:\n%s", _("Could not load the room list"), catlist);
		show_ok_dialog(mymsg);
		/* gtk_label_set_text( troom_label, " " ); */ 
		process_gtk_events(); /* added */
		free(catebuf); catebuf=NULL;
		return;
	}

	/* added: PhrozenSmoke */
	/* gtk_label_set_text( troom_label, " " ); */ 
	process_gtk_events(); /* added */
	free(catebuf); catebuf=NULL;
}

