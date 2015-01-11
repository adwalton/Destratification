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
  17/10/2014 - Changed control of Boiler relay to be based on TTop (rather than Capacity). Also changed to control via a second PID loop and code from Arduino PID library to allow this to drive a relay
  28/10/2014 - reduce Relay setpoint to 52 degrees (rather than 55). Added boiler % to LCD display. Set minimum boiler on time to 60 secs.
                Increased window to 10 mins. Increased energy averaging to 25 sample 15s apart
  29/10/2014 - reduced boiler PID "P" value from 20 to 15 to reduce overshoot
  31/10/2014 - set trap to inhibit PID control of boiler when TTop is above relaySetPoint
  3/11/2014 - reduced "I" parameter from 1 to 0.1
  7/11/2014 - Changed boiler control to work from middleTemp instead of TTop
  11/12/2014 - Confirmed P30 0I 0D settings using test code on prototype Arduino. Have now added code to change I to 0.01 when error is less than 1.5 degrees C
  16/12/2014 - Adjusted energy array and sample time to 40 samples 15S apart. Also increased 'temp steady' threshold from 0.01 to 0.03. Added backlight control for LCD - turn it off 
               when energy level is static (and unless cylinder is full
  18/12/2014 - Adjusted energy samples to 60 at 10S intervals (total 10 mins). Reduced "P" from 30 to 25 to slow down boiler heating and reduce over shoot.
  03-01-2015 - Adjusted energy array to 24 at 10S intervals (total 4 mins). Also removed 2S delay from end of main loop
  10/01/2015 - Stopped lighting up LCD when Energy is Low. Changed Low Energy LED Flash warning to only use Pump and Full LEDs
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
const int lowAlarm = 42; // % of energy below which warning is displayed
//
double Setpoint; //define Destratification pump PID setpoint variable
double tempInput; // variable to be used for measured temperature value
double pumpSpeed = 0; //initialise pump PWM output variable
double minPumpSpeed = 0; //minimum PWM value to be used for pump
double maxPumpSpeed = 255; //maximum PWM value available
double boilerLevel = 0; //initialise boiler PID output variable
//
int topADCValue = 0; //variable to store ADC value
int middleADCValue = 0; //variable to store ADC value
int bottomADCValue = 0; //variable to store ADC value
int flashCount = 0; // Used in flash function to indicate pump speed
int flashCountE = 0; //Used to flash function to indicate energy level
int smoothingIndex = 10; // used to smooth temperature measurements
// int energySteady = 0; // Used to inhibit pump if the energy is steady : 0 = energy changing, 1 = energy not changing
double topTemp;
double middleTemp;
double bottomTemp;
double relaySetPoint = 42; // middleTemp value at which relay will be energised and prevent further gas heating of the water
float energy; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
float maxEnergy = 8.2; // total capacity of the cylinder in kWh - used to halt destrat activity (Was 7.8)
const unsigned nRecentEnergies = 24; //Number of recent energy values to store. MUST BE EVEN.
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
unsigned recentEnergiesIndex = 0; // initialise pointer to energy array
unsigned recentEnergiesInterval = 10000; // time between successive energy readings (ms)
float newAverageEnergy = 0.0f; // variable to store calculated value of most recent energy average
float oldAverageEnergy = 0.0f; // variable to store calculated value of earlier energy average
float energyGradient = 0.0f; // variable to store calculated rate of change in energy over time 
const float deltaTime = (float)nRecentEnergies * 0.5f * (float)recentEnergiesInterval; // time between old and new average energy measurements - used to calculate the gradient
unsigned elapsedMillis = 0; // to allow millis function to be used to time sampling rate
unsigned newMillis = 0; // as above
float hoursUntilFull = 1.0f / 0.0f;
unsigned long windowSize = 600000; // value for relay control window size
unsigned long windowStartTime; //used for relay control
unsigned long now; // used for boiler relay on-time calc
unsigned long boilerOn = 0; // value used to store scaled version of boiler demand level
float boilerPercent; // variable to hold % boiler level
  //
  // Function to ConvertADC Values to Degrees C
  //
float calcTempFromReadValue(int readValue);
void flashLED(); //function to flash LED on Arduino board
// initialize the pump PID Loop
PID myPID(&topTemp, &pumpSpeed, &Setpoint,60,0.01,0, REVERSE);
// initialize the Boiler PID Loop
PID boilerPID(&middleTemp, &boilerLevel, &relaySetPoint, 25, 0, 0, DIRECT); // error (degrees) * P = boilerLevel value
//
void setup() {
  // Set up pins for status LEDs
  pinMode(pumpLEDPin,OUTPUT);
  pinMode(fullLEDPin,OUTPUT);
  pinMode(offLEDPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  //
  windowStartTime = millis(); //initialise value for relay control
  //
  tempInput = analogRead(topTempPin); //read temperature value (no longer used
  Setpoint = 63; // initilise temperature (in celsius) setpoint for destratification pump PID control algorithm
  //
  myPID.SetOutputLimits(minPumpSpeed, maxPumpSpeed); 
  myPID.SetMode(AUTOMATIC); // turn on the PID loop
  myPID.SetSampleTime(20000); // from PID library - sets sample time to X milliseconds
  //
  boilerPID.SetMode(AUTOMATIC); // turn on PID loop
  //boilerPID.SetOutputLimits(0, windowSize); // Commented Out - since it doesn't seem to work (probably because my windowSize is too big?)
  boilerPID.SetSampleTime(60000); // set sample time to 1 minute (note - this doesn't seem to work?)
  //
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
  boilerPID.Compute();
  boilerPercent = boilerLevel/2.55; // set boiler % variable
  //
  if (boilerLevel == 255)
  {
    boilerOn = windowSize;
  }
  else
  {
    boilerOn = (windowSize / 255) * boilerLevel; // calc scaled version of boiler demand level
  }
  if (middleTemp > relaySetPoint) //Override PID if TTop > setpoint
  { 
    boilerOn = 0;
    boilerPercent = 0;
  }
  if ((relaySetPoint - middleTemp) < 1.5) // Routine to introduce "I" component when close to setpoint - to eliminate persistent error
  {
    boilerPID.SetTunings(30,0.1,0);
  }
  else
  {
    boilerPID.SetTunings(30,0,0);
  } 
  //Serial.println(" ");
  //Serial.print("now - windowStartTime = ");
  Serial.print((now - windowStartTime));
  //Serial.print("windowSize = ");
  //Serial.println(windowSize);
  Serial.print(", ");
  Serial.print(topTemp);
  Serial.print(", ");
  Serial.print(middleTemp);
  Serial.print(", ");
  Serial.print(bottomTemp);
  Serial.print(", ");
  Serial.print(boilerPercent);
  Serial.print(", ");
  Serial.print(((energy/maxEnergy)*100));
 //
 //************************************************
 //* turn the relay pin on/off based on pid output
 //************************************************
  now = millis();
  //
  // Set trap to stop boiler On values less than one whole minute
  //
  if (boilerOn > 0)
  {
    if (boilerOn < 60000)
    {
      boilerOn = 60000;
    }
  } 
 Serial.print(", ");
 Serial.println(boilerOn);
 // 
  if(now - windowStartTime > windowSize)
  { //time to shift the Relay Window
    windowStartTime += windowSize;
  }
  if(boilerOn > now - windowStartTime) digitalWrite(relayPin,HIGH);
  else digitalWrite(relayPin,LOW);
  //  
  lcd.setCursor(0,0);
  if (middleTemp < lowAlarm) // print warning if capacity is < lowAlarm
  {
   lcd.print("*** WARNING ****");
   lcd.setCursor(0,1); // set to next line
   lcd.print("** TEMP IS LOW *");
   //lcd.backlight(); // turn backlight on
   //
   // FLASH Left Two LEDs to Indicate LOW TEMP
   digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,HIGH);
   delay(100);
   //digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(100);
   //digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,HIGH);
   digitalWrite(fullLEDPin,HIGH);
   delay(100);
   //digitalWrite(offLEDPin,LOW);
   digitalWrite(pumpLEDPin,LOW);
   digitalWrite(fullLEDPin,LOW);
   delay(1000);
   lcd.clear();
  }
  else // do nothing
  {
  }
  //lcd.autoscroll();
 // char string[30];
 // sprintf(string, "T:%2dC M:%2dC B:%2dC", (int)topTemp, (int)middleTemp, (int)bottomTemp);
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
      lcd.backlight(); // turn backlight on, since cylinder is full
      lcd.setCursor(0,0);
      lcd.print(string);
      delay(2000);
    }
    if(abs(oldAverageEnergy - newAverageEnergy) < 0.02) // If change in energy is below threshold consider the temperatures to be steady
    {
      sprintf(string, "** Temp Steady *");
      lcd.noBacklight(); // turn off backlight when nothing is happening
//      energySteady = 1; // set flag to inhibit pump operation
    }
    else if(hoursUntilFull > 0.0f)
    {
      char number[10];
      dtostrf(hoursUntilFull, 3, 1, number);
      sprintf(string, "%s Hrs to Full ", number);
      lcd.backlight(); // turn backlight on, since energy level is changing
//     energySteady = 0; //set flag to allow pump operation
    }
    else if(hoursUntilFull < 0.0f)
    {
      char number[10];
      dtostrf(-hoursUntilFull, 3, 1, number);
      sprintf(string, "%s Hrs to Empty", number);
      lcd.backlight(); // turn backlight on, since energy level is changing
//      energySteady = 0; //set flag to allow pump operation
    }
    else //hoursUntilFull == 0.0f
    {
      sprintf(string, "**Inf Gradient**");
    }
    
    lcd.print(string);
  }
