#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

######################################
#  pY! Webcam
#  
#  Copyright (c) 2004-2006
#  Erica Andrews
#  PhrozenSmoke ['at'] yahoo.com
#  http://phpaint.sourceforge.net/pyvoicechat/
#  License: GNU General Public License
######################################
#######################################
#  pY! WebCam - Library and User Interface:
#     A simple program for viewing Y! webcams
#     using the 'jasper' executable for JPEG-2000
#     support
#
#  This program is distributed free
#  of charge (open source) under the 
#  terms of the GNU General Public 
#  License
#######################################
######################################
# This program borrows alot of code from Ayttm, 
# however, this program is a completely 
# restructured webcam viewer that handles 
# sockets and threads in a much cleaner manner.
# It is designed for simplicity and portability.
# It uses the extern 'jasper' executable instead of the 
# libJasper library (libJapser libraries are not available
# for Python.) It runs as an EXTERNAL program to 
# Gyach Enhanced, so that if it DOES crash, it 
# crashes ALONE, rather than taking down an entire 
# chat program with it.
######################################


import os,socket,sys,string,time,struct,copy,random,binascii, gettext
from threading import Thread

# Some PyGtk2 path settings while we upgrade from PyGtk-1 to PyGtk2
try:
		import pygtk
		pygtk.require("2.0")
except:
		pass
try:
		import sys
		sys.path=["/usr/lib/python2.2/site-packages/gtk-2.0"]+sys.path
		sys.path=["/usr/lib/python2.3/site-packages/gtk-2.0"]+sys.path
		import gtk
		import gtk.gdk 
		GDK=gtk.gdk
		GTK=gtk		
		from gtk import *
except:
		print "You do not have PyGtk-2 properly installed."
		sys.exit(0)
		pass


APP_VERSION="0.1"

global connect_status, tcp_socket, last_error
connect_status=0  # 0-no conn., 1-connecting, 2-connected, 3-disconn
tcp_socket=None

def get_connect_status():
	global connect_status
	return connect_status

main_server="webcam.yahoo.com"
main_port=5100

global cam_srv, cam_port
cam_srv=main_server
cam_port=main_port

last_error=None

global camuser,whoami,webcam_key
camuser=""
whoami=""
webcam_key=""

global to_read, data_size
to_read=0
data_size=0

global quit_called
quit_called=1

global imbuf
imbuf=""

global webcam_downloading  
# are we uploading/downloading?
# only downloading is supported right now
webcam_downloading =1

global my_ip
my_ip="127.0.0.1"

global frame_setter_method
frame_setter_method=None

global prefwidth, prefheight
prefwidth=320
prefheight=240

def setup_connection(c,w,k):
	global camuser,whoami,webcam_key
	camuser=str(c).strip()
	whoami=str(w).strip()
	webcam_key=str(k).strip()

def reset_connection():
	global cam_srv, cam_port
	cam_srv=main_server
	cam_port=main_port

def get_last_error(): 
	global last_error
	if not last_error: return None
	ll=""+str(last_error)
	last_error=None
	return ll

def pop_error(somestr):
	global last_error
	last_error=str(somestr).strip()

def dbprint(somestr):
	run_debug=0
	if run_debug:
		print str(somestr)

def get_jasper_out():
	return "/tmp/"+str(camuser)+".bmp"
def get_jasper_in():
	return "/tmp/"+str(camuser)+".jp2"

def get_jasper_cmd():
	return "jasper -f \"/tmp/"+str(camuser)+".jp2\" -F  \"/tmp/"+str(camuser)+".bmp\""

global image_ready
image_ready=0

def get_current_image():
	global image_ready
	return image_ready


def setup_ip():
	global my_ip
	my_ip=str(os.popen("/sbin/ifconfig `netstat -nr | grep '^0\\.0' | tr -s ' ' ' ' | cut -f 8 -d' '` | grep inet | tr -s ' ' ':' | cut -f4 -d:").readline().replace("\n","").strip() )

def get_len_from_char(somechar):
	gg="0x"+binascii.hexlify(somechar)
	return eval(gg)

