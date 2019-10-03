/*****************************************************************************
 * gyache-photos.c, Yahoo photos IMV emulation
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
 * Copyright (C) 2003-2006, Erica Andrews (Phrozensmoke ['a'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *
 *****************************************************************************/
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "../plugin_api.h"
#include "../images.h"
#include "../bootprevent.h"
#include "../profname.h"


extern void focus_pm_entry(GtkWidget *pmwin);
extern void set_tuxvironment (GtkWidget *pmwidget, GtkWidget *pmwindow, char *tuxviron);
extern void deactivate_tuxvironment(PM_SESSION *pms);
extern int ymsg_photo_imvironment( YMSG_SESSION *session, char *who, char *msg, int ptype);
extern GtkWidget *show_confirm_dialog_config(char *mymsg, char *okbuttonlabel, char *cancelbuttonlabel, int with_callback);

void yphoto_send_outgoing(PM_SESSION *pms, char *who, char *msg14, int ptype13);
void on_yphoto_forward(GtkButton *widget, gpointer user_data);

char yphoto_urls[256];
char yphoto_infos[608];
GHashTable *yphoto_cache=NULL;
GHashTable *yphoto_albums=NULL;
char *yphoto_buf=NULL;
char *yphoto_b64buf=NULL;
char *yphoto_cookie=NULL;
char *yphoto_url_prefix=NULL;
char *yphoto_challenge=NULL;
char *yphoto_server=NULL;
GList *oyphoto_list = NULL;
int YPHOTO_MEM=24576;

/* whether to retrieve data in base64 encoding or not, it 
	really just wastes space and sometimes stops all of 
	our albums from showing up, and its not exactly a 
	strong 'encryption' method either */

int YPHOTO_USE_BASE64=0;

int YPH_DEBUG=0;  /* turn debugging on or off */ 

char *yphoto_cookie_append="B=9a4k50l00fmia&b=2; Q=q1=AACAAAAAAAAAbw--&q2=QJMimQ--; F=a=.Z6jMDgsvaBtGShSqwPBTq0Ky1SSv3GwNnlt3BTHmMAZ2FO0YmBtuOWyfWKR&b=B0ZL; LYC=l_v=2&l_lv=7&l_l=f7hep4dicea4&l_s=qyuztxrs5r3tusxvy2ts343ys1xq4t5u&l_lid=1at6cle&l_r=6o&l_lc=0_0_32_0_-1&l_um=1_1_0_0_0; U=mt=aETWYZ2MhYukV8IY6JwV4Zx1O8VdrKvKeQt1Wg--&ux=q9r0AB&un=4ruau026ct429; PROMO=sessionID=; LYS=l_fh=1&l_vo=myla; ";



struct yphoto_item {
	char *caption;
	char *url;
	int width;
	int height;
	int position;
};

struct yphoto_album {
	char *who;
	char *url;
	char *title;	
	char *url_key;
	char *url_key_pending;
	GHashTable *yphotos;
	int position;
	int i_invite;
	int they_invite;
	int loaded;
	int slideshow;
	int private_session;
	int bounces;
	PM_SESSION *pms;
	GtkWidget *pm_hbox;
	GtkWidget *panel;
};

static const char * yphoto_b64s = 
 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

int yphoto_b64_decode(char *target, size_t tlen, const char *source, size_t slen)
{
const char*inp;
char*outp;
char*outend;
const char*inend;
int bits=0;
int working=0;
char *i;

if(slen==0)
slen=strlen(source);
outp=target;
inp=source;
outend=target+tlen;
inend=source+slen;
for(;outp<outend && inp<inend;inp++) {
if ( ((char)*inp)=='\n') {continue;}
working<<=6;
i=strchr(yphoto_b64s,(char)*inp);
if(i==NULL) {
return(-1);
}
if(*i=='=')  { /* pad char */
if((working&0xFF) != 0)
return(-1);
break;
}
bits+=6;
working |= (i-yphoto_b64s);
if(bits>=8) {
*(outp++)=(char)((working&(0xFF<<(bits-8)))>>(bits-8));
bits-=8;
}
}
if(outp == outend)  {
*(--outp)=0;
return(-1);
}
*outp=0;
return(outp-target);
}

static int add_char(char *pos, char ch, int done, char *end)
{
if(pos>=end)  {
return(1);
}
if(done)
*pos=yphoto_b64s[64];
else
*pos=yphoto_b64s[(int)ch];
return(0);
}

int yphoto_b64_encode(char *target, size_t tlen, const char *source, size_t slen)  {
const char*inp;
char*outp;
char*outend;
const char*inend;
char*tmpbuf=NULL;
int done=0;
char enc;
int buf=0;

if(slen==0)
slen=strlen(source);
inp=source;
if(source==target)  {
tmpbuf=(char *)malloc(tlen);
if(tmpbuf==NULL)
return(-1);
outp=tmpbuf;
}
else
outp=target;

outend=outp+tlen;
inend=inp+slen;
for(;(inp < inend) && !done;)  {
enc=*(inp++);
buf=(enc & 0x03)<<4;
enc=(enc&0xFC)>>2;
if(add_char(outp++, enc, done, outend))  {
if(target==source)
free(tmpbuf);
return(-1);
}
enc=buf|((*inp & 0xF0) >> 4);
if(add_char(outp++, enc, done, outend))  {
if(target==source)
free(tmpbuf);
return(-1);
}
if(inp==inend)
done=1;
buf=(*(inp++)<<2)&0x3C;
enc=buf|((*inp & 0xC0)>>6);
if(add_char(outp++, enc, done, outend))  {
if(target==source)
free(tmpbuf);
return(-1);
}
if(inp==inend)
done=1;
enc=((int)*(inp++))&0x3F;
if(add_char(outp++, enc, done, outend))  {
if(target==source)
free(tmpbuf);
return(-1);
}
if(inp==inend)
done=1;
}
if(outp<outend)
*outp=0;
if(target==source)  {
memcpy(target,tmpbuf,tlen);
free(tmpbuf);
}
return(outp-target);
}


char *get_yphoto_item_filename(char *url) {
	struct stat sbuf;
	char *ret=NULL;
	char *uppy=NULL;
	if (! yphoto_cache) {return ret;}
	ret=g_hash_table_lookup(yphoto_cache, url);
	if (ret) {
		if (  stat( ret, &sbuf )) { /* aint there, so re-download */ 			
			char *nret=NULL;
			uppy=download_image(url);
			if (! uppy) {return NULL;}
			if (strlen(uppy)<2) {return NULL;}
			nret=g_strdup(uppy);
			g_hash_table_insert(yphoto_cache, g_strdup(url), nret);
			g_free(ret);
			return nret;
		}
		return ret;
	}
	uppy=download_image(url);
	if (! uppy) {return NULL;}
	if (strlen(uppy)<2) {return NULL;}
	ret=g_strdup(uppy);
		if (YPH_DEBUG) {
		printf("URL: %s\n", url); fflush(stdout);
		printf("ret: %s\n", ret); fflush(stdout);
		}
	g_hash_table_insert(yphoto_cache, g_strdup(url), ret);
	return ret;
}

GdkPixbuf *get_yphoto_item_pixbuf(char *url) {
	GdkPixbuf *imbuf=NULL;
	char *pixfile=NULL;
	pixfile=get_yphoto_item_filename(url);
	if (! pixfile) {
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_no_photo);
	} else {imbuf=gdk_pixbuf_new_from_file( pixfile , NULL);}	
	if (! imbuf) {
		imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_no_photo);
	}
	return imbuf;
}

GdkPixbuf *get_yphoto_item_pixbuf_thumb(char *url, int thumbnail) {
	GdkPixbuf *spixbuf=NULL;
	GdkPixbuf *buf=NULL;
	int awidth=0;
	int aheight=0;	
	buf =get_yphoto_item_pixbuf(url);
	if (! buf) {return spixbuf;}
	awidth = gdk_pixbuf_get_width( buf );
	aheight = gdk_pixbuf_get_height( buf );
	if (thumbnail==1) {
		aheight = 64;
		awidth = 64;
	} else {
		int scaler=awidth;
		if (thumbnail== -1) {
			if (scaler>640) {scaler=640;}
			aheight = ( 1.0 *  aheight / awidth ) * scaler;
			if (aheight>480) {aheight=480;}
			awidth = scaler;
		} else {
		if (scaler>255) {scaler=255;}
		aheight = ( 1.0 *  aheight / awidth ) * scaler;
		if (aheight>320) {aheight=320;}
		awidth = scaler;
		}
	}
	spixbuf = gdk_pixbuf_scale_simple( buf, awidth, aheight,GDK_INTERP_BILINEAR );
	g_object_unref(buf);
	return spixbuf;
}

void yphoto_item_free(gpointer p) {
	struct yphoto_item *f = p;
	char *ss=NULL;
	if (!f) {return;}
	if (f->caption) {	g_free(f->caption); f->caption=ss;}
	if (f->url) {	g_free(f->url); f->url=ss;}
}

struct yphoto_item *yphoto_item_new()
{
	struct yphoto_item *ret;
	ret = g_new0(struct yphoto_item, 1);
	ret->caption=NULL;
	ret->url=NULL;
	ret->width=-1;
	ret->height=-1;
	ret->position=0;
	return ret;
}

struct yphoto_item *yphoto_item_find(GHashTable *hashy, char *bud)
{
	struct yphoto_item *f = NULL;
	char tmp_user[16];
	if (! hashy) {return f;}
	strncpy( tmp_user, bud, 14 );
	lower_str(tmp_user);
	f = g_hash_table_lookup(hashy, tmp_user);
	return f;
}

struct yphoto_item *yphoto_item_jumpto(GHashTable *hashy, char *bud, GtkWidget *imimage, GtkWidget *label) {
	char ydims[16];
	char yfile[16];
	char caption[24];
	char title[88];
	char *ptr=NULL;
	GdkPixbuf *buf=NULL;	
	struct yphoto_item *f = NULL;

