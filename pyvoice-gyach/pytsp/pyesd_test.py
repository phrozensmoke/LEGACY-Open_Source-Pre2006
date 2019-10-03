#! /usr/bin/env python

import pyesd
import copy

##  suggested recording params:  rate=22050/44100, channels=1, bits=16 (stereo)
## mono (bits=8) does not tend to work for most mics
## playback  :  esdcat  rec.raw -r 22050 -b 

if __name__ == '__main__' :
	#f=open("pcmout.raw")
	ii=0
	l=[]
	j=0
	ss=""
	#l.append(f.read())
	#f.close()
	#print ff[572000:len(ff)]
	#ii=pyesd.esd_play_sound_file("pcmout.raw", 4096, 2, 16)
	print "BUFFER:  "+str(pyesd.get_record_buffer_size())
	print "Recording..."
	
	ss=""+pyesd.esd_record_sound(150000,22050,1,16)
	print "Done."
	print "record len:  "+str(len(ss))
	#print "SS:  "+ss
	f=open("rec.raw","w")
	f.write(ss)
	f.flush()
	f.close()
	#pyesd.cvar.MYBUFF=l[0]
	#print "CVAR:  "+pyesd.cvar.MYBUFF

	print "ii:  "+str(ii)
	#pyesd.print_args(l[0])
	print "ANY ERRORS:  "+pyesd.get_pyesd_error_msg()

