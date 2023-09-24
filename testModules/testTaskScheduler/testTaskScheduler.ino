#include <TaskScheduler.h>
Scheduler runner;


// #define EXAMPLE1
#define EXAMPLE2



/* simple delay */
#ifdef EXAMPLE1
void callback();
void callback_step2();
void callback_step3();

Task t1(TASK_IMMEDIATE, 3, &callback);

void callback() {
  Serial.println("1 : " + String(millis()));
  t1.setCallback(&callback_step2);
  t1.delay(1000);
}
void callback_step2() {
  Serial.println("2 : " + String(millis()) );
  t1.setCallback(&callback_step3);
  t1.delay(1000);
}
void callback_step3() {
  Serial.println("3 : " + String(millis()));
  t1.setCallback(&callback);
  t1.delay(1000);
}


void setup() {
  Serial.begin(115200);
  // Serial.println("Scheduler TEST");
  runner.init();
  // Serial.println("Initialized scheduler");
  runner.addTask(t1);
  // Serial.println("added t_run01");
  t1.enable();
  // Serial.println("Enabled t_run01");
}

void loop() {
  runner.execute();
}
#endif



#ifdef EXAMPLE2

// #define _TASK_SELF_DESTRUCT 
int outsideVal = 0;


bool delay1000_step1();
bool delay1000_step2();
void run01();
Task t_delay1000(0, TASK_FOREVER, &delay1000_step1);
Task t_run01(3000, 3, &run01);


bool delay1000_step1(){
  Serial.println("----before delay : " + String(millis()));
  t_delay1000.setCallback(&delay1000_step2);
  t_delay1000.delay(1000);
}
bool delay1000_step2(){
  Serial.println("----after delay : " + String(millis()));
  t_delay1000.setCallback(&delay1000_step1);
  runner.deleteTask(t_delay1000);
}


void run01(){
  Serial.println("=== here is run01, time : " + String(millis()) + " ===");
  Serial.println("outsideVal : " + String(outsideVal) );
  if(1){
    Serial.println("--in run01 if statement");
    // if (t_delay1000.isFirstIteration()) {    }
    runner.addTask(t_delay1000);
    Serial.println("--before enable : " + String(millis()));
    t_delay1000.enable();
    Serial.println("--after enable : " + String(millis()));
  }
  outsideVal += 1;
  Serial.println("outsideVal : " + String(outsideVal) );
}


void setup() {
  Serial.begin(115200);
  // Serial.println("Scheduler TEST");
  runner.init();
  // Serial.println("Initialized scheduler");
  runner.addTask(t_run01);
  // Serial.println("added t_run01");
  t_run01.enable();
  // Serial.println("Enabled t_run01");
}

void loop() {
  runner.execute();
}
#endif
