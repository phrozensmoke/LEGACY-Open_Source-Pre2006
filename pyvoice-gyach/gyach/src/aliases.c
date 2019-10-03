/*****************************************************************************
 * aliases.c
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

#include "gyach.h"
#include "aliases.h"
#include "commands.h"
#include "interface.h"
#include "main.h"
#include "users.h"
#include "util.h"
#include "yahoochat.h"
#include "gytreeview.h"

int  alias_count;
int  alias_selected = -1;

GtkTreeView *chat_aliases;
GtkWidget *alias_menu;
GtkWidget *alias_window;
GtkTreeIter alias_find_iter;

#define GYACH_CMD_SEP "<;>"

/*********************************/
/* Default aliases list */
/*********************************/
struct ALIAS {
	char *alias_name;
	char *cmd_value1;
	char *cmd_value2;
};

static struct ALIAS aliases[] = {
	{ "Agree",	"agrees whole-heartedly.", "agrees with $0." },
	{ "Apologize",	"apologizes.", "apologizes to $0." },
	{ "Applaud",	"applauds fervently.", "applauds $0 fervently hmmm." },
	{ "Beam",	"beams.", "beams at $0." },
	{ "Beg",	"begs like a dog.", "begs $0." },
	{ "Blush",	"blushes in embarassment.", "blushes at $0 in embarassment." },
	{ "Bow",	"bows gracefully.", "bows to $0 gracefully." },
	{ "Cheer",	"cheers enthusiastically.", "cheers $0 enthusiastically." },
	{ "Clap",	"claps briefly.", "claps for $0 briefly." },
	{ "Disagree",	"disagrees.", "disagrees with $0." },
	{ "Faint",	"faints dead away.", "faints dead away at the sight of $0's." },
	{ "Flirt",	"flirts with the room.", "flirts with $0." },
	{ "Grin",	"grins evilly.", "grins at $0 evilly." },
	{ "Grovel",	"grovels.", "grovels at $0's feet." },
	{ "Hug",	"waves Hi to everyone.", "gives $0 a great big hug." },
	{ "Kiss",	"kisses.", "kisses $0." },
	{ "Laugh",	"laughs hysterically.", "laughs at $0." },
	{ "Listen",	"listens.", "listens to $0 with undivided attention." },
	{ "Melt",	"melts.", "melts in $0's arms." },
	{ "Misgrin",	"grins mischievously.", "grins at $0 mischievously." },
	{ "Nod",	"nods solemnly.", "nods to $0 approvingly." },
	{ "Nudge",	"nudges everyone in the side.", "nudges $0 in the side." },
	{ "Pout",	"pouts.", "pouts at $0." },
	{ "Sit",	"sits down.", "sits down next to $0." },
	{ "Smile",	"smiles.", "smiles at $0." },
	{ "Stand",	"stands up.", "stands up next to $0." },
	{ "Wave",	"waves at everyone.", "waves at $0." },
	{ NULL, NULL, NULL }
};

/*
 * alias datafile fmt is "alias:everyone|individual"
 * see http://www.edge-op.org/files/chatstandard for more info
 */
