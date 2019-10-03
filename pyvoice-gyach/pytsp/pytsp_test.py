#! /usr/bin/env python

import pytsp
import copy,time

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
	#pytsp.esd_play_sound_file("aggy.raw", 8000, 2, 16)
	#print "BUFFER:  "+str(pytsp.get_record_buffer_size())
	#print "Recording..."
	
	#ss=""+pytsp.esd_record_sound(150000,22050,1,16)
	print "doing init"
	pytsp.pytsp_init()
	#ss=pytsp.compress_raw_to_tsp("pcmout.raw", 8000, 16, 1)
	#ss=pytsp.esd_play_sound_file("/tmp/pytsp_raw_storage_root",  8000, 16, 1)
	#ss=pytsp.decompress_tsp_to_esd("/tmp/pytsp_tsp_storage_root",  8000, 8, 1)
	#ss=pytsp.decompress_tsp_to_esd("/tmp/pytsp_tsp_record_root",  8000,16, 1)
	#time.sleep(3)
	reci=0
	if reci==0:
		ss=pytsp.decompress_tsp_to_esd("/tmp/pytsp_tsp_record_root",  8000,16, 1)


		#time.sleep(3)
		#ss=pytsp.decompress_tsp_to_esd("truespeechout.raw",  8000,16, 1)

	print "TSP file:  "+pytsp.get_tsp_storage_file()
	print "RAW file:  "+pytsp.get_raw_storage_file()


	#tfile=pytsp.tsp_record_sound(16320)
	if reci==1:
		time.sleep(1)
		print "Recording..."
		tfile=pytsp.tsp_record_sound(50000)
		print "tfile:  "+str(tfile) 
		print "Done."  
	#print "record len:  "+str(len(ss))
	print "SS:  "+str(ss)
	print "doing uninit"

	#f=open("rec.raw","w")
	#f.write(ss)
	#f.flush()
	#f.close()
	#pyesd.cvar.MYBUFF=l[0]
	#print "CVAR:  "+pyesd.cvar.MYBUFF

	print "ii:  "+str(ii)
	#pyesd.print_args(l[0])
	print "ANY ERRORS:  ESD: "+pytsp.get_pyesd_error_msg()+"   TSP:  "+pytsp.get_pytsp_error_msg()
	pytsp.pytsp_uninit()

