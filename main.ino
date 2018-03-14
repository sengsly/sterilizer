#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>       //include timer interrupt
#define TEMP_SENSOR_PIN 2
#define HEATER_PIN 3
#define START_PIN 4
#define STEAM_DURATION 300         // 5*60 = 5 minutes
#define COOLING_DURATION 300      //
#define WARM_TEMP   40              //warm water temperatures
#define MAX_RUN_TIME 900          //15*60=15 minutes


enum WORKING_MODE {
  Boil,
  Warm,
  None
};
enum WORKING_STAGE{
  Heating,
  Steaming,
  Cooling,
  Stopping
};

//Variables
//==========================================================================================
WORKING_MODE workingMode=None;
WORKING_STAGE workingStage=Stopping;
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
unsigned countWorkTimer=0;
unsigned countStageTimer=0;
float currentTemperature=-300;
//==========================================================================================

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
  countWorkTimer=0;
  countStageTimer=0;
  workingStage=WORKING_STAGE::Heating;
  workingMode=WORKING_MODE::Boil;
  Serial.print("Start heating");
}

void loop(void)
{ 
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
void timerOneSecond(){
    // Timer counting
    switch(workingMode){
      case WORKING_MODE::Boil:
        countWorkTimer++;
        countStageTimer++;
        if(workingStage==WORKING_STAGE::Cooling){
          turnheaterOff();
          if(countStageTimer>=COOLING_DURATION){
            workingStage=WORKING_STAGE::Stopping;
            workingMode=WORKING_MODE::None;
          }
        }else if(workingStage==WORKING_STAGE::Heating){
          if(currentTemperature<100) {
            if (countWorkTimer>=MAX_RUN_TIME ){
              workingStage=WORKING_STAGE::Cooling;
              countStageTimer=0;
              turnheaterOff();
            }else{
              turnHeaterOn();
            }
          }else{
            workingStage=WORKING_STAGE::Steaming;
            countStageTimer=0;
          }
        } else if(workingStage==WORKING_STAGE::Steaming){
          if(countStageTimer>=STEAM_DURATION){
            workingStage==WORKING_STAGE::Cooling;
          }else{
            turnHeaterOn();
          }
        }
        break;
      case WORKING_MODE::Warm:
        if(currentTemperature<WARM_TEMP){
          countStageTimer++;
          turnHeaterOn();
        }else{
          turnheaterOff();
        }
        //workingMode=WORKING_MODE::None;
        countWorkTimer++;
        break;
      case WORKING_MODE::None:
        countWorkTimer=0;
        break;
    }
}