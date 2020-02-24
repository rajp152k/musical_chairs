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
#include <vector>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>

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
void choosing(int);
int  user_interact();
void set_U_sleep(int);
void set_P_sleep(int,int);
void output(int which_task,
	    int nplayers,
	    int id,
	    int laps,
	    int lap_no,
	    unsigned long long time_taken);
void step_back(int );

const int lpst_mcst=1;//b/w lap_start and music_start
const int mcst_mcsp=2;//b/w music_start and music_stop
const int mcsp_lpsp=3;//b/w music_stop and lap_stop
const int lpsp_lpst=0;//b/w lap_stop and lap_start

//defining constant states for the program: four distinct states

int exec_state=lpsp_lpst;//initial state
//changed by the current thread in control at different times

struct Pinfo{
	//creating an array in heap that can be read by everyone
	int id;
	bool alive;
	bool sitting;
	int position;
	int sleep_time=0;//set before every turn: in microseconds
};

struct Shared{//storage of common shared variables
	int NP;
	thread* Players;
	Pinfo* player_info;
	int chairs;//number of chairs available
	int* chair_status;
	int umpire_sleep_dur=0;

	atomic_flag umpire_go = ATOMIC_FLAG_INIT;
	int last_standing;
	int standing_count;
	mutex shared_mtx;
	mutex ready_mtx;
	mutex go_mtx;
	condition_variable ready;
	condition_variable go;
};

struct Shared shared;

void umpire_main(int nplayers)
{
	int input;
	while(shared.NP>1){
		printf("shared.NP : %d\n",shared.NP);
		shared.go.notify_all();//wasted for the first time
		while(shared.umpire_go.test_and_set()){
			//wait till all are standing
			0;
		}
		shared.umpire_go.clear();
		input = user_interact();


		exec_state = lpst_mcst;//was lpsp_lpst
		printf("exec state: %d\n",exec_state);
		//taking in player sleep times
		while(1){
			input = user_interact();
			if(input!= 6){
				break;
			}
		}

		//input ==2 was already read

		exec_state = mcst_mcsp;
		printf("exec state: %d\n",exec_state);
		//music start command given
		//storing umpire sleep if given
		input = user_interact();
		if(input==5){//if umpire sleep was called
			input = user_interact();
		}
		//umpire sleeps for the designated time: default is 0
		//stored in a variable for later use

		//input ==3 was already read



		exec_state = mcsp_lpsp;
		printf("exec state: %d\n",exec_state);
		//music stop detected
		shared.ready.notify_all();
		//players start choosing
		shared.umpire_go.test_and_set();
		//waiting till the last standing player
		//umpire sleeps
		this_thread::sleep_for(chrono::microseconds(shared.umpire_sleep_dur));
		//players start choosing
		while(shared.umpire_go.test_and_set()){
			//cleared from one of the choosing calls
			0;
		}
		shared.umpire_go.clear();
		//one standing player has now killed itself
		//stored in shared.last_standing
		shared.NP--;
		shared.chairs--;
		shared.last_standing=-1;
		shared.umpire_sleep_dur=0;
		for(auto i=0;i<shared.chairs;i++){
			shared.chair_status[i]=-1;
		}
		//waiting for lap_stop to be called
		
		input = user_interact();
		//input ==4 was read
		//lap stop detected
		exec_state=lpsp_lpst;
		printf("exec state: %d\n",exec_state);
	}
	printf("game over\n");
}

