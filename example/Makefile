TARGET = example

SRCS  = $(wildcard *.cpp)
OBJS  = $(SRCS:%.cpp=%.o)

CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -Wall -Wextra -Werror -Wfatal-errors

CXXFLAGS += -I../library -I../client

CXXLIBS  = -L ../library -lpthread -lgomp -lmemory_debugger -lws2_32

all: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(CXXLIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $*.o $<

Version.h:
	$(VERSION_CMD)

clean:
	$(RM) $(TARGET) $(OBJS)
