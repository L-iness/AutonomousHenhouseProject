#include <LowPower.h>

// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <Wire.h>
#include <RTClib.h>

RTC_DS1307 rtc;

//For the following values, see :
//https://arduinodiy.wordpress.com/2017/03/07/calculating-sunrise-and-sunset-on-arduino-or-other-microcontroller/

DateTime right_now;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Presence detectors used to detect when the door is resp. open or closed.
int PIN_highSensor  =  7;
int PIN_lowSensor   =  6;


// DC H bridge Motor
int PIN_AIN1  = 12; // Direction motor 1
int PIN_PWMA  =  3; // Speed moteur 1

// The mandatory all purpose "i" variable
int i;

// Use serial print for debug purpose in functions
unsigned VERBOSE              =  1;



/*
 * 6 different Sunrise and Sunset hours in function of the month in the year.
 */


//Sunrise time between january and february is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_1         = 10;
unsigned SUNRISE_MINUTE_1       = 00;

//Sunset time between january and february is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_1          = 19;
unsigned SUNSET_MINUTE_1        = 20;

//Sunrise time between march and april is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_2         = 8;
unsigned SUNRISE_MINUTE_2       = 30;

//Sunset time between march and april is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_2           = 21;
unsigned SUNSET_MINUTE_2         = 00;


//Sunrise time between may and june is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_3         = 07;
unsigned SUNRISE_MINUTE_3       = 37;

//Sunset time between may and june is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_3          = 22;
unsigned SUNSET_MINUTE_3        = 06;

//Sunrise time between july and august is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_4         = 07;
unsigned SUNRISE_MINUTE_4       = 37;

//Sunset time between july and august is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_4          = 22;
unsigned SUNSET_MINUTE_4        = 00;

//Sunrise time between september and october is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_5         = 8;
unsigned SUNRISE_MINUTE_5       = 30;

//Sunset time between september and october is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_5          = 20;
unsigned SUNSET_MINUTE_5        = 30;

//Sunrise time between november and december is hh:mm = SUNRISE_HOUR:SUNRISE_MINUTE
unsigned SUNRISE_HOUR_6         = 10;
unsigned SUNRISE_MINUTE_6       = 00;

//Sunset time between november and december is hh:mm = SUNSET_HOUR:SUNSET_MINUTE
unsigned SUNSET_HOUR_6          = 18;
unsigned SUNSET_MINUTE_6        = 30;




enum State_enum {INIT, WAIT_NIGHT, CLOSE_DOOR, WAIT_DAY, OPEN_DOOR};
uint8_t state = INIT;

unsigned NIGHT_DURATION = 9;
unsigned DAY_DURATION = 8;


/****************************************************************************
 *   ____  _____ _____ _   _ ____  
 *  / ___|| ____|_   _| | | |  _ \ 
 *  \___ \|  _|   | | | | | | |_) |
 *   ___) | |___  | | | |_| |  __/ 
 *  |____/|_____| |_|  \___/|_|    
 * 
 ***************************************************************************/
void setup() {

  Serial.begin(57600); // Don't forget to change into 57600 baud in the serial monitor to see what's printed

#ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  Serial.print("Start SETUP");
  
  pinMode(PIN_highSensor , INPUT);
  pinMode(PIN_lowSensor  , INPUT);
  pinMode(PIN_AIN1       , OUTPUT);
  pinMode(PIN_PWMA       , OUTPUT);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");

  }
  
/*
 * following line sets the RTC to the date & time this sketch was compiled
 * rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
 * This line sets the RTC with an explicit date & time, for example to set
 * January 21, 2014 at 3am you would call:
 * rtc.adjust(DateTime(2018, 5, 28, 13, 36, 0));
 */
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // DON'T FORGET TO PUT IT IN COMMENT IN ORDER TO NOT RESET IT


  Serial.print('/');
  Serial.println("End SETUP");
}

/****************************************************************************
 *   _     ___   ___  ____  
 *  | |   / _ \ / _ \|  _ \ 
 *  | |  | | | | | | | |_) |
 *  | |__| |_| | |_| |  __/ 
 *  |_____\___/ \___/|_|    
 * 
 ***************************************************************************/
void loop() {
  
  Serial.println("Start LOOP");

  DateTime now = rtc.now();
  
  // Print the current date and time
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();


   /*************
   * CLOSING
   *************/
  // Default : Closing the hatch for the hens' safety
  Serial.println("Close the hatch");
  closeTheHatch();


  // Puts Arduino into sleep mode to save the battery
  
  Serial.println("Wait until sunrise");
  // wait until sunrise, verbose mode with "1" as argument, depends on the month of the year

  
  Serial.println("Entering Low power");

  i = 0;

  // Sleeping for 9 hours until sunrise
  while (i<4050) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    i++;  
  }
 
   
  waitSunrise();// Different time of Sunrise in function of the month of the year


  /*************
   * OPENING
   ************/
  Serial.println("Open the hatch");
  openTheHatch();

  Serial.println("Wait until sunset");
  // wait until sunset, verbose mode with "1" as argument, depends on the month of the year
  
  Serial.println("Entering Low power");

  i = 0;
  
  // Sleeping for 8 hours until sunset
  while (i<3600) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
    i++;
  }


  waitSunset(); // Different time of Sunset in function of the month of the year
  

}


