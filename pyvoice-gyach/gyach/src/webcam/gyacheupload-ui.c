/* UI module for the GyachE-Broadcaster program:
   This application is used to send webcam streams to
    Yahoo. Right now this program has only been tested
    with a single Video4Linux device: An OV511 DLink
    C100 USB webcam.  The program uses Video4Linux-1
    for image capture and the libJasper library for Jpeg-2000
    image conversions.  */

/* This program borrows alot of code from both Ayttm and 
    Gyach-E itself, as well as the old 'videodog' program
    for a few decent V4L usage examples.

    It is designed for simplicity, speed, 
    memory-friendliness, and stability: It runs as an EXTERNAL 
    program to Gyach Enhanced, so that if it DOES crash, it 
    crashes ALONE, rather than taking down an entire chat program
    with it. It is a clean, efficient SINGLE-THREADED application 
*/

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
 * Copyright (C) 2003-2006 Erica Andrews (Phrozensmoke ['at'] yahoo.com)
 * Released under the terms of the GPL.
 * *NO WARRANTY*
 *****************************************************************************/
/* 
	Videodog is Copyright (C) 2000, 
	Gleicon S. Moraes - gleicon@uol.com.br rde@linuxbr.com 

	Ayttm is  Copyright (C) 2003, the Ayttm team
	and Copyright (C) 1999-2002, Torrey Searle <tsearle@uci.edu>
*/

# include "gyacheupload.h"
#include "gytreeview.h"
# include <jasper/jasper.h>

#define yahoo_put32(buf, data) ( \
		(*((buf)) = (unsigned char)((data)>>24)&0xff), \
		(*((buf)+1) = (unsigned char)((data)>>16)&0xff), \
		(*((buf)+2) = (unsigned char)((data)>>8)&0xff), \
		(*((buf)+3) = (unsigned char)(data)&0xff), \
		4)

#include "../pixmaps/gyach-icon.xpm"
#include "../pixmaps/status_here.xpm"
#include "../pixmaps/status_here_male.xpm"

char **pixmap_gyach_icon = gyach_icon_xpm;
char **pixmap_status_here = status_here_xpm;
char **pixmap_status_here_male = status_here_male_xpm;

int is_friend(char *who);

char *CAM_VERSION="0.3";
char webcam_description[41]="";
char *local_address=NULL;
char *whoami = NULL;
char *webcam_to_view = NULL;
char *webcam_key = NULL;
char *second_server=NULL;
static char *gyache_err=NULL;

char packet_data[2050];
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
GtkWidget *vidpropwin=NULL;
char cammsg[255]="";
GList *viewers_list = NULL;
char *viewer_selected=NULL;
GtkTreeView *cam_viewers=NULL;
char *friends_list=NULL;
GtkWidget *allowall=NULL, *ignoreall=NULL, *allowfriends=NULL, *denyallbutfriends=NULL;
GtkWidget *ignoreanon=NULL;
int ignore_all=0, allow_friends=0, allow_everybody=0, deny_all_but_friends=0;
int ignore_anon=1;
int jasper_started=0;
int last_send_status=0;
char *lastconnwho=NULL;
char user_ip[24]="";
char user_extern_ip[24]="";

/* This is big enough in 'upload' state */
int MAX_PACKET_SIZE=2048;

int app_debugger=0;

void set_dummy_image() {
	gtk_image_set_from_stock(GTK_IMAGE(current_image), GTK_STOCK_STOP, GTK_ICON_SIZE_DIALOG);
}
void set_dummy_image_start() {
	gtk_image_set_from_stock(GTK_IMAGE(current_image), GTK_STOCK_CONVERT, GTK_ICON_SIZE_DIALOG);
}


void update_conn_status() {
	if (webcam_connected) 
	{
			if (webcam_connecting)  {
				gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Connecting...") ); 
													}
			else {
				if (video_paused || (! image_need_update) ) {
					gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Broadcasting  [Paused]") ); 
																					  } else {
					gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Broadcasting") ); 
																								}
					}
	} else	{gtk_label_set_text(GTK_LABEL(vstatuslabel),_("Not Broadcasting") );}
}

