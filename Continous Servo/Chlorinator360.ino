// Imports the servo library
#include <Servo.h>

// Sets pin for TDS sensor
#define TdsSensorPin A1

#define VREF 3.3      
// analog reference voltage(Volt) of the ADC

// Number of counts in the buffer for data
#define SCOUNT  30           																																						                                                                            

// Initalize variables
int analogBuffer[SCOUNT];    																																						                                          
int analogBufferTemp[SCOUNT];	
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;


// Value [ms] to stop the continous servo from continously spinning
const int STOP = 1540;

// Value to get about 45 degree rotation
const int RUN = 122;

// Creates an instance of servo class
Servo s;

// To monitor change in servo setting
int currPos = 0;

//Chlorine is around 1 - 3.00 ppm for pools and spas respectivly
const float absTDSvalue = 100;
// TDS value that will trigger the dispensing mechanism

// Sets a prevPos to monitor servo changes
int prevPos = currPos;

void setup()
{
  // Sets baud rate so that outputs of the arduino can be monitored in the serial port
  Serial.begin(19200);

  // Sets the TDS Pin to take inputs  
  pinMode(TdsSensorPin, INPUT);


  //Sets the servo to digital pin 10 and sets the value to zero
  s.attach(10);
  s.write(0);
  delay(RUN);
  s.writeMicroseconds(STOP);

}
  
void loop()
{
  static unsigned long analogSampleTimepoint = millis();
  
  // Fills an array with raw data from the TDS sensor  
  if (millis() - analogSampleTimepoint > 50U)  																																		                                                                //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    																													                                                      //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  
  static unsigned long printTimepoint = millis();

  // Utalizes the filled array to get an average value that is then reported to the serial monitor
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
    if (tdsValue == 0)
    {
      tdsValue = 0.00001;
    }
  }
  
  float prevTDS = 0;

  // Vaildates that readings are made prior to actuating the dispensing mechanism
  if (tdsValue != 0 && prevTDS != tdsValue && tdsValue < absTDSvalue)
  {
    Serial.println(tdsValue);
    prevTDS = tdsValue;
    tdsValue = 0;
    prevPos = currPos;
    currPos += 45;
    s.write(currPos);
    delay(RUN);
    s.writeMicroseconds(STOP);
    Serial.println(s.read());
    if (currPos == 45 || currPos == 135 || currPos == 225 || currPos == 315)
    { 
      //delay(1000);
      prevPos = currPos;
      currPos += 45;
      s.write(currPos);
      delay(RUN);
      s.writeMicroseconds(STOP);
      //delay(10000);
    }
  }
}

// Averages the data in the array of raw
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
