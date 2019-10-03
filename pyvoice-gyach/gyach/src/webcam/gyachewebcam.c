/* Socket-reading and image
   conversion module for the GyachE-Webcam viewer program */

/* This program borrows alot of code from both Ayttm and 
    Gyach-E itself, however, this program is a completely 
    restructured webcam viewer that handles sockets and threads
    in the Gyach-E way, not the Ayttm way of many threads, many 
    structs, etc.  It is designed for simplicity, speed, 
    memory-friendliness, and stability: It runs as an EXTERNAL 
    program to Gyach Enhanced, so that if it DOES crash, it 
    crashes ALONE, rather than taking down an entire chat program
    with it. It is a clean, efficient SINGLE-THREADED application */

/*****************************************************************************
 * gyachewebcam.c
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
 * Copyright (C) 2003-2006 Erica Andrews (Phrozensmoke ['a'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *****************************************************************************/
/* 
	Ayttm is  Copyright (C) 2003, the Ayttm team
	and Copyright (C) 1999-2002, Torrey Searle <tsearle@uci.edu>
*/

# include "gyachewebcam.h"
# include <jasper/jasper.h>

#include "../pixmaps/gyach-icon.xpm"

char **pixmap_gyach_icon = gyach_icon_xpm;

char *CAM_VERSION="0.2";
char *webcam_description="OV511+ WebCam";
char *local_address=NULL;
char *whoami = NULL;
char *webcam_to_view = NULL;
char *webcam_key = NULL;
char *second_server=NULL;
static char *gyache_err=NULL;

char packet_data[10242];
unsigned int packet_type=0;
unsigned long packet_size=0;
unsigned int packet_reason=0;
unsigned int packet_timestamp=0;
int camsocket=-1;
int webcam_connected=0;
int webcam_connecting=0;
int image_need_update=0;
int video_paused=0;
GdkPixbuf *current_pixbuf=NULL;
GtkWidget *current_image=NULL;
GtkWidget *dialogwin=NULL;
GtkWidget *vpause=NULL;
GtkWidget *vstatuslabel=NULL;
GtkWidget *uipstatuslabel=NULL;
char cammsg[255]="";
char user_ip[17]="";
char user_extern_ip[17]="";

int app_debugger=0;

int jasper_started=0;

/* I've never seen Yahoo send a JP2 image over 5kb, but just to be safe ... */
int MAX_PACKET_SIZE=10240;

void set_dummy_image() {
	gtk_image_set_from_stock(GTK_IMAGE(current_image), GTK_STOCK_STOP, GTK_ICON_SIZE_DIALOG);
}
void set_dummy_image_start() {
	gtk_image_set_from_stock(GTK_IMAGE(current_image), GTK_STOCK_CONVERT, GTK_ICON_SIZE_DIALOG);
}
void set_dummy_image_start2() {
	gtk_image_set_from_stock(GTK_IMAGE(current_image), GTK_STOCK_REFRESH, GTK_ICON_SIZE_DIALOG);
}