void shutdown_sockets() {
	if (camsocket != -1)  {close(camsocket); camsocket=-1;}
	camsocket=-1;
	webcam_connected=0;
	webcam_connecting=0;
	set_dummy_image();
	update_conn_status();
	remove_all_viewers();
	last_send_status=0;
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

void webcam_send_image(unsigned char *image, unsigned int length)
{
	u_char packet[6144];
	int wslen=0;
	u_char *spacket;

	if (length>6100) {return;} /* image is too large */

	packet_size=length;
	spacket=webcam_header(packet, -3); 
	wslen += 13;
	wslen += length;
	memcpy(spacket, image, length);
	webcam_sock_send( packet, wslen);
}

void yahoo_webcam_accept_viewer(char* who, int accept)
{
	u_char packet[301]="";
	// u_char *spacket=packet;
	char swho[85]="";
	unsigned char header_len = 13;
	unsigned int pos = 0;
	unsigned int len = 0;

	packet[pos++] = header_len;
	packet[pos++] = 0;
	packet[pos++] = 5; /* version byte?? */
	packet[pos++] = 0;

	len= snprintf(swho, 84, "u=%s\r\n",who); 
	pos += yahoo_put32(packet + pos, len);

	if (accept==-1)  { /* booting an already connected user */
		packet[pos++] = 6; /* packet type */
	} else {
		packet[pos++] = 0; /* packet type */
	}

	if (accept==-1)  { /* booting an already connected user */
		pos += yahoo_put32(packet + pos, 0);
	} else {
		pos += yahoo_put32(packet + pos, accept);	
	}
	memcpy(packet + pos, swho, len);
	pos +=len;
	packet[pos]='\0';
	webcam_sock_send( packet, pos);
}


void show_webcam_packet() {
	if ( ! webcam_recv_data( ))  {return ; }
	
	if (packet_type==0x00)  	{   /* user requests to view webcam  */		
		if (packet_size>0)  {
			char *viewwho=packet_data;
			char *tttmp;
			tttmp = strchr(viewwho+2, 0x0d);  /* return-char */
			if (tttmp)  {*tttmp = '\0';}
			webcam_viewer_connect(viewwho+2 , 2);
			return ;
										}
											}

	if (packet_type==0x05)  {	/* response packets when uploading */		
		if (packet_timestamp>0) {image_need_update=1;}
		else { image_need_update=0;}
		if (last_send_status != image_need_update) {
			last_send_status=image_need_update;
			update_conn_status();
		}
			/* we'll send the image in the next main_iteration loop */
		return ;
										}


	/*
		When a user connects, we get a packet with their name
		(packet type 0x0C), followed by a 'response' packet
		telling us to start sending images again, followed by 
		a packet with the IP address of the user who just
		connected...why Yahoo splits up the username from
		their IP, i don't know...
	*/

	if (packet_type==0x0C)  {  /* user connected */ 			
		if (packet_size>0)  {
			char *viewwho=packet_data;
			char *tttmp;
			tttmp = strchr(viewwho, 0x0d);  /* return-char */
			if (tttmp)  {*tttmp = '\0';}
	
			user_extern_ip[snprintf(user_extern_ip, 15, "%s", "0.0.0.0")]='\0';
			user_ip[snprintf(user_ip, 15, "%s", "0.0.0.0")]='\0';

			/* Save their name for IP address collection 2-3 packets from now */
			if (lastconnwho) {free(lastconnwho); lastconnwho=NULL;}
			lastconnwho=strdup(viewwho);
			webcam_viewer_connect(viewwho, 1);
			return ;
									  }  /* end if packet_size>0 */
										}  /* end if user connected */

	if (packet_type==0x0D)  {	/* user disconnected */  		
		if (packet_size>0)  {
			char *viewwho=packet_data;
			char *tttmp;
			tttmp = strchr(viewwho, 0x0d);  /* return-char */
			if (tttmp)  {*tttmp = '\0';}
			webcam_viewer_connect(viewwho, 0);
			if (lastconnwho) {free(lastconnwho); lastconnwho=NULL;}
			return ;
									   }
										}

	/* collect the IP address of whoever just connected */
	if ( (packet_type==0x13) && (packet_size>0) )  { /* IP Packet */
							char *tttmp, *uip1, *uip2;
								if (app_debugger) {printf("gathering IP-start\n"); fflush(stdout);}
							uip1=strstr(packet_data,"i=");
							uip2=strstr(packet_data,"j=");
							if (uip1 && uip2) {
								tttmp = strchr(uip1+2, 0x0d);  /* return-char */
								if (tttmp)  {*tttmp = '\0';}
								user_ip[snprintf(user_ip, 15, "%s", uip1+2)]='\0';
								tttmp = strchr(uip2+2, 0x0d);  /* return-char */
								if (tttmp)  {*tttmp = '\0';}
								user_extern_ip[snprintf(user_extern_ip, 15, "%s", uip2+2)]='\0';
									if (app_debugger) {printf("gathering IP-done\n"); fflush(stdout);}
								if (lastconnwho) {webcam_viewer_connect(lastconnwho, 1);}
								if (lastconnwho) {free(lastconnwho); lastconnwho=NULL;}
														}
												  									} 

}


void get_next_server() {
	u_char srequest[12]="";
	u_char wrequest[76]="";
	int wslen=0;
	u_char *wreqptr = webcam_header( srequest,  -1);
	u_char *camreqptr = webcam_header( wrequest, -2);

	wslen=snprintf(wreqptr, 10, "%s", "<RUPCFG>");

	if (! webcam_sock_send( srequest,  wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																		}

	wslen=snprintf(camreqptr , 75-9, "%s", "f=1\r\n");
	wslen +=8;

	if (! webcam_send_packet( wrequest, wslen, 8 ))  {
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
	u_char wrequest[601]="";
	int wslen=0;
	static char sholder[500]="";
	u_char *wreqptr = webcam_header( srequest,  -1);
	u_char *camreqptr;

	wslen=snprintf(wreqptr, 10, "%s", "<SNDIMG>");
	if (! webcam_sock_send( srequest,  wslen ))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																		}

	packet_size=snprintf(sholder ,499, 
	"a=2\r\nc=us\r\nu=%s\r\nt=%s\r\ni=%s\r\no=w-2-5-1\r\np=%d\r\nb=%s\r\n",
	whoami, webcam_key, local_address,  YCAM_DIALUP, webcam_description
	);

	camreqptr = webcam_header( wrequest, 1);
	wslen=snprintf(camreqptr ,600-14, "%s", sholder );

	wslen +=13;

	if (! webcam_send_packet( wrequest, wslen , 13))  {
		show_ok_dialog(gyache_err);
		shutdown_sockets();
		return;
																			}
	webcam_connected=1;	
	webcam_connecting=0;	
	image_need_update=1;
	update_conn_status();
}


void start_connection()  {
	if (!cam_is_open) {return; }

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

	
	/* We could do something like this: Make up an IP address as the 
	   'external' IP address that Yahoo will show to the people viewing our
	   cam, but this is pretty useless because Yahoo will send the 
	   viewer BOTH the 'external' (fake) IP and your real IP, and it is
	   completely up to the viewer's program to show either one or 
	   both.  Basically, faking the IP does not provide any level of 
	   privacy at all, as most third-party apps probably will show 
	   either the real IP or both.  The point: If you don't want your IP
	  exposed, just stay off the webcam system completely.   */

	/* 
	if (local_address) {free(local_address);}
	local_address= strdup("172.248.22.61");
	return;   */

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
	char  gerrmsg[151]="";
	char *ptr = buf;

	if (camsocket<0) {return 0;}

	while( size ) {
		sent = send( camsocket, ptr, size, 0 );
		if ( sent < 1 ) {
			snprintf( gerrmsg, 150,  "Error sending packet to socket: %d\n",
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

int webcam_send_packet(  u_char *bufp, int size, int headlen ) {
	/* set data length in packet */
	// bufp[headlen-1]=size - headlen;

	/* don't try to send if we're not connected */
	if ( camsocket == -1 ) {return( 0 );}
	/* send the packet */
	return webcam_sock_send( bufp, size );
}

int webcam_sock_recv( u_char *buf, int size ) {
	int recvd;
	char  gerrmsg[151]="";
	char *ptr = buf;

	if (camsocket<0) {return 0;}

	while( size ) {
		recvd = recv(camsocket, ptr, size, 0 );
		if ( recvd < 1 ) {
			snprintf( gerrmsg, 150, "Error receiving packet from socket: "
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


	/* grab the forwarding server if it's available and needed  */
	if ( (header_len>18) && (!second_server) )  {
			char webcsrv[20]="";
			buf[header_len] = '\0';
			memcpy( webcsrv, buf + 3, 16 );
			webcsrv[16]='\0';
			second_server=strdup(webcsrv);
				return( 1 );
																			}


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
	if (app_debugger) { printf("Packet:  header-%d  type-%d  reason-%d  size-%d   time-%d\n", header_len, packet_type, packet_reason, packet_size, packet_timestamp);
	fflush(stdout); 
							 } 
	 */

	/* avoid buffer overflows and weird behavior ... */
	if (packet_size<1) {return( 1 );}   /*  packet with data length=0 ? */
	if (packet_size>MAX_PACKET_SIZE)  {packet_size=MAX_PACKET_SIZE;}

	if ( ! webcam_sock_recv(buf + header_len, packet_size )) {return( 0 );}
	buf[packet_size + header_len] = '\0';
	memcpy( packet_data, buf + header_len, packet_size );
	packet_data[packet_size ] = '\0';
	return( 1 );
}


u_char *webcam_header(u_char *buf, int packtype) {
	u_char *ptr = buf;

	/* for headerless packets, just create the pointer */
	if (packtype==-1) {return( ptr );}

	if (packtype==-2) {
		unsigned char magic_nr[] = {8, 0, 1, 0};
		memcpy(buf, magic_nr, sizeof(magic_nr));
		ptr += 4;
		*((u_long *)ptr) =	htonl( 0x00000000 );	 /* packet length, to be filled in later */
		ptr += 4; 
	} else if (packtype==-3) {
		unsigned char  pytype[] = {2};
		unsigned char magic_nr[] = {13, 0, 5, 0};
		memcpy(buf, magic_nr, sizeof(magic_nr));
		ptr += 4;
		*((u_long *)ptr) =	htonl( packet_size );	 /* generic size */
		ptr += 4; 
		memcpy(ptr, pytype, sizeof(pytype));
		ptr += 1; 
		packet_timestamp++;
		*((u_long *)ptr) =	htonl( packet_timestamp );	 /* generic timestamp */
		ptr += 4; 
	} else {
		unsigned char magic_nr[] = {13, 0, 5, 0};
		unsigned char magic_nr2[] = {1, 0, 0, 0, 1};
		memcpy(buf, magic_nr, sizeof(magic_nr));
		ptr += 4;
		*((u_long *)ptr) =	htonl( packet_size );	 /* generic size */
		ptr += 4; 
		memcpy(ptr, magic_nr2, sizeof(magic_nr2));
		ptr += 5;
		}
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
	char outopts[95]="";

		if (app_debugger) { printf("Jasper-1\n");  fflush(stdout); }

	if (! jasper_started) {
	
	if(jas_init()) {
		if (app_debugger) {printf("Could not init jasper\n");}
		return NULL;
	}
		jasper_started=1;
							  }

		if (app_debugger) {printf("Jasper-2\n");  fflush(stdout);}

		outfmt = jas_image_strtofmt("jpc");

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

	if(!strcmp(jas_image_fmttostr(infmt), "jpc")) {
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

	if (app_debugger) {printf( "Encoding to format: %d %s\n", outfmt, "jpc");}

	/* We need compression options, otherwise Jasper creates HUGE 
	    Jpeg-2000 images from PNM (over 130kb!), these options get 
	    us to 3.5-5kb with reasonable quality - most of the images created
	   seem to be exactly 4096bytes, and normal Yahoo webcam 
	   images are usually between 2.5kb-5.0kb */

		/* This is what I was using before */
		snprintf(outopts, 93, "%s", "cblkwidth=64\ncblkheight=64\nnumrlvls=4\nrate=0.0165\nprcheight=128\nprcwidth=2048"); 

	/* 
	snprintf(outopts, 93, "%s", "cblkwidth=64\ncblkheight=64\nnumrlvls=4\nrate=0.0165\nprcheight=128\nprcwidth=2048\nnmode=real"); 
	*/ 
	
	/* Patch suggested by one random user who was using 
	Jasper 1.701 (an unsupported version), send in JPC 
	format, instead of JP2 using the following options... - NOTE: The JPC 
	implementation in Jasper is incomplete...after more testing
	this patch DID work it appears */

	/*  */
	snprintf(outopts, 93, "%s", "cblkwidth=64\ncblkheight=64\nnumrlvls=4\nrate=0.0165\nprcheight=128\nprcwidth=2048\nmode=real"); 
	

	/* snprintf(outopts, 93, "%s", "cblkwidth=96\ncblkheight=96\nnumrlvls=4\nrate=0.015625\nprcheight=128\nprcwidth=8192"); */

	if((jas_image_encode(image, out, outfmt, outopts))) {
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
	if (exit_on_error) {
		cam_app_quit=1;
		cleanup_v4l();
		mem_cleanup();
		exit(-1);
	}
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
	snprintf(cammsg, 253, "Gyach Enhanced  (%s):\n\n%s", _("Webcam Broadcaster"),  tmpest);

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

	if (exit_on_error) {
  		gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);
	} else  {
  		gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);
			}

  	gtk_box_pack_start (GTK_BOX (dlbox), dlabel, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);

	gtk_box_pack_start (GTK_BOX (dvbox), dbutton, FALSE, FALSE, 8);

  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_dialogw), NULL);

gtk_window_set_modal(GTK_WINDOW(dwindow), 1);
gtk_widget_show_all(dwindow);

	windowstyle = gtk_widget_get_style( dwindow );
	pm_icon = gdk_pixmap_create_from_xpm_d( dwindow->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( dwindow->window, NULL, pm_icon, mask_icon );

}

void show_error_dialog(char *mymsg)  {
	exit_on_error=1;
	cam_is_open=0;
	show_ok_dialog(mymsg);
}

void on_close_ok_propw (GtkWidget *widget, GdkEvent *event, gpointer  user_data)
{
	gtk_widget_destroy(vidpropwin);
}

void on_update_vid_prop(GtkButton *widget, gpointer  user_data)
{
	GtkWidget *tmp_widget;
	int thue=0,tbright=0,tcontrast=0,tcolor=0, tfc=0;

	tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "hue");
	thue= 256 * (int) gtk_range_get_value((GtkRange *) tmp_widget);
	tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "color");
	tcolor= 256 * (int) gtk_range_get_value((GtkRange *) tmp_widget);
	tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "contrast");
	tcontrast= 256 * (int) gtk_range_get_value((GtkRange *) tmp_widget);
	tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "brightness");
	tbright= 256 * (int) gtk_range_get_value((GtkRange *) tmp_widget);
	tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "tfc");
	tfc=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tmp_widget));
	set_vid_properties(thue, tcontrast, tcolor, tbright, tfc);
}

void show_vidprop_dialog (GtkMenuItem *menuitem, gpointer  user_data) {
	GtkWidget *dwindow;
	GtkWidget *dvbox;
	GtkWidget *dlbox;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;
	GtkWidget *ubutton;
	GtkObject *hadj1, *hadj2, *hadj3, *hadj4;
	GtkWidget *hscale1, *hscale2, *hscale3, *hscale4, *tfc;
	GtkStyle *windowstyle;
	GdkPixmap *pm_icon;
	GdkBitmap *mask_icon;

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  vidpropwin=dwindow;
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Camera Properties"));
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);

  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);  

	dlabel=gtk_label_new(_("GYach Enhanced: Camera Properties"));
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);

	ubutton=get_pixmapped_button(_("Apply"), GTK_STOCK_APPLY);
 	 gtk_object_set_data (GTK_OBJECT (ubutton), "mywindow", dwindow);

	dbutton=get_pixmapped_button(_("Close"), GTK_STOCK_CANCEL);
 	 gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);

	dlbox = gtk_vbox_new (FALSE, 2);

  	gtk_box_pack_start (GTK_BOX (dvbox), dlabel, FALSE, FALSE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(" "), FALSE, FALSE, 0);

	hadj1=gtk_adjustment_new (hue>-1?(int)(hue/256):215, 0, 256, 1, 1, 1);
	hscale1=gtk_hscale_new (GTK_ADJUSTMENT (hadj1));
	hadj2=gtk_adjustment_new (contrast>-1?(int)(contrast/256):215, 0, 256, 1, 1, 1);
	hscale2=gtk_hscale_new (GTK_ADJUSTMENT (hadj2));
	hadj3=gtk_adjustment_new (brightness>-1?(int)(brightness/256):215, 0, 256, 1, 1, 1);
	hscale3=gtk_hscale_new (GTK_ADJUSTMENT (hadj3));
	hadj4=gtk_adjustment_new (colour>-1?(int)(colour/256):215, 0, 256, 1, 1, 1);
	hscale4=gtk_hscale_new (GTK_ADJUSTMENT (hadj4));

  	gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(_("Hue:")), FALSE, FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (dvbox), hscale1, FALSE, FALSE, 0);

  	gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(_("Contrast:")), FALSE, FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (dvbox), hscale2, FALSE, FALSE, 0);

  	gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(_("Brightness:")), FALSE, FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (dvbox), hscale3, FALSE, FALSE, 0);

  	gtk_box_pack_start (GTK_BOX (dvbox), gtk_label_new(_("Color:")), FALSE, FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (dvbox), hscale4, FALSE, FALSE, 0);

	  tfc=gtk_check_button_new_with_mnemonic (_("Fix Color"));
	 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tfc), fix_color);
  	gtk_box_pack_start (GTK_BOX (dvbox), tfc, FALSE, FALSE, 3);

	 gtk_object_set_data (GTK_OBJECT (ubutton), "tfc", tfc);
 	 gtk_object_set_data (GTK_OBJECT (ubutton), "hue", hscale1);
 	 gtk_object_set_data (GTK_OBJECT (ubutton), "contrast", hscale2);
 	 gtk_object_set_data (GTK_OBJECT (ubutton), "brightness", hscale3);
 	 gtk_object_set_data (GTK_OBJECT (ubutton), "color", hscale4);

  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);
	gtk_box_pack_start (GTK_BOX (dlbox), ubutton, TRUE, TRUE, 4);
	gtk_box_pack_start (GTK_BOX (dlbox), gtk_label_new(" "), TRUE, TRUE, 4);
	gtk_box_pack_start (GTK_BOX (dlbox), dbutton, TRUE, TRUE, 4);

  gtk_signal_connect (GTK_OBJECT (ubutton), "clicked",
                      GTK_SIGNAL_FUNC (on_update_vid_prop), NULL);

  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_close_ok_propw), NULL);

  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_close_ok_propw), NULL);