def set_frame_setter_method(somemeth):
	global frame_setter_method
	frame_setter_method=somemeth

def set_resolution(w,h):
	global prefwidth, prefheight
	if w>0: prefwidth=w
	if h>0: prefheight=h

def yahoo_got_webcam_image(sdata):
	global imbuf, to_read, frame_setter_method, image_ready
	imbuf=imbuf+sdata
	if len(imbuf)==to_read:
		if not camuser:
			imbuf=""
			return
		if frame_setter_method:
			jf=open(get_jasper_in(),"w")
			jf.write(imbuf)
			jf.close()
			threads_enter()
			image_ready=0
			threads_leave()
			try:
				os.remove(get_jasper_out())
			except:
				pass
			jf=os.popen(get_jasper_cmd())
			jf.readlines()
			jf.close()
			if os.path.exists(get_jasper_out()):
				if os.path.getsize(get_jasper_out())>0:
					threads_enter()
					image_ready=1
					#print "im ready"
					threads_leave()
					#threads_enter()
					#frame_setter_method(get_current_image())
					#threads_leave()
		to_read=0
		imbuf=""

def yahoo_get_webcam_data(rxqueue):
	global to_read, data_size
	reason=0
	pos=0
	begin=0
	end=0
	closed=0
	header_len=0	
	connect=0
	rxlen=len(rxqueue)
	packet_type=0
	timestamp=-1
	addit=""
	
	# if we are not reading part of image then read header 
	if not to_read :
		header_len=get_len_from_char(rxqueue[pos])
		pos =pos+1
		packet_type=0

		if (rxlen < header_len):
			return 1

		if (header_len >= 8):
			reason = get_len_from_char(rxqueue[pos])
			pos =pos+1
			# next 2 bytes should always be 05 00 
			pos += 2
			data_size = get_len_from_char(rxqueue[4:8])
			pos += 4
			to_read = data_size
		if (header_len >= 13):
			packet_type = get_len_from_char(rxqueue[pos])
			dbprint( "packet:  "+str([rxqueue]) )
			dbprint( "packet type:  "+str(packet_type) )
			pos =pos+1
			timestamp = get_len_from_char(rxqueue[pos:])
			dbprint( "timestamp:  "+str(timestamp))
			pos += 4
	else:
		packet_type=0x02

	pos = header_len
	begin = pos
	pos += to_read

	if data_size>0:  # read the rest of the packet
		readit=0
		while readit<data_size:
			addit=addit+tcp_recv(data_size-readit)
			readit=len(addit)
		pos += len(addit)
		dbprint(  "add:  "+str(len(addit))+"   data-size:  "+str(data_size))

	#  if it is not an image then make sure we have the whole packet
	if not packet_type == 0x02:
		# print "match:  "+str((pos - begin))+ "  "+str(data_size)+"  "+str(to_read)
		if not ((pos - begin) == data_size):
			to_read = 0
			return 1

	if packet_type == 0x00:
			# add stuff or uploading images here

			if webcam_downloading:
				# timestamp/status field
				# 0 = declined viewing permission 
				# 1 = accepted viewing permission 
				if (timestamp == 0) :
					pop_error(_("The user has denied you viewing permission."))
					tcp_close()
					return 0

	if packet_type ==  0x02:  # image data, addit
			#yahoo_got_webcam_image(rxqueue[begin:])
			yahoo_got_webcam_image(addit)
			return  1
	if packet_type ==  0x05: # response packets when uploading
			# uploading not supported yet
			return 1
	if packet_type ==  0x07:  # connection is closing 
			if reason ==   0x01: #  user closed connection
					pop_error(_("The user has stopped broadcasting."))
					tcp_close()
					return 0
			if reason ==  0x0F:    #  user cancelled permission
					pop_error(_("The user has cancelled viewing permission."))
					tcp_close()
					return 0
	return 1