void update_conn_status() {
	if (webcam_connected) 
		{
			if (webcam_connecting)  {
				gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Requesting Permission...") ); 
													}
			else {gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Connected") ); }
			}
	else
		{gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Not Connected") );}
}

void shutdown_sockets() {
	if (camsocket != -1)  {close(camsocket); camsocket=-1;}
	camsocket=-1;
	webcam_connected=0;
	webcam_connecting=0;
	set_dummy_image();
	update_conn_status();
	gtk_label_set_text(GTK_LABEL(uipstatuslabel), " ");
	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(vpause), 0);
}

void mem_cleanup() {
	packet_type=0;
	packet_size=0;
	packet_reason=0;
	packet_timestamp=0;
	video_paused=0;
	shutdown_sockets();
	if (gyache_err) {free(gyache_err); }
	gyache_err=strdup(_("Unknown Error"));
	if (second_server) {free(second_server); second_server=NULL;}
}

void show_webcam_packet() {
	if ( ! webcam_recv_data( ))  {return ; }
	
	if (packet_type==0x00)  {  /* status packets */
		if (app_debugger) {printf("packet timestamp: %d\n", packet_timestamp); fflush(stdout);}

		if (packet_timestamp==0)  {
				show_ok_dialog(_("The user has denied you viewing permission."));
				shutdown_sockets();
				return ;
													}
										}

	if (packet_type==0x02)  {  /* Image data */
		image_need_update=1;
		if (webcam_connecting)  { /* got our first image, connection started */
			webcam_connecting=0;
			update_conn_status();
												}
										}

	if (packet_type==0x07)  { /*  connection closing */
		if (packet_reason==0x01)  {
			show_ok_dialog(_("The user has stopped broadcasting."));
			shutdown_sockets();
			return ;
													}
		if (packet_reason==0x0F)  {
			show_ok_dialog(_("The user has cancelled viewing permission."));
			shutdown_sockets();
			return ;
													}
		/* all other unknown cases */
		show_ok_dialog(_("The webcam connection has been closed."));
		shutdown_sockets();
		return;
										}

		/* grab the IP address of the cam we are viewing */
		if ( (packet_type==0x13) && (packet_size>0) )  {
							char *tttmp, *uip1, *uip2;							
								if (app_debugger) {printf("gathering IP-start\n"); fflush(stdout);}
							uip1=strstr(packet_data,"i=");
							uip2=strstr(packet_data,"j=");
							if (uip1 && uip2) {
								char uipstat[41]="";
								tttmp = strchr(uip1+2, 0x0d);  /* return-char */
								if (tttmp)  {*tttmp = '\0';}
								user_ip[snprintf(user_ip, 15, "%s", uip1+2)]='\0';
								tttmp = strchr(uip2+2, 0x0d);  /* return-char */
								if (tttmp)  {*tttmp = '\0';}
								user_extern_ip[snprintf(user_extern_ip, 15, "%s", uip2+2)]='\0';
									if (app_debugger) {printf("gathering IP-done\n"); fflush(stdout);}
								snprintf(uipstat, 40 , "[%s / %s]", user_ip, user_extern_ip);
								gtk_label_set_text(GTK_LABEL(uipstatuslabel), uipstat);
														}
							return;
													  									}

}


void get_next_server() {
	u_char srequest[12]="";
	u_char wrequest[256]="";
	int wslen=0;
	u_char *wreqptr = webcam_header( srequest,  -1);
	u_char *camreqptr = webcam_header( wrequest, 1);

	wslen=snprintf(wreqptr, 10, "%s", "<RVWCFG>");

	if (! webcam_sock_send( srequest,  wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																		}

	wslen=snprintf(camreqptr , 255-9, "g=%s\r\n", webcam_to_view);
	wslen +=8;

	if (! webcam_send_packet( wrequest, wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																			}

	if ( ! webcam_recv_data( ))  {return ;  }

	if (!second_server)  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
								}
	if (app_debugger) {printf("next-server: %s  \n", second_server); fflush(stdout);}
}


void connect_next_server() {
	u_char srequest[12]="";
	u_char wrequest[1024]="";
	int wslen=0;
	u_char *wreqptr = webcam_header( srequest,  -1);
	u_char *camreqptr = webcam_header( wrequest, 1);

	wslen=snprintf(wreqptr, 10, "%s", "<REQIMG>");
	if (! webcam_sock_send( srequest,  wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																		}

	wslen=snprintf(camreqptr ,1023-9, 
	"a=2\r\nc=us\r\ne=21\r\nu=%s\r\nt=%s\r\ni=%s\r\ng=%s\r\no=w-2-5-1\r\np=%d\r\n",
	whoami, webcam_key, local_address, webcam_to_view, YCAM_DIALUP
	);

	wslen +=8;

	if (! webcam_send_packet( wrequest, wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																			}
	webcam_connected=1;	
	update_conn_status();
	set_dummy_image_start2();
}


void start_connection()  {
	mem_cleanup();  /* reset everything */
	webcam_connecting=1;
	set_local_addresses();
	if (!local_address) {return;}
	if (!whoami) {return;}
	if (!webcam_to_view) {return;}
	if (!webcam_key) {return;}

	camsocket=create_webcam_socket( YAHOO_WEBCAM_HOST, YAHOO_WEBCAM_PORT) ;
	if (camsocket<0)  {shutdown_sockets(); return;}

	get_next_server();
	if (camsocket != -1)  {close(camsocket); camsocket=-1;}
	if (!second_server) {shutdown_sockets(); return;}

	camsocket=create_webcam_socket(second_server, YAHOO_WEBCAM_PORT) ;
	if (camsocket<0)  {shutdown_sockets(); return;}

	connect_next_server();
}

void set_local_addresses()
{
	char command[] = "/sbin/ifconfig `netstat -nr | grep '^0\\.0' | tr -s ' ' ' ' | cut -f 8 -d' '` | grep inet | tr -s ' ' ':' | cut -f4 -d:";
	char buff[1024];
	char addresses[1024];
	struct hostent * hn;
	FILE * f = NULL;

	gethostname(buff,sizeof(buff));

	hn = gethostbyname(buff);
	if(hn) {
		char *quad = hn->h_addr_list[0];
		snprintf(addresses, sizeof(addresses), "%d.%d.%d.%d",
				quad[0], quad[1], quad[2], quad[3] );
	} 
	if((!hn || (addresses && !strcmp(addresses,"127.0.0.1"))) 
	&& (f = popen(command, "r")) != NULL ) {
		int i=0;

		do {
			int r = fgetc(f);
			buff[i] = (char)r;
			if(buff[i]=='\r' || buff[i]=='\n' || r==EOF)
				buff[i]='\0';
			else if(i >= sizeof(buff)-1) {
				buff[i]='\0';
				/*return error?*/
			}
		} while(buff[i++]);
		
		pclose(f);
		strncpy(addresses, buff, sizeof(addresses));
	} else {
		addresses[0]=0;
	}
	if (local_address) {free(local_address);}
	local_address= strdup(addresses);
}


int create_webcam_socket( char *webcamhost, int webcamport) 
  {
	int sock;
	struct sockaddr_in sa;
	struct hostent *hinfo;
	int sock_res;

	if (!webcamhost) {return -1;}

	hinfo = gethostbyname( webcamhost);
	if (!  hinfo)     {  /* added: PhrozenSmoke, to avoid core dumps */
		show_ok_dialog(_("Could not connect to webcam host"));
		return -1;
					   }

	memset( &sa, 0, sizeof(sa));
	memmove((void*)&sa.sin_addr.s_addr, hinfo->h_addr, hinfo->h_length );

	sa.sin_family = AF_INET;
	sa.sin_port = htons( webcamport );

	sock = socket( AF_INET, SOCK_STREAM, 6 );
	if (sock==-1) {  /* added: PhrozenSmoke, to avoid core dumps */
		show_ok_dialog(_("Could not connect to webcam host"));
		return -1;
					   }

	sock_res=connect( sock, (struct sockaddr*)&sa, sizeof(sa));

	if (sock_res==-1) {  /* added: PhrozenSmoke, to avoid core dumps */
		show_ok_dialog(_("Could not connect to webcam host"));
		close(sock);
		return -1;
							   }

	return sock;
  }

int webcam_sock_send( u_char *buf, int size ) {
	int sent;
	char  gerrmsg[256]="";
	char *ptr = buf;

	if (camsocket<0) {return 0;}

	while( size ) {
		sent = send( camsocket, ptr, size, 0 );
		if ( sent < 1 ) {
			sprintf( gerrmsg, "Error sending packet to socket: %d\n",
				errno );
			if (gyache_err) {free(gyache_err); }
			gyache_err=strdup(gerrmsg);
			break;
		}
		ptr += sent;
		size -= sent;
	}
	return( size == 0 );
}

int webcam_send_packet(  u_char *bufp, int size ) {
	/* set data length in packet */
	bufp[7]=size - 8;

	// *((u_short *)(bufp+7)) = size - 8 ;

	/* don't try to send if we're not connected */
	if ( camsocket == -1 ) {return( 0 );}
	/* send the packet */
	return webcam_sock_send( bufp, size );
}

int webcam_sock_recv( u_char *buf, int size ) {
	int recvd;
	char  gerrmsg[256]="";
	char *ptr = buf;

	if (camsocket<0) {return 0;}

	while( size ) {
		recvd = recv(camsocket, ptr, size, 0 );
		if ( recvd < 1 ) {
			sprintf( gerrmsg, "Error receiving packet from socket: "
				"%d\n", errno );
			if (gyache_err) {free(gyache_err); }
			gyache_err=strdup(gerrmsg);
			break;
		}
		ptr += recvd;
		size -= recvd;
	}
	return( size == 0 );
}

int webcam_recv_data( ) {
	char wtsize[3]="";
	u_char buf[MAX_PACKET_SIZE+1];
	unsigned int header_len=13;

	packet_type=0;
	packet_size=0;
	packet_reason=0;
	packet_timestamp=0;

	/* receive first byte, its our header length */
	if ( ! webcam_sock_recv(  buf, 1 )) {	return( 0 );}

	/* 1st byte */
	header_len=buf[0];

	/* get rest of the header */
	if ( ! webcam_sock_recv(  buf, header_len-1 )) {return( 0 );}

	if (header_len<8) {return( 0 );} 	/* Something strange going on */

	/* 2nd byte */
	packet_reason = buf[0];

	/* grab packet size, necessary if this is an image */
	wtsize[0]=buf[5];
	wtsize[1]=buf[6];
	wtsize[2]='\0';
	packet_size=  ((((*(wtsize))&0xff)<<8) + ((*((wtsize)+1)) & 0xff));

		if (app_debugger) { printf("im: %d %d\n", wtsize[2] , wtsize[3]); fflush(stdout); }


	if (header_len>12) {
		/* byte 9 */
		packet_type = buf[7];

		/* bytes 10-13 */  /* timestamp not really accurate or important in most cases */
		packet_timestamp = ntohl((u_long)*((u_long *)(buf+8)));
								}

	/*  
	printf("Packet:  header-%d  type-%d  reason-%d  size-%d   time-%d\n", header_len, packet_type, packet_reason, packet_size, packet_timestamp);
	fflush(stdout); 
	*/


	/* grab the forwarding server if it's available and needed  */
	/* old */     /* if ( (header_len==0x17) && (!second_server) )  { */

	if ( (header_len>13) && (!second_server) )  {
		if (packet_reason==0)  {
			char webcsrv[20]="";
			buf[header_len] = '\0';
			memcpy( webcsrv, buf + 3, 16 );
			webcsrv[16]='\0';
			second_server=strdup(webcsrv);
											  } else {  /* status of 0x06 or other */
				char  gerrmsg[256]="";
				sprintf(gerrmsg, "%s", _("The user does not have their webcam online."));
				if (gyache_err) {free(gyache_err); }
				gyache_err=strdup(gerrmsg);
														}
				return( 1 );
	}


		/* without image data, it usually just a header, so skip the rest -disabled */ /* 
	if (packet_type != 0x02)  {		
		return( 1 );
	}  */

	/* avoid buffer overflows and weird behavior ... */
	if (packet_size<1) {return( 1 );}   /* packet with data length=0 ? */
	if (packet_size>MAX_PACKET_SIZE)  {packet_size=MAX_PACKET_SIZE;}

	if ( ! webcam_sock_recv(buf + header_len, packet_size )) {return( 0 );}
	buf[packet_size + header_len] = '\0';
	memcpy( packet_data, buf + header_len, packet_size );
	packet_data[packet_size ] = '\0';
	return( 1 );
}


u_char *webcam_header(u_char *buf, int packtype) {
	unsigned char magic_nr[] = {8, 0, 1, 0};
	u_char *ptr = buf;

	/* for headerless packets, just create the pointer */
	if (packtype==-1) {return( ptr );}

	memcpy(buf, magic_nr, sizeof(magic_nr));
	ptr += 4;
	*((u_long *)ptr) =	htonl( 0x00000000 );	 /* packet length, to be filled in later */
	ptr += 4; 
	return( ptr );
}


/*  Image Updating Stuff */

unsigned char * image_2_jpg(char *in_img, int size)
{
	char *out_img = NULL;
	jas_stream_t *in, *out;
	jas_image_t *image;
	int infmt;
	int outfmt;

		if (app_debugger) { printf("Jasper-1\n");  fflush(stdout); }

	if (! jasper_started) {
	if(jas_init()) {
		if (app_debugger) {printf("Could not init jasper\n");}
		return NULL;
	}
		jasper_started=1;
							  }

		if (app_debugger) {printf("Jasper-2\n");  fflush(stdout);}

		outfmt = jas_image_strtofmt("jpg");

		if (app_debugger) {printf("Jasper-3\n");  fflush(stdout);}

	if(!(in = jas_stream_memopen((unsigned char *)in_img, size))) {
		if (app_debugger) {printf("Could not open jasper input stream\n");}
		return NULL;
	}

		if (app_debugger) {printf("Jasper-4\n");  fflush(stdout);}

	infmt = jas_image_getfmt(in);

	if (app_debugger) {
		printf( "Got input image format: %d %s\n", infmt, jas_image_fmttostr(infmt));
		fflush(stdout);
							  }

	if(infmt <= 0) {return NULL; }

		if (app_debugger) {printf("Jasper-5\n");  fflush(stdout);}

	if(!strcmp(jas_image_fmttostr(infmt), "jpg")) {
		/* image is already jpeg */
		jas_stream_close(in);
		return NULL;
	}

		if (app_debugger) {printf("Jasper-6\n");  fflush(stdout);}

	if(!(image = jas_image_decode(in, infmt, NULL))) {
		if (app_debugger) {printf( "Could not decode image format\n");}
		return NULL;
	}

		if (app_debugger) {printf("Jasper-7\n");  fflush(stdout);}

	if(!(out = jas_stream_memopen(out_img, 0))) {
		if (app_debugger) {printf( "Could not open output stream\n");}
		return NULL;
	}

	if (app_debugger) {printf( "Encoding to format: %d %s\n", outfmt, "jpg");}

	if((jas_image_encode(image, out, outfmt, NULL))) {
		if (app_debugger) {printf("Could not encode image format\n");}
		return NULL;
	}

		if (app_debugger) {printf("Jasper-8\n");  fflush(stdout);}

	jas_stream_flush(out);

	size = ((jas_stream_memobj_t *)out->obj_)->bufsize_;
	if (app_debugger) {printf( "Encoded size is: %d\n", size);}
	jas_stream_close(in);
	out_img=malloc(size+1);
	memcpy(out_img, ((jas_stream_memobj_t *)out->obj_)->buf_, size);
	packet_size=size;

	if (app_debugger) {printf("Jasper-9\n");  fflush(stdout);}

	jas_stream_close(out);
	jas_image_destroy(image);

	/* jas_image_clearfmts(); */ /* this is bad */

	if (app_debugger) {printf("Jasper-10\n");  fflush(stdout);}

	return out_img;	
}


void update_webcam_image ()
{
	char *jpg_buf;
	GdkPixbufLoader *pixloader;

	if (packet_size<0) {return;}

	if (app_debugger) {printf("Jasper conversion...\n"); fflush(stdout);}

	jpg_buf =image_2_jpg(packet_data, packet_size);

	if (app_debugger) {printf("Jasper conversion-done\n"); fflush(stdout);}

	if (! jpg_buf) {
		if (app_debugger) {printf("No JPeg!\n");  fflush(stdout);   }
		return ;}

	pixloader = gdk_pixbuf_loader_new();
	gdk_pixbuf_loader_write(pixloader, jpg_buf, packet_size, NULL);
	gdk_pixbuf_loader_close(pixloader, NULL);

	/* if (app_debugger) {printf("Pixbuf size:  %d \n", packet_size );  fflush(stdout); } */ 

	free(jpg_buf);

	current_pixbuf = gdk_pixbuf_loader_get_pixbuf(pixloader);
	if (!current_pixbuf) {
		if (app_debugger) {printf("No Pixbuf !\n" );  fflush(stdout); }
		current_pixbuf=NULL; return;
								}

	gtk_image_set_from_pixbuf(GTK_IMAGE(current_image), current_pixbuf);
	gdk_pixbuf_unref(current_pixbuf);
	current_pixbuf=NULL;
	if (app_debugger) {printf("pixbuf okay\n"); fflush(stdout);}
}


/* GUI stuff */

GtkWidget *get_pixmapped_button(char *button_label, const gchar *stock_id) {
	GtkWidget *buttbox;
	GtkWidget *dbutton;
	GtkWidget *dlab;

	dlab=gtk_label_new(button_label);
  	gtk_label_set_justify (GTK_LABEL (dlab), GTK_JUSTIFY_LEFT);
  	gtk_misc_set_alignment (GTK_MISC (dlab), 0, 0.5);

	dbutton=gtk_button_new();
	buttbox=gtk_hbox_new(FALSE, 2);
  	gtk_container_set_border_width (GTK_CONTAINER (buttbox), 1);
	gtk_box_pack_start (GTK_BOX (buttbox), GTK_WIDGET(gtk_image_new_from_stock(stock_id, GTK_ICON_SIZE_BUTTON) ), FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (buttbox), GTK_WIDGET(dlab), TRUE, TRUE,2);
	gtk_container_add (GTK_CONTAINER (dbutton), buttbox); 
	gtk_widget_show_all(dbutton);
	return dbutton;
}


void on_close_ok_dialogw (GtkWidget *widget, GdkEvent *event, gpointer  user_data)
{
	gtk_widget_destroy(dialogwin);
}


void show_ok_dialog(char *mymsg)  {

	GtkWidget *dwindow;
	GtkWidget *dvbox;
	GtkWidget *dlbox;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;

	GtkStyle *windowstyle;
	GdkPixmap *pm_icon;
	GdkBitmap *mask_icon;

	char *tmpest=NULL;

	tmpest=strdup(mymsg);
	snprintf(cammsg, 253, "Gyach Enhanced  (%s):\n\n%s", _("Webcam Viewer"),  tmpest);

	free(tmpest);

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  dialogwin=dwindow;
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Message"));
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);


  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);  

	dlabel=gtk_label_new("");
	gtk_label_set_text(GTK_LABEL(dlabel),cammsg); /* for gettext support, gchar */
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);

	dbutton=get_pixmapped_button(_("OK"), GTK_STOCK_YES);
 	 gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);

	dlbox = gtk_hbox_new (FALSE, 2);
  	gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);

  	gtk_box_pack_start (GTK_BOX (dlbox), dlabel, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);

	gtk_box_pack_start (GTK_BOX (dvbox), dbutton, FALSE, FALSE, 8);

  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

