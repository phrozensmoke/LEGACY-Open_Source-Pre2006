/*****************************************************************************
 * htmlrender.c
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
 * VERY preliminary code for handling basic HTML content
 * For 'My Yahoo' content - TODO, fix handling of relative URLS in method 'process_httpurl'
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
#include <libgtkhtml/gtkhtml.h>

#include "users.h"
#include "util.h"
#include "gyach.h"
#include "webconnect.h"


GtkWidget *myurlentry=NULL;
GtkWidget *myyahcombo=NULL;
GtkWidget *myyahoo_edbutton=NULL;
GtkWidget *myyahoo_catbutton=NULL;
HtmlDocument *myahoodoc=NULL;
GtkWidget *myyahoohtmlview=NULL;
char mybaseurl[352]="";
char currenturl[352]="";
char html_about_blank[550]="";
char *htmlbuf=NULL;
char *html_fallback="<body bgcolor=\"#FFFFFF\"><br>  <br><center><font size=\"4\" color=\"#CC0000\"><b>Could not load the requested URL</b></center></font>";

/* MyYahoo 'tearoff' pages are very small, as well as YMSG 'news alerts'...this
   is quite enough space */
int MY_YAHOO_DATASIZE=35008;


void open_extern_url(char *anyurl) {
	if (!anyurl) {return;}
	snprintf( html_about_blank, 450, "%s", anyurl );
	display_url( (void *) strdup(html_about_blank) );
}

void open_edit_my_yahoo(GtkButton *widget, gpointer user_data) {
	open_extern_url("http://my.yahoo.com/");
}




void set_base_url(char *some_url) {
	char turl[352]="";
	char *mptr;
	char *uptr;
	char *mbase=NULL;

	strncpy(turl,some_url,350);

	//printf("setbase-1:  %s\n", turl); fflush(stdout);

	uptr=turl;
	sprintf(mybaseurl,"%s","");

	mptr = strrchr( uptr, '/' );
	if (mptr) {
		*mptr = '\0';
		strncpy(mybaseurl,uptr, 350);
		if (!strncasecmp(mybaseurl,"http://",7))  {
			if (strlen(mybaseurl)==7) {
				uptr=mptr+1;
				strncat(mybaseurl,uptr,340);
												 }
																  }
			 } 

	mbase=strchr(mybaseurl,'?');
	if (mbase) {*mbase='\0';}
	mbase=mybaseurl;
	//printf("setbase:  %s\n", mbase); fflush(stdout);

}

void
html_cancel_stream (HtmlStream *stream,  gpointer    user_data,  gpointer    cancel_data)
{
        // g_print ("CANCEL!!\n");
        /* Not sure what to do here */
}

char *process_httpurl(const gchar *url) {
	char myurl[352]="";
	char *murl;

	if (! strncasecmp(url,"javascript:", 11)) {return "";}
	if (! strncasecmp(url,"java:", 5)) {return "";}
	if (!strncasecmp(url,"file:", 5)) {
		snprintf(myurl,350, "%s", url);
		murl=myurl;
		return murl;
	}
	if (! strstr(url,"http://") ) {
		if (strstr(url,"://")) {return "";}  /* some other protocol */
									} else    {
		snprintf(myurl,350, "%s", url);
		murl=myurl;
		return murl;
												}
	
	snprintf(myurl,350,"%s",url);
	if (strlen(url)>0 && (! strstr(url,"http://") ))  {
		if (url[0]=='/') 	{
			if (strchr(url,'/')==strrchr(url,'/')) {
				snprintf(myurl,350,"%s%s",mybaseurl,url);
			} else {
			char tmpath[352];
			char *rptrr=NULL;
			snprintf(tmpath,350, "%s", strchr(url,'/'));
			rptrr=strrchr(tmpath,'/');
			if (rptrr) {*rptrr='\0';}			
			/* printf("incoming:  %s\nbase: %s\ntmpath:  %s\n",myurl, mybaseurl, tmpath); fflush(stdout); */ 
				if (strstr(mybaseurl,tmpath )) {
									if (rptrr)  {rptrr++;} else {rptrr=tmpath;}
									snprintf(myurl, 350, "%s/%s", mybaseurl, rptrr);
													   } else { snprintf(myurl,350,"%s%s",mybaseurl,url);}
					  }

		} else if (url[0]=='?') 	{
			char *mbase=NULL;
			mbase=strchr(currenturl,'?');
			if (mbase) {*mbase='\0';}
			snprintf(myurl,350,"%s%s",currenturl,url);
		} else {snprintf(myurl,350,"%s/%s",mybaseurl,url);}
						 											}
	murl=myurl;
	return murl;
}