gtk_window_set_modal(GTK_WINDOW(dwindow), 1);
gtk_widget_show_all(dwindow);

	windowstyle = gtk_widget_get_style( dwindow );
	pm_icon = gdk_pixmap_create_from_xpm_d( dwindow->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( dwindow->window, NULL, pm_icon, mask_icon );
}


void on_about_menu  (GtkMenuItem *menuitem, gpointer  user_data) {
	char abuf[300]="";  
	snprintf(abuf, 298, "Copyright (c) 2004\nErica Andrews\nPhrozenSmoke ['at'] yahoo.com\nLicense: GNU General Public License\nCam Broadcaster Version: %s", CAM_VERSION);
	show_ok_dialog(abuf);
}


void on_shutdown_app(GtkWidget *widget, GdkEvent *event, gpointer  user_data)
{	
	cam_app_quit=1;
	cleanup_v4l();
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
	update_conn_status();
}

void on_menu_disconnect(GtkMenuItem *menuitem, gpointer  user_data) {
	if (webcam_connecting) {return;}
	if (webcam_connected) {shutdown_sockets();}
}

void on_menu_connect(GtkMenuItem *menuitem, gpointer  user_data) {
	if (webcam_connecting) {return;}
	if (! webcam_connected) {start_connection() ;}
}