gtk_widget_show_all(dwindow);

	windowstyle = gtk_widget_get_style( dwindow );
	pm_icon = gdk_pixmap_create_from_xpm_d( dwindow->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( dwindow->window, NULL, pm_icon, mask_icon );

}



void on_about_menu  (GtkMenuItem *menuitem, gpointer  user_data) {
	char abuf[300]="";  
	snprintf(abuf, 298, "Copyright (c) 2004\nErica Andrews\nPhrozenSmoke ['at'] yahoo.com\nLicense: GNU General Public License\nCam Viewer Version: %s", CAM_VERSION);
	show_ok_dialog(abuf);
}


void on_shutdown_app(GtkWidget *widget, GdkEvent *event, gpointer  user_data)
{	
	// gtk_widget_destroy(main_window);
	cam_app_quit=1;
	mem_cleanup();
	exit(0);
}

void on_shutdown_app_cb(GtkMenuItem *menuitem, gpointer  user_data) {
	on_shutdown_app(NULL, NULL, NULL);
}

void on_video_pause_toggled  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(vpause)->active) {
		video_paused=1;
																				} else {video_paused=0;}
}

void on_menu_disconnect(GtkMenuItem *menuitem, gpointer  user_data) {
	if (webcam_connecting) {return;}
	if (webcam_connected) {shutdown_sockets();}
}

