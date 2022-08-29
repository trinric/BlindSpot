#include <DS3231.h>
#include <math.h>
#include <Wire.h>
#include <A4988.h>
#include <BasicStepperDriver.h>
#include <DRV8825.h>
#include <DRV8834.h>
#include <DRV8880.h>
#include <MultiDriver.h>
#include <SyncDriver.h>

#define MOTOR_STEPS 200
#define RPM 30
#define DIR 2
#define STEP 3
#define SLEEP 13
#define MS1 10
#define MS2 11
#define MS3 12
A4988 stepper(MOTOR_STEPS, DIR, STEP, SLEEP, MS1, MS2, MS3);

const int mode = 8;

const double lat = 42.901501;
const double lng = -71.554367;

const double wh = 39; //height from eye to top of window in inches
const double d = 38; // distance from eye to wall in inches
const double updateTime = 10000;

double ph = wh; //previous height starting at the max height. This will change every loop
double nh;
double sr = 0.75; //radius of spool in inches.
double sc = sr * 2 * PI; // spool circumference

double prevAngle;
double startingAngle;

double current_time = 0;
int doy;

DS3231 clock;
RTCDateTime dt;

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
const int rolePerMinute = 15;         // Adjustable range of 28BYJ-48 stepper is 0~17 rpm


void setup() {
  stepper.begin(RPM, 8);
  Serial.begin(9600);
  //starting max angle of window
  startingAngle = atan(wh/d);
  prevAngle = startingAngle;
  clock.begin();
  //clock.setDateTime(__DATE__, __TIME__); 
 
}

void loop() {
  dt = clock.getDateTime();
  doy = day_of_year(dt.month, dt.day, dt.year);
  current_time = time_to_min(dt.hour, dt.minute)-60;
  double newAngle = solar_angle();
  Serial.println(current_time);
  Serial.print("New: ");
  Serial.println(newAngle);
  Serial.print("Old: ");
  Serial.println(prevAngle);
  if(newAngle < prevAngle && newAngle > 0){
    double drop = d*(tan(prevAngle) - tan(newAngle));
    double spoolRev = drop / sc; 
    //spoolRev is what fraction of a full revolution you want. So if the circumference was 2 inches, 
    //and you needed to lower the blinds by 1 inch, you would do half a rotation.
    int numSteps = round((200 * mode)*spoolRev); //How many 'steps' of the motor to go. 
    stepper.move(numSteps);
    prevAngle = newAngle; 
    
  }
  Serial.println("~~~~~~~~~~~~");
  delay(updateTime);
}

/* 
All of the solar calculations below
*/
int time_to_min(int hour, int minutes) {
  return hour*60 + minutes;
}

int day_of_year(int month, int d, int year) { 
  int months[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  int day = d;
  for(int i=1; i<month; i++) {
    day = day + months[i];
    if(i == 2 && year%4 == 0) {
      day = day + 1;
    }
  }
  return day;
}
double solar_angle(){
  //This is the magic angle we want.
  double ast = calculate_ast();
  double h = degree_to_rad(hour_angle(ast));
  double l = degree_to_rad(lat);
  double decl = degree_to_rad(declination(doy)); //confirmed okay
  double sin_val = cos(l)*cos(decl)*cos(h) + sin(l)*sin(decl);
  double angle = asin(sin_val);
  return angle;
}
double calculate_ast(){
  // IN MINUTES
  double lst = current_time;
  double et = equationOfTime(doy); //confirmed.
  double lstm = find_lstm(); //confirmed.
  double ast = lst + (4.0)*(lng-lstm) + et;
  return ast;
  
}
double equationOfTime(int day_of_year){
  //Confirmed Accurate
  double d = 360.0*((day_of_year-81.0)/365.0);
  d = degree_to_rad(d);
  double et = 9.87*sin(2*d) - 7.53*cos(d) - 1.5*sin(d);
  return et;
}

double find_lstm() {
  //Confirmed
  return 15.0*round(lng/15.0);
}
double hour_angle(double minutes){
  //minutes is number past midnight in AST.
  return (minutes - 720.0)/4.0;
}
double declination(int day_of_year) {
   return 23.45*sin(degree_to_rad(((day_of_year + 284.0)/365.0)*360.0));
}

/*
 * Convenience conversions
 */

double degree_to_rad(double degree){
  // Accurate to like 5 decimal places.
  return (degree*71) / 4068;
}

double rad_to_degree(double radian){
  return (radian * 4068) / 71;
}