GYach Enhanced 
Copyright (c) 2003-2004 Erica Andrews
PhrozenSmoke ['at'] yahoo.com
http://phpaint.sourceforge.net/pyvoicechat/
License:  GNU General Public License (GPL)


GYach (Original)
Copyright (c) 2000-2003 Chris Pinkham
cpinkham ['at'] corp.infi.net
http://www4.infi.net/~cpinkham/gyach/
License:  GNU General Public License (GPL)



NO TECHNICAL SUPPORT IS PROVIDED FOR GYACH ENHANCED - DO NOT EMAIL ME WITH YOUR PROBLEMS, FIX IT YOURSELF!


WHAT IS GYACH ENHANCED?

	GYach Enhanced is an unoffical 'fork' of the original GYach.  I liked original GYach over all other Yahoo chat clients for Linux, but there were too many of Yahoo's features missing.  GYach enhanced is an attempt to give Linux users many of the features you would have/see running the Windows Yahoo client - hopefully, without duplicating the bugs and clumsy interface of the Windows client and without treating the user like they are a complete idiot. I loved GYach, but I got REALLY tired of connecting to Yahoo chat rooms on Linux and constantly having to tell my friends "oh, sorry, my program doesn't support that feature - wait, let me get on Windows."


WHAT DOES GYACH ENHANCED HAVE THAT THE ORIGINAL GYACH DOESN'T?

	- VOICE CHAT: Gyach Enhanced is designed to work with pY! Voice Chat, making GYach Enhanced and pY! Voice Chat the FIRST software supporting Yahoo voice chat on Linux systems  (everybody that said it couldn't be done, were just too lazy to TRY.)
	- WEB CAM SUPPORT:  Both viewing webcams AND sending webcam streams to other users: The program features 2 webcam viewers and one webcam broadcaster.
	- Gyach Enhanced is also the first Yahoo application for Linux with support for the vast majority of the new features added to the Windows Y! Messenger 6: Display Images in the PM window, Avatars, Audibles, and Stealth Settings.
	-  Support for Yahoo 'nicknames' like you see on windows, sometimes user 'tommy1985' comes in under the nickname 'Randolf'...everybody on Windows would see 'Randolf' but people on Linux looked stupid calling the user 'tommy1985' and the rest of the room was wondering who the hell we were addressing.  UNLIKE windows, in the list of chat room members, you can see BOTH the user's real screen name and their 'nickname' (so you easily know who to ignore or mute if necessary.)
	- ENCRYPTED MESSAGING SUPPORT:  Send/receive encrypted instant messages using your choice of 3 available encryption plugins
	- Faders / Alt support ...see other people's faders and 'ALT' tags
	- A fader YOU can use, now you can send text using a fader with cool color instead of the cheesy standard colors
	- Full Yahoo! Address Book support
	- Full support for Yahoo 'conferences' - these are different from chat rooms:  You can now start a conference, invite friends, join a conference, etc.
	- Sound event support  (uses ESound only)
	- Lots of interesting 'snoops' that let you know when your friends are online 'invisible' or have gone 'invisible' instead of really logging off  (Very cute watching people say "I REALLY have to go to bed so I can get up early tomorrow...later.", only to see them go 'invisible' in the Yahoo Games Pool room hahaha.)
	- Support for viewing your My Yahoo! content (addressbook, weather, sports, etc.)
	- Notebook layout with buddy list, chat, and My Yahoo stuff on one easy to manage 'tab' interface.
	- Much improved anti-spam protection to avoid annoying posts and PMs from spam bots
	- True support for other people's colors (not the same old 5-6 colors you would see over 
	and over in the original GYach.
	- Better support for other people's fonts, with control over font sizes to avoid microscopic text and large 'billboards' on the screen, fonts are mapped to a 'concept' loading fonts on the system that look like what the other user is trying to send  (even if that exact font isn't available on our system.)
	- Support for viewing  the increasingly popular SmileyCentral.com smileys (without needing to download their spyware-tainted software!)
	- Foreign language support, with a Spanish language translation already complete
	- A basic buddy list (on it's own tab)
	- The ability to create Yahoo rooms with features you want, using a room-creation interface similar to the one on Windows
	- Easy-access buttons and menu items for getting user profiles, sending files. inviting people to rooms, sending PMs, viewing webcams, etc.
	- A completely over-hauled user interface, eliminated almost all fixed positions, and dammit, it just looks much prettier.
	- A 'smiley' selection window next to the 'send' boxes
	- Complete over-hauled PM and 'Find text...' dialog boxes.
	- Added color, font, and smiley support to PM windows, along with a nice interface and many other features
	- Each users age-sex-location is displayed (if it's available in their profile.) as they enter the room.
	- Dialog boxes are used for error messages, mail alerts, or when a file has been sent to you.
	- Support for 'buzzing' a user and receiving 'buzzes' when someone wants to get your attention.
	- A better choice of colors for enter/leave messages - the ugly bright lime greens and flaming bright reds are gone.
	- The Help window now displays some HELP.
	- Almost all features are now accessible from buttons and menu items, eliminating the need for memorizing a bunch of '/' commands (we're not on IRC anymore!), though the '/' commands are still supported for those who want to use them.
	- Dialog boxes when network connections fail: logins, getting room lists, etc.
	- Better handling of failed Yahoo login attempts:  Unlike the old GYach, you are TOLD your login has failed, and if possible, why (locked account, bad password, etc.)
	- The user interface is now a 'notebook' with 2 tabs:  Chat and Buddies, so you can see a REAL buddy list that you can click on instead of seeing your list of buddies uselessly printed on the chat window.
	- Buttons and menu items to access useful things like PM windows, file transfer, profiles, add/remove a buddy, etc.  (again, eliminating the need for '/' commands.) - also, the Buddy list tab let's you see the 'status' of your buddies.
	- Accept/send voice chat invitations
	- Notification of webcam invites
	- Notification when friends enter/leave Yahoo Games.
	- Very basic support for IMVironments (no animation or interaction though) just nice background images
	- Loads of security/privacy features to help ward off would-be stalkers, lunatics, and wannabe  'hackers'
	- Much improved icons for users in chat room: Gender-based icons for males/females, 
	  icons for people with webcams (both male/female also)
	- A buddy list that shows you ALL kinds of things most other buddy lists don't show: 
	  watching for 'invisible' friends, showing friends who are in chats/games/sms, friends who 
	  have their webcams on, etc.
	- Most of the time you'll get 'advanced notice' before a friend PMs you, with the 
          'peer-to-peer' hint (this is only true if the friend is using Windows Yahoo client)
	- 'Quick' profiles for people in chat rooms
	- A boot prevent module that monitors dangerous activity, does what it can to 
	  prevent you from being booted or receiving boot 'floods', and does some very 
          wicked things to people who are harassing you/stalking you
	- The ability to mute chat room people automatically based on age and gender
	- Lots of other features, check the screen shots and read the 'ChangeLog'...



COMPILE AND INSTALL

If while running 'make' you get complaints that files in the .deps directory cannot be found, run the 'config.status' script in the top directory, then  copy 'config.h.backup' to 'config.h'.  NOTE:  You may need to edit the Makefile and the 'config.h' header to suit your configuration tastes (Right now, there is NO intention of creating a configure script...I don't need it, and I don't feel like writing one.)  The C-based webcam viewer is now compiled by default and is in the subdirectory ./src/webcam/. Finally, type 'make'.   

If you want to be able to compile the webcam viewer and use it, you NEED libJasper (the Jpeg 2000 library) - version 1.500 or better for PyWebCam, and version 1.700.2 or better for GyachE-Webcam (version 1.700.5 is highly recommended.)  

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



NOTE:  Compiling for Gtk-1 is NO LONGER SUPPORTED AT ALL.


NO TECHNICAL SUPPORT IS PROVIDED FOR GYACH ENHANCED - DO NOT EMAIL ME WITH YOUR PROBLEMS, FIX IT YOURSELF!


I MAKE no guarantee that I will make future releases of GYach Enhanced.   (This means, if Yahoo changes the protocol and you can't connect anymore - FIX your code instead of whining to me, I'll only be maintaining my own copy.) However, I will definitely be willing to release language translations anyone wishes to contribute.

TRANSLATIONS
	Currently GYach Enhanced supports 2 languages: English and Spanish.  If you are 
	familiar with gettext, you may make new language translations using the 'gyach.pot' 
	translation catalogue in the './locale/' sub-directory.  Your translated catalogue 
	should be placed in the appropriate sub-directory of the './locale/' directory. The 
	'appropriate sub-directory' is the lower-case of the first 2 letters of your locale name: 
	Examples:  
		locale: 'es_MX' ->  directory: 'es'
		locale: 'ru_RU' ->  directory: 'ru'
	If you wish to have your language translation added to GYach Enhanced, send your 
	UNCOMPILED 'gyach.po' file in a .gz, .zip, .tar.gz, or .tar.bz2 file to:
		PhrozenSmoke [-AT-] yahoo.com
	Also, corrections to the Spanish translation are welcomed.



Various help documents are now available on the web:
	Help installing Gyach-E:  http://phpaint.sourceforge.net/pyvoicechat/install.php
	Help using Gyach-E:  http://phpaint.sourceforge.net/pyvoicechat/gyache-help.html


