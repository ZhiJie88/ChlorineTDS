#include <Servo.h>
#define TdsSensorPin A1
#define VREF 3.3      																																								                                                                              // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           																																						                                                                            // sum of sample point

int analogBuffer[SCOUNT];    																																						                                                                            // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];	
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;
Servo s;
int currPos = 0;
//Chlorine is around 1 - 3.00 ppm for pools and spas respectivly
const float absTDSvalue = 850;
int prevPos = currPos;

void setup()
{
  Serial.begin(19200);
  pinMode(TdsSensorPin, INPUT);
  s.attach(10);
}
  
void loop()
{
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 50U)  																																		                                                                //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    																													                                                      //read the analog value and store into the buffer
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
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; 																								                                              // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); 																												                                                    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; 																											                                                  //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge);		//convert voltage value to tds value
    Serial.print("voltage:");
    Serial.print(averageVoltage,2);
    Serial.print("V   ");
    Serial.print("TDS----Value:");
    Serial.print(tdsValue, 4);
    Serial.println("ppm");
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