void load_local_document(HtmlDocument *doc, char *filename) {
	/* Handles file:/ URLS */
	struct stat sbuf;
	int fd;
	int bytes;
	char filebuf[513];
	int grabbed=0;
	int maxreadin=MY_YAHOO_DATASIZE-2048;	

	if (!filename) {return;}
	if (strlen(filename)<5) {return;}

	if ( stat( filename+5, &sbuf )) {
		/* file doesn't exist so exit */  
		snprintf(html_about_blank, 548, "%s<br>Could not open local file: %s",html_fallback, filename+5);
		html_document_open_stream (doc, "text/html");
		html_document_write_stream (doc, html_about_blank, strlen(html_about_blank));
		html_document_close_stream (doc);
		return;
	}

	fd = open( filename+5, O_RDONLY, 0600 );
	if ( fd == -1 ) {
		/* can't open file */ 
		snprintf(html_about_blank, 548, "%s<br>Could not open local file: %s",html_fallback, filename+5);
		html_document_open_stream (doc, "text/html");
		html_document_write_stream (doc, html_about_blank, strlen(html_about_blank));
		html_document_close_stream (doc);
		return;
	}
	sprintf(htmlbuf,"%s", "");
	bytes = read( fd, filebuf, 512 );
	while( bytes && (grabbed<maxreadin)) {
		strncat(htmlbuf, filebuf, bytes);
		grabbed +=bytes;
		bytes = read( fd, filebuf, 512 );
	}
	close( fd );
	html_document_open_stream (doc, "text/html");
	html_document_write_stream (doc, htmlbuf, grabbed);
	html_document_close_stream (doc);
	gtk_entry_set_text(GTK_ENTRY(myurlentry),filename);
}


void load_yahprofile(HtmlDocument *doc, char *some_url) {
	int url_length=0;
	int it_printed=0;
	char *starter=NULL;
	char *ender=NULL;

	set_max_url_fetch_size(MY_YAHOO_DATASIZE-1);
	url_length = fetch_url( some_url, htmlbuf, ymsg_sess->cookie );  
	if (url_length<5) {snprintf(htmlbuf, 1024, "%s",html_fallback);}

	starter=strstr(htmlbuf, "<table cellspacing=0 width=\"100%\" cellpadding=2 border=0>");
	if (starter) {
	ender=strstr(starter, "<table border=\"0\" cellpadding=\"4\" cellspacing=\"0\" width=\"750\">");

	}

	html_document_open_stream (doc, "text/html");

	if (starter && ender && strstr(starter, "</td>\n<td width=\"1%\">"))   {
		char *clipper=NULL;
		*ender='\0';
		clipper=strstr(starter, "</td>\n<td width=\"1%\">");
		while (clipper) {
			if (!strstr(clipper+1, "</td>\n<td width=\"1%\">")) {break;}
			clipper=strstr(clipper+1, "</td>\n<td width=\"1%\">");
		}
		if (clipper)  {
			char *duppy=malloc(strlen (starter)+325);
			*clipper='\0';
			snprintf(duppy,strlen (starter)+325-2, "<body bgcolor=\"#FFFFFF\" text=\"#000000\"><font face=\"Arial\" size=\"+1\"><b>%s's Profile</b></font><br> &nbsp; <br><table cellspacing=0 width=\"98%%\" cellpadding=2 border=0><td>%s</td></tr></table>", some_url+26, starter);
			html_document_write_stream (doc, duppy, strlen (duppy));
			free(duppy);
			it_printed=1;
		}
	}

	if (!it_printed) {html_document_write_stream (doc, htmlbuf, strlen (htmlbuf));}
	html_document_close_stream (doc);
	if (htmlbuf) {free(htmlbuf); htmlbuf=NULL;}
	clear_fetched_url_data();
}


