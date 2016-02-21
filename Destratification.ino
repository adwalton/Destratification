/*
Destrat'3

Version without PID
*/
// include the library code:-
#include <avr/wdt.h> // WATCHDOG Library (part of standard Arduino IDE installation) 
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
const int piezoPin = 37; // Pin used to drive piezo sounder
//
float Setpoint = 59.0; //define Destratification pump PID setpoint variable and initilise temperature (in celsius)
int minPumpSpeed = 100; //minimum PWM value to be used for pump
int pumpSpeed = minPumpSpeed; //initialise pump PWM output variable
int maxPumpSpeed = 255; //maximum PWM value available
float pumpProportional = 40.0; // proportional parameter 
int boilerLevel = 255; //initialise boiler PID output variable
boolean boilerRelayOn = false; // Used in datalogging to show whether boiler is actually turned on via relay (60 = ON, 0 = OFF)
//
int topADCValue = 0; //variable to store ADC value
int middleADCValue = 0; //variable to store ADC value
int bottomADCValue = 0; //variable to store ADC value
//int flashCount = 0; // Used in flash function to indicate pump speed
//int flashCountE = 0; //Used to flash function to indicate energy level
//
// Set up Temperature Smoothing and Averaging variables
//
float numberTempSamples = 20.0; // number of temperature samples used in rolling average calculation
unsigned long tempSampleInterval = 2000; // Interval (mS) between successive rolling average calculations
unsigned long lastTempSample; // 'Millis' reading when last rolling average samples were calculated
unsigned long immersionSwitchInterval = 300000; // minimum time allowed between switches of Immersion Heater control relay
unsigned long lastImmersionSwitch; // Used to store time Immersion Relay was last switched
float averageTopTemp = 59.0;
float averageMiddleTemp = 46.0;
float averageBottomTemp = 41.0;
float newAverageTopTemp = 59.0;
float newAverageMiddleTemp = 46.0;
float newAverageBottomTemp = 41.0;
//
const int smoothNo = 100; // Number of loops used in temp sensor smoothing
unsigned long smoothTime = 1; // mS delay between each smoothing ADC sample
int count = 0; //loop counter for temperature measurement smoothing
//
float smoothTopTemp;
float smoothMiddleTemp;
float smoothBottomTemp;
float topTemp = 59.0;
float middleTemp = 46.0;
float bottomTemp = 41.0;
float boilerRelaySetpoint = 66.0; // % energy value above which relay will be energised and prevent further gas heating of the water
float energy = 7.5; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
float lastEnergyPercent; // Used to stop repeated lines being output to serial monitor
float boilerPercent = 67.0;
float boilerProportional = 0.15; //proportional parameter applied to square of boiler error to give boilerLevel
float maxEnergy = 7.5; // total capacity of the cylinder in kWh - used to trip immersion heater relay
const unsigned nRecentEnergies = 20; //Number of recent energy values to store. MUST BE EVEN.
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
unsigned recentEnergiesIndex = 0; // initialise pointer to energy array
unsigned long recentEnergiesInterval = 15000; // time between successive energy readings (ms)
float newAverageEnergy = 7.5f; // variable to store calculated value of most recent energy average
float oldAverageEnergy = 7.5f; // variable to store calculated value of earlier energy average
float energyGradient = 0.0f; // variable to store calculated rate of change in energy over time 
const float deltaTime = (float)nRecentEnergies * 0.5f * (float)recentEnergiesInterval; // time between old and new average energy measurements - used to calculate the gradient
unsigned long elapsedMillis = 0; // to allow millis function to be used to time sampling rate
unsigned  long newMillis = 0; // as above
float hoursUntilFull = 1.0f / 0.0f;
unsigned long windowSize = 600000; // value for relay control window size
unsigned long windowStartTime; //used for relay control
unsigned long now; // used for boiler relay on-time calc
unsigned long boilerOn = 0; // value used to store scaled version of boiler demand level
float energyPercent = 100.0; // variable to hold % energy level
//
// Set up variables for timing main loop and hence checking whether Watchdog reset should be forced
//
unsigned long minLoopTimeAllowed = 1900; // minimum allowed duration of main loop - if actual loop time is shorter a Watchdog reset is forced
unsigned long lastLoopTime = millis(); // initialise time of last main loop
//
//
// Function to ConvertADC Values to Degrees C
//
float calcTempFromReadValue(int readValue);
//
// Function to flash Pump LED to indicate % running
//
//void flashLED();
//
void setup() {
  wdt_enable(WDTO_4S); // Set WATCHDOG (maximum POSSIBLE time is 8 Seconds)
//
// Set up pins for status LEDs, Relays and piezo sounder
//
  pinMode(pumpLEDPin,OUTPUT);
  pinMode(fullLEDPin,OUTPUT);
  pinMode(offLEDPin,OUTPUT);
  pinMode(13,OUTPUT); // onboard LED
  pinMode(relayPin,OUTPUT);
  pinMode(immersionPin,OUTPUT);
  pinMode(piezoPin,OUTPUT);
//
  digitalWrite(relayPin,HIGH); // Turn off boiler relay in first instance
//
  windowStartTime = millis(); //initialise value for relay control
//
  Serial.begin(9600); // Start serial comms
  lcd.begin(16,2); // Start LCD Display
  //
  // Prime Rolling Average Array with values
  //
  for(unsigned i = 0; i < nRecentEnergies; ++i) // Initiatise array for storing energy history (used in rolling average calculation)
  {
    recentEnergies[i] = maxEnergy;
  }
  //
  // Briefly display software details on startup
  //
    lcd.setCursor(0,0); // set to top line
    lcd.print("   Destrat' 3   ");
    tone(piezoPin,500,200);
//    tone(piezoPin,500,2000);
    delay(2000);
//    noTone(piezoPin);
    wdt_reset(); // WATCHDOG Reset
    lcd.setCursor(0,0); // set to top line
    lcd.print("Date: 21/02/2016");
//    tone(piezoPin,1000,2000);
    delay(2000);
//    noTone(piezoPin);
    wdt_reset(); // WATCHDOG Reset
    lcd.setCursor(0,0); 
    lcd.print("Pump Setpoint   ");
    lcd.setCursor(5,1);
    lcd.print(Setpoint);
    lcd.print("C");
//    tone(piezoPin,1500,2000);
    delay(2000);
//    noTone(piezoPin);
    wdt_reset(); // WATCHDOG Reset
    lcd.setCursor(0,0); 
    lcd.print("Boiler Setpoint");
    lcd.setCursor(5,1);
    lcd.print(boilerRelaySetpoint);
    lcd.print("%");
//    tone(piezoPin,2000,2000);
    delay(2000);
//    noTone(piezoPin);
    wdt_reset(); // WATCHDOG Reset
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
    if(energy >= maxEnergy)
     {
        digitalWrite(immersionPin,HIGH);
        digitalWrite(fullLEDPin,HIGH);
        lcd.setCursor(0,0);
        lcd.print("**TANK IS FULL**");
        delay(800);
     } 
    else
     {
       digitalWrite(immersionPin,LOW);
       digitalWrite(fullLEDPin,LOW);
     }
   lastImmersionSwitch = millis();
  }