				if (YPH_DEBUG) {printf("DEBUG-a\n"); fflush(stdout);}
	f=yphoto_item_find(hashy, bud);
	if (! f) {return f;}
	buf=get_yphoto_item_pixbuf_thumb(f->url, 0);
	if (buf) {
		gtk_image_set_from_pixbuf(GTK_IMAGE(imimage), buf);
		g_object_unref(buf);
	}
				if (YPH_DEBUG) {printf("DEBUG-B\n"); fflush(stdout);}
	snprintf(ydims, 14, "%d x %d", f->width, f->height);
	snprintf(caption, 22, "%s", f->caption);
	ptr=strrchr(f->url,'/');
	if (ptr) {
		ptr++;
		snprintf(yfile, 14, "%s", ptr);
		ptr=strchr(yfile,'?');
		if (ptr) {*ptr='\0';}
	} else {snprintf(yfile, 14, "%s", "");}
				if (YPH_DEBUG) {printf("DEBUG-C\n"); fflush(stdout);}

	snprintf(title, 54, "%s\n%s\n[%d/%d]  %s", caption,  yfile, f->position+1,
	g_hash_table_size( hashy ) , ydims);
	gtk_label_set_text(GTK_LABEL(label), _utf(title) );
				if (YPH_DEBUG) {printf("DEBUG-D\n"); fflush(stdout);}

	return f;
}

int clear_yphoto_hash_cb(gpointer key, gpointer value, gpointer user_data) {
	g_free(key);
	yphoto_item_free(value);
	g_free(value);   /* segfault watch */
 	return 1;

} 
void clear_photos_from_album(GHashTable *hashy) {
	if ( ! hashy ) { return; }
	g_hash_table_foreach_remove(hashy,clear_yphoto_hash_cb,NULL);
}


struct yphoto_album *yphoto_album_new(char *who)
{
	struct yphoto_album *ret=NULL;
	if (!who) {return ret;}
	if (! yphoto_albums) {return ret;}
	ret = g_new0(struct yphoto_album, 1);
	ret->who=g_strdup(who);
	ret->url=NULL;
	ret->title=NULL;
	ret->url_key_pending=NULL;
	ret->url_key=NULL;
	ret->pms=NULL;
	ret->pm_hbox=NULL;
	ret->panel=NULL;
	ret->yphotos=g_hash_table_new(g_str_hash, g_str_equal);
	ret->position=-1;
	ret->i_invite=0;
	ret->they_invite=0;
	ret->loaded=0;
	ret->slideshow=0;
	ret->bounces=0;
	ret->private_session=0;
	g_hash_table_insert(yphoto_albums, g_strdup(who), ret);
	return ret;
}

struct yphoto_album *yphoto_album_find(char *bud)
{
	struct yphoto_album *f = NULL;
	char tmp_user[72];
	if (! yphoto_albums) {return f;}
	strncpy( tmp_user, bud, 70 );
	lower_str(tmp_user);
	f = g_hash_table_lookup(yphoto_albums, tmp_user);
	return f;
}

void yphoto_album_free(gpointer p) {
	struct yphoto_album *f = p;
	char *ss=NULL;
	if (!f) {return;}
	if (f->pms && f->panel) {deactivate_tuxvironment(f->pms);  }
	if (f->pm_hbox && f->panel) {
		if (f->private_session) {
			GtkWidget *closer=NULL;
			closer=gtk_object_get_data(GTK_OBJECT(f->pm_hbox), "window");
			if (closer) {gtk_widget_destroy(closer);}
		} else {
		gtk_container_remove(GTK_CONTAINER(f->pm_hbox), f->panel);
			/*  gtk_widget_destroy(f->panel); */  /* seg-fault */ 
			}
		f->panel=NULL;
		/* f->pm_hbox=NULL; */   /* seg-fault */ 
	}
	if (f->who) {	g_free(f->who); f->who=ss;}
	if (f->url) {	g_free(f->url); f->url=ss;}
	if (f->title) {	g_free(f->title); f->title=ss;}
	if (f->url_key) {	g_free(f->url_key); f->url_key=ss;}
	if (f->url_key_pending) {	g_free(f->url_key_pending); f->url_key_pending=ss;}
	if (f->yphotos) {clear_photos_from_album(f->yphotos); }
	if (f->yphotos) {g_hash_table_destroy(f->yphotos);}
	f->yphotos=NULL;
}

void dealloc_yphoto_buffers() {
	if (yphoto_buf) {free(yphoto_buf); yphoto_buf=NULL;}
	if (yphoto_cookie) {free(yphoto_cookie); yphoto_cookie=NULL;}
	if (yphoto_b64buf) {free(yphoto_b64buf); yphoto_b64buf=NULL;}
}

int yphoto_cleanup_cb(gpointer key, gpointer value, gpointer user_data) {
	g_free(key);
	unlink(value);
	g_free(value);   /* segfault watch */
 	return 1;

} 
void yphoto_cleanup() {
	/* delete all cached files out of the tmp directory */ 
	if ( ! yphoto_cache ) { return; }
	g_hash_table_foreach_remove(yphoto_cache,yphoto_cleanup_cb,NULL);
	dealloc_yphoto_buffers();
}

void remove_yphoto_album(char *user)  {
	char tmp_user[72];
	gpointer key_ptr;
	gpointer value_ptr;
	struct yphoto_album *f = NULL;
	if ( ! yphoto_albums ) { return; }
	strncpy( tmp_user, user, 70 );
	lower_str( tmp_user );

	f=yphoto_album_find(user);
	if (f)  {
		if ( g_hash_table_lookup_extended( yphoto_albums, tmp_user, &key_ptr,
			&value_ptr )) 				{
				g_hash_table_remove( yphoto_albums, tmp_user );
			if ( key_ptr )
				g_free( key_ptr ); // seg-fault watch ? 
			if ( value_ptr ) {
				yphoto_album_free(value_ptr);
				g_free( value_ptr );  // seg-fault watch ?  
			}
											  }

	 }
	dealloc_yphoto_buffers();

 }


int yphoto_conn_ready() {
	if (! yphoto_cookie) {yphoto_cookie=malloc(896);}
	if (! yphoto_cookie) {dealloc_yphoto_buffers();  return 0; }
	if (! yphoto_buf) {yphoto_buf=malloc(YPHOTO_MEM);}
	if (! yphoto_buf) {dealloc_yphoto_buffers(); return 0;}
	if (! yphoto_b64buf) {yphoto_b64buf=malloc(YPHOTO_MEM*2);}
	if (! yphoto_b64buf) {dealloc_yphoto_buffers(); return;}
	set_max_url_fetch_size(YPHOTO_MEM-2);
	sprintf(yphoto_buf, "%s", "");
	sprintf(yphoto_b64buf, "%s", "");
	sprintf(yphoto_cookie, "%s", "");
	YPH_DEBUG=ymsg_sess->debug_packets;
	return 1;
}


void yphoto_pclick() {
	if (! yphoto_conn_ready()) {return;}	
	snprintf(yphoto_cookie, 892, "%s%s" , yphoto_cookie_append, ymsg_sess->cookie);
	snprintf(yphoto_urls, 254, "%s",  "http://pclick.internal.yahoo.com/p/s=96485693/lng=us/rand=0.21274627587944295"); 
	fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
	clear_fetched_url_data();
}


void yphoto_info_load() {
	char *starter=NULL;
	char *stopper=NULL;
	int url_length=0;
	if (! yphoto_conn_ready()) {return;}	
	snprintf(yphoto_cookie, 892, "%s" , ymsg_sess->cookie);
	snprintf(yphoto_urls, 254, "%s",  "http://apiserver.drive.yahoo.com/a/init?.ver=2.5&.src=pg&rand=0.8880263527136096"); 
	url_length=fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
	clear_fetched_url_data();
	if (url_length<2) {dealloc_yphoto_buffers();  return;}
	if (yphoto_url_prefix) {free(yphoto_url_prefix); yphoto_url_prefix=NULL;}
	if (yphoto_challenge) {free(yphoto_challenge); yphoto_challenge=NULL;}
	if (yphoto_server) {free(yphoto_server); yphoto_server=NULL;}
	starter=strstr(yphoto_buf,"URLPREFIX=");
	if (starter) {
		snprintf(yphoto_urls, 200, "%s", starter+10);
		stopper=strchr(yphoto_urls,'\n');
		if (stopper) {*stopper='\0';}
		yphoto_url_prefix=strdup(yphoto_urls);
		if (YPH_DEBUG) {printf("yphoto_url_prefix:  %s\n",  yphoto_url_prefix); fflush(stdout);} 
	}
	starter=strstr(yphoto_buf,"BC_API_SERVER=");
	if (starter) {
		snprintf(yphoto_urls, 200, "%s", starter+14);
		stopper=strchr(yphoto_urls,'\n');
		if (stopper) {*stopper='\0';}
		yphoto_server=strdup(yphoto_urls);
				if (YPH_DEBUG) {printf("yphoto_server:  %s\n",  yphoto_server); fflush(stdout); }
	}
	starter=strstr(yphoto_buf,"CHALLENGE=");
	if (starter) {
		snprintf(yphoto_urls, 200, "%s", starter+10);
		stopper=strchr(yphoto_urls,'\n');
		if (stopper) {*stopper='\0';}
		yphoto_challenge=strdup(yphoto_urls);
		if (YPH_DEBUG) {printf("yphoto_challenge:  %s\n",  yphoto_challenge); fflush(stdout); }
	}
}