void load_html_document(HtmlDocument *doc, char *some_url) {
	int url_length=0;	

	//printf("GOT URL-1:  %s\n", some_url); fflush(stdout);

	if (some_url && (strlen(some_url)>0)) {
		set_base_url(some_url); 
		gtk_entry_set_text(GTK_ENTRY(myurlentry),some_url);
		snprintf(currenturl,350,"%s",some_url);
		if (!strcasecmp(some_url, "about:blank"))  {
			snprintf(html_about_blank, 548, "<body bgcolor=\"#EFF3F7\" text=\"#000000\" link=\"#000099\" vlink=\"#880000\"><center><br><b><font size=\"6\"  color=\"#660099\">Gyach Enhanced</font><font size=\"4\" color=\"#000088\"><br>%s<br><br><a href=\"%s\">%s</a><br><br>_______________________<br>  <br></font><font size=\"3\" color=\"#000000\"><i>Copyright (c) 2003-2006, Erica Andrews</i><br>PhrozenSmoke ['at'] yahoo.com<br><br>License: GNU General Public License</font></center>",VERSION,GYACH_URL,GYACH_URL); 
			url_length=strlen(html_about_blank);
			if (htmlbuf) {free(htmlbuf); htmlbuf=NULL;}
			html_document_open_stream (doc, "text/html");
			html_document_write_stream (doc, html_about_blank, url_length);
			html_document_close_stream (doc);
			return;
										  			   		} else {
			if (!htmlbuf)  {htmlbuf=malloc(MY_YAHOO_DATASIZE); }
			if (!htmlbuf)  {
				snprintf(html_about_blank, 548, "%s", html_fallback);
				html_document_open_stream (doc, "text/html");
				html_document_write_stream (doc, html_about_blank, strlen(html_about_blank));
				html_document_close_stream (doc);
				return;
			}
			sprintf(htmlbuf,"%s", "");
			if (!strncasecmp(some_url,"file:", 5)) {
					load_local_document(doc, some_url);
					if (htmlbuf) {free(htmlbuf); htmlbuf=NULL;}
					return;
				} else {
					if (!strncasecmp(some_url,"http://profiles.yahoo.com/", 26))  {
						load_yahprofile(doc, some_url);
						return;
					}
					/* Yahoo 'tear-off pages for My Yahoo are normally very small */
					set_max_url_fetch_size(MY_YAHOO_DATASIZE-1);
					url_length = fetch_url( some_url, htmlbuf, ymsg_sess->cookie );  

						  }
										  			  				  }

													  }  else {return; }  /* invalid/missing url */

	/* printf("GOT URL:  %s\n", some_url); fflush(stdout);  */
	
	if (url_length<5) {snprintf(htmlbuf, 1024, "%s",html_fallback);}

	/* This 'clear' call was causing a seg-fault */
	//html_document_clear (doc);

	if (! strcasecmp(some_url,"http://my.yahoo.com/tearoff/channel.html"))  {
		/* clean up some stuff that screws up news page margins */
		char *cleaner=strstr(htmlbuf,"leftmargin=0");
		if (cleaner) { *(cleaner+11)='6'; }
		cleaner=strstr(htmlbuf,"<li>");
		while (cleaner) {
			*cleaner='<';
			*(cleaner+1)='b';
			*(cleaner+2)='r';
			*(cleaner+3)='>';
			cleaner=strstr(htmlbuf,"<li>");
		}
	}

	if (strstr(some_url, "my.yahoo.com/preview/")) {
		char *cleaner=NULL;
		cleaner=strstr(htmlbuf,"</a>\"\n<tr><td align=center>");
		if (cleaner && (strstr(some_url, "pirphotos.html")) ) {
				/* Fix some bogus HTML coding from Yahoo */ 
				char *dud=NULL;
				char goo[32];
				snprintf(goo, 28, "%s" , "</a>\"<br>&nbsp; <br>          ");
				dud=strdup(goo);
				memcpy(cleaner, dud, 27);
				free(dud);
		}
		cleaner=strstr(htmlbuf,"<small>");
		while (cleaner) {
			*cleaner=' ';
			*(cleaner+1)=' ';
			*(cleaner+2)=' ';
			*(cleaner+3)=' ';
			*(cleaner+4)=' ';
			*(cleaner+5)=' ';
			*(cleaner+6)=' ';
			cleaner=strstr(htmlbuf,"<small>");
		}
		cleaner=strstr(htmlbuf,"</small>");
		while (cleaner) {
			*cleaner=' ';
			*(cleaner+1)=' ';
			*(cleaner+2)=' ';
			*(cleaner+3)=' ';
			*(cleaner+4)=' ';
			*(cleaner+5)=' ';
			*(cleaner+6)=' ';
			*(cleaner+7)=' ';
			cleaner=strstr(htmlbuf,"</small>");
		}
	}

	html_document_open_stream (doc, "text/html");
	html_document_write_stream (doc, htmlbuf, strlen (htmlbuf));
	html_document_close_stream (doc);
	if (htmlbuf) {free(htmlbuf); htmlbuf=NULL;}
	clear_fetched_url_data();
}

/* This method is for throwing random HTML strings
    to the HTML widget, such as the HTML-formatted
    strings that come with the new Y! Messenger 'new alert' packets  
*/

