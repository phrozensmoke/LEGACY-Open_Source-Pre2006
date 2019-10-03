Last Update:  February 11, 2004

NO TECHNICAL SUPPORT IS PROVIDED
FOR THESE WEBCAM APPLICATIONS.

DO NOT EMAIL ME ABOUT GETTING THEM 
RUNNING ON YOUR SYSTEM, WITH 
QUESTIONS ABOUT INSTALLATION, OR ANY
OTHER REQUESTS FOR HELP - No Exceptions.
This IS your help.

_____________________________________________

  	GyachE-Webcam, PyWebCam, and
	GyachE-Broadcaster

  	Copyright (c) 2004
  	Erica Andrews
  	PhrozenSmoke ['at'] yahoo.com
  	http://phpaint.sourceforge.net/pyvoicechat/
  	License: GNU General Public License
_____________________________________________

Information about GyachE-Broadcaster can be found
toward the bottom of this text file.
_____________________________________________

GyachE-Webcam and PyWebCam README:


This directory contains TWO external webcam 
viewing utilities originally designed for Gyach 
Enhanced, but capable of being easily used 
by other chat applications with simple command 
lines.  Both programs require libJasper for 
Jpeg-2000 image conversions (version 1.700-2
or better for GyachE-Webcam, or 1.500 or better
for PyWebCam).  I advise you to use version 
1.700-5 (or better) as there are several KNOWN
memory leaks in version 1.700-2.


I STRONGLY ENCOURAGE YOU TO USE
THE PATCHED VERSION OF JASPER 
AVAILABLE AT THE GYACH ENHANCED
WEBSITE:

http://sourceforge.net/project/showfiles.php?group_id=57756

When you get to that site, scroll down to the bottom to
the 'Supplemental Packages' section.

This version of libJasper provided at the Gyach Enhanced site 
is a PATCHED version that fixes several bad memory leaks in 
the official version libJasper that can lead to slowed performance 
and even crashes.  If you choose to use these webcam utilities 
WITHOUT the recommended patched version of libJasper, 
expect problems!  However, if you choose to be hard-headed...

UNPATCHED versions of Jasper/
libJapser are available at the following locations:

	(Source Code, 1.4M)
	ftp://ftp.imagemagick.org/pub/ImageMagick/delegates/jasper-1.700.5.zip  

	http://www.ece.uvic.ca/~mdadams/jasper
	ftp://ftp.imagemagick.org/pub/ImageMagick/delegates/  (browse the FTP directory)
	http://www.rpmfind.net
	(if those fail, USE Google!)


First, let me say this:  These programs NEED to
be launched by Gyach Enhanced or another Y! 
chat program: They are NOT programs you can 
just pop open and 'browse' to the webcam you 
wish to see - Yahoo doesn't work that way, and 
typing bogus command-line arguments is not going
to get you into anybody's cam either.


_____________________________________________
INSTALLING/BUILDING:

	Edit the 'config.h' header in the top directory to 
	your liking, edit the Makefile(s) to your liking, 
	then type 'make', then 'make install' from this
	directory.  If you wish to link in libJasper+libjpeg
	staticly (this ensures your webcam viewer won't
	stop working if you ever happen to make an 
	upgrade of libJasper/libJpeg to an uncompatible
	version.), type 'make -f Makefile.static' then type
	'make install'.  That's it.  Figure out the rest on your
	own.  :-)  Please keep in mind that libJapser compiles
	against libJpeg, so upgrading one or the other 
	can easily create problems.  If you upgrade either 
	of these libraries frequently, strongly consider 
	staticly linking in libJasper and libJpeg.

_____________________________________________
GyachE-Webcam:

