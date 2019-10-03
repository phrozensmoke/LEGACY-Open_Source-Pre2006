#! /usr/bin/env python
# -*- coding: ISO-8859-1 -*-

#######################################
#  pY! Voice Chat - pyvoice.py: library
#  
#  Copyright (c) 2003-2006 by Erica Andrews
#  PhrozenSmoke [-AT-] yahoo.com
#  http://phpaint.sourceforge.net/pyvoicechat/
#
#  The first Linux client for Yahoo!
#  voice chat.
#
#  Network, RTP, and sound functionality
#  library.
#  
#  Neither pY! Voice Chat nor its author
#  are in anyway endorsed by or 
#  affiliated with Yahoo! (tm) or 
#  DSP Group's TrueSpeech (tm).
#  Yahoo! (tm) is a registered 
#  trademark of Yahoo! 
#  Communications.  TrueSpeech (tm)
#  is a registered trademark of 
#  DSP Group. The TrueSpeech (tm)
#  codec and format are patented 
#  by DSP Group.
#  
#  This program is distributed free
#  of charge (open source) under the 
#  terms of the GNU General Public 
#  License
#######################################

import pytsp

# below - the GUI must decide between PyGtk-2/PyGtk-2 before 
# importing this module, leave this alone: generic

from gtk import *
import os,socket,sys,string,time,struct,copy,random,binascii
from threading import Thread


def _(somestr):
	""" 
	Used internally. Gettext support 'place-holding' method. Method is replaced 
	by the graphical user interface with a REAL gettext support method.
	"""
	return somestr

global debug_on
debug_on=0   # do we want to print debugging stuff to stdout?

global is_recording, record_sequence, rsound_data, can_record, record_time,mic_test
is_recording=0  # 0=not recording, 1= trying to send sound, but cant, 2= recording okay

# Below is our UDP packet sequence counter for RTP packets
record_sequence=1  # We will always start this number with 1...Yahoo does
global r_starttime
r_starttime=0
rsound_data=""  # our recorded data to be sent, in a 'buffer' kind of thing
record_time=0
can_record=0
mic_test=0

snd_buflimit=2048  # limit bytes in sound receive buffer 
sound_cut1=1568
sound_cut2=1824

global eighty_byte_model
eighty_byte_model=0

global esd_volume, allow_vol_update
esd_volume=0
allow_vol_update=0


def set_debug(dbval):
	""" 
	Used internally by the graphical user interface to toggle debuggin on/off, 
	where dbval=0 or dbval=1.
	"""
	global debug_on
	debug_on=dbval

def dbprint(some_str):
	""" Used internally. Prints debugging information to stdout. """
	if debug_on==1:
		print "[DEBUG]  "+str(some_str)

# stuff for stats
global total_packets,audio_packets,user_stats,error_msg, total_packets_S,audio_packets_S
total_packets=0  # received
audio_packets=0  # received
total_packets_S=0  # sent
audio_packets_S=0  # sent
user_stats={}
error_msg=""

empty_buffer=0

def pop_error(errm):
	""" Used internally. Sets the current error message. """
	global error_msg
	error_msg=str(errm)

def get_pyvoice_error():
	""" 
	Returns a string containing the last error message.  This is monitored 
	by the graphical user interface and shown to the user as-needed.
	"""
	global error_msg
	emsg=str(error_msg)
	error_msg=''
	return emsg

def get_tsp_info():
	""" Used internally. Returns a string containing the PyTSP library information. """
	return pytsp.get_pytsp_info()

def get_esd_info():
	""" Used internally. Returns a string containing the PyESD library information. """
	return pytsp.get_pyesd_info()

def get_tsp_version():
	""" Used internally. Returns a string containing the PyTSP version. """
	return pytsp.get_pytsp_version()

def get_esd_version():
	""" Used internally. Returns a string containing the PyESD version. """
	return pytsp.get_pyesd_version()

def get_esd_sversion():
	""" Used internally. Returns a string containing the PyESound version. """
	return pytsp.get_esd_version()

def get_sound_engine_info():
	"""
	Returns a string containing information about PyTSP, PyESD, and ESound.
	"""
	vstats=[
		"PyTSP",
		get_tsp_info(),
		_("License")+":  GNU General Public License",
		_("PyTSP Version")+":  "+get_tsp_version(),
		_("TrueSpeech Codec")+":  "+str(pytsp.get_codec_location()),
		_("TrueSpeech Codec - DLL")+":  "+str(pytsp.get_codec_dll_location()),
		"\n",
		_("Neither pY! Voice Chat nor its author are in anyway endorsed by or affiliated with DSP Group's TrueSpeech (tm). The TrueSpeech (tm) codec and format are patented by DSP Group."),
		"\n",
		"PyESD",
		get_esd_info(),
		_("License")+":  GNU General Public License",
		_("PyESD Version")+":  "+get_esd_version(),
		"\n",
		"ESound",
		_("Client Name")+":  "+pytsp.get_esd_client_name(),
		_("Host")+":  "+pytsp.get_esd_host(),
		_("License")+":  GNU General Public License",
		_("ESound Version")+":  "+get_esd_sversion(),
	]
	return string.join(vstats,"\n")

def get_pyvoice_stats():
	"""
	Returns a string containing useful statistics about the current pY! Voice Chat session.
	"""
	dus={}
	for ii in get_users_in_room():
		dus[ii]=1
	vstats=[
		_("User")+":  "+chat_user,
		_("Room")+":  "+chat_room,
		_("Room Number")+":  "+chat_serial,
		_("Cookie")+":  "+chat_cookie,
		_("Users In Room")+":  "+str(len(dus)),
		_("Ignored Users")+":  "+str(len(ignored_users)),
		_("On Mute")+":  "+str({0:_("No"),1:_("Yes")}[on_mute]),
		"\n",
		_("Connection State")+":  "+str(chat_connect_status),
		_("Initial Server")+":  "+vchat_server+":"+str(vchat_port),
		_("Redirected to Server")+":  "+vchat_server2+":"+str(vchat_port),
		_("UDP Server")+":  "+vchat_udp_server+":"+str(vchat_port_udp),
		_("TCP/UDP Packets Received")+":  "+str(total_packets),
		_("Audio Packets Received")+":  "+str(audio_packets),
		_("TCP/UDP Packets Sent")+":  "+str(total_packets_S),
		_("Audio Packets Sent")+":  "+str(audio_packets_S),
		_("Last Error")+":  "+str(error_msg),
		_("PyTSP Version")+":  "+get_tsp_version(),
		_("PyESD Version")+":  "+get_esd_version(),
		_("ESound Version")+":  "+get_esd_sversion(),
		_("Last PyTSP Error")+":  "+str(pytsp.get_pytsp_error_msg()),
		_("Last PyESD Error")+":  "+str(pytsp.get_pyesd_error_msg()),
		_("Platform")+":  "+str(sys.platform),
	]
	return string.join(vstats,"\n")

# some control over idiotic behavior
global on_mute
on_mute=0

def set_mute(mvar):
	"""
	Toggles mute on/off, where mvar=0 or mvar=1. For muting the chat room, 
	though voice data will continue to be received, just not played to the sound device.
	"""
	global on_mute
	on_mute=mvar

def get_stats_for_user(mvar):
	"""
	Returns a string containing the number of audio packets sent by the selected 
	user in the voice chat room.
	"""
	global user_stats
	if not user_stats.has_key(mvar): return "0"
	return str(user_stats[mvar])

global ignored_users
ignored_users={}
pyvoice_ignore_file=os.environ['HOME']+"/.pyvoice_ignore"
gyach_ignore_file=os.environ['HOME']+"/.yahoorc/gyach/ignore"

global chat_connect_status
chat_connect_status=0   # 0=disconnected, 1=connecting,  2=fully connected

global sound_spool_started
sound_spool_started=0

global sdivider
sdivider="@"  # for large room lists and sound packets, to find user's nick name

# voice servers: 66.218.70.32-66.218.70.43
# list of voice servers at: http://vc.yahoo.com:5001/ 

# Working start servers:
# 66.218.70.39:5001, 66.218.70.38:5001, 66.218.70.41:5001,
# 66.218.70.37:5001  66.218.70.40:5001, 
#  ** 66.218.70.37:5001

# voice chat tcp tends to use port 5001, while udp tends to use port 5000
global vchat_server

vchat_server="66.218.70.37"   #66.218.70.40, 66.218.70.37:5001
vchat_port=5001   # 5001  - 5000 usually returns a 'rejected' notice 
vchat_port_udp=5000

global tcp_not_udp, no_auto_server_change
tcp_not_udp=0   # if true, only tcp connections will be used
no_auto_server_change=0  # if true, we will not automatically change voice rooms as needed


## what happens is that we connect to our initial 'vchat_server', send the first login 
#  string, then Yahoo redirects us to 2nd voice chat server.  We close the connection
# to our first server, connect to the server we were redirected to, send the 
# 2nd 'confirmation' login string, and continue our TCP activities.  It is THIS 2nd 
# server that will be used as our UDP server (for port 5000)

global vchat_server2, vchat_udp_server   # will get these later
vchat_server2=''
vchat_udp_server=''

# Basically, here's how it works: We send our first login string to our selected 
# 'vchat_server', the Yahoo server responds by redirecting us to another voice 
# chat server by sending us a string with that contains a 4-byte hex string
# that represents an IP address

def create_ip_from_hex(somes):
	"""
	Creates an IP address from a 4-byte hex string.  This
	method determines the IP address of the server Yahoo
	is telling us to forward our connection to.  This method
	converts strings like '\\xd8mv\\x91' to '216.109.118.145', 
	'B\\xdaF'' to '66.218.70.39', etc.  This method takes 
	one argument: A 4-byte hex string.  This method is called by
	'parse_server_redirection' to find the redirected IP address.
	"""
	if len(somes)<4:
		dbprint("Wrong string length submitted in 'create_ip_from_hex': "+str(len(somes)) +"  "+str([somes]) )
		return vchat_server
	somestr=binascii.hexlify(somes[:4])  # first 4 bytes only
	crIP=""
	g="0x"+somestr[0:2]
	crIP=crIP+str(eval(g))+"."
	g="0x"+somestr[2:4]
	crIP=crIP+str(eval(g))+"."
	g="0x"+somestr[4:6]
	crIP=crIP+str(eval(g))+"."
	g="0x"+somestr[6:8]
	crIP=crIP+str(eval(g))
	return crIP


def get_vchat_server_list():
	"""
	This method returns a list of known voice chat 
	servers to be listed in the Graphical User Interface for
	user selection.  This method takes no arguments 
	and returns a python list of IP addresses.
	"""
	vchat_srv_list = [
	"66.218.70.37",  # v6.vc.scd.yahoo.com
	"66.218.70.38",
	"66.218.70.39",
	"66.218.70.40",
	"66.218.70.41",
	"66.218.70.42",
	"66.218.70.43",
	"66.218.70.44",  
	"66.218.70.45", 
	"66.218.70.46",  
	"66.218.70.47", 
	"66.218.70.32",
	"66.218.70.33",
	"66.218.70.34",
	"66.218.70.35",
	"66.218.70.36",
	"216.109.118.144",   # v1.vc.dcn.yahoo.com
	"216.109.118.145",   # v2.vc.dcn.yahoo.com
	"216.109.118.146",
	"216.109.118.147", 
	"216.109.118.148", 
	"216.109.118.149", 
	"216.109.118.150", 
	"216.109.118.151", 
	"216.109.118.152", 
	#  "216.109.118.153",  # rsvd-153.vc.dcn.yahoo.com, apparently 'reserved'
	#  "216.109.118.154",  # rsvd-154.vc.dcn.yahoo.com, apparently 'reserved'
	#  "216.109.118.155",  # rsvd-155.vc.dcn.yahoo.com, apparently 'reserved'
	#  "216.109.118.156",  # rsvd-156.vc.dcn.yahoo.com, apparently 'reserved'
	#  "216.109.118.157",  # rsvd-157.vc.dcn.yahoo.com, apparently 'reserved'
	#  "216.109.118.158",  # rsvd-158.vc.dcn.yahoo.com, apparently 'reserved'
	"216.109.118.159", 

	"68.142.226.64",  # v1.vc.re2.yahoo.com
	"68.142.226.65",
	"68.142.226.66",
	"68.142.226.67",
	"68.142.226.68",
	"68.142.226.69",  # v6.vc.re2.yahoo.com
	"68.142.226.70",
	"68.142.226.71",
	"68.142.226.72",  # v9.vc.re2.yahoo.com
	"68.142.226.73",  # vc1.vc.vip.re2.yahoo.com
	]
	return vchat_srv_list 



## OUR LOGIN DATA ##

global chat_user, chat_room, chat_cookie, chat_serial

chat_user=""
chat_room=""
chat_cookie=""
chat_serial=""

#chat_cookie="6wB_.ArnJatIDY2sxrvKldFPlfO.QEPyM-"
#chat_serial="1600084740"



# The 3-8 char 'nickname' the voice chat server assigns to our screename
# It appears in a 336-length packet looking like:
# '�A^H^F!aCA!Y^', and the nick is between the '�A^H' and the 'CA!Y^'
# So, we get assigned a ridiculous nick like '^F!a'

global chat_name_nick
chat_name_nick=""

