
/*
  Collect Cylinder Temperatures from Three LM35 sensors on the Hot Water Cylinder and use
  the top one to control the destratification pump via a PID algorithm
  (obtained from the Arduino site - library)
  Also drives serial LCD display to show various values including gradient of energy over time, based on an array of 
  sampled values used to produce a rolling average
  Also drives main relay to disengage the gas central heating cylinder thermostat when the water is hot enough (but leaving 'headroom' for solar heating)
  9/6/2014 - added flash to Amber LED when capacity < 'lowAlarm' value (also used to display warning message) and increased gain on proportional PID parameter
  11/6/2014 - changed PID setPoint to be actual centigrade value, rather than ADC one. Also reduced LED flash count to give max of 10 (not 11)
  13/6/2014 - moved PID compute call into timed section that runs on fixed cycle (this to improve the I D calculation accuracy). Also increased Max Energy to 7.8kWh and TTop setpoint to 60C
  17/6/2014 - implemented SetSampleTime from PID library to manage sample rate (instead of using change of 13/6/2014. Also turned off all but proportional PID. 
              Increased energy steady deadband from 0.03 to 0.04. Also fixed non-working "Tank Is Full" message
  19/6/2014 - changed temp print out to show decimal places. Also changed setpoint to 61C
  20/6/2014 - increased max energy to 8.2 kWh and TTop setpoint to 62C. Also reduced PID sample time from 30s to 15s
  24/6/2014 - Added Integral cooeficient back into PID. Improved format of temperature displays. Reduced energy averaging interval to 10 Secs 
  29/6/2014 - Reduced "I" value from 10 to 5 and threashold for displaying "Temp Steady" from 0.015 to 0.01
  12/8/2014 - Elliminated "I" from PID. Reduced "P" coefficient from 150 to 120. Changed logic to flash amber LED to indicate energy level (until full, when green LED comes on). 1 flash = 10% of energy
  24/8/2014 - Increased Top Temp Setpoint to 63C and reduced PID cooefficients to 60,0.05,0
  30/8/2014 - Added 'ripple' flash of all LEDs to indicate when midTemp is below alarm level
  3/9/2014 - Reduced "I" to 0.01 (was 0.05)
 */

// include the library code:
#include <PID_v1.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
//
const int bottomTempPin = 8; // analog pin for bottom transducer
const int middleTempPin = 9; // analog pin for middle transducer
const int topTempPin = 10; // analog pin for top transducer
const int pumpPin = 10; // PWM pin for pump control
const int offLEDPin = 31; // Pin for driving Red/Amber LED indicating pump is off
const int fullLEDPin = 33; // Pin for driving LED indicating that Cylinder is 'full'
const int pumpLEDPin = 35; // Pin for driving pump active LED
const int meterPin = 9; //PWM pin for 'fuel gauge' (NOT CURRENTLY USED)
const int relayPin = 11; // Pin used to drive Central Heating Relay
const int lowAlarm = 40; // % of energy below which warning is displayed
//
double Setpoint; //define PID setpoint variable
double tempInput; // variable to be used for measured temperature value
double pumpSpeed = 0; //initialise pump PWM output variable
double minPumpSpeed = 0; //minimum PWM value to be used for pump
double maxPumpSpeed = 255; //maximum PWM value available

//
int topADCValue = 0; //variable to store ADC value
int middleADCValue = 0; //variable to store ADC value
int bottomADCValue = 0; //variable to store ADC value
int flashCount = 0; // Used in flash function to indicate pump speed
int flashCountE = 0; //Used to flash function to indicate energy level
// int energySteady = 0; // Used to inhibit pump if the energy is steady : 0 = energy changing, 1 = energy not changing
double topTemp;
double middleTemp;
double bottomTemp;
float relaySetPoint = 60; // % Energy Value at which relay will be energised and prevent further gas heating of the water
float energy; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
float maxEnergy = 8.2; // total capacity of the cylinder in kWh - used to halt destrat activity (Was 7.8)
const unsigned nRecentEnergies = 20; //Number of recent energy values to store. MUST BE EVEN.
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
unsigned recentEnergiesIndex = 0; // initialise pointer to enegy array
unsigned recentEnergiesInterval = 10000; // time between successive energy readings (ms)
float newAverageEnergy = 0.0f; // variable to store calculated value of most recent energy average
float oldAverageEnergy = 0.0f; // variable to store calculated value of earlier energy average
float energyGradient = 0.0f; // variable to store calculated rate of change in energy over time 
const float deltaTime = (float)nRecentEnergies * 0.5f * (float)recentEnergiesInterval; // time between old and new average energy measurements - used to calculate the gradient
unsigned elapsedMillis = 0; // to allow millis function to be used to time sampling rate
unsigned newMillis = 0; // as above
float hoursUntilFull = 1.0f / 0.0f;
  //
  // Function to ConvertADC Values to Degrees C
  //
float calcTempFromReadValue(int readValue);
void flashLED(); //function to flash LED on Arduino board

