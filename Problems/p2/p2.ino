/*
 Life emulation problem

 Scientists have discovered alien life on planet Leviche 538X.

 Unlike earth-based lifeforms, Levicheans have three genders, he, she and it.

 On reaching adulthood, Levichean organisms go to a mating area in search of
 other Levicheans. When a Levichean of one gender comes together with two
 other Levicheans of the other two genders (for example, a she runs into a he
 and an it) in this area, they form a lifelong physical bond and attach
 themselves into a triad. Once in a triad, Levicheans wait sometime and then
 respawn, available to reproduce yet again. As an earth
 scientist, you have been tasked with simulating the mating habits of the
 Levicheans, using threads, locks and condition variables. Each Levichean
 is modeled by a thread. Fill in the missing code segments below such that

 a) the Levichean triad formation is modeled according to the specification
 above, paying special attention to make sure that the three-way join is
 simulated correctly,

 b) the code makes forward progress whenever possible to do so (i.e. your
 simulation should accommodate every mating opportunity that is present on
 Leviche 538X).
 */

#include <proc.h>

class Loudspeaker {
  Lock* _l;
  Cond* _c;
  int num_waiting;
  bool available;

public:
  Loudspeaker() {
    _l = new Lock();
    _c = new Cond(_l);
    num_waiting = 0;
    available = true;
  };

  void acquire (){
    _l->lock();
    if (!available) {
      _l->unlock();
      _c->wait();
    }
    available = false;
    _l->unlock();
  }

  void release() {
    _l->lock();
    available = true;
    if (_c->waiting()) {
      _c->signal();
    } else {
      _l->unlock();
    }
  }

};

class MatingArea {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */
    Lock* _l;
    Cond* _c_he;
    Cond* _c_she;
    Cond* _c_it;
    int hes;
    int shes;
    int its;

  public:
    MatingArea () {  //Constructor for club
      _l = new Lock();
      _c_he = new Cond(_l);
      _c_she = new Cond(_l);
      _c_it = new Cond(_l);
      hes = 0;
      shes = 0;
      its = 0;
    }

    void he_ready() {
      /*TODO: Only return when there is a she and an it also ready*/
      _l->lock();
      if(shes > 0 && its > 0) {
        _c_she->signal();
        _c_it->signal();
        _l->unlock();
        return;
      }
      hes++;
      _l->unlock();
      _c_he->wait();
      hes--;
      return;
    }

    void she_ready() {
      /*TODO: Only return when there is a he and an it also ready*/
      _l->lock();
      if(hes > 0 && its > 0) {
        _c_he->signal();
        _c_it->signal();
        _l->unlock();
        return;
      }
      shes++;
      _l->unlock();
      _c_she->wait();
      shes--;
    }

    void it_ready() {
      /*TODO: Only return when there is a he and a she also ready*/
      _l->lock();
      if(hes > 0 && shes > 0) {
        _c_he->signal();
        _c_she->signal();
        _l->unlock();
        return;
      }
      its++;
      _l->unlock();
      _c_it->wait();
      its--;
    }
};

MatingArea ma;
Loudspeaker speaker;

class He : Process {
  int _id;

  public:
    He (int id) {
      _id = id;
    }

    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.print("He (");
      Serial.print(_id);
      Serial.println("): I'm born!");
      Serial.print("He (");
      Serial.print(_id);
      Serial.println("): Adult now, time to form a triad!");
      ma.he_ready(); //do not pass until there is a she and an it
      Serial.print("He (");
      Serial.print(_id);
      Serial.println("): Yay, I'm part of a triad!");
    }
};

class She : Process {

  public:
    She () {}

    void loop () {
      delay(random(300, 1500)); //waste time
      speaker.acquire();
      Serial.println("She: I'm born!");
      Serial.println("She: Adult now, time to form a triad!");
      speaker.release();
      ma.she_ready(); //do not pass until there is a he and an it
      speaker.acquire();
      Serial.println("She: Yay, I'm part of a triad!");
      speaker.release();
    }
};

class It : Process {

  public:
    It () {}

    void loop () {
      delay(random(300, 1500)); //waste time
      speaker.acquire();
      Serial.println("It: I'm born!");
      Serial.println("It: Adult now, time to form a triad!");
      speaker.release();
      ma.it_ready(); //do not pass until there is an it and a she
      speaker.acquire();
      Serial.println("It: Yay, I'm part of a triad!");
      speaker.release();
    }
};


// the setup routine runs once when you press reset:
void setup() {

  He *h;
  She *s;
  It *i;

  Serial.begin(9600); // open serial terminal
  Serial.flush();

  Process::Init();  // start the threading library

  ma = MatingArea();
  speaker = Loudspeaker(); // For print statements

  h = new He(1); //start first thread
  h = new He(2); //start second thread
  s = new She(); //start third thread
  i = new It(); //start fourth thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}
