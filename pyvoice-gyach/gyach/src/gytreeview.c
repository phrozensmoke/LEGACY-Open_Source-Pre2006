/*****************************************************************************
 * gytreeview.c
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
 * Wrappers for GtkTreeView stuff
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include "gytreeview.h"


  gint sort_iter_compare_func (GtkTreeModel *model,
                          GtkTreeIter  *a,
                          GtkTreeIter  *b,
                          gpointer      userdata)
  {
    gint sortcol = GPOINTER_TO_INT(userdata);
    gint ret = 0;
   gchar *name1, *name2;

   gtk_tree_model_get(model, a, sortcol, &name1, -1);
   gtk_tree_model_get(model, b, sortcol, &name2, -1);

        if (name1 == NULL || name2 == NULL)        {
          if (name1 == NULL && name2 == NULL) { ret=0;}
          else {ret = (name1 == NULL) ? -1 : 1; }
        }  else {
	 ret=(gint) strcasecmp(name1,name2);

	/* Below is what the docs suggest using, but the way 
	   it sorts is sometimes illogical and just flat out sucks */
          /*  ret = g_utf8_collate(name1,name2);  */

        }

  g_free(name1);
  g_free(name2); 
  return ret;
  }


GtkTreeModel *freeze_treeview(GtkWidget *view) {
	GtkTreeModel *model;
 	model = gtk_tree_view_get_model(GTK_TREE_VIEW(view));
  	g_object_ref(model); /* Make sure the model stays with us after the tree view unrefs it */
  	gtk_tree_view_set_model(GTK_TREE_VIEW(view), NULL); /* Detach model from view */
	return model;
}

void unfreeze_treeview(GtkWidget *view, GtkTreeModel *model) {
  	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model); /* Re-attach model to view */
  	g_object_unref(model);
}

/* set up sorting for basic 1-column lists and trees */

void set_basic_treeview_sorting(GtkWidget *widgy, int gylist_type) {
	GtkTreeModel *model;
	GtkTreeSortable *sortable;
	gint sort_col=1;
 	model = gtk_tree_view_get_model(GTK_TREE_VIEW(widgy));
	sortable = GTK_TREE_SORTABLE(model);

	switch (gylist_type) {
		case GYLIST_TYPE_DOUBLE: 
			sort_col=GYDOUBLE_LABEL;
			break;
		case GYLIST_TYPE_TRIPLE: 
			sort_col=GYTRIPLE_COL1;
			break;
		case GYLIST_TYPE_QUAD: 
			sort_col=GYQUAD_COL1;
			break;
		case GYLIST_TYPE_ROOM: 
			sort_col=GYROOM_COL1;
			break;
		case GYLIST_TYPE_SINGLE: 
		default:
			sort_col=GYSINGLE_COL1;
			break;
	}

    	gtk_tree_sortable_set_sort_func(sortable, sort_col, sort_iter_compare_func,
     	GINT_TO_POINTER(sort_col), NULL);

	if (gylist_type==GYLIST_TYPE_QUAD) {
    		gtk_tree_sortable_set_sort_func(sortable, GYQUAD_COL2, sort_iter_compare_func,
     		GINT_TO_POINTER(GYQUAD_COL2), NULL);
	}

    	/* set initial sort order */
    	gtk_tree_sortable_set_sort_column_id(sortable, sort_col, GTK_SORT_ASCENDING);
}


gboolean on_gytreeview_col_clicked(GtkTreeViewColumn *col, gpointer user_data) {
	if (!col) {return FALSE;}
	gtk_tree_view_column_set_sort_indicator(col, FALSE);
	return FALSE;
}



