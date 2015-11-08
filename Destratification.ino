/*
Destrat'3

Version without PID
*/
// include the library code:-
#include <avr/wdt.h> // WATCHDOG 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
//
boolean onboardLED; // define flag for onboard LED status.Onboard LED used to give a 'heartbeat' that toggles each run of main loop
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
const int immersionPin = 44; // Pin used for Immersion ON / OFF Relay
//
float Setpoint = 59.0; //define Destratification pump PID setpoint variable and initilise temperature (in celsius)
int minPumpSpeed = 100; //minimum PWM value to be used for pump
int pumpSpeed = minPumpSpeed; //initialise pump PWM output variable
int maxPumpSpeed = 255; //maximum PWM value available
float pumpProportional = 100; // proportional parameter 
int boilerLevel = 255; //initialise boiler PID output variable
boolean boilerRelayOn = false; // Used in datalogging to show whether boiler is actually turned on via relay (60 = ON, 0 = OFF)
//
int topADCValue = 0; //variable to store ADC value
int middleADCValue = 0; //variable to store ADC value
int bottomADCValue = 0; //variable to store ADC value
int flashCount = 0; // Used in flash function to indicate pump speed
int flashCountE = 0; //Used to flash function to indicate energy level
//
// Set up Temperature Smoothing and Averaging variables
//
float numberTempSamples = 5; // number of temperature samples used in rolling average calculation
unsigned long tempSampleInterval = 20000; // Interval (mS) between successive rolling average calculations
unsigned long lastTempSample; // 'Millis' reading when last rolling average samples were calculated
unsigned long immersionSwitchInterval = 300000; // minimum time allowed between switches of Immersion Heater control relay
unsigned long lastImmersionSwitch; // Used to store time Immersion Relay was last switched
float averageTopTemp = 50.0;
float averageMiddleTemp = 45.0;
float averageBottomTemp = 40.0;
float newAverageTopTemp = 50.0;
float newAverageMiddleTemp = 45.0;
float newAverageBottomTemp = 40.0;
//
const int smoothNo = 6; // Number of loops used in temp sensor smoothing
unsigned long smoothTime = 20; // mS delay between each smoothing ADC sample
int count = 0; //loop counter for temperature measurement smoothing
//
float smoothTopTemp;
float smoothMiddleTemp;
float smoothBottomTemp;
float topTemp = 50.0;
float middleTemp = 45.0;
float bottomTemp = 40.0;
float boilerRelaySetpoint = 67.0; // % energy value above which relay will be energised and prevent further gas heating of the water
float energy = 5.0; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
float lastEnergyPercent; // Used to stop repeated lines being output to serial monitor
float boilerPercent = 67.0;
float boilerProportional = 0.15; //proportional parameter applied to square of boiler error to give boilerLevel
float maxEnergy = 7.6; // total capacity of the cylinder in kWh - used to trip immersion heater relay
const unsigned nRecentEnergies = 40; //Number of recent energy values to store. MUST BE EVEN.
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
unsigned recentEnergiesIndex = 0; // initialise pointer to energy array
unsigned long recentEnergiesInterval = 12000; // time between successive energy readings (ms)
float newAverageEnergy = 5.0f; // variable to store calculated value of most recent energy average
float oldAverageEnergy = 5.0f; // variable to store calculated value of earlier energy average
float energyGradient = 0.0f; // variable to store calculated rate of change in energy over time 
const float deltaTime = (float)nRecentEnergies * 0.5f * (float)recentEnergiesInterval; // time between old and new average energy measurements - used to calculate the gradient
unsigned long elapsedMillis = 0; // to allow millis function to be used to time sampling rate
unsigned  long newMillis = 0; // as above
float hoursUntilFull = 1.0f / 0.0f;
unsigned long windowSize = 600000; // value for relay control window size
unsigned long windowStartTime; //used for relay control
unsigned long now; // used for boiler relay on-time calc
unsigned long boilerOn = 0; // value used to store scaled version of boiler demand level
float energyPercent = 80.0; // variable to hold % energy level
//
// Function to ConvertADC Values to Degrees C
//
float calcTempFromReadValue(int readValue);
//
// Function to flash Pump LED to indicate % running
//
void flashLED();
//
void setup() {
  wdt_enable(WDTO_8S); // WATCHDOG set to maximum available time of 8 Seconds
//
// Set up pins for status LEDs
//
  pinMode(pumpLEDPin,OUTPUT);
  pinMode(fullLEDPin,OUTPUT);
  pinMode(offLEDPin,OUTPUT);
  pinMode(relayPin,OUTPUT);
  pinMode(immersionPin,OUTPUT);
  pinMode(13,OUTPUT); // onboard LED
  //
  windowStartTime = millis(); //initialise value for relay control
  //
  //Setpoint = 59.0; 
  //
  Serial.begin(9600); // Start serial comms
  lcd.begin(16,2); // Start LCD Display
  //
  for(unsigned i = 0; i < nRecentEnergies; ++i) // Initiatise array for storing energy history (used in rolling average calculation). Set to 75% of maximum
  {
    recentEnergies[i] = maxEnergy * 0.75;
  }
  //
  // Briefly display software details on startup
  //
    lcd.setCursor(0,0); // set to top line
    lcd.print("   Destrat' 3   ");
    delay(2000);
    lcd.setCursor(0,0); // set to top line
    lcd.print("Date: 07/11/2015");
    delay(2000);
    lcd.setCursor(0,0); 
    lcd.print("Pump Setpoint   ");
    lcd.setCursor(5,1);
    lcd.print(Setpoint);
    delay(2000);
    lcd.setCursor(0,0); 
    lcd.print("Boilr Setpnt (%)");
    lcd.setCursor(5,1);
    lcd.print(boilerRelaySetpoint);
    delay(2000);
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
  boilerPercent = (boilerRelaySetpoint - energyPercent) * (boilerRelaySetpoint - energyPercent) * boilerProportional + 5; // Calc boilerLevel as propotional to square of % error plus offset
//
// Data validation on boilerPercent
//
  if (boilerPercent < 10.0)
    {
      boilerPercent = 10.0;
    }
  if (boilerPercent > 90.0)
    {
      boilerPercent = 100.00;
    }
  //
  if (boilerPercent == 100.0)
  {
    boilerOn = windowSize;
  }
  else
  {
    boilerOn = (windowSize / 100) * boilerPercent; // calc scaled version of boiler demand level
  }
  if (energyPercent > boilerRelaySetpoint) //Override logic if energy > setpoint
  { 
    boilerOn = 0;
    boilerPercent = 0.0;
  }
//
// Data Logging
//
  energyPercent =(energy/maxEnergy)*100.0;
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
    if (pumpSpeed > minPumpSpeed)
    {
    Serial.print((pumpSpeed - minPumpSpeed) * 100 / (maxPumpSpeed-minPumpSpeed));
    }
    else
    {
    Serial.print("0.0");
    }
    Serial.print(", ");
    Serial.print(boilerPercent);
    Serial.print(", ");
    Serial.println(boilerRelayOn);
    lastEnergyPercent = energyPercent;
  }
 //
 //************************************************
 //* turn the boiler relay pin on/off based on boilerLevel
 //************************************************
  now = millis();
  //
  // Set trap to stop boiler On values less than one whole minute
  //
  //if (boilerOn > 0)
  //{
  //  if (boilerOn < 60000)
  //  {
  //    boilerOn = 60000;
  //  }
  //} 
 // 
  if (now - windowStartTime > windowSize)
  { //time to shift the Relay Window
    windowStartTime += windowSize;
  }
  if (boilerOn > now - windowStartTime)
  { 
    boilerRelayOn = false; // Used in datalogging
    digitalWrite(relayPin,HIGH);
  }
  else 
  {
    boilerRelayOn = true; // Used in datalogging 
    digitalWrite(relayPin,LOW);
  } 
  //  
  lcd.setCursor(0,0);