# other users' voice chat nicknames
chat_name_nicks={}

global who_is_talking,who_is_talking_expire
who_is_talking=""
who_is_talking_expire=time.time()  # make sure we don't keep 'stale' talking names


global tcp_socket, tcp_socket2, tcp_socket3
tcp_socket=None
tcp_socket2=None  # for our server redirect
tcp_socket3=None  # for our server redirect
global udp_socket
udp_socket=None
global poll_connection
poll_connection=1
global tcp_data, tcp_data2
tcp_data=None
tcp_data2=None

global sound_data
sound_data=""   # holds TSP sounds in like a 'spool' structure

global last_login_string
last_login_string=""



letter_len_map={}   # a-z based 'size' attribs, used mainly for screen names 
letter_len_map_num={}
letter_len=1

for ii in string.letters[:26]:
	letter_len_map[letter_len]=ii.upper()
	letter_len_map_num[ii.upper()]=letter_len
	letter_len=letter_len+1

hex_len_map={}  # hex numbers 0-255
hex_len_map_num={}
letter_len=1

while letter_len<256:
		hex_len_map[letter_len]="%c" % letter_len;
		hex_len_map_num["%c" % letter_len]=letter_len
		letter_len=letter_len+1




def open_tcp(some_srv=vchat_server,which_socket=0):
	"""
	Used internally to open a TCP socket to a Yahoo voice 
	server on port 5001. This method should never raise an exception.
	Returns 1 if successful, 0 on failure (error messages are saved for retrieval if 
	failure occurs.)
	"""
	global chat_connect_status, vchat_server2, vchat_udp_server	
	HOST = some_srv    # The remote host
	PORT = vchat_port            # The same port as used by the server
	global tcp_socket, tcp_socket2, tcp_socket3
	if which_socket==0: 
		tcp_socket = None
		vchat_server2=some_srv
		vchat_udp_server=some_srv
	if which_socket==1: tcp_socket2 = None
	if which_socket==2:
		tcp_socket3=None
		PORT=5000
	#print "HOST:  "+str(HOST)+"   "+str(PORT)
	for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM):
    		af, socktype, proto, canonname, sa = res
    		try:
			if which_socket==0: tcp_socket = socket.socket(af, socktype, proto)
			if which_socket==1: tcp_socket2 = socket.socket(af, socktype, proto)
			if which_socket==2: tcp_socket3 = socket.socket(af, socktype, proto)
    		except socket.error, msg:
			if which_socket==0: tcp_socket = None
			if which_socket==1: tcp_socket2 = None
			if which_socket==2: tcp_socket3 = None
			continue
    		try:
			if which_socket==0:
				tcp_socket.connect(sa)
				if tcp_socket: tcp_socket.setblocking(0)
				else: raise socket.error, _("Network connection failed.")
			if which_socket==1:
				tcp_socket2.connect(sa)
				if tcp_socket2:  tcp_socket2.setblocking(0)
				else: raise socket.error, _("Network connection failed.")
			if which_socket==2:
				tcp_socket3.connect(sa)
				if tcp_socket3:  tcp_socket3.setblocking(0)
				else: raise socket.error, _("Network connection failed.")
    		except socket.error, msg:
			if which_socket==0:
				try:
					tcp_socket.close()
				except:
					pass
				tcp_socket = None
			if which_socket==1:
				try:
					tcp_socket2.close()
				except:
					pass
				tcp_socket2 = None
			if which_socket==2:
				try:
					tcp_socket3.close()
				except:
					pass
				tcp_socket3 = None
			dbprint("Socket Error MSG:  "+str(msg) )
			continue
    		break
	if which_socket==0:
		if tcp_socket is None:			
			chat_connect_status=0
    			dbprint('Could not open TCP socket on host: '+str(HOST)+'  port: '+str(PORT) )
			pop_error(_('Could not open TCP socket')+' - host: '+str(HOST)+'  port: '+str(PORT))
			return 0
	if which_socket==1:
		if tcp_socket2 is None:			
			chat_connect_status=0
    			dbprint('Could not open TCP socket on host: '+str(HOST)+'  port: '+str(PORT) )
			pop_error(_('Could not open TCP socket')+' - host: '+str(HOST)+'  port: '+str(PORT))
			return 0
	if which_socket==2:
		if tcp_socket3 is None:			
			chat_connect_status=0
    			dbprint('Could not open TCP socket on host: '+str(HOST)+'  port: '+str(PORT) )
			pop_error(_('Could not open TCP socket')+' - host: '+str(HOST)+'  port: '+str(PORT))
			return 0
	return 1



def tcp_send(data,which_socket=0,with_size=0):
	"""
	Used internally to send data to a TCP socket. 
	This method should never raise an exception.  Returns an 'int' - the 
	length of the data sent, or -1 if sending fails.
	"""
	global tcp_socket, tcp_socket2, total_packets_S, tcp_socket3
	if which_socket==0:
		try:
			if tcp_socket:
        			if with_size==1: tcp_socket.send(struct.pack("<H", len(data)))
				hh=tcp_socket.send(data)				
				total_packets_S=total_packets_S+1
				return hh
			return 0
		except:
			return -1
	if which_socket==1:
		try:
			if tcp_socket2:
        			if with_size==1: tcp_socket2.send(struct.pack("<H", len(data)))
				hh=tcp_socket2.send(data)
				total_packets_S=total_packets_S+1
				return hh
			return 0
		except:
			return -1
	if which_socket==2:
		try:
			if tcp_socket3:
        			data="\x00"+chr(len(data)+4)+"\x00\x00"+data
				#print "TCP data:  "+str([data])
				hh=tcp_socket3.send(data)
				total_packets_S=total_packets_S+1
				return hh
			return 0
		except:
			return -1
	return -1

def tcp_recv(len,which_socket=0):
	"""
	Used internally to receive data from a TCP socket.  This method should never 
	raise an exception.  This method may return a string of data or an empty string ''.
	"""
	global tcp_socket, tcp_socket2, tcp_socket3
	if which_socket==0:
		try:
			if tcp_socket:
				return tcp_socket.recv(len)
			return ""
		except:
			return ""
	if which_socket==1:
		try:
			if tcp_socket2:
				return tcp_socket2.recv(len)
			return ""
		except:
			return ""
	if which_socket==2:
		try:
			if tcp_socket3:
				return tcp_socket3.recv(len)
			return ""
		except:
			return ""

def tcp_close(which_socket=0):
	"""
	Used internally to close a TCP socket. This method should never raise an exception.
	"""
	global tcp_socket, tcp_socket2, tcp_socket3
	if which_socket==0:
		if tcp_socket:
			try:
				tcp_socket.close()			
			except:
				pass
			tcp_socket=None
	if which_socket==1:
		if tcp_socket2:
			try:
				tcp_socket2.close()			
			except:
				pass
			tcp_socket2=None
	if which_socket==2:
		if tcp_socket3:
			try:
				tcp_socket3.close()			
			except:
				pass
			tcp_socket3=None




def open_udp(*args):
	"""
	Used internally to open a UDP socket to a Yahoo voice 
	server on port 5000. This method should never raise an exception.
	Returns 1 if successful, 0 on failure (error messages are saved for retrieval if 
	failure occurs.)
	"""
	if tcp_not_udp==1: return open_tcp( vchat_udp_server,2)
	global udp_socket
	udp_socket = None
    	try:
			udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
			udp_socket.setblocking(0)
    	except socket.error, msg:
			udp_socket = None
			dbprint("UDP Socket error MSG:  "+str(msg))
	if udp_socket is None:
    		dbprint('Could not open UDP socket' )
		pop_error(_('Could not open UDP socket') )
		return 0
	return 1

def udp_close():
	"""
	Used internally to close a UDP socket. This method should never raise an exception.
	"""
	if tcp_not_udp==1: 
		tcp_close(2)
		return
	global udp_socket
	if udp_socket:
		try:
			global chat_connect_status
			chat_connect_status=0
			udp_socket.close()
		except:
			pass
		udp_socket=None

def udp_recv(len):
	"""
	Used internally to receive data from a UDP socket.  Generally used to receive 
	RTP data over Yahoo's RTP-over-UDP connections. This method should never 
	raise an exception.  This method may return a tuple or an empty string ''.
	"""
	if tcp_not_udp==1: return [tcp_recv(len,2),""]
	global udp_socket
	try:
		if udp_socket:
			return udp_socket.recvfrom(len)
		return ""
	except:
		return ""


def udp_send(data, phost=None, pport=None):
	"""
	Used internally to send data to a UDP socket. Generally this method is 
	used to send RTP data to Yahoo's RTP-over-UDP connections.
	This method should never raise an exception.  Returns an 'int' - the 
	length of the data sent, or -1 if sending fails.
	"""
	if tcp_not_udp==1: return tcp_send(data,2)
	global udp_socket
	try:
		if udp_socket:

			HOST = vchat_udp_server    # The remote host
			PORT = vchat_port_udp            # The same port as used by the server
			if not phost==None:
				if not pport==None:  # send to a specific port and host
					HOST=phost
					PORT=pport
			for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_DGRAM):
    				af, socktype, proto, canonname, sa = res
				#print "len-data:  "+str(len(data))
				hh=udp_socket.sendto(data,sa)
				#print "UDP sent length:  "+str(hh)
				global total_packets_S
				total_packets_S=total_packets_S+1
				return hh
		return -1
	except:
		return -1


def get_configuration_file():
	"""
	Used internally as a central method for returning the name of the PyVoice library
	configuration file, usually of the form '/tmp/pyvoice_chat_start_[unix-username]'.  
	This is the same configuration file that must be written to 
	by the Yahoo chat client (probably GYach Enhanced).  If written to correctly 
	by the Yahoo chat client, the file should be formed as follows:
		[CHAT USER NAME]
		[CHAT ROOM NAME]
		[CHAT ROOM SERIAL NUMBER]
		[CHAT ROOM COOKIE]
	"""
	cfname="/tmp/pyvoice_chat_start_"
	if os.environ.has_key("USER"):
		cfname=cfname+os.environ['USER']
	else: cfname=cfname+"user"
	return cfname


def read_config():
	"""
	Used internally to read the voice chat configuraton file created by GYach Enhanced
	when we enter a new room that allows voice chat.
	"""
	global chat_user, chat_room, chat_cookie, chat_serial
	if no_auto_server_change==1: 
		if len(chat_room)>1: return
	cfname=get_configuration_file()
	try:
		f=open(cfname)
		flist=f.read().split("\n")
		f.close()
		if len(flist)>3:
			chat_user=flist[0].strip().lower()   # lower-case usernames seem to work most consistently
			chat_room=flist[1].strip()
			chat_cookie=flist[2].strip()
			chat_serial=flist[3].strip()
			dbprint("Config: "+chat_user+"  "+chat_room+"  "+chat_cookie+"  "+chat_serial)
	except:	
		dbprint("Could not read config file:  "+cfname)
		pop_error(_("Could not read config file")+":  "+cfname)



def get_requested_chat_room():
	"""
	Used internally by the Graphical User Interface to compare the current 
	voice chat room we are in to the chat room the Yahoo! chat client 
	(probably GYach Enhanced) is in.  If the current room does not match 
	the room that the Yahoo! chat client is in, the Graphical User Interface 
	stops the current voice chat connection and reconnects to the right 
	voice chat room by calling 'stopPyVoice()' followed by 'startPyVoice()'.
	This method returns a string and is polled at regular intervals by the 
	user interface.
	"""	
	global chat_room
	if no_auto_server_change==1: return chat_room
	req_room=""
	cfname=get_configuration_file()
	try:
		f=open(cfname)
		flist=f.read().split("\n")
		f.close()
		if len(flist)>3:
			req_room=flist[1].strip()
			dbprint("get_requested_chat_room:  Config: "+req_room)
	except:	
		dbprint("get_requested_chat_room:  Could not read config file:  "+cfname)
	return req_room



def tcpPoll(which_socket=0):  # general purpose method for polling tcp sockets
	"""
	Used internally to poll a TCP socket for incoming data.  Accessed by internal
	TCP reading Threads.
	"""
	global poll_connection,tcp_socket,tcp_socket2
	while poll_connection==1:
			if poll_connection==0: break
			if which_socket==0:
				if not tcp_socket: break
			if which_socket==1:
				if not tcp_socket2: break
			data=tcp_recv(2000,which_socket)
			global tcp_data			
			if not data=='': 
					tcp_data=data
					try:
						processTCPData(which_socket)
					except:
						pass
			else: 
				try:
					time.sleep(0.07)  # very short sleep to be nice to CPU
				except:
					pass
			if poll_connection==0: break	
			if which_socket==0:
				if not tcp_socket: break
			if which_socket==1:
				if not tcp_socket2: break


def tcpThreadRead(*args):  # vchat_server handling
	"""
	Used internally to provide the Thread functionality for handling our very first 
	TCP connection to a Yahoo! voice server.
	"""
	global vchat_server
	time.sleep(1)
	if open_tcp(vchat_server):
		global chat_connect_status
		chat_connect_status=1
		time.sleep(0.2)
		if sendTCPLogin()==-1:
			dbprint("Login-1 failed")
			pop_error(_("Could not login to voice chat server"+"  [TCP]"))
			tcp_close(0)
			return 0
		tcpPoll(0)
		#tcp_close(0)
	return 0