void read_aliases() {
	GtkTreeModel *freezer;
	GtkTreeIter iter;
	FILE *fp;
	struct ALIAS *alias;
	char line[1024];
	char tmp_line[2048];
	char *alias_name = NULL;
	char *cmd_value1 = NULL;
	char *cmd_value2 = NULL;
	char *ptr = NULL;
	char *ptr2 = NULL;
	char alias_filename[192] = "";
	char empty_value[1] = "";
	int old_format = 0;


	freezer=freeze_treeview(GTK_WIDGET(chat_aliases));

	/* remove any items already in the list */
	gy_empty_model(freezer, GYTV_TYPE_LIST);

	alias_name = line;

	snprintf( alias_filename, 190, "%s/commands", GYACH_CFG_COMMON_DIR );

	fp = fopen( alias_filename, "r" );

	alias_count = 0;

	if ( fp ) {
		while( fgets( line, 1023, fp )) {
			ptr = skip_whitespace( line );
			if ( *ptr == '#' ) {
				continue;
			}

			if (( ! strchr( line, ']' )) &&
				( ! strchr( line, ':' ))) {
				continue;
			}

			while(( line[strlen(line)-1] == '\r' ) ||
				  ( line[strlen(line)-1] == '\n' )) {
				line[strlen(line)-1] = '\0';
			}

			/* stuff to convert from cheetachat format I think */
			/* should be able to delete this */
			while(( ptr = strstr( line, "%N" )) != NULL ) {
				*ptr = '$';
				*(ptr+1) = '0';
			}

			alias_count++;

			ptr = line;
			while(( *ptr != ']' ) &&
				( *ptr != ':' )) {
				*ptr = tolower( *ptr );
				ptr++;
			}

			if ( ! old_format ) {
				ptr = strchr( line, ':' );
				ptr2 = strchr( line, ']' );
				if (( ptr ) &&
					( ptr2 ) &&
					( ptr2 < ptr )) {
					/* a line with a ] separator that also contains a : */
					ptr = NULL;
				}
			} else {
				ptr = NULL;
			}
			if ( ! ptr ) {
				/* assume old "command]value|value" format */
				/* convert to new format (ie, in order to emote need a ':') */
				old_format = 1;
				ptr = strchr( line, ']' );
				if ( ! ptr ) {
					continue;
				}
				strncpy( tmp_line, line, ptr - line );
				tmp_line[ptr - line] = '\0';
				if (( *(ptr+1) == '!' ) ||
					( *(ptr+1) == ':' ) ||
					( *(ptr+1) == '/' )) {
					strcat( tmp_line, ":" );
				} else {
					strcat( tmp_line, "::" );
				}
				strncat( tmp_line, ptr + 1, 2040 );
				strncpy( line, tmp_line, 1022 );
				ptr = strchr( line, ':' );
			}
			*ptr = '\0';


			cmd_value1 = ptr + 1;
			cmd_value1 = skip_whitespace( cmd_value1 );
			ptr++;

			ptr = strchr( cmd_value1, '|' );
			if ( ptr ) {
				*ptr = '\0';
				if ( old_format ) {
					strncpy( tmp_line, line, ptr - line );
					tmp_line[ptr - line] = '\0';
					strcat( tmp_line, "|:" );
					strcat( tmp_line, ptr + 1 );
					ptr = strchr( tmp_line, '|' );
				}
				cmd_value2 = ptr + 1;
				cmd_value2 = skip_whitespace( cmd_value2 );
			} else {
				/* leave 2nd part value empty */
				cmd_value2 = empty_value;
			}

			if (( ! strlen( cmd_value1 )) &&
				( strlen( cmd_value2 ))) {
				cmd_value1 = cmd_value2;
				cmd_value2 = empty_value;
			}

			if ( strlen( cmd_value1 ) > 511 ) {
				cmd_value1[511] = '\0';
			}

			if ( strlen( cmd_value2 ) > 511 ) {
				cmd_value2[511] = '\0';
			}

			gtk_list_store_append(GTK_LIST_STORE(freezer), &iter);

			gtk_list_store_set(GTK_LIST_STORE(freezer), &iter,  
			GYTRIPLE_COL1, alias_name, GYTRIPLE_COL2, cmd_value1, 
			GYTRIPLE_COL3, cmd_value2, -1);
		}
		fclose( fp );
	} else {
		/* no alias file so set defaults */
		alias = &aliases[0];
		while( alias->alias_name ) {
			gtk_list_store_append(GTK_LIST_STORE(freezer), &iter);

			gtk_list_store_set(GTK_LIST_STORE(freezer), &iter,  
			GYTRIPLE_COL1, alias->alias_name, GYTRIPLE_COL2, alias->cmd_value1, 
			GYTRIPLE_COL3, alias->cmd_value2, -1);
			
			alias++;
		}

		/* save our default set to the file */
		save_aliases();
	}

	unfreeze_treeview(GTK_WIDGET(chat_aliases), freezer);
}