void on_confirm_yes(GtkButton *widget, gpointer  user_data)
{
	char *who=gtk_object_get_data (GTK_OBJECT (widget), "who");
	GtkWidget *tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "mywindow");
	if (who) {
		yahoo_webcam_accept_viewer(who, 1);
		free(who);
		who=NULL;
	}
	if (tmp_widget) {
		gtk_widget_destroy(tmp_widget);
	}
}

void on_confirm_no (GtkWidget *widget, GdkEvent *event, gpointer  user_data)
{
	char *who=gtk_object_get_data (GTK_OBJECT (widget), "who");
	GtkWidget *tmp_widget=gtk_object_get_data (GTK_OBJECT (widget), "mywindow");
	if (who) {
		yahoo_webcam_accept_viewer(who, 0);
		free(who);
		who=NULL;
	}
	if (tmp_widget) {
		gtk_widget_destroy(tmp_widget);
	}
}


void show_confirm_dialog(char *who)  {
	char mymsg[201]="";
	GtkWidget *dwindow;
	GtkWidget *dlbox;
	GtkWidget *dvbox;
	GtkWidget *dhbox;
	GtkWidget *dlabel;	
	GtkWidget *dbutton;
	GtkWidget *cbutton;
	char *dwho=strdup(who);

  dwindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_object_set_data (GTK_OBJECT (dwindow), "mywindow", dwindow);
  gtk_object_set_data (GTK_OBJECT (dwindow), "who", dwho);
  gtk_window_set_title (GTK_WINDOW (dwindow), _("GYach Enhanced: Question"));
  gtk_window_set_policy (GTK_WINDOW (dwindow), TRUE, TRUE, FALSE);
  gtk_window_set_position (GTK_WINDOW (dwindow), GTK_WIN_POS_CENTER);

  dvbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (dwindow), dvbox); 
  gtk_container_set_border_width (GTK_CONTAINER (dvbox), 6);

	snprintf(mymsg, 200, "%s:\n   '%s'", _("Do you wish to allow this user to view your webcam?"), who);
	dlabel=gtk_label_new(mymsg);
	gtk_label_set_line_wrap(GTK_LABEL(dlabel),1);

	dlbox = gtk_hbox_new (FALSE, 2);
  	gtk_box_pack_start (GTK_BOX (dlbox), GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_QUESTION, GTK_ICON_SIZE_DIALOG)), FALSE, FALSE , 2);

  	gtk_box_pack_start (GTK_BOX (dlbox), dlabel, TRUE, TRUE, 4);
  	gtk_box_pack_start (GTK_BOX (dvbox), dlbox, TRUE, TRUE, 4);

  dhbox = gtk_hbox_new (FALSE, 2);
  gtk_box_pack_start (GTK_BOX (dvbox), dhbox, FALSE, FALSE, 2);
  gtk_container_set_border_width (GTK_CONTAINER (dhbox), 6);

	dbutton=get_pixmapped_button(_("YES"), GTK_STOCK_YES);
  gtk_object_set_data (GTK_OBJECT (dbutton), "mywindow", dwindow);
  gtk_object_set_data (GTK_OBJECT (dbutton), "who", dwho);
	cbutton=get_pixmapped_button(_("NO"), GTK_STOCK_NO);
  gtk_object_set_data (GTK_OBJECT (cbutton), "mywindow", dwindow);
  gtk_object_set_data (GTK_OBJECT (cbutton), "who", dwho);

  gtk_box_pack_start (GTK_BOX (dhbox), dbutton, FALSE, FALSE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), gtk_label_new(" "), TRUE, TRUE,2);
  gtk_box_pack_start (GTK_BOX (dhbox), cbutton, FALSE, FALSE,2);

  gtk_signal_connect (GTK_OBJECT (dbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_confirm_yes), NULL);
  gtk_signal_connect (GTK_OBJECT (cbutton), "clicked",
                      GTK_SIGNAL_FUNC (on_confirm_no), NULL);

   /* Keep this disabled, otherwise, we can end up sending TWO 'no'
	replies, or a 'Yes', followed by a 'No' */
  /* 
  gtk_signal_connect (GTK_OBJECT (dwindow), "delete_event",
                      GTK_SIGNAL_FUNC (on_confirm_no), NULL);
  */