void on_menu_connect(GtkMenuItem *menuitem, gpointer  user_data) {
	if (webcam_connecting) {return;}
	if (! webcam_connected) {start_connection() ;}
}

void gyache_image_window_new(int width, int height, const char *title)
{
	GtkWidget *main_window;
	GtkWidget *vbox1;
	GtkWidget *btnClose;
	GtkStyle *windowstyle;
	GdkPixmap *pm_icon;
	GdkBitmap *mask_icon;
	GtkWidget *connection_menu;
	GtkWidget *about;      
	GtkWidget *help_main_menu;
	GtkWidget *help_main;
	GtkWidget *quit;
	GtkWidget *menubar1;
	GtkWidget *connection;
	GtkWidget *vimframe;
	GtkWidget *stopconn;
	GtkWidget *startconn;

	if (!title) {title = _(" View Webcam... ");}

if (app_debugger) {printf("openWindow-1a\n"); fflush(stdout);}

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);

if (app_debugger) {printf("openWindow-1b\n"); fflush(stdout);}

	gtk_window_set_title(GTK_WINDOW(main_window), title);

if (app_debugger) {printf("openWindow-1c\n"); fflush(stdout);}

	gtk_window_set_wmclass (GTK_WINDOW (main_window), "gyachEWebcam", "gyachewebcam");

