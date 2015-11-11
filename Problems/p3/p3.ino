 /* Barrier synchronization
 *
 * A common pattern used in parallel programs is to have processing in two stages: computation and communication.
 * First, all threads do computation or IO in parallel.  When every thread has finished,
 * the threads communicate their computed values with one another, and based on those
 * values, another round of computation takes place, followed by another communication
 * step.
 *
 * In order to facilitate this style of computation, it's useful to use a synchronization
 * construct known as a barrier.  Barriers cause all threads to wait until all other
 * threads have also called wait.  Then all threads are allowed to continue processing.
 * The barrier prevents threads from communicating before other threads are finished,
 * or from starting the next round of computation before other threads have communicated.
 *
 * In this example, we have three threads
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


int ROW_VALUES[7] = {
  0x01, // row 1 - 0000 0001
  0x02, // row 2 - 0000 0010
  0x04, // row 3 - 0000 0100
  0x08, // row 4 - 0000 1000
  0x10, // row 5 - 0001 0000
  0x20, // row 6 - 0010 0000
  0x40  // row 7 - 0100 0000
};

void light_LED(int col, int iter) {
  int row_num = iter % 7;

  // Report LED status
  // Serial.print("Displaying pixel at (");
  // Serial.print(col);
  // Serial.print(",");
  // Serial.print(row_num);
  // Serial.print(") for iteration ");
  // Serial.println(iter);

  // Light the LED
  digitalWrite(COLUMN_PINS[col], LOW);
  digitalWrite(STROBE_PIN, LOW);
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ROW_VALUES[row_num]);
  digitalWrite(STROBE_PIN, HIGH);
  delay(1000);
}

void reset_columns () {
	for (int i=0; i<5; i++) {
        digitalWrite(COLUMN_PINS[i], HIGH);
    }
}

class Loudspeaker {
  Lock* _l;
  Cond* _c;
  bool avail;

public:
  Loudspeaker() {
    _l = new Lock();
    _c = new Cond(_l);
    avail = true;
  };

  void announce(char const *message) {
    acquire();
    Serial.print(message);
    Serial.println();
    Serial.flush();
    release();
  }

  void acquire (){
    _l->lock();
    if (!avail) {
      _l->unlock();
      _c->wait();
    }
    avail = false;
    _l->unlock();
  }

  void release() {
    _l->lock();
    avail = true;
    if (_c->waiting()) {
      _c->signal();
    } else {
      _l->unlock();
    }
  }

};

class Barrier {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */
    Lock* _l;
    Cond* _c;
    int _num;
    int waiting;

  public:
    Barrier (int num) {  //Constructor for barrier
      _l = new Lock();
      _c = new Cond(_l);
      _num = num;
      waiting = 0;
    }

    void reinit() {
      _l = new Lock();
      _c = new Cond(_l);
    }

    void wait(int id, int barrier_num) {
      /* Only return when _num other threads have also called wait*/
      _l->lock();
      // Serial.print(id);
      // Serial.print(" - HIT BARRIER (waiting=");
      // Serial.print(waiting);
      // Serial.print(") barrier_num=");
      // Serial.println(barrier_num);
      // Serial.flush();

      if (waiting < (_num - 1)) {
        waiting++;
        // Serial.print(id);
        // Serial.print(" - WAIT (waiting=");
        // Serial.print(waiting);
        // Serial.println(")");
        // Serial.flush();
        _l->unlock();
        _c->wait();
        waiting--;
      }
      if (waiting > 0 && _c->waiting()) {
        // Serial.print(id);
        // Serial.print(" - SIGNAL (waiting=");
        // Serial.print(waiting);
        // Serial.println(")");
        // Serial.flush();
        _c->signal();
      } else {
        // Serial.print(id);
        // Serial.print(" - CONTINUE (waiting=");
        // Serial.print(waiting);
        // Serial.println(")");
        // Serial.flush();
        _l->unlock();
        if (barrier_num == 2) {
        	reset_columns();
        }
      }
    }
};

Barrier B(3);
Loudspeaker SPEAKER;

class CalcThread : Process {
  public:
  virtual int get_num();
};

CalcThread* THREADS [3];

class Sum : public CalcThread {

  int _id;
  int _iteration;
  int _num;

  public:
    Sum (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }

    void loop () {
      SPEAKER.announce("Sum starting computation!");

      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1,100);
      // Wait for everyone to finish computation
      B.wait(_id, 1);

      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      int output = 0;
      for (int i = 0; i < 3; ++i) {
        output += THREADS[i]->get_num();
      }

      //SPEAKER.announce("Sum DONE computation!");
      SPEAKER.acquire();
      light_LED(_id, _iteration);
      Serial.print("Sum: ");
      Serial.println(output);
      SPEAKER.release();

      // Wait for everyone to finish communication
      B.wait(_id, 2);
    }
};

class Mean : public CalcThread {

  int _id;
  int _iteration;
  int _num;

  public:
    Mean (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }
    void loop () {
      SPEAKER.announce("Mean starting computation!");

      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1, 100);
      // Wait for everyone to finish computation
      B.wait(_id, 1);

      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      float output = 0.0;
      for (int i = 0; i < 3; ++i) {
        output += THREADS[i]->get_num()/3.0;
      }

      //SPEAKER.announce("Mean DONE computation!");
      SPEAKER.acquire();
      Serial.print("Mean: ");
      Serial.println(output);
      light_LED(_id, _iteration);
      SPEAKER.release();

      // Wait for everyone to finish communication
      B.wait(_id, 2);
    }
};

class Printer : public CalcThread {

  int _id;
  int _iteration;
  int _num;

  public:
    Printer (int id) {
      _id = id;
      _iteration = 0;
      _num = 0;
    }

    int get_num() {
      return _num;
    }
    void loop () {
      SPEAKER.announce("Printer starting computation!");

      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1, 100);
      // Wait for everyone to finish computation
      B.wait(_id, 1);

      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/

      //SPEAKER.announce("PRINTER result");
      SPEAKER.acquire();
      light_LED(_id, _iteration);
      Serial.print("Numbers: [");
      for (int i = 0; i < 3; ++i) {
        Serial.print(THREADS[i]->get_num());
      }
      Serial.println("]");
      SPEAKER.release();

      // Wait for everyone to finish communication
      B.wait(_id, 2);
    }
};

// the setup routine runs once when you press reset:
void setup() {
  Sum *s;
  Mean *m;
  Printer *p;

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
  shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, ROWS_ALL_OFF);
  digitalWrite(STROBE_PIN, HIGH);

  Serial.begin(9600); // open serial terminal
  Serial.flush();

  Process::Init();  // start the threading library

  B.reinit();

  s = new Sum(1); //start first thread
  m = new Mean(2); //start second thread
  p = new Printer(3); //start third thread

  THREADS[0] = s;
  THREADS[1] = m;
  THREADS[2] = p;
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}

