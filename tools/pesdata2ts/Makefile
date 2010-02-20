C      = gcc
CFLAGS  = -g -MD -Wall -I. -I../../include 
LFLAGS  = -lc 

OBJS = pesdata2ts.o
TARGET = pesdata2ts
DESTDIR = /usr/local/bin/

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LFLAGS)

install: all
	install -m 755 $(TARGET) $(DESTDIR) 

clean:
	rm -f $(TARGET) $(OBJS) core* *~ *.d

-include $(wildcard *.d) dummy

