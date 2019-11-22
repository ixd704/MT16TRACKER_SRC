TARGET = libjsoncpp.so
CC = g++
CFLAGS = -H -Wall -O2 -fPIC

lib:
	$(CXX) -olibjsoncpp.so -shared -fPIC *.cpp -I.



.PHONY:	default all clean

default:	$(TARGET)
all:	default
	



.PRECIOUS:	$(TARGET) 


$(TARGET):	
	$(CC) $(CFLAGS) -Wall -shared -o $@

clean:
	-rm -f $(TARGET)

install:
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/include
	for i in $(TARGET) ; do cp -a $$i $(DESTDIR)/lib ; done
	cp -a json/ $(DESTDIR)/include/

