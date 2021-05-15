/*
ESPboy Digital Ruler with vl53l1x sensor
for www.ESPboy.com project by RomanS
v1.0
*/

#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include <Wire.h>
#include <VL53L1X.h>

#define DELAY_BETWEEN_READS 50
#define AVERAGE_QUANTITY 10

ESPboyInit myESPboy;
VL53L1X sensor;

int16_t measuringsRange[AVERAGE_QUANTITY];
int16_t measuringsAmbient[AVERAGE_QUANTITY];
bool distanceStartFrom; //0 - top of the device; 1 - bottom of the device +9cm

void setup(){
  Serial.begin(115200);

  myESPboy.begin("Digital ruler");

  Wire.begin();
  Wire.setClock(400000); // use 400 kHz I2C
  sensor.setTimeout(500);

 if (!sensor.init()){
    myESPboy.tft.setTextColor(TFT_RED);
    myESPboy.tft.setTextSize(1);
    myESPboy.tft.drawString(F("Module not found"),16,120);
    while (1) delay(100);
  }

  sensor.setDistanceMode(VL53L1X::Long);
  sensor.setMeasurementTimingBudget(50000);
  sensor.startContinuous(50);

  memset(measuringsRange, 0, sizeof(measuringsRange));
  memset(measuringsAmbient, 0, sizeof(measuringsAmbient));

  distanceStartFrom=0;
}




void loop(){
  static uint32_t range, ambient, stat, count=0;
  static String tsStatus;
  

  sensor.read();
  range = sensor.ranging_data.range_mm;
  stat = sensor.ranging_data.range_status;
  tsStatus = VL53L1X::rangeStatusToString(sensor.ranging_data.range_status);
  ambient = sensor.ranging_data.ambient_count_rate_MCPS;

  measuringsRange[count] = range;
  measuringsAmbient[count] = ambient;
  count++;
  if(count>AVERAGE_QUANTITY)count=0;

  myESPboy.tft.setTextColor(TFT_WHITE, TFT_BLACK);
  myESPboy.tft.setTextSize(1);
  String toPrint;
  if (distanceStartFrom) toPrint = F("Distance from bottom");
  else toPrint = F("Distance from top   ");
  myESPboy.tft.drawString(toPrint,0,0);

  if(myESPboy.getKeys()) {
    distanceStartFrom = !distanceStartFrom;
    delay(200);}

  if(stat == 0) myESPboy.tft.setTextColor(TFT_GREEN, TFT_BLACK);
  else myESPboy.tft.setTextColor(TFT_RED, TFT_BLACK);

  myESPboy.tft.drawString(F("                     "),0,8);
  myESPboy.tft.drawString(tsStatus,0,8);

  range=0;
  ambient=0;
  for (uint8_t i=0; i<AVERAGE_QUANTITY; i++){
    range += measuringsRange[i];
    ambient += measuringsAmbient[i];
  };
  range/=AVERAGE_QUANTITY;
  ambient/=AVERAGE_QUANTITY;
  if(distanceStartFrom)range+=90;
  
  myESPboy.tft.setTextSize(4);
  myESPboy.tft.drawString("       ",0,32);
  myESPboy.tft.drawString((String)range,0,32);
  myESPboy.tft.setTextSize(2);
  myESPboy.tft.drawString(F("mm"),0,64);

  myESPboy.tft.setTextSize(1);
  toPrint = F("Ambient: ");
  toPrint += (String)ambient;
  myESPboy.tft.drawString(F("                   "),0,96);
  myESPboy.tft.drawString(toPrint,0,96);
  
  delay(DELAY_BETWEEN_READS);  
}