if (app_debugger) {printf("openWindow-1d\n"); fflush(stdout);}

	vbox1 = gtk_vbox_new(FALSE, 0);
if (app_debugger) {printf("openWindow-1e\n"); fflush(stdout);}
	gtk_container_set_border_width (GTK_CONTAINER (vbox1), 2);
	gtk_box_set_spacing(GTK_BOX (vbox1) , 3);

if (app_debugger) {printf("openWindow-1f-menu\n"); fflush(stdout);}

  menubar1 = gtk_menu_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox1), menubar1, FALSE, FALSE, 0);
  gtk_widget_show (menubar1);

  connection = gtk_menu_item_new_with_label (_("Connection"));
  gtk_container_add (GTK_CONTAINER (menubar1), connection);

  connection_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (connection), connection_menu);

  vpause = gtk_check_menu_item_new_with_label (_("Pause Video"));
  gtk_container_add (GTK_CONTAINER (connection_menu), vpause);

	gtk_signal_connect_object(GTK_OBJECT(vpause), "toggled",
				GTK_SIGNAL_FUNC(on_video_pause_toggled),
				NULL);

  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

  startconn = gtk_image_menu_item_new_with_label (_("Connect"));
  gtk_container_add (GTK_CONTAINER (connection_menu), startconn);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(startconn), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_YES, GTK_ICON_SIZE_MENU) ));

	gtk_signal_connect_object(GTK_OBJECT(startconn), "activate",
				GTK_SIGNAL_FUNC(on_menu_connect),
				NULL);

  stopconn = gtk_image_menu_item_new_with_label (_("Disconnect"));
  gtk_container_add (GTK_CONTAINER (connection_menu), stopconn);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(stopconn), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_NO, GTK_ICON_SIZE_MENU) ));

	gtk_signal_connect_object(GTK_OBJECT(stopconn), "activate",
				GTK_SIGNAL_FUNC(on_menu_disconnect),
				NULL);

  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

  quit = gtk_image_menu_item_new_with_label (_("Quit"));
  gtk_container_add (GTK_CONTAINER (connection_menu), quit);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(quit), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_QUIT, GTK_ICON_SIZE_MENU) ));

	gtk_signal_connect_object(GTK_OBJECT(quit), "activate",
				GTK_SIGNAL_FUNC(on_shutdown_app_cb),
				GTK_OBJECT(main_window));

  help_main = gtk_menu_item_new_with_label (_("Help"));
  gtk_container_add (GTK_CONTAINER (menubar1), help_main);
  gtk_widget_show (help_main);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help_main));

  help_main_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_main), help_main_menu);

  about = gtk_image_menu_item_new_with_label (_("About GYach-E Webcam Viewer..."));
  gtk_container_add (GTK_CONTAINER (help_main_menu), about);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(about), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));

  gtk_signal_connect (GTK_OBJECT (about), "activate",
                      GTK_SIGNAL_FUNC (on_about_menu),
                      NULL);