def tcpThreadRead2(*args):  
	"""
	Used internally to handle our voice chat server redirection, when we first attempt 
	to login to one TCP server and Yahoo! forwards us to another TCP voice chat server.
	Closes old TCP connections and provides the Thread functionality for handling 
	the server redirection.
	"""
	global vchat_server2, last_login_string
	global poll_connection
	poll_connection=1
	tcp_close(1)
	tcp_close(0)
	dbprint("Server redirect thread started ["+vchat_server2+"]")
	if open_tcp(vchat_server2,1):
		time.sleep(0.2)
		dbprint("last_login:  "+str([last_login_string]))
		if not tcp_send(last_login_string,1) > 0:  # send 2nd login string immediately			
			dbprint("Server Jump/ Login-2 failed")
			pop_error(_("Voice chat server redirection failed"))
			tcp_close(1)
			return 0		
		tcpPoll(1)
		tcp_close(1)
	return 0


def do_server_redirect():  
	"""
	Used internally to handle our voice chat server redirection, when we first attempt 
	to login to one TCP server and Yahoo! forwards us to another TCP voice chat server.
	"""
	global vchat_server2	
	if not vchat_server2=='':
		# start new thread for new tcp socket
		ts=Thread(None,tcpThreadRead2)  
		ts.start()
	else:
		dbprint( "Server redirect called, but no server to redirect to" )


def get_y_exe(): 
	"""
		The method 'get_y_exe' is used for creating random, appropriate 'exe' 
		values to make it harder for Yahoo! to block us as a 'foreign' chat 
		application (if they ever decide to do that.), used internally.
		example,  known to work:  YAHJVOX
	"""
	let=string.letters[:26]
	ypag="YAH"+let[random.randrange(25)]+let[random.randrange(25)]+let[random.randrange(25)]+let[random.randrange(25)]
	return ypag.upper()


def get_yah_sig():
	""" 
	Used interally to return a random 'application signature' to Yahoo! during 
	the login process of 'send_pager_auth'.
	"""
	sigl=[
	"b6ee597e4f61f861f5beaa04",
	"68b9f0cd4f61f861f5beaa04",
	"0798eb464f61f861f5beaa04",
	"3386b4c04f61f861f5beaa04",
	"a178e2314f61f861f5beaa04",
	"3e96fec14f61f861f5beaa04",
	"2d4857f44f61f861f5beaa04",
	"2c0283074f61f861f5beaa04",
	"b03a9ddd4f61f861f5beaa04",
		]
	return sigl[random.randrange(len(sigl)-1)]


def get_rand_sound_card():
	""" 
	Used interally to return a random sound card name to Yahoo's servers.
	"""
	sigl=[
	"ESS AudioDrive (1)",
	"SB",
	"SBAwe",
	"ESS Maestro3",
	"Via AudioDrive(1)",
	"AMD InterWave",
	"SB 16",
	"Avance Logic ALS1X0",
	"ESound",
	"S3 SonicVibes",
	"Trident WaveDX",
	"ESS 1869",
	"SiS SI7018",
	"ESS 1688",
	"Aztech AZT2320", 
		]
	return sigl[random.randrange(len(sigl)-1)]


def sendTCPLogout():
	""" Used internally for sending the 'logout' string when we ask to disconnect from the voice server

	     The 'logout' packet is always exactly 60 bytes and is pretty straight forward.
	      Examples:

		\\x00\\x3c\\x00\\x00\\x81\\xc9\\x000\\x07=f\\x0b\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x81\\xcb\\x00\\x08\\x07=f\\x0b

		\\x00\\x3c\\x00\\x00\\x81\\xc9\\x000\\x06#2\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x81\\xcb\\x00\\x08\\x06#2\\x01
	"""
	global chat_name_nick
	logout_header="\x00<\x00\x00\x81\xc9\x000"+chat_name_nick	
	logout_tail="\x81\xcb\x00\x08"+chat_name_nick
	filler="\x00"*( 60- len(logout_header) - len(logout_tail))
	logout=logout_header+filler+logout_tail
	dbprint("logout:  "+str([logout]))
	tcp_send(logout,1)


def send_pager_auth():
	"""
		Used internally.  During the login process Yahoo asks us to provide information
		about our Yahoo! chat client and our audio setup.  This method provides that 
		information to the Yahoo TCP connection. The name of our application is the 
		'exe' value we send.  
	
		Strangely enough - the 'exe' value must be ANYTHING but 'YPAGER'. 
		I believe users of YahElite and Y!mLite report this same problem. The method 
		'get_y_exe' is used for creating random, appropriate 'exe' values to make it 
		harder for Yahoo! to block us as a 'foreign' chat application (if they 
		ever decide to do that.)
	"""
	global chat_name_nick
	sndcrd=get_rand_sound_card()
	pauth1="\x00\x00\x81\xc9\x000"+chat_name_nick+"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\xcc\x00"

	# temporary, fill in later... holds the length of the packet from the 
	# '\x80\xcc\x00' to the end of the very end of packet (including padding) and 
	# pauthlen1 itself.
	pauthlen1="\xc0"  

	pauth11="\x00\x00\x00\x00CA!Y\x00\x07\x00\x00\x00\x00\x00\x00\x0b"

	global chat_user

	#pauth3="exe="+get_y_exe()+"&sig="+get_yah_sig()+"&app=mc(5, 6, 0, 1358)&u="+chat_user+"&ia=us&lib=yacscom(45)&in=3,1,104,5.0,ESS AudioDrive (1)&out=3,1,104,5.0,ESS AudioDrive (1)"

	pauth3="exe=YPAGER&sig="+get_yah_sig()+"&app=mc(6,0,0,1671)&u="+chat_user+"&ia=us&lib=yacscom(45)&in=3,1,104,5.0,"+sndcrd+"&out=3,1,104,5.0,"+sndcrd
	if len(pauth3)>254:
		pauth3="exe=YPAGER&sig="+get_yah_sig()+"&app=mc(6,0,0,1671)&u="+chat_user+"&ia=us&lib=yacscom(45)&in=3,1,104,5.0,SB&out=3,1,104,5.0,SB"
	pauth2=str(get_hex_num_len(len(pauth3)))	
	# The '2' below is for 'preauth' a few lines down
	total_len=len(pauth3) +len(pauth2) + len(pauth1) +len(pauthlen1)+len(pauth11) +2
	padding="\x00" * (4-(total_len%4))
	#padding=get_padding(4-(total_len%4))
	# The '4' below is for '\x80\xcc\x00'+pauthlen1
	cay_len=len(padding)+len(pauth11)+len(pauth3)+len(pauth2)+4
	pauthlen1="%c" % cay_len
	total_len=total_len+len(padding)
	dbprint("cay-len: "+str(cay_len))
	dbprint("total:  "+str(total_len))
	#preauth="\x00"+str(get_hex_num_len(total_len))
	preauth=struct.pack("!h",total_len)  # will be 2 bytes
	pauth=preauth+pauth1+pauthlen1+pauth11+pauth2+pauth3+padding

	dbprint("pauth length:  "+str(len(pauth)))
	dbprint("pauth:  "+str([pauth]))
	tcp_send(pauth,1)


def get_chat_status():
	"""
		Gets the Yahoo! voice chat connection status:
			0=not connected at all
			1=connecting/logging in
			2=full connected
	"""
	global chat_connect_status
	return chat_connect_status


#################################################
#					TCP FUNCTIONALITY METHODS
#################################################

def parse_server_redirection(data):
			"""
			Internal method called when Yahoo's TCP server sends us a packet redirecting us 
			to another TCP voice chat server.  This method parses the packet for the 1 special
			character that is mapped to a Yahoo TCP server IP address.  If the redirection 
			is unknown or appear bogus, the connection is stopped.  Otherwise, 
			a new connection is made the the TCP server we have been forwarded to.
			Sometimes, we can be redirected to up to 3-4 servers, apparently for Yahoo's 
			load-balancing attempts.  This method calls 'do_server_redirect' to make a new 
			connection to the TCP server we have been redirected to.  This method 
			calls the 'create_ip_from_hex' method to convert a 4-byte hex string 
			provided by Yahoo into the IP address we are forwared to.
			"""
			global chat_connect_status
			global tcp_data, poll_connection, chat_room, chat_serial
			rstarter="ch/"  # regular chat room
			cserial=chat_serial
			if chat_serial=="[PYCONFERENCE]":   # PM or priv. conference
				cserial="\x00"
				rstarter="me/"
			rdirect=data[data.find(rstarter+chat_room)-10:data.find(rstarter+chat_room)]
			dbprint("rdirect string:  "+str([rdirect]))

			if data.find(cserial)> -1:
				strstart=rdirect.find("\x00\x00\x08\x04")
				strender=rdirect.rfind("\x03")

				if (strstart<0): strstart=rdirect.find("\x00\x01\x08\x04")
				if (strstart<0): strstart=rdirect.find("\x08\x04")

				if (strstart<0):
					dbprint("Server redirection START string not found!")
				if (strender<0):
					dbprint("Server redirection END string not found!")
				if (strender<strstart):
					dbprint("Server redirection END string appears before START string!")
					strender=strstart+8

				if strstart > -1:  # we have a server redirect		
						global vchat_server2, vchat_udp_server
						rdirstr=rdirect[strstart+4:strender]
						dbprint("Redirect server string chunk:  "+str([rdirstr]))
						vchat_server2=create_ip_from_hex(rdirstr)
						vchat_udp_server=vchat_server2
						dbprint("Redirected to server: "+str(vchat_server2))
						do_server_redirect()
						return 0
				else: 
					poll_connection=0  # force disconnect
					dbprint("No rdirect")
			else : 
				poll_connection=0  # force disconnect
				dbprint("No chat serial")



def parse_user_leave(data):
					"""
					Internal method that parses TCP packets sent from Yahoo informing us that a user 
					has left the voice chat room.  The packet will provide the 4-byte synchronization source 
					ID of the user who has left.  This method takes care of removing the exiting user from 
					the list of chat room members.  This method does NOT clear 'ignored' users from the 
					ignore list.

					Example Yahoo TCP 'user has left' packet:
					\\x00T\\x00\\x00\\x81\\xc8\\x00D\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x82\\xcb\\x00\\x0c\\x01\\x00\\xda\\x14\\x81\\x00\\xda\\x14

					The 'gone' ID comes right after the '\\x82\\xcb\\x00\\x0c', 
					So this 'gone' ID is '\\x01\\x00\\xda\\x14'

					"""
					global chat_name_nicks, chat_name_nick
					data=data[data.find("\x82\xcb")+4:]
					gone=data[:4]
					dbprint("Got GONE string:  ["+str([gone])+"]")
					dbprint ("Depart:  "+str([gone]) )
					dbprint ("Div: "+str([sdivider]) )
					dbprint ("data:  "+str([data]))
					if chat_name_nicks.has_key(gone):
						dbprint(" *User '"+chat_name_nicks[gone]+"' has left the voice room.*")
						del chat_name_nicks[gone]
						return 1
					if chat_name_nicks.has_key(gone+sdivider):
						dbprint(" *User '"+chat_name_nicks[gone+sdivider]+"' has left the voice room.*")
						del chat_name_nicks[gone+sdivider]
					if chat_name_nicks.has_key(sdivider+gone):
						dbprint(" *User '"+chat_name_nicks[sdivider+gone]+"' has left the voice room.*")
						del chat_name_nicks[sdivider+gone]
						return 1
					
					if gone==chat_name_nick:  # we got booted? or asked to disconnect
						if tcp_not_udp==0:
							pop_error(_("You have been disconnected from the Yahoo! voice chat server."))
							stopPyVoice()



def parse_synch_id(data):
					"""
					Internal method for parsing the 'chat_name_nick' (the RTP synchronization source 
					ID) we are assigned from Yahoo's TCP server.  This 4-byte ID is needed for 
					connecting and communicated via RTP over Yahoo's UDP server.  
					"""
					global chat_name_nick
					dbprint("got nick string")
					cnick=data[data.find("\x80\xcc\x01\x08")+4:data.find("CA!Y")]
					dbprint("Got chat nick [cnick]:  "+str([cnick])+"  len: "+str(len(cnick)))
					chat_name_nick=cnick
					if not chat_name_nick=='':
						ts=Thread(None,udpThreadRead)  # start new thread for new udp socket
						ts.start()




