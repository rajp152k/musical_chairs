/*
 * Program: Musical chairs game with n players and m intervals.
 *
 * Author:		    Raj PATIL		| AKASHDEEP SINGH
 * ROLL #:		    CS18BTECH11039	| CS18BTECH11003
 */

#include <stdlib.h>  /* for exit, atoi */
#include <iostream>  /* for fprintf */
#include <errno.h>   /* for error code eg. E2BIG */
#include <getopt.h>  /* for getopt */
#include <assert.h>  /* for assert */
#include <chrono>	/* for timers */
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <sstream>
using namespace std;
/*
 * Forward declarations
 */

void usage(int argc, char *argv[]);
unsigned long long musical_chairs(int nplayers);

using namespace std;


int main(int argc, char *argv[])
{
    int c;
	int nplayers = 0;

    /* Loop through each option (and its's arguments) and populate variables */
    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"help",            no_argument,        0, 'h'},
            {"nplayers",         required_argument,    0, '1'},
            {0,        0,            0,  0 }
        };

        c = getopt_long(argc, argv, "h1:", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
        case 0:
            cerr << "option " << long_options[option_index].name;
            if (optarg)
                cerr << " with arg " << optarg << endl;
            break;

        case '1':
            nplayers = atoi(optarg);
            break;

        case 'h':
        case '?':
            usage(argc, argv);

        default:
            cerr << "?? getopt returned character code 0%o ??n" << c << endl;
            usage(argc, argv);
        }
    }

    if (optind != argc) {
        cerr << "Unexpected arguments.\n";
        usage(argc, argv);
    }


	if (nplayers == 0) {
		cerr << "Invalid nplayers argument." << endl;
		return EXIT_FAILURE;
	}

    unsigned long long game_time;

	game_time = musical_chairs(nplayers);

    cout << "Time taken for the game: " << game_time << " us" << endl;

    exit(EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void usage(int argc, char *argv[])
{
    cerr << "Usage:\n";
    cerr << argv[0] << "--nplayers <n>" << endl;
    exit(EXIT_FAILURE);
}

//custom function declarations; definitions in the end

void setup(int);
void choose(int);
void shuffle_array(int nplayers);
void assign_velocity(int nplayers);
int  user_interact();
void set_U_sleep(int);
void set_P_sleep(int,int);
void output(int which_task, int nplayers, int id, int laps, int lap_no, unsigned long long time_taken);

struct Pinfo{
	//creating an array in heap that can be read by everyone
	int id;
	bool alive;
	bool sitting;
	int position;
	int velocity;
	int sleep_time;//set before every turn: in microseconds
};

struct Shared{//storage of common shared variables
	int NP;
	thread* Players;
	Pinfo* player_info;
	int chairs;
	int* chair_status;
	int standing_count;
	int umpire_sleep_dur;
};


struct Shared shared;

void umpire_main(int nplayers)
{
	int input;
	while(shared.NP>1){
		input = user_interact();
		if(input!=1){
			fprintf(stderr,"incorrect order of commands: ignoring\n");
			fprintf(stderr,"lap restarted\n");
			continue;
		}
		setup(shared.NP);
		//taking in player sleep times
		while(1){
			input = user_interact();
			if(input!= 6){
				break;
			}
		}
		if (input == !2){
			fprintf(stderr,"incorrect order of commands: ignoring\n");
			fprintf(stderr,"lap restarted\n");
			continue;
		}
		//music start command given
		//players waiting on a condition variable are broadcast

		input = user_interact();
		if(input==5){//if umpire sleep was called
			input = user_interact();
		}
		//umpire sleeps for the designated time: default is 0
		if(input!=3){
			fprintf(stderr,"incorrect order of commands: ignoring\n");
			fprintf(stderr,"lap restarted\n");
			continue;
		}
		//music stop detected
		//players start choosing
		//umpire kills the last standing player
		//waiting for lap_stop to be called
		input = user_interact();
		if(input != 4){
			fprintf(stderr,"incorrect order of commands: ignoring\n");
			fprintf(stderr,"lap restarted\n");
			continue;
		}
		//lap stop detected
		//cleanup procedures to be called
		shared.NP--;
	}
}

void player_main(int plid)
{
	0;
}

//all the relevant code is roots from musical_chairs
unsigned long long musical_chairs(int nplayers)
{
	srand(time(0));
	auto t1 = chrono::steady_clock::now();

	thread umpire(umpire_main,nplayers);
	shared.NP = nplayers;
	shared.player_info = new Pinfo[nplayers];
	shared.Players = new thread[nplayers];
	shared.chair_status = new int[nplayers-1];

	//first setup: creating the players
	for(auto i=0;i<nplayers;i++){
		shared.Players[i] = thread(player_main,i);
	}


	//waiting for players to join
	for(auto i=0;i<nplayers;i++){
		shared.Players[i].join();
	}
	//waiting for umpire to join
	umpire.join();
	auto t2 = chrono::steady_clock::now();
	auto d1 = chrono::duration_cast<chrono::microseconds>(t2 - t1);

	delete []  shared.Players;
	delete []  shared.player_info;
	delete []  shared.chair_status;

	return d1.count();
}

//UMPIRE FUNCTIONS
void setup(int n){
	//given the condition of n players and n-1 chairs
	//this randomly assigns the positions to the players
	//has global side-effects
	for(auto i=0;i<n;i++){
		shared.player_info[i].alive=true;
		shared.player_info[i].sitting=false;
	}
        shuffle_array(n);
        assign_velocity(n);
}
//cleanup called when standing count reaches 1
void cleanup(int n){//called when n players played the current round
	int killed_id=-1;
	for(auto i=0;i<n;i++){
		if(shared.player_info[i].sitting==false){
			killed_id = i;
			break;
		}
	}
	struct Pinfo temp;
	if(killed_id!=-1){
		//swapping killed player with last player alive so that
		//alive players occur first
		temp = shared.player_info[killed_id];
		shared.player_info[killed_id] = shared.player_info[n-1];
		shared.player_info[n-1] = temp;
		return;
	}
	else{
		fprintf(stderr,"undefined behaviour\n");
		return;
	}
}
void shuffle_array(int nplayers)
{
        int arr[nplayers-1];
        for(auto i=0; i<nplayers-1; i++)
                arr[i] = i;
        shuffle(arr, arr+nplayers-1, default_random_engine(0));
        for(auto i=0; i<nplayers-1; i++)
                shared.player_info[i].position = arr[i];
        // assigning positions from 0 to n-2 to n-1 players
        shared.player_info[nplayers-1].position = 0; //assigned postion 0 to last player
        return;
}
void assign_velocity(int nplayers)
{
        for(auto i=0; i<nplayers-1; i++)
                shared.player_info[i].velocity = ((shared.player_info[i].position)%2 == 0 ? 1 : -1);
        //assign velocity = 1 to players with even position and -1 to players with odd position
        shared.player_info[nplayers-1].velocity = -1;//last player has position 0, assigning velocity = -1
        //as atleast 1 player has to be on the opposite side of the chair
        return;
}
int  user_interact()
{
	int task_duration;
        int player_id;
        string for_stream;
        while(getline(cin, for_stream)){//takes input until EOF
                //string inside loop
                istringstream file_input(for_stream);
                string task;
                file_input >> task;
                if(task == "umpire_sleep"){
                        file_input >> task_duration;//taking inputs from stringstream
			set_U_sleep(task_duration);
			return 5;
                }
                else if(task == "player_sleep"){
                        file_input >> player_id >> task_duration;
			set_P_sleep(player_id,task_duration);
			return 6;
                }
                else if(task == "lap_start"){
			return 1;
                }
                else if(task == "music_start"){
			return 2;
                }
                else if(task == "music_stop"){
			return 3;
                }
                else if(task == "lap_stop"){
			return 4;
                }
                else{
			fprintf(stderr,"invalid command entered: ignored\n");
                }
        }
}

void set_U_sleep(int dur){
	shared.umpire_sleep_dur = dur;
}
void set_P_sleep(int id,int dur){
	shared.player_info[id].sleep_time = dur;
}
//PLAYER FUNCTIONS
void step(int i){//called on shared.player_info[i]
	//called as per lock step synchronization
	if(shared.player_info[i].position == shared.chairs-1 &&
	   shared.player_info[i].velocity == 1){
		shared.player_info[i].velocity=-1;
	}
	else if(shared.player_info[i].position == 0 &&
		shared.player_info[i].velocity == -1 ){
		shared.player_info[i].velocity=1;
	}
	else{
		shared.player_info[i].position +=
		shared.player_info[i].velocity;
	}
}
void choose(int i){//called on shared.player_info[i]
	//called as per lock step synchronization
	if(shared.chair_status[shared.player_info[i].position] ==-1 &&
	   shared.player_info[i].alive){
		if(shared.player_info[i].position % 2 ==1 &&
		   shared.player_info[i].velocity == 1){
			shared.chair_status[shared.player_info[i].position] = i;
			shared.player_info[i].sitting=true;
			shared.standing_count--;
		}
		if(shared.player_info[i].position % 2 ==0 &&
		   shared.player_info[i].velocity == -1){
			shared.chair_status[shared.player_info[i].position] = i;
			shared.player_info[i].sitting=true;
			shared.standing_count--;
		}
	}
}
void output(int which_task, int nplayers, int id, int laps, int lap_no, unsigned long long time_taken)
{
        if(which_task == 1)
                fprintf(stdout, "Musical Chairs: %d player game with %d laps.\n", nplayers, laps);
        else if(which_task == 2)
                fprintf(stdout, "======= lap# %d =======\n%d could not get chair\n**********************\n", lap_no, id);
        else if(which_task == 3)
                fprintf(stdout, "Winner is %d\n", id);
        else
                fprintf(stdout, "Time taken for the game: %llu", time_taken);
}