void set_myyahoo_html_data(char *myhtmldata) {
	int url_length=0;	
	if (!myhtmldata) {return;}
	if (!myahoodoc) {return;}
	if (!htmlbuf)  {htmlbuf=malloc(MY_YAHOO_DATASIZE); }
	if (!htmlbuf)  {
		snprintf(html_about_blank, 548, "%s", html_fallback);
		html_document_open_stream (myahoodoc, "text/html");
		html_document_write_stream (myahoodoc, html_about_blank, strlen(html_about_blank));
		html_document_close_stream (myahoodoc);
		return;
		}
	url_length=snprintf(htmlbuf,9000, "<body bgcolor=\"#FFFFFF\"><table border=\"0\" cellpadding=\"5\" width=\"98%%\"><tr><td width=\"100%%\">%s", myhtmldata);
	strcat(htmlbuf,"</td></tr></table><br>");
	html_document_open_stream (myahoodoc, "text/html");
	html_document_write_stream (myahoodoc, htmlbuf, strlen (htmlbuf));
	html_document_close_stream (myahoodoc);
	gtk_entry_set_text(GTK_ENTRY(myurlentry),"alert:News");
	snprintf(htmlbuf, 128, "%sindex.php", GYACH_URL);
	set_base_url(htmlbuf); 
}

void load_myyahoo_URL(char *turl) {
	if ( myahoodoc && turl) {
		load_html_document(myahoodoc, turl);
	}
}

