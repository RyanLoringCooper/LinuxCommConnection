CXX 		= g++
CXXFLAGS 	= -Wall -std=c++11
OBJS		= NetworkConnection.cpp CommConnection.cpp
LIBS		= -lpthread
BUILD_DIR 	= build

nettest:
	if [ ! -d "$(BUILD_DIR)" ]; then mkdir build; fi
	$(CXX) $(CXXFLAGS) -o $(BUILD_DIR)/nettest NetworkConnectionTest.cpp -g $(OBJS) $(LIBS)

clean:
	$(RM) -rf build