void parse_yphoto_album(struct yphoto_album *ya, char *pdata) {
	int photo_counter=0;
	int pic_width=0;
	int pic_height=0;
	char *starter=NULL;
	char *stopper=NULL;

	snprintf(yphoto_b64buf, YPHOTO_MEM, "%s", pdata);
	snprintf(yphoto_cookie, 6, "%s", "");

	starter=strstr(yphoto_b64buf, "prefix:\"");
	if (starter) {
		starter += strlen("prefix:\"");
		stopper=strchr(starter,'"');	
		if (stopper) {
			*stopper='\0';
			snprintf(yphoto_cookie, 256, "%s", starter);
			ya->url=g_strdup(yphoto_cookie);
			*stopper='"';
		}
	}


	if (! ya->url) {return; }

	starter=strstr(yphoto_b64buf, "caption:\"");
	if (starter) {
		starter += strlen("caption:\"");
		stopper=strchr(starter,'"');	
		if (stopper) {
			*stopper='\0';
			ya->title=g_strdup(starter);
			*stopper='"';
		}
	}

	if (! ya->title) {ya->title=g_strdup("Album");}
	
	starter=strstr(yphoto_b64buf, "fullName:\"");	
	while (starter) {
		char pfind[24];
		char *pstart=NULL;
		char *purl=NULL;
		char *pcaption=NULL;
		char *pstop=NULL;
		starter += strlen("fullName:\"");
		stopper=strstr(starter,"order:");	
		if (stopper) {*stopper='\0';}
		snprintf(yphoto_cookie, 892, "%s", starter);

		snprintf(pfind, 22, "%s", "caption:\"");
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += strlen(pfind);
			pstop=strchr(pstart, '"');
			if (pstop) { *pstop='\0'; pcaption=strdup(pstart); *pstop='"'; 	}
		}

		snprintf(pfind, 22, "%s", "thumb:\"");
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += strlen(pfind);
			pstop=strchr(pstart, '"');
			if (pstop) { *pstop='\0'; purl=strdup(pstart); *pstop='"'; 	}
		}

		snprintf(pfind, 22, "%s", "fileName:\"");
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += strlen(pfind);
			pstop=strchr(pstart, '"');
			if (pstop) { 
				*pstop='\0'; 
				if (! pcaption) {pcaption=strdup(pstart); }
				else {
					if (strlen(pcaption)<2) {free(pcaption); pcaption=strdup(pstart); }
					else {
						char *redup=NULL;
						redup=malloc(strlen(pcaption)+32);
						if (redup) {
							snprintf(redup, strlen(pcaption)+5, "%s  [", pcaption);
							strncat(redup, pstart, 22);	
							strcat(redup,"]");
							free(pcaption);
							pcaption=strdup(redup);
							free(redup);
						}
					}
				}
				*pstop='"'; 
			}
		}

		snprintf(pfind, 22, "%s", "width:\"");
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += strlen(pfind);
			pstop=strchr(pstart, '"');
			if (pstop) { *pstop='\0'; pic_width=atoi(pstart);  *pstop='"'; 	}
		}
		snprintf(pfind, 22, "%s", "height:\"");
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += strlen(pfind);
			pstop=strchr(pstart, '"');
			if (pstop) { *pstop='\0'; pic_height=atoi(pstart);  *pstop='"'; 	}
		}

		if (purl && (! pcaption)) {pcaption=strdup("");}

		if (purl && pcaption) {
			struct yphoto_item *yit=NULL;
			yit=yphoto_item_new();
			if (yit) {
				snprintf(yphoto_cookie, 512, "%s%s", ya->url, purl);
				yit->url=g_strdup(yphoto_cookie);
				yit->caption=g_strdup(pcaption);
				yit->height=pic_height;
				yit->width=pic_width;
				yit->position=photo_counter;
				snprintf(pfind, 10, "\"%d\"", yit->position);
				g_hash_table_insert(ya->yphotos, g_strdup(pfind), yit);
				photo_counter++;
			}
		}

		if (pcaption) {free(pcaption);}
		if (purl) {free(purl);}
		if (! stopper) {starter=NULL; break;}
		stopper++;
		starter=stopper;
	}

}

void yphoto_post_album(char *album, char *token, char *caption, struct yphoto_album *ya) {
	char me_user[64];
	char *starter=NULL;
	char *stopper=NULL;
	char *tmpbuf=NULL;
	int url_length=0;
	int written=0;
	if (! yphoto_url_prefix) {dealloc_yphoto_buffers(); return;}
	if (! yphoto_server) {dealloc_yphoto_buffers(); return;}

	snprintf(me_user, 62, "%s", ymsg_sess->user);
	lower_str(me_user);
	snprintf(yphoto_buf, 300, "['{prefix:\"%s\",path:\"/%s\",dirName:\"%s\",caption:\"%s\",token:\"%s\",id:\"%s\",files:[", yphoto_url_prefix, album, album, caption, token, me_user);

	starter=yphoto_b64buf;
	
	while (starter) {
		char pfilen[24];
		char pfind[24];
		int scwidth=0;
		int scheight=0;
		char *pstart=NULL;
		char *pname=NULL;
		char *pcaption=NULL;
		char *purl=NULL;
		char *ptoken=NULL;
		char *pdescription=NULL;
		char *pstop=NULL;
		stopper=strchr(starter,'\n');	
		if (stopper) {*stopper='\0';}
		snprintf(yphoto_cookie, 892, "%s%c", starter, 0x01);

		pstart=yphoto_cookie;
		if (pstart) {
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pname=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%cdesc%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; pdescription=strdup(pstart); *pstop=0x01; }
		}


		snprintf(pfind, 22, "%ccaption%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1;
			pstop=strchr(pstart, 0x01);
			if (pstop) {
				*pstop='\0'; pstart += strlen(pfind)-1; 
				pcaption=strdup(pstart); 
				if (! pdescription) {pdescription=strdup(pstart);}
				*pstop=0x01; 
				snprintf(pfilen, 22, "%s", pcaption);
				pstop=strchr(pfilen, '.');
				if (pstop) {*pstop='\0';}
				}
		}

		snprintf(pfind, 22, "%cscaltname%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; purl=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%cscwidth%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; scwidth=atoi(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%cscheight%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; scheight=atoi(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%ctok%c", 0x01, 0x02);
		pstart=strstr(yphoto_cookie, pfind);
		if (pstart) {
			pstart += 1;
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0';  pstart += strlen(pfind)-1; ptoken=strdup(pstart); *pstop=0x01;}
		}


		if (YPH_DEBUG) {
			printf("STUFF: %s %s %s %s %d %d\n", ptoken , pcaption, pname , purl, 
			scwidth, scheight ); 
			fflush(stdout);
		}

		if (! ptoken) {ptoken=strdup("phls76ABVnqztUvq");}
		if (! pcaption) {pcaption=strdup("??");}

		if (ptoken && pcaption && purl && pname) {
			snprintf(yphoto_cookie, 512, "{fullName:\"%s\",caption:\"%s\",thumb:\"%s\",fileName:\"%s\",tok:\"%s\",width:\"%d\",height:\"%d\",order:0}" ,  pname, pdescription?pdescription:pcaption, purl, pfilen , ptoken, scwidth, scheight);
			if (written) {strcat(yphoto_buf, ","); }
			strncat(yphoto_buf, yphoto_cookie, 512);
			written=1;
		}

		if (ptoken) {free(ptoken);}
		if (pdescription) {free(pdescription);}
		if (pname) {free(pname);}
		if (purl) {free(purl);}
		if (pcaption) {free(pcaption);}
		if (! stopper) {starter=NULL; break;}
		stopper++;
		starter=stopper;
		if (strlen(yphoto_buf)>(YPHOTO_MEM-1024) ) {break;}
	}



	strcat(yphoto_buf, "]}']");

	parse_yphoto_album(ya, yphoto_buf);
	if (ya->private_session) {
			GtkWidget *mypcap=NULL;
			ya->position=-1;
			ya->loaded=1;
			ya->i_invite=1;
			snprintf(yphoto_cookie, 32, " %s",ya->title?ya->title:" " );
			strcat(yphoto_cookie, " ");
			mypcap=gtk_object_get_data(GTK_OBJECT(ya->panel), "frame" );
			if (mypcap) {
				gtk_frame_set_label(GTK_FRAME(mypcap), _utf(yphoto_cookie));
			}
			on_yphoto_forward(NULL, ya);
	return; 
	}

	snprintf(yphoto_cookie, 892, "B=9a4k50l00fmia&b=2; Q=q1=AACAAAAAAAAAbw--&q2=QJMimQ--; F=a=.Z6jMDgsvaBtGShSqwPBTq0Ky1SSv3GwNnlt3BTHmMAZ2FO0YmBtuOWyfWKR&b=B0ZL; LYC=l_v=2&l_lv=7&l_l=f7hep4dicea4&l_s=qyuztxrs5r3tusxvy2ts343ys1xq4t5u&l_lid=1at6cle&l_r=6o&l_lc=0_0_32_0_-1&l_um=1_1_0_0_0; U=mt=aETWYZ2MhYukV8IY6JwV4Zx1O8VdrKvKeQt1Wg--&ux=q9r0AB&un=4ruau026ct429; PROMO=sessionID=; LYS=l_fh=1&l_vo=myla; %s" , ymsg_sess->cookie);
	snprintf(yphoto_urls, 254, "%s", "http://in1.msg.sc5.yahoo.com/share/put/"); 

		if (YPH_DEBUG) {
		printf("\n post- Data: [%d]\n%s\n", strlen(yphoto_buf), yphoto_buf);
			fflush(stdout);
		}

	sprintf(yphoto_b64buf, "%s", "");
	yphoto_b64_encode(yphoto_b64buf, YPHOTO_MEM*2, yphoto_buf , strlen(yphoto_buf));

	sprintf(yphoto_buf, "%s", "");
		tmpbuf=malloc(strlen(yphoto_b64buf)+32);
		if (tmpbuf) {
				/* post format: data=[stuff]== */
			snprintf(tmpbuf,strlen(yphoto_b64buf)+29, "data=%s", yphoto_b64buf);
			if (strstr(tmpbuf,"==")) {
				char *pttr=NULL;
				pttr=strstr(tmpbuf,"==");
				if (pttr) {*pttr='\0';}
			}
			strcat(tmpbuf, "%3D%3D");
				if (YPH_DEBUG) {
					printf("\npost- b64: %s\n", tmpbuf);
					fflush(stdout);
				}
			set_url_post_data(tmpbuf);
			url_length=fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
			unset_url_post_data();
			clear_fetched_url_data();
			free(tmpbuf);
		}
	if (url_length<2) {dealloc_yphoto_buffers(); return;}

	if (YPH_DEBUG) {
			printf("\n receive- Data: [%d]\n%s\n", strlen(yphoto_buf), yphoto_buf);
			fflush(stdout);
	}

	starter=strstr(yphoto_buf, "url=");
	if (! starter) {dealloc_yphoto_buffers();  return;}
	starter += 4;
	stopper=strstr(starter, "%3A");
	while (stopper) {
		*stopper=':'; *(stopper+1)=' '; *(stopper+2)=' ';
		stopper=strstr(starter, "%3A");
	}
	stopper=strstr(starter, "%2F");
	while (stopper) {
		*stopper='/'; *(stopper+1)=' '; *(stopper+2)=' ';
		stopper=strstr(starter, "%2F");
	}
	stopper=strstr(starter, "%3F");
	while (stopper) {
		*stopper='?'; *(stopper+1)=' '; *(stopper+2)=' ';
		stopper=strstr(starter, "%3F");
	}
	stopper=strstr(starter, "%3D");
	while (stopper) {
		*stopper='='; *(stopper+1)=' '; *(stopper+2)=' ';
		stopper=strstr(starter, "%3D");
	}
	snprintf(yphoto_cookie, 4, "%s" , "");
	url_length=0;
	while (*starter) {
		if (*(starter) ==' ') {starter++; continue;}
		yphoto_cookie[url_length++]=*(starter)++;
	}
	yphoto_cookie[url_length]='\0';
	
	ya->url_key=g_strdup(yphoto_cookie);
	if (ya->url && ya->who && ya->title && ya->url_key && 
	ya->pms && ya->pm_hbox && ya->panel ) {
		if (strstr(ya->url_key, "http:")) {
			char bud[8];
			GtkWidget *imimage=NULL;
			GtkWidget *mypcap=NULL;
			GtkWidget *label=NULL;
			imimage=gtk_object_get_data(GTK_OBJECT(ya->panel), "image" );
			if (! imimage) {return;}
				if (YPH_DEBUG) {printf("DEBUG-1\n"); fflush(stdout);}
			label=gtk_object_get_data(GTK_OBJECT(ya->panel), "label" );
			if (! label) {return;}
			mypcap=gtk_object_get_data(GTK_OBJECT(ya->panel), "frame" );
			if (! mypcap) {return;}
				if (YPH_DEBUG) {printf("DEBUG-2\n"); fflush(stdout);}
			ya->position=0;
			ya->loaded=1;
			ya->i_invite=1;
				if (YPH_DEBUG) {printf("DEBUG-3\n"); fflush(stdout);}
			snprintf(yphoto_cookie, 24, " %s",ya->title );
			strcat(yphoto_cookie, " ");
				if (YPH_DEBUG) {printf("DEBUG-4\n"); fflush(stdout);}
			gtk_frame_set_label(GTK_FRAME(mypcap), _utf(yphoto_cookie));
				if (YPH_DEBUG) {printf("DEBUG-5\n"); fflush(stdout);}
			snprintf(bud, 6, "\"%d\"", ya->position);
				if (YPH_DEBUG) {printf("DEBUG-6\n"); fflush(stdout);}

			/* send invite with URL notify here */
			snprintf(yphoto_infos, 12 ,"%s", "null" );
			yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 3);
			snprintf(yphoto_infos, 350 ,"\"%s\"", ya->url_key);
			yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 2);

			yphoto_item_jumpto(ya->yphotos, bud, imimage, label);
				if (YPH_DEBUG) {printf("DEBUG-7\n"); fflush(stdout);}
			if (ya->pms) {focus_pm_entry(ya->pms->pm_window);}
				if (YPH_DEBUG) {printf("DEBUG-8\n"); fflush(stdout);}

		} 
	}
}



