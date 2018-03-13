#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>       //include timer interrupt
#define TEMP_SENSOR_PIN 2
#define HEATER_PIN 3
#define START_PIN 4
#define STEAM_DURATION 300         // 5*60 = 5 minutes
#define WARM_TEMP   40              //warm water temperatures


enum WORKING_MODE {
  Steam,
  Warm,
  None
};

WORKING_MODE workingMode=None;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
unsigned countingTimer=0;
void setup(void)
{

  // start serial port
  Serial.begin(9600);
  Serial.println("Kimseng bottle sterilizer V1.1");
  sensors.begin();
  pinMode(START_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(START_PIN), startInterrupt, FALLING);
  Timer1.initialize(1000000); // set a timer of length 1000000 microseconds 
  Timer1.attachInterrupt( timerOneSecond ); // attach the service routine here
}
void startInterrupt(){
  workingMode=Steam;
  Serial.print("Start heating");
}

void loop(void)
{ 
  float currentTemperature;
  sensors.requestTemperatures(); // Söend the command to get temperatures
  Serial.print("Temperature for Index 0 is: ");
  currentTemperature=sensors.getTempCByIndex(0);
  Serial.println(currentTemperature);  
  delay(500);
}

void refreshDisplay(){
  //refresh display each calls

}

void turnheaterOff(){

}

void turnHeaterOn(){

}
void timerOneSecond()
{
    // Timer counting
    switch(workingMode){
      case Warm:
        if(countingTimer>=WARM_TEMP){
          turnheaterOff();
        }else{
          turnHeaterOn();
        }
        countingTimer++;
        break;
      case Steam:
        countingTimer++;
        break;
      case None:
        countingTimer=0;
        break;
    }
}