GtkTreeView *create_gy_treeview(int view_type, int gylist_type, int headers_visible, 
	int with_pixbuf,  char **col_headers	) {

  GtkTreeViewColumn   *col;
  GtkCellRenderer     *renderer;
  GtkWidget           *view;
  GtkTreeModel        *model=NULL;
  int pix_int=0, pix_vis=0, col1=0, col2=0, col3=0, col4=0;
  int pack_pixbuf=0;
  char **heady=col_headers;

 	view = gtk_tree_view_new();
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(view), headers_visible);

	switch (gylist_type)  {

		case GYLIST_TYPE_DOUBLE:
			pix_int=GYDOUBLE_PIX;
			pix_vis=GYDOUBLE_PIX_TOGGLE;
			col1=GYDOUBLE_LABEL; col2=GYDOUBLE_COL2;
			if (view_type==GYTV_TYPE_TREE) {
  				model = GTK_TREE_MODEL(
						gtk_tree_store_new(
							GYDOUBLE_NUMCOLS+1,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							GDK_TYPE_COLOR,  
							G_TYPE_STRING,
                                 	G_TYPE_STRING, 
							G_TYPE_STRING, 
							G_TYPE_STRING ,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,  
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING ,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING ,


							GDK_TYPE_COLOR /* background color */
							));				
			} else {
  				model = GTK_TREE_MODEL(
						gtk_tree_store_new(
							GYDOUBLE_NUMCOLS,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							GDK_TYPE_COLOR,  
							G_TYPE_STRING,
                                 	G_TYPE_STRING, 
							G_TYPE_STRING, 
							G_TYPE_STRING ,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, 
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF,  
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING ,
							G_TYPE_BOOLEAN, GDK_TYPE_PIXBUF, G_TYPE_STRING 
							));				
			}
			pack_pixbuf=with_pixbuf;
			break;

		case GYLIST_TYPE_TRIPLE:
			col1=GYTRIPLE_COL1; col2=GYTRIPLE_COL2; col3=GYTRIPLE_COL3;
			if (view_type==GYTV_TYPE_TREE) {
  				model = GTK_TREE_MODEL(
				gtk_tree_store_new(GYTRIPLE_NUMCOLS, G_TYPE_STRING,
				G_TYPE_STRING,G_TYPE_STRING));	
			} else {
  				model = GTK_TREE_MODEL( 
				gtk_list_store_new(GYTRIPLE_NUMCOLS,G_TYPE_STRING,
				G_TYPE_STRING,G_TYPE_STRING));	
			}
			pack_pixbuf=0;
			break;

		case GYLIST_TYPE_QUAD:
			pix_int=GYQUAD_PIX;
			pix_vis=GYQUAD_PIX_TOGGLE;
			col2=GYQUAD_COL1; col3=GYQUAD_COL2; col4=GYQUAD_COL3;
			if (view_type==GYTV_TYPE_TREE) {
  				model = GTK_TREE_MODEL(
						gtk_tree_store_new(
							GYQUAD_NUMCOLS,	G_TYPE_BOOLEAN,GDK_TYPE_PIXBUF,
							GDK_TYPE_COLOR,
                                 	G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING  ));	
			} else {
  				model = GTK_TREE_MODEL(
						gtk_list_store_new(
							GYQUAD_NUMCOLS,	G_TYPE_BOOLEAN,GDK_TYPE_PIXBUF,
							GDK_TYPE_COLOR,
                                 	G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING  ));	
			}
			pack_pixbuf=0;
			break;

		case GYLIST_TYPE_ROOM:
			pix_int=GYROOM_PIX;
			pix_vis=GYROOM_PIX_TOGGLE;
			col1=GYROOM_COL1; 
			if (view_type==GYTV_TYPE_TREE) {
  				model = GTK_TREE_MODEL(
						gtk_tree_store_new(
							GYROOM_NUMCOLS,
							G_TYPE_BOOLEAN,
							GDK_TYPE_PIXBUF, 
                                 	GDK_TYPE_COLOR, G_TYPE_STRING, G_TYPE_POINTER  ));		
			} else {
  				model = GTK_TREE_MODEL(
						gtk_list_store_new(
							GYROOM_NUMCOLS,
							G_TYPE_BOOLEAN,
							GDK_TYPE_PIXBUF, 
                                 	GDK_TYPE_COLOR, G_TYPE_STRING, G_TYPE_POINTER  ));		
			}
			pack_pixbuf=with_pixbuf;
			break;		


		case GYLIST_TYPE_SINGLE:
		default:
			pix_int=GYSINGLE_PIX;
			pix_vis=GYSINGLE_PIX_TOGGLE;
			col1=GYSINGLE_COL1; 
			if (view_type==GYTV_TYPE_TREE) {
  				model = GTK_TREE_MODEL(
						gtk_tree_store_new(
							GYSINGLE_NUMCOLS,
							G_TYPE_BOOLEAN,
							GDK_TYPE_PIXBUF, 
                                 	GDK_TYPE_COLOR, G_TYPE_STRING, G_TYPE_STRING  ));				
			} else {
  				model = GTK_TREE_MODEL(
						gtk_list_store_new(
							GYSINGLE_NUMCOLS,
							G_TYPE_BOOLEAN,
							GDK_TYPE_PIXBUF, 
                                 	GDK_TYPE_COLOR, G_TYPE_STRING, G_TYPE_STRING  ));		
			}
			pack_pixbuf=with_pixbuf;
			break;			

	}  /* end switch */



	/* Column 1 - all list/trees will have it */
  	col = gtk_tree_view_column_new();
		if (gylist_type==GYLIST_TYPE_QUAD) {
			gtk_tree_view_column_set_reorderable(col, TRUE);
		}
	gtk_tree_view_column_set_title(col, *heady?*heady:"");
	gtk_tree_view_column_set_alignment(col , 0.03);



	if (pack_pixbuf || (gylist_type==GYLIST_TYPE_QUAD) ) {
  		renderer = gtk_cell_renderer_pixbuf_new();	
 		gtk_tree_view_column_pack_start(col, renderer, FALSE);
		gtk_tree_view_column_set_attributes (col, renderer, 
     		"pixbuf", pix_int,"visible", pix_vis,NULL);
		g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 1, NULL);
	}

		/* avatar on buddy lists */
		if ( (view_type==GYTV_TYPE_TREE) && 
			(gylist_type == GYLIST_TYPE_DOUBLE) && pack_pixbuf ) { 
				gtk_tree_view_column_set_alignment(col , 0.02);
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer, 
     				"pixbuf", GYDOUBLE_AVA,"visible", GYDOUBLE_AVA_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 2, "yalign", 0.5,  NULL);
		}


	if (gylist_type != GYLIST_TYPE_QUAD) {
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,"editable",FALSE, NULL);
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		gtk_tree_view_column_set_attributes (col, renderer, 
     			"text", col1,
     			NULL);
		if (gylist_type == GYLIST_TYPE_SINGLE) {
			gtk_tree_view_column_add_attribute (col, renderer, 
     				"foreground-gdk", GYSINGLE_COLOR);
			}
		if (gylist_type == GYLIST_TYPE_DOUBLE) {
			gtk_tree_view_column_add_attribute (col, renderer, 
     				"foreground-gdk", GYDOUBLE_COLOR);

			if (gylist_type == GYLIST_TYPE_DOUBLE) {
			gtk_tree_view_column_add_attribute (col, renderer, 
     				"background-gdk", GYDOUBLE_NUMCOLS);
				}
			}
		if (gylist_type == GYLIST_TYPE_ROOM) {
			gtk_tree_view_column_add_attribute (col, renderer, 
     				"foreground-gdk", GYROOM_COLOR);
			}
		g_object_set(renderer, "ypad", 0, "yalign", 0.5, NULL);
		g_object_set(renderer, "xalign", 0.0, "ypad", 0, "xpad", 2, NULL);


		if ( (view_type==GYTV_TYPE_TREE) && 
			(gylist_type == GYLIST_TYPE_DOUBLE) ) { 
				renderer = gtk_cell_renderer_text_new();
				g_object_set(renderer,"editable",FALSE, "text", "   ", NULL);
				gtk_tree_view_column_pack_start(col, renderer, TRUE);
		}

	}



  	gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
	gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	if (gylist_type != GYLIST_TYPE_QUAD) {gtk_tree_view_column_set_resizable(col, TRUE);}


		if ( (view_type==GYTV_TYPE_TREE) && 
			(gylist_type == GYLIST_TYPE_DOUBLE)  ) { 
				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_STEALTH_PIX,"visible",
					 GYDOUBLE_STEALTH_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 8, "yalign", 0.5, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_SMS_PIX,"visible",
					 GYDOUBLE_SMS_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 1, "yalign", 0.5, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_CHAT_PIX,"visible",
					 GYDOUBLE_CHAT_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 1, "yalign", 0.5, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_WEBCAM_PIX,"visible",
					 GYDOUBLE_WEBCAM_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 1, "yalign", 0.5, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		}


	/* Column 2, if any */
	if (gylist_type >= GYLIST_TYPE_DOUBLE) {
  		col = gtk_tree_view_column_new();
		if (gylist_type==GYLIST_TYPE_QUAD) {
			gtk_tree_view_column_set_sort_column_id(col, col2);
			g_signal_connect (G_OBJECT (col), "clicked", G_CALLBACK (on_gytreeview_col_clicked), NULL);
			gtk_tree_view_column_set_reorderable(col, TRUE);
		}
		gtk_tree_view_column_set_alignment(col , 0.03);
		if (*heady) {heady++;}
		gtk_tree_view_column_set_title(col, *heady?*heady:"");
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,"editable",FALSE, NULL);

		if ( (view_type==GYTV_TYPE_TREE) && 
			(gylist_type == GYLIST_TYPE_DOUBLE) ) { 
			g_object_set(renderer,"editable",TRUE, NULL);
		}


		gtk_tree_view_column_pack_start(col, renderer, (gylist_type !=  GYLIST_TYPE_DOUBLE)?TRUE:FALSE);
		gtk_tree_view_column_set_attributes (col, renderer, 
     			"text", col2,
     			NULL);

		if (gylist_type == GYLIST_TYPE_QUAD) {
			gtk_tree_view_column_add_attribute (col, renderer, 
     				"foreground-gdk", GYQUAD_COLOR);
			}

		g_object_set(renderer, "ypad", 0, "yalign", 0.5, NULL);

		if (gylist_type == GYLIST_TYPE_DOUBLE) {		
			g_object_set(renderer, "xalign", 0.00, "ypad", 0, "xpad",8, NULL);
				gtk_tree_view_column_add_attribute (col, renderer, 
     				"foreground-gdk", GYDOUBLE_COLOR);
		} else	 {
			g_object_set(renderer, "xalign", 0.0, "ypad", 0, "xpad", 2, NULL);
					}

  		gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

		gtk_tree_view_column_set_resizable(col, TRUE);

		if ((gylist_type==GYLIST_TYPE_QUAD)) {
			gtk_tree_view_column_set_clickable(col, TRUE);
		} else {gtk_tree_view_column_set_clickable(col, FALSE);}


	}  /* end column 2 */


		if ( (view_type==GYTV_TYPE_TREE) && 
			(gylist_type == GYLIST_TYPE_DOUBLE)  ) { 
				GdkColor focolor;
				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_text_new();
				g_object_set(renderer,"editable",FALSE, "text", "  ", NULL);
				gtk_tree_view_column_pack_start(col, renderer, FALSE);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

				/* game status */
				col = gtk_tree_view_column_new();
				focolor.red   = 0x7e * ( 65535 / 255 );
				focolor.green = 0x77 * ( 65535 / 255 );
				focolor.blue  = 0xa9 * ( 65535 / 255 );
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_GAMES_PIX,"visible",
					 GYDOUBLE_GAMES_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 5, "yalign", 0.5, NULL);
				renderer = gtk_cell_renderer_text_new();
				gtk_tree_view_column_pack_start(col, renderer, TRUE);
				g_object_set(renderer,"editable",FALSE, NULL);
				gtk_tree_view_column_set_attributes (col, renderer, 
     					"text", GYDOUBLE_GAMES_COL,"visible",
					 GYDOUBLE_GAMES_TOGGLE,
     					NULL);
				g_object_set(renderer, "ypad", 0, "yalign", 0.5, "xalign", 0.0, NULL);
				g_object_set(renderer,"foreground-gdk", &focolor, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

				/* launchast status */
				col = gtk_tree_view_column_new();
				renderer = gtk_cell_renderer_pixbuf_new();	
 				gtk_tree_view_column_pack_start(col, renderer, FALSE);
				gtk_tree_view_column_set_attributes (col, renderer,      												"pixbuf", GYDOUBLE_LC_PIX,"visible",
					 GYDOUBLE_LC_TOGGLE,NULL);
				g_object_set(renderer, "xalign", 0.0, "ypad", 1, "xpad", 5,"yalign", 0.5,  NULL);
				renderer = gtk_cell_renderer_text_new();
				gtk_tree_view_column_pack_start(col, renderer, TRUE);
				g_object_set(renderer,"editable",FALSE, NULL);
				gtk_tree_view_column_set_attributes (col, renderer, 
     					"text", GYDOUBLE_LC_COL, "visible",
					 GYDOUBLE_LC_TOGGLE, 
     					NULL);
				g_object_set(renderer, "ypad", 0, "yalign", 0.5,  "xalign", 0.0, NULL);
				g_object_set(renderer,"foreground-gdk", &focolor, NULL);
  				gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
				gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		}



	/* Column 3, if any */
	if (gylist_type >= GYLIST_TYPE_TRIPLE) {
  		col = gtk_tree_view_column_new();
		if (gylist_type==GYLIST_TYPE_QUAD) {
			gtk_tree_view_column_set_sort_column_id(col, col3);
			g_signal_connect (G_OBJECT (col), "clicked", G_CALLBACK (on_gytreeview_col_clicked), NULL);
			gtk_tree_view_column_set_reorderable(col, TRUE);
		}
		gtk_tree_view_column_set_alignment(col , 0.03);
		if (*heady) {heady++;}
		gtk_tree_view_column_set_title(col, *heady?*heady:"");
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,"editable",FALSE, NULL);
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		gtk_tree_view_column_set_attributes (col, renderer, 
     			"text", col3,
     			NULL);
		g_object_set(renderer, "ypad", 0, "yalign", 0.5, NULL);
		g_object_set(renderer, "xalign", 0.0, "ypad", 0, "xpad", 2, NULL);
  		gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_resizable(col, TRUE);
		if ((gylist_type==GYLIST_TYPE_QUAD)) {
			gtk_tree_view_column_set_clickable(col, TRUE);
		} else {gtk_tree_view_column_set_clickable(col, FALSE);}
	}

	/* Column 4, if any */
	if (gylist_type >= GYLIST_TYPE_QUAD) {
  		col = gtk_tree_view_column_new();
		if (gylist_type==GYLIST_TYPE_QUAD) {
			gtk_tree_view_column_set_reorderable(col, TRUE);
		}
		gtk_tree_view_column_set_alignment(col , 0.03);
		if (*heady) {heady++;}
		gtk_tree_view_column_set_title(col, *heady?*heady:"");
		renderer = gtk_cell_renderer_text_new();
		g_object_set(renderer,"editable",FALSE, NULL);
		gtk_tree_view_column_pack_start(col, renderer, TRUE);
		gtk_tree_view_column_set_attributes (col, renderer, 
     			"text", col4,
     			NULL);
		g_object_set(renderer, "ypad", 0, "yalign", 0.5, NULL);
		g_object_set(renderer, "xalign", 0.0, "ypad", 0, "xpad", 2, NULL);
  		gtk_tree_view_append_column(GTK_TREE_VIEW(view), col);
		gtk_tree_view_column_set_sizing(col, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
		gtk_tree_view_column_set_resizable(col, TRUE);
	}

  	gtk_tree_view_set_model(GTK_TREE_VIEW(view), model);
  	g_object_unref(model); /* destroy model automatically with view */
  	gtk_tree_selection_set_mode(gtk_tree_view_get_selection(GTK_TREE_VIEW(view)), GTK_SELECTION_SINGLE);
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(view));

	g_object_set(view,"enable-search",FALSE, NULL);
	g_object_set(view,"rules-hint",FALSE, NULL);
	g_object_set(view,"reorderable",FALSE, NULL);

	return GTK_TREE_VIEW(view);
}

void gy_treeview_adjust_columns(GtkWidget *view) {
	gtk_tree_view_columns_autosize(GTK_TREE_VIEW(view));
}

void gy_empty_model(GtkTreeModel *tree, int gylist_type) {
	if (gylist_type==GYTV_TYPE_TREE) {
		gtk_tree_store_clear(GTK_TREE_STORE(tree));
	}
	if (gylist_type==GYTV_TYPE_LIST) {
		gtk_list_store_clear(GTK_LIST_STORE(tree));
	}
}



