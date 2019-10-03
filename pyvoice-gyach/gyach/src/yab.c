/*****************************************************************************
 * yab.c
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
 * VERY preliminary code for handling Yahoo address books
 *****************************************************************************/
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include "main.h"
#include "images.h"
#include "util.h"
#include "interface.h"
#include "callbacks.h"
#include "users.h"
#include "profname.h"
#include "gyach.h"
#include "webconnect.h"
#include "gytreeview.h"


char *wiggy[46];
char *yabs[46];
char *titles[46];
int limits[46];

GtkWidget *yab_entry_list=NULL;
GtkWidget *yab_widget_newb=NULL;
GtkWidget *yab_widget_viewb=NULL;
GtkWidget *yab_widget_editb=NULL;
GtkWidget *yab_widget_delb=NULL;
GtkWidget *yab_widget_profb=NULL;
GtkWidget *yab_widget_imb=NULL;
GtkWidget *yab_widget_myb=NULL;
GtkWidget *yab_widget_loadb=NULL;
GtkWidget *yab_widget_saveab=NULL;
int addressbook_loaded=0;

char *yab_row_selected=NULL;
GHashTable *yahoo_yab_entries=NULL;
GtkWidget *yab_window=NULL;
int yab_search_shown=0;
int yabcounter=0;
int enable_addressbook=1;
char *yab_search_term=NULL;

void unfreeze_yab_address_book();
void enable_yab_address_book();
void freeze_yab_address_book();


#define YAB_HOOK(component,name,widget) \
  gtk_object_set_data (GTK_OBJECT (component), name, widget)

struct yahoo_yab {
      char *yi;
      char *id;
      char *fn;
      char *ln;
      char *nn;
      char *e0;
      char *hp;
      char *wp;
      char *mo;
      char *pr;
      char *mn;
      char *ti;
      char *fa;
      char *pa;
      char *ot;
      char *e1;
      char *e2;
      char *pu;
      char *ha; 
      char *hc;
      char *hs;
      char *hz;
      char *hn;
      char *co;
      char *wa; 
      char *wc;
      char *ws;
      char *wz;
      char *wn;
      char *wu;
      char *bi;
      char *an;
      char *cm;
      char *c1;
      char *c2;
      char *c3;
      char *c4;
};


struct yahoo_yab *selected_yab=NULL;


void yahoo_yab_free(gpointer p)
{
	struct yahoo_yab *f = p;
	char *ss=NULL;
	if (!f) {return;}
      if (f->yi) { g_free(f->yi); f->yi =ss; }
      if (f->id) { g_free(f->id); f->id =ss; }
      if (f->fn) { g_free(f->fn); f->fn =ss; }
      if (f->ln) { g_free(f->ln); f->ln =ss; }
      if (f->nn) { g_free(f->nn); f->nn =ss; }
      if (f->e0) { g_free(f->e0); f->e0 =ss; }
      if (f->hp) { g_free(f->hp); f->hp =ss; }
      if (f->wp) { g_free(f->wp); f->wp =ss; }
      if (f->mo) { g_free(f->mo); f->mo =ss; }
      if (f->pr) { g_free(f->pr); f->pr =ss; }
      if (f->mn) { g_free(f->mn); f->mn =ss; }
      if (f->ti) { g_free(f->ti); f->ti =ss; }
      if (f->fa) { g_free(f->fa); f->fa =ss; }
      if (f->pa) { g_free(f->pa); f->pa =ss; }
      if (f->ot) { g_free(f->ot); f->ot =ss; }
      if (f->e1) { g_free(f->e1); f->e1 =ss; }
      if (f->e2) { g_free(f->e2); f->e2 =ss; }
      if (f->pu) { g_free(f->pu); f->pu =ss; }
      if (f->ha) { g_free(f->ha); f->ha =ss; } 
      if (f->hc) { g_free(f->hc); f->hc =ss; }
      if (f->hs) { g_free(f->hs); f->hs =ss; }
      if (f->hz) { g_free(f->hz); f->hz =ss; }
      if (f->hn) { g_free(f->hn); f->hn =ss; }
      if (f->co) { g_free(f->co); f->co =ss; }
      if (f->wa) { g_free(f->wa); f->wa =ss; } 
      if (f->wc) { g_free(f->wc); f->wc =ss; }
      if (f->ws) { g_free(f->ws); f->ws =ss; }
      if (f->wz) { g_free(f->wz); f->wz =ss; }
      if (f->wn) { g_free(f->wn); f->wn =ss; }
      if (f->wu) { g_free(f->wu); f->wu =ss; }
      if (f->bi) { g_free(f->bi); f->bi =ss; }
      if (f->an) { g_free(f->an); f->an =ss; }
      if (f->cm) { g_free(f->cm); f->cm =ss; }
      if (f->c1) { g_free(f->c1); f->c1 =ss; }
      if (f->c2) { g_free(f->c2); f->c2 =ss; }
      if (f->c3) { g_free(f->c3); f->c3 =ss; }
      if (f->c4) { g_free(f->c4); f->c4 =ss; }
}


struct yahoo_yab *yahoo_yab_new()
{
	struct yahoo_yab *f;
	f = g_new0(struct yahoo_yab, 1);
      f->yi=NULL;
      f->id=NULL;
      f->fn=NULL;
      f->ln=NULL;
      f->nn=NULL;
      f->e0=NULL;
      f->hp=NULL;
      f->wp=NULL;
      f->mo=NULL;
      f->pr=NULL;
      f->mn=NULL;
      f->ti=NULL;
      f->fa=NULL;
      f->pa=NULL;
      f->ot=NULL;
      f->e1=NULL;
      f->e2=NULL;
      f->pu=NULL;
      f->ha=NULL; 
      f->hc=NULL;
      f->hs=NULL;
      f->hz=NULL;
      f->hn=NULL;
      f->co=NULL;
      f->wa=NULL; 
      f->wc=NULL;
      f->ws=NULL;
      f->wz=NULL;
      f->wn=NULL;
      f->wu=NULL;
      f->bi=NULL;
      f->an=NULL;
      f->cm=NULL;
      f->c1=NULL;
      f->c2=NULL;
      f->c3=NULL;
      f->c4=NULL;
	return f;
}

struct yahoo_yab *yahoo_yab_find(char *bud)
{
	struct yahoo_yab *f = NULL;
	char tmp_user[16];
	strncpy( tmp_user, bud, 14 );
	lower_str(tmp_user);
	if ( ! yahoo_yab_entries ) { return f; }
	f = g_hash_table_lookup(yahoo_yab_entries, tmp_user);
	return f;
}

struct yahoo_yab *create_or_find_yahoo_yab(char *bud)
{
	struct yahoo_yab *f = NULL;
	char tmp_user[16];
	strncpy( tmp_user, bud, 14 );
	lower_str(tmp_user);
	f=yahoo_yab_find(tmp_user);
	if (!f) {
		char *duppy=NULL;
		if ( ! yahoo_yab_entries ) { return f; }
		duppy=g_strdup(tmp_user);
		f = yahoo_yab_new();
		f->id=g_strdup(tmp_user);
		g_hash_table_insert(yahoo_yab_entries, duppy , f);
		}
	return f;
}

void clear_yab_object_hash() {
	if (yahoo_yab_entries) {g_hash_table_destroy(yahoo_yab_entries);}
	yahoo_yab_entries= g_hash_table_new(g_str_hash, g_str_equal);
}

int clear_yab_hash_cb(gpointer key, gpointer value,
        gpointer user_data) {
		if (key) {g_free(key);}
		if (value) {
			yahoo_yab_free(value);
			g_free(value);   /* segfault watch */
		}
 	return 1;
} 

void empty_yab_book() {
	if ( yab_row_selected ) {
		free( yab_row_selected );  
		yab_row_selected=NULL;
	  }
	if ( ! yahoo_yab_entries ) { clear_yab_object_hash(); return; }
	g_hash_table_foreach_remove(yahoo_yab_entries,clear_yab_hash_cb,NULL);
	clear_yab_object_hash();
}

void collect_yab_list_selected()  {
		GtkTreeIter yabiter;
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		if (!yab_entry_list) {
			if ( yab_row_selected ) {
				free( yab_row_selected );  
				yab_row_selected=NULL;
			}
		return;
		}
		selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(yab_entry_list));
		if ( gtk_tree_selection_get_selected(selection, &model, &yabiter)) {
			gchar *sfound;
			gtk_tree_model_get(model, &yabiter,GYSINGLE_TOP, &sfound, -1);		
			if ( yab_row_selected ) {	free( yab_row_selected );  }
			yab_row_selected=strdup(sfound);
			g_free(sfound);
		} else {
			if ( yab_row_selected ) {
				free( yab_row_selected );  
				yab_row_selected=NULL;
			}
		}
}


