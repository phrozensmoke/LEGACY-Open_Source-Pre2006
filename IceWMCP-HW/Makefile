TOPDIR		= $(CURDIR)
SUBDIRS		= src
TARGETS		= hwscan hwinfo user
CLEANFILES	= hwinfo hwinfo.static hwscan hwscan.static doc/libhd doc/*~
LIBDIR		= /usr/lib

include Makefile.common

SHARED_FLAGS	=
OBJS_NO_TINY	= names.o parallel.o modem.o

.PHONY:	fullstatic static shared tiny doc diet tinydiet uc tinyuc

hwscan: 
	gcc -c -Wall -O2 -I./src/hd hwscan.c
	#hwscan.o 

hwinfo: 
	gcc -c -Wall -O2 -I./src/hd hwinfo.c
	#hwinfo.o 

user:
	./Build_User
	./CopyOver_User

dev:
	./Build
	./CopyOver

# kept for compatibility
shared:
	@make

tiny:
	@make EXTRA_FLAGS=-DLIBHD_TINY SHARED_FLAGS=

diet:
	@make CC="diet gcc" EXTRA_FLAGS="-fno-pic -DDIET" SHARED_FLAGS=

tinydiet:
	@make CC="diet gcc" EXTRA_FLAGS="-fno-pic -DLIBHD_TINY -DDIET" SHARED_FLAGS=

uc:
	@make CC="/opt/i386-linux-uclibc/usr/bin/gcc" EXTRA_FLAGS="-fno-pic -DUCLIBC" SHARED_FLAGS=

tinyuc:
	@make CC="/opt/i386-linux-uclibc/usr/bin/gcc" EXTRA_FLAGS="-fno-pic -DLIBHD_TINY -DUCLIBC" SHARED_FLAGS=


install:
	install -m 644 ./hardware-ids/hd.ids /var/lib/hardware/
	echo "Running PyInstallShield Installer"
	./INSTALL/PyInstallShield