void close_yphoto_sel(GtkButton *widget, gpointer user_data) {
	GtkWidget *win=NULL;
	win=gtk_object_get_data(GTK_OBJECT(widget), "window");
	if (win) {gtk_widget_destroy(win);}
	dealloc_yphoto_buffers(); 
}

void reset_yphoto_album(struct yphoto_album *ya) {
	if (ya->url) {	g_free(ya->url); ya->url=NULL;}
	if (ya->title) {	g_free(ya->title); ya->title=NULL;}
	if (ya->url_key) {	g_free(ya->url_key); ya->url_key=NULL;}
	clear_photos_from_album(ya->yphotos);
	ya->position=0;
	ya->i_invite=0;
	ya->they_invite=-1;
	ya->loaded=0;
	ya->slideshow=0;
	ya->bounces=0;
}

void load_yphoto_album(struct yphoto_album *ya) {
	char *starter=NULL;
	char *stopper=NULL;
	int url_length=0; 
	if (! ya) {return;}
	if (! ya->url_key_pending) {return;}
	yphoto_pclick();
	yphoto_pclick();
	if (! yphoto_conn_ready()) {dealloc_yphoto_buffers();  return;}	
	ya->slideshow=0;
	snprintf(yphoto_cookie, 892, "B=9a4k50l00fmia&b=2; Q=q1=AACAAAAAAAAAbw--&q2=QJMimQ--; F=a=.Z6jMDgsvaBtGShSqwPBTq0Ky1SSv3GwNnlt3BTHmMAZ2FO0YmBtuOWyfWKR&b=B0ZL; LYC=l_v=2&l_lv=7&l_l=f7hep4dicea4&l_s=qyuztxrs5r3tusxvy2ts343ys1xq4t5u&l_lid=1at6cle&l_r=6o&l_lc=0_0_32_0_-1&l_um=1_1_0_0_0; U=mt=aETWYZ2MhYukV8IY6JwV4Zx1O8VdrKvKeQt1Wg--&ux=q9r0AB&un=4ruau026ct429; PROMO=sessionID=; LYS=l_fh=1&l_vo=myla; %s" , ymsg_sess->cookie);
	snprintf(yphoto_urls, 254,"%s", ya->url_key_pending); 
	url_length=fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
	clear_fetched_url_data();
	if (url_length<2) {dealloc_yphoto_buffers(); return;}

	if (YPH_DEBUG) {
		printf("\nssData: [%d]\n%s\n", strlen(yphoto_buf), yphoto_buf);
			fflush(stdout);
	}

	starter=strstr(yphoto_buf, "=1010\n");
	if (! starter) {dealloc_yphoto_buffers(); return;}	
		starter += 6; 

	stopper=strstr(yphoto_buf, "==");
	if (stopper) {*stopper='\0';}

	if (YPH_DEBUG) {
		printf("\nss-b64-Data: [%d]\n%s\n", strlen(starter), starter);
			fflush(stdout);
	}

	yphoto_b64_decode(yphoto_b64buf, YPHOTO_MEM*2, starter , strlen(starter));
		if (YPH_DEBUG) {printf("\nb64: %s\n", yphoto_b64buf); fflush(stdout);}
	if (ya->loaded) {reset_yphoto_album(ya);}
	if (ya->url_key) {g_free(ya->url_key); }
	ya->url_key=g_strdup(ya->url_key_pending);
	g_free(ya->url_key_pending); 
	ya->url_key_pending=NULL;
	snprintf(yphoto_buf, YPHOTO_MEM-2, "%s", yphoto_b64buf);
	parse_yphoto_album(ya, yphoto_buf);

		if (YPH_DEBUG) {
			printf("GOT-it-1: URL: %s WHO: %s  Title: %s,\nKey: %s, PMS: %d, PMSbox: %d   PMS-panel: %d\n", 
			ya->url, ya->who, ya->title, ya->url_key, ya->pms?1:0, ya->pm_hbox?1:0, ya->panel?1:0); 
			fflush(stdout);
		}


	if (ya->url && ya->who && ya->title && ya->url_key && 
	ya->pms && ya->pm_hbox && ya->panel ) {
			char bud[8];
			GtkWidget *imimage=NULL;
			GtkWidget *mypcap=NULL;
			GtkWidget *label=NULL;
			imimage=gtk_object_get_data(GTK_OBJECT(ya->panel), "image" );
			if (! imimage) {return;}
		if (YPH_DEBUG) {printf("GOT-it-2\n"); fflush(stdout);}
			label=gtk_object_get_data(GTK_OBJECT(ya->panel), "label" );
			if (! label) {return;}
		if (YPH_DEBUG) {printf("GOT-it-3\n"); fflush(stdout);}
			mypcap=gtk_object_get_data(GTK_OBJECT(ya->panel), "frame" );
			if (! mypcap) {return;}
		if (YPH_DEBUG) {printf("GOT-it-4\n"); fflush(stdout);}
			ya->position=0;
			ya->loaded=1;
			ya->i_invite=2;
			ya->they_invite=1;
			snprintf(yphoto_cookie, 24, " %s",ya->title );
			strcat(yphoto_cookie, " ");
			gtk_frame_set_label(GTK_FRAME(mypcap), _utf(yphoto_cookie));
			snprintf(yphoto_infos, 3 ,"%s", "1" );
		if (YPH_DEBUG) {printf("GOT-it-5\n"); fflush(stdout);}
			yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 4);
			snprintf(bud, 6, "\"%d\"", ya->position);
			yphoto_item_jumpto(ya->yphotos, bud, imimage, label);
				if (ya->pms) { focus_pm_entry(ya->pms->pm_window); }
		if (YPH_DEBUG) {printf("GOT-it-6\n"); fflush(stdout);}
	}
}


