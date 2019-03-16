TARGET = example

SRCS  = $(wildcard *.cpp)
OBJS  = $(SRCS:%.cpp=%.o)

CXXFLAGS = -g
CXXFLAGS += -std=c++11
CXXFLAGS += -Wall -Wextra -Werror -Wfatal-errors

CXXFLAGS += -I../library

CXXLIBS  = -L ../library -lpthread -lgomp -lmemory_debugger -lws2_32

all: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(CXXLIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $*.o $<

clean:
	$(RM) $(TARGET) $(OBJS) *~
