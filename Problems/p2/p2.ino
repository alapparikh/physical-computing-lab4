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

class MatingArea {

    /* We have declared one lock and one condition
     * variable here.  You may (or may not) find it
     * useful to declare additional condition variables
     * to solve the problem.  We just left these here as
     * a reminder on the syntax. */
    Lock* _l;
    Cond* _c;

  public:
    MatingArea () {  //Constructor for club
      _l = new Lock();
      _c = new Cond(_l);
    }

    void he_ready() {
      /*TODO: Only return when there is a she and an it also ready*/
    }

    void she_ready() {
      /*TODO: Only return when there is a he and an it also ready*/
    }

    void it_ready() {
      /*TODO: Only return when there is a he and a she also ready*/
    }
};

MatingArea ma;

class He : Process {

  public:
    He () {}

    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.println("He: I'm born!");
      Serial.println("He: Adult now, time to form a triad!");
      ma.he_ready(); //do not pass until there is a she and an it
      Serial.println("He: Yay, I'm part of a triad!");
    }
};

class She : Process {

  public:
    She () {}

    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.println("She: I'm born!");
      Serial.println("She: Adult now, time to form a triad!");
      ma.she_ready(); //do not pass until there is a he and an it
      Serial.println("She: Yay, I'm part of a triad!");
    }
};

class It : Process {

  public:
    It () {}

    void loop () {
      delay(random(300, 1500)); //waste time
      Serial.println("It: I'm born!");
      Serial.println("It: Adult now, time to form a triad!");
      ma.it_ready(); //do not pass until there is an it and a she
      Serial.println("It: Yay, I'm part of a triad!");
    }
};


// the setup routine runs once when you press reset:
void setup() {

  He *h;
  She *s;
  It *i;

  Serial.begin(9600); // open serial terminal
  Process::Init();  // start the threading library

  ma = MatingArea();
  h = new He(); //start first thread
  h = new He(); //start second thread
  s = new She(); //start third thread
  i = new It(); //start fourth thread
}

// the loop routine runs over and over again forever:
void loop() {
  Process::Start();
}

