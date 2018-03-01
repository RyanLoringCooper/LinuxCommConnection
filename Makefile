CXX				= g++
LIBS			= -lpthread
CXXFLAGS		= -Wall -O2 -std=c++11 -g
SRC_SUFFIX		= .cpp
PRG_SUFFIX		= .bin
TEST_SUFFIX		= .test
HEADERS			= $(wildcard src/*.h)
SRCS			= $(patsubst src/%$(SRC_SUFFIX), %$(SRC_SUFFIX), $(wildcard src/*$(SRC_SUFFIX)))
SRC_OBJS		= $(SRCS:$(SRC_SUFFIX)=.o)
PRGS			= $(patsubst %$(SRC_SUFFIX), %, $(SRCS))
TESTS			= $(patsubst tests/%$(SRC_SUFFIX), %$(SRC_SUFFIX), $(wildcard tests/*$(SRC_SUFFIX)))
TEST_OBJS		= $(TESTS:$(SRC_SUFFIX)=.o)
TEST_PRGS		= $(patsubst %$(SRC_SUFFIX), %, $(TESTS))
BINS			= $(patsubst %, %$(PRG_SUFFIX), $(PRGS))
TEST_BINS		= $(patsubst %, %$(TEST_SUFFIX), $(TEST_PRGS))
OBJ_DIR			= obj/
BUILD_DIR		= bin/
LIB_TARGET		= LinuxCommConnection
VPATH			= tests src src/impls obj

.SECONDEXPANSION:
BIN = $@

all: tests | lib

.PHONY: lib
lib: $(SRC_OBJS)
	@echo
	@echo Building lib
	@mkdir -p $(BUILD_DIR)$(LIB_TARGET)/lib
	@mkdir -p $(BUILD_DIR)$(LIB_TARGET)/include
	ar rcs $(BUILD_DIR)$(LIB_TARGET)/lib/lib$(LIB_TARGET).so $(patsubst %.o, $(OBJ_DIR)%.o, $(SRC_OBJS))
	@cp $(HEADERS) $(BUILD_DIR)$(LIB_TARGET)/include
	@echo Create lib
	@echo

.PHONY: tests
tests: $(TEST_BINS)
	@echo Compiled tests

%$(TEST_SUFFIX): $(TEST_OBJS) $(SRC_OBJS) 
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)$(BIN:$(TEST_SUFFIX)=) $(patsubst %.o, $(OBJ_DIR)%.o, $(patsubst %$(TEST_SUFFIX), %.o, $@)) $(patsubst %.o, $(OBJ_DIR)%.o, $(SRC_OBJS)) $(LIBS)

clean:
	$(RM) -rf $(BUILD_DIR)
	$(RM) -rf $(OBJ_DIR)

%.o : %$(SRC_SUFFIX)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $(OBJ_DIR)$@ $< $(LIBS)
