TARGET = memdbg

SRCS  = $(wildcard *.cpp)
OBJS  = $(SRCS:%.cpp=%.o)

CXXFLAGS =-g
CXXFLAGS += -std=c++11
CXXFLAGS += -Wall -Wextra -Werror -Wfatal-errors

# CXXFLAGS += -DDEBUG_ALLOC
CXXFLAGS += -I$(DEV_LIB_DIR)/freeglut/include

CXXLIBS   =
	VERSION_CMD = python git_version.py

CXXLIBS  += -lpthread -lgomp -lws2_32
CXXLIBS  += -L$(DEV_LIB_DIR)/freeglut/lib
CXXLIBS  += -lfreeglut
CXXLIBS  += -lopengl32
CXXLIBS  += -lglu32

all: Version.h $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(CXXLIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $*.o $<

Version.h:
	$(VERSION_CMD)

clean:
	$(RM) $(TARGET) $(OBJS) *.o *~ Version.h
