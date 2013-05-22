#*********************************************************************
#                                                                              
#  Makefile
#
#  $Id: Makefile 236 2010-10-19 18:41:15Z olchansk $
#
#**********************************************************************
SHELL = /bin/sh
ARCH := $(shell $(ROOTSYS)/bin/root-config --arch)
LDFLAGS =

ifeq ($(ARCH),linux)
ARCH_FLAGS = -m32
OPT_FLAGS = -O2 -g
LDFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS) -Wl,-rpath,$(ROOTSYS)/lib
NEED_STRLCPY = 1
HAVE_DEPEND  = 1
HAVE_ROODYSO = 1
endif

ifeq ($(ARCH),linuxx8664gcc)
ARCH_FLAGS =
OPT_FLAGS = -O2 -g
LDFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS) -Wl,-rpath,$(ROOTSYS)/lib
NEED_STRLCPY = 1
HAVE_DEPEND  = 1
HAVE_ROODYSO = 1
endif

ifeq ($(ARCH),macosx) # MacOSX 10.3.9
ARCH_FLAGS =
OPT_FLAGS = -O2 -g
LDFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS)
endif

ifeq ($(ARCH),macosx64) # MacOSX 10.6.4
ARCH_FLAGS =
OPT_FLAGS = -O2 -g
LDFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS)
HAVE_DEPEND  = 1
endif

ifeq ($(LDFLAGS),)
$(error $(ARCH) unsupported architecture)
endif

CINT = $(ROOTSYS)/bin/rootcint

ROOTCFLAGS   := $(shell $(ROOTSYS)/bin/root-config --cflags)
ROOTGLIBS    := $(shell $(ROOTSYS)/bin/root-config --glibs)

HAVE_HBOOK    := $(shell cat $(ROOTSYS)/lib/libHbook.so 2> /dev/null)
HAVE_MINICERN := $(shell cat $(ROOTSYS)/lib/libminicern.so 2> /dev/null)
HAVE_SPECTRUM := $(shell cat $(ROOTSYS)/lib/libSpectrum.so 2> /dev/null)

ifdef HAVE_HBOOK
ROOTCFLAGS += -DHAVE_HBOOK
ROOTGLIBS  += -lHbook
ifdef HAVE_MINICERN
ROOTGLIBS  += -lminicern
endif
endif

ifdef HAVE_SPECTRUM
ROOTGLIBS  += -lSpectrum
endif

CXXFLAGS = $(ARCH_FLAGS) $(OPT_FLAGS) -Wall -Wuninitialized -fPIC $(ROOTCFLAGS)
INCLUDE  := -I. -Iinclude

HAVE_NETDIRECTORY := $(shell cat ../rootana/libNetDirectory/TNetDirectory.h 2> /dev/null)

ifdef HAVE_NETDIRECTORY
INCLUDE  += -DHAVE_NETDIRECTORY -I../rootana/libNetDirectory
ROOTGLIBS += -L../rootana/libNetDirectory -lNetDirectory
endif

OBJS = obj/main.o obj/DataSourceTDirectory.o obj/Roody.o obj/RoodyDict.o \
       obj/TGTextDialog.o obj/TGTextDialogDict.o \
       obj/TPeakFindPanel.o obj/TPeakFindPanelDict.o \
       obj/MTGListTree.o obj/MTGListTreeDict.o \
       obj/TAxisLimitsDialog.o obj/TAxisLimitsDialogDict.o \
       obj/RoodyXML.o obj/mxml.o

ifdef NEED_STRLCPY
CXXFLAGS += -DNEED_STRLCPY
OBJS += obj/strlcpy.o
endif

#------------------------------------------------------------------------------

.SUFFIXES:	.c .cxx .cpp .o .so

ALL = depend bin obj lib html/index.html bin/roody

ifdef HAVE_ROODYSO
ALL += lib/roody.so
endif

all:	$(ALL)

.PHONY: include/rootsys.h

bin:
	-mkdir -p bin

obj:
	-mkdir -p obj

lib:
	-mkdir -p lib

docs: html/index.html
	-cd doxfiles; doxygen roodydox.cfg

html/index.html:
	-cd doxfiles; doxygen roodydox.cfg

bin/roody:	obj/main.o $(OBJS)
	@echo "*** ROOTSYS = $(ROOTSYS) ***";
	g++ -o $@ $(LDFLAGS) $^ $(ROOTGLIBS) -lstdc++ 
	@echo "*** roody done ***"

lib/roody.so:	$(OBJS)
		@echo "*** ROOTSYS = $(ROOTSYS) ***";
		g++ -shared $(LDFLAGS) $^ -o $@
		@echo "*** roody.so done ***"

depend:
ifdef HAVE_DEPEND
	-g++ $(CXXFLAGS) $(INCLUDE) -c -MM src/*.cxx > Makefile.depends1
	sed 's#^#obj/#' Makefile.depends1 > Makefile.depends2
	sed 's#^obj/ #   #' Makefile.depends2 > Makefile.depends
	-rm -f Makefile.depends1 Makefile.depends2
endif

obj/main.o:			include/rootsys.h src/main.cxx

include/rootsys.h:
	echo char const *rootsys = \"$(ROOTSYS)\"\; > $@

obj/%Dict.o: include/%Dict.h

include/%Dict.h: src/%Dict.cxx

src/%Dict.cxx: include/%.h
	LD_LIBRARY_PATH=$(ROOTSYS)/lib $(CINT) -f $@ -c $^

install:	lib/roody.so
		@echo Installing $^ in $(ROOTSYS)/libs ...
		cp $^ $(ROOTSYS)/lib

clean:
		rm -v -f roody obj/* core src/*Dict.* lib/*.so include/rootsys.h src/rootsys.h html/*.html

obj/%.o: src/%.c
			gcc $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

obj/%.o: src/%.cxx
			g++ $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

obj/%.o: src/%.cpp
			g++ $(CXXFLAGS) $(INCLUDE) -c -o $@ $<

-include Makefile.depends

#end