void select_yphoto_sel(GtkButton *widget, gpointer user_data) {
	struct yphoto_album *ya=NULL;
	char *starter=NULL;
	char *stopper=NULL;
	int url_length=0;
	char *album=NULL;
	char *token=NULL;
	char *caption=NULL;

	ya=user_data;
	if (! ya) {return;}

	album=gtk_object_get_data(GTK_OBJECT(widget), "album");
	if (YPH_DEBUG) {printf("Selection-album: %s\n", album); fflush(stdout);}

	token=gtk_object_get_data(GTK_OBJECT(widget), "token");
	if (YPH_DEBUG) {printf("Selection-token: %s\n", token); fflush(stdout);}

	caption=gtk_object_get_data(GTK_OBJECT(widget), "caption");
	if (YPH_DEBUG) {printf("Selection-caption: %s\n", caption); fflush(stdout);}

	if (! album) {dealloc_yphoto_buffers(); return;}
	if (! token) {dealloc_yphoto_buffers(); return;}
	if (! caption) {dealloc_yphoto_buffers(); return;}

	if (! yphoto_url_prefix) {dealloc_yphoto_buffers(); return;}
	if (! yphoto_server) {dealloc_yphoto_buffers(); return;}
	if (! yphoto_conn_ready()) {dealloc_yphoto_buffers();  return;}	
	ya->slideshow=0;
	snprintf(yphoto_cookie, 892, "%s" , ymsg_sess->cookie);

	/* This returns bloated and poorly encrypted base-64 format info that often provides 
	and incomplete list of photos in the album */ 
	/* 
	snprintf(yphoto_urls, 254, "%slist?.ver=1.1&.src=pg&.moreids=y&.flags=tok&.recur=n&.path=/%s&enc=base64&rand=0.38021902989431977",  yphoto_server, album); */

	/* 
		Yahoo seems to have some kind of limit on the 
		amount of data its servers send back about photo albums, 
		sometimes leading to incomplete listings...if this happens, 
		disable the 'tok' flag in the URL, make sure base-64 
		encoding is OFF, and fall back to 'ver=1.1' (shorter headers) 
	*/

	snprintf(yphoto_urls, 254, "%slist?.ver=2.5&.src=pg&.moreids=y&.flags=tok&.recur=n&.path=/%s%s&rand=0.38021902989431977",  yphoto_server, album, YPHOTO_USE_BASE64?"&enc=base64":"");

	url_length=fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
	clear_fetched_url_data();
	if (url_length<2) {dealloc_yphoto_buffers(); return;}

	if (YPH_DEBUG) {
		printf("\nssData: [%d]\n%s\n", strlen(yphoto_buf), yphoto_buf);
			fflush(stdout);
	}

	starter=strstr(yphoto_buf, "\n\n");
	if (! starter) {dealloc_yphoto_buffers(); return;}	
		starter += 2; 

	if (YPH_DEBUG) {
		printf("\nss-b64-Data: [%d]\n%s\n", strlen(starter), starter);
			fflush(stdout);
	}

	if (YPHOTO_USE_BASE64) {
		yphoto_b64_decode(yphoto_b64buf, YPHOTO_MEM*2, starter , strlen(starter));
	} else {snprintf(yphoto_b64buf, (YPHOTO_MEM*2)-3, "%s", starter );}

	if (YPH_DEBUG) {printf("\nb64: %s\n", yphoto_b64buf); fflush(stdout);}

	reset_yphoto_album(ya);
	if ((! ya->private_session) && (ya->pms) ) {
			snprintf(yphoto_infos, 12 ,"%s", " " );
			yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 7);
			snprintf(yphoto_infos, 12 ,"%s", "null" );
			yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 5);
	}

	yphoto_post_album(album, token, caption, ya);
	close_yphoto_sel(widget, user_data);
	if (! ya->loaded) {show_ok_dialog(_("The photo album could not be loaded."));}
}


void remove_yphoto_dups() {
	oyphoto_list = gyach_g_list_free( oyphoto_list );
}

void yphoto_append_albums(struct yphoto_album *ya) {
	int hlen=0;
	GtkWidget *tuxwin;
	GtkWidget *vbox_main;
        GtkWidget *okbutton;
	GtkWidget *canbutton;
	GtkWidget *hbox;
	char *starter=NULL;
	char *stopper=NULL;
	int url_length=0;
		if (YPH_DEBUG) {printf("here-11\n"); fflush(stdout);}
	yphoto_pclick();
		if (YPH_DEBUG) {printf("here-12\n"); fflush(stdout);}
	yphoto_pclick();
		if (YPH_DEBUG) {printf("here-13\n"); fflush(stdout);}
	yphoto_info_load();
		if (YPH_DEBUG) {printf("here-14\n"); fflush(stdout);}

	if (! yphoto_url_prefix) {dealloc_yphoto_buffers(); return;}
	if (! yphoto_server) {dealloc_yphoto_buffers(); return;}
	if (! yphoto_conn_ready()) {dealloc_yphoto_buffers(); return;}	
	snprintf(yphoto_cookie, 892, "%s" , ymsg_sess->cookie);

	/* This sends our data back in version 1.1. base-64 format but often contains 
	an incomplete list of photo albums.  */ 
	/* 
	snprintf(yphoto_urls, 254, "%s%s",  yphoto_server, "list?.ver=1.1&.src=pg&.flags=tok,showcover&.moreids=n&.recur=n&.path=/&enc=base64&rand=0.5809511944457084");  */ 

	/* 
		Yahoo seems to have some kind of limit on the 
		amount of data its servers send back about photo albums, 
		sometimes leading to incomplete listings...if this happens, 
		disable the 'tok' flag in the URL, make sure base-64 
		encoding is OFF, and fall back to 'ver=1.1' (shorter headers) 
	*/

	snprintf(yphoto_urls, 254, "%slist?.ver=2.5&.src=pg&.flags=tok,showcover&.moreids=y&.recur=n&.path=/%s&rand=0.5809511944457084",  yphoto_server, YPHOTO_USE_BASE64?"&enc=base64":""); 

	url_length=fetch_url(yphoto_urls, yphoto_buf , yphoto_cookie);
	clear_fetched_url_data();
	if (url_length<2) {dealloc_yphoto_buffers(); return;}

	if (YPH_DEBUG) {
		printf("\nuData: [%d]\n%s\n", strlen(yphoto_buf), yphoto_buf);
			fflush(stdout);
	}

	starter=strstr(yphoto_buf,"URLPREFIX=");
	if (starter) {
		snprintf(yphoto_urls, 200, "%s", starter+10);
		stopper=strchr(yphoto_urls,'\n');
		if (stopper) {*stopper='\0';}
		if (yphoto_url_prefix) {free(yphoto_url_prefix); yphoto_url_prefix=NULL;}
		yphoto_url_prefix=strdup(yphoto_urls);
		if (stopper) {*stopper='\n';}
		if (YPH_DEBUG) {printf("yphoto_url_prefix-2:  %s\n",  yphoto_url_prefix); fflush(stdout);} 
	}
	stopper=NULL;

	starter=strstr(yphoto_buf, "\n\n");
	if (! starter) {dealloc_yphoto_buffers(); return;}	
		starter += 2;

	if (YPH_DEBUG) {
		printf("\nstartData: [%d]\n%s\n", strlen(starter), starter);
			fflush(stdout);
	}

	if (YPHOTO_USE_BASE64) {
		yphoto_b64_decode(yphoto_b64buf, YPHOTO_MEM*2, starter , strlen(starter));
	} else {snprintf(yphoto_b64buf, (YPHOTO_MEM*2)-3, "%s", starter );}

	if (YPH_DEBUG) {
	printf("\nb64: %s\n", yphoto_b64buf);
			fflush(stdout);
	}

  tuxwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (tuxwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (tuxwin), _("Select A Photo Album...") );
  gtk_window_set_position (GTK_WINDOW (tuxwin), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (tuxwin), TRUE);
  vbox_main = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 5);
  gtk_container_add (GTK_CONTAINER (tuxwin), vbox_main);
  gtk_box_pack_start (GTK_BOX (vbox_main), gtk_label_new(_("Select A Photo Album...")), FALSE, FALSE, 5);
	hbox=gtk_hbox_new (FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (vbox_main), hbox, FALSE, FALSE, 2);

	remove_yphoto_dups(); 

	starter=yphoto_b64buf;
	
	while (starter) {
		char pfind[24];
		char *pstart=NULL;
		char *palbum=NULL;
		char *pdate=NULL;
		char *pcaption=NULL;
		char *pcover=NULL;
		char *ptoken=NULL;
		char *pstop=NULL;
		stopper=strchr(starter,'\n');	
		if (stopper) {*stopper='\0';}
		snprintf(yphoto_buf, 2048, "%s%c", starter, 0x01);

		pstart=yphoto_buf;
		if (pstart) {
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; palbum=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%cdate%c", 0x01, 0x02);
		pstart=strstr(yphoto_buf, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; pdate=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%ccaption%c", 0x01, 0x02);
		pstart=strstr(yphoto_buf, pfind);
		if (pstart) {
			pstart += 1;
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; pcaption=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "coverimg_altname%c", 0x02);
		pstart=strstr(yphoto_buf, pfind);
		if (pstart) {
			pstart += 1; 
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0'; pstart += strlen(pfind)-1; pcover=strdup(pstart); *pstop=0x01; }
		}

		snprintf(pfind, 22, "%ctok%c", 0x01, 0x02);
		pstart=strstr(yphoto_buf, pfind);
		if (pstart) {
			pstart += 1;
			pstop=strchr(pstart, 0x01);
			if (pstop) {*pstop='\0';  pstart += strlen(pfind)-1; ptoken=strdup(pstart); *pstop=0x01;}
		}

		if (! pdate) {pdate=strdup("1086090622");}
		if (! pcaption) {pcaption=strdup("??");}
		if (! ptoken) {ptoken=strdup("phls76ABVnqztUvq");}

	if (YPH_DEBUG) {
		printf("STUFF: %s %s %s %s\n", ptoken , pcaption, palbum , pcover); 
		fflush(stdout);
	}

		if (ptoken && pcaption && palbum && pcover) {
			char mycap[16];
			gchar cop_cover[32];
			gchar cop_album[32];
			gchar cop_token[32];
			char *duppy1=NULL;
			char *duppy2=NULL;
			char *duppy3=NULL;
			GdkPixbuf *pbuf=NULL;
			GtkWidget *bbox=NULL;
			GtkWidget *imimage=NULL;
			bbox=gtk_vbox_new (FALSE, 0);
			okbutton=gtk_button_new();
			gtk_container_add(GTK_CONTAINER(okbutton), bbox);
			imimage=gtk_image_new();
			snprintf(yphoto_urls, 254, "%s%s", yphoto_url_prefix, pcover);

			pbuf=get_yphoto_item_pixbuf_thumb(yphoto_urls, 1);
			if (pbuf) { 
				gtk_image_set_from_pixbuf(GTK_IMAGE(imimage), pbuf); 
				g_object_unref(pbuf); 
			} else {
				gtk_image_set_from_stock(GTK_IMAGE(imimage), GTK_STOCK_NEW, GTK_ICON_SIZE_DND); 
					} 
			gtk_box_pack_start (GTK_BOX (bbox), imimage, TRUE, TRUE, 2);
			snprintf(mycap, 9, "%s", pcaption);
			strcat(mycap, "...");
			gtk_box_pack_start (GTK_BOX (bbox), gtk_label_new(_utf(mycap)), FALSE, FALSE, 0);
			snprintf(cop_album, 30, "%s", palbum);
			snprintf(cop_cover, 30, "%s", pcaption);
			snprintf(cop_token, 30, "%s", ptoken);
			duppy1=strdup(cop_album);  /* These get freed next time around */ 
			duppy2=strdup(cop_cover);
			duppy3=strdup(cop_token);
			oyphoto_list = g_list_append( oyphoto_list, duppy1);
			oyphoto_list = g_list_append( oyphoto_list, duppy2);
			oyphoto_list = g_list_append( oyphoto_list, duppy3);
			gtk_object_set_data(GTK_OBJECT(okbutton), "album", duppy1 );
			gtk_object_set_data(GTK_OBJECT(okbutton), "caption", duppy2 );
			gtk_object_set_data(GTK_OBJECT(okbutton), "token", duppy3 );
			hlen++;

			if (hlen==6) {
				hbox=gtk_hbox_new (FALSE, 0);
  				gtk_box_pack_start (GTK_BOX (vbox_main), hbox, FALSE, FALSE, 2);
				hlen=0;
			}
		snprintf(yphoto_infos, 300, "\"%s\"\n%s", pcaption, palbum);
		if (pdate) {
			time_t mytime=0;
			char *tmp_time=NULL;
			char timebuf[40]="";
			mytime=time(NULL);
			mytime=mytime-time(NULL)+atol(pdate);
			tmp_time=strdup(ctime_r( &mytime, timebuf));
			strncat(yphoto_infos,"\nLast Modified: ", 32);
			strncat(yphoto_infos,tmp_time, 45 );
			if (tmp_time) {free(tmp_time);}
		}
		set_tooltip(okbutton, _utf(yphoto_infos));
		gtk_box_pack_start (GTK_BOX (hbox), okbutton, TRUE, TRUE, 2);
		gtk_object_set_data (GTK_OBJECT (okbutton), "window",tuxwin);
  		gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      GTK_SIGNAL_FUNC (select_yphoto_sel),
                      ya);
		}

		if (ptoken) {free(ptoken);}
		if (pcover) {free(pcover);}
		if (pdate) {free(pdate);}
		if (palbum) {free(palbum);}
		if (pcaption) {free(pcaption);}
		if (! stopper) {starter=NULL; break;}
		stopper++;
		starter=stopper;
	}


	if (hlen<7) {
		gtk_box_pack_start (GTK_BOX (vbox_main), gtk_label_new(""), TRUE, TRUE, 0);
	}
  canbutton=  get_pixmapped_button(_("Cancel"), GTK_STOCK_CANCEL);
  set_tooltip(canbutton,_("Cancel"));
  gtk_box_pack_start (GTK_BOX (vbox_main), canbutton, FALSE, FALSE, 6);
  gtk_object_set_data (GTK_OBJECT (canbutton), "window",tuxwin);
  gtk_signal_connect (GTK_OBJECT (canbutton), "clicked",
                      GTK_SIGNAL_FUNC (close_yphoto_sel),
                      NULL);
  gtk_widget_show_all(tuxwin);

}


