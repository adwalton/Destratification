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
  13/2/2015 - Increase energy array back up to 80 samples at 15S intervals = 20mins total
  01/06/2015 - Reduced Boiler PID proportional parameter from 25 to 20
  21/8/2015 -  Reduced Boiler PID proportional parameter from 20 to 15
  28/8/2015 - Reduced Pump TTop Setpoint to 60 degrees and Max energy to 8.0kWh
  31/8/2015 - Reduced 'steady temperature' threshold from 0.02 to 0.01
  14/09/2015 - Reduced Boiler PID proportional parameter from 15 to 2
  22/09/2015 - Reduced Boiler PID proportional parameter from 2 to 0.5
  29/9/2015 - Added code for Immersion Power Relay control. Turns OFF power to immersion if cylinder newAverageEnergy exceeds maxEnergy. Also made change to keep pump active all the time - so 
              TTop should be held at 'Setpoint' temperature. Setpoint reduced to 59C and maxEnergy reduced to 7.5kWh. Increased delay between samples used to calc average energy from 
              15 to 300 Sec (i.e. 5 mins) and number to 12 (i.e. 1 hour)
  30/10/2015 - Added smoothing function to temperature acquisition
  07/10/2015  - Changed temperature smoothing to add rolling exponential average over numberTempSamples at tempSampleInterval (mS). Set initial average temps to 55C to stop boiler or 
                pump starting after system reset
  09/10/2015 - Added positive minimum delay time allowed between immersion heater control relay operations (immersionSwitchInterval). Value set to 5 mins (300000 mS). Tidied up LCD display 
                cycling; including fixing situation where Pump Speed was not displayed when Low Temp warning was active
  12/10/2015 - Reduced Boiler relay set point and "Low Temp" warning level to middleTemp = 41 (rather than 42C). Corrected data type for immersion switching delay to unsigned long (to match 
                result from Millis() )  Reduce "p" component of Pump PID Loop from 200 to 100
  14/10/2015 - Added Watchdog function
  15/10/2015 - Updated datalogging serial output for easy Excel Import. Also changed boiler PID loop to control on total estimated energy % (not middleTemp)
  17/10/2015 - Added routine to add Integration paramenter to Pump PID if the distance from the setpoint temperature is less that 0.5C (to kill any persistent error)
 */
