#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimerOne.h>       //include timer interrupt
#include <Arduino.h>
#include <U8x8lib.h>



#define START_PIN 2
#define SHUTDOWN_PIN  3
#define TEMP_SENSOR_PIN 4
#define HEATER_PIN 5
#define BUZZER_PIN  6
#define STEAM_DURATION 300         // 5*60 = 5 minutes
#define COOLING_DURATION 300      //
#define WARMING_DURATION 300
#define WARM_TEMP   40              //warm water temperatures
#define MAX_RUN_TIME 900          //15*60=15 minutes


enum WORKING_MODE {
  Boil,
  Warm,
  None,
  Shutdown
};
enum WORKING_STAGE{
  Heating,
  Waming,
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
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);	// I2C / TWI 
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/ U8X8_PIN_NONE); 	      

unsigned countWorkTimer=0;
unsigned countStageTimer=0;
float currentTempF=-300;
//char  currentTempC[5];
//char bufRunningTime[14];
char charBuffer[14];

//==========================================================================================

void setup(void)
{

  // start serial port
  Serial.begin(9600);
  Serial.println("Kimseng bottle sterilizer V1.1");
  sensors.begin();

  //====================Setup OLED=====================
  u8x8.begin();
  u8x8.setPowerSave(0);

  //====================Setup OLED=====================

  workingMode=WORKING_MODE::None;
  workingStage=WORKING_STAGE::Stopping;

  pinMode(HEATER_PIN,OUTPUT);
  pinMode(START_PIN, INPUT_PULLUP);
  pinMode(SHUTDOWN_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(START_PIN), startButton, CHANGE);
  attachInterrupt(digitalPinToInterrupt(SHUTDOWN_PIN), shutdownButtont, FALLING);
  digitalWrite(HEATER_PIN,LOW);
  Timer1.initialize(1000000); // set a timer of length 1000000 microseconds 
  Timer1.attachInterrupt( timerOneSecond ); // attach the service routine here
  Timer1.stop();
}

void shutdownButtont(){
  countWorkTimer=0;
  countStageTimer=0;
  workingMode=WORKING_MODE::Shutdown;
  Timer1.stop();
  Serial.print("Shutdown the machine");
}


void startButton(){
  countWorkTimer=0;
  countStageTimer=0;
  workingStage=WORKING_STAGE::Heating;
  workingMode=WORKING_MODE::Boil;
  Timer1.start();
  Serial.print("Start heating");
}

void loop(void)
{ 
  //Serial.println(currentTempF);  
  timerOneSecond();
  delay(500);
}

void drawDisplay(){
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.drawString(3,0,"Sterilizer");
  u8x8.drawString(0,3,"Mode : ");
  u8x8.drawString(0,5,"Temp : ");
  u8x8.drawString(0,7,"Time : ");
  
  switch (workingMode){
    case WORKING_MODE::Boil:
      strcpy(charBuffer, "Boil");
      break;
    case WORKING_MODE::Shutdown:
      strcpy(charBuffer, "Off");
      break;
    case WORKING_MODE::Warm:
      strcpy(charBuffer, "Warm");
      break;
    case WORKING_MODE::None:
    default:
      strcpy(charBuffer, "");
      break;
  }

  u8x8.drawString(7,3,charBuffer);        //Temperature
  dtostrf(currentTempF, 3, 2, charBuffer);
  u8x8.drawString(7,5,charBuffer);        //Temperature
  convertSecond2Time(countWorkTimer);
  u8x8.drawString(7,7,charBuffer);        //Temperature


  //u8x8.drawString(7,5,currentTempC);
  //u8x8.drawString(0,7,bufRunningTime);

  Serial.print("Current temperatureis : ");
  Serial.println(currentTempF);
}

void turnheaterOff(){
  digitalWrite(HEATER_PIN,LOW);
}

void turnHeaterOn(){
  digitalWrite(HEATER_PIN,HIGH);
}

void convertSecond2Time(unsigned lpSecond){
  unsigned int runSecond=lpSecond % 60;
  unsigned int runMinute=lpSecond / 60;
  sprintf(charBuffer,"%3d:%02d",runMinute,runSecond);
}
void timerOneSecond(){

  sensors.requestTemperatures(); // send the command to get temperatures
  currentTempF=sensors.getTempCByIndex(0);
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
          if(currentTempF<100) {
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
        if(currentTempF>=WARM_TEMP){
          turnheaterOff();
          workingStage=WORKING_STAGE::Waming;
        }else{
          turnHeaterOn();
        };
        if (workingStage==WORKING_STAGE::Waming) {
          countStageTimer++;
        } else {
          countStageTimer=0;
        }
        if(countStageTimer>=WARMING_DURATION || countWorkTimer>=MAX_RUN_TIME){
          workingMode=WORKING_MODE::None;
          workingStage=WORKING_STAGE::Stopping;
        };
        break;
      case WORKING_MODE::None:
        //countWorkTimer=0;
        break;
    }
    drawDisplay();
}