	IceWMCP Tray Icon Plugin

	Copyright (C) 2004 Erica Andrews 
	(PhrozenSmoke ['at'] yahoo.com)
	http://icesoundmanager.sourceforge.net

	Support for FreeDesktop (IceWM/Gnome/KDE) 
	system tray icons.


NO TECHNICAL SUPPORT WILL BE PROVIDED 
FOR IceWMCP-TrayIcon, SINCE IT IS CONSIDERED
A PLUG-IN! PLEASE DO NOT E-MAIL WITH REQUESTS 
OR QUESTIONS ABOUT HOW TO COMPILE OR RUN
IceWMCP-TrayIcon.

ABOUT:
	This is the IceWMCP Tray Icon program.
	It shows a small icon in the 'icewmtray'
	system tray for quick access to all IceWMCP
	programs for easily configuring IceWM.
	It is considered a 'plugin' to IceWMCP.
	IceWMCP-TrayIcon is known to work on 
	IceWM (version 1.2.13) but should also 
	work with Gnome-2, KDE-3, and any other
	window manager with a NETWM/FreeDesktop.org-
	compliant tray notification area.

REQUIREMENTS:
	You need Gtk+-2 version 2.0.6 or better, 
	IceWM version 1.2.10 or better WITH 
	the 'icewmtray' program built, installed, 
	and running on your system.  If the 
	'icewmtray' program is not currently
	running, you will need to start it before
	the IceWMCP-TrayIcon will work. Also, 
	you should have IceWMCP (IceWM 
	Control Panel) installed on your system, 
	otherwise the tray icon will not be 
	very useful for you.

BUILDING AND INSTALLING:
	To configure for building, you will need 
	to edit the few variables at the beginning 
	of the 'Makefile'.  There is no 'configure' 
	script since this program is so small 
	(A 'configure' script would be overkill).
	Instructions are inside the 'Makefile' for 
	configuring the application.  When you 
	are done, type 'make', then type 
	'make install'.  That's it.  If you have 
	trouble compiling, try using one of the
	pre-compiled copies on the IceWMCP 
	web site:
		http://icesoundmanager.sourceforge.net

RUNNING IceWMCP-TrayIcon:
	Once the application is installed, you can 
	run the tray icon by executing IceWMCP-TrayIcon,
	normally at /usr/X11R6/bin/IceWMCP-TrayIcon.
	You should set up your IceWM startup script to
	run IceWMCP-TrayIcon when IceWM starts (if
	this is the behavior you want).  You can also 
	accomplish the same behavior by putting 
	a line to launch IceWMCP-TrayIcon in your 
	~/.xinitrc file.  If you use Bash as your shell, 
	you can put a line like this: "IceWMCP-TrayIcon &"
	in your IceWM startup script or you ~/.xinitrc file.

SOFTWARE LICENSE:
	A copy of the GNU General Public License under
	which this software is distributed can be found in
	the 'IceWMCP-TrayIcon-LICENSE' file.

OTHER NOTES:
	The .xpm data for the icons is stored in the 
	trayicons.h files (don't edit it unless you know
	what you are doing!).  IceWMCP-TrayIcon
	responds to both Left and Right mouse clicks: 
	Just click on it either way to get to the menu.
	To close the tray icon, click 'IceWMCP Tray 
	Icon' -> 'Close IceWMCP Tray Icon' on the 
	menu that pops up from the tray icon.
	Everything else should be self-explanatory.

TRANSLATORS:
	IceWMCP-TrayIcon is completely translatable.
	See the 'icewmcp-trayicon.pot' that comes 
	with the source distribution, or grab it from 
	the CVS server at: 
		http://icesoundmanager.sourceforge.net/cvs.php
	If you have the source distribution and 'pygettext' you 
	can also re-create 'icewmcp-trayicon.pot' if necessary 
	by running the './make_pot' script.  Then, follow
	the standard Gettext procedures.  See the translator's 
	page (http://icesoundmanager.sourceforge.net/translator.php)
	for information on how to have your translations included 
	in the next release.

