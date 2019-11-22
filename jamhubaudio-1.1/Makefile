CFLAGS   = -Wall
CXXFLAGS = -g -Wall -fpermissive -L. -I/home/pavel/upgrade/ltib/rootfs/usr/lib/glib-2.0/include -I/home/pavel/upgrade/ltib/rootfs/usr/include/glib-2.0 -I/opt/build/ltib/rootfs/usr/include/libxml2 -I/opt/build/ltib/rootfs/usr/include/gstreamer-0.10 

LDFLAGS = -L. -L/home/pavel/upgrade/ltib/rootfs/usr/lib -lz -lxml2 -lglib-2.0 -lgobject-2.0 -lgthread-2.0 -lgmodule-2.0 -ljsoncpp -lprotobuf-c
#-lgstreamer-0.10 -lz -lxml2 -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgstapp-0.10 -lgthread-2.0 -ljansson -lprotobuf-c

ifneq ($(HOST_RESOURCE_DIR),)
CXXFLAGS += -I$(HOST_RESOURCE_DIR)/include
LDFLAGS += -L$(HOST_RESOURCE_DIR)/lib
CXXFLAGS +=-I/usr/include/glib-2.0 -I/usr/include/glib-2.0/include -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ 
endif

OBJECTS = tracker.pb-c.o
SOURCES = $(shell echo *.cpp)
OBJECTS += $(SOURCES:.cpp=.o)

ifdef SYSROOT
CFLAGS   += -I$(SYSROOT)/$(prefix)/include
CXXFLAGS += -I$(SYSROOT)/$(prefix)/include
LDFLAGS  += -L$(SYSROOT)/$(prefix)/lib
endif

progs    =  audio
prefix   =  /usr
DESTDIR  =

all :  $(progs)

install : $(DESTDIR)/$(prefix)/bin
	cp -a $(progs) $(DESTDIR)/$(prefix)/bin/
    
distclean clean :
	rm -f $(progs)
	rm -f *.o
	rm -f tracker.pb-c.h tracker.pb-c.c


$(DESTDIR)/$(prefix)/bin:
	mkdir -p $@

pre-build:
	protoc-c tracker.proto --c_out=.

audio.o: audio.cpp
	$(CXX) -c $(CXXFLAGS) -fpermissive -o audio.o $<

tracker.pb-c.o: tracker.pb-c.c
	$(CXX) -c $(CXXFLAGS) -o tracker.pb-c.o $<

$(progs): $(OBJECTS)
	$(CXX) $(CXXFLAGS)  -o $(progs) $(OBJECTS) $(LDFLAGS)

tracker.pb-c.c: pre-build