def get_cam_feed(*args):
	global camuser,whoami,webcam_key, my_ip, quit_called
	setup_ip()
	starter = "\x08\x00\x01\x00"
	packet = ""
	conn_type=0  # connection type: 0-dialup, 1-dsl, 2-T1
	who =webcam_downloading
	if not who:
		starter = "\x0d\x00\x05\x00\x01\x00\x00\x00\x01" 

	if who:
		sdata="<REQIMG>"
		tcp_send(sdata)
		sdata="a=2\r\nc=us\r\ne=21\r\nu="+whoami+"\r\nt="+webcam_key+"\r\ni="+my_ip+"\r\ng="+camuser+"\r\no=w-2-5-1\r\np="+str(conn_type)+"\r\n"

	else :  # uploading
		sdata="<SNDIMG>"
		# we'll deal with this later

	lenstr="\x00\x00\x00%c" % len(sdata)
	packet=starter+lenstr+sdata
	tcp_send(packet)
	while 1:
		if quit_called: return
		time.sleep(0.15)
		sdata=tcp_recv(13)
		if not sdata=="":
			if not yahoo_get_webcam_data(sdata): break	

def get_next_server(*args):
	global quit_called
	tserver=None
	sdata=""
	while 1:
		if quit_called: return
		sdata=tcp_recv(1024)
		if not sdata=="":
			if len(sdata)>3:
				#print str([sdata])
				status=get_len_from_char(sdata[1])
				#print "status:  "+str(status)
				if status== 0:
					if len(sdata)>18:
						tserver=str(sdata[4:18])
						return tserver
				if status== 6:
					pop_error(_("The user does not have their webcam online."))
					return None
				pop_error("Unknown data returned from the webcam server")
				return None
			else:
				pop_error("Unknown data returned (too short).")
				return None

def connect_first_server(*args):
    try:
	global cam_srv, connect_status, quit_called
	reset_connection()
	if not open_tcp(): 
		connect_status=0
		return
	starter = "\x08\x00\x01\x00"
	packet = ""
	who =webcam_downloading
	connect_status=1
	if who:
		sdata="<RVWCFG>"
	else :
		sdata="<RUPCFG>"

	lenstr="\x00\x00\x00%c" % len(sdata)
	packet=sdata
	tcp_send(packet)
	sdata=tcp_recv(1024)
	if who:
		sdata="g="+camuser+"\r\n"
		lenstr="\x00\x00\x00%c" % len(sdata)
		packet=starter+lenstr+sdata
		tcp_send(packet)
	else:
		sdata="g="+camuser+"\r\n"
		lenstr="\x00\x00\x00%c" % len(sdata)
		packet=starter+lenstr+sdata
		tcp_send(packet)
	if quit_called: return
	sdata=tcp_recv(1024)
	#time.sleep(0.0)
	if quit_called: return
	tserver=get_next_server()
	dbprint("Got next server:  "+str(tserver))
	if not tserver:
		tcp_close()
		connect_status=0
		return
	cam_srv=tserver
	if quit_called: return
	if not open_tcp(): 
		connect_status=0
		return
	connect_status=2
	if quit_called: return
	get_cam_feed()
	connect_status=3
    except:
	pass

def start_connection(*args):
	global connect_status, quit_called, to_read
	global data_size, imbuf,image_ready
	if connect_status==1: return
	if connect_status==2: return
	quit_called=0
	to_read=0
	data_size=0
	image_ready=0
	imbuf=""
	#timeout_add(10,connect_first_server )
	ts=Thread(None,connect_first_server)
	ts.start()
	return 0

def stop_connection(*args):
	global connect_status, quit_called
	if connect_status==0: return
	if connect_status==3: return
	quit_called=1
	time.sleep(0.85)
	tcp_close()
	connect_status=3

