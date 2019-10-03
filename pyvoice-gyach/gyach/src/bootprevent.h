/*****************************************************************************
 * bootprevent.h
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
 * VERY preliminary code for adjusting and responding to booting attempts
 *****************************************************************************/


extern unsigned long sock_sleep;
extern int enable_preemptive_strike;
extern int enable_basic_protection;

   /* fight fire with file ?? */
  /* Should we send a series of packets back to people sending us unwanted cam invites, conf/chat 
     invites, etc. - packets which will...umm, discourage them from continuing harassing us?  */

extern int possible_boot_attempts;

/* monitors possible boot attempts, taking appropriate action, such as adjusting socket flow control */
extern void check_boot_attempt();


/* send a series of packets back to people sending us unwanted cam invites, conf/chat 
     invites, etc. - packets which will...umm, discourage them from continuing harassing us?  */

extern void preemptive_strike(char *tmp);


extern void boot_attempt(char *who, char *method);
extern int is_boot_attempt(char *who, char *method, int packet_size);
extern void emergency_keep_alive(char *who);
extern void check_packet_suppression();
extern void push_packet_suppression ();
extern void flooder_buddy_list_protect();
extern int remove_flooder( char *flooder );
extern int is_flooder( char *flooder );
extern int add_flooder( char *flooder );
extern void log_possible_flood_attack(char *who, int points, char *method);
extern void show_ok_dialog(char *mymsg);