// initialize the PID Loop
PID myPID(&topTemp, &pumpSpeed, &Setpoint,60,0.01,0, REVERSE);

void setup() {
  // Set up pins for status LEDs
  pinMode(pumpLEDPin,OUTPUT);
  pinMode(fullLEDPin,OUTPUT);
  pinMode(offLEDPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  //
  tempInput = analogRead(topTempPin); //read temperature value (no longer used
  Setpoint = 63; // initilise temperature (in celsius) setpoint for PID control algorithm
  //
  myPID.SetOutputLimits(minPumpSpeed, maxPumpSpeed); 
  myPID.SetMode(AUTOMATIC); // turn on the PID loop
  myPID.SetSampleTime(20000); // from PID library - sets sample time to X milliseconds
  Serial.begin(9600); //start serial comms
  lcd.begin(16,2);
  
  for(unsigned i = 0; i < nRecentEnergies; ++i) // Initiatise array for storing temperature history (used in rolling average calculation)
  {
    recentEnergies[i] = 0.0f;
  }
}
void loop() {
  
  //Set Gas Central Heating Override Relay
  //
  if (((energy/maxEnergy)*100) > relaySetPoint)
  {
    digitalWrite(relayPin,LOW);
  }
  else
  {
    digitalWrite(relayPin,HIGH);
  }
  //  
  lcd.setCursor(0,0);
  if (middleTemp < lowAlarm) // print warning if capacity is < lowAlarm
  {
   lcd.print("*** WARNING ****");
   lcd.setCursor(0,1); // set to next line
   lcd.print("** TEMP IS LOW *");
   //
   // FLASH ALL LEDs to Indicate LOW TEMP
   digitalWrite(offLEDPin,HIGH);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,HIGH);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,HIGH);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,HIGH);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,HIGH);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   //
   //digitalWrite(offLEDPin,LOW); //set Amber/Red LED OFF - to generate flash effect on this LED while capacity is below lowAlarm threshhold
   //
   delay(2000);
   lcd.clear();
  }
  else // do nothing
  {
  }
  //lcd.autoscroll();
  char string[30];
  sprintf(string, "T:%2dC M:%2dC B:%2dC", (int)topTemp, (int)middleTemp, (int)bottomTemp);
  lcd.setCursor(0,0);
  lcd.print(topTemp);
  lcd.setCursor(5,0);
  lcd.print(" ");
  lcd.print(middleTemp);
  lcd.setCursor(10,0);
  lcd.print("  ");
  lcd.print(bottomTemp);
//  lcd.print(string);
//
  if(hoursUntilFull == hoursUntilFull) //hoursUntilFull is not NAN
  {
    char string[30];
    
    delay(2000);
    lcd.setCursor(0, 0);
    if(energy >= maxEnergy)
    {
      sprintf(string, "**Tank is Full**");
      lcd.setCursor(0,0);
      lcd.print(string);
      delay(2000);
    }
    if(abs(oldAverageEnergy - newAverageEnergy) < 0.01) // If change in energy is below threshold consider the temperatures to be steady
    {
      sprintf(string, "** Temp Steady *");
//      energySteady = 1; // set flag to inhibit pump operation
    }
    else if(hoursUntilFull > 0.0f)
    {
      char number[10];
      dtostrf(hoursUntilFull, 3, 1, number);
      sprintf(string, "%s Hrs to Full ", number);
//     energySteady = 0; //set flag to allow pump operation
    }
    else if(hoursUntilFull < 0.0f)
    {
      char number[10];
      dtostrf(-hoursUntilFull, 3, 1, number);
      sprintf(string, "%s Hrs to Empty", number);
//      energySteady = 0; //set flag to allow pump operation
    }
    else //hoursUntilFull == 0.0f
    {
      sprintf(string, "**Inf Gradient**");
    }
    
    lcd.print(string);
  }
  
  lcd.setCursor(0,1); // set to second line
//  Serial.print(", ");
//  Serial.println(" C");
  //lcd.print(" M ");      
  //lcd.print(middleTemp);
//  Serial.print(", ");
//  Serial.println(" C");
  //lcd.print(" B ");      
  //lcd.print(bottomTemp);