void open_my_yahoo(GtkButton *widget, gpointer user_data) {
	char *mycat;
	char *pref_page=NULL;
	char turl[112]="";
	if (!myahoodoc) {return;}
	if (!logged_in) {
	 snprintf(turl,32, "%s", "about:blank");
	 load_myyahoo_URL(turl);
	 return;
	}

	mycat=strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(myyahcombo)->entry)));
	if (strlen(mycat)<1) {free(mycat); return;}	
	if (!strncmp(mycat,"----", 4))  {free(mycat); return;}	

	snprintf(turl, 3, "%s", ""); 

	/* check for horoscopes first */
	if (! strcmp(mycat,_("Horoscope - Aquarius")  ))  { pref_page="aquarius";}
	if (! strcmp(mycat,_("Horoscope - Pisces")  ))  { pref_page="pisces";}
	if (! strcmp(mycat,_("Horoscope - Taurus")  ))  { pref_page="taurus";}
	if (! strcmp(mycat,_("Horoscope - Aries")  ))  { pref_page="aries";}
	if (! strcmp(mycat,_("Horoscope - Gemini")  ))  { pref_page="gemini";}
	if (! strcmp(mycat,_("Horoscope - Cancer")  ))  { pref_page="cancer";}
	if (! strcmp(mycat,_("Horoscope - Leo")  ))  { pref_page="leo";}
	if (! strcmp(mycat,_("Horoscope - Virgo")  ))  { pref_page="virgo";}
	if (! strcmp(mycat,_("Horoscope - Libra")  ))  { pref_page="libra";}
	if (! strcmp(mycat,_("Horoscope - Scorpio")  ))  { pref_page="scorpio";}
	if (! strcmp(mycat,_("Horoscope - Sagittarius")  ))  { pref_page="sagittarius";}
	if (! strcmp(mycat,_("Horoscope - Capricorn")  ))  { pref_page="capricorn";}


	 /* Set the horoscope date */
	if (pref_page) {  /* a horoscope page */
		struct tm *tvtm_now; 
		time_t tvtime_now = time( NULL );
		tvtm_now = localtime( &tvtime_now);
		snprintf(turl, 110, "http://uk.horoscope.yahoo.com/daily/%d%02d%02d/%s.html",
		tvtm_now->tm_year+1900, tvtm_now->tm_mon+1, tvtm_now->tm_mday, pref_page);  
	} else {
		/* Set the basic base URL */
		snprintf(turl, 32, "%s", "http://my.yahoo.com/preview/"); 
	}


	if (! strcasecmp(mycat,_("Weather")  )  )  {strcat(turl, "weather.html"); }
	if (! strcasecmp(mycat,_("Sports")  )  )  {	strcat(turl, "scoreboard.html"); }
	if (! strcasecmp(mycat,_("Calendar")  )  )  {strcat(turl, "smallcal.html?.pirf=pircalendar");}
	if (! strcasecmp(mycat,_("Bookmarks")  )  )  {strcat(turl, "sites.html");}
	if (! strcasecmp(mycat,_("Auctions")  )  )  {strcat(turl, "auctions.html"); }
	if (! strcasecmp(mycat,_("Travel")   )  )  {strcat(turl, "faretrack.html");}
	if (! strcasecmp(mycat,_("Portfolios")   )  )  {strcat(turl, "quotes.html"); }
	if (! strcasecmp(mycat,_("E-Mail Preview")   )  )  {strcat(turl, "pirymail.html?.pirf=yml"); }
	if (! strcasecmp(mycat,_("Top Headline")  )  )  {strcat(turl, "lead.html"); }
	if (! strcasecmp(mycat,_("Movie Showtimes")  )  )  {strcat(turl, "mymovies.html"); }
	if (! strcasecmp(mycat, _("New Music Releases")  )  )  {strcat(turl, "cdrel.html"); }
	if (! strcasecmp(mycat, _("My Horoscope")   )  )  {	strcat(turl, "hscp.html"); }
	if (! strcasecmp(mycat, _("My Briefcase") ) )  {strcat(turl, "briefcase.html?.pirf=briefcase2");}
	if (! strcasecmp(mycat, _("Recipes")   )  )  {strcat(turl, "hleat.html"); }
	if (! strcasecmp(mycat, _("Sports Team News")   )  )  {	strcat(turl, "teamrep.html");}
	if (! strcasecmp(mycat, _("Currency Rates")   )  )  {strcat(turl, "tc.html");}
	if (! strcasecmp(mycat, _("My Invitations")   )  )  {strcat(turl, "invites.html");  }
	if (! strcasecmp(mycat, _("Notepad")  ))  {strcat(turl, "pirnotes.html?.pirf=pirnotepad");  }
	if (! strcasecmp(mycat, _("Fitness")  )  )  {strcat(turl, "hlfit.html");  }
	if (! strcasecmp(mycat, _("Health")   )  )  {strcat(turl, "hltip.html"); }
	if (! strcasecmp(mycat, _("Community Service")   )  )  {strcat(turl, "phil.html"); }
	if (! strcasecmp(mycat, _("RSS Newsfeeds")  )  )  {	strcat(turl, "ycontent.html");}
	if (! strcasecmp(mycat, _("New Movie Releases")   )  )  {strcat(turl, "movierels.html"); }
	if (! strcasecmp(mycat, _("New Video Releases")  )  )  {strcat(turl, "vrels.html"); }
	if (! strcasecmp(mycat, _("Jobs")   )  )  {	strcat(turl, "pircareers.html?.pirf=hotjobs");}
	if (! strcasecmp(mycat, _("Lotto Results")  )  )  {	strcat(turl, "lotto.html");}
	if (! strcasecmp(mycat, _("Stock Market Summary")  ))  {strcat(turl, "mktsummary.html"); }
	if (! strcasecmp(mycat, _("Allergy Watch")  )  )  {	strcat(turl, "allergy.html"); }
	if (! strcasecmp(mycat, _("Automobiles")  )  )  {strcat(turl, "autos.html");  }
	if (! strcasecmp(mycat, _("Saved Searches")  )  )  {strcat(turl, "searches.html");}
	if (! strcasecmp(mycat, _("Forums")  )  )  {strcat(turl, "mboards.html"); }
	if (! strcasecmp(mycat, _("Tips")  )  )  {	strcat(turl, "tips.html"); }
	if (! strcasecmp(mycat, _("My Photos")   )  )  {strcat(turl, "pirphotos.html?.pirf=photos"); }
	if (! strcasecmp(mycat, _("City Guide")   )  )  {strcat(turl, "lcategories.html");   }

	if (! strcasecmp(mycat, _("Parenting Tips")  )  )  {	strcat(turl, "rents.html"); }
	if (! strcasecmp(mycat, _("Diet Tracker")  )  )  {	strcat(turl, "diet.html"); }
	if (! strcasecmp(mycat, _("Pet Tips")  )  )  {	strcat(turl, "ppets.html"); }
	if (! strcasecmp(mycat, _("Ask the Doctor")  )  )  {	strcat(turl, "askdoctor.html"); }
	if (! strcasecmp(mycat, _("Dummies eTips")  )  )  {	strcat(turl, "dmmytip.html"); }
	if (! strcasecmp(mycat, _("Asthma Watch")  )  )  {	strcat(turl, "asthma.html"); }
	if (! strcasecmp(mycat, _("Word of the Day")  )  )  {	strcat(turl, "wotd.html"); }
	if (! strcasecmp(mycat, _("Gossip")  )  )  {	strcat(turl, "gossip.html"); }


	/* These below are a little different ... */

	if (! strcasecmp(mycat,_("News")  )  )  {
		snprintf(turl, 100, "%s", "http://my.yahoo.com/sections/t/p1.html");
															   }

	if (! strcasecmp(mycat,_("Game Scores")   )  )  {
		snprintf(turl, 110,  "http://profiles.games.yahoo.com/games/profile2?name=%s", ymsg_sess->user);
																			 }

	if (! strcasecmp(mycat,_("TV Guide")   )  )  {
		struct tm *tvtm_now;  /* Set the hour to view */
		time_t tvtime_now = time( NULL );
		tvtm_now = localtime( &tvtime_now);
		snprintf(turl, 110, "http://my.yahoo.com/preview/tv.html?.tvsh=%d",tvtm_now->tm_hour);
																	 }

			/* for testing - remove */
			/* This loads a page that relies heavily on Javascript */
	/*  
	if (! strncasecmp(mycat,_("Games") , strlen( _("Games") )  ))  {
		char gamey[512];
		snprintf(gamey, 510,  "http://y1.games.vip.scd.yahoo.com/messenger/messengertab_updater?ymsgr=%s", gyach_url_encode(ymsg_sess->cookie));
		load_myyahoo_URL(gamey);	
		free(mycat);
		return;
																				    }
	*/  


	if (strlen(turl)>1) {
		if (strstr(turl, "http://")) {
			load_myyahoo_URL(turl);
		}
	}
	free(mycat);
}


