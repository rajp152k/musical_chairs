/* Custom header files
 * contains implementation of
 *	Player
 *	Umpire
 */

#include <iostream>
#include <thread>
#include <unistd.h>
#include <vector>
using namespace std;


//variable volatility will be decided at the time of writing threaded code
class Shared{
	/*dynamic memory handled by the constructor and destructor of this
	 * class*/
public:
	vector <int> chairs;//dynamic size
	vector <int> player;//all players are a part of this vector
	bool MUSIC;//umpire toggles this
	int num_seated;//manipulated by everyone 
	int total_alive;//only umpire handles this 
};

class Player{
private:
	int id=-1;
	bool alive = true;
	bool sitting = false;
	int position = -1;
	int velocity=1;
	//each player also has access to  the number of chairs present : N
	int* volatile N;

	//will be setting as a pointer to a volatile int
public:
	void step(){
		if(velocity==1 && position==(*N-1)){
			velocity = -1;
		}
		if(velocity==-1 && position==0){
			velocity=1;
		}
		position+=velocity;
	}
	void run(){
		if(alive){
			sitting=false;
			while(1){
				step();
			}
		}
	}
	void try_seat(){
		if(alive){
			//access to odd chair numbers if velocity=1
			//as they are stepping back
			//everyone starts out clockwise
			//explained in report
			//and even ones if it is -1
			if(velocity==1){
				if(position%2==1){
					//check of that position is available
					//if not do nothing and continue
					//stepping backward
					//update position in common structure
					sitting = true;//will be nested in a loop
					// increment a common volatile var by 1
					//else continue stepping e
				}
			}
			else{
				if(position%2==0){
					//the same control flow as above
					sitting=true;
					//else continue stepping
				}
			}
		}
	}
	void find_seat(){
		velocity*=-1;//steps in the other direction: explained in report
		try_seat();//first try before stepping back once.
		while(!sitting){
			step();//steps back as velocity is reversed
			try_seat();
		}
		//is killed if the common var reaches N-1 and the rest is
		//cleanup by the umpire
	}
};