def parse_member_list(data):
				"""
					Internal method for parsing a list of members in the current voice chat room. 
					This method also parse new, single-user 'join' packets, adding all newly found 
					users to the list of room members. 

					Here's part of a sample chat list:
					\\x00\\x00\\x00\\x00\\x9d+\\x14\\x01\\x11dizstudrocczfemz@\\x00\\x00\\x9d9\\x04\\x01\\x0eoooooooh_lila@\\x00\\x00\\x00\\x00\\x00\\x9d;\\xe4\\x01\\x0ccharlavoz03

					Voice chat room 'join' packets let us know that we are connected to voice chat 
					and have 'list' of voice chat users.

					For private conferences and PM conferences, the Yahoo! usernames in the member 
					list are of the structure:  'username@123.60.20.5:5000'.
				"""
				dbprint("Got voice chat room member list...parsing.")
				global chat_connect_status, ignored_users
				global tcp_data, poll_connection, sdivider
				global total_packets
				global chat_name_nick
				chat_connect_status=2
				divider="@"
				#print str([data])
				if data.find("\x81\xca\x00")>-1:  # single user join
					# so far I've seen Yahoo send '@' separated lists of members and '\t'
					# separated lists of members....also lists containing one member on a 
					# 'join'...the 'separator' may change from day to day
					vclist=data[data.find("\x81\xca\x00")+3:]
					if len(vclist)>1: divider=vclist[1]  # 2nd byte
					if len(vclist)>1: vclist=vclist[1:]
					#print "divider:  "+str([divider])
				else:   # big room list
					vclist=data[data.find("\xca")+2:]
					if len(vclist)>1: divider=vclist[1]  # 2nd byte
					#if vclist.count("@")>vclist.count(divider): divider="@"
					sdivider=divider
					if len(vclist)>1: vclist=vclist[1:]
					#print "divider:  "+str([divider])

				names=vclist.split("\x00"+divider)
				# sometimes Yahoo uses more than one name divider, which
				# could cause us to miss names in the list...the work around is 
				# to re-parse with nulls...this may place duplicates in the 
				# chat_name_nicks dict, but dups won't show in the GUI
				names=names+vclist.split("\x00")
				global user_stats
				allowed=string.letters[:26]+string.letters[:26].upper()+string.digits+"_-"
				dbprint("name divider:  "+str([divider]))
				reactivate_igg={}  # for reactivating the 'ignore' for already ignored users
				for ii in names:
					ii=ii.replace("\x00\x01","")
					if len(ii)<3: continue
					if ii.find("\x01")==-1: continue
					if ii.endswith("\x01"): continue
					nick=ii[:ii.rfind("\x01")]
					if len(nick)>4: nick=nick[len(nick)-4:]
					namelen=get_hex_len(ii[ii.rfind("\x01")+1])
					name=ii[ii.rfind("\x01")+2:]
					
					name=name[:namelen].replace("\x00","")
					if allowed.find(name[len(name)-1]) ==-1: name=name[:len(name)-1]

					if len(name)>1:
						if len(nick)>1:
							if len(nick)==3:  nick=divider+nick
							if len(nick)==4:  # all source synchronization ID's are 4 bytes
								chat_name_nicks[nick]=name
								if ignored_users.has_key(name):
									reactivate_igg[nick]=1  #schedule an ignored user to be re-ignored
								#dbprint ("Nick:   "+str([nick]) )
								if not user_stats.has_key(name): 
									user_stats[name]=0
							#else: print "strange nick:   "+str([nick])
				dbprint("Chat member list:\n\n"+str(chat_name_nicks)+"\n")

				# now, before we start playing sound, let's re-add people to 'ignore' who we already had on 
				# ignore, since their 'chat_name_nick' or synchronization source ID will have changed if they 
				# left the voice room and re-entered, and we must re-ignore all annoying people if WE 
				# left the voice room and then returned because our 'chat_name_nick' or synchronization source 
				# ID will change each time we come into a voice room.

				for rra in reactivate_igg.keys(): 
					send_ignoreon_packet(rra)

				start_sound_spool()  # this method will return and do nothing if a sound thread is already running



#################################################
#					END TCP FUNCTIONALITY METHODS
#################################################


			
def processTCPData(which_socket=0):
	"""
		Used internally by a thread to handle data received from a Yahoo! TCP socket.
	"""
	global tcp_data, poll_connection, sdivider
	global total_packets
	global chat_name_nick
	if tcp_data==None: return 0
	if tcp_data=='': return 0
	total_packets=total_packets+1
	data=copy.copy(tcp_data)
	tcp_data=None
	dbprint("TCP DATA LEN:  "+str(len(data)))
	dbprint("TCP DATA: "+str(data))
	dbprint("TCP DATA: "+str([data]))
	if poll_connection==0: return 0

	# do case-based responding here

	if data.find("\x00T")>-1:   # a notice that somebody left the voice room		
		if data.find("\x82\xcb")>-1:
					parse_user_leave(data)
					return 1

	if data.find("\x06server")>-1:
		# we successfully hooked into the udp server
		dbprint("got 'Fserver' string")
		send_pager_auth()
		dbprint("sent pager auth")
		return 1

	if data.find("\xd1\x20C\x02\x13\x88D")> -1:  # we got our nick?
		if data.find("CA!Y") >-1:
			if data.find("\x80\xcc\x01\x08") > -1 :
				if data.find("CA!Y") > data.find("\x80\xcc\x01\x08"):
					parse_synch_id(data)
					return 1
		rstarter="ch/"
		if chat_serial=="[PYCONFERENCE]": rstarter="me/"
		if data.find(rstarter+chat_room)>-1:
				parse_server_redirection(data)
				return 0
		dbprint("chat 'nick' authorization problem")
		return 0
	
	# a login 'confirm' request - basically we just send our login string once again
	# AFTER we perform the 'server jump' because we will also be redirected to another
	# voice chat server
	if data.find("\x81\xc8\x00D")> -1:
		# we got a server redirect...it will also repeat the room name and serial number	
		rstarter="ch/"
		if chat_serial=="[PYCONFERENCE]": rstarter="me/"
		if data.find(rstarter+chat_room)>-1:
				parse_server_redirection(data)
				return 0

		elif data.find("\xca") >-1:  # we are connected to voice chat and have 'list' of voice chat users
				parse_member_list(data)
				return 0
	
		else : 
			# see if we got a packet that says our voice chat request is 'not authorized'
			# it will contain the string "\x02\x01\x04" - which spells "BAD" in hex
			if data.find("CA!Y")> -1:
				if data.find("\x02\x01\x04")> -1:
					dbprint("Voice chat request denied, bad login")
					pop_error(_("Voice chat connection failed: bad login"))
					stopPyVoice()
					return 0
			else: 	dbprint("Unknown packet")

		return 0
	dbprint ("UNKNOWN PACKET:   "+str([data])  )

	return 0


def get_room_length(room_len):
	""" 
	Internally used method to get the room length of 'room_len'.  This method returns 
	a single character whose hexidemal value is equal to the length of 'room_len'.
	"""
	dbprint ("room_len: "+str(room_len) )
	if room_len<255: 
		rlen= "%c" % room_len
		return rlen
	return "\x00"




def sendTCPLogin(*args):
	"""
		Used internally by a thread to send our initial login string to a Yahoo! TCP 
		server.  This method should not raise an exception. Returns an int: -1 or 0 
		on failure, or  greater than 1 on success.

		For a regular chat room, the login string will look something like this:
		\\x00\\xb4\\x00\\x00\\x81\\xc9\\x000\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\r\\x80\\xcc\\x00\\x80\\x00\\x00\\x00\\x00CA!Y\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x01d\\x02\\x00\\x02e\\x02\\x00\\x03\\x02\\x0cpmokername\\x03$ch/pmokername\'sroom:1::1600084740\\x07"6wB_.ArnJatIDY2sxrvKldFPlfO.QEPyM-\\x06\\x00\\x05\\x02\\x00\\x19\\x08\\x04\\xac\\x91Ha

		For a private conference, the login string will look something like this:
		\\x00|\\x00\\x00\\x81\\xc9\\x000\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x80\\xcc\\x00H\\x00\\x00\\x00\\x00CA!Y\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x02d\\x02\\x00\\x02e\\x02\\x00\\x03\\x02\\x07mmm__ab\\x03\\x10me/mmm__ab-72249\\x07\\x00\\x06\\x00\\x05\\x02\\x00\\x19\\x08\\x04\\xac\\xaf\\x10\\x96\\x00\\x1e\\x04

		Here is the basic structure of the TCP login packet:
			First 2 bytes:  denote the length of the packet (in hex)
				example: '\\x00\\xb4' = 180 bytes packet length
				derived from:  struct.pack("!h", 180 )  
			Next 50 bytes:  '\\x00\\x00\\x81\\xc9\\x000\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00'
			Next 2 bytes:  '\\x80\\xcc'
			Next 2 bytes:  
				2 chars denoting the length of the remainder of the 
				packet (including the padding), plus 4  (for the 2 bytes above, 
				plus the 2 byte of these chars themselves.)
				example: '\\x00\\xb4' = 180 bytes packet length
				derived from:  struct.pack("!h", 180 )  
			Next 25 bytes:			'\\x00\\x00\\x00\\x00CA!Y\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x01d\\x02\\x00\\x02e\\x02\\x00\\x03\\x02'
			Next 1 bytes:  
				one char denoting the length of the username of the person logging on, the 'length' is a hex char
			Next (variable number) of bytes:  
				the username of the person logging on
			Next 1 byte: '\\x03'
			Next 1 byte:  
				one char denoting the length of the FULL room name - the char is hex.

				For regular chat rooms, this length includes the length of 'ch/' + the length of the room name 
				+ the length of '::' + the length of the chat room's 'serial number' - example: 
				the length of 'ch/pmokernameasroom:1::1600084740' .  The length of this string is 36, so the 
				hex char needed here is '$' (0x24).   In this example, the room name is
				'pmokernameasroom:1'  - regular chat rooms always are followed by a ':' and a room number.

				For private conference rooms, this length includes the length of 'me/' + the length of the 
				room name   (NO '::', NO 'serial number')  - example:  the length of 'me/mmm__ab-72249' is 
				16, so the hex char needed for this one is '\\x10'  (0x10)

			Next 3 bytes:
				regular chat room:  'ch/'
				private conference or PM:  'me/'
			Next (variable number) of bytes:  
				the full name of the chat room the user is entering
				regular chat room example: 'En espanol:1'
				private conference example: 'myname-21847'
			Next 2 bytes (regular chat room only):  '::'
			Next (variable number of bytes, regular chat room only):  
				The 'serial number' of the chat room, example '1600084740'
				These serial numbers always seem to start with '16000'.
				Chat room 'serial' numbers do not appear to be used in private conferences and PM
				conferences.
			Next 1 byte: '\\x07'
			Next 1 byte:
				regular chat room:  '"'   (quotation mark)
				private conference:  '\\x00'   (null byte)
			Next (variable number) of bytes  (regular chat room only):
				The 'cookie' for the chat room, example: '6wB_.ArnJatIDY2sxrvKldFPlfO.QEPyM-'
				Chat room 'cookies' are for regular chat rooms online (not private conferences), and 
				seem to be 34 bytes in length, if I remember correctly.
			Last  9 bytes:  '\\x06\\x00\\x05\\x02\\x00\\x19\\x08\\x04\\xac'
			Maybe be necessary:
				0-3 bytes of 'padding', since all packets must have a length that is a multiple of 4.
				The 'padding' can be junk character bytes.	

			* Note private conferences and PM conferences do not use chat room 'serial' numbers or 
			   chat room 'cookies'.

	"""
	global chat_user, chat_room, chat_cookie, chat_serial
	if chat_user=='': return -1
	if chat_room=='': return -1
	if chat_serial=='': return -1
	if chat_cookie=='': return -1
	global chat_connect_status, eighty_byte_model
	chat_connect_status=1
	eighty_byte_model=0

	#  login  string will look something like this... 
	#\x00\xb4\x00\x00\x81\xc9\x000\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\r\x80\xcc\x00\x80\x00\x00\x00\x00CA!Y\x00\x01\x00\x00\x00\x00\x00\x01d\x02\x00\x02e\x02\x00\x03\x02\x0cpmokername\x03$ch/pmokername\'sroom:1::1600084740\x07"6wB_.ArnJatIDY2sxrvKldFPlfO.QEPyM-\x06\x00\x05\x02\x00\x19\x08\x04\xac\x91Ha

	room_starter="ch/"  # for regular chat rooms
	is_conference=0
	if chat_serial=="[PYCONFERENCE]":   # PM or private conference
		room_starter="me/"
		is_conference=1
		eighty_byte_model=1

	login1="  "  # 2-byte space saver, packet length, filled in later
	login3="\x00\x00\x81\xc9\x000\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"
	login4="\x80\xcc"
	login5="  "  # 2-byte space saver, a second length, do later
	login6="\x00\x00\x00\x00CA!Y\x00\x01\x00\x00\x00\x00\x00\x01d\x02\x00\x02e\x02\x00\x03\x02"+str(get_hex_num_len(len(chat_user)))+chat_user+"\x03"

	if is_conference==1: room_len=len(room_starter+chat_room)   # PM or priv. conference
	else:  room_len=len(room_starter+chat_room+"::"+chat_serial)   # regular chat room
	login6=login6+get_room_length(room_len)


	if is_conference==1:   # PM or private conference
		login6=login6+room_starter+chat_room+"\x07\""+chat_cookie+"\x06\x00\x05\x02\x00\x19\x08\x04\xac"
	else:  # regular chat room
		login6=login6+room_starter+chat_room+"::"+chat_serial+"\x07\""+chat_cookie+"\x06\x00\x05\x02\x00\x19\x08\x04\xac"

	total_len=len(login4)+len(login6)+len(login5)+len(login1)+len(login3) 
	#print "total: "+str(total_len)+"   room:  "+str(room_len)
	login5_len=len(login4)+len(login6)+len(login5)+get_best_padding(4-(total_len%4)   )
	login5=struct.pack("!h",login5_len)  # will be 2 bytes
	part2=login4+login5+login6 
	padding=get_padding(4-(total_len%4))
	#if total_len<172: padding=padding+get_padding(172-(total_len +len (padding) ))
	total_len=total_len+len(padding)
	part2=part2+padding
	login1=struct.pack("!h",total_len)  # will be 2 bytes
	parts=login1+login3+part2

	dbprint("sendTCPLogin - PARTS:  ("+str(len(parts))+") :  "+parts)
	dbprint(str([parts]))

	global last_login_string
	last_login_string=parts.replace("CA!Y\x00\x01\x00\x00\x00\x00\x00\x01d\x02\x00", "CA!Y\x00\x01\x00\x00\x00\x00\x00\x02d\x02\x00")
	
	return tcp_send(parts,0)






