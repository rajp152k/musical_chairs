OS II: MUSICAL_CHAIRS ( HOMEWORK ASSIGNMNENT 2 )

RAJ PATIL		: 	CS18BTECH11039
AKASHDEEP SINGH KALRA	:	CS18BTECH11003

README 

Note the following makefile can be used to compile the project with optimized and non-optimized binaries.

```
#compilation directions

all: program o_program
#o_program is the -O2'ed binary

CXX = g++
CXXFLAGS = -std=c++11 -pthread

program: musicalchairs.cpp 
	$(CXX) $(CXXFLAGS) -o $@ $^

o_program: musicalchairs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ -O2
```

program is the normal binary and o_program is the optimized binary.
note that we have to compile the program with the provided flags as above.

outputting sample outputs for a small test case ( 4 players ) along with the compilation and running the binary.

# compilation:

rajp152k@Raj:~/links/source/musical_chairs/project$ make
g++ -std=c++11 -pthread -o program musicalchairs.cpp
g++ -std=c++11 -pthread -o o_program musicalchairs.cpp -O2

# running o_program (optimized) :
rajp152k@Raj:~/links/source/musical_chairs/project$ ./o_program --np 4 <inp.txt | cat > stdout.txt

usage : ./o_program --nplayers <enter number of players>

the contents on inp.txt and stdout.txt are as follows :

inp.txt:

lap_start
player_sleep 0 5000
player_sleep 1 2000
player_sleep 2 3000
player_sleep 3 4000
music_start
umpire_sleep 200
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
player_sleep 2 3000
music_start
umpire_sleep 200000
music_stop
lap_stop
lap_start
player_sleep 0 1000
player_sleep 1 2000
music_start
umpire_sleep 600000
music_stop
lap_stop


stdout.txt:

Musical Chairs: 4 player game with 3 laps.
======= lap# 1 =======
0 could not get chair
**********************
======= lap# 2 =======
2 could not get chair
**********************
======= lap# 3 =======
1 could not get chair
**********************
Winner is 3
Time taken for the game: 808069 us


NOTE: running the program without redirection and observing the output on the console will result in a much larger output as I am printing all the debugging printf's in the stderr filestream which is ignored by the autograder.