//  if (energyPercent < boilerRelaySetpoint) // print warning if heat stored is < boiler setpoint
    if (boilerPercent > 10.0) // print warning if energy stored is < significantly under boiler setpoint
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
  if(abs(hoursUntilFull * 60.0f) > 99.0f)  // Display mins up to 99, otherwise general 'warming' or 'cooling' statement
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
      if(energy < maxEnergy)
      {
        lcd.setCursor(0,1);
        lcd.print("                ");
        lcd.setCursor(0,1);
        lcd.print(int(hoursUntilFull * 60.0f));
        lcd.print(" mins to Full");
      }
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
    lcd.print(int((pumpSpeed - minPumpSpeed) * 100 / (maxPumpSpeed-minPumpSpeed)));
    lcd.print("%");
    delay(800);
  }
//
//  Display Boiler % if Running
//
  lcd.setCursor(0,1); // set to second line
//  
  if (int(boilerPercent) > 0)
  {
    lcd.setCursor(0,1);
    lcd.print("Boiler ON       ");
    lcd.setCursor(10,1);
    lcd.print(int(boilerPercent));
    lcd.print("% ");
    if (boilerRelayOn)
    {
      lcd.setCursor(15,1);
      lcd.print("-"); // Append "+" to indicate when boiler relay is actually on
    }
    else
    {
      lcd.setCursor(15,1);
      lcd.print("+"); // Append space to indicate when boiler relay is actually off
    }
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
    energy = ((((topTemp + middleTemp + bottomTemp)/3.0)-15.0)) * 0.1975;
// WAS  energy = ((((topTemp + middleTemp + bottomTemp)/3)-15)*170000.0*4.183)/3.6/1000000.0; before being simplified
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
//  analogWrite(meterPin,(energy / maxEnergy)*255); // set voltage output for 'fuel gauge' (NOTE: not currently implemented)
//
//  Run pump PID and output to pump driver pin
//
//      myPID.Compute();
      pumpSpeed = ((topTemp - Setpoint) * pumpProportional) + minPumpSpeed; // pumpSpeed = minPumpSpeed + error time proportional value
      if (pumpSpeed > 254)
      {
          pumpSpeed = 254;
      }
      if (pumpSpeed < 1)
      {
          pumpSpeed = 0;
      }  
      if (topTemp > Setpoint)
      {
          analogWrite(pumpPin, pumpSpeed);
      }
      else
      {
          analogWrite(pumpPin,0);
      }
//  
// Tweak PID values near the setpoint
//
//    if ((topTemp - Setpoint) < 0.5) // Routine to increase "I" component when close to setpoint - to eliminate persistent error
//    {
//      myPID.SetTunings(100,0.1,0); // increased "I" near setpoint
//    }
//    else
//    {
//      myPID.SetTunings(100,0,0); // default values
//    } 
//
      if (pumpSpeed > minPumpSpeed) // flash pump LED when pump is running
        {
            digitalWrite(pumpLEDPin,HIGH);
//          flashLED(); // Turned off to prevent delay that it introduces from crashing the program
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
    delay(150);
    digitalWrite(pumpLEDPin,LOW);
    delay(150);
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
    delay(150);
    digitalWrite(offLEDPin,LOW);
    delay(150);
    flashCountE = flashCountE - 10; 
  }
   wdt_reset(); // WATCHDOG Reset
} 