void save_aliases() {
	gboolean valid;
	GtkTreeIter iter;
	gchar *alias_name;
	gchar *cmd_value1;
	gchar *cmd_value2;
	char alias_filename[192] = "";
	FILE *fp;

	// DBG( 1, "save_aliases()\n" );

	snprintf( alias_filename, 190, "%s/commands", GYACH_CFG_COMMON_DIR );

	fp = fopen( alias_filename, "wb" );

	if ( fp ) {
		valid = gtk_tree_model_get_iter_first(gtk_tree_view_get_model(chat_aliases), &iter);
		while(valid) {
			char *lseldup=NULL;
			char *lseld=NULL;
			
			gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL1, &alias_name,GYTRIPLE_COL2, &cmd_value1 ,GYTRIPLE_COL3, &cmd_value2 , -1);

			lseldup=strdup(alias_name);

			fprintf( fp, "%s:", lseldup );

			free(lseldup);
			lseldup=strdup(cmd_value1);

			if ( strlen( lseldup )) {
				fprintf( fp, "%s", lseldup );
			}

			free(lseldup);
			lseldup=strdup(cmd_value2);
			lseld=strdup(cmd_value1);

			if (( strlen( lseldup )) &&
				(  strcmp( lseld, lseldup ))) {
				if ( strlen( lseld ))
					fprintf( fp, "|" );
				fprintf( fp, "%s", lseldup );
			}
			free(lseldup);
			free(lseld);
			g_free(alias_name);
			g_free(cmd_value1);
			g_free(cmd_value2);
			fprintf( fp, "\n" );
			
			valid = gtk_tree_model_iter_next(gtk_tree_view_get_model(chat_aliases), &iter);
		}
		fclose( fp );
	}
}

int alias_find( char *alias ) {
	gboolean valid;
	gchar *alias_ptr;
	char *lseldup=NULL;
	int found = -1;
	int i=0;

	// DBG( 11, "alias_find( '%s' )\n", alias );

	valid = gtk_tree_model_get_iter_first(gtk_tree_view_get_model(chat_aliases), &alias_find_iter);
	while( valid ) {
		gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &alias_find_iter,GYTRIPLE_COL1, &alias_ptr, -1);
		lseldup=strdup(alias_ptr);

		if ( ! strcasecmp( alias, lseldup )) {
			found = i;
			free(lseldup); 
			g_free(alias_ptr);
			break;
		}
		i++;
		free(lseldup); 
		g_free(alias_ptr);
		valid = gtk_tree_model_iter_next(gtk_tree_view_get_model(chat_aliases), &alias_find_iter);
	}

	// DBG( 12, "RETURN alias_find() == %d\n", found );
	return( found );
}

void edit_alias() {
	GtkTreeSelection *selection;
	GtkTreeModel *model;
	GtkTreeIter iter;
	gchar *alias_name;
	gchar *cmd_value1;
	gchar *cmd_value2;
	GtkWidget *w_alias_name;
	GtkWidget *w_cmd_value1;
	GtkWidget *w_cmd_value2;
	char *lseldup=NULL;

	// DBG( 11, "edit_alias()\n" );

	selection=gtk_tree_view_get_selection(chat_aliases);
	if (gtk_tree_selection_get_selected(selection, &model, &iter)) {

	gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL1, &alias_name,GYTRIPLE_COL2, &cmd_value1 ,GYTRIPLE_COL3, &cmd_value2 , -1);
	} else {
		/* Nothing selected */
		alias_name=g_strdup("");
		cmd_value1=g_strdup("");
		cmd_value2=g_strdup("");
	}

    if ( ! alias_window ) {
		alias_window = create_alias_window();
		gtk_widget_show( alias_window );
	}

	w_alias_name = lookup_widget( alias_window, "alias_name" );
	w_cmd_value1 = lookup_widget( alias_window, "cmd_value1" );
	w_cmd_value2 = lookup_widget( alias_window, "cmd_value2" );

	lseldup=strdup(alias_name);
	gtk_entry_set_text( GTK_ENTRY(w_alias_name),lseldup );
	free(lseldup);   
	lseldup=strdup(cmd_value1);
	gtk_entry_set_text( GTK_ENTRY(w_cmd_value1), lseldup );
	free(lseldup);   
	lseldup=strdup(cmd_value2);
	gtk_entry_set_text( GTK_ENTRY(w_cmd_value2), lseldup );
	free(lseldup);     
	g_free(alias_name);  
	g_free(cmd_value1);  
	g_free(cmd_value2);
}

