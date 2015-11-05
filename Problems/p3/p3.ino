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



class Barrier {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */
    Lock* _l;
    Cond* _c;
    int _n;

  public:
    Barrier (int n) {  //Constructor for club
      _l = new Lock();
      _c = new Cond(_l);
      _n = n;
    }

    void wait() {
      /*TODO: Only return when _n other threads have also called wait*/
    }
};

Barrier B(3);

class CalcThread : Process {
  public:
  virtual int get_num();
};

CalcThread* threads [3];

class Sum : CalcThread {

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
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1,100);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      B.wait();

      int output = 0;
      for (int i = 0; i < 3; ++i)
        output += threads[i]->get_num();

      Serial.print("Sum = ");
      Serial.println(output);

      B.wait();
    }
};

class Mean : CalcThread {

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
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(1, 100);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      B.wait();

      float output = 0.0;
      for (int i = 0; i < 3; ++i)
        output += threads[i]->get_num()/3.0;
      
      Serial.print("Mean = ");
      Serial.println(output);

      B.wait();
    }
};

class Printer : CalcThread {

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
      delay(random(300, 1500)); //do some computation
      _iteration++;
      _num = random(300, 1500);
      /* TODO: Make the pixel at (_id, _iteration) of the LED matrix light up (and stay lit)*/
      B.wait();

      Serial.print("Numbers: [");
      for (int i = 0; i < 3; ++i) {
        Serial.print(threads[i]->get_num());
      }
      Serial.println("]");

      B.wait();
    }
};

// the setup routine runs once when you press reset:
void setup() {

  Sum *s;
  Mean *m;
  Printer *p;
  
  Serial.begin(9600); // open serial terminal
  Process::Init();  // start the threading library

  s = new Sum(1); //start first thread
  m = new Mean(2); //start second thread
  p = new Printer(3); //start third thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}