void submit_clicked (HtmlDocument *document, const gchar *method, const gchar *url, const gchar *encoding, gpointer data)
{
	if (strlen(currenturl)>0) { open_extern_url(currenturl); }
}


void link_clicked (HtmlDocument *doc, const gchar *url, gpointer data)
{
	char *myurl;
	myurl=strdup(process_httpurl(url));
	if (!myurl) {return;}
	if (strlen(myurl)<1) {return;}
  	/*  printf ("Requested URL-clicked: %s \n", myurl); fflush(stdout);  */ 

	/* Only handle 'tearoff' pages in the built-in browser
	This supports vieiwing (not editing) of the TV Guide  */
	if (strstr(myurl,"yahoo.com/tearoff/") || strstr(myurl,"my.yahoo.com/preview/") ||
		strstr(myurl, "yahoo.com/preview/pirphotos.html?") ) { 
		load_html_document(doc, myurl); 
		free(myurl);
		return;
	}

	/* profiles */
	if ( (!strncasecmp(myurl,"http://profiles.yahoo.com/", 26)) && (strlen(myurl)>27)) { 
		load_html_document(doc, myurl); 
		free(myurl);
		return;
	}

	/* game profiles */
	if ( (!strncasecmp(myurl,"http://profiles.games.yahoo.com/games/profile2?name=", 52)) && (strlen(myurl)>53)) { 
		load_html_document(doc, myurl); 
		free(myurl);
		return;
	}

	/* for now, we'll open all other clicked links with the extern html browser, 
	    since we still have work to do on handling relative URLs, etc. */

	open_extern_url(myurl);
	free(myurl);
}


void url_requested (HtmlDocument *html, const gchar *url, HtmlStream *html_stream, gpointer data)
{

	char *tfile;
  	gint   fd;
  	gint   len;
 	guchar buf [513];
	char *myurl=NULL;

	if (strstr(url, "/my/next.gif")) {
		myurl=strdup("http://www.digitallords.com/icons/forward.png");
	} else if (strstr(url, "/my/prev.gif")) {
		myurl=strdup("http://www.digitallords.com/icons/back.png");
	} else if (strstr(url, "/my/arrowdown.gif")) {
		myurl=strdup("http://www.digitallords.com/icons/down.png");
	} else if (strstr(url, "/bc/sfld.gif")) {
		myurl=strdup("http://www.digitallords.com/icons/folder.png");
	} else {
	myurl=strdup(process_httpurl(url));
		}

	if (! myurl) {return;}
	if (strlen(myurl)<1) { free(myurl); return;}
  	//printf ("Requested URL: %s \n", myurl); fflush(stdout); 

	html_stream_set_cancel_func (html_stream, html_cancel_stream, NULL);

	tfile=download_image(myurl);
	free(myurl);

	if (!tfile) {return;}
	if (strlen(tfile)<1) {return;}

  	//printf ("Requested FILE: %s \n", tfile); fflush(stdout);

  	fd = open (tfile, O_RDONLY);

  	if (fd < 0)
 	 {
    	//printf("(FAILED)\n");  fflush(stdout);
    	return;
  	}	
   	//printf ("(ok)\n");  fflush(stdout);
  	for (; (len = read (fd, buf, 512)) > 0; )
  	{
   	 html_stream_write (html_stream, buf, len);
  	}
 	 close (fd);
	
	unlink(tfile);
}


