/* ChlorinatorTDS.ino
 * Author: Christopher Xu - KSU ME 574 Group 16
 */

///<summary>
/// Creates initial references
///</summary>
#include <Servo.h>
#define TdsSensorPin A1
#define VREF 3.3                                                                                                                                                                  
#define SCOUNT  30                                                                                                                                                                
Servo s;

///<summary>
///Initializes the analog buffer
///</summary>
int analogBuffer[SCOUNT];                                                                                                                                                          

///<summary>
///Initializes the temp analog buffer
///</summary>
int analogBufferTemp[SCOUNT]; 

///<summary>
/// Creates indexers for the arrays
///</summary>
int analogBufferIndex = 0, copyIndex = 0;

///<summary>
/// Initialize values to 0
///</summary>
float averageVoltage = 0, tdsValue = 0, temperature = 25;


///<summary>
/// Initializes the position value to zero
///</summary>
int currPos = 0;
//Chlorine is around 1 - 3.00 ppm for pools and spas respectivly

///<summary>
/// Upper limit for TDS values
///</summary>
const float absTDSvalue = 350;

///<summary>
/// Initializes previous poistion to the current position initailliy
///</summary>
int prevPos = currPos;


///<summary>
///Initializes all components, sets the TDS to pin A1 and the servo to pin 10
///</summary>
void setup()
{
  Serial.begin(19200);
  pinMode(TdsSensorPin, INPUT);
  s.attach(10);
}


///<summary>
///Arduino loop, operates on a set of timers to read and compute data, moving the dispensing mechanism if data is within range
///</summary>
void loop()
{
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 50U)                                                                               
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);  
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (currPos == 0 || currPos == 180)
  {
    s.write(currPos);
  }
  if (millis() % 10000U == 0 && millis()-printTimepoint > 5000U)
  {
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0;                                                                                              
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                                                                                                            
    float compensationVolatge = averageVoltage / compensationCoefficient;                                                                                                        
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge);   
    Serial.print("voltage:");
    Serial.print(averageVoltage,2);
    Serial.print("V   ");
    Serial.print("TDS----Value:");
    Serial.print(tdsValue, 4);
    Serial.println("ppm");
    if (tdsValue == 0)
    {
      tdsValue = 0.00001;
    }
  }
  float prevTDS = 0;
  if (tdsValue != 0 && prevTDS != tdsValue && tdsValue < absTDSvalue)
  {
    Serial.println(tdsValue);
    prevTDS = tdsValue;
    tdsValue = 0;
    if (currPos == 0 || (currPos > prevPos && currPos != 180))
    {
      prevPos = currPos;
      currPos += 45;
      s.write(currPos);
      Serial.println(s.read());
      if (currPos == 45 || currPos == 135)
      { 
        delay(1000);
        prevPos = currPos;
        currPos += 45;
        s.write(currPos);
      }
      delay(10000);
    }
    else
    {
      prevPos = currPos;
      currPos -= 45;
      s.write(currPos);
      Serial.println(s.read());
      if (currPos == 45 || currPos == 135)
      {
        delay(10000);
        prevPos = currPos;
        currPos -= 45;
        s.write(currPos);
      }
      delay(10000);
    }
  }
}


///<summary>
///From a given list of data gathered from a sensor, an average number is computed
///</summary>
///<param name="bArray">Array of sensor readings</param>
///<param name="iFilterLen">Length of the array of sensor readings</param>
///<return>The average sensor reading in a given array of sensoring readings<\returns>
int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}