void append_yab_to_clist(gpointer key, gpointer value,
        gpointer user_data) {
	char us_stat[192];
	gchar *rd;
	gchar *rkey;
	GdkPixbuf *imbuf=NULL;
	GtkTreeIter iter;
	GtkTreeModel *model;
	model =(GtkTreeModel *)user_data;
	struct yahoo_yab *YABBY=value;
	if (!key) {return; }
	if (!value) {return;}
	gtk_list_store_append(GTK_LIST_STORE(model), &iter);
	
	snprintf(us_stat, 72, "%s", "?");
	
	if ( (YABBY->yi) && (strlen(YABBY->yi)>2) ) {
		if (! strstr(YABBY->yi,"@") ) {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_on);
		} else {
			imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_off);
			}
		snprintf(us_stat, 72, "%s", YABBY->yi);
	} else {
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_stealth_off);
		if (YABBY->e0) {snprintf(us_stat, 72, "%s", YABBY->e0);}
		else if (YABBY->fn) {snprintf(us_stat, 72, "%s", YABBY->fn);}
		else if (YABBY->nn) {snprintf(us_stat, 72, "%s", YABBY->nn);}
		else if (YABBY->co) {snprintf(us_stat, 72, "%s", YABBY->co);}
		else if (YABBY->ln) {snprintf(us_stat, 72, "%s", YABBY->ln);}
		else {snprintf(us_stat, 72, "%s", "?");} 
	}

	if (strlen(us_stat)<2) {snprintf(us_stat, 72, "%s  [%s]", "??", (char *)key );}

	if ( (YABBY->fn) || (YABBY->ln) ) {
		strcat(us_stat,"    [");
		if (YABBY->fn) {strncat(us_stat, YABBY->fn, 50);}
		if ( (YABBY->fn) && (YABBY->ln) ) { strcat(us_stat, " "); }
		if (YABBY->ln) {strncat(us_stat, YABBY->ln, 50);}
		strcat(us_stat,"]");
	}
	rd=g_strdup(us_stat);
	rkey=g_strdup(key);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
			GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE, 
			GYSINGLE_PIX, imbuf,
			GYSINGLE_COL1, rd,
			GYSINGLE_TOP, rkey,
			GYSINGLE_COLOR, NULL, -1);
	g_free(rd);
	g_free(rkey);
	if (imbuf) {g_object_unref(imbuf);}
}

void populate_yab_clist()  {
	if (yahoo_yab_entries) {
			GtkTreeModel *model;	
			model=freeze_treeview(GTK_WIDGET(yab_entry_list));
			gtk_list_store_clear(GTK_LIST_STORE(model));
			g_hash_table_foreach(yahoo_yab_entries, append_yab_to_clist, model );
			unfreeze_treeview(GTK_WIDGET(yab_entry_list), model);
			gtk_widget_show_all (GTK_WIDGET(yab_entry_list));			
	}
			if ( yab_row_selected ) {
				free( yab_row_selected );  
				yab_row_selected=NULL;
			}
}


/* partially borrowed from libyahoo2 */



