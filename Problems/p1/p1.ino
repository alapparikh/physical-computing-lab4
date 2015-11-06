/*
/*
 Time Waster Wars: Reddit vs. 4chan

 a. Add synchronization primitives to ensure that
		(a) the club is exclusively redditors or 4channers, i.e. no redditors
				should enter as long as there are 4channers in the club,
				and vice versa,
		(b) the club should always be used as long as there are
				customers

		Note that starvation is not something you need to worry
		about. If the club becomes redditors and remains exclusively
		redditors for all time, the waiting 4channers will just have
		to get old at the door.

 Modify only the code of the class Club to make the program
 correct. (but add the LED manipulation to the other classes)

 Place your synchronization variables inside the Club instance.
 */

#include <proc.h>


void hang_out() {
	delay(random(300,1500)); //waste time
}

class Club {
	
	/* We have declared one lock and one condition
	 * variable here.  You may (or may not) find it
	 * useful to declare additional condition variables
	 * to solve the problem.  We just left these here as
	 * a reminder on the syntax. */
	Lock *_l;
	Cond *_cr; 
	Cond *_c4; 

	int num_redditors;
	int num_4channers;

	public:
		Club () {  //Constructor for club
			_l = new Lock();
			_cr = new Cond(_l);
			_c4 = new Cond(_l);
			num_redditors = 0;
			num_4channers = 0;
		}

		~Club () { //Destructor for club
			delete _l;
			delete _cr;
			delete _c4;
		}

		void reinit() {
			// This shouldn't be necessary, but it doesn't work without it
			_l = new Lock();
			_cr = new Cond(_l);
			_c4 = new Cond(_l);
		}
		
		void redditor_enter() {
			/*TODO: make it so that this function only returns when it is safe for
			 *      redditor to enter the club. */

			_l->lock();
			if(num_4channers > 0) {
				_l->unlock();
				_cr->wait();
			}
			num_redditors++;
			if(_cr->waiting()) {
				_cr->signal();
			} else {
				_l->unlock();
			}
		}

		void redditor_exit() {
			/*TODO: Exit the club. */
			_l->lock();
			num_redditors--;
			if(num_redditors > 0){
				_l->unlock();
			} else {
				if(_c4->waiting()){
					_c4->signal();
				} else {
					_l->unlock();
				}
			}
		}

		void fourchanner_enter() {
			/*TODO: make it so that this function only returns when it is safe for
			 *      redditor to enter the club. */
			_l->lock();
			if(num_redditors > 0) {
				_l->unlock();
				_c4->wait();
			}
			num_4channers++;
			if(_c4->waiting()) {
				_c4->signal();
			} else {
				_l->unlock();
			}
		}

		void fourchanner_exit() {
			/*TODO: Exit the club. */
			_l->lock();
			num_4channers--;
			if(num_4channers > 0){
				_l->unlock();
			} else {
				if(_cr->waiting()){
					 _cr->signal();
				} else {
					_l->unlock();
				}
		 	}
		}
};

Club daclub;
int columns [5] = {0,0,0,0,0};

void printColumns() {
	for(int i = 0; i < 5; i++){
		Serial.print(columns[i]);
	}
	Serial.println();
}

class Redditor: Process {
	int _id;

public:
	Redditor (int id) { 
		_id = id; 
	}

	void loop () {
		daclub.redditor_enter();
		Serial.print("Redditor ");
		Serial.print(_id);
		Serial.println(": in the club");
		/* TODO: light up column #_id on the LED matrix */
		columns[_id-1] = 1;
		Serial.print("1a");
		printColumns();
		hang_out();
		Serial.print("1b");
		columns[_id-1] = 0;
		printColumns();
		daclub.redditor_exit();
	}
};

class Fourchanner: Process {
	int _id;

public:
	Fourchanner (int id) { 
		_id = id; 
	}

	void loop () {
		daclub.fourchanner_enter();
		Serial.print("Fourchanner ");
		Serial.print(_id);
		Serial.println(": in the club");
		/* TODO: light up column #_id on the LED matrix */
		columns[_id-1] = 1;
		Serial.print("2a");
		printColumns();
		hang_out();
		columns[_id-1] = 0;
		Serial.print("2b");
		printColumns();
		daclub.fourchanner_exit();
	}
};


// the setup routine runs once when you press reset:
void setup() {                

	Redditor *r;
	Fourchanner *f;
	
	Serial.begin(9600); // open serial terminal
	Serial.flush();

	Process::Init();  // start the threading library

	daclub = Club();
	daclub.reinit();

	r = new Redditor(1); //start first thread
	r = new Redditor(2); //start second thread
	r = new Redditor(3); //start second thread
	f = new Fourchanner(4); //start third thread
	f = new Fourchanner(5); //start fourth thread

}

// the loop routine runs over and over again forever:
void loop() {
	Process::Start();
}

