TARGET = memory_debugger.a
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)

CC=gcc

CCFLAGS  = -g
CCFLAGS += -Wall -Wextra -Werror -Wfatal-errors
CCFLAGS += -I .

all: $(TARGET)

$(TARGET): $(OBJS)
	ar rcs $@ $^
	ranlib $@
	cp memory_debugger.a memory_debugger.lib

.c.o:
	$(CC) $(CCFLAGS) -c -o $*.o $<

clean:
	$(RM) $(TARGET) $(OBJS)
