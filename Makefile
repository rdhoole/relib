CC = gcc
CFLAGS = -Wall -fPIC

LD = ld
LDFLAGS = -shared $(shell /usr/bin/python3.7-config --ldflags)

RM = rm -f

TARGET_LIB = relib.so

SRCS = relib.c
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}
$(LIB): $(OBJS)
	$(CC) ${CFLAGS} -o $^
$(TARGET_LIB): $(OBJS)
	$(LD) ${LDFLAGS} $^ -o $@

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS}
