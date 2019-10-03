TOPDIR		= ..
SUBDIRS		= hd isdn ids
TARGETS		= touch $(LIBHD)
CLEANFILES	= $(LIBHD) $(LIBHD_D) $(LIBHD_SO) libhd.so*

.PHONY: touch

include $(TOPDIR)/Makefile.common

ifneq "$(findstring $(ARCH), i386 x86_64)" ""
SUBDIRS	+= int10
endif

#ifeq "$(findstring $(ARCH), i386 alpha)" ""
#SUBDIRS	:= $(filter-out pnpdump, $(SUBDIRS))
#endif

$(LIBHD): $(OBJS)


ifdef SHARED_FLAGS
$(LIBHD_SO): $(LIBHD)

else
$(LIBHD_SO):
endif

subdirs: touch

touch:
	@touch -r $(LIBHD) $(LIBHD_D) 2>/dev/null || true
