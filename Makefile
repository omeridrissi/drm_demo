CC      := gcc
CFLAGS  := -Wall -Wextra -O2 -I/usr/include/drm
LDFLAGS :=
LDLIBS  := -ldrm

SRCS    := $(wildcard *.c)
OBJS    := $(SRCS:.c=.o)
TARGET  := main

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

# Auto-dependency generation
-include $(OBJS:.o=.d)
