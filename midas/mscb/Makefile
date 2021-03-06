########################################################
#  Makefile for msc executable under linux
#
#  If the environment variable MIDASSYS is defined,
#  the mxml.c, strlcpy, and musbstd.c files are
#  taken from there. Otherweise the files from the 
#  mxml/ and midas/ subdirectories below 
#  the mscb directory are used.
#
#
#  $Id: Makefile 5260 2012-02-16 12:31:59Z ritt $
########################################################

# get OS type from shell
OSTYPE = $(shell uname)

ifeq ($(OSTYPE),Darwin)
OSTYPE = darwin
endif

ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

# directory where mxml.c and strlcpy.c resides
ifdef MIDASSYS
MXMLDIR       = $(MIDASSYS)/../mxml/
else
MXMLDIR       = ../../mxml/
endif

# directory where musbstd.h resides
ifdef MIDASSYS
MIDASINC      = $(MIDASSYS)/include
else
MIDASINC      = ../include
endif

# directory where musbstd.c resides
ifdef MIDASSYS
USBDIR        = $(MIDASSYS)/drivers/usb/
else
USBDIR        = ../drivers/usb/
endif

OUTNAME       = msc 
CC            = gcc -g -O0 -DHAVE_MRPC
FLAGS         = -Wall -Wuninitialized -I$(MXMLDIR) -I$(MIDASINC)

STRLCPY       = strlcpy.o

ifeq ($(OSTYPE),linux)
CC   += -DOS_LINUX -DHAVE_LIBUSB -DHAVE_USB -fpic
LIBS  = -lusb
endif

ifeq ($(OSTYPE),darwin)
CC   += -DOS_DARWIN -DOS_LINUX -DHAVE_USB -DHAVE_LIBUSB10
FLAGS += -I/usr/local/include/libusb-1.0
LIBS  = /usr/local/lib/libusb-1.0.a -framework IOKit -framework CoreFoundation
STRLCPY       =
endif

MSCB_OBJ      = msc.o mscb.o mscbrpc.o
MXML_OBJ      = mxml.o $(STRLCPY)
USB_OBJ       = musbstd.o
OBJECTS       = $(MSCB_OBJ) $(MXML_OBJ) $(USB_OBJ)

all: $(OUTNAME)

$(OUTNAME): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(OUTNAME) $(LIBS)

$(MSCB_OBJ): %.o: %.c
	$(CC) $(FLAGS) -c $<

$(MXML_OBJ): %.o: $(MXMLDIR)%.c
	$(CC) $(FLAGS) -c $<

$(USB_OBJ): %.o: $(USBDIR)%.c
	$(CC) $(FLAGS) -c $<

shared: $(OBJECTS)
	$(CC) -shared -o mscb.so $(OBJECTS) $(LIBS)

clean:
	rm *.o $(OUTNAME)