gtk_widget_show_all(dwindow);
}


void on_perm_menu1  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(allowall)->active) {
		allow_everybody=1;
		deny_all_but_friends=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(denyallbutfriends ), 0);
		ignore_all=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ignoreall), 0);
		allow_friends=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(allowfriends), 0);
																				} else {allow_everybody=0;}
}

void on_perm_menu2  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(ignoreall)->active) {
		 ignore_all=1;
		deny_all_but_friends=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(denyallbutfriends ), 0);
		allow_everybody=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(allowall), 0);
		allow_friends=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(allowfriends), 0);
																				} else { ignore_all=0;}
}

void on_perm_menu3  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(allowfriends)->active) {
		allow_friends=1;
		allow_everybody=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(allowall), 0);
		 ignore_all=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ignoreall), 0);
																				} else {allow_friends=0;}
}

void on_perm_menu4  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(denyallbutfriends)->active) {
		deny_all_but_friends=1;
		allow_everybody=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(allowall), 0);
		 ignore_all=0;
		gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ignoreall), 0);
																				} else {deny_all_but_friends=0;}
}

void on_perm_menu5  (GtkCheckMenuItem *menuitem, gpointer user_data) {
	if (GTK_CHECK_MENU_ITEM(ignoreanon)->active) {
		 ignore_anon=1;
																				} else { ignore_anon=0;}
}