void on_yphoto_accept_invite(GtkButton *widget, gpointer user_data) {
	GtkWidget *mywin=NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {	 return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	reset_yphoto_album(ya);
	load_yphoto_album(ya);
	mywin=gtk_object_get_data(GTK_OBJECT(widget),"mywindow");
	if (mywin) {gtk_widget_destroy(mywin);}
	if (! ya->loaded) {
		show_ok_dialog(_("The photo album could not be loaded."));
		ya->i_invite=3;
		if (ya->pms) {
				snprintf(yphoto_infos, 15 ,"%s", "1" );
				yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 3);
		}
	}
}

void on_yphoto_reject_invite(GtkButton *widget, gpointer user_data) {
	GtkWidget *mywin=NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {	 return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	ya->i_invite=3;
	snprintf(yphoto_infos, 3 ,"%s", "1" );
	yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 3);
	mywin=gtk_object_get_data(GTK_OBJECT(widget),"mywindow");
	if (mywin) {gtk_widget_destroy(mywin);}
}

void on_yphoto_goto(GtkButton *widget, gpointer user_data) {
	/* go to website */ 
	struct yphoto_album *ya=NULL;
	ya=user_data;
	snprintf(yphoto_urls, 128, "%s", "http://photos.yahoo.com");
	snprintf( yphoto_infos, 600, browser_command,yphoto_urls);
	my_system( yphoto_infos );	 
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
}


void on_yphoto_fullview_close(GtkButton *widget, gpointer user_data) {
	GtkWidget *mywin=NULL;
	mywin=gtk_object_get_data(GTK_OBJECT(widget),"mywindow");
	if (mywin) {gtk_widget_destroy(mywin);}
}

void on_yphoto_full_view(GtkButton *widget, gpointer user_data) {
	/* full view pic  */ 
	char bud[8];
	GdkPixbuf *buf=NULL;	
	struct yphoto_item *f = NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}

	snprintf(bud, 6, "\"%d\"", ya->position);
	f=yphoto_item_find(ya->yphotos, bud);
	if (! f) {return ;}
	buf=get_yphoto_item_pixbuf_thumb(f->url, -1);
	if (! buf) {
		return; 
		} 	else {
	GtkWidget *tuxwin, *vbox_main, *imimage, *ybutton;
  tuxwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (tuxwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (tuxwin), _("Y! Photos") );
  gtk_window_set_position (GTK_WINDOW (tuxwin), GTK_WIN_POS_CENTER);
  vbox_main = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 3);
  gtk_container_add (GTK_CONTAINER (tuxwin), vbox_main);
	imimage=gtk_image_new();
	gtk_image_set_from_pixbuf(GTK_IMAGE(imimage), buf);
	gtk_box_pack_start (GTK_BOX (vbox_main), imimage, TRUE, TRUE, 0);
	g_object_unref(buf);

	ybutton=get_pixmapped_button(_("Close"), GTK_STOCK_CLOSE);
  	set_tooltip(ybutton,_("Close"));
	gtk_object_set_data(GTK_OBJECT(ybutton), "mywindow", tuxwin);
	gtk_box_pack_start(GTK_BOX(vbox_main), ybutton, FALSE, FALSE, 5);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_fullview_close),
                      NULL);

  gtk_widget_show_all(tuxwin);
		}
}

 


void on_yphoto_save(GtkButton *widget, gpointer user_data) {
	/* save pic  */ 
	char yfile[32];
	char bud[8];
	int pbs=0;
	char *ptr=NULL;
	GdkPixbuf *buf=NULL;	
	struct yphoto_item *f = NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}

	snprintf(bud, 6, "\"%d\"", ya->position);
	f=yphoto_item_find(ya->yphotos, bud);
	if (! f) {return ;}
	buf=get_yphoto_item_pixbuf(f->url);
	if (! buf) {return; }

	ptr=strrchr(f->url,'/');
	if (ptr) {
		ptr++;
		snprintf(yfile, 25, "GyachE-%s", ptr);
		ptr=strchr(yfile,'.');
		if (ptr) {*ptr='\0';}
		ptr=strchr(yfile,'?');
		if (ptr) {*ptr='\0';}
	} else {snprintf(yfile, 24, "%s-000-%d", "GyachE-", (f->position>-1)?f->position:0 );}
	strcat(yfile,".png");
	snprintf(yphoto_urls, 254, "%s/%s", GYACH_CFG_DIR, yfile);
	snprintf(bud, 5, "%s", "png");
	pbs = gdk_pixbuf_save(buf,yphoto_urls,bud,NULL,NULL);
	snprintf(yphoto_infos, 512, "%s:\n%s", pbs?_("File successfully saved."):_("File could not be saved."), yphoto_urls);
	show_ok_dialog(yphoto_infos);
	g_object_unref(buf);
}

void on_yphoto_session_close(GtkButton *widget, gpointer user_data) {
	/* close session */ 
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (ya->pm_hbox ) {
		if (ya->private_session) {
			GtkWidget *closer=NULL;
			closer=gtk_object_get_data(GTK_OBJECT(ya->pm_hbox), "window");
			if (closer) {gtk_widget_destroy(closer);}
			reset_yphoto_album(ya);
			return; 
		} 
	}
	remove_yphoto_album(ya->who);
}

void on_yphoto_session_select(GtkButton *widget, gpointer user_data) {
	/* select session */ 
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	yphoto_append_albums(ya);
}


void on_yphoto_slideshow_off(GtkButton *widget, gpointer user_data) {
	GtkWidget *button=NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {	 return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}
	if (ya->slideshow==0) {return;}
	button=gtk_object_get_data(GTK_OBJECT(ya->panel), "slide" );
	if (button) {gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NORMAL);}
	if ((ya->slideshow != -1) && (! ya->private_session)  ) {
		snprintf(yphoto_infos, 3 ,"%s", "0" );
		yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 9);
	}
	ya->slideshow=0;
}

void on_yphoto_move_to(GtkButton *widget, gpointer user_data, int pforward) {
	char bud[8];
	struct yphoto_item *f = NULL;
	GtkWidget *imimage=NULL;
	GtkWidget *button=NULL;
	GtkWidget *label=NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}
	imimage=gtk_object_get_data(GTK_OBJECT(ya->panel), "image" );
	if (! imimage) {return;}
	label=gtk_object_get_data(GTK_OBJECT(ya->panel), "label" );
	if (! label) {return;}
	button=gtk_object_get_data(GTK_OBJECT(ya->panel), "button" );
	if (! button) {return;}
	gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);
	if (pforward>0) {ya->position++;}
	else {ya->position--;}
	if ( (ya->position > g_hash_table_size( ya->yphotos )) || (ya->position< -1)  ) {
		ya->position=-1;
		gtk_image_set_from_stock(GTK_IMAGE(imimage), GTK_STOCK_OPEN, GTK_ICON_SIZE_DIALOG);
		gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NORMAL);
		set_tooltip(button,_("Select a photo album"));
		return;
	}
	snprintf(bud, 6, "\"%d\"", ya->position);
	f=yphoto_item_jumpto(ya->yphotos, bud, imimage, label);

	if ((ya->they_invite==1) || (ya->they_invite==2) ) {
		if ( (ya->slideshow < 1) && (! ya->private_session) && (pforward != 2)) {
		yphoto_send_outgoing(ya->pms, ya->pms->pm_user, bud, 8);
		}
	}

	if (f) {
		snprintf(yphoto_infos, 600, "[%s]\n\"%s\"\n%d x %d\n(%d/%d)\n\n%s", ya->title, f->caption,  f->width, f->height, f->position+1, g_hash_table_size( ya->yphotos ), f->url );
		set_tooltip(button,_utf(yphoto_infos));
	}
}