## UDP STUFF

def udpPoll():  
	"""
	Used internally to poll a UDP socket for incoming data.  Accessed by internal
	UDP reading Threads.
	"""
	global poll_connection,udp_socket
	while poll_connection==1:
			if poll_connection==0: break	
			if not udp_socket: break
			data=udp_recv(2000)	
			if poll_connection==0: break	
			if not udp_socket: break
			if len(data)>0: 					
					global udp_data
					udp_data=data[0]
					try:
						processUDPData()
					except:
						pass
			else: 
				try:
					time.sleep(0.07)  # very short sleep to be nice to CPU
				except:
					pass
			if poll_connection==0: break	
			if not udp_socket: break


def udpThreadRead(*args):  
	"""
		Used internally by a thread to handle connecting to and polling a Yahoo! UDP 
		voice chat server.
	"""
	dbprint("UDP thread started:  "+str(vchat_udp_server)+"   "+str(vchat_port_udp))
	if open_udp():
		if sendUDPLogin()==-1:
			dbprint("UDP Login-1 failed")
			pop_error(_("Could not login to voice server")+"  [UDP]")
			udp_close()
			return 0
		udpPoll()
		udp_close()
	return 0



def send_udp_ack_keepalive(data):
		"""
		Internal method to send a 'keep-alive' string back to Yahoo's UDP server when Yahoo 'pings' us or 
		asks 'are you still there?'  

		This happens when somebody stops talking (sometimes 
		WHILE somebody is talking also). Yahoo kind of sends 
		a 12-byte packet that let's us know they stopped talking and asks
		"do you read me?".  If we fail to respond to enough of these 
		packets, Yahoo will boot us from the voice server after about 10-15
		minutes.  The response is simple:

		We get sent a packet that looks something like:
		\\x80\\xa2\\x00\\x01\\x07?r\\xcb\\x00\\x00\\x00\\x00

			Format:
			First 2 bytes: '\x80\xa2'
			Next 2 bytes: Packet sequence number (ex: '\\x00\\x01')
			Next 4 bytes: Packet time stamp (ex: '\\x07?r\\xcb')
			Last 4 bytes: Null bytes (usually) - '\\x00\\x00\\x00\\x00'

			* This packet has no RTP 'payload'.
			* It is an 'empty' RTP packet with just the basic RTP header.

		Our response - The only thing we really need to do is change the 
		RTP marker  '\\xa2' to '\\x22'  (a quotation mark).

		So we will send back a packet that looks like (as an example):
		\\x80\\x22\\x00\\x01\\x07?r\\xcb\\x00\\x00\\x00\\x00

		NOTE: We will always get a similar 12-byte string sent back to us 
		right after we have logged into the UDP server using method 'sendUDPlogin'.
		And, we do NOT want to respond to that string.  We will only send a 
		response if the string sent to us does NOT include our 'chat_nick_name' (or 
		RTP synchronization ID).

		"""
		global chat_name_nick
		if data.find(chat_name_nick)==-1:
			try:
				udp_ack=data[2:]  # grab last 10 bytes as-is
				# now just change the marker and acknowledge
				udp_ack_resp="\x80\x22"+udp_ack
				dbprint("UDP keep-alive ack sent:  ["+str(len(udp_ack_resp))+"]     "+str([udp_ack_resp]))
				udp_send(udp_ack_resp)
			except:
				pass


def processUDPData():
	"""
		Used internally by a thread to handle data received from a Yahoo! UDP socket.
		Generally, RTP data is handled here.
	"""
	global udp_data,poll_connection,total_packets, chat_name_nick
	total_packets=total_packets+1
	data=copy.copy(udp_data)
	udp_data=None
	udplen=len(data)
	dbprint("UDP DATA LEN:  "+str(udplen))
	dbprint("UDP DATA: "+str(data))
	dbprint("UDP DATA: "+str([data]))

	if udplen==112:  # we got a packet with truespeech sound in it
		dbprint("going_to_extract")
		try:
			extract_tsp_sound(data)
			dbprint("sound extract done")
		except:
			pass
		return 0

	if udplen==12:  
		send_udp_ack_keepalive(data)
		return 0




def sendUDPLogin():
	"""
	Used internally by a running thread to handle logging into the Yahoo UDP 
	voice chat server. 

	Here is our initial, first RTP packet communication with Yahoo's UDP server.
	The UDP 'login' string is always 12 bytes (not including the UDP headers 
	handled by Python).  Here is the structure of the 12-byte UDP packet:
		First 2 bytes: '\\x80\\xa2'
		RTP packet seqence: in hex, always 1, padded to 2 bytes ('\\x00\\x01')
		RTP packet time stamp: in hex, always 0, padded to 4 bytes ('\\x00\\x00\\x00\\x00')
		4-byte voice server 'nick name' assigned to us (synchronization source ID)

		** This packet has no RTP 'payload'.
		
	Example RTP login packet: 
		\\x80\\xa2\\x00\\x01\\x00\\x00\\x00\\x00\\xef\\xb0\\xc7\\x22
	"""
	global chat_name_nick, record_sequence
	r_sequence="%04X" % record_sequence
	r_seq=binascii.unhexlify(r_sequence)
	udp_login="\x80\xa2"+r_seq+"\x00\x00\x00\x00"+chat_name_nick
	record_sequence=record_sequence+1
	dbprint("sendUDPLogin:  "+str([udp_login])+"   "+str(len(udp_login)))
	return udp_send(udp_login)


def get_best_padding(suggest):
	""" Used internally to get a recommended packet padding size. """
	if suggest==1: return 5
	if suggest==2: return 6
	return suggest

def get_padding(str_len):
	"""
		Used internally by various methods to get needed packet padding.
		Packets sent to yahoo must always be a length that is a multiple of 4, so 
		this method returns necessary padding.
		As far as I can see, this padding is just 'junk characters'. This method 
		returns a string of length 'str_len' containing 'junk characters'.
	"""

	if str_len==0:	return ""
	padd="H\x00\x00\x00\x91Ha\x00\x9a\x8b\xda\x9a\x8b\xda\x9a\x8b\xda\x9a\x8b\xda\x9a\x8b\xda\x9a\x8b\xda"
	if str_len==1: return padd[:5]
	if str_len==2: return padd[:6]
	return padd[:str_len]



def get_abc_len(letterS):  
	""" Used internally. Letter to hex number mapping. Returns a string. """
	if letter_len_map_num.has_key(letterS):
		return letter_len_map_num[letterS]
	return 1

def get_abc_num_len(numberS): 
	""" Used internally. Hex number to letter mapping. Returns a string. """
	if letter_len_map.has_key(numberS):
		return letter_len_map[numberS]
	return 'A'
	
def get_hex_len(hexchar): 
	""" Used internally. Char to hex number mapping. Returns a string. """
	if hex_len_map_num.has_key(hexchar):
		return hex_len_map_num[hexchar]
	return 1

def get_hex_num_len(hexnum):
	""" Used internally. Hex number to char mapping. Returns a string. """
	if hex_len_map.has_key(hexnum):
		return hex_len_map[hexnum]
	return 'A'


def guess_whose_talking(pack_part):
	""" Used internally.  Makes a logical, usually accurate guess at who is speaking if 
	all other parsing methods have failed.  Accepts a string.  Returns a string. """
	global chat_name_nicks, sdivider
	if len(pack_part)==3:
		for ii in chat_name_nicks.keys():
			if ii.find(sdivider+pack_part)>-1: return chat_name_nicks[sdivider+ii]
	for ii in chat_name_nicks.keys():
		if ii.find(pack_part)>-1: return chat_name_nicks[ii]
	return ""


def extract_tsp_sound(some_packet):
	""" 
	Used internally to extract TrueSpeech data from a RTP-over-UDP packet 
	received from the Yahoo! UDP voice chat server.  Each sound packet should 
	be a total of 112 bytes in length, hold 96 bytes of TrueSpeech data.  This 
	method extracts the 96 bytes of sound and adds it to the sound buffer 
	to be played on the sound device.  This method also uses information in the 
	packet to determine who is talking.

	The sound packet looks something like this:
	\\x80"\t0\\x00\\x19\\x06\\xe0\t\\xc64j\\x00\\x02\\xd5d\\x1b\\xbd5\\xe4\\xf0\\x14\\xdd\\x1f\\x01\\x8f.\\x01^\\x9e\\x19z\rj\\xf4\\xdc\\xa1\\xca\\xdd<2\\x83\\xbf\\x8c\\xc9\\x02y\\x15l\\xb3\\xb4\\xa7\\x13\\xf2\\x15\\x0f\\xfa\\x06\\x989_*\\x1a\\x9c\\xdcu\\xc2\\x0c?{C\\x9b\r\\xc1a\\x1c\\xa5\\xed\\xd7\\x11m!\\x99\\xc5\\xa1l\\x18\\x00\\x8b\n\\x8b\\xa7\\xc0\\xd1\\x13\\xdd\\xadu6\\xfc\\xca\\xf8\\x06\\xdc\\x8c\\xbb=\\xcb\\x15\\x90\\xd3\\xff\\xf7\\x00\t\\x06dns-0

	Anything starting with '\\x' is an escape character.  The RTP header is between the 
	'\\x80' and the '\\x00\\x02' (12 bytes).  The payload (the tsp sound) starts 
	immediately after the '\\x00\\x02' and ends with the last '\\xff'.  And, the tsp sound 
	should be exactly 96 bytes once extracted. Trying to play sounds of other sizes 
	has only resulted in garbled junk sound for me, even though the TrueSpeech codec 
	'tries' to play it, it just makes a bunch of noise.  So we will only play sounds that are 
	96 bytes once extracted.  Escape characters in python are much like escape characters
	in C...in Python: \\xff = \\ff in C, \\x02 in Python = \\02 in C, etc.

	Also, the length of the data sent to the TrueSpeech codec (through pytsp) must 
	be a multiple of 32  (96, 120, 240, 480, etc.).  It is not recommended to send 
	less than 240 bytes to pytsp,  since it won't sound like much.  Values between 960 and 
	1920+ seem best.  If we send data to truespeech codec when the length is NOT
	a multiple of 32, the sound we hear comes out very garbled and junky.

	Each packet also contains the short 'voice chat nick name' of the Yahoo user who is currently 
	talking:  this 'nick name' always starts at exactly the 9th byte and goes all the way up to the 
	'\\x00\\x02' (which denotes start of TSP sound).  The 'chat nick name' (officially called 
	the RTP synchronization source ID) is always 4 bytes exactly.
	"""
	if some_packet.startswith("\x80\""):
	   if some_packet.find("\x00\x02")>-1:
		if some_packet.rfind("\xff") > some_packet.find("\x00\x02"):

			global audio_packets,  on_mute, can_record, record_time
			audio_packets=audio_packets+1			
			# grab the 'nick name' of the Yahoo user who just sent this sound

			global sdivider
			alt_talks=some_packet[8:14]
			# alt_talks helps support older Yahoo clients
			if alt_talks.startswith("\x00"): alt_talks=alt_talks[1:]
			if alt_talks.find("\x00\x02")>-1:
				alt_talks=alt_talks[:alt_talks.rfind("\x00\x02")]
			if alt_talks.find("\x00")>-1:
				alt_talks=alt_talks[:alt_talks.rfind("\x00")]
			who_talks=some_packet[some_packet.find(sdivider)+1:]		
			who_talks=who_talks[:who_talks.find("\x00\x02")]
			if who_talks.endswith(sdivider): who_talks=who_talks[:len(who_talks)-1]
			while who_talks.find(sdivider)>-1:
				who_talks=who_talks[who_talks.find(sdivider)+1:]
			who_talks=who_talks.replace("\x00","")
			if chat_name_nicks.has_key(who_talks) or chat_name_nicks.has_key(who_talks+sdivider) or  chat_name_nicks.has_key(sdivider+who_talks) or chat_name_nicks.has_key(alt_talks):
				global who_is_talking,ignored_users,user_stats,who_is_talking_expire
				if chat_name_nicks.has_key(who_talks):
					who_is_talking=chat_name_nicks[who_talks]	
				elif chat_name_nicks.has_key(sdivider+who_talks):
					who_is_talking=chat_name_nicks[sdivider+who_talks]	
				elif chat_name_nicks.has_key(alt_talks):
					who_is_talking=chat_name_nicks[alt_talks]	
				else :
					who_is_talking=chat_name_nicks[who_talks+sdivider]	
				who_is_talking_expire=time.time()
				if not user_stats.has_key(who_is_talking): 
					user_stats[who_is_talking]=0
				user_stats[who_is_talking]=user_stats[who_is_talking]+1
				# if this person is igged, don't play their TSP
				if ignored_users.has_key(who_is_talking): return 1  
			else: 
				guessed=''
				try:
					guessed=guess_whose_talking(alt_talks)
				except:
					pass
				if not guessed=='': 
					who_is_talking=guessed
					who_is_talking_expire=time.time()
					if not user_stats.has_key(who_is_talking): 
						user_stats[who_is_talking]=0
					user_stats[who_is_talking]=user_stats[who_is_talking]+1
					# if this person is igged, don't play their TSP
					if ignored_users.has_key(who_is_talking): return 1  
				else:
					dbprint ("who-talks-NICK2:  "+str([who_talks]) )
					dbprint ( "who-talks-ALT2:  "+str([alt_talks]) )
					dbprint ("who-talks-NICK-PACK:  "+str([some_packet[:95]]) )
					dbprint ("who-talks-SDIV:  "+str([sdivider]) )

			# return if we are on mute
			if on_mute==1: return 1

			target=96
			if eighty_byte_model==1:
				target=64

			sdata=some_packet[some_packet.find("\x00\x02")+2:some_packet.rfind("\xff")]
			if len(sdata)==target:
				global sound_data
				# also avoid over-stuffing the buffer
				if len(sound_data) < snd_buflimit: sound_data=sound_data+sdata
				who_is_talking_expire=time.time()	
				can_record=0
				record_time=time.time()
			else:
				dbprint("sound bad len:  "+str(len(sdata)))
				dbprint("bad len packet:  "+str([some_packet]))
				dbprint("bad len:  "+str([sdata]))
				dbprint("bad len find:  "+str(some_packet.find("\xff") )+"   rfind: "+str(some_packet.rfind("\xff") ))

				# There are a few folks walking around with old/strange Yahoo clients
				# that occassionally send non-standard, but often playable packets
				# here...we will try to salvage what we can - we can fix some packets 
				# with excess data, but we can do nothing about packets that are too short

				#attempt to salvage some 'bad' packets...sometimes packets have 
				# multiple  '\x00\x02' starters...causing 8 bytes of junk
				targ8=target+8
				if len(sdata)== targ8 :
					if sdata.find("\x00\x02")==6:
						sdata=sdata[8:]
						if len(sdata)==target:
							dbprint("bad len package salvaged  [104]")
							# also avoid over-stuffing the buffer
							if len(sound_data) < snd_buflimit: 	sound_data=sound_data+sdata
							who_is_talking_expire=time.time()
							can_record=0
							record_time=time.time()
						else: dbprint("bad len package NOT salvaged")
					else: dbprint("x00x02  present at  "+str(sdata.find("\x00\x02")))
				#somes we have data that ends with duplicate '\xff' enders, causing 
				# 1 extra byte of junk...fix
				targ1=target+1
				if len(sdata)==targ1:
					if sdata.endswith("\xff"):
						sdata=sdata[:len(sdata)-1]
						if len(sdata)==target:
							dbprint("bad len package salvaged [97]")
							# also avoid over-stuffing the buffer
							if len(sound_data) < snd_buflimit: 	sound_data=sound_data+sdata
							who_is_talking_expire=time.time()
							can_record=0
							record_time=time.time()
						else: dbprint("bad len package NOT salvaged")		

		else: dbprint("sound: xff   vs  x00x02 problem")
	else: dbprint("sound:  no x80")





