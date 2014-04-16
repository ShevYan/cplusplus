CC = gcc
LD = gcc
INCLUDES =
CFLAGS = -Wall -g
LIBS = -lsysfs

SRC = ssm.c ssm_discovery.c
OBJ =$(SRC:.c=.o)
TARGET = $(firstword $(basename $(SRC)))

# all: $(TARGET)

.SUFFIXES: .c .o

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<

$(TARGET): $(OBJ)
	${LD} -o $@ $(OBJ) ${LIBS}

clean:
	rm -f $(OBJ) $(TARGET)