//  
  lcd.setCursor(0,1); // set to second line
//  
  if (pumpSpeed > minPumpSpeed)
  {
    char string[30];
    sprintf(string, "Pump %2d%% Cap %2d%%", (int)(pumpSpeed * 100.0f / (maxPumpSpeed-minPumpSpeed)),(int)((energy/maxEnergy)*100)); // thia line was changed to replace "255.0f" with calc value
    lcd.print(string);
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
//  Print Boiler % if Running
//
  lcd.setCursor(0,1); // set to second line
//  
  if (boilerLevel > 0.9)
  {
    char string[30];
    sprintf(string, "Boil %2d%% Cap %2d%%", (int)(boilerPercent),(int)((energy/maxEnergy)*100)); // this line was changed to replace "255.0f" with calc value
    lcd.print(string);
  }
  else
  {
    // do nothing
  }
  //
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
//    Serial.println("Recent energy levels");
    for(unsigned i = 0; i < nRecentEnergies-1; ++i)
    {
 //     Serial.print(recentEnergies[i]);
 //     Serial.print(" ");
    }
 //   Serial.println(recentEnergies[nRecentEnergies-1]);
 //   Serial.print("New average energy: "); Serial.println(newAverageEnergy);
 //   Serial.print("Old average energy: "); Serial.println(oldAverageEnergy);
  }
  analogWrite(meterPin,(energy / maxEnergy)*255); // set voltage output for 'fuel gauge' (NOTE: not currently implemented)
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
//
  //delay(2000);
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
  