// include the library code:-
#include <avr/wdt.h> // WATCHDOG 
#include <PID_v1.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
//
boolean onboardLED; // define flag for onboard LED status.Onboard LED used to give a 'heartbeat' that toggles each run of main loop
//
int boilerActive; // used in datalogging to show whether boiler is on (50 = ON, 0 = OFF)
const int bottomTempPin = 8; // analog pin for bottom transducer
const int middleTempPin = 9; // analog pin for middle transducer
const int topTempPin = 10; // analog pin for top transducer
const int pumpPin = 10; // PWM pin for pump control
const int offLEDPin = 31; // Pin for driving Red/Amber LED indicating pump is off
const int fullLEDPin = 33; // Pin for driving LED indicating that Cylinder is 'full'
const int pumpLEDPin = 35; // Pin for driving pump active LED
const int meterPin = 9; //PWM pin for 'fuel gauge' (NOT CURRENTLY USED)
const int relayPin = 11; // Pin used to drive Central Heating Relay
const int lowAlarm = 41; // % of energy below which warning is displayed
const int immersionPin = 44; // Pin used for Immersion ON / OFF Relay
//
double Setpoint; //define Destratification pump PID setpoint variable
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
//
// Set up Temperature Smoothing and Averaging variables
//
double numberTempSamples = 5; // number of temperature samples used in rolling average calculation
int tempSampleInterval = 20000; // Interval (mS) between successive rolling average calculations
int lastTempSample; // 'Millis' reading when last rolling average samples were calculated
unsigned long immersionSwitchInterval = 300000; // minimum time allowed between switches of Immersion Heater control relay
unsigned long lastImmersionSwitch; // Used to store time Immersion Relay was last switched
float averageTopTemp = 50;
float averageMiddleTemp = 45;
float averageBottomTemp = 40;
float newAverageTopTemp = 50;
float newAverageMiddleTemp = 45;
float newAverageBottomTemp = 40;
//
int smoothNo = 6; // Number of loops used in temp sensor smoothing
const int smoothTime = 20; // mS delay between each smoothing ADC sample
int count = 0; //loop counter for temperature measurement smoothing
//
double smoothTopTemp;
double smoothMiddleTemp;
double smoothBottomTemp;
double topTemp = 50;
double middleTemp = 45;
double bottomTemp = 40;
double relaySetPoint = 67; // % energy value at which relay will be energised and prevent further gas heating of the water
double energy; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
double lastEnergyPercent; // Used to stop repeated lines being output to serial monitor
float boilerPercent;
float maxEnergy = 7.5; // total capacity of the cylinder in kWh - used to trip immersion heater relay
const unsigned nRecentEnergies = 40; //Number of recent energy values to store. MUST BE EVEN.
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
unsigned recentEnergiesIndex = 0; // initialise pointer to energy array
unsigned recentEnergiesInterval = 15000; // time between successive energy readings (ms)
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
double energyPercent = 80; // variable to hold % boiler level
//
// Function to ConvertADC Values to Degrees C
//
float calcTempFromReadValue(int readValue);
void flashLED(); //function to flash LED on Arduino board
// initialize the pump PID Loop
PID myPID(&topTemp, &pumpSpeed, &Setpoint,100,0,0, REVERSE); // PID for Pump Control
// initialize the Boiler PID Loop
PID boilerPID(&energyPercent, &boilerLevel, &relaySetPoint, 0.5, 0, 0, DIRECT); // error (degrees) * P = boilerLevel value
//
void setup() {
  wdt_enable(WDTO_8S); // WATCHDOG set to maximum of 8 Seconds
  // Set up pins for status LEDs
  pinMode(pumpLEDPin,OUTPUT);
  pinMode(fullLEDPin,OUTPUT);
  pinMode(offLEDPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  pinMode(immersionPin,OUTPUT);
  pinMode(13,OUTPUT); // onboard LED
  //
  windowStartTime = millis(); //initialise value for relay control
  //
  Setpoint = 59; // initilise temperature (in celsius) setpoint for destratification pump PID control algorithm
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
//
// Toggle Onboard LED each loop
//
    onboardLED = !onboardLED;
    digitalWrite(13,onboardLED);
 // 
  wdt_reset(); // WATCHDOG Reset
//
// Switch Immersion Relay based on 'Full' energy level and using minimum time allowed between switches
//
  now = millis();
  if (now - lastImmersionSwitch > immersionSwitchInterval)
   { 
    if(energy > maxEnergy)
     {
       digitalWrite(immersionPin,HIGH);
     } 
    else
     {
       digitalWrite(immersionPin,LOW);
     }
   lastImmersionSwitch = millis();
  }   
  //
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
  if (energyPercent > relaySetPoint) //Override PID if energy > setpoint
  { 
    boilerOn = 0;
    boilerPercent = 0;
  }
  if ((relaySetPoint - energyPercent) < 1.5) // Routine to introduce "I" component when close to setpoint - to eliminate persistent error
  {
    boilerPID.SetTunings(30,0.1,0);
  }
  else
  {
    boilerPID.SetTunings(30,0,0);
  } 
//
// Data Logging
//
  energyPercent =(energy/maxEnergy)*100;
//
  if(abs(lastEnergyPercent - energyPercent) > 0.1) // Only print if energy has changed 
  {
    Serial.print(millis());
    Serial.print(", ");
    Serial.print(topTemp);
    Serial.print(", ");
    Serial.print(middleTemp);
    Serial.print(", ");
    Serial.print(bottomTemp);
    Serial.print(", ");
    Serial.print(((energy/maxEnergy)*100));
    Serial.print(", ");
    Serial.print(pumpSpeed * 100 / (maxPumpSpeed-minPumpSpeed));
    Serial.print(", ");
    Serial.print(boilerPercent);
    Serial.print(", ");
    Serial.println(boilerActive);
    lastEnergyPercent = energyPercent;
  }
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
 // 
  if(now - windowStartTime > windowSize)
  { //time to shift the Relay Window
    windowStartTime += windowSize;
  }
  if(boilerOn > now - windowStartTime)
   { 
     digitalWrite(relayPin,HIGH);
     boilerActive = 50; // Used in datalogging
   }
  else 
   {
     digitalWrite(relayPin,LOW);
     boilerActive = 0; // Used in datalogging 
   } 
  //  
  lcd.setCursor(0,0);
  if (middleTemp < lowAlarm) // print warning if capacity is < lowAlarm
  {
     lcd.print("*** WARNING ****");
     lcd.setCursor(0,1); // set to next line
     lcd.print("** TEMP IS LOW *");
     lcd.noBacklight(); // Flash backlight
     delay(200);
     lcd.backlight();
     delay(200);
     lcd.noBacklight();
     delay(200);
     lcd.backlight(); // turn backlight on
     delay(200);
     lcd.noBacklight();
     delay(200);
     lcd.backlight(); // turn backlight on
  }
 //
 // Display Temperatures
 //
  lcd.setCursor(0,0);
  lcd.print(topTemp);
  lcd.setCursor(4,0);
  lcd.print("  ");
  lcd.print(middleTemp);
  lcd.setCursor(10,0);
  lcd.print("  ");
  lcd.print(bottomTemp);
  lcd.setCursor(0,1);
  lcd.print(" Energy =       ");
  lcd.setCursor(10,1);
  lcd.print(int(energyPercent));
  lcd.print("%");
  delay(800);
//
    if(energy >= maxEnergy)
    {
      lcd.setCursor(0,0);
      lcd.print("**TANK IS FULL**");
      delay(800);
    }
    if(int(abs(hoursUntilFull * 60.0f)) > 99)  // Display mins up to 2 hours, otherwise general 'warming' or 'cooling' statement
    {
      lcd.setCursor(0,1);
      if(newAverageEnergy > oldAverageEnergy)
      {
       lcd.print(" WARMING Slowly ");
      }
      else
      {
       lcd.print(" COOLING Slowly "); 
      }
    }
    else 
    {
      if(hoursUntilFull > 0.0f)
      {
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(int(hoursUntilFull * 60.0f));
        lcd.print(" mins to Full");
      }
      else
      {
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(abs(int(hoursUntilFull * 60.0f)));
        lcd.print(" mins to Empty");
      }
    }
//  
  delay(800);
  lcd.setCursor(0,1); // Set to second line
//  
  if (pumpSpeed > minPumpSpeed)
  {
      lcd.print("Pump ON         ");
      lcd.setCursor(9,1);
      lcd.print(int(pumpSpeed * 100.0f / (maxPumpSpeed-minPumpSpeed)));
      lcd.print("%");
      delay(800);
  }
//
//  Print Boiler % if Running
//
  lcd.setCursor(0,1); // set to second line
//  
  if (boilerLevel > 0.9)
  {
      lcd.setCursor(0,1);
      lcd.print(" Boiler ON      ");
      lcd.setCursor(11,1);
      lcd.print(int(boilerPercent));
      lcd.print("%");
  }
//
// Initialise values for smoothed temperature acquisition
//
  count = 0;
  topADCValue = 0;
  middleADCValue = 0;
  bottomADCValue = 0;
//
  while (count < smoothNo)
  {
    topADCValue = topADCValue + analogRead(topTempPin);
    delay(smoothTime);
    middleADCValue = middleADCValue + analogRead(middleTempPin);
    delay(smoothTime);
    bottomADCValue = bottomADCValue + analogRead(bottomTempPin);
    delay(smoothTime);
    count = count + 1;
  }
  topADCValue = topADCValue / smoothNo;
  middleADCValue = middleADCValue / smoothNo;
  bottomADCValue = bottomADCValue / smoothNo;
//
// End of smoothed temperature acquisition
//
  smoothTopTemp = calcTempFromRead(topADCValue);
  smoothMiddleTemp = calcTempFromRead(middleADCValue);
  smoothBottomTemp = calcTempFromRead(bottomADCValue);
//
// Update Rolling Average Temperatures and energy
//
  newMillis = millis();
  if (newMillis - lastTempSample > tempSampleInterval)
  {
  newAverageTopTemp = (averageTopTemp * ((numberTempSamples - 1)/(numberTempSamples))) + (smoothTopTemp / numberTempSamples);
  newAverageMiddleTemp = (averageMiddleTemp * ((numberTempSamples - 1)/(numberTempSamples))) + (smoothMiddleTemp / numberTempSamples);
  newAverageBottomTemp = (averageBottomTemp * ((numberTempSamples - 1)/(numberTempSamples))) + (smoothBottomTemp / numberTempSamples);
//
  topTemp = newAverageTopTemp;
  middleTemp = newAverageMiddleTemp;
  bottomTemp = newAverageBottomTemp;
//
  averageTopTemp = newAverageTopTemp;
  averageMiddleTemp = newAverageMiddleTemp;
  averageBottomTemp = newAverageBottomTemp;
//
  lastTempSample = millis(); //Reset last sample time
//
  }
//
  energy = ((((topTemp + middleTemp + bottomTemp)/3)-15)*170000*4.183)/3.6/1000000;
//
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
//  
    hoursUntilFull = (maxEnergy - energy) / (energyGradient * 3600000.0f);
    elapsedMillis = newMillis;
  }
  analogWrite(meterPin,(energy / maxEnergy)*255); // set voltage output for 'fuel gauge' (NOTE: not currently implemented)
//
//  Run pump PID and output to pump driver pin
//
      myPID.Compute();
      analogWrite(pumpPin,pumpSpeed); 
//  
// Tweak PID values near the setpoint
//
    if ((topTemp - Setpoint) < 0.5) // Routine to increase "I" component when close to setpoint - to eliminate persistent error
    {
      myPID.SetTunings(100,0.1,0); // increased "I" near setpoint
    }
    else
    {
      myPID.SetTunings(100,0,0); // default values
    } 
//
      if (pumpSpeed > minPumpSpeed) // flash pump LED when pump is running
        {
          flashLED();
        }
      else
        {
          digitalWrite(pumpLEDPin,LOW);
        }
//
//  Set Cylinder Full LED if required
//
    if (energy < maxEnergy)
    {
      digitalWrite(fullLEDPin,LOW);
      flashEnergyLED (); // Flash Amber LED to indicate energy level
    } // end of energy < max energy 
    else
    {
        // cylinder has reached capacity, so light the full LED
        digitalWrite(fullLEDPin,HIGH);
    }
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
  wdt_reset(); // WATCHDOG Reset
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
   wdt_reset(); // WATCHDOG Reset
} 
  