//
//  Set Gas Central Heating Override Relay
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
  energyPercent =(energy/maxEnergy)*100.0;
//
// Data Logging - comment out unless actually being used, since it may cause hang-ups due to serial timing issues
//
/*
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
*/ 
// END OF DATALOGGING SECTION (Normally commented-out)  
//
//************************************************
//* turn the boiler relay pin on/off based on boilerLevel
//************************************************
  now = millis();
//   
  if (now - windowStartTime > windowSize)
  { //time to shift the Relay Window
    windowStartTime += windowSize;
  }
  if (boilerOn > now - windowStartTime)
  { 
    boilerRelayOn = false; // Used in datalogging
    digitalWrite(relayPin,HIGH);
    digitalWrite(offLEDPin, HIGH); // Extinguish Amber LED
  }
  else 
  {
    boilerRelayOn = true; // Used in datalogging 
    digitalWrite(relayPin,LOW);
    digitalWrite(offLEDPin, LOW); // Light Amber LED
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
//  lcd.print(energyPercent);
  lcd.print(int(energyPercent));
  lcd.print("%");
  delay(800);

  if(energy >= maxEnergy)
  {
      lcd.setCursor(0,0);
      lcd.print("**TANK IS FULL**");
      delay(800);
  }
  else
  {
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
 }
//  
  delay(800);
  lcd.setCursor(0,1); // Set to second line
//  
  if (pumpSpeed >= minPumpSpeed)
  {
    lcd.print("Pump ON         ");
    lcd.setCursor(9,1);
    lcd.print(int(5 + (((pumpSpeed) - minPumpSpeed) * 95 / (maxPumpSpeed-minPumpSpeed)))); // "+ 10" offset to ensure 0% isn't displayed when pump is running slowly
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
    delay(800);
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
  if ((newMillis - lastTempSample) > tempSampleInterval)
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
    if((newMillis - elapsedMillis) > recentEnergiesInterval)
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
//  Set Pump Speed
//
//  if ((topTemp-Setpoint) > 0)
  if (topTemp > Setpoint)
  {
    pumpSpeed = (((topTemp - Setpoint)*(topTemp - Setpoint)) * pumpProportional) + minPumpSpeed; // pumpSpeed = minPumpSpeed + error times proportional value
    if (pumpSpeed > maxPumpSpeed)
    {
      pumpSpeed = maxPumpSpeed;
    }
    analogWrite(pumpPin, pumpSpeed);
    digitalWrite(pumpLEDPin,HIGH);
  }
  else
  {
    pumpSpeed = 0; 
    analogWrite(pumpPin, pumpSpeed);
    digitalWrite(pumpLEDPin,LOW);
  }
    wdt_reset(); // WATCHDOG Reset
    lastLoopTime = millis() - now; // reset main loop timer and then test it's value
    Serial.println(lastLoopTime); 
    if (lastLoopTime < minLoopTimeAllowed)
    {
      delay(5000); // Add delay > Watchdog reset time, so that it forces a reset
    }
} // end of main Loop
//
// Function to ConvertADC Values to Degrees C
//
    float calcTempFromRead(int readValue)
    {
    return ((float)readValue * 500.0f) / 1024.0f;
    }

