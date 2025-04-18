# -*- Makefile -*-

# $Id: platform_sunos5_g++.GNU 85039 2009-04-06 22:29:35Z shuston $

# SunOS 5.x (Solaris 2.x) with g++

# NOTE: On Solaris86, you'll need to use GNU as instead of /usr/ccs/bin/as,
#       if you want -gstabs+ and -pipe support.

debug ?= 1
optimize ?= 1
threads ?= 1
exceptions ?= 1
pipes ?= 1
buildbits ?= 32

ifeq ($(exceptions),0)
  CCFLAGS += -fno-exceptions
endif # exceptions

ifeq ($(shell /bin/uname -m),i86pc)
  #### gcc on Solaris86 doesn't use -g
  DCFLAGS         += -gstabs+
  DCCFLAGS        += -gstabs+
else  # ! i86pc
  DCFLAGS         += -g
  DCCFLAGS        += -g
endif # ! i86pc

CC              = gcc
CXX             = g++
CFLAGS          += -W -Wall -Wpointer-arith
CCFLAGS         += -W -Wall -Wpointer-arith

ifeq ($(threads),1)
  CPPFLAGS      += -D_REENTRANT
endif

DLD             = $(CXX)
LD              = $(CXX)
LDFLAGS         +=
LIBS            += -lsocket -ldl -lgen -lnsl -lposix4 -lthread
OCFLAGS         += -O3
OCCFLAGS        += -O3
PIC             = -fPIC
AR              = ar
ARFLAGS         = ruv
RANLIB          = @true
SOFLAGS         += -shared $(CPPFLAGS)
SOBUILD         = $(COMPILE.cc) $(PIC) -o $(VSHDIR)$*.so $<
PRELIB          = @true

# Get common Solaris settings
include $(ACE_ROOT)/include/makeinclude/platform_sunos5_common.GNU

# Test for template instantiation, add to SOFLAGS if SONAME set,
# add -E to LDFLAGS if using GNU ld
#
include $(ACE_ROOT)/include/makeinclude/platform_g++_common.GNU

CCFLAGS         += $(TEMPLATES_FLAG)

# Make sure we resolve all symbols at link time, see bugzilla 2710
LDFLAGS         += -z now

## Unfortunately, gcc 3.0 fails to many cpp files with
## optimization enabled.  Even -O causes the compiler
## to core dump on some files.
ifeq ($(shell $(CXX) --version),3.0)
  override optimize = 0
endif