void gyache_image_window_new(int width, int height, const char *title)
{
	GtkWidget *main_window;
	GtkWidget *vbox1;
	GtkWidget *btnClose;
	GtkStyle *windowstyle;
	GdkPixmap *pm_icon;
	GdkBitmap *mask_icon;
	GtkWidget *connection_menu, *vidprop;
	GtkWidget *about;      
	GtkWidget *help_main_menu;
	GtkWidget *help_main;
	GtkWidget *quit;
	GtkWidget *menubar1;
	GtkWidget *connection;
	GtkWidget *vimframe;
	GtkWidget *stopconn;
	GtkWidget *startconn, *imbox;
	GtkWidget *perm_main, *perm_main_menu;

	if (!title) {title = "Gyach Enhanced Webcam Broadcaster";}

			user_extern_ip[snprintf(user_extern_ip, 15, "%s", "0.0.0.0")]='\0';
			user_ip[snprintf(user_ip, 15, "%s", "0.0.0.0")]='\0';

if (app_debugger) {printf("openWindow-1a\n"); fflush(stdout);}

	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position (GTK_WINDOW (main_window), GTK_WIN_POS_CENTER);

if (app_debugger) {printf("openWindow-1b\n"); fflush(stdout);}

	gtk_window_set_title(GTK_WINDOW(main_window), title);

if (app_debugger) {printf("openWindow-1c\n"); fflush(stdout);}

	gtk_window_set_wmclass (GTK_WINDOW (main_window), "gyachEUpload", "gyacheupload");

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


  vidprop = gtk_image_menu_item_new_with_label (_("Camera Properties..."));
  gtk_container_add (GTK_CONTAINER (connection_menu), vidprop);
  gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(vidprop), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_PROPERTIES, GTK_ICON_SIZE_MENU) ));

	gtk_signal_connect_object(GTK_OBJECT(vidprop), "activate",
				GTK_SIGNAL_FUNC(show_vidprop_dialog),
				NULL);

  gtk_container_add (GTK_CONTAINER (connection_menu), gtk_menu_item_new ());

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

  perm_main = gtk_menu_item_new_with_label (_("Permissions"));
  gtk_container_add (GTK_CONTAINER (menubar1), perm_main);

  perm_main_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (perm_main), perm_main_menu);

  allowall = gtk_check_menu_item_new_with_label  (_("Automatically Allow All Users To View" ));
  gtk_container_add (GTK_CONTAINER (perm_main_menu), allowall);

  allowfriends = gtk_check_menu_item_new_with_label  (_("Automatically Allow All Friends To View" ));
  gtk_container_add (GTK_CONTAINER (perm_main_menu), allowfriends);

  gtk_container_add (GTK_CONTAINER (perm_main_menu), gtk_menu_item_new ());

  ignoreall = gtk_check_menu_item_new_with_label  (_("Automatically Deny All Users" ));
  gtk_container_add (GTK_CONTAINER (perm_main_menu), ignoreall);

  denyallbutfriends = gtk_check_menu_item_new_with_label  (_("Automatically Deny All Except Friends" ));
  gtk_container_add (GTK_CONTAINER (perm_main_menu), denyallbutfriends);

  ignoreanon = gtk_check_menu_item_new_with_label  (_("Automatically Deny Anonymous Users" ));
  gtk_container_add (GTK_CONTAINER (perm_main_menu), ignoreanon);
gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(ignoreanon), 1);

  gtk_signal_connect (GTK_OBJECT (allowall), "toggled",
                      GTK_SIGNAL_FUNC (on_perm_menu1),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignoreall), "toggled",
                      GTK_SIGNAL_FUNC (on_perm_menu2),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (allowfriends), "toggled",
                      GTK_SIGNAL_FUNC (on_perm_menu3),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (denyallbutfriends), "toggled",
                      GTK_SIGNAL_FUNC (on_perm_menu4),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (ignoreanon ), "toggled",
                      GTK_SIGNAL_FUNC (on_perm_menu5),
                      NULL);

  help_main = gtk_menu_item_new_with_label (_("Help"));
  gtk_container_add (GTK_CONTAINER (menubar1), help_main);
  gtk_widget_show (help_main);
  gtk_menu_item_right_justify (GTK_MENU_ITEM (help_main));

  help_main_menu = gtk_menu_new ();
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (help_main), help_main_menu);

  about = gtk_image_menu_item_new_with_label (_("About GYach-E Webcam Broadcaster..."));
  gtk_container_add (GTK_CONTAINER (help_main_menu), about);
gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(about), 
	GTK_WIDGET(gtk_image_new_from_stock(GTK_STOCK_DIALOG_INFO, GTK_ICON_SIZE_MENU) ));

  gtk_signal_connect (GTK_OBJECT (about), "activate",
                      GTK_SIGNAL_FUNC (on_about_menu),
                      NULL);

if (app_debugger) {printf("openWindow-1h\n"); fflush(stdout);}

	snprintf(webcam_description, 25, "%s", _("Unknown Camera"));
	start_cam();

	if (!exit_on_error) {cam_is_open=1;}

