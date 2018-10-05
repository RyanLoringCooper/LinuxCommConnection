CXX			= g++
LIBS			= -lpthread
CXXFLAGS		= -Wall -std=c++11 -g
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

.PHONY: all
all: tests | lib archive 

.PHONY: lib
lib: $(SRC_OBJS) copyHeaders
	@mkdir -p $(BUILD_DIR)$(LIB_TARGET)/lib
	@echo
	@echo Building shared object
	$(CXX) $(CXXFLAGS) -shared -o $(BUILD_DIR)$(LIB_TARGET)/lib/lib$(LIB_TARGET).so $(patsubst %.o, $(OBJ_DIR)%.o, $(SRC_OBJS)) $(LIBS)
	@echo Created shared object
	@echo

.PHONY: archive
archive: $(SRC_OBJS) copyHeaders
	@mkdir -p $(BUILD_DIR)
	@echo
	@echo Building archived lib
	ar rcs $(BUILD_DIR)$(LIB_TARGET)/lib/$(LIB_TARGET).a $(patsubst %.o, $(OBJ_DIR)%.o, $(SRC_OBJS))
	@echo Created archived lib 
	@echo

.PHONY: copyHeaders
copyHeaders:
	@mkdir -p $(BUILD_DIR)$(LIB_TARGET)/include/$(LIB_TARGET)
	@cp $(HEADERS) $(BUILD_DIR)$(LIB_TARGET)/include/$(LIB_TARGET)/
	@cp LICENSE.txt $(BUILD_DIR)$(LIB_TARGET)

.PHONY: tests
tests: $(TEST_BINS)
	@echo Compiled tests

.PHONY: install
install: lib
	mv $(BUILD_DIR)$(LIB_TARGET)/include/$(LIB_TARGET) /usr/include
	mv $(BUILD_DIR)$(LIB_TARGET)/lib/lib$(LIB_TARGET).so /usr/lib

.PHONY: uninstall
uninstall: 
	rm -rf /usr/include/$(LIB_TARGET)
	rm -rf /usr/lib/lib$(LIB_TARGET).so

%$(TEST_SUFFIX): $(TEST_OBJS) $(SRC_OBJS) 
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)$(BIN:$(TEST_SUFFIX)=) $(patsubst %.o, $(OBJ_DIR)%.o, $(patsubst %$(TEST_SUFFIX), %.o, $@)) $(patsubst %.o, $(OBJ_DIR)%.o, $(SRC_OBJS)) $(LIBS)

.PHONY: clean
clean:
	$(RM) -rf $(BUILD_DIR)
	$(RM) -rf $(OBJ_DIR)

%.o : %$(SRC_SUFFIX)
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -fPIC $< -o $(OBJ_DIR)$@ $(LIBS)
