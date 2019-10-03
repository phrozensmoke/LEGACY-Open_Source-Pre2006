Copyright 2003 Erica Andrews  (PhrozenSmoke@yahoo.com). 
All rights reserved.
http://ganim8.sourceforge.net

GtkSnapshot is part of gAnim8, a suite for building, viewing, and editing animated GIFS and small videos, but is also available as a stand-alone version.   GtkSnapshot is a Gtk-based program for taking screenshots of X desktops, individual windows, or a portion of an X screen. Technically, it"s just a quick frontend for ImageMagick's "import", "convert", and "mogrify" utilities.  NOTE:  GtkSnapshot is still EXPERIMENTAL, but seems to be working rather well.  It is a new addition to the gAnim8 suite.  This program uses ImageMagick's 'import' utility for making screenshots - so, please do not whine to me if GtkSnapshot doesn't take large screenshots as fast as you want: GtkSnapshot is ONLY a frontend for ImageMagick's 'import' utility.  So, all actual creation of screenshots is done by ImageMagick.

Some image formats listed may not be supported on your system and may fail to convert or even crash this program. If you have trouble converting or saving screenshots to very obscure formats, please check the ImageMagick documentation for additional information on extra filters and third-party programs which may be required for the format. Also, make sure your have a recent version of ImageMagick.

Running the program:  You should be able to launch the program by simply executing 'gtksnapshot', or by executing 'python /usr/share/gAnim8/GtkSnapshot.py'.


This program requires Python 2.2 or better, Python Numeric, PyGTK 0.6.9 or better,  Gtk+ 1.2.8-4 or better, Imlib 1.9.8.1-6 or better, ImageMagick 5.2.3 or better (5.4.6 or better recommended).  Recommended but not required: gifsicle 1.3.0 or better for GIF compression.


GETTING GIFSICLE:
You can get the latest Gifsicle source code and binary packages (RPM) from  http://www.lcdf.org/gifsicle/

NOTE: This program is built on top of Gifsicle, so you can do nothing with gAnim8 until you get Gifsicle:
http://www.lcdf.org/gifsicle/

There are some RPMS for Gifsicle on  speakeasy.rpmfind.net and http://www.lcdf.org/gifsicle/


GETTING IMAGEMAGICK:
The ImageMagick package can be found on both speakeasy.rpmfind.net and www.imagemagick.org .


gAnim8, the gAnim8 Conversion Tool, and GtkSnapshot are distributed under the W3C License. This program and its source may be freely modified and redistributed for non-commercial,  non-profit use only. Permission to use, copy, modify, and distribute this software and its documentation, with or without modification,  for any purpose and WITHOUT FEE OR ROYALTY is hereby granted, provided these credits are left in-tact and a copy of the 'LICENSE' document accompanies all copies of this software. COPYRIGHT HOLDERS WILL NOT BE LIABLE FOR ANY DIRECT,INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF ANY USE OF THE SOFTWARE OR DOCUMENTATION.  See the document 'LICENSE' for a complete copyof this software license agreement.