if (app_debugger) {printf("openWindow-2\n"); fflush(stdout);}

	gtk_widget_show(current_image);

	vimframe=gtk_frame_new(NULL);

	gtk_frame_set_shadow_type(GTK_FRAME(vimframe), GTK_SHADOW_IN);

	gtk_widget_set_usize(current_image, width, height);
	gtk_container_add(GTK_CONTAINER(vimframe), current_image);
	gtk_container_set_border_width(GTK_CONTAINER(vimframe), 5);

	imbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(imbox), vimframe, TRUE, TRUE, 1);
	gtk_box_pack_start(GTK_BOX(imbox), create_viewer_panel(), FALSE, FALSE, 2);
	gtk_box_pack_start(GTK_BOX(vbox1), imbox, TRUE, TRUE, 1);

	if (webcam_to_view) {snprintf(cammsg, 125, "%s: %s", webcam_to_view, webcam_description);}
	else {snprintf(cammsg, 125, "%s: %s", "?", webcam_description);}

	gtk_box_pack_start(GTK_BOX(vbox1), gtk_label_new(cammsg), FALSE, FALSE, 1);
	vstatuslabel=gtk_label_new("");
	gtk_box_pack_start(GTK_BOX(vbox1), vstatuslabel, FALSE, FALSE, 1);
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
	
	if (!exit_on_error) {
	gtk_widget_show_all(main_window);

	windowstyle = gtk_widget_get_style( main_window);
	pm_icon = gdk_pixmap_create_from_xpm_d(main_window->window,
		&mask_icon, &(windowstyle->bg[GTK_STATE_NORMAL]),
		(gchar **)pixmap_gyach_icon );
	gdk_window_set_icon( main_window->window, NULL, pm_icon, mask_icon );
							 }
}


/* Viewer management stuff */

gint gstrcmp( gpointer a, gpointer b ) {
	char cmpa[65];
	char cmpb[65];
	char *tttmp;
	if (( ! a ) && ( ! b )) {return( 0 );}
	if ( ! a ) {return( -1 );}
	if ( ! b ) {return( 1 );}

	strncpy(cmpa, (char *)a, 64);
	strncpy(cmpb, (char *)b, 64);
	tttmp = strstr(cmpa, "  ["); 
	if (tttmp)  {*tttmp = '\0';}
	tttmp = strstr(cmpb, "  ["); 
	if (tttmp)  {*tttmp = '\0';}
	/* printf("strings:  %s   %s\n", cmpa, cmpb); fflush(stdout); */
	return( strcasecmp( cmpa, cmpb ));
}

GList *gyach_g_list_free( GList *list ) {
	GList *node;

	if ( ! list )
		return( NULL );

	node = g_list_first( list );
	while( node ) {
		free( node->data );
		node = g_list_next( node );
	}
	g_list_free( list );

	return( NULL );
}

void remove_all_viewers()  {
	char viewstat[28]="";
	gy_empty_model(gtk_tree_view_get_model(cam_viewers), GYTV_TYPE_LIST);
	viewers_list = gyach_g_list_free(viewers_list);
	if ( viewer_selected ) {free( viewer_selected ); viewer_selected=NULL;}
	snprintf(viewstat, 27, "%s (%d)",_("Users"), 0);
	gtk_tree_view_column_set_title(gtk_tree_view_get_column(cam_viewers, 0),  viewstat);
}

void update_viewer_list()  {
	GtkTreeModel *freezer;
	GtkTreeIter iter;
	GList *node;
	char *viewr;
	GdkPixbuf *imbuf=NULL;
	char *listitem=NULL;
	char viewstat[28]="";
	char listershort[128];
	char *listitem2=NULL;
	int rows=0;


	freezer=freeze_treeview(GTK_WIDGET(cam_viewers));

	/* remove any items already in the list */
	gy_empty_model(freezer, GYTV_TYPE_LIST);

	if (viewers_list) {
		node = g_list_first( viewers_list );
		while( node ) {
			char *spptr=NULL;
			viewr=node->data ;
			listitem =strdup( viewr);
			snprintf(listershort, 126, "%s", listitem);  /* create name without the IP address */
			spptr=strchr(listershort,' ');			
			if (spptr) {*spptr='\0';}
			listitem2=strdup(listershort);
			gtk_list_store_append(GTK_LIST_STORE(freezer), &iter);

			if (is_friend(listitem2)) {
				imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here_male);
			} else {
				imbuf=gdk_pixbuf_new_from_xpm_data((const char**)pixmap_status_here);
			}

			gtk_list_store_set(GTK_LIST_STORE(freezer), &iter,  
				GYSINGLE_COL1, listitem, 
				GYSINGLE_PIX_TOGGLE, imbuf?TRUE:FALSE,  
				GYSINGLE_PIX, imbuf, 
				GYSINGLE_COLOR, NULL, 
				GYSINGLE_TOP,listershort, -1);
			if (imbuf) {g_object_unref(imbuf); imbuf=NULL;}
			node = g_list_next( node );
			free(listitem);
			free(listitem2);
			rows++;
							}
						}

	snprintf(viewstat, 27, "%s (%d)",_("Users"), rows);
	unfreeze_treeview(GTK_WIDGET(cam_viewers), freezer);
	gtk_tree_view_column_set_title(gtk_tree_view_get_column(cam_viewers, 0),  viewstat);
	if (imbuf) {g_object_unref(imbuf);}
}


void check_is_viewer_selected  ()
{
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		GtkTreeIter iter;

	if (! gtk_tree_view_get_model(cam_viewers)  ) {
		if ( viewer_selected ) {free( viewer_selected ); viewer_selected=NULL;}
		return;
	}

		selection=gtk_tree_view_get_selection(cam_viewers);
		if ( gtk_tree_selection_get_selected(selection, &model, &iter)) {
			gchar *sfound;
			gtk_tree_model_get(model, &iter,GYSINGLE_TOP, &sfound, -1);		
			if ( viewer_selected ) {	free( viewer_selected);  }
			viewer_selected=strdup(sfound);
			g_free(sfound);
		} else {
			if ( viewer_selected ) {
				free( viewer_selected );  
				viewer_selected=NULL;
			}
		}

	if (app_debugger) {printf("viewer selected:  %s\n", viewer_selected); fflush(stdout);}
}