def open_tcp():
    global cam_srv,cam_port, tcp_socket
    HOST = cam_srv    # The remote host
    PORT = cam_port            # The same port as used by the server

    try:
       for res in socket.getaddrinfo(HOST, PORT, socket.AF_INET, socket.SOCK_STREAM, 6):
    		af, socktype, proto, canonname, sa = res
    		try:
			tcp_socket = socket.socket(af, socktype, proto)
    		except socket.error, msg:
			tcp_socket = None
			continue
    		try:
			tcp_socket.connect(sa)
			if tcp_socket: tcp_socket.setblocking(0)
			else: raise socket.error, _("Network connection failed.")
    		except socket.error, msg:
				try:
					tcp_socket.close()
				except:
					pass
				tcp_socket = None
				dbprint("Socket Error MSG:  "+str(msg) )
				continue
    		break
    except socket.error, msg:
    			dbprint('Could not open TCP socket on host: '+str(msg) )
			pop_error(_('Could not open TCP socket')+' '+str(msg) )
			return 0

    if tcp_socket is None:			
    			dbprint('Could not open TCP socket on host: '+str(HOST)+'  port: '+str(PORT) )
			pop_error(_('Could not open TCP socket')+' - host: '+str(HOST)+'  port: '+str(PORT))
			return 0
    return 1

def tcp_send(data):
	global tcp_socket
	try:
			if tcp_socket:
				hh=tcp_socket.send(data)
				return hh
			return 0
	except:
			return -1
	return -1

def tcp_recv(len):
	global tcp_socket
	try:
			if tcp_socket:
				return tcp_socket.recv(len)
			return ""
	except:
			return ""

def tcp_close():
	global tcp_socket
	if tcp_socket:
			try:
				tcp_socket.close()			
			except:
				pass
			tcp_socket=None


######## GUI STUFF ###############
def to_utf8(somestr):
	unistr = unicode(str(somestr),"iso8859-1")
	utfstr = unistr.encode("utf-8")
	return utfstr

def getBaseDir() :
	if sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]=='': return "."+os.sep
	return sys.argv[0][0:sys.argv[0].rfind(os.sep)+1]

locale_dirs=[getBaseDir()+'/locale','/usr/share/gyach/locale','/usr/X11R6/share/gyach/locale','/usr/gyach/locale','/usr/local/share/gyach/locale']

# gettext locale support - pywebcam
pywebcam_xtext=gettext.NullTranslations()

try:
	# open pyvoice.mo in /usr/share/locale or whatever
	f_gettext=0

	for LOCALE_DIR in locale_dirs:  # open the first catalog we find, try ./locale dir first
		try:
			pywebcam_xtext=gettext.translation("gyach",LOCALE_DIR)
			f_gettext=1
			break
		except:
			pass
	if not f_gettext==1: raise TypeError   # couldnt find a gettext catalog, raise exception, use empty catalog
except:
	pywebcam_xtext=gettext.NullTranslations()

def _(somestr):
	return to_utf8(pywebcam_xtext.gettext(somestr))

def set_window_icon(mywindow,my_xpm, is_stock=0):
	if is_stock==1:
		try:
			tempmen=MenuItem()
			# steal the GTK stock icon for window frame
			mywindow.set_icon(tempmen.render_icon(my_xpm,ICON_SIZE_DIALOG))
			tempmen.destroy()
		except:
			pass
	try:
		mywindow.set_icon(GDK.pixbuf_new_from_xpm_data(my_xpm))
	except:
		pass

# borrowed from my IceWMCP
def loadScaledImage(picon,newheight=40,newwidth=40, is_stock=0, pixmask=0):
    try:		
	if is_stock==1:
		tempmen=MenuItem()
		img=tempmen.render_icon(picon,ICON_SIZE_SMALL_TOOLBAR)
		tempmen.destroy()
	elif str(type(picon))==str(type([])):
		img=GDK.pixbuf_new_from_xpm_data(picon)
	else:
        	img = gtk.gdk.pixbuf_new_from_file(picon)
	if newheight>-1:
        	img2 = img.scale_simple(newheight,newwidth,gtk.gdk.INTERP_BILINEAR)
       		pix,mask = img2.render_pixmap_and_mask()
	else: pix,mask = img.render_pixmap_and_mask()
	if pixmask:
		return [pix,mask]
        icon = gtk.Image()
        icon.set_from_pixmap(pix, mask)
        icon.show()
        return icon
    except:
        return None

dummy_xpm= [
"1 1 2 1",
" 	c None",
".	c #000000",
"."]