//  Serial.print(", ");
//  Serial.println(" C");
//  Serial.print(" Pump Speed = ");
  
  if (pumpSpeed > minPumpSpeed)
  {
    char string[30];
    sprintf(string, "Pump %2d%% Cap %2d%%", (int)(pumpSpeed * 100.0f / (maxPumpSpeed-minPumpSpeed)),(int)((energy/maxEnergy)*100)); // thia line was changed to replace "255.0f" with calc value
    lcd.print(string);
    // lcd.print("Pump ");
    // lcd.print((pumpSpeed/255)*100); // convert to % and print when running
    // lcd.print("% ");
  }
  else
  {
    char num1[10];
    char num2[10];
    dtostrf(energy, 5, 2, num1);
    dtostrf((energy/maxEnergy)*100, 5, 1, num2);
    lcd.print(num1); lcd.print(" kWh ");
    lcd.print(num2); lcd.print("%");
  }
  //Serial.println(" kWh");
  //
  topADCValue = analogRead(topTempPin);
  topTemp = calcTempFromRead(topADCValue);
  middleADCValue = analogRead(middleTempPin);
  middleTemp = calcTempFromRead(middleADCValue);
  bottomADCValue = analogRead(bottomTempPin);
  bottomTemp = calcTempFromRead(bottomADCValue);
  energy = ((((topTemp + middleTemp + bottomTemp)/3)-15)*170000*4.183)/3.6/1000000;
  
  newMillis = millis();
  if(newMillis - elapsedMillis > recentEnergiesInterval)
  {
    recentEnergies[recentEnergiesIndex] = energy;
    recentEnergiesIndex = (recentEnergiesIndex + 1) % nRecentEnergies;
    newAverageEnergy = oldAverageEnergy = 0.0f;
    for(unsigned i = recentEnergiesIndex; i < recentEnergiesIndex + (nRecentEnergies / 2); ++i)
    {
      oldAverageEnergy += recentEnergies[i % nRecentEnergies];
    }
    for(unsigned i = recentEnergiesIndex + (nRecentEnergies / 2); i < recentEnergiesIndex + nRecentEnergies; ++i)
    {
      newAverageEnergy += recentEnergies[i % nRecentEnergies];
    }
    newAverageEnergy /= (float)nRecentEnergies * 0.5f;
    oldAverageEnergy /= (float)nRecentEnergies * 0.5f;
    energyGradient = (newAverageEnergy - oldAverageEnergy)/deltaTime;
    
    hoursUntilFull = (maxEnergy - energy) / (energyGradient * 3600000.0f);
    elapsedMillis = newMillis;
    
    //Print out array
    Serial.println("Recent energy levels");
    for(unsigned i = 0; i < nRecentEnergies-1; ++i)
    {
      Serial.print(recentEnergies[i]);
      Serial.print(" ");
    }
    Serial.println(recentEnergies[nRecentEnergies-1]);
    Serial.print("New average energy: "); Serial.println(newAverageEnergy);
    Serial.print("Old average energy: "); Serial.println(oldAverageEnergy);
  }
  analogWrite(meterPin,(energy / maxEnergy)*255); // set voltage output for 'fuel gauge' (NOTE: not currently implemented
  //
  //if (energySteady = 0) //Execute pump control if energy level is not steady
  //{
    if (energy < maxEnergy)
    {
      digitalWrite(fullLEDPin,LOW);
      flashEnergyLED (); // Flash Amber LED to indicate energy level
      tempInput = analogRead(topTempPin);
      myPID.Compute(); // this line moved into timed energy smoothing loop
      analogWrite(pumpPin,pumpSpeed);
  //
        if (pumpSpeed > minPumpSpeed) // flash pump LED when pump is running
        {
          //digitalWrite(offLEDPin,LOW);
          flashLED();
        }
        else
        {
          digitalWrite(pumpLEDPin,LOW);
          //digitalWrite(offLEDPin,HIGH);
        }
     } // end of energy < max energy 
    else
    {
    // cylinder has reached capacity, so take no destrat action, just light the full LED
    digitalWrite(fullLEDPin,HIGH);
    digitalWrite(offLEDPin,LOW);
    pumpSpeed = 0; // Stop Pump
    analogWrite(pumpPin,pumpSpeed);
    }
//} // end of is energy steady loop
//else
//{ // energy is steady
 // if (energy > maxEnergy) 
//  {
///     digitalWrite(fullLEDPin,HIGH); // set Full LED On;
//     pumpSpeed = 0; // Stop Pump
//     analogWrite(pumpPin,pumpSpeed);
//  }
//  else
//  {
//    digitalWrite(fullLEDPin,LOW);
//    digitalWrite(offLEDPin,HIGH);
//    pumpSpeed = 0; // Stop Pump
//    analogWrite(pumpPin,pumpSpeed);
//  }
//} // end of energy steady loop

  delay(2000);
} // end of main Loop
//
  // Function to ConvertADC Values to Degrees C
  //
  float calcTempFromRead(int readValue) {
    return ((float)readValue * 500.0f) / 1024.0f;
  }
 void flashLED() // Function to flash pump LED when pump is running. 0 to 10 flashes according to speed value
{
  flashCount = pumpSpeed;
  while (flashCount > 0)
  {
    digitalWrite(pumpLEDPin, HIGH);
    delay(200);
    digitalWrite(pumpLEDPin,LOW);
    delay(200);
    flashCount = flashCount - 25.5;
  }
} 
void flashEnergyLED() // Function to flash pump LED when pump is running. 0 to 10 flashes according to speed value
{
  flashCountE = ((energy/maxEnergy)*100);
  while (flashCountE > 0)
  {
    digitalWrite(offLEDPin, HIGH);
    delay(200);
    digitalWrite(offLEDPin,LOW);
    delay(200);
    flashCountE = flashCountE - 10; 
 }
} 
  