External webcam viewing utility written in C that uses libJasper:  

		While some of the code is borrowed from Ayttm,
		this viewer is much less memory intensive, is 
		singled-threaded, and does not rely on 
		Gtk 'input_add' (which often causes deadlocks, memory 
		leaks, etc.), Also, unlike the Ayttm cam viewer, this viewer 
		is an EXTERNAL program (meaning that IF it does crash, 
		it will crash ALONE instead of taking down Gyach 
		Enhanced and your Yahoo connection with it - I think 'risky' 
		apps are best run separately)...also, the app uses 
		GtkImage with GdkPixbuf instead of the soon-to-be-deprecated 
		GtkPixmap + GdkPixmap combination used in Ayttm.  The 
		app relies on the more simplified socket-reading characteristic 
		of the Gyach codebase, instead of the overly complicated 'callback' 
		system used in the Ayttm viewer  (In other words, it does the same 
		thing with much less code and much less confusion).  
		Currently, Gyache-Webcam only supports VIEWING,  but I hope 
		to have a 'sister' program available for sending webcam images 
		when I get a chance to play with the V4L stuff.  The program is 
		automatically launched by Gyach Enhanced, but as it takes 
		3 simple command-line arguments, it can easily be used as a 
		webcam viewer for ANY Yahoo app capable of spitting out 
		Yahoo's 'webcam key' on the command-line.  (Plugin suitable!)  
		Gyache-Webcam requires libJasper 1.700-2 or better (1.700-5 
		is recommended, as there are several known memory leaks in 
		version 1.700-2)....For the libJ2K fans - Sorry, I won't be 
		adding support for using libJ2K for the Jpeg-2000 support 
		anytime soon: When I tried using the latest release, 
		all it did was segfault, and when it wasn't doing that, it 
		dumped too much junk to stdout and created REALLY bad 
		renditions of the image (turned clear color images into black 
		and white distorted garbage, and the API is very Messy)...
		Everybody knows that I do my best to keep Gyach Enhanced 
		from printing excessive junk to stdout/stderr...so libJ2K is not 
		a candidate as it prints HUNDREDS of lines to stderr PER 
		converted image - that's crazy.  Gyache-Webcam uses as 
		few calls to malloc/calloc/realloc/strdup, etc  as possible 
		to help  minimize the risk of memory leaks now and in the future.
		This code is in the ./src/webcam/ subdirectory of the source
		code path.

   This program borrows alot of code from both Ayttm and 
    Gyach-E itself, however, this program is a completely 
    restructured webcam viewer that handles sockets and threads
    in the Gyach-E way, not the Ayttm way of many threads, many 
    structs, etc.  It is designed for simplicity, speed, 
    memory-friendliness, and stability: It runs as an EXTERNAL 
    program to Gyach Enhanced, so that if it DOES crash, it 
    crashes ALONE, rather than taking down an entire chat program
    with it. It is a clean, efficient SINGLE-THREADED application 

_____________________________________________
PyWebCam:

  pY! WebCam - Library and User Interface:
     A simple program for viewing Y! webcams
     using the 'jasper' executable for JPEG-2000
     support

External webcam viewing utility written in Python that uses the 'jasper' executable:  
		This webcam viewer was actually written before the Gyache-Webcam C version,
		but both are similar in look-and-feel.  However, pYWebCam uses the 'jasper'
		executable (from libJasper) instead of the library itself.  For this reason, it 
		is KNOWN to be slower than the C version and uses more system resources
		than the C version (It is not recommended for incredibly slow systems).  However, 
		I'm keeping it around (no harm in having TWO Yahoo cam viewers) because it 
		still does a good job and is useful for people who have the 'jasper' executable 
		running on their system but cannot (or will not) upgrade to libJasper-1.700-2+, 
		or want an example of a Python implementation of viewing Yahoo webcams. It 
		does not rely on a particular version of libJasper, as long as whatever version 
		you are running is capable of converting from Jpeg-2000 to Jpeg format.  For 
		this reason, it is probably more portable.  It uses PyGtk-2 for the user interface, 
		so PyGtk-2 1.99 or better is required.  The program is 
		automatically launched by Gyach Enhanced, but as it takes 
		3 simple command-line arguments, it can easily be used as a 
		webcam viewer for ANY Yahoo app capable of spitting out 
		Yahoo's 'webcam key' on the command-line.  (Plugin suitable!)  
		This code is in the ./src/webcam/ subdirectory of the source
		code path.


 This program borrows alot of code from Ayttm, 
 however, this program is a completely 
 restructured webcam viewer that handles 
 sockets and threads in a much cleaner manner.
 It is designed for simplicity and portability.
 It uses the extern 'jasper' executable instead of the 
 libJasper library (libJapser libraries are not available
 for Python.) It runs as an EXTERNAL program to 
 Gyach Enhanced, so that if it DOES crash, it 
 crashes ALONE, rather than taking down an entire 
 chat program with it.