dummy_xpm2= [
"1 1 2 1",
" 	c None",
".	c #000088",
"."]

def loadPixmap(picon,windowval=None):
    try:
        p=Image()
	if (picon.startswith("gtk-")) :  # stock icon
		# hmmm, no way to really scale stock icons to a special size
		p.set_from_stock(picon,ICON_SIZE_BUTTON)
	else: p.set_from_file(str(picon).strip())
        p.show_all()
        return p
    except:
        return None

def getImage(im_file,lab_err="") : # GdkPixbuf Image loading
    try:
        myim= gtk.Image()
	if (im_file.startswith("gtk-")) :  # stock icon
		# hmmm, no way to really scale stock icons to a special size
		myim.set_from_stock(im_file,ICON_SIZE_BUTTON)
        else: myim.set_from_file(str(im_file).strip())
        return myim
    except:
        return gtk.Label(str(lab_err))

def diaHide(*args):
	try:
		args[0].get_data("window").destroy()
	except:
		pass

def show_ok_dialog(somemsg):
	WMCLASS="pywebcam"
	WMNAME="PyWebCam"
	pydia=Window(WINDOW_TOPLEVEL)
	pydia.set_wmclass(WMCLASS,WMNAME)
	pydia.set_title("pY! Webcam")
	set_window_icon(pydia, STOCK_DIALOG_INFO ,1)
	pydia.set_position(WIN_POS_CENTER)
	pydia.set_modal(1)
	pybox=VBox(0,0)
	pybox.set_border_width(3)
	pybox.set_spacing(3)
	pybox2=HBox(0,0)
	pybox2.set_border_width(5)
	pybox2.set_spacing(3)	
	img=Image()
	img.set_from_stock(STOCK_DIALOG_INFO,ICON_SIZE_DIALOG)
	pybox2.pack_start(img,0,0,0)
	lab=Label(str(somemsg))
	lab.set_line_wrap(1)
	lab.set_justify(JUSTIFY_LEFT)
	pybox2.pack_start(lab,1,1,3)
	pybox.pack_start(pybox2,1,1,0)
	pybox.pack_start(HSeparator(),0,0,3)
	cbutton=getPixButton (None,STOCK_OK,_("OK"),1, 0)
	Tooltips().set_tip(cbutton, _("OK"))
	pybox.pack_start(cbutton,0,0,2)
	cbutton.connect("clicked", diaHide)
	cbutton.set_data("window",pydia)
	pydia.add(pybox)
	pydia.show_all()
	

def getPixButton (iwin,picon,lab="?",both=0, nobutton=0) :  
        try:
	  if nobutton==0: b=Button()
	  if both==1:
	    h=HBox(0,0)
	    h.set_spacing(4)
	    h.set_border_width(2)
            h.pack_start(getImage(str(picon),to_utf8(lab)),0,0,0)
	    l=Label(str(lab))
	    l.set_alignment(0,0.5)
            h.pack_start(l,1,1,0)
            h.show_all()
	    if nobutton==0: b.add(h)
	    else: b=h
	  else:
	    if nobutton==0: b.add(loadPixmap(str(picon),None))
	    else: b=loadPixmap(str(picon),None)
          b.show_all()        
          return b
        except:
          if nobutton==0: return Button(to_utf8(str(lab)))
	  else: return Label(to_utf8(str(lab)))


