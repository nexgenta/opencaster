CC = gcc
CFLAGS = -Wall -g -mcpu=405 -msoft-float -mmultiple -mstring -meabi -pipe -Os -mno-fused-madd -static

OBJS = cwtest.o
TARGET = cwtest
DESTDIR = /usr/local/bin/

all: $(TARGET)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) 

install: all
	install -m 755 $(TARGET) $(DESTDIR) 

clean:
	rm -f $(TARGET) $(OBJS) core* *~ *.d

-include $(wildcard *.d) dummy