def stopPyVoice(*args):
	""" 
	Used internally by the graphical user interface to logout of the Yahoo! voice chat 
	servers and close all sockets.  This method may also be used to close a connection 
	that is in the process of 'logging in' to the voice chat servers. NOTE: This method 
	does not stop playback of any remaining sound in the sound buffer. So, it is 
	possible to continue hearing a few seconds of sound on the sound device after 
	disconnecting from the Yahoo voice chat servers.
	"""
	global poll_connection, allow_vol_update
	sendTCPLogout()
	poll_connection=0
	tcp_close(0)
	tcp_close(1)
	udp_close()
	chat_name_nicks.clear()
	global who_is_talking
	who_is_talking=""
	global sound_data
	sound_data=""
	global chat_connect_status
	chat_connect_status=0
	global esd_volume
	esd_volume=0
	global can_record,record_sequence
	record_sequence=1
	allow_vol_update=0
	can_record=0
	time.sleep(0.2)
	pytsp.esd_uninitPlayback()  # don't hold the play line unnecessarily, re-opened as it's needed
	pytsp.esd_uninitRecord()  # don't hold the record line unnecessarily, re-opened as it's needed
	save_ignore_list()


def setup_esd_host(some_host):
	""" 
	Used internally by the graphical user interface to set the ESound host for 
	sound playback.  This does NOT set the host for recording/sending sound, which 
	is always 'localhost'.
	"""
	if not some_host==None:
		if len(str(some_host))>0: pytsp.set_esd_host(str(some_host))


def startPyVoice(*args):
	""" 
	Used internally by the graphical user interface to start the login process to the 
	Yahoo voice chat servers.
	"""
	global poll_connection
	poll_connection=1
	read_config()
	load_ignore_list()
	# don't hold the record line unnecessarily
	# it will be re-opened as it's needed
	pytsp.esd_uninitRecord()  
	pytsp.esd_uninitPlayback()
	ts=Thread(None,tcpThreadRead)
	ts.start()

def get_chat_room_name():
	""" 
	Used internally by the graphical user interface get the current voice chat room 
	name.  Returns a string.
	"""
	return chat_room

def get_who_is_talking():
	""" 
	Used internally by the graphical user interface get the username of the 
	Yahoo! user who is currently speaking.  Returns a string. This method may 
	return an empty string '' if nobody is currently speaking or if the
	username of the user speaking could not be determined.
	"""
	global who_is_talking
	return who_is_talking

def get_users_in_room():
	""" 
	Used internally by the graphical user interface get a list of Yahoo! usernames
	currently in the Yahoo voice chat room.  Returns a list.  This method may 
	return a empty list [] or a list with duplicate usernames.  It is the job of the 
	graphical user interface to handle the elimination of duplicate usernames.
	"""
	uslist=chat_name_nicks.values()
	return uslist


def send_ignoreon_packet(rtpkey):
	"""
	This method sends a very simple packet to Yahoo's voice TCP server informing 
	Yahoo that we want to ignore the Yahoo user with the 4-byte synchronization 
	source ID 'rtpkey'.  If all goes well, Yahoo will stop sending us sound from the 
	user we wish to ignore.  This feature should be considered experimental.
	
	The packet sent is always 76 bytes in length.  Here is the format:
		First 8 bytes:  '\\x00L\\x00\\x00\\x81\\xc9\\x000'
		Next 4 bytes:  Our 4-byte synchronization source ID  'chat_name_nick'
		Next 44 bytes:  '\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x01\\xd5\\x00\\x00\\x00\\x03\\x00\\x00\\x00\\x00\\x00\\x00\\xb74\\x80\\xcc\\x00\\x18'
		Next 4 bytes:  The 4-byte synchronization source ID of the user to be ignored
		Last 16 bytes:  'CA!Y\\x00\r\\x00\\x00\\x00\\x00\\x00\\x04\\x0e\\x02\\x00\\x00'

	Here is an example 'ignore' on packet:
	\\x00L\\x00\\x00\\x81\\xc9\\x000\\x04%\\x8e=\\x00\\x00\\x00\\x00\\x01\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x01\\xd5\\x00\\x00\\x00\\x03\\x00\\x00\\x00\\x00\\x00\\x00\\xb74\\x80\\xcc\\x00\\x18\\x03\\xaeA\\xadCA!Y\\x00\r\\x00\\x00\\x00\\x00\\x00\\x04\\x0e\\x02\\x00\\x00
	"""
	global chat_name_nick
	if not len(chat_name_nick)==4: return
	if not len(rtpkey)==4: return
	iggpacket="\x00L\x00\x00\x81\xc9\x000"+chat_name_nick+"\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x01\xd5\x00\x00\x00\x03\x00\x00\x00\x00\x00\x00\xb74\x80\xcc\x00\x18"+rtpkey+"CA!Y\x00\r\x00\x00\x00\x00\x00\x04\x0e\x02\x00\x00"
	dbprint("Ignore ON packet:  len: "+str(len(iggpacket))+"    "+str([iggpacket]))
	tcp_send(iggpacket,1)




def send_ignoreoff_packet(rtpkey):
	"""
	This method sends a very simple packet to Yahoo's voice TCP server informing 
	Yahoo that we want to UN-ignore the Yahoo user with the 4-byte synchronization 
	source ID 'rtpkey'.  If all goes well, Yahoo will start sending us sound again 
	from the user we wish to un-ignore. This feature should be considered experimental.
	Possible problem: If Yahoo! fails to un-ignore or un-mute the user 'rtpkey', we may 
	find that the user is on 'mute' until we logoff then log back on to Yahoo!
	
	The packet sent is always 76 bytes in length.  Here is the format:
		First 8 bytes:  '\\x00L\\x00\\x00\\x81\\xc9\\x000'
		Next 4 bytes:  Our 4-byte synchronization source ID  'chat_name_nick'
		Next 44 bytes:  '\\x00\\x00\\x00\\x00\\x0c\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x02\\xd5\\x00\\x00\\x00$\\x00\\x00\\x00\\x00\\x00\\x01\\x1b4\\x80\\xcc\\x00\\x18'
		Next 4 bytes:  The 4-byte synchronization source ID of the user to be un-ignored
		Last 16 bytes:  'CA!Y\\x00\\x0f\\x00\\x00\\x00\\x00\\x00\r\\x0e\\x02\\x00\\x00'

	Here is an example 'ignore' off packet:
	\\x00L\\x00\\x00\\x81\\xc9\\x000\\x04%\\x8e=\\x00\\x00\\x00\\x00\\x0c\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x00\\x02\\xd5\\x00\\x00\\x00$\\x00\\x00\\x00\\x00\\x00\\x01\\x1b4\\x80\\xcc\\x00\\x18\\x03\\xaeA\\xadCA!Y\\x00\\x0f\\x00\\x00\\x00\\x00\\x00\r\\x0e\\x02\\x00\\x00
	"""
	global chat_name_nick
	if not len(chat_name_nick)==4: return
	if not len(rtpkey)==4: return
	iggpacket="\x00L\x00\x00\x81\xc9\x000"+chat_name_nick+"\x00\x00\x00\x00\x0c\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x02\xd5\x00\x00\x00$\x00\x00\x00\x00\x00\x01\x1b4\x80\xcc\x00\x18"+rtpkey+"CA!Y\x00\x0f\x00\x00\x00\x00\x00\r\x0e\x02\x00\x00"
	dbprint("Ignore OFF packet:  len: "+str(len(iggpacket))+"    "+str([iggpacket]))
	tcp_send(iggpacket,1)



def user_is_ignored(mname):
	""" 
	Used internally by the graphical user interface and the pyvoice.py library to 
	determine if the Yahoo! user 'mname' is currently on 'ignore'. Returns an int: 
	0=not ignored, 1=ignored.  Currently, sound data 
	from the user will continue to be received, but will not be played to the sound 
	device.
	"""
	global ignored_users
	return ignored_users.has_key(mname)


def load_ignore_list():
	"""
	Used internally to load the list of ignored users from the file ~/.pyvoice_ignore .  This method 
	also merges in the list of GYach Enhanced ignored users from the file ~/.yahoorc/gyach/ignore 
	if the file exists and is readable.  This method is called automatically and does not throw an 
	exception. This method does not return a value.	
	"""
	iggfiles=[pyvoice_ignore_file,gyach_ignore_file]
	for yy in iggfiles:
		try:
			f=open(yy)
			flist=f.read().split("\n")
			f.close()
			for ii in flist:
				iggit=ii.replace("\t","").replace("\r","").strip().lower()
				if len(iggit)>0: ignored_users[iggit]=1
		except:
			pass



def save_ignore_list():
	"""
	Used internally to save the list of ignored users in the file ~/.pyvoice_ignore .  This method
	is called automatically whenever ignore_user() or unignored_user() is called and does not
	throw an exception.	 This method does not return a value.
	"""
	try:
		f=open(pyvoice_ignore_file,"w")
		l=ignored_users.keys()
		l.sort()
		for ii in l:
			f.write(ii+"\n")
		f.flush()
		f.close()
	except:
		pass


def ignore_user(mname):
	""" 
	Used internally by the graphical user interface to place a user on ignore, where 
	'mname' is the name of the Yahoo user to be ignored.  This method then 
	sends a UDP packet using the method 'send_ignoreon_packet' to inform the 
	Yahoo voice server that we no longer wish to receive sound from user  
	'mname'.  (This feature should be considered experimental.)  As a backup, if Yahoo 
	fails to honor our 'ignore' request, PyVoice will make every attempt to avoid 
	playing sound from ignored users even if it is sent to us from the Yahoo voice 
	server.
	"""
	global ignored_users, chat_name_nicks
	ignored_users[mname]=1
	for ii in chat_name_nicks.keys():
			ckey=ii
			cuser=chat_name_nicks[ii]
			if cuser==mname:
				send_ignoreon_packet(ckey)
	save_ignore_list()
		