class pywebcam:
    def __init__(self) :
	WMCLASS="pywebcam"
	WMNAME="PyWebCam"
	self.version=APP_VERSION
	self.states={
		0:_("Not Connected"),
		1:_("Connecting..."),
		2:_("Connected"),
		3:_("Disconnected"),
				   }
	self.last_state=-1
	self.last_status=""
	self.updating=0
	self.run_anim=1
	self.NEW_PIC=None
	TIPS=Tooltips()
	pycamwin=Window(WINDOW_TOPLEVEL)
	pycamwin.set_wmclass(WMCLASS,WMNAME)
	pycamwin.set_title("pY! Webcam"+": "+str(camuser))
	set_window_icon(pycamwin, STOCK_CONVERT ,1)
	pycamwin.set_position(WIN_POS_CENTER)
	pycamwin.realize()
	self.pycamwin=pycamwin
	mainvbox1=VBox(0,1)
	mainvbox1.set_spacing(1)
	mainvbox=VBox(0,1)
	mainvbox.set_spacing(0)
	mymenubar=MenuBar()
	self.mymenubar=mymenubar

        ag=AccelGroup()
        itemf=ItemFactory(MenuBar, "<main>", ag)
        itemf.create_items([
            # path              key           callback    action#  type
  ("/_"+_("Connection")+"",  "<alt>F",  None,  0,"<Branch>"),
  ("/_"+_("Connection")+"/_"+_("Pause Video"), "<control>P", self.pauseVideo,10,"<CheckItem>"),
  ("/_"+_("Connection")+"/sep", None,None,10,"<Separator>"),
  ("/_"+_("Connection")+"/_"+_("Connect"), "<control>C", self.startVideo,10,"<StockItem>",STOCK_YES),
  ("/_"+_("Connection")+"/_"+_("Disconnect"), "<control>D", self.stopVideo,10,"<StockItem>",STOCK_NO),
  ("/_"+_("Connection")+"/sep", None,None,10,"<Separator>"),
  ("/_"+_("Connection")+"/_"+_("Quit"), "<control>Q", self.doQuit,10,"<StockItem>", STOCK_QUIT),
  ("/_"+_("Help"),  "<alt>H",  None,16, "<LastBranch>"), 
  ("/_"+_("Help")+"/_"+_("About pY! WebCam..."), "F2", self.doAbout,17, "<StockItem>", STOCK_DIALOG_INFO),
        ])

        pycamwin.add_accel_group(ag)
	self.ag=ag
	self.itemf=itemf
	self.prefheight=240
	self.prefwidth=320
	set_resolution(self.prefwidth,self.prefheight)
        mymenubar=itemf.get_widget("<main>")
        mymenubar.show()
	mainvbox1.pack_start(mymenubar,0,0,0)
	mainvbox1.pack_start(mainvbox,1,1,2)
	mainvbox.set_spacing(2)
	mainvbox.set_border_width(1)
	self.frame=Frame()
	self.frame.set_border_width(4)
	self.showDummy1()
	mainvbox1.pack_start(self.frame,0,0,0)
	mainvbox1.pack_start(Label(_("Webcam")+": "+str(camuser)),0,0,0)
	self.status=Label("")
	mainvbox1.pack_start(self.status,0,0,0)
	closeb=getPixButton (None,STOCK_QUIT,_("Close"),1, 0)
	TIPS.set_tip(closeb,_("Close"))
	mainvbox1.pack_start(closeb,0,0,4)
	closeb.connect("clicked", self.doQuit)
	pycamwin.add(mainvbox1)
	set_frame_setter_method(self.set_current_frame)
	pycamwin.connect("destroy", self.doQuit)
	pycamwin.show_all()
	self.run_anim=1
	self.checkmenu=None
	self.entered=0
	self.infile=get_jasper_out()
	self.images=0
	timeout_add(400, self.updateAnimation)
	timeout_add(400, self.updateStatusError)
	self.startVideo()

    def startVideo(self,*args):
	if self.checkmenu:
		self.checkmenu.set_active(0)
	self.run_anim=1
	start_connection()

    def stopVideo(self,*args):
	stop_connection()

    def pauseVideo(self,*args):
	dbprint( "pause")
	if not self.checkmenu:
		self.checkmenu=args[1]	
	if self.checkmenu.get_active():
		self.run_anim=0
	else: self.run_anim=1

    def updateStatusError(self,*args):
	istate=get_connect_status()
	if not istate==self.last_state:
		try:
			self.status.set_text(self.states[istate])
		except:
			pass
		if istate==3:
			self.run_anim=1
			self.setDummy2()
			self.updateAnimation()
		if istate==0:
			self.run_anim=1
			self.setDummy1()
			self.updateAnimation()
		self.last_state=istate
	istatus=get_last_error()
	if istatus:
		istatus=str(istatus)
		if len(istatus)>0:
			if not self.last_status==istatus:
				show_ok_dialog(istatus)
		self.last_status=istatus
	return 1

    def updateAnimation(self,*args):
		dbprint(  "update" )
		if self.run_anim==0: return 1
		threads_enter()
		im_ready=get_current_image()		
		if im_ready:
			# load scaled image
			self.NEW_PIC=loadScaledImage(  
			self.infile, prefwidth , prefheight, 0, 1)
			self.images=self.images+1
		threads_leave()
		dbprint ("new pick:  "+str(self.NEW_PIC)+"  "+str(self.images))
		if not self.NEW_PIC: 
			#threads_leave()
			return 1
		try:
			self.CURRENT_PIC.set_from_pixmap(self.NEW_PIC[0], self.NEW_PIC[1])
			#self.frame.remove(self.CURRENT_PIC)
			#self.CURRENT_PIC.destroy()
			#self.CURRENT_PIC.unmap()
		except:
			pass
		try:
			#self.frame.add(self.NEW_PIC)
			#self.CURRENT_PIC=self.NEW_PIC
			self.NEW_PIC=None
			#self.frame.show_all()
		except:
			pass
		dbprint ("updated")
		#threads_leave()
		return 1

    def set_current_frame(self,someimage):
		self.updating=1
		self.NEW_PIC=someimage
		self.updating=0

    def showDummy1(self, *args) :
	img=loadScaledImage(dummy_xpm2,self.prefwidth,self.prefheight, 0)
	if img:
		self.frame.add(img)
		self.CURRENT_PIC=img
    def setDummy2(self, *args) :
	img=loadScaledImage(dummy_xpm,self.prefwidth,self.prefheight, 0, 1)
	if img:
		self.set_current_frame(img)
    def setDummy1(self, *args) :
	img=loadScaledImage(dummy_xpm2,self.prefwidth,self.prefheight, 0, 1)
	if img:
		self.set_current_frame(img)

    def doQuit(self, *args) :
	self.stopVideo()
	mainquit()

    def doAbout(self, *args) :
	show_ok_dialog("pY! Webcam\n\nCopyright (c) 2004\nErica Andrews\nPhrozenSmoke ['at'] yahoo.com\nhttp://phpaint.sourceforge.net/pyvoicechat/\nLicense: GNU General Public License\npY! Webcam Version:  "+self.version)