int yphoto_run_animation(struct yphoto_album *ya) {
	if (! ya) {return 0;}
	if (ya->slideshow !=  1) {return 0;}
	on_yphoto_move_to(NULL, ya, 1);
	if (ya->slideshow==1) {return 1;}
	return 0;
}

void on_yphoto_slideshow_on(GtkButton *widget, gpointer user_data) {
	GtkWidget *button=NULL;
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window); }
	if (! ya->loaded) {	 return;}
	if (ya->slideshow==1) { return;}
	button=gtk_object_get_data(GTK_OBJECT(ya->panel), "slide" );
	if (button) {gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);}

	if ((ya->they_invite==1) || (ya->they_invite==2) ) {
	if ( (ya->slideshow != 2) && (! ya->private_session) ) {
		snprintf(yphoto_infos, 3 ,"%s", "1" );
		yphoto_send_outgoing(ya->pms, ya->pms->pm_user, yphoto_infos, 9);
		}
	}
	ya->slideshow=1;
	g_timeout_add( 5500, (void *)yphoto_run_animation, ya );
}

void on_yphoto_forward(GtkButton *widget, gpointer user_data) {
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}
	if (ya->slideshow) { return;}
	on_yphoto_move_to(widget, user_data, 1);
}
void on_yphoto_back(GtkButton *widget, gpointer user_data) {
	struct yphoto_album *ya=NULL;
	ya=user_data;
	if (! ya) {return;}
	if (ya->pms) { focus_pm_entry(ya->pms->pm_window);}
	if (! ya->loaded) {	 return;}
	if (ya->slideshow) { return;}
	on_yphoto_move_to(widget, user_data, 0);
}



GtkWidget *append_yphoto_panel(struct yphoto_album *ya) {
	GtkWidget *yframe, *buttbox, *ycaption, *ybutton, *yimage, *ybox, *topbox;
	ya->panel=gtk_vbox_new(FALSE, 1);
	gtk_box_set_spacing (GTK_BOX (ya->panel), 3);
	gtk_container_set_border_width(GTK_CONTAINER(ya->panel),4);

	topbox=gtk_hbox_new(FALSE, 1);
	gtk_box_set_spacing (GTK_BOX (topbox), 3);
	gtk_container_set_border_width(GTK_CONTAINER(topbox),2);
	gtk_box_pack_start(GTK_BOX(ya->panel), topbox, FALSE, FALSE, 0);

	ybutton=get_pixmapped_button(_("Y! Photos"), GTK_STOCK_SELECT_COLOR);
	 set_tooltip(ybutton,_("Yahoo Photos website"));
	gtk_box_pack_start(GTK_BOX(topbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC ( on_yphoto_goto),
                      NULL);

	ybutton=get_pixmapped_button(_("Close"), GTK_STOCK_CLOSE);
  	set_tooltip(ybutton,_("Close"));
	gtk_box_pack_start(GTK_BOX(topbox), ybutton, TRUE, TRUE, 2);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_session_close),
                      ya);
	
	yframe=gtk_frame_new(" ");
	gtk_box_pack_start(GTK_BOX(ya->panel), yframe, TRUE, TRUE, 1);
	gtk_object_set_data(GTK_OBJECT(ya->panel), "frame",yframe );
	ybox=gtk_vbox_new(FALSE, 1);
	gtk_box_set_spacing (GTK_BOX (ybox), 2);
	gtk_container_set_border_width(GTK_CONTAINER(ybox),2);
	gtk_container_add(GTK_CONTAINER(yframe),ybox);

	ybutton=gtk_button_new();
	yimage=gtk_image_new();
	gtk_image_set_from_stock(GTK_IMAGE(yimage), GTK_STOCK_OPEN, GTK_ICON_SIZE_DND);
	gtk_container_add(GTK_CONTAINER(ybutton),yimage);
	gtk_box_pack_start(GTK_BOX(ybox), ybutton, TRUE, TRUE, 1);
	gtk_object_set_data(GTK_OBJECT(ya->panel), "image",yimage );
	gtk_object_set_data(GTK_OBJECT(ya->panel), "button",ybutton );
	set_tooltip(ybutton,_("Select a photo album"));
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_session_select),
                      ya);

	ycaption=gtk_label_new(" ");
	gtk_box_pack_start(GTK_BOX(ybox), ycaption, FALSE, FALSE, 1);
	gtk_object_set_data(GTK_OBJECT(ya->panel), "label",ycaption );

	buttbox=gtk_hbox_new(TRUE, 1);
	gtk_box_set_spacing (GTK_BOX (buttbox), 2);
	gtk_container_set_border_width(GTK_CONTAINER(buttbox),2);
	gtk_box_pack_start(GTK_BOX(ybox), buttbox, FALSE, FALSE, 1);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("Start slideshow"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
	gtk_object_set_data(GTK_OBJECT(ya->panel), "slide",ybutton );
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_slideshow_on),
                      ya);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("Stop slideshow"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_slideshow_off),
                      ya);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_ZOOM_IN, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("View large-sized image"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_full_view),
                      ya);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("Save"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_save),
                      ya);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("Previous picture"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_back),
                      ya);

	ybutton=gtk_button_new();
	gtk_container_add(GTK_CONTAINER(ybutton),GTK_WIDGET(
		gtk_image_new_from_stock(GTK_STOCK_GO_FORWARD, GTK_ICON_SIZE_BUTTON) ) );
	 set_tooltip(ybutton,_("Next picture"));
	gtk_box_pack_start(GTK_BOX(buttbox), ybutton, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (ybutton), "clicked",
                      GTK_SIGNAL_FUNC (on_yphoto_forward),
                      ya);

	gtk_widget_show_all(ya->panel);
	gtk_box_pack_start(GTK_BOX(ya->pm_hbox), ya->panel, FALSE, FALSE, 2);
	return ya->panel;
}




void yphoto_send_outgoing(PM_SESSION *pms, char *who, char *msg14, int ptype13) {
	YPH_DEBUG=ymsg_sess->debug_packets;
	set_current_pm_profile_name(retrieve_profname_str(GTK_WIDGET(retrieve_profname_entry(pms->pm_window)))  );
	ymsg_photo_imvironment( ymsg_sess , who,msg14, ptype13);
	reset_current_pm_profile_name();
}

void toggle_yphoto_tuxvironment(PM_SESSION *pms) {
	char *current_tux=NULL;
	current_tux=gtk_object_get_data (GTK_OBJECT (pms->pm_window), "imv");
	if (current_tux) {
		if (! strcmp(current_tux, "photos;20")) {return;}
		free(current_tux);
	}
	gtk_object_set_data (GTK_OBJECT (pms->pm_window), "imv",strdup("photos;20"));
	set_tuxvironment (pms->pm_text, pms->pm_window, (char *)gtk_object_get_data (GTK_OBJECT (pms->pm_window), "imv"));
}

void on_yphoto_session_start(PM_SESSION *pms) {
	struct yphoto_album *ya=NULL;

	YPH_DEBUG=ymsg_sess->debug_packets;

	if (enable_basic_protection &&
	 (! find_temporary_friend(pms->pm_user)) && (! find_friend(pms->pm_user) ) ) {
		snprintf(yphoto_infos, 600, "%s  '%s' %s  %s %s\n",
			YAHOO_COLOR_RED, pms->pm_user, 
			_("has attempted to share a Yahoo photo album."), 
			_("For security reasons, sharing a Yahoo photo album is restricted to users who are friends or temporary friends."), 
			YAHOO_COLOR_BLACK);
		append_to_open_pms(pms->pm_user,yphoto_infos , 1);
		snprintf(yphoto_infos, 3 ,"%s", "1" );
		yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
		deactivate_tuxvironment(pms);
		return; 
	}

	if (pms->sms) {return;}
	ya=yphoto_album_find(pms->pm_user);
	if (ya) {return; }
	if (! ya) {ya=yphoto_album_new(pms->pm_user);}
	if (ya) {
		ya->pms=pms;
		toggle_yphoto_tuxvironment(pms);
		ya->pm_hbox=gtk_object_get_data(GTK_OBJECT(pms->pm_window), "hbox_main");
		if (! ya->pm_hbox) {return; }
		append_yphoto_panel(ya);
		gtk_widget_show_all(pms->pm_window);

				snprintf(yphoto_infos, 15 ,"%s", " " );
				yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 0);
				snprintf(yphoto_infos, 15 ,"%s", "null" );
				yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 5);

	}


}



