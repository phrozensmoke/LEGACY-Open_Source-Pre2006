# -*- coding: ISO-8859-1 -*-

################################################
## IceMe
# =====
#
# Copyright 2000-2002, Dirk Moebius <dmoebius@gmx.net> 
# and Mike Hostetler <thehaas@binary.net>
#
# This work comes with no warranty regarding its suitability for any purpose.
# The author is not responsible for any damages caused by the use of this
# program.
#
# This software is distributed under the GNU General Public License
#################################################
#################################
#  With Modifications by Erica Andrews 
#  (PhrozenSmoke ['at'] yahoo.com)
#  February 2003-February 2004
#  
#  This is a modified version of IceMe 
#  1.0.0 ("IceWMCP Edition"), optimized for 
#  IceWM ControlPanel.
#  
#  Copyright (c) 2003-2004
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://icesoundmanager.sourceforge.net
#################################
#############################################
#	This program is free software; you can redistribute
#	it and/or modify it under the terms of the GNU 
#	General Public License as published by the 
#	Free Software Foundation; either version 2 of the
#	License, or (at your option) any later version.
#
#	This program is distributed in the hope that it will 
#	be useful, but WITHOUT ANY WARRANTY; 
#	without even the implied warranty of 
#	MERCHANTABILITY or FITNESS FOR A 
#	PARTICULAR PURPOSE.
#
#	You should have received a copy of the GNU 
#	General Public License along with this program; 
#	if not, write to the Free Software Foundation, Inc., 
#	59 Temple Place - Suite 330, Boston, MA 
#	02111-1307, USA.
#############################################
#############################
#  PyGtk-2 Port Started By: 
#  	David Moore (djm6202@yahoo.co.nz)
#	March 2003
#############################
#############################
#  PyGtk-2 Port Continued By: 
#	Erica Andrews
#  	PhrozenSmoke ['at'] yahoo.com
#	October/November 2003
#############################

import sys, os, string

## here are some things to tweak, if you want
### patterns for icons
ICON_PATTERNS = ["mini/*","*"]

### icon types:
ICON_TYPES = ['xpm',"png"]

#######################################
##### Don't change anything under here!
##############################################
# the location of the script itself:
SCRIPT_DIR = sys.path[0]

# the user home dir:
HOME = os.environ["HOME"]

# the icewm dir in the users home:
from icewmcp_common import *
HOME_ICEWM = getIceWMPrivConfigPath()

# the list of paths in the environment variable $PATH:
PATH = string.split(os.environ["PATH"], os.pathsep)

# the command to run IcePref:
ICEPREF = "icepref"
# constants for entries in the tree menu, used by IceMenuTree and MenuParser:
MENUTREE_PROG = 1
MENUTREE_RESTART = 2
MENUTREE_SUBMENU = 3
MENUTREE_SUBMENU_END = 4
MENUTREE_SEPARATOR = 5
MENUTREE_MENUFILE= 6
MENUTREE_UNKNOWN = 42

# what a seperator should look like:
SEP_STRING = "------------------------"  # changed 4.3.2003, Erica Andrews - separator is a little longer

# constants from unistd.h for the second argument to os.access(...).
# these may be OR'd together.
R_OK = 4    # test for read permission
W_OK = 2    # test for write permission
X_OK = 1    # test for execute permission
F_OK = 0    # test for existence



