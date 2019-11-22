TARGET = menu
LIBS = -lm -lpthread
CC = g++
CFLAGS = -g -Wall -O2 -fpermissive -I.
LDFLAGS = -lprotobuf-c -ljsoncpp

ifneq ($(HOST_RESOURCE_DIR),)
CFLAGS += -I$(HOST_RESOURCE_DIR)/include
LDFLAGS += -L$(HOST_RESOURCE_DIR)/lib
endif

ifneq ($(HAVE_LCD_EMULATION),)
CFLAGS += -DUSE_CURSES_LCD
LDFLAGS += -lncurses
endif


.PHONY:	default all clean

default:	$(TARGET)
all:	default
	
OBJECTS = tracker.pb-c.o
OBJECTS += $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o:	%.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@

%.o:	%.cpp $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -c $< -o $@



.PRECIOUS:	$(TARGET) $(OBJECTS)


$(TARGET):	$(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)
	-rm -f tracker.pb-c.h tracker.pb-c.c

install:
	mkdir -p $(DESTDIR)/bin
	for i in $(TARGET) ; do cp -a $$i $(DESTDIR)/bin ; done

tracker.pb-c.c: gen-protobuf-files

gen-protobuf-files:
	protoc-c -I.. ../tracker.proto --c_out=.

tracker.pb-c.o: tracker.pb-c.c
	$(CXX) -c $(CFLAGS) -o tracker.pb-c.o $<