int check_alias( char *alias, char *args )
{
	gboolean valid;
	GtkTreeIter iter;
	gchar *alias_name;
	gchar *alias_ptr;
	gchar *cmd_value_to_use;
	gchar *cmd_value1;
	gchar *cmd_value2;
	u_char  tmp[1152] = "";
	u_char  tmp_alias[32];
	char *ptr;
	char *end;
	int i, x;
	int found = -1;
	regex_t test_regex;

	// DBG( 21, "check_alias( %p, '%s', '%s' )\n", sess, alias, args );

	for( i = 0; i < strlen( alias ); i++ ) {
		alias[i] = tolower( alias[i] );
	}

	i = 0;
	valid = gtk_tree_model_get_iter_first(gtk_tree_view_get_model(chat_aliases), &iter);
	while( valid ) {
		gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL1, &alias_ptr, -1);

		alias_name=strdup(alias_ptr);
		strncpy( tmp_alias, alias_name , 28);
		for( x = 0; x < strlen( tmp_alias ); x++ ) {
			tmp_alias[x] = tolower( tmp_alias[x] );
		}

		if ( ! strcmp( alias, tmp_alias )) {
			free(alias_name);
			alias_name = tmp_alias;
			found = i;
			g_free(alias_ptr);
			break;
		}
		i++;
		free(alias_name);
		g_free(alias_ptr);
		valid = gtk_tree_model_iter_next(gtk_tree_view_get_model(chat_aliases), &iter);
	}

	if ( found >= 0 ) {
		char *acmd1=NULL;
		gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL2, &cmd_value1, GYTRIPLE_COL3, &cmd_value2, -1);

		regcomp( &test_regex, "(^%s|[^\\]%s|^\\$\\*|[^\\]\\$\\*|^\\$[0-9]|[^\\]\\$[0-9])",
			REG_EXTENDED | REG_ICASE | REG_NOSUB );

		acmd1=strdup(cmd_value1);
		cmd_value_to_use = strdup(acmd1);
		if ( args[0] ) {
			if ( regexec( &test_regex, acmd1, 0, NULL, 0 )) {
				free(cmd_value_to_use);
				cmd_value_to_use = strdup(cmd_value2);
			}
		} else {
			if ( ! regexec( &test_regex, acmd1, 0, NULL, 0 )) {
				free(cmd_value_to_use);
				cmd_value_to_use = strdup(cmd_value2);
			}
		}
		free(acmd1);
		g_free(cmd_value2);
		g_free(cmd_value1);
		regfree( &test_regex );

		if ( args[0] ) {
			  /* added: PhrozenSmoke, sent emotes with /emote */
			snprintf(tmp, 25,  "%s", "/emote <i>");
			strncat( tmp, cmd_value_to_use, 1024 );
			/* strcpy( tmp, cmd_value_to_use );  */

			if (( strchr( tmp, '$' )) ||
				( strstr( tmp, "%s" ))) {
				char *snalias=get_screenname_alias(args);
				char snbuf[72]="";
				snprintf(snbuf,70, "'%s'",snalias);
				strncpy( tmp, replace_args( tmp, snbuf ), 1024);
				free(snalias);
				args[0] = '\0';
			}

			free(cmd_value_to_use);
			cmd_value_to_use = tmp;
		}    else  {  /* added: PhrozenSmoke, sent emotes with /emote */

			snprintf(tmp, 25,  "%s", "/emote <i>");
			strncat( tmp, cmd_value_to_use, 1024 );
			free(cmd_value_to_use);
			cmd_value_to_use = tmp;
					 }

		ptr = cmd_value_to_use;
		end = strstr( cmd_value_to_use, GYACH_CMD_SEP );
		if ( end ) {
			while( end ) {
				*end = '\0';
				chat_command( ptr );

				ptr = end + strlen( GYACH_CMD_SEP );
				end = strstr( ptr, GYACH_CMD_SEP );
			}
		}
		chat_command( ptr );		
		// DBG( 22, "RETURN check_alias() == TRUE\n" );

		return( TRUE );
	} else {
		// DBG( 22, "RETURN check_alias() == FALSE\n" );

		return( FALSE );
	}
}

void append_aliases_to_list( GList *list ) {
	gboolean valid;
	GtkTreeIter iter;
	gchar *alias_name;
	u_char  tmp_alias[32];
	int x;
	char *talias=NULL;

	valid = gtk_tree_model_get_iter_first(gtk_tree_view_get_model(chat_aliases), &iter);
	while( valid ) {
		gtk_tree_model_get(gtk_tree_view_get_model(chat_aliases), &iter,GYTRIPLE_COL1, &alias_name, -1);
		talias=strdup(alias_name);

		strcpy( tmp_alias, "/" );
		strncat( tmp_alias, talias , 28);
		for( x = 0; x < strlen( tmp_alias ); x++ ) {
			tmp_alias[x] = tolower( tmp_alias[x] );
		}

		g_list_append( list, strdup( tmp_alias ));
		free(talias);
		g_free(alias_name);
		valid = gtk_tree_model_iter_next(gtk_tree_view_get_model(chat_aliases), &iter);
	}
}