HtmlDocument *get_html_document(char *some_url) {
	HtmlDocument *doc;

	doc = html_document_new ();
	g_signal_connect (G_OBJECT (doc), "request_url",
	G_CALLBACK (url_requested), NULL);

	load_html_document(doc, some_url);

	g_signal_connect (G_OBJECT (doc), "link_clicked",
	G_CALLBACK (link_clicked), NULL);

	g_signal_connect (G_OBJECT (doc), "submit",
	G_CALLBACK (submit_clicked), NULL);

	return doc;
}


GtkWidget *create_html_widget(char *some_url) {
	GtkWidget *view;
	GtkWidget *scrolled_window;
	GtkWidget *vbox1;

  vbox1 = gtk_vbox_new (FALSE, 0);
  myurlentry=gtk_entry_new();
  gtk_entry_set_max_length(GTK_ENTRY(myurlentry),400);
  if (some_url) {gtk_entry_set_text(GTK_ENTRY(myurlentry),some_url);}
  gtk_editable_set_editable(GTK_EDITABLE(myurlentry),FALSE );
  gtk_box_pack_start (GTK_BOX (vbox1),myurlentry, FALSE, FALSE,4);

	myahoodoc=get_html_document(some_url);
	view = html_view_new ();
	myyahoohtmlview=view;

html_view_set_document (HTML_VIEW (view), myahoodoc);
html_view_set_magnification (HTML_VIEW (view), 1.4);

scrolled_window = gtk_scrolled_window_new (NULL, NULL);
gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
gtk_container_add (GTK_CONTAINER (scrolled_window), view);
gtk_box_pack_start (GTK_BOX (vbox1), scrolled_window, TRUE, TRUE,1);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolled_window), GTK_SHADOW_IN);
gtk_widget_show_all(vbox1);
return vbox1;
}


void clear_my_yahoo_document() {
	if (myahoodoc) { load_html_document(myahoodoc, "about:blank");}
}

void hide_my_yahoo(GtkButton *widget, gpointer user_data) {
	clear_my_yahoo_document();
}

void on_myyahoo_zoomin(GtkButton *widget, gpointer user_data) {
	if (!myyahoohtmlview) {return;}
	html_view_zoom_in (HTML_VIEW (myyahoohtmlview));
}
void on_myyahoo_zoomout(GtkButton *widget, gpointer user_data) {
	if (!myyahoohtmlview) {return;}
	html_view_zoom_out (HTML_VIEW (myyahoohtmlview));
}




