INCLUDES = -I./include
TARGETS = usbasp_send list send
CC = gcc
CFLAGS = -Wall -g
LIBS = -lusb-1.0 -L.

SRCS = src/usbasp_spi.c examples/send_usbasp.c examples/list.c
OBJS = $(SRCS:.c=.o)


all: $(OBJS) usbasp_send usb_list

libusbasp.a: src/usbasp_spi.o
	ar rcs libusbasp.a $^

usbasp_send: libusbasp.a examples/send_usbasp.o
	$(CC) $(CFLAGS) $(INCLUDES) -o usbasp_send $^ $(LFLAGS) $(LIBS) -lusbasp

usb_list: examples/list.o
	$(CC) $(CFLAGS) $(INCLUDES) -o list $^ $(LFLAGS) $(LIBS) 


.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	rm -f *.o *.d $(TARGETS) $(OBJS) libusbasp.a