/****************************************************************************
 * 
 * FSM implementation, including state transition all functions calls
 * depending on which state the system is.
 * List of states : INIT, WAIT_NIGHT, CLOSE_DOOR, WAIT_DAY, OPEN_DOOR
 * 
 ***************************************************************************/
void state_machine_run() 
{
  switch(state)
  {
    case INIT:
    
      break;

       
    case WAIT_NIGHT:
     
      break;
 
    case CLOSE_DOOR:
      
      break;
 
    case WAIT_DAY:
      
      break;
      
    case OPEN_DOOR:
      
      break;
  }
}






/****************************************************************************
 * 
 * Waiting until a limit hour:minute format time today, get the current time 
 * from RTC every 15 minutes and compare it the limit time. When the current 
 * time is greater than the limit it breaks out
 * 
 ***************************************************************************/



void waitUntilLimit(int limitHour, int limitMinute, int v, String Message)
{
  DateTime      limit;
  DateTime      currentTime;
  unsigned long currentTime_unix;
  unsigned long limit_unix;
  long          diff = 1;
  unsigned long waitDelay;
  
  currentTime      = rtc.now();
  currentTime_unix = currentTime.unixtime();
  
  limit      = DateTime(currentTime.year(), currentTime.month(), currentTime.day(), limitHour, limitMinute, 0);
  limit_unix = limit.unixtime();

  diff = limit_unix - currentTime_unix;

  if(diff > 0)
    waitDelay = diff*1000;
  else
    waitDelay = 1;
 
  Serial.println((String)"Wait delay   : " + waitDelay);

  
  if(v==1)
  {
    Serial.println("=============================================================");
    Serial.println((String)"Wait until   : " + Message);
    Serial.print("limit        : ");
    SerialPrintDate(limit);
    Serial.print("currentTime  : ");
    SerialPrintDate(currentTime);
    Serial.println((String)"Limit_unix   : " + limit_unix);
    Serial.println((String)"Current unix : " + currentTime_unix);
    Serial.println((String)"Diff unix    : " + diff);
  }


  while (diff > 0) // there is a break inside
  {
    //delay(SLEEP_LOOP);
    
    currentTime      = rtc.now();
    currentTime_unix = currentTime.unixtime();
    diff = limit_unix - currentTime_unix;

    if(v==1)
    {
      Serial.println("=============================================================");
      Serial.println((String)"Wait until   : " + Message);
      Serial.print("limit        : ");
      SerialPrintDate(limit);
      Serial.print("currentTime  : ");
      SerialPrintDate(currentTime);
      Serial.println((String)"Limit_unix   : " + limit_unix);
      Serial.println((String)"Current unix : " + currentTime_unix);
      Serial.println((String)"Diff unix    : " + diff);
    }
  }
  if(v==1)
  {
    Serial.println("=============================================================");
  }

}




  /*  
   *   Different time of Sunrise in function of the month of the year
   */

void waitSunrise(){

  DateTime now = rtc.now();
   
  if (now.month() == 01 or now.month() == 02) {
    waitUntilLimit(SUNRISE_HOUR_1 , SUNRISE_MINUTE_1, VERBOSE, "Sunrise");
  }
  else if (now.month() == 03 or now.month() == 04) {
     waitUntilLimit(SUNRISE_HOUR_2 , SUNRISE_MINUTE_2, VERBOSE, "Sunrise");
  }
  else if (now.month() == 05 or now.month() == 06) {
     waitUntilLimit(SUNRISE_HOUR_3 , SUNRISE_MINUTE_3, VERBOSE, "Sunrise");
  }
  else if (now.month() == 07 or now.month() == 0x8) {
     waitUntilLimit(SUNRISE_HOUR_4 , SUNRISE_MINUTE_4, VERBOSE, "Sunrise");
  }
  else if (now.month() == 0x9 or now.month() == 10) {
     waitUntilLimit(SUNRISE_HOUR_5 , SUNRISE_MINUTE_5, VERBOSE, "Sunrise");
  }
  else if (now.month() == 11 or now.month() == 12) {
     waitUntilLimit(SUNRISE_HOUR_6 , SUNRISE_MINUTE_6, VERBOSE, "Sunrise");
  }

}






  /*  
   *   Different time of Sunset in function of the month of the year
   */