def unignore_user(mname):
	""" 
	Used internally by the graphical user interface to remove a user from ignore, where 
	'mname' is the name of the Yahoo user to be un-ignored.  This method then 
	sends a UDP packet using the method 'send_ignoreoff_packet' to inform the Yahoo 
	voice server that we wish to resume receiving sound from user 'mname'. (This 
	should be considered experimental for now.)
	"""
	global ignored_users
	if ignored_users.has_key(mname):
		try:
			del ignored_users[mname]
		except:
			pass
	for ii in chat_name_nicks.keys():
			ckey=ii
			cuser=chat_name_nicks[ii]
			if cuser==mname:
				send_ignoreoff_packet(ckey)
	save_ignore_list()


def set_voice_server(serv):
	""" 
	Used internally by the graphical user interface to set the selected voice chat 
	server to non-null string 'serv'.  
	"""
	global vchat_server
	if not serv==None: vchat_server=str(serv)

def get_voice_server():
	""" 
	Used internally by the graphical user interface to get the name of the name of the 
	voice chat server currently being used for initial TCP connections to Yahoo! Returns 
	a string.  This method does NOT return the name of any TCP servers we may have 
	been re-directed to.
	"""
	global vchat_server
	return vchat_server

def open_sound():
	""" 
	Used internally by the graphical user interface.  This method MUST be called by 
	the graphical user interface before anything else to initiate the PyTSP library 
	used by pyvoice and prepare the sound device. No sound should be 
	played or recorded before this method is called, which means no UDP or TCP 
	connections should be initiated before this method is called.
	"""
	pytsp.pytsp_init()
	timeout_add(30, update_esd_volume)


def close_sound():
	""" 
	Used internally by the graphical user interface.  This method MUST be called by 
	the graphical user interface as the LAST method called in the pyvoice library to 
	uninitiate the PyTSP library used by pyvoice and close/free the sound device(s).
	"""
	pytsp.pytsp_uninit()
	save_ignore_list()


def start_sound_spool(*args):
	"""
	Used internally by a running UDP socket-reading thread.  After we have successfully 
	logged into the Yahoo voice chat UDP server, this method is called to launch a thread 
	which plays sound to the sound device as it is retrieved, extracted, and added 
	to the sound buffer by the method 'extract_tsp_sound'.  This thread automatically 
	'dies' when we disconnect from the voice servers.  This method launches a thread 
	for the 'runSoundThread' method.
	"""
	global sound_spool_started
	if sound_spool_started==1: return
	sound_spool_started=1
	dbprint("Sound spool started")
	ts=Thread(None,runSoundThread)
	ts.start()

def runSoundThread(*args):
	"""
	Used internally by a thread to check the sound buffer for available sound to 
	play to the sound device.
	"""
	global chat_connect_status   # poll connection
	while chat_connect_status==2:
		try:
			spool_sound()
		except:
			dbprint("Exception in runSoundThread from spool_sound")
			pass
	global sound_spool_started
	sound_spool_started=0



def spool_sound(*args):
	"""
	Used internally by a thread to check the sound buffer for available sound to 
	play to the sound device.  Called at intervals by the 'runSoundThread' 
	method.  If an adequate amount of sound is available in the sound buffer, 
	sound is fed in fragments to the sound device.  Also, this method takes 
	care of closing the ESound sound device if it has been idle for a while, and 
	clears the name of the Yahoo user said to be speaking if that name has 
	gone 'stale' with no incoming sound from that person for several seconds, 
	since Yahoo sometimes fails to tell us when a user has stopped talking. 
	In addition, if this method finds sound to play to the sound device, it 
	temporarily disables recording/sending of sound data to avoid the 
	problem of 'snatching the mic' from another user who is already talking - 
	which most civilized people consider to be rude.  Once there is no more 
	sound to play, the ability to record/send sound is restored.
	"""
	global sound_data
	global 	chat_connect_status
	global who_is_talking_expire, who_is_talking, can_record, record_time
	global esd_volume, allow_vol_update
	if not chat_connect_status==2: return 0
	my_file=pytsp.get_tsp_storage_file()
	if len(sound_data)==0: 
		rectime=time.time()-record_time
		if rectime>1.7:
			can_record=1
	talktime=time.time()-who_is_talking_expire
	if talktime>1.7:
		allow_vol_update=0
	if talktime>2.7:
		who_is_talking=""
		esd_volume=0
		allow_vol_update=0
	# NOTE: any sound data we send to pytsp must be a multiple of 32 in string length
	# Explanation:  We have so many 'cases' here because we want to play the biggest
	# available block of sound to avoid hearing sound that 'skips' alot but without
	# slowing down the machine too much - unless a smaller amount of data has been 
	# sitting 'idle' in the buffer, then we will play whatever is available...but, the 
	# bigger the sample we can play the less skipping we will hear


	# if len(sound_data)>50:	print "len:   "+str(len(sound_data))

	if len(sound_data)> (sound_cut2-1):	   # Too much sound, try to clean out buffer
		poss=len(sound_data)/32
		remainder=(poss*32)%32
		possible=(poss*32)-remainder
		f=open(my_file,"w")
		f.write(sound_data[:possible])
		f.flush()
		f.close()
		allow_vol_update=1
		ss=pytsp.decompress_tsp_to_esd(my_file,  8000, 16, 1)		
		sound_data=sound_data[possible:]  # 928, 960
		who_is_talking_expire=time.time()
		update_esd_volume() # only update volume when sound playing or esd will hang
		can_record=0
		record_time=time.time()
		dbprint("Played "+str(ss)+" TrueSpeech bytes from User: '"+get_who_is_talking()+"'")
		# print "full "+str(possible)
		time.sleep(0.2)
		return 0


	if len(sound_data)> (sound_cut1-1):	 # this is our aim, what we hit most of the time
		f=open(my_file,"w")
		f.write(sound_data[:sound_cut1])
		f.flush()
		f.close()
		allow_vol_update=1
		ss=pytsp.decompress_tsp_to_esd(my_file,  8000, 16, 1)
		sound_data=sound_data[sound_cut1:]
		update_esd_volume() # only update volume when sound playing or esd will hang
		who_is_talking_expire=time.time()
		can_record=0
		record_time=time.time()
		dbprint("Played "+str(ss)+" TrueSpeech bytes from User: '"+get_who_is_talking()+"'")
		time.sleep(0.2)
		return 1
		

	else:  # nothing in the buffer or not enough, so let's 'sleep' for a bit, cpu rest 
		rectime=time.time()-record_time
		if rectime>1.5:
			can_record=1
		time.sleep(0.12)			
		pushit=0
		sndcutter=sound_cut1-1
		if not chat_connect_status==2: return 0
		if len(sound_data)> sndcutter: pushit=1
		talktime=time.time()-who_is_talking_expire
		if talktime>1.0:  pushit=1     # play any 'stale' sound in the buffer
		if pushit==1:
			poss=len(sound_data)/32
			remainder=(poss*32)%32
			possible=(poss*32)-remainder
			
			if possible>32:	
				#print "fallback"
				f=open(my_file,"w")
				f.write(sound_data[:possible])
				f.flush()
				f.close()
				allow_vol_update=1
				sound_data=sound_data[possible:]  
				ss=pytsp.decompress_tsp_to_esd(my_file,  8000, 16, 1)		
				if possible>700: update_esd_volume() 
				# only update volume when sound playing or esd will hang
				dbprint("Played "+str(ss)+" TrueSpeech bytes from User: '"+get_who_is_talking()+"'")
		
				who_is_talking_expire=time.time()
				can_record=0
				record_time=time.time()
				return 0


		talktime=time.time()-who_is_talking_expire
		rectime=time.time()-record_time
		if rectime>1.5:
			can_record=1
		if talktime>1.7:
			allow_vol_update=0
		if talktime>2.7:
			who_is_talking=""
			esd_volume=0
			allow_vol_update=0
		if talktime>6:  
			# sound has been idle (nobody talking) for many seconds, so we will close the ESound
			# connection and free up the line, it will be re-opened when it's needed again, maybe this
			# will help with the '/dev/zero - too many files open' problem I experienced
			# better on system resources as well.
			pytsp.esd_uninitPlayback()
			esd_volume=0
			allow_vol_update=0
	if chat_connect_status==2: return 1
	return 0



def get_is_recording():
	"""
	Used internally by the graphical user interface and the pyvoice.py library 
	to determine if we are recording/sending sound out. Returns an int:
		0=Not recording/sending anything at all
		1=Attempting to record/send but failing
		2=Recording/sending successfully
	"""
	global is_recording, mic_test, can_record, chat_connect_status
	if mic_test==1:
		if is_recording==1: return 2
		return 1
	if is_recording==1: 
		if can_record==0: return 1  # 1
		if chat_connect_status<2: return 1
		return 2
	return 0

def stopMicTest(*args):
	"""
	Used internally by the pyvoice.py library to stop the microphone test.
	"""
	global chat_connect_status,sound_data,is_recording,who_is_talking,mic_test
	global esd_volume, allow_vol_update
	#print "stopMicTest"
	time.sleep(10)
	chat_connect_status=0
	pytsp.esd_uninitRecord()
	pytsp.esd_uninitPlayback()
	sound_data=""
	who_is_talking=""
	is_recording=0
	esd_volume=0
	allow_vol_update=0
	mic_test=0
	return 0

def is_mic_test():
	"""
	Used internally by the pyvoice.py library to distinguish between whether 
	we are truly recording/sending data to Yahoo or conducting a local microphone 
	test.  Returns an int: 0=not a mic test, 1=is a mic test.
	"""
	global mic_test
	return mic_test

def doMicTest(*args):
	"""
	Used internally by the pyvoice.py library to run a microphone test.  The method 
	pretends that we are connected to Yahoo, records a few seconds of sound, then 
	plays it back on the sound device. This method is access by the thread launched 
	in 'startMicTest'.  No sound data is sent to the Yahoo voice chat servers.
	"""
	global is_recording,chat_connect_status,sound_data
	global who_is_talking_expire,who_is_talking,mic_test
	mic_test=1
	is_recording=1
	chat_connect_status=2
	sdata=""
	i=0
	who_is_talking=_("MIC TEST")
	#print "time sleep"
	time.sleep(1)
	#print "time sleep"
	ts=Thread(None,stopMicTest)
	ts.start()

	while i<2:
	   try:
		tfile=pytsp.tsp_record_sound(16321)  # 16321
		if tfile=='':
			i=i+1
			#print "no data"
			continue
		f=open(tfile)		
		ss=f.read()
		f.close()
		#print "recording"
		i=i+1
		sdata=sdata+ss
	   except:
		#print "error"
		pass
	sound_data=sdata+sound_data
	is_recording=0
	dbprint ( "MIC TEST DATA: "+str(len(sound_data)) )
	who_is_talking=_("MIC PLAY")
	who_is_talking_expire=time.time()
	start_sound_spool()
	return 0

def startMicTest(*args):	
	"""
	Used internally by the pyvoice.py library to launch the microphone test thread. 
	This method launches a thread to handle the method 'doMicTest'.
	"""
	if get_is_recording()==0:
		#pytsp.esd_uninitPlayback()
		ts=Thread(None,doMicTest)
		ts.start()

	
	
def update_esd_volume(*args):
	global esd_volume
	global sound_data, allow_vol_update
	try:
		i=0
		if allow_vol_update==1: i=int(pytsp.get_esd_volume()) 
		esd_volume=i
	except:
		pass
	return 1

def get_esd_volume():
	"""
	Used internally by the graphical user interface to get the current playback volume 
	on the ESound sound daemon.  Returns an 'int':  the 'accuracy' of which is not 
	yet known.  This value is polled constantly by the graphical user interface for 
	on-screen volume display.
	"""
	global esd_volume
	return  esd_volume



def recording_off(*args):
	"""
	Used internally by the graphical user interface to stop recording and sending 
	sound data to the Yahoo! voice chat servers.
	"""
	global is_recording
	pytsp.esd_stop_recording()
	is_recording=0

	#global can_record,chat_connect_status
	#can_record=1
	#chat_connect_status=0


def recording_on(*args):
	"""
	Used internally by the graphical user interface to start recording and sending 
	sound data to the Yahoo! voice chat servers.  This method launches the 
	'recordForSend' thread.
	"""
	#global can_record,chat_connect_status
	#can_record=1
	#chat_connect_status=2

	global is_recording, rsound_data
	is_recording=1	
	global record_sequence
	global r_starttime
	record_sequence=2
	r_starttime=0
	ts=Thread(None,recordForSend)
	ts.start()


