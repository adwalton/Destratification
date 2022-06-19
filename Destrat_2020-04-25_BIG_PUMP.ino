/*
  14/3/2016   Lowered TTop setpoint from 59 > 58C
  17/03/2016  Reduced TTop 58 > 46 and Max Energy 7.4 > 6.12
  18/03/2016  Increased boiler setpoint 66%>>75% to keep the kWh level about the same, after reducing the overall temperatures
  20/03/2016  Add delay to keep the destrat pump off for fixed period since last run - to stop pump hunting
  22/03/2016  Increased TTop Setpoint to 52C
  25/03/2016  Increase TTop setpoint to 53C. Added tones to sound when tank is full and immersion relay first switched OFF. Corrected error in pump anti-hunting logic
  30/03/2016  TTop set to 52C but maxEnergy upped to 6.95kWh
  02/04/2016  pumpProportional increased 40 >> 50
  24/04/2016  minLoopTimeAllowed decreased 1800 > 1500mS
  26/04/2016  Reduced maxEnergy to 6.75kW becasue I could hear 'kettling' at the higher value and the destrat pump was going to 80%+ as capacity approached 98%
  01/05/2016  Reduced boiler relay setpoint 70 > 65%. This may be too low but worth a try
  10/05/2016  Increased minimum allowed boiler % from 10% > 12%. (Since removing lagging from on immersion heater 10% doesn't seem to be enough
              (during the summer when the radiators are off)). Parameterised minimum boiler% as "minBoilerPercent" constant
  15/05/2016  Increased delay on Immersion relay from 5 mins to 10 mins
  16/05/2016  Increased delay on Immersion relay from 10 mins to 15 mins
  21/05/2016  Increase display time for Energy value
  26/05.2016  Increased min boiler on from 12 to 15%
  29/05/2016  Changed 'time to' displays to show mins and hours
  30/05/2016  Added 'Days' to time-to display options
  31/05/2016  Added emergency desconnection of immersion if topTemp exceeds new variable - maxTopTempAllowed. When tripped, display warning and flash Green 'Full' LED
  01/06/2016  Increased minBoilerOn 15% > 20%. Changed Time-To-Full logic to show time to boilerSetpoint below boilerSetpoint and time to full above this level
  07/06/2016  Adjusted gradient calc so that it still uses full range when cooling, even below boiler setpoint
  09/06/2016  Reduced maxTopTemp 60 >> 55. Reduced maxEnergy by 5%
  12/06/2016  Changed energyPercent to smoothed value (using 10 samples). This is to try and stop boiler cycling too much once setpoint is reached
  13/06/2016  Added smiley face! :)
  21/06/2016  Changed smiley's to be permanently displayed
  07/07/2016  TFT Added (LCD 2 x 16 display still works on I2C bus)
  10/07/2016  Added colour gradient to reflect temperature in cylinder + other cosmetic improvements to graph, etc.
  12/07/2016  Added pump & boiler status indication to rolling chart display
  15/07/2016  Changed energy graph scaling to be from lowestGraphPercent to 100% (rather than previous 0-100%). Also parameterised the red level displayed on the TFT energy graph (redEnergyPercen)
  17/07/2016  Added status display for immersion heater cut-out relay
  21/07/2016  Moved boiler and pump 'dials' to the right and added Warming/Cooling arrow with hourUntilFull numerals
  25/07/2016  Added 'scale' next to warming/cooling arrows. Reduced max energy 6.5 > 6.45kWh. Increased red energy level on TFT 40 > 50%. Reduced energy smoothing 'smoothNumber' 12 > 8
              Added blue spot to centre of Pump 'Dial' when pump is ON
  28/07/2016  Expanded cylinder graphic to use all pixels. Adjusted immersion relay logic to turn it on immerdiately when energy level falls below maxEnergy
  31/07/2016  Changed immersion logic to hyteresis (rather than timer)
  03/08/2016  Reduced numberTempSamples 20 > 10
  06/08/2016  Reduced maxEnergy 6.45 > 6.3
  14/08/2016  Added temperature offset(s) to compensate for error introduced when relays draw current. e.g. temperature readings change by c.1.4C when the boiler relay switches
  15/08/2016  While calibrating the ADC for the above offset it became apparent that ADC A8 was not working properly. Switched to A7 and all is fine again
  22/08/2016  Added & scale to RH side of graph
  02/09/2016  Switched to Dallas OneWire Temperature (including a fourth 'TEnd' sensor)
  03/09/2016  Added HW Outlet Temperature to TFT - and on graph
  04/09/2016  Changed boiler maximum non-100% value to be 80% (rather than previous 90%) This makes the minimum switching time for the boiler 2 mins at the lowest level and the highest
  06/09/2016  Added boiler coil temp transducer and included on TFT Display (including graph)
  07/09/2016  Improved and simplified % energy graph auto-scaling
  08/09/2016  Improved Cylinder Temperture display updates by adding sumTemps and oldSumTemps variable to detect significant changes in actual temps before updating display
  12/09/2016  Added traps to catch suspected errors in Dallas temperature reads
  18/09/2016  Improved 'traps' for Dallas errors and increased maxEnergy value 7.1 > 7.2 kWh
  08/10/2016  Increased maxEnergy 7.2 > 7.3 kWh
  10/10/2016  Added IFs to prevent boiler & HW temps being plotted on graph unless they're 'interesting' (to reduce clutter)
  20/10/2016  Removed graphs of coil and hot water outlet temp. Instead added status bar didplay of when coil temp > middleTemp or outlet temp > middleTemp
  11/11/2016  Removed boiler heating status bar from TFT display
  12/11/2016  Changed display of hot water being drawn to be based on outlet temp being > top temp - 3
  11/12/2016  Swapped priority of 'boiler ON' v 'HW drawn' status display above graph. Also reduced boilersetpoint 65% > 64%
  31/03/2017  Version of this file LOST due to backup failure :(
  31/03/2017  Added inner dial on boiler display to show status of relay window - uses windowPointer variable to track % progress through each time window
  01/04/2017  Changed pump display to use pointer
  19/04/2017  Changed threshold for displaying 'water beeing drawn' status from 0.85 > 0.78
  26/04/2017  Immersion relay failed so changed logic to elliminate the use of this relay. Instead the pump is disabled when energy > 100% and hence cause the convensional thermostat to trip
  27/04/2017  Also disabled pump when energy level is falling.
  01/05/2017  Changed threshold for displaying 'water beeing drawn' status from 0.78 > 0.75
  03/05/2017  Swapped order of status display logic to prioritise 'pump on' over 'full cylinder'
  04/05/2017  Added Status = 4 to replace previous logic for displaying hot water draw. Changed status to remove immersion relay status and use energyPercent >=100 instead. Physical relay removed
              System now relys on the original thermostat instead
  06/05/2017  Removed 'yellow' immersion relay open status. Added boolean valriable to temporally store status thresholds between graph array updates - thereby giving a more accurate record
              Also added hotFlow spot idicator to cylinder graphic when hot water is being drawn
  16/05/2017  Removed 'boiler on' from status display
  10/07/2017  Added graphic indicator to show status of graph refresh timer
  01/10/2017  Tried new rule for flagging boiler heating of water -> middleTemp  * 1.04 Also corrected logic for resetting hotFlow & boilerHeating flags
  03/10/2017  Added rule to prevent hotFlow flag being set if the temperature is falling. Also adjusted boilerHeating flag rule to add check that energyGradient warming exceeds 30W,
              as well as the coilTemp being warm
  22/10/2017  BoiletSetpoint 70%
  06/01/2018  Added code after main timed loop to force re-plot of graph if energyPercent changes by more than 1%, even if a timed, incremental replot isn't due
  12/01/2018  BoiletSetpoint 67.5%
  04/08/2018  Updated minPump Speed to account for new pump having different performance (was 100,now 110). Also increase frequesncy of temperature display updates (was 0.3C change; now 0.2C)
  15/09/2018  Updated minPump Speed to account for new pump having different performance (was 110,now 120)
  09/02/2019  Increased boiler setpoint 67.5 >> 70%
  06/11/2019  Change logic that sets boilerHeating flag true - now needs coilTemp to be 20C hotter than bottomTemp, as well as heating > 1000W (was previously 500W)
  14/11/2019  Changed logic that sets boilerHeating flag true - now needs coilTemp to be hotter than middleTemp, as well as heating > 1500W
  23/11/2019  Increased boiler heating setpoint from 70%
  18/12/2019  Changed BoilerHeating flag trigger to coil > topTemp & warming > 100W
  12/04/2020  Add IF to continously update cylinder temps, while the destrat pump is running. Increased maxEnergy from 7.3 >> 8.3 kWh
  13/04/2020  maxEnergy reduced to 8.1 and boilerSetpoint to 68%
  14/04/2020  maxEnergy reduced to 8.0
  16/04/2020  Set pump output level to always be 255 - to give constant 12V to drive relay and 240V mains pump. maxEnery set to 8.4 boilerRelaySetpoint = 65%
  19/04/2020  maxEnery set to 8.89 boilerRelaySetpoint = 62%
  20/04/2020  Updated Pump Display using new variables totalPumpONTime, CurrentPumpONTime, pumpONCount
  23/04/2020  Reduced temperature that bottom cylinder display changes from white to black. Was 29C, now 24C
  25/04/2020  Changed full to 8.134 kWh & TTop setpoint 60 > 62C
  27/04/2020  Change pumpMinOFF 90s > 4 mins
*/
char Date[ ] = "03/05/2020";
// include the library code:-
#include <avr/wdt.h> // WATCHDOG Library (part of standard Arduino IDE installation) 
#include <Wire.h>
#include <OneWire.h> // For OneWire Dallas sensors
#include <DallasTemperature.h> // For OneWire Temperature sensors with modifed WConstants.h
//
// Libraries and definitions for TFT Display
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"
#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif
#define _sclk 52
#define _miso 50
#define _mosi 51
#define _cs 53
#define _dc 9
#define _rst 8
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);
OneWire oneWire (24); //Pin for OneWire Bus DQ
DallasTemperature sensors (&oneWire); // onewire parameter on Dallas instance
//
// END of TFT display and Dallas OneWire sensor libraries and definitions
//
// COLOURS for TFT
//
const int t7 = 64168; // Colours for various lines and shapes
const int t6 = 64656; // 'warm' pink
const int t5 = 65112;
const int t4 = 65535;
//const int t3 = 56927;
const int t2 = 46271;
const int t1 = 33375; // 'cold' blue
const int BkGnd = 12678; // Background Grey
const int Chart = 29614; // Chart Colour (light grey)
int lowestGraphPercent = 0; // Variable to store current lowest energy % displayed on TFT. 0 to 100% if actual % is < energyGraphScaleSwitch . Otherwise the range is reduced
const int normalLowestGraphPercent = 20; // lowest energy % displayed on graph when actual % > energyGraphScaleSwitch
//const int normalLowestGraphPercent = 40; // lowest energy % displayed on graph when actual % > energyGraphScaleSwitch
const float energyGraphScaleSwitch = 30.0; // value below which energy % graph is switched to 0 - 100% (rather than normalLowestGraphPercent - 100%)
//const float energyGraphScaleSwitch = 50.0; // value below which energy % graph is switched to 0 - 100% (rather than normalLowestGraphPercent - 100%)
const int redEnergyPercent = 50; // Energy % below which RED is displayed on the TFT graph
const int orange = 64492; // Orange for endTemp graph
int i = 0; //Loop counter
//
boolean onboardLED; // define flag for onboard LED status.Onboard LED used to give a 'heartbeat' that toggles each run of main loop
//
const int bottomTempPin = 7; // analog pin for bottom transducer - NOTE: was 8 but this channel is faulty!!!
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
// Variables for Pump Dial Pointer
//
float theta; // Angle of pointer
int x1;
int y1;
int x2;
int y2;
int x3;
int y3;
//
float Setpoint = 62.0; //Setpoint for Top temperature(in celsius). Used to control destrat pump
int minPumpSpeed = 120; //minimum PWM value to be used for pump
int pumpSpeed = minPumpSpeed; //initialise pump PWM output variable
int previousPumpSpeed = 0; // Variable used to stop redrawing display 'dial' if speed hasn't changed
int maxPumpSpeed = 255; //maximum PWM value available
float pumpProportional = 50.0; // proportional parameter
int boilerLevel = 255; //initialise boiler output variable
boolean boilerRelayOn = false; // Used in datalogging to show whether boiler is actually turned on via relay
boolean firstFull = true; // Flag used to allow 'Tank Full Tune' to play once only
boolean immersionOFF = false; // Flag to store status of immersion cut-out relay
boolean hotFlow = false; // Flag set when hot water has been drawn  (but not yet drawn on TFT display)
boolean boilerHeating = false; // Flag set when coilTemp > middleTemp, i.e. boiler is heating the cylinder
boolean pumpOn = false; // Flag set when pump has been on (but not yet drawn on TFT display)
//
// Cylinder graph pixel locations for Y coordinates of temperature colour-coded fill
//
const int topPixel = 4; // top of temperature colour coding fill on cylinder graphic
const int midPixel = 119;
const int botPixel = 235;
float sumTemps = 0; // Used to record sum of all temps and use to retrict display updates until actual change occurs
float oldSumTemps = 0;
float pixelTemp; // Variable to store interpolated temperature for each row of pixels in cylinder graphic
float redTemp = Setpoint; // Minimum temperature that gives 100% red on TFT cylinder graphic
float blueTemp = 20.0;// Maximum temperature that gives 100% blue on TFT cylinder graphic
float arrowScaling = 0.45; // Scaling factor used to determine the length of the Warming / Cooling arrows on the TFT
int arrowLength = 0; // Calculated length of warming/cooling arrows on TFT Display
int red; // Variable to hold colour component values for TFT. 0-31 for Red and Blue, 0-63 for Green
int green; // for Adafruit TFT Display https://learn.adafruit.com/adafruit-gfx-graphics-library/graphics-primitives
int blue;
unsigned int pixelColour; // Variable to store calcuated colour value derived from pixelTemp - used to display cylinder graphic
//
unsigned long immersionSwitchInterval = 900000; // minimum time (mS) allowed between switches of Immersion Heater control relay
unsigned long lastImmersionSwitch; // Used to store time Immersion Relay was last switched
unsigned long pumpMinOff = 240000; // Minimum time between pump switch ONs (to stop pump 'hunting')
unsigned long lastPumpOFF = millis() + pumpMinOff; //Millis when pump last switched ON
float averageTopTemp = Setpoint;
float averageMiddleTemp = Setpoint;
float averageBottomTemp = Setpoint;
float newAverageTopTemp = Setpoint;
float newAverageMiddlboileeTemp = Setpoint;
float newAverageBottomTemp = Setpoint;
//
unsigned long smoothTime = 1; // mS delay between each smoothing ADC sample
int count = 0; //loop counter for temperature measurement smoothing
//
float smoothTopTemp;
float smoothMiddleTemp;
float smoothBottomTemp;
//
float topTemp = Setpoint;
float lastTopTemp = topTemp; // Used to store previous temp value, for use in fixing mis-reads of Dallas transducers
float middleTemp = Setpoint / 2;
float lastMiddleTemp = middleTemp;
float bottomTemp = Setpoint / 3;
float lastBottomTemp = bottomTemp;
float endTemp = Setpoint; //Transducer on HW outlet pipe
float lastEndTemp = endTemp; // Used in trap logic to elliminate 'wild' Dallas transducer readings
float smoothedEndTemp = endTemp; // Used to prevent hotFlow flag being set when endTemp is falling; i.e. after hot water draw has ended
float coilTemp = Setpoint / 2; //Transducer on boiler coil
float lastCoilTemp = coilTemp;
//
const float maxTopTempAllowed = 69.00; // above this temperature the immersion relay is immediately opened to stop further heating of the cylinder
float boilerRelaySetpoint = 67.0; // % energy value above which relay will be energised and prevent further gas heating of the water
float lastEnergyPercent; // Used to stop repeated lines being output to serial monitor and for smoothing energy value
float boilerPercent = 0.0;
float previousBoilerPercent = 10.0;// Make non-zero to force initial draw of boiler 'dial'
const float minBoilerPercent = 20.00; // Minimum boiler % allowed
float boilerProportional = 0.15; //proportional parameter applied to square of boiler error to give boilerLevel
const float maxEnergy = 8.134; // total capacity of the cylinder in kWh
float energy = maxEnergy; // Variable to hold calculated energy above 15C that gives an indication of the total heat in the cylinder
const int smoothNo = 2; // Number of loops used in temp sensor smoothing
// const int smoothNo = 100; // Number of loops used in temp sensor smoothing
const int nRecentEnergies = 4;
float recentEnergies[nRecentEnergies]; // Create array to store energy readings
int initialCount = 1; // counter used to override timed graph updates to allow fast population of graph on system startup
int graphEnergies[200]; // Set up array to store energy history for display
int graphTop[200]; // Set up arrays for three temperatures
int graphMiddle[200];
int graphBottom[200];
int graphEnd[200];
int graphCoil[200];
int graphStatus[200]; // Array to store pump and boiler On times. 0=all OFF, 1 = pump ON, 2 = Boiler ON, 3 = Immersion OFF
unsigned recentEnergiesIndex = 0; // initialise pointer to energy array
unsigned long recentEnergiesInterval = 150000; // time between successive energy readings (ms)and Update of TFT Energy Graph and rolling average energy for 'time-to' calculations
float updatePercent = 0.0; // Used to hold % of way through update cycle for energy graph
float newAverageEnergy = maxEnergy; // variable to store calculated value of most recent energy average
float oldAverageEnergy = maxEnergy; // variable to store calculated value of earlier energy average
float energyGradient = 0.0f; // variable to store calculated rate of change in energy over time
float newSmoothedEnergy = maxEnergy; // Added to give simple but smoother energy estimate that may give better warm-up/cool-down predictions
float oldSmoothedEnergy = maxEnergy;
float smoothNumber = 2.00; // Number of energy values to smooth over
//float smoothNumber = 12.00; // Number of energy values to smooth over
const float deltaTime = (float)nRecentEnergies * 0.5f * (float)recentEnergiesInterval; // time between old and new average energy measurements - used to calculate the gradient
unsigned long elapsedMillis = 0; // to allow millis function to be used to time sampling rate
unsigned  long newMillis = 0; // as above
float hoursUntilFull = 1.0f / 0.0f;
unsigned long windowSize = 606000; // value for boiler relay control window size 10.1 mins (not 10 mins - to reduce aliasing on status display on TFT)
unsigned long windowStartTime; //used for boiler relay control
unsigned long now; // used for boiler relay on-time calc
float windowPointer = 0.0; // used for boiler relay window progress display
unsigned long boilerOn = 0; // value used to store scaled version of boiler demand level
float energyPercent = 100.0; // variable to hold % energy level
float previousEnergyPercent = 100.0; // Used to identify and plot changes in energy level between routine update cycles
float hysteresisEnergyPercent = 100.0; // energy level at which immersion relay turns off (it turns back on when energy <100%)
//
// Set up variables for timing main loop and hence checking whether Watchdog reset should be forced
//
//unsigned long minLoopTimeAllowed = 1500; // minimum allowed duration of main loop - if actual loop time is shorter a Watchdog reset is forced
//unsigned long lastLoopTime = millis(); // initialise time of last main loop
//
// Variables for destrat pump monitoring
//
unsigned long totalPumpONTime = 0;
unsigned long currentPumpONTime = 0;
unsigned long previousCurrentPumpONTime = 1; // set not equal to currentPumpONTime to ensure initial write to TFT
unsigned long pumpONCount = 0;
unsigned long lastPumpONTime = 0; // (Pump monitoring code also uses lastPumpOFF variable, defined previously)
boolean pumpCycleStarted = false;
float pumpResetPercent = 50.0; // Energy level at and below which, the pump statistics are reset
//
void graphGrid(); // Draw energy graph grid
void drawCylinder(); // Draw Cylinder graphic
void displayTemps(); // Write 3 temperatures to TFT
void plotEnergy(); // Draw energy histogram
void displayPump(); // Draw pump on TFT
void displayBoiler(); // Display Boiler Status on TFT
void displayWarming(); // Display rate of warming (power in)
void displayCooling(); // Display rate of cooling (power out)
//
void setup() {
  //  wdt_enable(WDTO_4S); // Set WATCHDOG (maximum POSSIBLE time is 8 Seconds)
  //
  //  Initialise TFT
  //
  sensors.begin(); // Start Dallas sensors
  sensors.setResolution(12); // Set Sensor ADC precision to 12 bits (alternative is 9 bits)
  //
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(BkGnd);
  tft.setTextSize (3);
  tft.setTextColor(ILI9340_YELLOW);
  tft.setCursor(38, 40);
  tft.println("Destrat 3 TFT  ");
  tft.println(" ");
  tft.setTextColor(t6);
  tft.setCursor(38, 100);
  tft.print(Date);
  delay(5000);
  tft.fillScreen(BkGnd);// Grey
  graphGrid();
  tft.fillRoundRect(0, 0, 88, 240, 20, ILI9340_YELLOW); // Draw Cylinder
  //
  tft.fillCircle(272, 49, 47, ILI9340_YELLOW);//Draw Boiler Dial
  tft.fillCircle(272, 49, 45, ILI9340_BLACK);
  //  tft.fillCircle(172, 49, 47, ILI9340_YELLOW);// Draw Pump Dial
  //  tft.fillCircle(172, 49, 45, ILI9340_BLACK);
  //
  pinMode(pumpLEDPin, OUTPUT);
  pinMode(fullLEDPin, OUTPUT);
  pinMode(offLEDPin, OUTPUT);
  pinMode(13, OUTPUT); // onboard LED
  pinMode(relayPin, OUTPUT);
  pinMode(immersionPin, OUTPUT);
  pinMode(piezoPin, OUTPUT);
  //
  digitalWrite(relayPin, HIGH); // Turn off boiler relay in first instance
  //
  windowStartTime = millis(); //initialise value for relay control
  //
  Serial.begin(9600); // Start serial comms
  for (unsigned i = 0; i < 201; ++i) // Initialise energy, temperature & status history for graphs - fill with sin curves - just for fun
  {
    graphEnergies[i] = 40;
    graphTop[i] = int((sin(i / 20.00) * 50) + 60);
    graphMiddle[i] = int((sin((i + 20) / 20.00) * 45) + 60);
    graphBottom[i] = int((sin((i + 30) / 20.00) * 40) + 60);
    graphEnd[i] = int((sin((i + 40) / 20.00) * 35) + 60);
    graphCoil[i] = int((sin((i + 50) / 20.00) * 30) + 60);
    graphStatus[i] = 0;
  }
  //
  // Briefly display software details on startup
  //
  tone(piezoPin, 500, 200);
  delay(500);
  tone(piezoPin, 500, 200);
  energy = ((((topTemp + middleTemp + bottomTemp) / 3.0) - 15.0)) * 0.1975;
  newAverageEnergy = energy;
  oldAverageEnergy = energy;
  newSmoothedEnergy = energy;
  oldSmoothedEnergy = energy;
  energyPercent = (energy / maxEnergy) * 100.0;
  lastEnergyPercent = energyPercent;
  //
  // Prime Rolling Average Array with values
  //
  for (unsigned i = 0; i < nRecentEnergies; ++i) // Initiatise array for storing energy history (used in rolling average calculation)
  {
    recentEnergies[i] = energy;
  }
  //
  // END OF SETUP
}
void loop() {
  //
  // Toggle Onboard LED each loop
  //
  onboardLED = !onboardLED;
  digitalWrite(13, onboardLED);
  //
  if (energyPercent < pumpResetPercent) // Reset pump statistics if energy drops below this %
  {
    totalPumpONTime = 0;
    pumpONCount = 0;
    previousCurrentPumpONTime = 0;
    currentPumpONTime = 0;
  }
  //
  //wdt_reset(); // WATCHDOG Reset
  //
  // Switch Immersion Relay based on 'Full' energy level and using minimum time allowed between switches
  //
  now = millis();
  if (topTemp > maxTopTempAllowed) // Trap excessive Top Temp and turn OFF immersion immediately
  {
    digitalWrite(immersionPin, HIGH);
    digitalWrite(fullLEDPin, HIGH);
    immersionOFF = true;
    delay(500);
    digitalWrite(fullLEDPin, LOW); //Flash Full LED
  }
  else //Otherwise do normal energy check switching of immersion relay
  {
    if (energyPercent >= hysteresisEnergyPercent)
    {
      digitalWrite(immersionPin, HIGH);
      digitalWrite(fullLEDPin, HIGH);
      if (!immersionOFF)
      {
        tone(piezoPin, 200, 400);
        delay(500);
        tone(piezoPin, 400, 400);
        delay(500);
        tone(piezoPin, 600, 400);
        delay(500);
        tone(piezoPin, 800, 400);
      }
      immersionOFF = true; // Set Immersion status flag
    }
    if (energyPercent < 95.0)
    {
      digitalWrite(immersionPin, LOW);
      digitalWrite(fullLEDPin, LOW);
      immersionOFF = false; // Set immersion status flag
    }
  }
  //
  //  Set Gas Central Heating Override Relay
  //
  boilerPercent = (boilerRelaySetpoint - energyPercent) * (boilerRelaySetpoint - energyPercent) * boilerProportional + 5; // Calc boilerLevel as propotional to square of % error plus offset
  //
  // Data validation on boilerPercent
  //
  if (boilerPercent < minBoilerPercent)
  {
    boilerPercent = minBoilerPercent;
  }
  if (boilerPercent > 80.0)
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
  //Blink Cylinder Full (green) LED when above boilerRelaySetpoint, to represent level of 'fullness'
  //
  if ((energyPercent > boilerRelaySetpoint) && !digitalRead(fullLEDPin))
  {
    i = int(energyPercent / 10.0);
//    i = int(((energyPercent - boilerRelaySetpoint) / (100.0 - boilerRelaySetpoint)) * 10.0);
    //Serial.print("i = ");
    //Serial.print(i);
    //Serial.print(" , Energy% = ");
    //Serial.println(energyPercent);
    while (i > 0)
    {
      digitalWrite(fullLEDPin, HIGH);
      delay(300);
      //      delay(int(((energyPercent - boilerRelaySetpoint)/(100.0 - boilerRelaySetpoint)) * 1000.0));
      digitalWrite(fullLEDPin, LOW);
      delay(200);
      i--;
    }
  }
  lastEnergyPercent = energyPercent;
  energyPercent = (energy / maxEnergy) * 100.0; // Unsmoothed energy value
  /* //
    // Data Logging - comment out unless actually being used
    //

    if (abs(lastEnergyPercent - energyPercent) > 0.05) // Only print if energy has changed
    {
     Serial.print(millis());
     Serial.print(", ");
     Serial.print(topTemp);
     Serial.print(", ");
     Serial.print(middleTemp);
     Serial.print(", ");
     Serial.print(bottomTemp);
     Serial.print(", ");
     Serial.print(endTemp);
     Serial.print(", ");
     Serial.print(energyPercent);
     Serial.print(", ");
     if (pumpSpeed > minPumpSpeed)
     {
       Serial.print((pumpSpeed - minPumpSpeed) * 100 / (maxPumpSpeed - minPumpSpeed));
     }
     else
     {
       Serial.print("0.0");
     }
     Serial.print(", ");
     Serial.print(boilerPercent);
     Serial.print(", ");
     Serial.print(boilerRelayOn);
     Serial.print(", ");
     Serial.println(immersionOFF);
     lastEnergyPercent = energyPercent;
    }

    // END OF DATALOGGING SECTION (Normally commented-out)
    //*/
  //*******************************************************
  //* turn the boiler relay pin on/off based on boilerLevel
  //*******************************************************
  now = millis();
  windowPointer = (float(now - windowStartTime) / float(windowSize)) * 100.0; // Position for window progress display (from 0 > 100) // % of way through boiler relay 'window' - for display
  //
  if (now - windowStartTime > windowSize)
  { //time to shift the Relay Window
    windowStartTime += windowSize;
  }
  if (boilerOn > now - windowStartTime)
  {
    boilerRelayOn = false; // Used in datalogging
    digitalWrite(relayPin, HIGH);
    digitalWrite(offLEDPin, HIGH); // Illuminate Amber LED
  }
  else
  {
    boilerRelayOn = true; // Used in datalogging
    digitalWrite(relayPin, LOW);
    digitalWrite(offLEDPin, LOW); // Extinguish Amber LED
  }
  //
  newMillis = millis();
  //
  // Collect temperatures
  //
  sensors.requestTemperatures();
  topTemp = sensors.getTempCByIndex (3);
  middleTemp = sensors.getTempCByIndex (1);
  bottomTemp = sensors.getTempCByIndex (0);
  endTemp = sensors.getTempCByIndex (2);
  coilTemp = sensors.getTempCByIndex (4);
  // Trap wild readings
  if (topTemp < 0.0  || topTemp > 99)
  {
    topTemp = lastTopTemp;
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
  }
  else
  {
    lastTopTemp = topTemp;
  }
  if (middleTemp < 0.0 || middleTemp > 99)
  {
    middleTemp = lastMiddleTemp;
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
  }
  else
  {
    lastMiddleTemp = middleTemp;
  }
  if (bottomTemp < 0.0 || bottomTemp > 99)
  {
    bottomTemp = lastBottomTemp;
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
  }
  else
  {
    lastBottomTemp = bottomTemp;
  }
  if (endTemp < 0.0 || endTemp > 99)
  {
    endTemp = lastEndTemp;
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
  }
  else
  {
    lastEndTemp = endTemp;
  }
  smoothedEndTemp = smoothedEndTemp * 0.9 + endTemp * 0.1; // Update smoothed end temp
  //Serial.print("Hot Flow - ");
  //  Serial.print(hotFlow);
  //  Serial.print(", ");
  //  Serial.print(endTemp);
  //  Serial.print(", ");
  //  Serial.println(smoothedEndTemp * 1.005);
  if (coilTemp < 0.0 || coilTemp > 99)
  {
    coilTemp = lastCoilTemp;
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
    delay(500);
    tone(piezoPin, 4000, 2000);
    noTone(piezoPin);
  }
  else
  {
    lastCoilTemp = coilTemp;
  }
  sumTemps = topTemp + middleTemp + bottomTemp + endTemp + coilTemp; // Update current aggregate of all temps
  energy = ((((topTemp + middleTemp + bottomTemp) / 3.0) - 15.0)) * 0.1975; // estimated energy in kWh, above base of 15 degrees C
  //
  /*
    Serial.print (millis());
    Serial.print (", Top= ");
    Serial.print (topTemp);
    Serial.print (", Mid= ");
    Serial.print (middleTemp);
    Serial.print (", Bot= ");
    Serial.print (bottomTemp);
    Serial.print (", Out= ");
    Serial.print (endTemp);
    Serial.print (", Coil= ");
    Serial.print (coilTemp);
    Serial.print (", Sum= ");
    Serial.print (sumTemps);
    Serial.print (", oldSum= ");
    Serial.print (oldSumTemps);
    Serial.print (", Diff= ");
    Serial.println (abs(sumTemps - oldSumTemps));
  */
  if ((abs(oldSumTemps - sumTemps) > 0.2) || (pumpSpeed != 0)) // If temperatures have significantly changed  OR destrat pump running, then update TFT display
  {
    drawCylinder(); // Update TFT Display
    displayTemps();
    oldSumTemps = sumTemps; // Reset old sum ready for next check for changes
  }
  //
  newMillis = millis();
  updatePercent = float(millis() - elapsedMillis) / recentEnergiesInterval; //Update record of % through graph update cycle for display on graphical indicator
  //
  //Draw Graph Update Cycle Indicator
  //
  for (float t = 0; t <= (2.0 * 3.14159); t = t + 0.0026)
  {
    if (t <= (2.0 * 3.14159 * updatePercent))
    {
      tft.drawLine(int(148 + (sin(t - 3.14159) * 7.0)), int(137 - (cos(t - 3.14159) * 7.0)), int(148 + (sin(t - 3.14159) * 12.0)), int(137 - (cos(t - 3.14159) * 12.0)), ILI9340_GREEN);
    }
  }
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if ((millis() - elapsedMillis) > recentEnergiesInterval || initialCount < 200) // START OF TIMED LOOP THAT UPDATES GRAPH
  {
    //
    //  Calculate smoothed energies
    //
    oldSmoothedEnergy = newSmoothedEnergy; // move last new energy value to 'old' variable
    newSmoothedEnergy = ((newSmoothedEnergy * (smoothNumber - 1)) + energy) / smoothNumber;
    //  ++++++++++++++++++++++++++++++++++++++++++++++++
    recentEnergies[recentEnergiesIndex] = energy;
    recentEnergiesIndex = (recentEnergiesIndex + 1) % nRecentEnergies;
    newAverageEnergy = oldAverageEnergy = 0.0f;
    for (unsigned i = recentEnergiesIndex; i < recentEnergiesIndex + (nRecentEnergies / 2); ++i)
    {
      oldAverageEnergy += recentEnergies[i % nRecentEnergies];
    }
    for (unsigned i = recentEnergiesIndex + (nRecentEnergies / 2); i < recentEnergiesIndex + nRecentEnergies; ++i)
    {
      newAverageEnergy += recentEnergies[i % nRecentEnergies];
    }
    newAverageEnergy /= (float)nRecentEnergies * 0.5f;
    oldAverageEnergy /= (float)nRecentEnergies * 0.5f;
    energyGradient = (newAverageEnergy - oldAverageEnergy) / deltaTime; // Line replaced by next line that uses smoothed energy instead
    //    energyGradient = (newSmoothedEnergy - oldSmoothedEnergy) / recentEnergiesInterval; // in kWh / ms
    arrowLength = int(((sqrt(abs(energyGradient) * 3600000) * arrowScaling) * 98) + 2);
    if (arrowLength > 100)
    {
      arrowLength = 100;
    }
    if (arrowLength < 2)
    {
      arrowLength = 2;
    }
    if (energyGradient > 0.0)
    {
      displayWarming();
    }
    else
    {
      displayCooling();
    }
    if (energyPercent > boilerRelaySetpoint) // Calculate Time to Full, if above Boiler Set Point - otherwise calc time to reaching boiler setpoint
    {
      hoursUntilFull = (maxEnergy - energy) / (energyGradient * 3600000.0f);
    }
    else
    {
      hoursUntilFull = ((maxEnergy * boilerRelaySetpoint / 100.0) - energy) / (energyGradient * 3600000.0f);
      if (hoursUntilFull < 0.0) // If cooling then change gradient to full range - so that time to empty may be calculated
      {
        hoursUntilFull = (maxEnergy - energy) / (energyGradient * 3600000.0f);
      }
    }
    //
    for (int i = 198; i > -1; i--) // Shift graph parameter arraya
    {
      graphEnergies[i + 1] = graphEnergies[i];
      graphTop[i + 1] = graphTop[i];
      graphMiddle[i + 1] = graphMiddle[i];
      graphBottom[i + 1] = graphBottom[i];
      graphEnd[i + 1] = graphEnd[i];
      graphCoil[i + 1] = graphCoil[i];
      graphStatus[i + 1] = graphStatus[i];
    }
    //    graphEnergies[0] = int(((energyPercent - lowestGraphPercent) * (100.0 / (100.0 - lowestGraphPercent))) * 1.20); // Store energy value scaled from lowestGraphPercent - 100% and mapped onto 120 pixels
    graphEnergies[0] = int(energyPercent); //Store current energy %
    if (graphEnergies[0] < 0) // Trap values < 0 and correct
    {
      graphEnergies[0] = 0;
    }
    if (graphEnergies[0] > 100) // Trap values > 100 and correct
    {
      graphEnergies[0] = 100;
    }
    graphTop[0] = int(((topTemp - 25.0) / 40.0) * 120.0) % 120;
    graphMiddle[0] = int(((middleTemp - 25.0) / 40.0) * 120.0) % 120;
    graphBottom[0] = int(((bottomTemp - 25.0) / 40.0) * 120.0) % 120;
    graphEnd[0] = int(((endTemp - 25.0) / 40.0) * 120.0) % 120;
    graphCoil[0] = int(((coilTemp - 25.0) / 40.0) * 120.0) % 120;
    //
    if (pumpOn)
    {
      graphStatus[0] = 1;
      pumpOn = false; // reset pump on flag
    }
    else
    {
      if (hotFlow)  // Hot water has been drawn - set status to 4
      {
        graphStatus[0] = 4;
      }
      else
      {
        if (boilerHeating) // If boiler heating set status to 2
        {
          graphStatus[0] = 2;
        }
        else
        {
          graphStatus[0] = 0;
        }
      }
      hotFlow = false;
      boilerHeating = false;
    }
    //
    // Change TFT energy scale to suit current energy value
    //
    if (energyPercent < energyGraphScaleSwitch)
    {
      lowestGraphPercent = 0;
    }
    else
    {
      lowestGraphPercent = normalLowestGraphPercent;
    }
    plotEnergy(); // Draw energy graph
    elapsedMillis = newMillis;
    initialCount++; // Increment initialCount counter that is used to quickly populate the graph arrays (by ignoring delay that is timed to graph time scale
    //
    tft.fillCircle(148, 137, 13, ILI9340_WHITE); // Fill to reset the update cycle graphic on each graph refresh
    tft.fillCircle(148, 137, 12, ILI9340_BLACK);
    tft.fillCircle(148, 137, 3, ILI9340_WHITE);
    previousEnergyPercent = energyPercent;
  } // END OF TIMED LOOP THAT UPDATES GRAPH
  //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (int(energyPercent) != int(previousEnergyPercent)) // RePlot graph if energy has changed - even before routine update cycle
  {
    plotEnergy();
    tft.fillCircle(148, 137, 13, ILI9340_WHITE); // Fill to reset the update cycle graphic on each graph refresh
    tft.fillCircle(148, 137, 12, ILI9340_BLACK);
    tft.fillCircle(148, 137, 3, ILI9340_WHITE);
    previousEnergyPercent = energyPercent;
  }
  displayPump(); // Display pump status on TFT
  displayBoiler(); // Display boiler status on TFT
  if (endTemp > (smoothedEndTemp * 1.005)) // Set hotFlow true if above temperature and not cooling
  {
    hotFlow = true; // Set flag to record hot water draw on next graph update
  }
  //  if ((coilTemp > middleTemp) && (energyGradient * 3600000000.0 > 500.0)) // If Coil is hot AND warming rate > X Watts then set boiler heating flag
  //  if ((coilTemp > middleTemp) && (energyGradient * 3600000000.0 > 1500.0)) // If Coil is hot AND warming > 500W then set boiler heating flag
  if ((coilTemp > topTemp) && (energyGradient * 3600000000.0 > 750.0)) // If Coil is hot AND warming > 750W then set boiler heating flag
  {
    boilerHeating = true; // Set flag to record boiler coil heating cylinder
  }
  //
  //  Set Pump Speed & Record Stats
  //
  if (topTemp > Setpoint)
  {
    if ((millis() - lastPumpOFF) > pumpMinOff)
    {
      if (energyPercent < 100.0 && energyGradient > 0.0) // Only allow pump to work if energy < 100% & cylinder warming - i.e. stop using the pump when the cylinder is full and let the thermostat trip instead
      {
        pumpSpeed = maxPumpSpeed;// Added to drive NEW PUMP via 12V relay - OR - SSR via 5V TTL output directly from the Arduino
        analogWrite(pumpPin, pumpSpeed);
        digitalWrite(pumpLEDPin, HIGH);
        if (!pumpCycleStarted) // Run once on each pump start
        {
          pumpONCount++; // Increment number of times pump has been ON
          lastPumpONTime = millis();
          currentPumpONTime = 0; // Start of pumping cycle, so reset timer
        }
        currentPumpONTime = currentPumpONTime + (millis() - lastPumpONTime);
        totalPumpONTime = totalPumpONTime + (millis() - lastPumpONTime);
        lastPumpONTime = millis();
        pumpOn = true; //Set flag to indicate that pump has been on
        pumpCycleStarted = true; // Flag to manage collection of pump statistics
      }
      else // Turn off pump & flash pump LED
      {
        pumpSpeed = 0;
        analogWrite(pumpPin, pumpSpeed);
        digitalWrite(pumpLEDPin, LOW);
        lastPumpOFF = millis(); //reset record of last time pump was turned OFF
        pumpCycleStarted = false;
        //
        digitalWrite(pumpLEDPin, HIGH); //Flash Pump LED to indicate pump is demanded by tempTop but inhibited by cylinder being full
        delay(300);
        digitalWrite(pumpLEDPin, LOW);
        delay(300);
        digitalWrite(pumpLEDPin, HIGH);
        delay(300);
        digitalWrite(pumpLEDPin, LOW);
        delay(300);
        digitalWrite(pumpLEDPin, HIGH);
        delay(300);
        digitalWrite(pumpLEDPin, LOW);
        delay(300);
      }
    }
  }
  else
  {
    pumpSpeed = 0;
    analogWrite(pumpPin, pumpSpeed);
    digitalWrite(pumpLEDPin, LOW);
    if (pumpCycleStarted) // Run this once on each pump stop
    {
      lastPumpOFF = millis(); //reset record of last time pump was turned OFF
      //      totalPumpONTime = totalPumpONTime + (millis() - lastPumpONTime);
    }
    //    lastPumpONTime = 0; // Reset timer
    pumpCycleStarted = false;
  }
} // END of main Loop
//
void graphGrid()
{
  tft.drawRect(120, 119, 200, 120, ILI9340_WHITE);
  tft.fillRect(121, 121, 198, 118, Chart);
}
void drawCylinder()
{
  for (int y = topPixel; y <= midPixel; y++) // Fill upper half of cylinder graphic
  {
    pixelTemp = topTemp - ((y - topPixel) * ((topTemp - middleTemp) / (midPixel - topPixel)));
    red = int(((pixelTemp - blueTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
    if (red < 0)
    {
      red = 0;
    }
    if (red > 31)
    {
      red = 31;
    }
    green = int((1.0 - (abs(pixelTemp - (((redTemp - blueTemp) / 2) + blueTemp)) / ((redTemp - blueTemp) / 2))) * 63);

    if (green < 0)
    {
      green = 0;
    }
    if (green > 63)
    {
      green = 63;
    }
    blue = int(((redTemp - pixelTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
    if (blue < 0)
    {
      blue = 0;
    }
    if (blue > 31)
    {
      blue = 31;
    }
    pixelColour = blue + green * 32 + red * 2048;
    //    tft.fillCircle(54,topPixel+24,34,pixelColour); // Fill 'dome' at top of cylinder graphic
    for (int x = 8; x <= 79; x++)
    {
      tft.drawPixel(x, y, pixelColour);
    }
  }
  for (int y = midPixel; y <= botPixel; y++) // Fill lower half of cylinder graphic
  {
    pixelTemp = middleTemp - ((y - midPixel) * ((middleTemp - bottomTemp) / (botPixel - midPixel)));
    red = int(((pixelTemp - blueTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
    if (red < 0)
    {
      red = 0;
    }
    if (red > 31)
    {
      red = 31;
    }
    green = int((1.0 - (abs(pixelTemp - (((redTemp - blueTemp) / 2) + blueTemp)) / ((redTemp - blueTemp) / 2))) * 63);
    if (green < 0)
    {
      green = 0;
    }
    if (green > 63)
    {
      green = 63;
    }
    blue = int(((redTemp - pixelTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
    if (blue < 0)
    {
      blue = 0;
    }
    if (blue > 31)
    {
      blue = 31;
    }
    pixelColour = blue + green * 32 + red * 2048;
    for (int x = 8; x <= 79; x++)
    {
      tft.drawPixel(x, y, pixelColour);
    }
  }
  for (int i = 0; i < 8; i++) // Round off corners of colour coded temperature fill
  {
    tft.drawRoundRect(i, i, (88 - 2 * i), (240 - 2 * i), (20 - i), ILI9340_YELLOW);
  }
}
void displayTemps()
{
  tft.setTextSize (3);
  if (topTemp >= 29.0)
  {
    tft.setTextColor(ILI9340_BLACK);
  }
  else
  {
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setCursor(24, 40);
  tft.print(int(topTemp));
  tft.fillRect(65, 40, 6, 20, ILI9340_YELLOW); //Draw columns next to numerals to indicate fractions of degrees
  tft.drawRect(65, 40, 6, 20, ILI9340_BLACK);
  tft.fillRect(66, 41, 4, int((1 - (topTemp - int(topTemp))) * 18), ILI9340_BLACK);
  //
  if (middleTemp >= 29.0)
  {
    tft.setTextColor(ILI9340_BLACK);
  }
  else
  {
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setCursor(24, 110);
  tft.print(int(middleTemp));
  tft.fillRect(65, 110, 6, 20, ILI9340_YELLOW);
  tft.drawRect(65, 110, 6, 20, ILI9340_BLACK);
  tft.fillRect(66, 111, 4, int((1 - (middleTemp - int(middleTemp))) * 18), ILI9340_BLACK);
  //
  //  if (bottomTemp >= 29.0) // Reduced to 24 degrees based on recent experience (23/4/2020)
  if (bottomTemp >= 24.0)
  {
    tft.setTextColor(ILI9340_BLACK);
  }
  else
  {
    tft.setTextColor(ILI9340_WHITE);
  }
  tft.setCursor(24, 180);
  tft.print(int(bottomTemp));
  tft.fillRect(65, 180, 6, 20, ILI9340_YELLOW);
  tft.drawRect(65, 180, 6, 20, ILI9340_BLACK);
  tft.fillRect(66, 181, 4, int((1 - (bottomTemp - int(bottomTemp))) * 18), ILI9340_BLACK);
  //
  //  Display Hot Water Outlet Temp
  //
  pixelTemp = endTemp;
  red = int(((pixelTemp - blueTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
  if (red < 0)
  {
    red = 0;
  }
  if (red > 31)
  {
    red = 31;
  }
  green = int((1.0 - (abs(pixelTemp - (((redTemp - blueTemp) / 2) + blueTemp)) / ((redTemp - blueTemp) / 2))) * 63);

  if (green < 0)
  {
    green = 0;
  }
  if (green > 63)
  {
    green = 63;
  }
  blue = int(((redTemp - pixelTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
  if (blue < 0)
  {
    blue = 0;
  }
  if (blue > 31)
  {
    blue = 31;
  }
  pixelColour = blue + green * 32 + red * 2048;
  tft.setTextSize(2);
  tft.fillTriangle(0, 0, 41, 0, 0, 41, pixelColour);
  if (endTemp < 30)
  {
    tft.setTextColor(ILI9340_WHITE);
  }
  else
  {
    tft.setTextColor(ILI9340_BLACK);
  }
  tft.setCursor(2, 2);
  //  tft.print("82");
  tft.print(endTemp, 0);
  //
  if (hotFlow) // Draw red spot if hot water is being drawn
  {
    tft.fillCircle(13, 25, 6, ILI9340_RED);
    tft.drawCircle(13, 25, 6, ILI9340_WHITE);
  }
  //++++++++++++++++++++++++++++
  //
  //  Display Boiler Coil Temp
  //
  pixelTemp = coilTemp;
  red = int(((pixelTemp - blueTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
  if (red < 0)
  {
    red = 0;
  }
  if (red > 31)
  {
    red = 31;
  }
  green = int((1.0 - (abs(pixelTemp - (((redTemp - blueTemp) / 2) + blueTemp)) / ((redTemp - blueTemp) / 2))) * 63);

  if (green < 0)
  {
    green = 0;
  }
  if (green > 63)
  {
    green = 63;
  }
  blue = int(((redTemp - pixelTemp) / ((redTemp - blueTemp) / 2)) * 31.0);
  if (blue < 0)
  {
    blue = 0;
  }
  if (blue > 31)
  {
    blue = 31;
  }
  pixelColour = blue + green * 32 + red * 2048;
  for (int i = 6; i < 75; i = i + 13)
  {
    tft.fillCircle(i, 216, 5, pixelColour);
  }
  tft.setTextSize(2);
  tft.fillTriangle(0, 240, 41, 240, 0, 199, pixelColour);
  if (coilTemp < 30)
  {
    tft.setTextColor(ILI9340_WHITE);
  }
  else
  {
    tft.setTextColor(ILI9340_BLACK);
  }
  tft.setCursor(2, 222);
  tft.print(coilTemp, 0);
  //
  if (boilerHeating) // Draw red spot if boiler coil is heating cylinder
  {
    tft.fillCircle(13, 210, 6, ILI9340_RED);
    tft.drawCircle(13, 210, 6, ILI9340_WHITE);
  }
}
void plotEnergy()
{
  for (int i = 0; i < 199; i++)
  {
    tft.drawLine(319 - i, 121, 319 - i, 239, Chart); // Clear whole column by drawing background colour
    if (i % 24 == 0) // Draw vertical Grid Lines every 15 mins
    {
      tft.drawLine(319 - i, 121, 319 - i, 239, t5);
    }
    if (i % 96 == 0 && i > 0) // Draw vertical Grid Lines every hour (but not at hour 'zero')
    {
      tft.drawLine(319 - i, 121, 319 - i, 239, t7);
    }
    else
    {
      if (graphEnergies[i] >= int(boilerRelaySetpoint)) // Draw green if energy% at, or over, boilerRelaySetpoint
      {
        tft.drawLine(319 - i, 121 + ((100 - graphEnergies[i]) * (120.0 / (100 - lowestGraphPercent))), 319 - i, 239, ILI9340_GREEN);
      }
      else
      {
        if (graphEnergies[i] >= redEnergyPercent) // Draw Yellow if energy > Red level
        {
          tft.drawLine(319 - i, 121 + ((100 - graphEnergies[i]) * (120.0 / (100 - lowestGraphPercent))), 319 - i, 239, ILI9340_YELLOW);
        }
        else
        {
          tft.drawLine(319 - i, 121 + ((100 - graphEnergies[i]) * (120.0 / (100 - lowestGraphPercent))), 319 - i, 239, t6);
        }
      }
      //
      // Plot Three Temperatures (3 pixels thick)
      //
      tft.drawPixel(319 - i, 239 - graphTop[i], ILI9340_RED);
      tft.drawPixel(319 - i, 239 - graphMiddle[i], t4);
      tft.drawPixel(319 - i, 239 - graphBottom[i], t1);
      //
      tft.drawPixel(319 - i, 240 - graphTop[i], ILI9340_RED);
      tft.drawPixel(319 - i, 240 - graphMiddle[i], t4);
      tft.drawPixel(319 - i, 240 - graphBottom[i], t1);
      //
      tft.drawPixel(319 - i, 241 - graphTop[i], ILI9340_RED);
      tft.drawPixel(319 - i, 241 - graphMiddle[i], t4);
      tft.drawPixel(319 - i, 241 - graphBottom[i], t1);
    }
    //
    // Display Status colours - clear column first
    //
    tft.drawLine(319 - i, 113, 319 - i, 118, ILI9340_BLACK); // If nothing ON draw white / black
    tft.drawPixel(319 - i, 119, ILI9340_WHITE); // fill in missing graph border pixel
    if (graphStatus[i] == 2) // If boiler heating cylinder, draw pixel
    {
//      tft.drawLine(319 - i, 113, 319 - i, 118, t5);
      tft.drawLine(319 - i, 113, 319 - i, 118, ILI9340_GREEN);
    }
    if (graphStatus[i] == 1) // If pump ON, draw striped line
    {
      tft.drawLine(319 - i, 113, 319 - i, 114, ILI9340_WHITE);
      tft.drawLine(319 - i, 115, 319 - i, 115, ILI9340_BLUE);
      tft.drawLine(319 - i, 116, 319 - i, 116, ILI9340_BLUE);
      tft.drawLine(319 - i, 117, 319 - i, 117, ILI9340_BLUE);
      tft.drawLine(319 - i, 118, 319 - i, 118, ILI9340_WHITE);
    }
    if (graphStatus[i] == 4) // Indicate when hot water is being drawn (remember that the graph variables here are in pixels, not degrees C)
    {
      tft.drawLine(319 - i, 113, 319 - i, 118, ILI9340_RED);
    }
  }
  //
  if (graphEnergies[75] > boilerRelaySetpoint) // Display Energy Level numerically on graph; use value 75 pixels from RHS to determine where and what colour to use for the text
  {
    tft.setTextSize(3);
    tft.setCursor(225, 210);
    tft.setTextColor(ILI9340_BLACK);
    tft.print(int(energyPercent));
    tft.print("%");
  }
  else
  {
    tft.setTextSize(3);
    tft.setCursor(240, 125);
    tft.setTextColor(ILI9340_WHITE);
    tft.print(int(energyPercent));
    tft.print("%");
  }
  //
  // Draw horizontal grid and temperature labels
  //
  tft.setTextSize(1);
  tft.setTextColor(ILI9340_WHITE);
  tft.setCursor(103, 131);
  tft.print("60");
  tft.drawLine(116, 135, 320, 135, t5);
  tft.setCursor(103, 161);
  tft.print("50");
  tft.drawLine(116, 165, 320, 165, t5);
  tft.setCursor(103, 191);
  tft.print("40");
  tft.drawLine(116, 195, 320, 195, t5);
  tft.setCursor(103, 221);
  tft.print("30");
  tft.drawLine(116, 225, 320, 225, t5);
  //
  // Draw Hour labels
  //
  tft.setTextColor(ILI9340_WHITE, t7);
  tft.setRotation(0);
  tft.setCursor(155, 93);
  tft.print("-4 hours");
  tft.setCursor(155, 188);
  tft.print("-8 hours");
  tft.setRotation(3);
  //
  //  Draw % Labels
  //
  tft.setTextSize(1);
  tft.setTextColor(ILI9340_WHITE);
  for (int j = 80; j > lowestGraphPercent; j = j - 20)
  {
    tft.setCursor(300, (117 + int((100 - j) * (120.0 / (100 - lowestGraphPercent))))); // Set cursor for % text at 3 pixels above actual row calculated = top left of 7 pixel high characters
    if (j < graphEnergies[0])
    {
      tft.setTextColor(ILI9340_BLACK);
    }
    tft.print(j);
    tft.print("%");
  }
}
void displayPump()
{
  //  if (abs(currentPumpONTime - previousCurrentPumpONTime > 0)) // Only refresh display if something has changed
  //{
  // Write Pump Status
  tft.fillRect(123, 2, 96, 100, ILI9340_BLACK);// Top left coord - then pixels wide and deep
  tft.setTextSize(2);
  tft.setCursor(126, 10);
  tft.setTextColor(ILI9340_WHITE);
  tft.print("TOT ");
  tft.print(totalPumpONTime / 60000);
  tft.print("m");
  //
  tft.setCursor(126, 45);
  tft.setTextColor(ILI9340_YELLOW);

  if (pumpCycleStarted)
  {
    tft.print("Now ");
    if (currentPumpONTime < 99000)
    {
      tft.print(currentPumpONTime / 1000);
      tft.print("s");
    }
    else
    {
      tft.print(currentPumpONTime / 60000);
      tft.print("m");
    }
    previousCurrentPumpONTime = currentPumpONTime;
  }
  else
  {
    tft.print("Lst ");
    if (previousCurrentPumpONTime < 99000)
    {
      tft.print(previousCurrentPumpONTime / 1000);
      tft.print("s");
    }
    else
    {
      tft.print(previousCurrentPumpONTime / 60000);
      tft.print("m");
    }
  }
  //
  tft.setCursor(126, 80);
  tft.setTextColor(ILI9340_WHITE);
  tft.print("  # ");
  tft.print(pumpONCount);
}
void displayBoiler()
{
  // Draw main dial
  //
  if (abs(previousBoilerPercent - boilerPercent) > 1.0) // Only redraw dial if % has changed
  {
    for (float t = 0; t <= (2.0 * 3.14159); t = t + 0.01)
    {
      if (t <= (2.0 * 0.0314159 * boilerPercent))
      {
        tft.drawLine(int(272 + (sin(t - 3.14159) * 23.0)), int(49 - (cos(t - 3.14159) * 23.0)), int(272 + (sin(t - 3.14159) * 42.0)), int(49 - (cos(t - 3.14159) * 42.0)), ILI9340_RED);
      }
      else
      {
        tft.drawLine(int(272 + (sin(t - 3.14159) * 23.0)), int(49 - (cos(t - 3.14159) * 23.0)), int(272 + (sin(t - 3.14159) * 42.0)), int(49 - (cos(t - 3.14159) * 42.0)), ILI9340_BLACK);
      }
      if ((int((t / (2.0 * 3.14169)) * 100) % 10) == 0) // Draw Grid Lines
      {
        tft.drawLine(int(272 + (sin(t - 3.14159) * 23.0)), int(49 - (cos(t - 3.14159) * 23.0)), int(272 + (sin(t - 3.14159) * 42.0)), int(49 - (cos(t - 3.14159) * 42.0)), t1);
      }
    }
    tft.setTextSize(1);
    tft.setCursor(242, 103);
    tft.setTextColor(ILI9340_WHITE);
    tft.fillRect(241, 102, 70, 9, BkGnd); // Wipe previous text
    tft.setCursor(242, 102);
    tft.print("Boiler ");
    tft.print(int(boilerPercent));
    tft.print("%");
    tft.fillCircle(272, 49, 14, t2);
    previousBoilerPercent = boilerPercent; // Update previous boilerPercent record
  }
  //
  // Draw boiler relay window status
  //
  for (float t = 0; t <= (2.0 * 3.14159); t = t + 0.01)
  {
    if (t <= (2.0 * 0.0314159 * windowPointer))
    {
      tft.drawLine(int(272 + (sin(t - 3.14159) * 15.0)), int(49 - (cos(t - 3.14159) * 15.0)), int(272 + (sin(t - 3.14159) * 22.0)), int(49 - (cos(t - 3.14159) * 22.0)), ILI9340_GREEN);
      //      tft.drawLine(272, 49, int(272 + (sin(t - 3.14159) * 22.0)), int(49 - (cos(t - 3.14159) * 22.0)), ILI9340_GREEN);
    }
    else
    {
      tft.drawLine(int(272 + (sin(t - 3.14159) * 15.0)), int(49 - (cos(t - 3.14159) * 15.0)), int(272 + (sin(t - 3.14159) * 22.0)), int(49 - (cos(t - 3.14159) * 22.0)), ILI9340_BLACK);
    }
    if ((int((t / (2.0 * 3.14169)) * 100) % 10) == 0) // Draw Grid Lines
    {
      tft.drawLine(int(272 + (sin(t - 3.14159) * 15.0)), int(49 - (cos(t - 3.14159) * 15.0)), int(272 + (sin(t - 3.14159) * 22.0)), int(49 - (cos(t - 3.14159) * 22.0)), t2);
    }
  }
  tft.fillCircle(272, 49, 14, t2);
  if (boilerRelayOn)
  {
    tft.fillCircle(272, 49, 10, ILI9340_YELLOW);
  }
  else
  {
    tft.fillCircle(272, 49, 10, ILI9340_RED);
  }

}
//
void displayWarming()  // Draw upward arrow scaled to reflect rate of heating
{
  tft.fillRect(94, 0, 28, 115, BkGnd);
  tft.fillTriangle(96, 100, 121, 100, 108, (100 - arrowLength), ILI9340_RED);
  for (int i = 101; i > (100 - (arrowLength - 1)); i--) // Draw scale next to arrow
  {
    tft.drawPixel(95, i, t4);
    if (i % 20 == 0)
    {
      tft.drawPixel(96, i, t4);
      tft.drawPixel(97, i, t4);
      tft.drawPixel(98, i, t4);
      tft.drawPixel(99, i, t4);
    }
  }
  tft.setRotation(0);
  tft.setTextSize(1);
  tft.setTextColor(ILI9340_WHITE);
  if (arrowLength < 60)
  {
    tft.setCursor(((100 - arrowLength) - 36), 207);
  }
  else
  {
    tft.setCursor(62, 207);
  }

  if (abs(energyGradient * 3600000.0) > 0.9999999)
  {
    tft.print(abs(energyGradient * 3600000.0));
    tft.print("kW");
  }
  else
  {
    tft.print(int(abs(energyGradient * 3600000000.0)));
    tft.print("W");
  }
  tft.setRotation(3);
}
void displayCooling()  // Draw downward arrow scaled to reflect rate of cooling
{
  tft.fillRect(94, 0, 28, 115, BkGnd);
  tft.fillTriangle(96, 1, 121, 1, 108, arrowLength, ILI9340_BLUE);
  for (int i = 1; i < arrowLength + 1; i++) // Draw scale next to arrow
  {
    tft.drawPixel(95, i, t4);
    if (i % 20 == 0)
    {
      tft.drawPixel(96, i, t4);
      tft.drawPixel(97, i, t4);
      tft.drawPixel(98, i, t4);
      tft.drawPixel(99, i, t4);
    }
  }
  tft.setRotation(0);
  tft.setTextSize(1);
  tft.setTextColor(ILI9340_WHITE);
  if (arrowLength < 70)
  {
    tft.setCursor((arrowLength + 7), 208);
  }
  else
  {
    tft.setCursor(4, 207);
  }
  if (abs(energyGradient * 3600000.0) > 0.9999999)
  {
    tft.print(abs(energyGradient * 3600000.0)); // display kW
    tft.print("kW");
  }
  else
  {
    tft.print(int(abs(energyGradient * 3600000000.0))); // display W
    tft.print("W");
  }
  tft.setRotation(3);
}