if (app_debugger) {printf("openWindow-1h\n"); fflush(stdout);}

	current_image = gtk_image_new();
	gtk_widget_set_usize(current_image, width, height);

if (app_debugger) {printf("openWindow-2\n"); fflush(stdout);}

	gtk_widget_show(current_image);

	vimframe=gtk_frame_new(NULL);

	gtk_frame_set_shadow_type(GTK_FRAME(vimframe), GTK_SHADOW_IN);

	gtk_widget_set_usize(current_image, width, height);
	gtk_container_add(GTK_CONTAINER(vimframe), current_image);
	gtk_container_set_border_width(GTK_CONTAINER(vimframe), 7);
	gtk_box_pack_start(GTK_BOX(vbox1), vimframe, TRUE, TRUE, 1);

	if (webcam_to_view) {snprintf(cammsg, 125, "%s: %s", _("Webcam"), webcam_to_view);}
	else {snprintf(cammsg, 125, "%s: %s", _("Webcam"), "?");}

	gtk_box_pack_start(GTK_BOX(vbox1), gtk_label_new(cammsg), FALSE, FALSE, 1);
	vstatuslabel=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox1), vstatuslabel, FALSE, FALSE, 1);

	uipstatuslabel=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox1), uipstatuslabel, FALSE, FALSE, 1);
	update_conn_status();

	btnClose = get_pixmapped_button(_("Close"), GTK_STOCK_QUIT);
	gtk_signal_connect_object(GTK_OBJECT(btnClose), "clicked",
				GTK_SIGNAL_FUNC(on_shutdown_app),
				GTK_OBJECT(main_window));

	gtk_box_pack_start(GTK_BOX(vbox1), btnClose, FALSE, FALSE, 3);
	gtk_container_set_border_width(GTK_CONTAINER(btnClose), 2);

	gtk_signal_connect(GTK_OBJECT(main_window), "delete_event",
				GTK_SIGNAL_FUNC (on_shutdown_app), 
                      NULL);

	gtk_container_add(GTK_CONTAINER(main_window), vbox1);
	set_dummy_image_start();
	gtk_widget_show_all(main_window);

	windowstyle = gtk_widget_get_style( main_window);
	pm_icon = gdk_pixmap_create_from_xpm_d(main_window->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( main_window->window, NULL, pm_icon, mask_icon );

}
