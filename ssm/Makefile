CC = gcc
LD = gcc

CFLAGS = -Wall -g

DEPDIR = .deps
GENDEPFLAGS = -MMD -MP -MF $(DEPDIR)/$(@F).d

INCLUDES = -I./include

LDDIR	= -L./lib
LIBS	= -lisa-l -lsysfs

SRCS =	ssm.c \
	ssm_discovery.c \
	ssm_container.c \
	ssm_util.c \
	ssm_bitmap.c \
	ssm_erasure_code.c \
	ssm_io.c

OBJS = $(SRCS:.c=.o)
TARGET = $(firstword $(basename $(SRCS)))

all: $(TARGET)

$(shell mkdir $(DEPDIR) 2>/dev/null)
-include $(wildcard $(DEPDIR)/*)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(GENDEPFLAGS) -c -o $@  $<

$(TARGET): $(OBJS)
	${LD} -o $@ $^ ${LDDIR} ${LIBS}

clean:
	rm -f $(OBJS) $(TARGET)
	rm -rf $(DEPDIR)