void on_remove_viewer(GtkButton *widget, gpointer  user_data)
{
	check_is_viewer_selected();
	if (!viewer_selected) {return;}
	/* this finally works, removing people who are already viewing */	
	/* And on the 6th day, God created packet sniffers... */
	yahoo_webcam_accept_viewer(viewer_selected, -1);
}


GtkWidget *create_viewer_panel() {
  GtkWidget *scrolledwindow6, *bd_tree, *vbox1, *dsbutton;
  char *col_headers[]={"",NULL};

  vbox1 = gtk_vbox_new(FALSE, 0);
  scrolledwindow6 = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow6), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  gtk_box_pack_start(GTK_BOX(vbox1), scrolledwindow6, TRUE, TRUE, 1);

	dsbutton = get_pixmapped_button(_("Remove User"), GTK_STOCK_REMOVE);
	gtk_signal_connect_object(GTK_OBJECT(dsbutton), "clicked",
				GTK_SIGNAL_FUNC(on_remove_viewer),
				NULL );

/* TODO: add a way to 'remove' existing viewers  */
 /*  gtk_widget_set_sensitive(dsbutton, 0);  */

  gtk_box_pack_start(GTK_BOX(vbox1), dsbutton, FALSE, FALSE, 1);
  col_headers[0]=_("Users");
  bd_tree = GTK_WIDGET(create_gy_treeview(GYTV_TYPE_LIST,GYLIST_TYPE_SINGLE, 1, 
	1, col_headers));
  gtk_widget_ref (bd_tree);
  gtk_container_add (GTK_CONTAINER (scrolledwindow6), bd_tree);

  cam_viewers=GTK_TREE_VIEW(bd_tree);
  set_basic_treeview_sorting(GTK_WIDGET(bd_tree),GYLIST_TYPE_SINGLE);
  gtk_container_set_border_width(GTK_CONTAINER(vbox1), 5);

  gtk_widget_set_usize(scrolledwindow6, 180,-2);
	if (app_debugger) {
	add_viewer( "phrozensmoke");
	add_viewer( "phrozensmoke2");
	add_viewer( "phrozensmoke3");
	add_viewer( "phrozensmoke4");
	add_viewer( "phrozensmoke5");
	add_viewer( "phrozensmoke6");
	add_viewer( "phrozensmoke7");
	add_viewer( "phrozensmoke8");	
	add_viewer( "phrozensmoke9");
	 update_viewer_list();
							}
  return vbox1;
}



void lower_str( char *str ) {
	char tmp[86];
	char *dst = str;
	char *src = tmp;
	strncpy( tmp, str , 85);
	while( *src ) {
		*(dst++) = tolower( *(src++) );
	}
}

void remove_viewer( char *user ) {
	GList *this_user;
	char tmp_user[61];
	char *ptr;

	if ( ! user ) {	return;	} 

	strncpy( tmp_user, user, 60 );
	lower_str( tmp_user );

	this_user = g_list_find_custom( viewers_list, tmp_user,
		(GCompareFunc)gstrcmp );

	if ( this_user ) {
		ptr = this_user->data;
		viewers_list = g_list_remove( viewers_list, ptr );
		free( ptr );
	}
}

void add_viewer( char *user ) {
	char tmp_user[100];
	if ( ! user ) {	return;	} 

	remove_viewer( user);
	snprintf(tmp_user, 99, "%s  [%s / %s]", user, user_ip, user_extern_ip);
	lower_str( tmp_user );
	if ( g_list_find_custom( viewers_list, tmp_user, (GCompareFunc)gstrcmp ))
		return;
	viewers_list = g_list_prepend( viewers_list, strdup( tmp_user ));
}

int is_friend(char *who) {
	if (!friends_list) {return 0;}
	if (!who) {return 0;}
	
	if (!strchr(friends_list,',')) {  /* only one name on list */
			if (strcmp(friends_list,who)==0) {return 1;}
			else {return 0;}
	}  else {
			char *frontptr=NULL;
			char *backptr=NULL;
			frontptr=friends_list;
			while (frontptr)  {
				backptr=strchr(frontptr,',');
				if (backptr) {*backptr='\0';}		
				if (strcmp(frontptr,who)==0) {
					if (backptr) {*backptr=',';}
					return 1;
				}
				if (backptr) {*backptr=','; backptr++;}
				
				frontptr=backptr;
			}

		}
	return 0;
}

void webcam_viewer_connect(char *who, int connect) {
	char cwho[61]="";
	strncpy(cwho, who, 59);
	lower_str(cwho);
	
	if (app_debugger ) {
		printf("User connected: %s, %d\n", cwho, connect); fflush(stdout);
	}

	if (connect==1) {
			add_viewer( cwho);
			update_viewer_list();
		return; 
	}

	if (connect==0) {
			remove_viewer( cwho);
			update_viewer_list();
		return; 
	}

	if (connect==2) { /* requesting permission */
		if (ignore_all)  {
			yahoo_webcam_accept_viewer(cwho, 0);
			return; 
							   }	

		if (deny_all_but_friends)  {
			if (! is_friend(cwho)) { 
				yahoo_webcam_accept_viewer(cwho, 0);
				return; 
											  }
							   }	

		if (ignore_anon)  {

			/* Do we want to ignore 'anonymous' users like 
			'__anonym__64.63.206.11464.63.206.114' ??  ...something
			is wrong with anybody that needs to view anonymously, this
			feature is on by default, automatically kick them  */

			if (! strncasecmp(cwho, "__anon", 6))  {
				yahoo_webcam_accept_viewer(cwho, 0);
				return; 
			}			
			if (! strncasecmp(cwho, "anonymous", 9))  {
				yahoo_webcam_accept_viewer(cwho, 0);
				return; 
			}
		}

		if (allow_everybody)  {
			yahoo_webcam_accept_viewer(cwho, 1);
			return; 
							   }	
		if ( allow_friends)  {
			if (is_friend(cwho)) {
				yahoo_webcam_accept_viewer(cwho, 1);
				return; 
			}
							   		}	
		/* request permission here */
		show_confirm_dialog(cwho);
						 }
}


