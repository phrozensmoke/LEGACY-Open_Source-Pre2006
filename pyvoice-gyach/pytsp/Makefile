include config.mak

# Generated automatically from Makefile.in by configure.
DEFINES=-DMPLAYER -D__WINE__ -Ddbg_printf=__vprintf \
	-DTRACE=__vprintf # -DDETAILED_OUT

LIB_OBJECTS= ldt_keeper.o pe_image.o module.o \
ext.o win32.o driver.o pe_resource.o \
resource.o registry.o elfdll.o afl.o vfl.o wrapper.o \
cpudetect.o mp_msg.o

# gcc-3.0 produces buggy code for acmStreamOpen() with
# "-O3 -fomit-frame-pointer" or "-O2 -fomit-frame-pointer
# -finline-functions -frename-registers" (code is OK with sole -O2),
# the bad code accesses parameters via %ebp without setting up a
# propper %ebp first!
# -fno-omit-frame-pointer works around this gcc-3.0 bug.  gcc-2.95.2 is OK.
# Note: -D_FILE_OFFSET_BITS=32 is required to disable using mmap64(),
# as it's broken in glibc 2.1.2 (bad header) and 2.1.3 (bad code)
# put in $(OPT_FLAGS) for super speed
WARN_FLAGS = -Wall
CFLAGS=-I. -I.. -U_FILE_OFFSET_BITS -D_REENTRANT $(EXTRA_INC) $(WARN_FLAGS) -fno-omit-frame-pointer -g3
#CFLAGS=-I. -I.. -O $(WARN_FLAGS) -g #-fno-omit-frame-pointer

all: libloader.a test

test: pytruespeech.o libloader.a


clean:
	-rm -f *.o libloader.a

distclean: clean

.c.o: $@
	$(CC) $(CFLAGS) $(DEFINES) -c $<

libloader.a:  $(LIB_OBJECTS) stubs.s
	$(CC) -c ./stubs.s -o stubs.o
	$(AR) -r libloader.a $(LIB_OBJECTS) stubs.o

dep:
	echo "dependency not required/supported"