def build_rec_packet(data64):
	"""
	Builds appropriate packets for sending recorded TrueSpeech data ('data65') from 
	the microphone.  Returns a string containing the formatted packet, or an 
	empty string '' if the length of 'data64' is not appropriate. This method encodes
	the TrueSpeech data into the appropriate RTP-format packet, generally 
	either 112 or 90 bytes in length, depending on the 'byte model' we use. 
	The default is to create packets of 112 bytes.

	Here we build the packet of recorded TrueSpeech data to be sent to the voice chat
	server.  Not including UDP headers, the RTP packets size of sent sound is always 
	exactly 112 bytes or 90 bytes.   Here's the structure  (112-byte packet):
		First 2 bytes:  '\\x80\\xac' (first packet) or  '\\x80\\x22' (all others)
		Packet sequence number: in hex, padded to 2 bytes
		Packet time stamp: in hex, multiple of 10, padded to 4 bytes
		4-byte voice server 'nick name' assigned to us (synchronization source ID)
		2-byte delimiter: '\\x00\\x02' - the start of the RTP 'payload'
		96 bytes of TrueSpeech data  (a multiple of 32) 
		2-byte footer: '\\xff'+[any_hex_char]   ex: '\\xff\\xef'
		
		* For 88-byte packets, we send 64 bytes of TrueSpeech data in the 'payload'
		   instead of 96, and the footer is '\\xff\\xf7\\x00\\x00\\x00\\x00\\x81\\xca\\x00\\x1c'
	Here's an example RTP Sound packet:
	
	\x80\x22\x00\x0d\x00\x00\x16\x80\x09\x6d\xb6\xc7\x00\x02\xa1\x42\x52\x51\x5a\x62\x2c\x59\x19\x17\xba\x5a\x85\xbf\x98\x70\xa4\x90\xf1\xbc\xa5\x80\xfa\xda\xf4\xd1\x79\x9d\x34\x8d\x70\x9d\xdb\xc2\x41\x51\x2b\xb9\x52\x7b\xd3\x8a\x02\x26\x2b\x37\x10\x1b\xc5\x71\x64\xcf\x35\xaf\x0b\xb0\xb4\x38\x8c\x48\x55\x00\x0b\x77\xff\xe1

	\\x80\\x22\\x00\\x0d\\x00\\x00\\x16\\x80\\x09\\x6d\\xb6\\xc7\\x00\\x02\\xa1\\x42\\x52\\x51\\x5a\\x62\\x2c\\x59\\x19\\x17\\xba\\x5a\\x85\\xbf\\x98\\x70\\xa4\\x90\\xf1\\xbc\\xa5\\x80\\xfa\\xda\\xf4\\xd1\\x79\\x9d\\x34\\x8d\\x70\\x9d\\xdb\\xc2\\x41\\x51\\x2b\\xb9\\x52\\x7b\\xd3\\x8a\\x02\\x26\\x2b\\x37\\x10\\x1b\\xc5\\x71\\x64\\xcf\\x35\\xaf\\x0b\\xb0\\xb4\\x38\\x8c\\x48\\x55\\x00\\x0b\\x77\\xff\\xe1

	The total packet size sent to Yahoo, including UDP headers (handled by Python) and 
	our RTP packet is  156  bytes, according to Ethereal - for the 112-byte 
	packet model....total TCP size for 88-byte model: 124 bytes.  8-)
	"""

	#print "data64: "+str(len(data64))
	rbreak=96
	if eighty_byte_model==1: rbreak=64
	if not len(data64)==rbreak:   # Truespeech data must be length 96 0r 64 
		return ""
	global  record_sequence
	global chat_name_nick, r_starttime

	r_sequence="%04X" % record_sequence
	r_seq=binascii.unhexlify(r_sequence)

	if r_starttime==0:
		timestamp="%08d" % 0
		#r_starttime=time.time()
		r_starttime=480
		t_stamp=binascii.unhexlify(timestamp)
	else:
		newtime=(time.time()-r_starttime)*360  # is 360 right? guessing for now
		nmod=newtime%40  # for time to a multiple of 40
		newtime=abs(newtime-nmod+40)
		#timestamp="%08X" % int(newtime)
		timestamp="%08X" % int(r_starttime)
		r_starttime=r_starttime+480
		t_stamp=binascii.unhexlify(timestamp)
	dbprint( "sound-send TIME:  "+timestamp)
	if  record_sequence==2:
		phead="\x80\xac"  # \xac,\xa2
	else:  phead="\x80\x22"

	phead=phead+r_seq+t_stamp+chat_name_nick  +"\x00\x02"
	if chat_serial=="[PYCONFERENCE]":
		phead=phead+data64  +"\xff\xf7\x00\x00\x00\x00\x81\xca\x00\x1c"
	else:
		phead=phead+data64  +"\xff\xef"
	record_sequence=record_sequence+1
	dbprint( "sound-send: len- "+str(len(phead)) )
	dbprint( "sound-send:  "+str([phead])  )
	return phead


def recordForSend(*args):
	"""
	Used internally by the pyvoice.py library.  It is accessed by the thread launched from 
	'recording_on'.  It polls the microphone, recording sound and encoding it to 
	TrueSpeech.  TrueSpeech sound is then added to a sound buffer to be sent 
	to the Yahoo voice chat UDP server by another thread.
	"""
	global rsound_data, can_record
	t_started=0
	pytsp.esd_uninitPlayback()
	time.sleep(0.1)
	
	rsound_data=""

	while get_is_recording()==2:
		try:
			tfile=pytsp.tsp_record_sound(8193)  # 8193, 4097...8193 seems best
			if tfile=='':
				continue
			f=open(tfile)		
			ss=f.read()
			f.close()
			rsound_data=rsound_data+ss
			if t_started==0:
				if can_record==1:
					if len(rsound_data)>=1920:
						ts=Thread(None,recordSend)
						ts.start()
						t_started=1  
		except:
			pass
	# old debugging stuff
	"""print "len rsound:  "+str(len(rsound_data))
	f=open("rsound","w")
	f.write(rsound_data)
	f.close()
	f=open("tsp.header")
	ff=f.read()
	f.close()
	f=open("rsound.wav","w")
	f.write(ff)
	f.write(rsound_data)
	f.close() """
	pytsp.esd_uninitRecord()  
	rsound_data=rsound_data+"[PYVOICE_DONE]"
	if t_started==0:
		ts=Thread(None,recordSend)
		ts.start()
		t_started=1  


def recordSend(*args):
	"""
	Used internally by the pyvoice.py library.  This method is launched as a thread 
	from 'recordForSend'.  It checks the sound buffer at intervals.  If 
	a sufficient amount of recorded TrueSpeech sound is 
	available to be sent to Yahoo, this method encodes the data into RTP-format 
	using 'build_rec_packet' and forwards the data on to Yahoo's voice chat server 
	for other users to hear.  This thread quits automatically when the user has 
	stopped recording/sending sound from the microphone.

	For private conferences and PM conferences, the Yahoo! usernames in the member 
	list are of the structure:  'username@123.60.20.5:5000', because apparently when 
	in PM chat or private conference, we must send the sound to all the IP addresses 
	on our list, and Yahoo's servers only act as a 'registry' for all the users in the 
	conference (basically, just letting us know when people come in and leave the 
	voice conference.)  This is just a GUESS on my part.  Either way, I have had 
	no luck sending sound to Yahoo's servers when in 'conference' mode and others 
	actually hearing it, so for 'conference' mode this application sends the sound to all 
	IP addresses in the conference.  For regular chat room mode, sound is transmitted 
	to one of Yahoo's UDP voice servers.  For private conferences and PMs, sound 
	will be 'broadcast' to each IP address present on our list of voice chat room members.
	Conference mode may mean trouble for firewall users if they don't have their 
	firewall configured to send/receive UDP packets from the IP addresses of their friends 
	in the private conference or PM.
	"""
	global rsound_data, audio_packets_S
	time_to_quit=0
	last_pack=""
	ss=""
	rbreak=96
	if chat_serial=="[PYCONFERENCE]": rbreak=64
	if eighty_byte_model==1: rbreak=64
	preferred_block=rbreak*30   # use either 10, 20, or 30  (1280, 1920, 960, 640)
	while 1:
		ss=""
		if len(rsound_data) <preferred_block:  
			if rsound_data.find("[PYVOICE_DONE]")==-1:
				time.sleep(0.05)  # allow buffer to fill up some more
				continue
		if rsound_data.find("[PYVOICE_DONE]")>-1:
			ss=rsound_data[:rsound_data.find("[PYVOICE_DONE]")]
			rsound_data=rsound_data[rsound_data.find("[PYVOICE_DONE]")+len("[PYVOICE_DONE]"):]
			time_to_quit=1
		else:  
			ss=rsound_data[:preferred_block]
			rsound_data=rsound_data[preferred_block:]
		while len(ss)>=rbreak:
			feed=ss[:rbreak]
			#print "sound-send: ss:  "+str(len(ss))+"     feed:  "+str(len(feed))
			ss=ss[rbreak:]
			udp_feed=build_rec_packet(feed)
			if len(udp_feed)>0: 
				try:
					last_pack=udp_feed
					if chat_serial=="[PYCONFERENCE22]":  # PM or conference, broadcast to all IP addresses
						users=get_users_in_room()
						for uu in users:
							if uu.find("@")<1: continue
							uparts=uu.split("@")
							if len(uparts)<2:  continue
							ip_user=uparts[0]
							if ip_user.lower()==chat_user.lower():  continue  # don't send to ourselves
							server=uparts[1]
							if server.find(":")<1: continue
							ip_host=server.split(":")[0]
							ip_port=server.split(":")[1]
							if len(ip_host)>1:
								if len(ip_port)>1:
									try:
										udp_send(udp_feed,ip_host,int(ip_port))
									except:
										dbprint("broadcast-send failed: "+str(ip_host)+"   "+str(ip_port))
					else:
						udp_send(udp_feed)

					# old local debugging stuff
					# extract_tsp_sound(udp_feed)

					audio_packets_S=audio_packets_S+1
					dbprint ("UDP sound sent:  "+str(len(udp_feed))+",  packet no: "+str(audio_packets_S)  )
					#time.sleep(0.001)
				except:
					dbprint("ERROR in recordSend")
			else: dbprint( "udp_feed: fail" )
		if time_to_quit==1: break

	# send remainder if 64 bytes or more
	if len(ss)>=64:
	   try:
		feed=ss[:64]
		ss=ss[64:]
		udp_feed=build_rec_packet(feed)
		if len(udp_feed)>0: 
			udp_send(feed)
			audio_packets_S+1
			dbprint ("UDP sound sent:  "+str(len(udp_feed))+",  packet no: "+str(audio_packets_S)  )
	   except:
		pass 

	dbprint ("Sound send Thread quit" )
	dbprint( "sound-send: len- "+str(len(last_pack)) )
	dbprint( "sound-send:  "+str([last_pack])  )
	sendShuttingUp()

	# old debugging stuff
	# ff=open("tsp.header").read()
	# f=open("sound_data.wav","w")
	# f.write(ff)
	# f.write(sound_data)
	# f.close()



def sendShuttingUp():
	"""
	This basically sends the RTP packet to Yahoo's UDP server letting them know we have 
	stopped talking.  The packet is always 12 bytes (not 
	including UDP headers handled by Python) and is very simple in format:

		First 2 bytes:  '\\x80\\x22'
		Packet sequence number: in hex, padded to 2 bytes
		Packet time stamp: in hex, always equals 0, padded to 4 bytes
		4-byte voice server 'nick name' assigned to us (synchronization source ID)

		** This packet has no RTP 'payload'.

	Here's an example "I'm shutting up" packet:
		\x80\x22\x00\x54\x00\x00\x00\x00\x09\x6d\xb6\xc7

		\\x80\\x22\\x00\\x54\\x00\\x00\\x00\\x00\\x09\\x6d\\xb6\\xc7

	NOTE: This is very similar to the UDP 'login' string we send as our very first 
	packet in the method 'sendUDPLogin'

	The total packet size, including UDP headers (handled by Python) and our RTP 
	packet above is 54 bytes according to Ethereal.
	"""
	
	global  record_sequence, chat_name_nick

	timestamp="%08d" % 0
	t_stamp=binascii.unhexlify(timestamp)

	r_sequence="%04X" % record_sequence
	r_seq=binascii.unhexlify(r_sequence)

	phead="\x80\x22"+r_seq+t_stamp+chat_name_nick
	record_sequence=record_sequence+1				
	dbprint( "sound-send, shutting up: len- "+str(len(phead))  )
	dbprint( "sound-send, shutting up:  "+str([phead])  )
	udp_send(phead)


def setTCPOnly(tcpval):
	"""
	This method is assessed by the graphical user interface and takes on argument, 'tcpval' 
	(integer: 0 or 1).  If tcpval is set to 1, only TCP connections will be used to communicate
	with the voice servers; otherwise the connection will start with TCP, then use UDP for 
	all RTP communications on port 5000.
	"""
	global tcp_not_udp
	if tcpval==1: tcp_not_udp=1
	else: tcp_not_udp=0

def getTCPOnly():
	"""
	Returns a boolean showing whether or not we are using strictly TCP sockets for communications.
	"""
	global tcp_not_udp
	if tcp_not_udp==1: return 1
	return 0

def setEnableAutoChange(autoval):
	"""
	This method is assessed by the graphical user interface and takes on argument, 'autoval' 
	(integer: 0 or 1).  If autoval is set to 1, the current voice room will not be changed 
	automatically even if the configuration file suggests we should be in another voice chat room.
	"""
	global no_auto_server_change
	if autoval==1: no_auto_server_change=1
	else: no_auto_server_change=0

def getEnableAutoChange():
	"""
	Returns a boolean showing whether or not pY! Voice will automatically change voice 
	chat rooms when the configuration file has changed.
	"""
	global no_auto_server_change
	if no_auto_server_change==1: return 0
	else: return 1



	