void yahoo_yab_read(unsigned char *d, int len)
{
	char *st, *en;
	char *data = (char *)d;
	data[len]='\0';
	struct yahoo_yab *yab=NULL;

		// printf("\nPARSE:\n  %s\n", data); fflush(stdout);
		// printf("read-1\n"); fflush(stdout);
	
	st = en = strstr(data, "id=\"");

			// printf("read-1a\n"); fflush(stdout);
	if(st) {
			// printf("read-1b\n"); fflush(stdout);
		st += strlen("id=\"");
			// printf("read-1c: %s\n", st); fflush(stdout);
		en = strchr(st, '"'); *en++ = '\0';
			// printf("read-1d\n"); fflush(stdout);
		yab=create_or_find_yahoo_yab(st);
				// printf("got yab:  %s\n", st); fflush(stdout);
		if (!yab) {return;} else {yabcounter++;}
			// printf("read-1e\n"); fflush(stdout);
	} else {return;}

		// printf("read-2\n"); fflush(stdout);

	st = strstr(en, "fn=\"");
	if(st) {
		st += strlen("fn=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->fn) {g_free(yab->fn);}
		yab->fn = yahoo_xmldecode(st);
	}
		// printf("read-3\n"); fflush(stdout);

	st = strstr(en, "mn=\"");
	if(st) {
		st += strlen("mn=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->mn) {g_free(yab->mn);}
		yab->mn = yahoo_xmldecode(st);
	}

	st = strstr(en, "ln=\"");
	if(st) {
		st += strlen("ln=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->ln) {g_free(yab->ln);}
		yab->ln = yahoo_xmldecode(st);
	}


	st = strstr(en, "nn=\"");
	if(st) {
		st += strlen("nn=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->nn) {g_free(yab->nn);}
		yab->nn = yahoo_xmldecode(st);
	}

		// printf("read-6\n"); fflush(stdout);

	st = strstr(en, "e0=\"");
	if(st) {
		st += strlen("e0=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->e0) {g_free(yab->e0);}
		yab->e0 = yahoo_xmldecode(st);
	}


	st = strstr(en, "yi=\"");
	if(st) {
		st += strlen("yi=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->yi) {g_free(yab->yi);}
		yab->yi=yahoo_xmldecode(st);
		 /* printf("got yab-name: %s\n",yab->yi); fflush(stdout); */ 
	} 

	st = strstr(en, "hp=\"");
	if(st) {
		st += strlen("hp=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->hp) {g_free(yab->hp);}
		yab->hp = yahoo_xmldecode(st);
	}

	st = strstr(en, "wp=\"");
	if(st) {
		st += strlen("wp=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wp) {g_free(yab->wp);}
		yab->wp = yahoo_xmldecode(st);
	}

	st = strstr(en, "pa=\"");
	if(st) {
		st += strlen("pa=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->pa) {g_free(yab->pa);}
		yab->pa = yahoo_xmldecode(st);
	}

	st = strstr(en, "fa=\"");
	if(st) {
		st += strlen("fa=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->fa) {g_free(yab->fa);}
		yab->fa = yahoo_xmldecode(st);
	}


		// printf("read-7\n"); fflush(stdout);

	st = strstr(en, "mo=\"");
	if(st) {
		st += strlen("mo=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->mo) {g_free(yab->mo);}
		yab->mo = yahoo_xmldecode(st);
	}

	st = strstr(en, "ot=\"");
	if(st) {
		st += strlen("ot=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->ot) {g_free(yab->ot);}
		yab->ot = yahoo_xmldecode(st);
	}

	st = strstr(en, "e1=\"");
	if(st) {
		st += strlen("e1=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->e1) {g_free(yab->e1);}
		yab->e1 = yahoo_xmldecode(st);
	}

	st = strstr(en, "e2=\"");
	if(st) {
		st += strlen("e2=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->e2) {g_free(yab->e2);}
		yab->e2 = yahoo_xmldecode(st);
	}


	st = strstr(en, "pu=\"");
	if(st) {
		st += strlen("pu=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->pu) {g_free(yab->pu);}
		yab->pu = yahoo_xmldecode(st);
	}

	st = strstr(en, "wu=\"");
	if(st) {
		st += strlen("wu=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wu) {g_free(yab->wu);}
		yab->wu = yahoo_xmldecode(st);
	}

	st = strstr(en, "ti=\"");
	if(st) {
		st += strlen("ti=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->ti) {g_free(yab->ti);}
		yab->ti = yahoo_xmldecode(st);
	}

	st = strstr(en, "co=\"");
	if(st) {
		st += strlen("co=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->co) {g_free(yab->co);}
		yab->co = yahoo_xmldecode(st);
	}


		// printf("read-8\n"); fflush(stdout);

	st = strstr(en, "wa=\"");
	if(st) {
		st += strlen("wa=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wa) {g_free(yab->wa);}
		yab->wa = yahoo_xmldecode(st);
	}

	st = strstr(en, "wc=\"");
	if(st) {
		st += strlen("wc=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wc) {g_free(yab->wc);}
		yab->wc = yahoo_xmldecode(st);
	}

	st = strstr(en, "ws=\"");
	if(st) {
		st += strlen("ws=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->ws) {g_free(yab->ws);}
		yab->ws = yahoo_xmldecode(st);
	}

	st = strstr(en, "wz=\"");
	if(st) {
		st += strlen("wz=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wz) {g_free(yab->wz);}
		yab->wz = yahoo_xmldecode(st);
	}

	st = strstr(en, "wn=\"");
	if(st) {
		st += strlen("wn=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->wn) {g_free(yab->wn);}
		yab->wn = yahoo_xmldecode(st);
	}


		// printf("read-9\n"); fflush(stdout);

	st = strstr(en, "ha=\"");
	if(st) {
		st += strlen("ha=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->ha) {g_free(yab->ha);}
		yab->ha = yahoo_xmldecode(st);
	}

	st = strstr(en, "hc=\"");
	if(st) {
		st += strlen("hc=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->hc) {g_free(yab->hc);}
		yab->hc = yahoo_xmldecode(st);
	}

	st = strstr(en, "hs=\"");
	if(st) {
		st += strlen("hs=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->hs) {g_free(yab->hs);}
		yab->hs = yahoo_xmldecode(st);
	}

	st = strstr(en, "hz=\"");
	if(st) {
		st += strlen("hz=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->hz) {g_free(yab->hz);}
		yab->hz = yahoo_xmldecode(st);
	}

	st = strstr(en, "hn=\"");
	if(st) {
		st += strlen("hn=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->hn) {g_free(yab->hn);}
		yab->hn = yahoo_xmldecode(st);
	}

	st = strstr(en, "bi=\"");
	if(st) {
		st += strlen("bi=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->bi) {g_free(yab->bi);}
		yab->bi = yahoo_xmldecode(st);
	}


		// printf("read-10\n"); fflush(stdout);

	st = strstr(en, "an=\"");
	if(st) {
		st += strlen("an=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->an) {g_free(yab->an);}
		yab->an = yahoo_xmldecode(st);
	}


	st = strstr(en, "c1=\"");
	if(st) {
		st += strlen("c1=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->c1) {g_free(yab->c1);}
		yab->c1 = yahoo_xmldecode(st);
	}

	st = strstr(en, "c2=\"");
	if(st) {
		st += strlen("c2=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->c2) {g_free(yab->c2);}
		yab->c2 = yahoo_xmldecode(st);
	}

	st = strstr(en, "c3=\"");
	if(st) {
		st += strlen("c3=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->c3) {g_free(yab->c3);}
		yab->c3 = yahoo_xmldecode(st);
	}

	st = strstr(en, "c4=\"");
	if(st) {
		st += strlen("c4=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->c4) {g_free(yab->c4);}
		yab->c4 = yahoo_xmldecode(st);
	}

	st = strstr(en, "cm=\"");
	if(st) {
		st += strlen("cm=\"");
		en = strchr(st, '"'); *en++ = '\0';
		if (yab->cm) {g_free(yab->cm);}
		yab->cm = yahoo_xmldecode(st);
	}

		// printf("read-11\n"); fflush(stdout);

}





char *create_yab_xml(struct yahoo_yab *f, int etype)
{
	char *ss=NULL;
	if (!f) {return NULL;}
	if (!f->id) { 
		if (etype != 1) { return NULL;} 
		}
	ss=malloc(2048);
	if (!ss) {return NULL;}

	snprintf(ss, 160,  "<ab k=\"%s\" cc=\"1\">\n<ct ", get_primary_screen_name() );
	lower_str(ss);  /* our screen name must be in lower_case */ 

	if (etype==1) { strcat(ss,"a=\"1\" "); }  /* add */
	if (etype==2) { strcat(ss,"e=\"1\" "); }  /* edit */
	if (etype==3) { strcat(ss,"d=\"1\" "); }  /* delete */


      if (f->yi) {strcat(ss,"yi=\""); strncat(ss, f->yi, 72); strcat(ss, "\" "); }
      if (etype != 1) { if (f->id) {strcat(ss,"id=\""); strncat(ss, f->id, 6); strcat(ss, "\" "); }  }
      if (f->fn)  {strcat(ss,"fn=\""); strncat(ss, f->fn, 26); strcat(ss, "\" "); }
      if (f->mn)  {strcat(ss,"mn=\""); strncat(ss, f->mn, 16); strcat(ss, "\" "); }
      if (f->ln)  {strcat(ss,"ln=\""); strncat(ss, f->ln, 34); strcat(ss, "\" "); }
      if (f->nn)  {strcat(ss,"nn=\""); strncat(ss, f->nn, 26); strcat(ss, "\" "); }
      if (f->e0)  {strcat(ss,"e0=\""); strncat(ss, f->e0, 62); strcat(ss, "\" "); }
      if (f->hp)  {strcat(ss,"hp=\""); strncat(ss, f->hp, 22); strcat(ss, "\" "); }
      if (f->wp) {strcat(ss,"wp=\""); strncat(ss, f->wp, 22); strcat(ss, "\" "); }
      if (f->mo)  {strcat(ss,"mo=\""); strncat(ss, f->mo, 22); strcat(ss, "\" "); }
      if (f->pr)  {strcat(ss,"pr=\""); strncat(ss, f->pr, 6); strcat(ss, "\" "); }
      if (f->ti)  {strcat(ss,"ti=\""); strncat(ss, f->ti, 24); strcat(ss, "\" "); }
      if (f->fa)  {strcat(ss,"fa=\""); strncat(ss, f->fa, 22); strcat(ss, "\" "); }
      if (f->pa)  {strcat(ss,"pa=\""); strncat(ss, f->pa, 22); strcat(ss, "\" "); }
      if (f->ot)  {strcat(ss,"ot=\""); strncat(ss, f->ot, 22); strcat(ss, "\" "); }
      if (f->e1)  {strcat(ss,"e1=\""); strncat(ss, f->e1, 62); strcat(ss, "\" "); }
      if (f->e2)  {strcat(ss,"e2=\""); strncat(ss, f->e2, 62); strcat(ss, "\" "); }
      if (f->pu)  {strcat(ss,"pu=\""); strncat(ss, f->pu, 74); strcat(ss, "\" "); }
      if (f->ha) {strcat(ss,"ha=\""); strncat(ss, f->ha, 48); strcat(ss, "\" "); } 
      if (f->hc)  {strcat(ss,"hc=\""); strncat(ss, f->hc, 20); strcat(ss, "\" "); }
      if (f->hs)  {strcat(ss,"hs=\""); strncat(ss, f->hs, 20); strcat(ss, "\" "); }
      if (f->hz)  {strcat(ss,"hz=\""); strncat(ss, f->hz, 12); strcat(ss, "\" "); }
      if (f->hn)  {strcat(ss,"hn=\""); strncat(ss, f->hn, 18); strcat(ss, "\" "); }
      if (f->co)  {strcat(ss,"co=\""); strncat(ss, f->co, 32); strcat(ss, "\" "); }
      if (f->wa)  {strcat(ss,"wa=\""); strncat(ss, f->wa, 48); strcat(ss, "\" "); }
      if (f->wc)  {strcat(ss,"wc=\""); strncat(ss, f->wc, 20); strcat(ss, "\" "); }
      if (f->ws)  {strcat(ss,"ws=\""); strncat(ss, f->ws, 20); strcat(ss, "\" "); }
      if (f->wz) {strcat(ss,"wz=\""); strncat(ss, f->wz, 12); strcat(ss, "\" "); }
      if (f->wn)  {strcat(ss,"wn=\""); strncat(ss, f->wn, 18); strcat(ss, "\" "); }
      if (f->wu)  {strcat(ss,"wu=\""); strncat(ss, f->wu, 74); strcat(ss, "\" "); }
      if (f->bi)  {strcat(ss,"bi=\""); strncat(ss, f->bi, 12); strcat(ss, "\" "); }
      if (f->an)  {strcat(ss,"an=\""); strncat(ss, f->an, 12); strcat(ss, "\" "); }
      if (f->c1)  {strcat(ss,"c1=\""); strncat(ss, f->c1, 40); strcat(ss, "\" "); }
      if (f->c2) {strcat(ss,"c2=\""); strncat(ss, f->c2, 40); strcat(ss, "\" "); }
      if (f->c3)  {strcat(ss,"c3=\""); strncat(ss, f->c3, 40); strcat(ss, "\" "); }
      if (f->c4)  {strcat(ss,"c4=\""); strncat(ss, f->c4, 40); strcat(ss, "\" "); }
      if (f->cm)  {strcat(ss,"cm=\""); strncat(ss, f->cm, 64); strcat(ss, "\" "); }

	
	strcat(ss, "vm=\"\" />\n</ab>\r\n");

	return ss;
}

void push_yab_status() {
	char countstat[48];
	snprintf(countstat,46,"%d %s", yabcounter, _("address book contacts loaded"));
	gtk_statusbar_pop( GTK_STATUSBAR(chat_status), st_cid );
	gtk_statusbar_push( GTK_STATUSBAR(chat_status), st_cid,countstat); 
}

void parse_yab_entries(char *yabbuf, int url_length)  {
	int pos = 0, end=0;
	
	while(pos < url_length-strlen("<ct")) {
		/* start with <ct */
		while(pos < url_length-strlen("<ct")+1 
			&& memcmp(yabbuf + pos, "<ct", strlen("<ct"))) {
				pos++;
		}
		if(pos >= url_length-1)  {  return; }
		end = pos+2;
		/* end with /> */
		while(end < url_length-strlen("/>")+1 && memcmp(yabbuf + end, "/>", strlen("/>"))) {
	       		end++;
		}
		if(end >= url_length-1) { return; }
		yahoo_yab_read( yabbuf + pos, end+1-pos);
		pos=end;
		pos++;

		/* printf("\nNEW PARSE: %s\n", yabbuf + pos);
		fflush(stdout); */
	}
}


void load_yahoo_addresss_book()  {
	char some_url[168];
	char *yabbuf=NULL;
	int url_length=0;
	yabcounter=0;

	yabbuf=malloc(100000);
	if (!yabbuf) {push_yab_status(); return;}
	set_max_url_fetch_size(100000-3);


	snprintf(some_url, 166, "%s%s",  "http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&diffs=0&t=0&tags=short&rt=0", version_emulation);

	/* snprintf(some_url, 166, "%s%s",  "http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&diffs=1&t=0&tags=short&rt=1083753168&prog-ver=", version_emulation);  */
	url_length = fetch_url( some_url, yabbuf, ymsg_sess->cookie );  
	if(url_length <= strlen("<ct")) {free(yabbuf); push_yab_status(); return;}
	parse_yab_entries(yabbuf, url_length);
	free(yabbuf);
	push_yab_status();
	clear_fetched_url_data();
}



void on_load_yab  (GtkButton *button, gpointer user_data)
{
	empty_yab_book();
	load_yahoo_addresss_book();
	populate_yab_clist();
	unfreeze_yab_address_book();
	addressbook_loaded=1;
}

gboolean on_yab_window_closed  (GtkWidget *widget, GdkEvent *event, gpointer user_data) {
	if ( yab_window) {gtk_widget_destroy( yab_window );}
	yab_window= NULL;
	return( TRUE );
}
gboolean on_yab_window_destroy_cb (GtkButton  *button, gpointer user_data) {
	return on_yab_window_closed  (NULL, NULL, NULL);
}




char *YaENC(const char *incoming) {
	char outgoing[256];
	char *ret=NULL;
	int counter=0;
	int ocounter=0;

	while (counter<strlen(incoming)) {
		if (! incoming[counter]) {break;}
		if (strlen(outgoing)>248) {break;}

		if (incoming[counter]=='&') { 
			outgoing[ocounter++]='&'; 
			outgoing[ocounter++]='a';
			outgoing[ocounter++]='m'; 
			outgoing[ocounter]='p'; 
			outgoing[ocounter++]=';'; 
		}
		else if (incoming[counter]=='>') { 
			outgoing[ocounter++]='&'; 
			outgoing[ocounter++]='g';
			outgoing[ocounter++]='t'; 
			outgoing[ocounter++]=';'; 
		}
		else if (incoming[counter]=='<') { 
			outgoing[ocounter++]='&'; 
			outgoing[ocounter++]='l';
			outgoing[ocounter++]='t'; 
			outgoing[ocounter++]=';'; 
		}
		else if (incoming[counter]=='"') { 
			outgoing[ocounter++]='&'; 
			outgoing[ocounter++]='q';
			outgoing[ocounter++]='u'; 
			outgoing[ocounter]='o'; 
			outgoing[ocounter++]='t'; 
			outgoing[ocounter++]=';'; 
		}
		else { outgoing[ocounter]=incoming[counter]; ocounter++;}
		counter++;
	}
	outgoing[ocounter]='\0';
	ret=g_strdup(outgoing);
	return ret;
}


void widgets_to_yab(GtkWidget *widget, struct yahoo_yab *f) {
	GtkWidget *tmp_widget=NULL;
	char *yi=NULL;
	char *id=NULL;
	char *pr=NULL;
	if (!f) {return;}
	if (!widget) {return;}

	if (f->id) {id=g_strdup(f->id);}
	if (f->yi) {yi=g_strdup(f->yi);}
	if (f->pr) {pr=g_strdup(f->pr);}
	yahoo_yab_free(f);
	f->id=id; f->pr=pr; f->yi=yi;


	if (! f->yi) {
	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "yi");
	if (tmp_widget) { 	f->yi=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}
	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "fn");
	if (tmp_widget) { 	f->fn=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ln");
	if (tmp_widget) { 	f->ln=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "nn");
	if (tmp_widget) { 	f->nn=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "e0");
	if (tmp_widget) { 	f->e0=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hp");
	if (tmp_widget) { 	f->hp=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wp");
	if (tmp_widget) { 	f->wp=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "mo");
	if (tmp_widget) { 	f->mo=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "mn");
	if (tmp_widget) { 	f->mn=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ti");
	if (tmp_widget) { 	f->ti=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "pa");
	if (tmp_widget) { 	f->pa=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "fa");
	if (tmp_widget) { 	f->fa=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ot");
	if (tmp_widget) { 	f->ot=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}


	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "e1");
	if (tmp_widget) { 	f->e1=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "e2");
	if (tmp_widget) { 	f->e2=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "pu");
	if (tmp_widget) { 	f->pu=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ha");
	if (tmp_widget) { 	f->ha=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hc");
	if (tmp_widget) { 	f->hc=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hs");
	if (tmp_widget) { 	f->hs=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hz");
	if (tmp_widget) { 	f->hz=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hn");
	if (tmp_widget) { 	f->hn=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "co");
	if (tmp_widget) { 	f->co=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wa");
	if (tmp_widget) { 	f->wa=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wc");
	if (tmp_widget) { 	f->wc=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ws");
	if (tmp_widget) { 	f->ws=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wz");
	if (tmp_widget) { 	f->wz=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wn");
	if (tmp_widget) { 	f->wn=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wu");
	if (tmp_widget) { 	f->wu=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "bi");
	if (tmp_widget) { 	f->bi=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "an");
	if (tmp_widget) { 	f->an=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "cm");
	if (tmp_widget) { 	f->cm=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "c1");
	if (tmp_widget) { 	f->c1=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "c2");
	if (tmp_widget) { 	f->c2=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "c3");
	if (tmp_widget) { 	f->c3=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "c4");
	if (tmp_widget) { 	f->c4=YaENC(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	}
}


void on_add_edit_yab_entry_cb(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	int adding=0;
	YABBY=gtk_object_get_data(GTK_OBJECT(button), "yab");
	if (!YABBY) {
		adding=1;
		YABBY=yahoo_yab_new();
	}
	if (!YABBY) {return;}
	else {
		char some_url[160];
		int url_length=0;
		char *sendout=NULL;
		char *yabbuf=NULL;
		widgets_to_yab(button, YABBY);
		sendout=create_yab_xml(YABBY, adding?1:2);
		if (! sendout) {return;}
		yabbuf=malloc(8192);
		if (!yabbuf) {return;}
		set_max_url_fetch_size(8000);
		snprintf(some_url, 158, "%s",  "http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&sync=1&tags=short&noclear=1&");
		set_url_post_data(sendout);
		url_length = fetch_url( some_url, yabbuf, ymsg_sess->cookie );  
		free(sendout);
		unset_url_post_data();
		if ( (url_length>2) && (strstr(yabbuf,"<ct ")) && (!strstr(yabbuf,"ae=\"")) ) {
			parse_yab_entries(yabbuf, url_length);
			populate_yab_clist();
			free(yabbuf); yabbuf=NULL;
			clear_fetched_url_data();
		}  else {
			free(yabbuf); yabbuf=NULL;
			clear_fetched_url_data();
			on_load_yab(NULL, NULL);
			show_ok_dialog(_("An error occurred updating the addressbook item."));
		}
		
		if (adding) {
			char *tstr=g_strdup("");
			clear_yab_hash_cb(tstr, YABBY, NULL);
		}
		on_yab_window_destroy_cb (NULL, NULL);
	}
}


void update_my_yab_info(GtkWidget *widget, gpointer user_data) {
	GtkWidget *tmp_widget=NULL;

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "fn");
	if (tmp_widget) { 	
		if (contact_first) {free(contact_first);}
		contact_first=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "ln");
	if (tmp_widget) { 	
		if (contact_last) {free(contact_last);}
		contact_last=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "e0");
	if (tmp_widget) { 	
		if (contact_email) {free(contact_email);}
		contact_email=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "wp");
	if (tmp_widget) { 	
		if (contact_work) {free(contact_work);}
		contact_work=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "hp");
	if (tmp_widget) { 	
		if (contact_home) {free(contact_home);}
		contact_home=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}

	tmp_widget=gtk_object_get_data(GTK_OBJECT(widget), "mo");
	if (tmp_widget) { 	
		if (contact_mobile) {free(contact_mobile);}
		contact_mobile=strdup(gtk_entry_get_text(GTK_ENTRY(tmp_widget))); 	
		}
	write_config();
	on_yab_window_destroy_cb (NULL, NULL);
}



int create_yab_arrays(int mine, struct yahoo_yab *YABBY, int with_breaks) { 
	int junk=0;
	int yy=0;
	for (yy=0; yy<42; yy++) {limits[yy]=40; yabs[yy]=NULL; titles[yy]=NULL; wiggy[yy]=NULL;}

      wiggy[junk]="yi";
      titles[junk]=_("ID:");
      limits[junk]=70;
      if (YABBY) { yabs[junk]=YABBY->yi; }
		junk++;
      wiggy[junk]="fn";
      titles[junk]=_("First Name:");
      limits[junk]=24;
      if (YABBY) { yabs[junk]=YABBY->fn; }
      if (mine) {  if (contact_first) {yabs[junk]=contact_first;}   }
		junk++;
      wiggy[junk]="mn";
      titles[junk]=_("Middle Name:");
      limits[junk]=15;
      if (YABBY) { yabs[junk]=YABBY->mn; }
		junk++;
      wiggy[junk]="ln";
      titles[junk]=_("Last Name:");
      limits[junk]=32;
      if (YABBY) { yabs[junk]=YABBY->ln; }
      if (mine) {  if (contact_last) {yabs[junk]=contact_last;}   }
		junk++;
      wiggy[junk]="nn";
      titles[junk]=_("Nick Name:");
      limits[junk]=24;
      if (YABBY) { yabs[junk]=YABBY->nn; }
		junk++;
      wiggy[junk]="e0";
      titles[junk]=_("E-mail:");
      limits[junk]=60;
      if (YABBY) { yabs[junk]=YABBY->e0; }
      if (mine) {  if (contact_email) {yabs[junk]=contact_email;}   }
		junk++;
      wiggy[junk]="hp";
      titles[junk]=_("Home #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->hp; }
      if (mine) {  if (contact_home) {yabs[junk]=contact_home;}   }
		junk++;

	if (mine) {
      wiggy[junk]="wp";
      titles[junk]=_("Work #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->wp; }
      if (mine) {  if (contact_work) {yabs[junk]=contact_work;}   }
		junk++;
	}

      wiggy[junk]="mo";
      titles[junk]=_("Mobile #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->mo; }
      if (mine) {  if (contact_mobile) {yabs[junk]=contact_mobile;}   }
		junk++;

      wiggy[junk]="fa";
      titles[junk]=_("Fax #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->fa; }
		junk++;
      wiggy[junk]="pa";
      titles[junk]=_("Pager #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->pa; }
		junk++;
      wiggy[junk]="ot";
      titles[junk]=_("Other #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->ot; }
		junk++;
      wiggy[junk]="e1";
      titles[junk]=_("Alt. E-mail:");
      limits[junk]=60;
      if (YABBY) { yabs[junk]=YABBY->e1; }
		junk++;
      wiggy[junk]="e2";
      titles[junk]=_("Alt. E-mail #2:");
      limits[junk]=60;
      if (YABBY) { yabs[junk]=YABBY->e2; }
		junk++;
      wiggy[junk]="pu";
      titles[junk]=_("Website:");
      limits[junk]=72;
      if (YABBY) { yabs[junk]=YABBY->pu; }
		junk++;
      wiggy[junk]="ha";
      titles[junk]=_("Address:");
      limits[junk]=46;
      if (YABBY) { yabs[junk]=YABBY->ha; }
		junk++;
      wiggy[junk]="hc";
      titles[junk]=_("City:");
      limits[junk]=18;
      if (YABBY) { yabs[junk]=YABBY->hc; }
		junk++;
      wiggy[junk]="hs";
      titles[junk]=_("State:");
      limits[junk]=18;
      if (YABBY) { yabs[junk]=YABBY->hs; }
		junk++;
      wiggy[junk]="hz";
      titles[junk]=_("Zip Code:");
      limits[junk]=10;
      if (YABBY) { yabs[junk]=YABBY->hz; }
		junk++;
      wiggy[junk]="hn";
      titles[junk]=_("Country:");
      limits[junk]=16;
      if (YABBY) { yabs[junk]=YABBY->hn; }
		junk++;

	      if (with_breaks) { wiggy[junk]="[break 2]"; junk++; }

      wiggy[junk]="co";
      titles[junk]=_("Company:");
      limits[junk]=30;
      if (YABBY) { yabs[junk]=YABBY->co; }
		junk++;
      wiggy[junk]="ti";
      titles[junk]=_("Title:");
      limits[junk]=22;
      if (YABBY) { yabs[junk]=YABBY->ti; }
		junk++;
      wiggy[junk]="wa";
      titles[junk]=_("Address:");
      limits[junk]=46;
      if (YABBY) { yabs[junk]=YABBY->wa; }
		junk++;

	if (! mine) {
      wiggy[junk]="wp";
      titles[junk]=_("Work #:");
      limits[junk]=20;
      if (YABBY) { yabs[junk]=YABBY->wp; }
		junk++;
	}

      wiggy[junk]="wc";
      titles[junk]=_("City:");
      limits[junk]=18;
      if (YABBY) { yabs[junk]=YABBY->wc; }
		junk++;
      wiggy[junk]="ws";
      titles[junk]=_("State:");
      limits[junk]=18;
      if (YABBY) { yabs[junk]=YABBY->ws; }
		junk++;
      wiggy[junk]="wz";
      titles[junk]=_("Zip Code:");
      limits[junk]=10;
      if (YABBY) { yabs[junk]=YABBY->wz; }
		junk++;
      wiggy[junk]="wn";
      titles[junk]=_("Country:");
      limits[junk]=16;
      if (YABBY) { yabs[junk]=YABBY->wn; }
		junk++;
      wiggy[junk]="wu";
      titles[junk]=_("Website:");
      limits[junk]=72;
      if (YABBY) { yabs[junk]=YABBY->wu; }
		junk++;

	      if (with_breaks) { wiggy[junk]="[break 3]"; junk++; }

      wiggy[junk]="bi";
      titles[junk]=_("Birthday:");
      limits[junk]=10;
      if (YABBY) { yabs[junk]=YABBY->bi; }
		junk++;
      wiggy[junk]="an";
      titles[junk]=_("Anniversary:");
      limits[junk]=10;
      if (YABBY) { yabs[junk]=YABBY->an; }
		junk++;
      wiggy[junk]="cm";
      titles[junk]=_("Notes:");
      limits[junk]=62;
      if (YABBY) { yabs[junk]=YABBY->cm; }
		junk++;
      wiggy[junk]="c1";
      titles[junk]=_("Comments #1:");
      limits[junk]=38;
      if (YABBY) { yabs[junk]=YABBY->c1; }
		junk++;
      wiggy[junk]="c2";
      titles[junk]=_("Comments #2:");
      limits[junk]=38;
      if (YABBY) { yabs[junk]=YABBY->c2; }
		junk++;
      wiggy[junk]="c3";
      titles[junk]=_("Comments #3:");
      limits[junk]=38;
      if (YABBY) { yabs[junk]=YABBY->c3; }
		junk++;
      wiggy[junk]="c4";
      titles[junk]=_("Comments #4:");
      limits[junk]=38;
      if (YABBY) { yabs[junk]=YABBY->c4; }
		junk++;

	wiggy[junk]=NULL;
	yabs[junk]=NULL;
	titles[junk]=NULL;

	return junk;
}


void yab_dump_to_file_cb(gpointer key, gpointer value,
        gpointer user_data) {
		int yy;
		int junk;
		gchar *capb2loc;
		gchar *incoming;
		struct yahoo_yab *YABBY=value;
		FILE *fp=user_data;
		if (!user_data) {return; }
		if (!value) {return;}

		yy=0; junk=0;
		junk=create_yab_arrays( 0 , YABBY, 0);

	while (yy<junk) {
		if (! titles[yy]) {break;}
		if (yabs[yy] && wiggy[yy]) {
			if (strlen(yabs[yy])<1) {yy++; continue; }
			capb2loc=_b2loc(titles[yy]);
			incoming=YaENC(yabs[yy] );
			if ( (!strcmp(wiggy[yy], "wu")) || (!strcmp(wiggy[yy], "pu")) ) {
				fprintf( fp, "<b>%s </b>&nbsp; <a href=\"%s\">%s</a><br>", capb2loc, incoming, incoming);

			} else if ( (!strcmp(wiggy[yy], "e0")) || 
				(!strcmp(wiggy[yy], "e1")) || (!strcmp(wiggy[yy], "e2")) ) {

				fprintf( fp, "<b>%s </b>&nbsp; <a href=\"mailto:%s\">%s</a><br>", capb2loc, incoming, incoming);

			} else if ( (!strcmp(wiggy[yy], "yi")) && (!strstr(yabs[yy], "@")) ) {

				fprintf( fp, "<b>%s </b>&nbsp; <a href=\"http://profiles.yahoo.com/%s\">%s</a><br>", capb2loc, incoming, incoming);

			} else if ( (!strcmp(wiggy[yy], "yi")) && (strstr(yabs[yy], "@")) ) {
				fprintf( fp, "<b>%s </b>&nbsp; <a href=\"mailto:%s\">%s</a><br>", capb2loc, incoming, incoming);

			} else { fprintf( fp, "<b>%s </b>&nbsp; %s<br>", capb2loc, incoming); }

			fflush( fp );
			g_free(incoming);
		}
		yy++;
	}

	fprintf( fp, "<br><hr width=\"40%%\"><br>");
}

void on_dump_yabs_to_file(GtkWidget *button, gpointer user_data) {
	FILE *fp;
	GtkWidget *tmp_widget;
	char *myfile;
	time_t time_llnow= time(NULL);

	      tmp_widget =gtk_object_get_data(GTK_OBJECT(button),"mywindow");
	      if (!tmp_widget) {return;}
	      myfile = strdup(gtk_file_selection_get_filename( GTK_FILE_SELECTION( tmp_widget )));
	      if (strlen(myfile)<1) {free(myfile); return;}

	fp = fopen( myfile, "wb" );
	if ( ! fp )  {
		show_ok_dialog(_("Error opening file for saving."));
		free(myfile);
		return;
				}

		fprintf( fp, "<html><title>Gyach Enhanced AddressBook</title><body bgcolor=\"#FFFFFF\" text=\"#000000\" link=\"#000099\"><br><font size=\"5\"  face=\"Arial\"  color=\"#000099\"><b><a href=\"%s\">Gyach Enhanced</a> AddressBook</b><br>&nbsp; &nbsp; </font><font size=\"4\" face=\"Arial\"  color=\"#000000\">%s<br>&nbsp; <br>&nbsp; <br></font><font size=\"3\" face=\"Arial\"  color=\"#000000\">",   GYACH_URL,  ctime(&time_llnow ) );
		fflush( fp );

	if (! selected_yab) {
	  if (yahoo_yab_entries) {
		g_hash_table_foreach(yahoo_yab_entries, yab_dump_to_file_cb, fp );
	   }
	} else {
		yab_dump_to_file_cb(selected_yab, selected_yab, fp);
	}

		fprintf( fp, "<br><br></font></body></html>");
		fflush( fp );
		fclose(fp);

	     gtk_widget_destroy( tmp_widget);
	     free(myfile);
		show_ok_dialog(_("File successfully saved."));
}

void on_export_yabs_to_file(GtkWidget *button, gpointer user_data) {
	GtkWidget *okbutton;
	char def_filename[256];
	selected_yab=NULL;
	if (user_data) {selected_yab=user_data;}
	if (! selected_yab) {
		 if (! yahoo_yab_entries) { return; }
		if (g_hash_table_size( yahoo_yab_entries ) <1) {return;}
		}
	snprintf( def_filename, 254, "%s/gyachE-AddressBook.html", GYACH_CFG_DIR );
	preselected_filename(def_filename);
	okbutton=create_fileselection2();
  	gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_dump_yabs_to_file),
                      NULL);
}




void show_yab_form(int mine, struct yahoo_yab *YABBY, int editable) {
	char ftite[80];
	GtkWidget *vbox303, *infoframe, *myslabel, *mybox, *mbox, *buttonbox;
	GtkWidget *closeb;
	GtkWidget *mainbox=NULL;
	GtkWidget *myentry=NULL;
	GtkWidget *notebook=NULL;
	GtkWidget *saveb=NULL;
	int yy=0;
	int junk=0;
	int vertcount=0;
	char *contactme=" fn ln e0 mo wp hp ";

  vbox303 = gtk_vbox_new (FALSE, 2);
  yab_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (yab_window), "gyachEDialog", "GyachE"); 
  gtk_window_set_position (GTK_WINDOW (yab_window), GTK_WIN_POS_CENTER);
  gtk_window_set_title (GTK_WINDOW (yab_window), _("Contact Information"));


  snprintf(ftite, 25,  " %s ", _("New")); 
  if (mine) {snprintf(ftite, 78,  " %s ", _("Contact Information:")); }
  else {
	if (YABBY) {
			if (YABBY->yi) {snprintf(ftite,78," %s ",YABBY->yi );}
			else if (YABBY->fn || YABBY->ln) {
				snprintf(ftite, 78,  " %s %s ", 
				YABBY->fn?YABBY->fn:"", 
				YABBY->ln?YABBY->ln:""  );
			} else if (YABBY->co) {snprintf(ftite,78," %s ",YABBY->co ); }  
			else { 	if (YABBY->e0) {snprintf(ftite,78," %s ",YABBY->e0 );}   }
		 }

	}

  infoframe = gtk_frame_new (ftite);
  mbox = gtk_vbox_new (FALSE, 2);
  buttonbox=gtk_hbox_new (FALSE, 2);
 gtk_box_pack_start (GTK_BOX (mbox), infoframe, TRUE, TRUE, 2);
 gtk_box_pack_start (GTK_BOX (mbox), buttonbox, FALSE, FALSE, 2);
  gtk_container_add (GTK_CONTAINER (yab_window), mbox);
  gtk_frame_set_shadow_type (GTK_FRAME (infoframe), GTK_SHADOW_IN );

	if (! mine) {
		mainbox = gtk_hbox_new (FALSE, 3);
  			notebook = gtk_notebook_new ();
  			gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  			gtk_notebook_set_tab_hborder (GTK_NOTEBOOK (notebook), 4);
  			gtk_notebook_set_tab_vborder (GTK_NOTEBOOK (notebook), 1);
			gtk_container_set_border_width (GTK_CONTAINER (notebook), 3);
		gtk_container_add (GTK_CONTAINER (infoframe), notebook );

		gtk_notebook_append_page(GTK_NOTEBOOK(notebook),mainbox,
		gtk_label_new(_("Home"))  );

  		gtk_box_pack_start (GTK_BOX (mainbox), vbox303, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (vbox303), 3);
	} else {
  		gtk_container_add (GTK_CONTAINER (infoframe), vbox303 );
		gtk_container_set_border_width (GTK_CONTAINER (vbox303), 3);
		}


	if ( (! mine)  && (YABBY) ) {		
		saveb=get_pixmapped_button(_("Save As..."), GTK_STOCK_SAVE_AS);
		set_tooltip(saveb,_("Save As..."));
 		gtk_box_pack_start (GTK_BOX (buttonbox), saveb, TRUE, TRUE, 4);
			gtk_signal_connect (GTK_OBJECT (saveb), "clicked",
                      GTK_SIGNAL_FUNC (on_export_yabs_to_file),
                      YABBY);
	}

	if (editable) {		
		saveb=get_pixmapped_button(_("Save Changes"), GTK_STOCK_SAVE);
		set_tooltip(saveb,_("Save Changes"));
 		gtk_box_pack_start (GTK_BOX (buttonbox), saveb, TRUE, TRUE, 4);
		if (YABBY) {
			gtk_object_set_data(GTK_OBJECT(saveb), "yab", YABBY);
		}

		if (! mine) {
  			gtk_signal_connect (GTK_OBJECT (saveb), "clicked",
                      GTK_SIGNAL_FUNC (on_add_edit_yab_entry_cb),
                      NULL);
		} else {
  			gtk_signal_connect (GTK_OBJECT (saveb), "clicked",
                      GTK_SIGNAL_FUNC (update_my_yab_info),
                      NULL);
		}
	}
  

	yy=0;
	sprintf(ftite, "%s", "");
	junk=create_yab_arrays( mine, YABBY, 1);

	while (yy<junk) {
		if (! wiggy[yy]) {break;}

		if ( (!strcmp(wiggy[yy], "[break 3]")) || (!strcmp(wiggy[yy], "[break 2]"))) {
			if (mine) {break;}
		gtk_box_pack_start (GTK_BOX (vbox303), gtk_label_new(" "), TRUE, TRUE, 0);
		vbox303 = gtk_vbox_new (FALSE, 2);
		mainbox = gtk_hbox_new (FALSE, 3);

		if (!strcmp(wiggy[yy], "[break 3]")) {
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),mainbox,
			gtk_label_new(_("Other"))  );
		} else {
			gtk_notebook_append_page(GTK_NOTEBOOK(notebook),mainbox,
			gtk_label_new(_("Work"))  );
		}
  		gtk_box_pack_start (GTK_BOX (mainbox), vbox303, TRUE, TRUE, 0);
		gtk_container_set_border_width (GTK_CONTAINER (vbox303), 3);

			vertcount=0;
			yy++; continue;
		}

		if (mine) {
			if (!strstr(contactme, wiggy[yy])) { yy++; continue;}
		}

		if (vertcount==11) {
			gtk_box_pack_start (GTK_BOX (vbox303), gtk_label_new(" "), TRUE, TRUE, 0);
			vbox303 = gtk_vbox_new (FALSE, 2);
  			gtk_box_pack_start (GTK_BOX (mainbox), vbox303, TRUE, TRUE, 0);
			gtk_container_set_border_width (GTK_CONTAINER (vbox303), 3);
			vertcount=0;
		}

  		mybox = gtk_hbox_new (TRUE, 3);
  		gtk_box_pack_start (GTK_BOX (vbox303), mybox, FALSE, FALSE, 0);
  		myslabel = gtk_label_new (titles[yy]);
  		gtk_box_pack_start (GTK_BOX (mybox), myslabel, TRUE, TRUE, 0);
  		gtk_label_set_justify (GTK_LABEL (myslabel), GTK_JUSTIFY_LEFT);
  		myentry = gtk_entry_new ();
  		gtk_entry_set_max_length(GTK_ENTRY(myentry), limits[yy] );
  		gtk_box_pack_start (GTK_BOX (mybox), myentry, TRUE, TRUE, 0);
		gtk_editable_set_editable(GTK_EDITABLE(myentry),editable?TRUE:FALSE );
		if (! strcmp(wiggy[yy], "yi")) {
			if (YABBY) {gtk_editable_set_editable(GTK_EDITABLE(myentry),FALSE ); }
		}
		gtk_entry_set_text(GTK_ENTRY(myentry), yabs[yy]?yabs[yy]:ftite);
		if (saveb) {YAB_HOOK(saveb,wiggy[yy],myentry);}

		vertcount++;
		yy++;
	}
	
	gtk_box_pack_start (GTK_BOX (vbox303), gtk_label_new(" "), TRUE, TRUE, 0);

	gtk_signal_connect_after (GTK_OBJECT (yab_window), "delete_event",
		GTK_SIGNAL_FUNC (on_yab_window_closed), 	NULL );

	closeb=get_pixmapped_button(_("Cancel"), GTK_STOCK_CLOSE);
	set_tooltip(closeb,_("Cancel"));
 	gtk_box_pack_start (GTK_BOX (buttonbox), closeb, TRUE, TRUE, 4);
  	gtk_signal_connect (GTK_OBJECT (closeb), "clicked",
                      GTK_SIGNAL_FUNC (on_yab_window_destroy_cb),
                      NULL);

	gtk_widget_show_all(yab_window);
}





void on_open_my_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	if (yab_window) {return;}
	show_yab_form(1, YABBY, 1);
}
void on_new_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	if (yab_window) {return;}
	show_yab_form(0, YABBY, 1);
}
void on_show_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	if (yab_window) {return;}
	collect_yab_list_selected();
	if (! yab_row_selected ) { return ;}
	YABBY=yahoo_yab_find(yab_row_selected);
	if (!YABBY) {return;}
	show_yab_form(0, YABBY, 0);
}

void onYabRowActivated(GtkTreeView *tv, GtkTreePath *tp, GtkTreeViewColumn *col, 
	gpointer user_data)  {
		on_show_yab_entry(NULL, NULL);
}

void on_edit_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	if (yab_window) {return;}
	collect_yab_list_selected();
	if (! yab_row_selected ) { return ;}
	YABBY=yahoo_yab_find(yab_row_selected);
	if (!YABBY) {return;}
	show_yab_form(0, YABBY, 1);
}

void on_pm_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	collect_yab_list_selected();
	if (! yab_row_selected ) { return ;}
	YABBY=yahoo_yab_find(yab_row_selected);
	if (!YABBY) {return;}
	if (! YABBY->yi) {return;}
	if (strlen(YABBY->yi)<1) {return;}
	if (strstr(YABBY->yi,"@")) {return;}
	if ( user_selected ) {free(user_selected);}
	user_selected=strdup(YABBY->yi);
	on_pm_activate (NULL, NULL);
}

void on_profile_yab_entry(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	char pr_cmd[80];
	collect_yab_list_selected();
	if (! yab_row_selected ) { return ;}
	YABBY=yahoo_yab_find(yab_row_selected);
	if (!YABBY) {return;}
	if (! YABBY->yi) {return;}
	if (strlen(YABBY->yi)<1) {return;}
	if (strstr(YABBY->yi,"@")) {return;}
	snprintf(pr_cmd, 78, "/profile %s", YABBY->yi); 
	chat_command(pr_cmd);
}


void on_del_yab_entry_cb(GtkWidget *button, gpointer user_data) {
	struct yahoo_yab *YABBY=NULL;
	GtkWidget *tmp_widget=NULL;

	tmp_widget=gtk_object_get_data(GTK_OBJECT(button), "mywindow");
	if (! yab_row_selected ) {  if (tmp_widget) {gtk_widget_destroy(tmp_widget);}   return;}

	YABBY=yahoo_yab_find(yab_row_selected);
	if (!YABBY) {  if (tmp_widget) {gtk_widget_destroy(tmp_widget);}   return;}
	else {
		char some_url[160];
		int url_length=0;
		char *sendout=NULL;
		char *yabbuf=NULL;
		sendout=create_yab_xml(YABBY, 3);
		if (! sendout) {return;}
		yabbuf=malloc(6144);
		if (!yabbuf) { if (tmp_widget) {gtk_widget_destroy(tmp_widget);}  return;}
		set_max_url_fetch_size(5500);
		snprintf(some_url, 158, "%s",  "http://address.yahoo.com/yab/us?v=XM&prog=ymsgr&.intl=us&sync=1&tags=short&noclear=1&");
		set_url_post_data(sendout);
		url_length = fetch_url( some_url, yabbuf, ymsg_sess->cookie );  
		free(sendout);
		unset_url_post_data();
		if ( (url_length>2) && (strstr(yabbuf,"<ct d=\"1\""))  ) {
			char tmp_user[16];
			gpointer key_ptr;
			gpointer value_ptr;
			snprintf(tmp_user, 14,  "%s", YABBY->id);
			if ( g_hash_table_lookup_extended(yahoo_yab_entries, tmp_user, &key_ptr,
				&value_ptr )) 				{
					g_hash_table_remove( yahoo_yab_entries, tmp_user );
					clear_yab_hash_cb(key_ptr,value_ptr , NULL);
											  	}
			populate_yab_clist();
			clear_fetched_url_data();
			free(yabbuf); yabbuf=NULL;
			if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
			return; 
		} else {
			clear_fetched_url_data();
			free(yabbuf); yabbuf=NULL;
			on_load_yab(NULL, NULL);
			show_ok_dialog(_("An error occurred deleting the addressbook item."));
			if (tmp_widget) {gtk_widget_destroy(tmp_widget);}
			return; 
		}

	}
}

void on_del_yab_entry(GtkWidget *button, gpointer user_data) {
	char warnmsg[128];
	GtkWidget *okbutton=NULL;

	collect_yab_list_selected();
	if (! yab_row_selected ) { return ;}
	snprintf(warnmsg, 126, "%s", _("Are you sure you want to DELETE this address book item?"));
	okbutton=show_confirm_dialog(warnmsg,"YES","NO");
	if (!okbutton) {return;}
  	gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      	GTK_SIGNAL_FUNC (on_del_yab_entry_cb), NULL);
}


GtkWidget *get_yab_panel() {
	GtkWidget *mypanel, *yabpanel;
	GtkWidget *hbox1, *scrolledwindow11;
	GtkWidget *editb, *newb, *imb, *viewb, *loadb, *myb, *delb, *profb, *saveab;
	char *col_headers[]={"",NULL};

	mypanel = gtk_hbox_new (FALSE, 0);
	hbox1 = gtk_vbox_new (FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (mypanel), hbox1, FALSE, FALSE,2);

  scrolledwindow11 = gtk_scrolled_window_new (NULL, NULL);
  gtk_box_pack_start (GTK_BOX (mypanel), scrolledwindow11, TRUE, TRUE,2);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow11), GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow11), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  col_headers[0]=_("Name");
  yab_entry_list = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 0, 
	1, col_headers));
  gtk_container_add (GTK_CONTAINER (scrolledwindow11), yab_entry_list);
    g_signal_connect (yab_entry_list, "row-activated", (GCallback) onYabRowActivated,
                      NULL);
	set_basic_treeview_sorting(GTK_WIDGET(yab_entry_list),GYLIST_TYPE_SINGLE);

	gtk_container_set_border_width (GTK_CONTAINER (hbox1), 8);
	newb=get_pixmapped_button(_("New"), GTK_STOCK_NEW);
	set_tooltip(newb,_("New"));
  	gtk_box_pack_start (GTK_BOX (hbox1), newb, FALSE, FALSE, 2);	

	viewb=get_pixmapped_button(_("View"), GTK_STOCK_ZOOM_100);
	set_tooltip(viewb,_("View"));
  	gtk_box_pack_start (GTK_BOX (hbox1), viewb, FALSE, FALSE, 2);	

	editb=get_pixmapped_button(_("Edit"), GTK_STOCK_FIND_AND_REPLACE);
	set_tooltip(editb,_("Edit"));
  	gtk_box_pack_start (GTK_BOX (hbox1), editb, FALSE, FALSE, 2);	


	saveab=get_pixmapped_button(_("Save As..."), GTK_STOCK_SAVE_AS);
	set_tooltip(saveab,_("Save As..."));
  	gtk_box_pack_start (GTK_BOX (hbox1), saveab, FALSE, FALSE, 2);

	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(" "), FALSE, FALSE, 0);

	delb=get_pixmapped_button(_("Delete"), GTK_STOCK_DIALOG_ERROR);
	set_tooltip(delb,_("Delete"));
  	gtk_box_pack_start (GTK_BOX (hbox1), delb, FALSE, FALSE, 2);


	gtk_box_pack_start (GTK_BOX (hbox1), gtk_hseparator_new(), FALSE, FALSE, 6);

	imb=get_pixmapped_button(_("IM"), GTK_STOCK_COPY);
	set_tooltip(imb,_("IM"));
  	gtk_box_pack_start (GTK_BOX (hbox1), imb, FALSE, FALSE, 2);	

	profb=get_pixmapped_button(_("Profile"), GTK_STOCK_DIALOG_INFO);
	set_tooltip(profb,_("Profile"));
  	gtk_box_pack_start (GTK_BOX (hbox1), profb, FALSE, FALSE, 2);	

	gtk_box_pack_start (GTK_BOX (hbox1), gtk_hseparator_new(), FALSE, FALSE, 6);

	myb=get_pixmapped_button(_("My Info"), GTK_STOCK_SELECT_COLOR);
	set_tooltip(myb,_("My Info"));
  	gtk_box_pack_start (GTK_BOX (hbox1), myb, FALSE, FALSE, 2);	

	gtk_box_pack_start (GTK_BOX (hbox1), gtk_hseparator_new(), FALSE, FALSE, 6);

	loadb=get_pixmapped_button(_("Load"), GTK_STOCK_REFRESH);
	set_tooltip(loadb,_("Load"));
  	gtk_box_pack_start (GTK_BOX (hbox1), loadb, FALSE, FALSE, 2);	

	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("   "), TRUE, TRUE,2);


  gtk_signal_connect (GTK_OBJECT (saveab), "clicked",
                      GTK_SIGNAL_FUNC (on_export_yabs_to_file),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (loadb), "clicked",
                      GTK_SIGNAL_FUNC (on_load_yab),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (imb), "clicked",
                      GTK_SIGNAL_FUNC (on_pm_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (delb), "clicked",
                      GTK_SIGNAL_FUNC (on_del_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (profb), "clicked",
                      GTK_SIGNAL_FUNC (on_profile_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (newb), "clicked",
                      GTK_SIGNAL_FUNC (on_new_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (editb), "clicked",
                      GTK_SIGNAL_FUNC (on_edit_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (viewb), "clicked",
                      GTK_SIGNAL_FUNC (on_show_yab_entry),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (myb), "clicked",
                      GTK_SIGNAL_FUNC (on_open_my_yab_entry),
                      NULL);

	yab_widget_newb=newb;
	yab_widget_viewb=viewb;
	yab_widget_editb=editb;
	yab_widget_delb=delb;
	yab_widget_profb=profb;
	yab_widget_imb=imb;
	yab_widget_myb=myb;
	yab_widget_loadb=loadb;
	yab_widget_saveab=saveab;

	gtk_container_set_border_width(GTK_CONTAINER(mypanel),3);

		yabpanel = gtk_vbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (yabpanel), gtk_label_new(_("Address Book")), FALSE, FALSE,1);
		gtk_box_pack_start (GTK_BOX (yabpanel), mypanel, TRUE, TRUE,2);

	gtk_widget_show_all(yabpanel);
	return yabpanel;
}


void freeze_yab_address_book() {
	if (yab_entry_list) {
		empty_yab_book();
		populate_yab_clist();
	}
	if (yab_widget_newb) {gtk_widget_set_sensitive(yab_widget_newb, FALSE);}
	if (yab_widget_viewb) {gtk_widget_set_sensitive(yab_widget_viewb, FALSE);}
	if (yab_widget_editb) {gtk_widget_set_sensitive(yab_widget_editb, FALSE);}
	if (yab_widget_delb) {gtk_widget_set_sensitive(yab_widget_delb, FALSE);}
	if (yab_widget_profb) {gtk_widget_set_sensitive(yab_widget_profb, FALSE);}
	if (yab_widget_imb) {gtk_widget_set_sensitive(yab_widget_imb, FALSE);}
	if (yab_widget_loadb) {gtk_widget_set_sensitive(yab_widget_loadb, FALSE);}
	if (yab_entry_list) {gtk_widget_set_sensitive(yab_entry_list, FALSE);}
	if (yab_widget_saveab) {gtk_widget_set_sensitive(yab_widget_saveab, FALSE);}
	addressbook_loaded=0;
}
void enable_yab_address_book() {
	addressbook_loaded=0;
	if (yab_widget_loadb) {gtk_widget_set_sensitive(yab_widget_loadb, TRUE);}
	if (yab_widget_myb) {gtk_widget_set_sensitive(yab_widget_myb, TRUE);}
}

void unfreeze_yab_address_book() {
	if (yab_widget_saveab) {gtk_widget_set_sensitive(yab_widget_saveab, TRUE);}
	if (yab_widget_newb) {gtk_widget_set_sensitive(yab_widget_newb, TRUE);}
	if (yab_widget_viewb) {gtk_widget_set_sensitive(yab_widget_viewb, TRUE);}
	if (yab_widget_editb) {gtk_widget_set_sensitive(yab_widget_editb, TRUE);}
	if (yab_widget_delb) {gtk_widget_set_sensitive(yab_widget_delb, TRUE);}
	if (yab_widget_profb) {gtk_widget_set_sensitive(yab_widget_profb, TRUE);}
	if (yab_widget_imb) {gtk_widget_set_sensitive(yab_widget_imb, TRUE);}
	if (yab_widget_myb) {gtk_widget_set_sensitive(yab_widget_myb, TRUE);}
	if (yab_widget_loadb) {gtk_widget_set_sensitive(yab_widget_loadb, TRUE);}
	if (yab_entry_list) {gtk_widget_set_sensitive(yab_entry_list, TRUE);}
}


void search_open_yab_cb(gpointer key, gpointer value,
        gpointer user_data) {

	struct yahoo_yab *YABBY=value;		
		if (! key) {return;}
		if (! value) {return;}		
		if (! YABBY) {return;}
		if (! YABBY->yi) {return;}
	if (! strcasecmp(yab_search_term, YABBY->yi)) {
	    if (! yab_window) {
		show_yab_form(0, YABBY, 1);
		yab_search_shown=1;
		}
	}
}

void search_open_yab(char *user) {
	if (! enable_addressbook) {return;}
	if (! user) {return;}
	if (strlen(user)<1) {return;}
	if (yab_window) {return;}
	if (!yahoo_yab_entries) {on_load_yab  (NULL, NULL);}
	if (! yahoo_yab_entries) {return;}
	if (! addressbook_loaded)  {on_load_yab  (NULL, NULL);}
	if (yab_search_term) {free(yab_search_term);}
	yab_search_term=strdup(user);
	yab_search_shown=0;
	g_hash_table_foreach(yahoo_yab_entries, search_open_yab_cb, NULL );
	if (! yab_search_shown) {
		struct yahoo_yab *YABBY=NULL;
		show_yab_form(0, YABBY, 1);
	}
}

