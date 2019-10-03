/*****************************************************************************
 * prflang.h
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
 *****************************************************************************/

#ifndef _PRFLANG_H_
#define _PRFLANG_H_


enum
{
  GYPRFL_LANG = 0,
  /*  GYPRFL_CHARSET, */ /* not needed only supporting charset iso-8859-1 */ 
  GYPRFL_ID,
  GYPRFL_MAIL, 
  GYPRFL_NAME,   
  GYPRFL_LOC,
  GYPRFL_AGE,
  GYPRFL_MAR,
  GYPRFL_SEX,
  GYPRFL_JOB,
  GYPRFL_HOB,
  GYPRFL_NEWS,
  GYPRFL_QUOTE,
  GYPRFL_PAGE,
  GYPRFL_COOL1,
  GYPRFL_COOL2,
  GYPRFL_COOL3,
  GYPRFL_SEARCH,  /* This is also the 'last updated' field */ 
  GYPRFL_CONFIRM  /* 18 */ 
} ;

char **gprfl_sel_lang;
char **gprfl_en_lang;  /* english fallback */ 

void select_profile_lang_strings(char *content);



#endif 




