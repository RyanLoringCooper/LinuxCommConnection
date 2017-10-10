CXX				= g++
LIBS			= -lpthread
CXXFLAGS		= -Wall -O2 -std=c++11 $(LIBS)
SRCS			= NetworkConnection.cpp CommConnection.cpp
OBJS			= $(SRCS:.cpp=.o)
OBJ_DIR			= obj/
BUILD_DIR		= build/
VPATH			= tests

all: PointerExample
	@echo Compiled all targets

PointerExample: $(OBJS) PointerExample.o
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)PointerExample $(patsubst %.o, $(OBJ_DIR)%.o, $(OBJS)) $(OBJ_DIR)PointerExample.o

clean:
	$(RM) -rf $(BUILD_DIR)
	$(RM) -rf $(OBJ_DIR)

%.o : %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)$@ $<

