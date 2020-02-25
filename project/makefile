#compilation directions


all: program o_program
#o_program is the -O2'ed binary

CXX = g++
CXXFLAGS = -std=c++11 -pthread

program: musicalchairs.cpp 
	$(CXX) $(CXXFLAGS) -o $@ $^

o_program: musicalchairs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -O2