def testit(*args):
	ig=0
	global to_read, camuser, image_ready
	while ig< 25:
		camuser="temptest"
		to_read=os.path.getsize("/tmp/lendinky63.jp2")
		sdata=open("/tmp/lendinky63.jp2").read()
		yahoo_got_webcam_image(sdata)
		ig=ig+1
		time.sleep(0.2)
	image_ready=0

def runApp(*args):
	"""
	Main method for launching the application.
	"""
	pywebcam()	
	#  Thread(None,testit).start()
	threads_init()
	threads_enter()
	mainloop()
	threads_leave()

if __name__ == '__main__':
	# print str(sys.argv)
	usage="\n   pY! Webcam\n   Gyach Enhanced External Webcam Viewer\n   Version:  %s\n   License:  GNU General Public License\n   Copyright (c) 2004 Erica Andrews\n   PhrozenSmoke ['at'] yahoo.com\n\n   Usage:\n   pywebcam [who_we_are_viewing] [your_screename] [web_cam_key_from_gyachE]\n\n   You should be running this\n   program in Gyach Enhanced,\n   NOT on the command-line!\n\n" % APP_VERSION
	if len(sys.argv)<4:
		print usage
		sys.exit(0)
	viewwho=sys.argv[1].strip()
	viewme=sys.argv[2].strip()
	viewkey=sys.argv[3].strip()
	badargs=0
	if len(viewwho)==0: badargs=1
	if len(viewkey)==0: badargs=1
	if len(viewme)==0: badargs=1
	if badargs:
		print usage
		sys.exit(0)
	setup_connection(viewwho,viewme,viewkey)
	runApp()	