void yphoto_handle_incoming(char *from, char *msg14, int ptype13) {
	char *ptr=NULL;
	int pic_num=0;
	GList *pm_lpackptr;
	PM_SESSION *pms	=NULL;
	struct yphoto_album *ya=NULL;

	if (( pm_lpackptr = find_pm_session( from )) != NULL ) {
	  pms = pm_lpackptr->data;
	}
	
	if (!pms) {return;}

	YPH_DEBUG=ymsg_sess->debug_packets;

	switch (ptype13) {
		case 0:
		case 7:
		case 5: /* This just echos back and starts the session */ 
			on_yphoto_session_start(pms);
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			set_last_comment( ya->who, "YPhoto IMV" );
			if (ptype13 != 0) {
				if ((ya->i_invite != 1) && (ya->i_invite != 3)  && (ya->bounces<9)  ) {
					snprintf(yphoto_infos, 15 ,"%s", msg14 );
					yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, ptype13);
					ya->bounces++;
				} else {toggle_yphoto_tuxvironment(pms);}
			}
			if (ptype13 ==5) {
				if ( (atoi(msg14)==1)  && (ya->bounces<9)  ) {
					if (ya->they_invite<1)  {ya->they_invite=2;}
					if (ya->i_invite == 1)  {
						snprintf(yphoto_infos, 12 ,"%s", "null" );
						yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
						}	
					if ( (ya->i_invite==1) && (ya->url_key)   ) {
						snprintf(yphoto_infos, 350 ,"\"%s\"", ya->url_key);
						yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 2);
						}
					}
			}
			if (ptype13 ==7) {
				if (ya->slideshow>0) {
					ya->slideshow=-1;
					on_yphoto_slideshow_off(NULL, ya);
				}
			}
			return; 
			break;


		case 9: /* Slideshow toggled */ 
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			if (ya->i_invite==3) {return;}
			if (ya->i_invite==-1) {return;}
			if (! strcasecmp(msg14, "null")) {return;}
			if (! ya->loaded) {
				snprintf(yphoto_infos, 15 ,"%s", "null" );
				yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
				return;
			}
			if (atoi(msg14)==0) {
				if (ya->slideshow>0) {
					ya->slideshow=-1;
					on_yphoto_slideshow_off(NULL, ya);
				}
			} else {
				if (ya->slideshow<1) {
					ya->slideshow=2;
					on_yphoto_slideshow_on(NULL, ya);
				}
			}
			return; 
			break;

		case 10: /* just ignore this for now, comes after accepting invite */ 
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			break;

		case 3: /*  request URL or rejection  */ 
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			if (ya->i_invite==3) {return;}
			if (ya->i_invite==-1) {return;}
			if (!strcmp(msg14,"1")) { /* invite rejected */
				if (ya->they_invite != 3) {
				snprintf(yphoto_infos, 400, " %s '%s' %s %s\n",  YAHOO_COLOR_RED, 
				pms->pm_user , _("has declined the invitation to view your photo album."), YAHOO_COLOR_BLACK);
				append_to_open_pms(pms->pm_user,yphoto_infos , 1);
				ya->they_invite=3;
				}

			} else { /* URL requested */ 
				if (ya->they_invite<1)  {ya->they_invite=2;}
				if ((ya->i_invite != 1) && (ya->i_invite != 3) ) {
					snprintf(yphoto_infos, 12 ,"%s", "null" );
					yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
				}
				if ( (ya->i_invite==1) && (ya->url_key) ) {
					snprintf(yphoto_infos, 350 ,"\"%s\"", ya->url_key);
					yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 2);
				}
			}
			return; 
			break;

		case 8: /*  Jump to a specific picture */ 
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			if (ya->i_invite==3) {return;}
			if (ya->i_invite==-1) {return;}
			if (! strcasecmp(msg14, "null")) {return;}
			if (! ya->loaded) {
				snprintf(yphoto_infos, 15 ,"%s", "null" );
				yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
				return;
			}
			ptr=strchr(msg14,'"');
			if (ptr) {
				snprintf(yphoto_infos, 12 ,"%s", ptr+1 );
				ptr=strchr(yphoto_infos,'"');
				if (ptr) {*ptr='\0';}
				pic_num=atoi(yphoto_infos)-1;
				ya->position=pic_num;
				if (ya->they_invite<1)  {ya->they_invite=1;}
				on_yphoto_move_to(NULL , ya, 2);	
			}
			return; 
			break;

		case 2: /*  Photo album sent */ 
			on_yphoto_session_start(pms);
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			if (! strcasecmp(msg14, "null")) {return;}
			if (! strstr(msg14, "http:")) {return;}
			ptr=strchr(msg14,'"');
			if (ptr) {
				GtkWidget *okbutton=NULL;
				GtkWidget *cbutton=NULL;
				snprintf(yphoto_infos, 350 ,"%s", ptr+1 );
				ptr=strchr(yphoto_infos,'"');
				if (ptr) {*ptr='\0';}
				ptr=strchr(yphoto_infos,'&');
				if (ptr) {*ptr='\0';}
				if (! strstr(yphoto_infos,"http:")) {return;}
				if (ya->url_key) {
					if (! strcmp(yphoto_infos, ya->url_key)) {return;}
				}
				if (ya->url_key_pending) {
					if (! strcmp(yphoto_infos, ya->url_key_pending)) {return;}
				}
				if (ya->url_key_pending) {
					g_free(ya->url_key_pending); ya->url_key_pending=NULL;
				}
				ya->url_key_pending=g_strdup(yphoto_infos);
				ya->i_invite=-1;

				if (YPH_DEBUG) {
				printf("Got invite URL: %s\n", ya->url_key_pending); fflush(stdout);
				}

				snprintf(yphoto_infos, 512, "'%s' %s", pms->pm_user, _("has invited you to view a photo album. Would you like to accept the invitation?"));
				okbutton=show_confirm_dialog_config(yphoto_infos,"Yes","No",0);
				if (!okbutton) {
					snprintf(yphoto_infos, 3 ,"%s", "1" );
					yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, 3);
					return;
					}
  				gtk_signal_connect (GTK_OBJECT (okbutton), "clicked",
                      	GTK_SIGNAL_FUNC (on_yphoto_accept_invite), ya);
				cbutton=gtk_object_get_data( GTK_OBJECT(okbutton), "cancel" );
				if (cbutton) {
  					gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      		GTK_SIGNAL_FUNC (on_yphoto_reject_invite), ya);
						 }

			}

			return; 
			break;

		case 4: /* accepted invite */ 
			ya=yphoto_album_find(pms->pm_user);
			if (! ya) {return;}
			ya->bounces=0;
			set_last_comment( ya->who, "YPhoto IMV" );
			if (ya->i_invite==3) {return;}
			snprintf(yphoto_infos, 12 ,"%s", ya->i_invite?"0,1":" ");
			yphoto_send_outgoing(pms, pms->pm_user, yphoto_infos, ya->i_invite?10:0);
			if (! ya->i_invite) {return; }
			if (ya->they_invite != 2 ) {
				snprintf(yphoto_infos, 400, " %s '%s' %s %s\n",  YAHOO_COLOR_GREEN, 
				pms->pm_user , _("has accepted the invitation to view a photo album."), YAHOO_COLOR_BLACK);
				append_to_open_pms(pms->pm_user,yphoto_infos , 1);
			}
			ya->position--;
			if ( (ya->position > g_hash_table_size( ya->yphotos )) || (ya->position< -1)  ) {
				ya->position=-1;
			}
			on_yphoto_move_to(NULL , ya, 1);	
			ya->they_invite=2;		
			return; 
			break;
	}
	
}

void on_yphoto_private_album(GtkMenuItem *widget, gpointer user_data) {
	char bud[40];
	struct yphoto_album *ya=NULL;
	snprintf(bud, 38, "%s", "[Gyach-E*PRIVATE45]");
	ya=yphoto_album_find(bud);
	if (! ya) {ya=yphoto_album_new(bud);}
	if (ya) {
	GtkWidget *tuxwin, *vbox_main;
	ya->private_session=1;
	ya->pms=NULL;
  tuxwin = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_wmclass (GTK_WINDOW (tuxwin), "gyachEDialog", "GyachE"); 
  gtk_window_set_title (GTK_WINDOW (tuxwin), _("Y! Photos") );
  gtk_window_set_position (GTK_WINDOW (tuxwin), GTK_WIN_POS_CENTER);
  vbox_main = gtk_vbox_new (FALSE, 0);
    gtk_container_set_border_width (GTK_CONTAINER (vbox_main), 3);
  gtk_container_add (GTK_CONTAINER (tuxwin), vbox_main);
  ya->pm_hbox=vbox_main;
  gtk_object_set_data(GTK_OBJECT(ya->pm_hbox), "window",tuxwin );
  append_yphoto_panel(ya);
  gtk_widget_show_all(tuxwin);
		}
}



int my_init() {
 	GtkWidget *mystatmenu;

				if (YPH_DEBUG) {printf("photo-init-0\n"); fflush(stdout);}

	if (! yphoto_cache) {yphoto_cache = g_hash_table_new(g_str_hash, g_str_equal);}
	if (! yphoto_albums) {yphoto_albums = g_hash_table_new(g_str_hash, g_str_equal);}
	if (! yphoto_cache) {return 0;}
	if (! yphoto_albums) {return 0;}

				if (YPH_DEBUG) {printf("photo-init-1\n"); fflush(stdout);}

	mystatmenu=gtk_object_get_data (GTK_OBJECT (chat_window), "tools_menu");
	if (mystatmenu)  {
		GtkWidget *yphotoeparator;
		GtkWidget *yphoto_status_check_button;

				if (YPH_DEBUG) {printf("photo-init-2\n"); fflush(stdout);}

  		yphotoeparator = gtk_menu_item_new ();
  		gtk_container_add (GTK_CONTAINER (mystatmenu), yphotoeparator);
  		gtk_widget_show (yphotoeparator);
  		gtk_widget_set_sensitive (yphotoeparator, FALSE);

				if (YPH_DEBUG) {printf("photo-init-3\n"); fflush(stdout);}

		yphoto_status_check_button=gtk_menu_item_new_with_label (
		_(" Browse My Photo Albums... "));
  		gtk_container_add (GTK_CONTAINER (mystatmenu), yphoto_status_check_button);
  		gtk_widget_show (yphoto_status_check_button);

				if (YPH_DEBUG) {printf("photo-init-4\n"); fflush(stdout);}

  gtk_signal_connect (GTK_OBJECT (yphoto_status_check_button), "activate",
                      GTK_SIGNAL_FUNC (on_yphoto_private_album),
                      NULL);

				if (YPH_DEBUG) {printf("photo-init-5\n"); fflush(stdout);}
		} 

#ifndef _YAHOO_PHOTOS_PLUGIN_
#define _YAHOO_PHOTOS_PLUGIN_ 1
#endif 

	return 1;
}


PLUGIN_INFO plugin_info = {
	PLUGIN_OTHER, 
	"GyachE-Photos",
	"A plugin to access the Yahoo Photos IMV and share photo albums with friends in PM windows. There is a KNOWN problem sharing photos with Windows users.", 
	"0.2-alpha", 
	"9/02/2004",
	"No special requirements.",
	"Erica Andrews [PhrozenSmoke ('at') yahoo.com]", my_init};