_____________________________________________
Using GyachE-Webcam and/or PyWebCam
As A Webcam Viewer for other Y! Chat Programs:

	First, the programs are for viewing Yahoo
	webcams ONLY: MSN and all the other 
	services are NOT supported.

Because both programs run externally, and are not 
built directly into Gyach Enhanced, they can easily 
be used as 'plugins' for other Y! chat applications.
To do so, they need to be launched with the 
following command-lines: 

gyache-webcam [who_we_are_viewing] [your_screename] [web_cam_key_from_gyachE]
	OR
pywebcam.py [who_we_are_viewing] [your_screename] [web_cam_key_from_gyachE]

The argument '[web_cam_key_from_gyachE]' is the webcam
key that the chat application must request using the YMSG
protocol.  (Figuring out how to do this is YOUR problem.) 
After the chat program has the webcam key, it can simply 
spawn a new process using the command-lines shown 
above.  If you don't understand what I'm talking about 
as far as 'webcam keys' and ymsg, you need NOT be 
reading this.  DO NOT ASK ME FOR HELP ON THIS 
FEATURE.  These are NOT applications that you can 
just launch on your own with no command line arguments 
and 'browse' webcams!  Remember to put all the 
command-line arguments in quotes.
_____________________________________________

GyachE-Broadcaster:

	This application allows sending of webcam streams
	to other Yahoo users Right now this program has only 
	been tested with a single Video4Linux device: An OV511 
	DLink C100 USB webcam.  The program uses Video4Linux-1
	for image capture and the libJasper library for Jpeg-2000
	image conversions. 

	Camera Requirements:
		To use this program to send a webcam stream to
		other Yahoo users, your camera must meet the
		following criteria:

		1. Be compliant and functional with the 
		Video4Linux (version 1) interface; V4L-2 is
		not supported.  If your camera does not 
		work with Video4Linux-1 or requires the use
		of additional software drivers which aren't
		part of the standard Linux kernel, you should
		expect problems.  This program relies on 
		V4L-1 only and will not be adapted to work
		with weird, non-standard, or proprietary
		software drivers.
		
		2. Be accessible from a DevFS (Device 
		File System) file, such as /dev/video, 
		/dev/video0, etc.  If your cam needs to be
		called up from something strange like a
		PCI bus address: No good.  Supposedly, 
		the DevFS is 'deprecated' in some newer
		kernels, so if your system doesn't support
		the device file system, you're out of luck.

		3. Your webcam must be able to capture 
		24-bit RGB images (most modern cams 
		can do this with no problem.)  If your cam
		uses some weird, non-standard color 
		palette, expect problems.

		4. Your cam CANNOT be a 'greyscale'
		or 'black and white' cam; I really do not
		think this is going to work with Gdk's 
		'colorspace' which requires RGB.

		5. Your cam MUST be able to capture
		images at 320x240 resolution.  If your 
		cam can't capture this size or larger, 
		you're in trouble.  This means old cams
		that can only capture really tiny pictures
		cannot be used.

	This program is known to work with the following
	camera: OV511 DLink C100 USB; If the program 
	does not work with your camera...open a text 
	editor, patch the code for yourself, and re-compile.
	:-)  Do not ask me to write 'patches' to support 
	your cam...I wrote this program for me, now, the
	code has been turned over to you as a starting
	point for getting your own cam working on Yahoo.

	This program borrows alot of code from both Ayttm and 
    	Gyach-E itself, as well as the old 'videodog' program
    	for a few decent V4L usage examples.

    	It is designed for simplicity, speed, memory-friendliness, 
	and stability: It runs as an EXTERNAL program to Gyach 
	Enhanced, so that if it DOES crash, it crashes ALONE, 
	rather than taking down an entire chat program with it. 
	It is a clean, efficient SINGLE-THREADED application .