void player_main(int plid){

	//alive when inside the while loop
	//exits when dies
	printf("created player %d\n",plid);
	unique_lock<mutex> shr_mutex(shared.shared_mtx);
	shr_mutex.unlock();
	unique_lock<mutex> ready_mutex(shared.ready_mtx);
	ready_mutex.unlock();
	unique_lock<mutex> go_mutex(shared.go_mtx);
	go_mutex.unlock();
	//acquiring and releasin locks for declaration
	while(1){
		//go condition var is notified when state is 0
		printf("player %d is setting up\n",plid);
		shared.player_info[plid].sitting=false;
		//standing at random positition
		shared.player_info[plid].position = rand()%shared.chairs;
		//waiting on ready till music stops
		ready_mutex.lock();
		//waiting to be notified when music stops
		shared.standing_count++;
		if(shared.standing_count==shared.NP){
			shared.umpire_go.clear();
			//umpire goes after last one stood up
		}
		printf("standing count :%d \n",shared.standing_count);
		//current state is lpsp_lpst
		shared.ready.wait(ready_mutex,[&]{
				  return (exec_state==mcsp_lpsp);
				  });
		ready_mutex.unlock();
		//players start choosing after sleeping for this long
		this_thread::sleep_for(chrono::microseconds(shared.player_info[plid].sleep_time));
		shared.player_info[plid].sleep_time=0;
		printf("player %d is choosing\n",plid);
		choosing(plid);
		if(!shared.player_info[plid].sitting){
			shared.player_info[plid].alive=false;
			printf("player %d lost\n",plid);
			shared.umpire_go.clear();
			shr_mutex.lock();
			shared.standing_count--;
			shr_mutex.unlock();
			break;
			//exits while loop if lost
			//and joins back to main thread of execution
			//waiting for the winner to be declared
		}
		//once they have chosen, they start sleeping on go condition variable
		printf("player %d proceeded\n",plid);
		shr_mutex.lock();
		if(shared.NP==2){//if the player proceeded when 2 were remaining
			// the player won
			printf("player %d won\n",plid);
			shr_mutex.unlock();
			break;
		}
		shr_mutex.unlock();
		//do no wait if last one alive
		go_mutex.lock();
		//current exec state is mcsp_lpsp
		shared.go.wait(go_mutex,[&]{
			       return (exec_state==lpsp_lpst);
			       });
		go_mutex.unlock();
		//umpire signalled by last standing from the choosing call
		//notified when the next music_start is read
		//lap_stop read after the above wait
	}
	printf("player %d exited the loop\n",plid);
}


unsigned long long musical_chairs(int nplayers)
{
	srand(time(0));
	auto t1 = chrono::steady_clock::now();
	shared.umpire_go.test_and_set();
	//as first all players standup and get ready
	thread umpire(umpire_main,nplayers);
	shared.NP = nplayers;
	shared.chairs = nplayers-1;
	shared.player_info = new Pinfo[nplayers];
	shared.Players = new thread[nplayers];
	shared.chair_status = new int[nplayers-1];
	for(auto i=0;i<shared.chairs;i++){
		shared.chair_status[i]=-1;
	}
	//first setup: creating the players
	for(auto i=0;i<nplayers;i++){
		shared.Players[i] = thread(player_main,i);
	}


	//waiting for players to join
	for(auto i=0;i<nplayers;i++){
		shared.Players[i].join();
		printf("player %d joined back\n",i);
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
void choose(int i){//called on shared.player_info[i]
	int pos = shared.player_info[i].position;
	//pointless in having a mutex for every chair as have to
	//manipulate standing count at a point
	if(shared.chair_status[pos]==-1){
		//check if chair is available
		shared.player_info[i].sitting=1;//sit
		shared.chair_status[shared.player_info[i].position]=i;
		shared.standing_count--;
		printf("player %d got a chair\n",i);
		printf("standing_count = %d\n",shared.standing_count);
		//decrement standing count
	}
}

void choosing(int i){//called on shared.player_info[i]
	while(!shared.player_info[i].sitting){
		unique_lock<mutex> shr_mutex(shared.shared_mtx);
		if(shared.standing_count==1){
			shared.last_standing=i;
			//storing to be modified
			printf("player %d was last\n",i);
			shr_mutex.unlock();
			break;
			//no point in choosing if last one standing
		}
		choose(i);
		shr_mutex.unlock();
		//if unsuccesful, step-back and try again
		if(!shared.player_info[i].sitting){
			step_back(i);
		}
	}
}

void step_back(int i){
	//steps back(forward is anticlockwise)
	shared.player_info[i].position = (++shared.player_info[i].position %
					  shared.chairs);
}

void output(int which_task,
	    int nplayers,
	    int id,
	    int laps,
	    int lap_no,
	    unsigned long long time_taken)
{
        if(which_task == 1)
                fprintf(stdout, "Musical Chairs: %d player game with %d laps.\n"
			,nplayers, laps);
        else if(which_task == 2)
                fprintf(stdout, "======= lap# %d =======\n%d could not get chair\n**********************\n"
			, lap_no, id);
        else if(which_task == 3)
                fprintf(stdout, "Winner is %d\n", id);
        else
                fprintf(stdout, "Time taken for the game: %llu", time_taken);
}


