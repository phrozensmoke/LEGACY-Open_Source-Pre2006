

import string

hex_map= {
	"^@":"\x00",
	"^A":"\x01",
	"^B":"\x02",
	"^C":"\x03",
	"^D":"\x04",
	"^E":"\x05",
	"^F":"\x06",
	"^G":"\x07",
	"^H":"\x08",
	"^I":"\x09",
	"^J":"\x0A",
	"^K":"\x0B",
	"^L":"\x0C",
	"^M":"\x0D",
	"^N":"\x0E",
	"^O":"\x0F",
	"^P":"\x10",
	"^Q":"\x11",
	"^R":"\x12",
	"^S":"\x13",
	"^T":"\x14",
	"^U":"\x15",
	"^V":"\x16",
	"^W":"\x17",
	"^X":"\x18",
	"^Y":"\x19",
	"^Z":"\x1A",
	"^[":"\x1B",
	"^\\":"\x1C",
	"^]":"\x1D",
	"^^":"\x1E",
	"^_":"\x1F",

}


def filter(somestr):
	s=somestr
	for ii in hex_map.keys():
		s=s.replace(ii,hex_map[ii])
	return s

def bind_tsp(somedict):
	s=""
	i=0
	header="\x91d\xaax"
	while i<len(somedict):
		src=somedict[i].replace("^","")
		if not len(src)==96:
			print "src len: "+str(len(s))
			#s=s+header+src
		else:
			s=s+src
		i=i+1
	return s