This program requires libJasper for 
Jpeg-2000 image conversions (version 1.700-2
or better).  I advise you to use version 
1.700-5 (or better) as there are several KNOWN
memory leaks in version 1.700-2.

I STRONGLY ENCOURAGE YOU TO USE
THE PATCHED VERSION OF JASPER 
AVAILABLE AT THE GYACH ENHANCED
WEBSITE:

http://sourceforge.net/project/showfiles.php?group_id=57756

When you get to that site, scroll down to the bottom to
the 'Supplemental Packages' section.

This version of libJasper provided at the Gyach Enhanced site 
is a PATCHED version that fixes several bad memory leaks in 
the official version libJasper that can lead to slowed performance 
and even crashes.  If you choose to use these webcam utilities 
WITHOUT the recommended patched version of libJasper, 
expect problems!  However, if you choose to be hard-headed...

UNPATCHED versions of Jasper/
libJapser are available at the following locations:

	(Source Code, 1.4M)
	ftp://ftp.imagemagick.org/pub/ImageMagick/delegates/jasper-1.700.5.zip  

	http://www.ece.uvic.ca/~mdadams/jasper
	ftp://ftp.imagemagick.org/pub/ImageMagick/delegates/  (browse the FTP directory)
	http://www.rpmfind.net
	(if those fail, USE Google!)


This  program NEEDS to be launched by Gyach 
Enhanced or another Y! chat program: 
You cannot simply start it up on your own, or 
feed it bogus command-line arguments and
expect to see your cam go online.

_____________________________________________

Using GyachE-Broadcaster
As A Webcam Broadcaster for other Y! Chat Programs:

	First, the program is for sending 
	webcam streams over the Yahoo
	service ONLY: MSN and all the other 
	services are NOT supported.

Because this program runs externally, and is not 
built directly into Gyach Enhanced, it can easily 
be used as a 'plugin' for other Y! chat applications.
To do so, it needs to be launched with the 
following command-lines: 

gyache-upload [your_screename] [web_cam_key_from_gyachE] [v4l_device] [optional_comma_separated_friends_list]

The argument '[web_cam_key_from_gyachE]' is the webcam
key that the chat application must request using the YMSG
protocol.  (Figuring out how to do this is YOUR problem.) 
The 'v4l_device' should be a DevFS device such as 
/dev/video, /dev/video0, or /dev/video1, etc.  The 
'optional_comma_separated_friends_list' is just what it
says, OPTIONAL, but it cam be used to automatically
allow friends to view your cam.  An example 
'optional_comma_separated_friends_list' value may
look something like this: "harry,kevin,james,kathrine..."
(there should be no spaces in the line at all.)
After the chat program has the webcam key, it can simply 
spawn a new process using the command-line shown 
above.  If you don't understand what I'm talking about 
as far as 'webcam keys' and ymsg, you need NOT be 
reading this.  DO NOT ASK ME FOR HELP ON THIS 
FEATURE.  Remember to put all the command-line 
arguments in quotes.
_____________________________________________
LICENSE 

(for GyachE-Webcam, PyWebCam,
and GyachE-Broadcaster):

 
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as
  published by the Free Software Foundation; either version 2 of the
  License, or (at your option) any later version.
 
  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston,
  MA 02111-1307, USA.
 
  Copyright (C) 2003-2004 Erica Andrews (Phrozensmoke ['a'] yahoo.com)
  Released under the terms of the GPL.
  NO WARRANTY
