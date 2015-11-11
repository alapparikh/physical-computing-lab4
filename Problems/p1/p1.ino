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

int STROBE_PIN = 11;
int DATA_PIN = 12;
int CLOCK_PIN = 13;

int COLUMN1_PIN = 4;
int COLUMN2_PIN = 5;
int COLUMN3_PIN = 6;
int COLUMN4_PIN = 7;
int COLUMN5_PIN = 8;
int COLUMN_PINS[5] = {
  COLUMN1_PIN, COLUMN2_PIN, COLUMN3_PIN, COLUMN4_PIN, COLUMN5_PIN
};
int COLUMNS[5] = {0, 0, 0, 0, 0};

int ROWS_ALL_OFF = 0x00;
int ROWS_ALL_ON = 0xFF;

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
      /* Make it so that this function only returns when it is safe for
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
      /* Exit the club. */
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
      /* Make it so that this function only returns when it is safe for
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
      /* Exit the club. */
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

void light_LED() {
  for(int i = 0; i < 5; i++){
    Serial.print(COLUMNS[i]);
    if (COLUMNS[i]) {
      digitalWrite(COLUMN_PINS[i], LOW);
    }
    else {
      digitalWrite(COLUMN_PINS[i], HIGH);
    }
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
    COLUMNS[_id - 1] = 1;
    Serial.print("1a");
    /* Light up column #_id on the LED matrix */
    light_LED();
    hang_out();
    Serial.print("1b");
    COLUMNS[_id - 1] = 0;
    light_LED();
    daclub.redditor_exit();
    hang_out();
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
    /* light up column #_id on the LED matrix */
    COLUMNS[_id - 1] = 1;
    Serial.print("2a");
    light_LED();
    hang_out();
    COLUMNS[_id - 1] = 0;
    Serial.print("2b");
    light_LED();
    daclub.fourchanner_exit();
    hang_out();
  }
};

// the setup routine runs once when you press reset:
void setup() {

  Redditor *r;
  Fourchanner *f;

  pinMode(STROBE_PIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(COLUMN1_PIN, OUTPUT);
  pinMode(COLUMN2_PIN, OUTPUT);
  pinMode(COLUMN3_PIN, OUTPUT);
  pinMode(COLUMN4_PIN, OUTPUT);
  pinMode(COLUMN5_PIN, OUTPUT);

  digitalWrite(COLUMN1_PIN, HIGH);
  digitalWrite(COLUMN2_PIN, HIGH);
  digitalWrite(COLUMN3_PIN, HIGH);
  digitalWrite(COLUMN4_PIN, HIGH);
  digitalWrite(COLUMN5_PIN, HIGH);

  digitalWrite(STROBE_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ROWS_ALL_ON);
  digitalWrite(STROBE_PIN, HIGH);

  Serial.begin(9600); // open serial terminal
  Serial.flush();

  Process::Init();  // start the threading library

  daclub = Club();
  daclub.reinit();

  r = new Redditor(1); //start first thread
  r = new Redditor(2); //start second thread
  r = new Redditor(3); //start third thread
  f = new Fourchanner(4); //start fourth thread
  f = new Fourchanner(5); //start fifth thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}

