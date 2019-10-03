/*****************************************************************************
 * images.c
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
 * cpinkham@infi.net, cpinkham@bc2va.org
 * http://www4.infi.net/~cpinkham/gyach/
 *****************************************************************************/

#include "../pixmaps/status_away.xpm"
#include "../pixmaps/status_here.xpm"
#include "../pixmaps/status_here_male.xpm"
#include "../pixmaps/status_here_female.xpm"
#include "../pixmaps/status_cam.xpm"
#include "../pixmaps/status_cam_male.xpm"
#include "../pixmaps/status_cam_female.xpm"
#include "../pixmaps/status_idle.xpm"
#include "../pixmaps/status_ignore.xpm"
#include "../pixmaps/status_mute.xpm"
#include "../pixmaps/bold.xpm"
#include "../pixmaps/italic.xpm"
#include "../pixmaps/underline.xpm"
#include "../pixmaps/gyach-icon.xpm"
#include "../pixmaps/colors.xpm"
#include "../pixmaps/buddonline.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/buddaway.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/buddidle.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/buddoffline.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/mail.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_buzz.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_close.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_erase.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_file.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_ignore.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_join.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_profile.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_send.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_voice.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_save.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_tux.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_encrypt.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/no_photo.xpm"  /* added PhrozenSmoke */

#include "../pixmaps/launchcast.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/sms.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/webcam.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/games.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/chat.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/stealth_on.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/stealth_off.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/pm_audible.xpm"  /* added PhrozenSmoke */
#include "../pixmaps/loglogo.xpm"  /* added PhrozenSmoke */


char **pixmap_log_logo = loglogo_xpm;

char **pixmap_status_here = status_here_xpm;
char **pixmap_status_here_male = status_here_male_xpm;
char **pixmap_status_here_female = status_here_female_xpm;
char **pixmap_status_cam = status_cam_xpm;
char **pixmap_status_cam_male = status_cam_male_xpm;
char **pixmap_status_cam_female = status_cam_female_xpm;
char **pixmap_status_away = status_away_xpm;
char **pixmap_status_ignore = status_ignore_xpm;
char **pixmap_status_mute = status_mute_xpm;
char **pixmap_status_idle = status_idle_xpm;
char **pixmap_bold = bold_xpm;
char **pixmap_italic = italic_xpm;
char **pixmap_underline = underline_xpm;
char **pixmap_gyach_icon = gyach_icon_xpm;
char **pixmap_colors = colors_xpm;
char **pixmap_buddy_offline = buddoffline_xpm;
char **pixmap_buddy_online = buddonline_xpm;
char **pixmap_buddy_away = buddaway_xpm;
char **pixmap_buddy_idle = buddidle_xpm;
char **pixmap_mail = mail_xpm;
char **pixmap_pm_buzz = pm_buzz_xpm;
char **pixmap_pm_close = pm_close_xpm;
char **pixmap_pm_erase = pm_erase_xpm;
char **pixmap_pm_file = pm_file_xpm;
char **pixmap_pm_ignore = pm_ignore_xpm;
char **pixmap_pm_join = pm_join_xpm;
char **pixmap_pm_profile = pm_profile_xpm;
char **pixmap_pm_send = pm_send_xpm;
char **pixmap_pm_voice = pm_voice_xpm;
char **pixmap_pm_save = pm_save_xpm;
char **pixmap_pm_tux = pm_tux_xpm;
char **pixmap_pm_encrypt = pm_encrypt_xpm;
char **pixmap_no_photo = no_photo_xpm;

char **pixmap_launchcast=launchcast_xpm;
char **pixmap_sms=sms_xpm;
char **pixmap_webcam=webcam_xpm;
char **pixmap_games=games_xpm;
char **pixmap_chat=chat_xpm;
char **pixmap_stealth_on=stealth_on_xpm;
char **pixmap_stealth_off=stealth_off_xpm;
char **pixmap_pm_audible=pm_audible_xpm;

static char *dummy_pixmap_xpm[] = {
"2 2 2 1",
" 	c None",
".	c #FFFFFF",
"  ",
". "};

char **pixmap_dummy=dummy_pixmap_xpm;