GtkWidget *get_my_yahoo_panel() {
	GtkWidget *mypanel;
	GtkWidget *hbox1;
	GtkWidget *hidebutton, *zoomout, *zoomin;
	GList *intllist = NULL;
	char *some_url="about:blank";
	
	mypanel = gtk_vbox_new (FALSE, 0);


	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(_("My Yahoo!")), FALSE, FALSE,2);
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(": "), FALSE, FALSE,0);

	myyahcombo=gtk_combo_new();

	intllist = g_list_append( intllist,_("E-Mail Preview") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("Weather") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist,_("Top Headline") );
	intllist = g_list_append( intllist, _("News") );
	intllist = g_list_append( intllist, _("RSS Newsfeeds")  );
	intllist = g_list_append( intllist, _("Lotto Results") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("Sports") );
	intllist = g_list_append( intllist, _("Sports Team News") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("Stock Market Summary")  );
	intllist = g_list_append( intllist, _("Portfolios") );
	intllist = g_list_append( intllist, _("Currency Rates")  );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("TV Guide") );
	intllist = g_list_append( intllist,_("Movie Showtimes")  );
	intllist = g_list_append( intllist, _("New Music Releases") );
	intllist = g_list_append( intllist, _("New Movie Releases")  );
	intllist = g_list_append( intllist, _("New Video Releases")  );
	intllist = g_list_append( intllist, _("Gossip") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("My Photos")  );
	intllist = g_list_append( intllist, _("My Briefcase") );
	intllist = g_list_append( intllist, _("Jobs") );
	intllist = g_list_append( intllist, _("Calendar") );
	 intllist = g_list_append( intllist, _("Game Scores") );
	intllist = g_list_append( intllist, _("Bookmarks") );
	intllist = g_list_append( intllist, _("My Invitations")  );
	intllist = g_list_append( intllist, _("Notepad")  );
	intllist = g_list_append( intllist, _("Saved Searches")  );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("Fitness")  );
	intllist = g_list_append( intllist, _("Health") );
	intllist = g_list_append( intllist, _("Allergy Watch")  );
	intllist = g_list_append( intllist, _("Asthma Watch")  );
	intllist = g_list_append( intllist, _("Ask the Doctor")  );
	intllist = g_list_append( intllist, _("Recipes")  );
	intllist = g_list_append( intllist, _("Diet Tracker")  );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("Tips")  );
	intllist = g_list_append( intllist, _("Parenting Tips")  );
	intllist = g_list_append( intllist, _("Pet Tips")  );
	intllist = g_list_append( intllist, _("Dummies eTips")  );
	intllist = g_list_append( intllist, _("Word of the Day") );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("City Guide") );
	intllist = g_list_append( intllist, _("Community Service") );
	intllist = g_list_append( intllist, _("Automobiles")  );
	intllist = g_list_append( intllist, _("Auctions") );
	intllist = g_list_append( intllist, _("Travel") );
	intllist = g_list_append( intllist, _("Forums")  );
	intllist = g_list_append( intllist, "--------------------" );
	intllist = g_list_append( intllist, _("My Horoscope") );
	intllist = g_list_append( intllist,_("Horoscope - Aquarius") );
	intllist = g_list_append( intllist,_("Horoscope - Pisces")  );
	intllist = g_list_append( intllist,_("Horoscope - Taurus")  );
	intllist = g_list_append( intllist,_("Horoscope - Aries")   );
	intllist = g_list_append( intllist,_("Horoscope - Gemini")  );
	intllist = g_list_append( intllist,_("Horoscope - Cancer")  );
	intllist = g_list_append( intllist,_("Horoscope - Leo")  );
	intllist = g_list_append( intllist,_("Horoscope - Virgo")  );
	intllist = g_list_append( intllist,_("Horoscope - Libra")  );
	intllist = g_list_append( intllist,_("Horoscope - Scorpio")  );
	intllist = g_list_append( intllist,_("Horoscope - Sagittarius") );
	intllist = g_list_append( intllist,_("Horoscope - Capricorn")  );

	

  	gtk_combo_set_popdown_strings( GTK_COMBO(myyahcombo), intllist );
  	gtk_editable_set_editable(GTK_EDITABLE(GTK_COMBO(myyahcombo)->entry),FALSE );
  	gtk_box_pack_start (GTK_BOX (hbox1), myyahcombo, TRUE, TRUE, 2);

	myyahoo_catbutton=get_pixmapped_button(_(" View "), GTK_STOCK_REFRESH);
	set_tooltip(myyahoo_catbutton,_(" View "));
  	gtk_box_pack_start (GTK_BOX (hbox1), myyahoo_catbutton, FALSE, FALSE, 2);	
	hidebutton=get_pixmapped_button(_(" Hide "), GTK_STOCK_REMOVE);
	set_tooltip(hidebutton,_(" Hide "));
  	gtk_box_pack_start (GTK_BOX (hbox1), hidebutton, FALSE, FALSE, 2);	
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("     "), FALSE, FALSE,2);

	zoomin=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(zoomin), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_MENU) ));
	set_tooltip(zoomin,_("Zoom In"));
  	gtk_box_pack_start (GTK_BOX (hbox1), zoomin, FALSE, FALSE, 2);	
	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new(" "), FALSE, FALSE, 0);
	zoomout=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(zoomout), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_ZOOM_OUT, GTK_ICON_SIZE_MENU) ));
	set_tooltip(zoomout,_("Zoom Out"));
  	gtk_box_pack_start (GTK_BOX (hbox1), zoomout, FALSE, FALSE, 2);	

	gtk_box_pack_start (GTK_BOX (hbox1), gtk_label_new("     "), FALSE, FALSE,2);
	myyahoo_edbutton=get_pixmapped_button(_(" Edit My Yahoo! "), GTK_STOCK_PREFERENCES);
	set_tooltip(myyahoo_edbutton,_(" Edit My Yahoo! "));
  	gtk_box_pack_start (GTK_BOX (hbox1), myyahoo_edbutton, FALSE, FALSE, 2);	

  gtk_signal_connect (GTK_OBJECT (myyahoo_catbutton), "clicked",
                      GTK_SIGNAL_FUNC (open_my_yahoo),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (hidebutton), "clicked",
                      GTK_SIGNAL_FUNC (hide_my_yahoo),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (zoomout), "clicked",
                      GTK_SIGNAL_FUNC (on_myyahoo_zoomout),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (zoomin), "clicked",
                      GTK_SIGNAL_FUNC (on_myyahoo_zoomin),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (myyahoo_edbutton), "clicked",
                      GTK_SIGNAL_FUNC (open_edit_my_yahoo),
                      NULL);


	gtk_box_pack_start (GTK_BOX (mypanel), hbox1, FALSE, FALSE,5);
	gtk_box_pack_start (GTK_BOX (mypanel), create_html_widget(some_url), TRUE, TRUE,2);
	gtk_widget_show_all(mypanel);
	return mypanel;
}