void waitSunset(){
  
  DateTime now = rtc.now();

  if (now.month() == 01 or now.month() == 02) {
    waitUntilLimit(SUNSET_HOUR_1 , SUNSET_MINUTE_1, VERBOSE, "Sunset");
  }
  else if (now.month() == 03 or now.month() == 04) {
     waitUntilLimit(SUNSET_HOUR_2 , SUNSET_MINUTE_2, VERBOSE, "Sunset");
  }
  else if (now.month() == 05 or now.month() == 06) {
     waitUntilLimit(SUNSET_HOUR_3 , SUNSET_MINUTE_3, VERBOSE, "Sunset");
  }
  else if (now.month() == 07 or now.month() == 0x8) {
     waitUntilLimit(SUNSET_HOUR_4 , SUNSET_MINUTE_4, VERBOSE, "Sunset");
  }
  else if (now.month() == 0x9 or now.month() == 10) {
     waitUntilLimit(SUNSET_HOUR_5 , SUNSET_MINUTE_5, VERBOSE, "Sunset");
  }
  else if (now.month() == 11 or now.month() == 12) {
     waitUntilLimit(SUNSET_HOUR_6 , SUNSET_MINUTE_6, VERBOSE, "Sunset");
  }


}
  

/****************************************************************************
 * 
 * Opening the hatch by driving a DC motor through a H bridge
 * waiting until the sensor is reached before stoping the motor
 * Used at morning to unleash the chicks
 * 
 ***************************************************************************/
void openTheHatch()
{
  int highSensor = 0;
  
  Serial.println("It is morning, unleash the beasts");
  highSensor = digitalRead(PIN_highSensor);
  Serial.println("Raising the hatch");
  int timer = 0; // Adding a timer in case the door's string is broken to save the battery

  while (highSensor == 1 && timer <= 8000)
  {
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_PWMA, HIGH);
    highSensor = digitalRead(PIN_highSensor);
    Serial.println((String)"highSensor : " + highSensor);
    timer = timer + 10;
    delay(10);
  }
  
  highSensor = 0;
  Serial.println("Stop raising the hatch");
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_PWMA, LOW);
  
  Serial.println("Open, have a nice day");
}


/****************************************************************************
 * 
 * Closing the hatch by driving a DC motor through a H bridge
 * waiting until the sensor is reached before stoping the motor
 * Used at morning to unleash the chicks
 * 
 ***************************************************************************/
void closeTheHatch()
{
  int lowSensor = 0;
  Serial.println("It is evening, seize the beasts");
  lowSensor = digitalRead(PIN_lowSensor);
  Serial.println("Descending the hatch");
  int timer = 0;// Adding a timer in case the door's string is broken to save the battery
  
  while (lowSensor == 0 && timer <= 8000)
  {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_PWMA, HIGH);
    lowSensor = digitalRead(PIN_lowSensor);
    Serial.println((String)"lowSensor : " + lowSensor);
    timer = timer +10;
    delay(10);
  }
  
  lowSensor = 0;

  Serial.println("Stop closing the hatch");
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_PWMA, LOW);
  
  Serial.println("Hatch closed, good night");
}



/****************************************************************************
 * 
 * Gets the day of the year from year, month (1-12) and day (1-31). 
 * leap year supported
 * 
 ***************************************************************************/
int calculateDayOfYear(int year, int month, int day)
{
  // from https://gist.github.com/jrleeman/3b7c10712112e49d8607
  // Given a day, month, and year (4 digit), returns 
  // the day of year. Errors return 999.
  
  int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int doy = 0;
  
  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }
  
  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year%4  == 0) {
    if (year%100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year%400 == 0) {
        daysInMonth[1] = 29;
      }
    }
  }

  // Make sure we are on a valid day of the month
  if (day < 1) 
    {
      return 999;
    } else if (day > daysInMonth[month-1]) {
    return 999;
  }
  
  
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }
  
  doy += day;
  return doy;
}


void waitXminute(int X , int v)
{
  Serial.println((String)"Enter waitXminute");
  for(i = 0 ; i < X ; i++)
  {
    delay(60000);
    if(v==1)
    {
      Serial.println((String)"wait minute : " + i);
    }
  }
}

void waitXhour(int X , int v)
{
  Serial.println((String)"Enter waitXhour");
  for(i = 0 ; i < X ; i++)
  {
    waitXminute(60 , v);
    if(v==1)
    {
      Serial.println((String)"wait hour : " + i);
    }
  }
}

/****************************************************************************
 * 
 * Print a line in the serial monitor with the date and hour with the
 * following format :
 * YYYY-MM-DD-HH-MM-SS
 * 
 ***************************************************************************/
void SerialPrintDate(DateTime thedate)
{
  Serial.println((String) thedate.year() + '-' + thedate.month() + '-' + thedate.day() + '-' + thedate.hour() +  '-' + thedate.minute() + '-' + thedate.second());
}
