#include <Wire.h>

// variables

// CONTROL ENABLE / DISABLE
// primary controls
bool useThrottle = true;
bool useDynamics = true;
bool useReverser = true;
bool useAutomaticBrake = true;
bool useIndependentBrake = true;
bool useBailOff = true;

// buttons and stuff
bool useHorn = true;
bool useAlerter = true;
bool useSander = true;
bool useBell = true;

// lights
bool useFrontHeadlight = true;
bool useRearHeadlight = true;

// switch panels
bool useGenField = true;
bool useControlFuel = true;
bool useEngineRun = true;
bool useGaugeLights = true;
bool useStepLights = true;
bool useDitchLights = true;

// indicators
bool useIndicatorPanel = true;

// I2C Flag (disable reverser, headlight controls, and indiactor light flags when using this)
// ONLY FOR USE WITH TWO ARDUINO BOARDS
// USE A MEGA TO AVOID THIS
bool use_I2C = false;


// PIN DEFINITIONS
// all unused controls (either not connected or used over I2C), set the value to 13, as that will not interfere with anything
// potentiometer analog pin definitions
const int autoBrake = A0;
const int indyBrake = A1;
const int throttle = A2;
const int dyno = A3;
const int hdlt_front = A4;
const int hdlt_rear = A5;

// button digital pin definitons
const int horn = 2;
const int bell = 3;
const int alerter = 4;
const int sand = 5;
const int bail_off = 6;
const int gaugeLight = 7;
const int stepLight = 8;
const int ditchLight = 9;
const int engineRun = 10;
const int fuelPump = 11;
const int genField = 12;
const int reverserForward = 26; // microswitch
const int reverserReverse = 27;

// indicator light defintions (unassigned currently, you will need to change these)
const int SAND = 22;
const int WHEEL_SLIP = 23;
const int BRAKE_WARN = 24;
const int PCS_OPEN = 25;

// CALIBRATION VALUES
// YOU WILL NEED TO FIND THESE USING THE SERIAL MONITOR
// throttle (0, 1, 2, 3, 4, 5, 6, 7, 8)
const int notchCenters[9] = {629, 599, 560, 521, 483, 443, 407, 369, 341};

// front light dial positions (off, dim, med, brt)
const int frontLightPositions[4] = {880, 710, 545, 370};

// rear light dial positions (off, dim, med, brt)
const int rearLightPositions[4] = {850, 680, 500, 330};

// dynamic brake
const int dynoMin = 360; // where the "SET UP" position is located
const int dynoMax = 687; // where the 8th position is located

// automatic brake
const int releaseMinimumThreshold = 272; 
const int fullService = 500;
const int fullServiceHandleOffThreshold= 570;
const int handleOffEmergencyThreshold= 635;

// independent brake
const int indyMin = 455;
const int indyMax = 690;

// below this point does not need to be of a concern to you unless you are rewriting the code tbf

// I2C DEFINITIONS DO NOT TOUCH
struct inputs {
  int F_HeadlightState;
  int R_HeadlightState;
  int genFieldState;
  int engineRunState;
  int controlPumpState;
  int stepLightState;
  int gaugeLightState;
  int ditchLightState;
};

struct indicators {
  int sand_indicator;
  int slip_indicator;
  int pcs_indicator;
  int brake_indicator;
};

// VARIABLES
// below this point are variables used within code and should not be modified
int sandState = 0;
int bellStateUDP = 0;
int hornStateUDP = 0;
int sandStateUDP = 0;
int automaticBrakePositionUDP = 0;
int sandToggleLogic = 0;
int autoValue = 0;
int mappedValue = 0;
int throttlePositionUDP = 0;
int throttlePosition = 0;  // Potentiometer value
int throttleNotch = 0;     // Calculated throttle notch (0-8)
int independentBrakePositionUDP = 0;
int independentBrakeBailUDP = 0;
int indyValue = 0;
int indyMappedValue = 0;
int dynoMappedValue = 0;
int reverserStateUDP = 0;
int dynamicBrakePositionUDP = 0;
int alerterPressedUDP = 0;
int genFieldUDP = 0;
int conFuelPumpUDP = 0;
int engRunUDP = 0;
int gaugeLightUDP = 0;
int stepLightUDP = 0;
int ditchLightUDP = 0;
int frontLightPosition = 0;
int rearLightPosition = 0;

// no clue what these do
const byte numChars = 32;
char receivedChars[numChars];   // an array to store the received data

bool newData = false;

// Track previous throttle position to determine direction
int previousThrottlePosition = 0;
bool increasingThrottle = true;

unsigned long autoHighStartTime = 0;
bool autoHighTimerRunning = false;
const unsigned long holdDuration = 500; // 500 ms
int autoMappedValue = 0;

int PB = 0;
int udpState;
int SSt = 0;
int WS = 0;
int PC = 0;

int headlight_recieved = 0;
int rearlight_recieved = 0;
int reverser_recieved = 0;
int genField_recieved = 0;
int engineRun_recieved = 0;
int controlPump_recieved = 0;
int stepLight_recieved = 0;
int gaugeLight_recieved = 0;
int ditchLight_recieved = 0;

void setup() {
  // Pin config
  pinMode(bell, INPUT_PULLUP);
  pinMode(horn, INPUT_PULLUP);
  pinMode(sand, INPUT_PULLUP);
  pinMode(alerter, INPUT_PULLUP);
  pinMode(bail_off, INPUT_PULLUP);
  pinMode(autoBrake, INPUT);
  pinMode(indyBrake, INPUT);
  pinMode(throttle, INPUT);
  pinMode(dyno, INPUT);
  pinMode(hdlt_front, INPUT);
  pinMode(hdlt_rear, INPUT);
  pinMode(gaugeLight, INPUT_PULLUP);
  pinMode(stepLight, INPUT_PULLUP);
  pinMode(ditchLight, INPUT_PULLUP);
  pinMode(genField, INPUT_PULLUP);
  pinMode(fuelPump, INPUT_PULLUP);
  pinMode(engineRun, INPUT_PULLUP);
  pinMode(reverserForward, INPUT_PULLUP);
  pinMode(reverserReverse, INPUT_PULLUP);
  pinMode(PCS_OPEN, OUTPUT);
  pinMode(SAND, OUTPUT);
  pinMode(WHEEL_SLIP, OUTPUT);
  pinMode(BRAKE_WARN, OUTPUT);
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // if the control is enabled, the function to determine the control state is run and printed to the serial
  // controls are enabled and disabled at the very top of the code in the boolean definitions
  // this code does NOT need to be modified to work (i think)


  if (useThrottle == true){int throttle_return = currentThrottle(); Serial.print(",TH:"); Serial.print(throttle_return);}
  if (useDynamics == true){int dynamic_return = currentDynamic(); Serial.print(",DB:"); Serial.print(dynamic_return);}
  if (useReverser == true){int reverser_return = currentReverser(); Serial.print(",RS:"); Serial.print(reverser_return);}
  if (useAutomaticBrake == true){int automaticBrakes_return = currentAutomatic(); Serial.print(",AB:"); Serial.print(automaticBrakes_return);}
  if (useIndependentBrake == true){int independentBrake_return = currentIndependent(); Serial.print(",IB:"); Serial.print(independentBrake_return);}
  if (useBailOff == true){int bailOff_return = currentBailOff(); Serial.print(",IL:"); Serial.print(bailOff_return);}
  if (useHorn == true){int horn_return = hornLogic(); Serial.print(",HR:"); Serial.print(horn_return);}
  if (useBell == true){int bell_return = bellLogic(); Serial.print(",BL:"); Serial.print(bell_return);}
  if (useAlerter == true){int alerter_return = alerterLogic(); Serial.print(",AL:"); Serial.print(alerter_return);}
  if (useSander == true){int sand_return = sandLogic(); Serial.print(",SD:"); Serial.print(sand_return);}
  if (use_I2C == true){indicatorLightsI2C(); requestData();}
  if (useGenField == true){int genField_return = genFieldLogic(); Serial.print(",GF:"); Serial.print(genField_return);}
  if (useControlFuel == true){int controlFuel_return = controlFuelLogic(); Serial.print(",FP:"); Serial.print(controlFuel_return);}
  if (useEngineRun == true){int engineRun_return = engineRunLogic(); Serial.print(",ER:"); Serial.print(engineRun_return);}
  if (useGaugeLights == true){int gaugeLights_return = gaugeLightsLogic(); Serial.print(",GL:"); Serial.print(gaugeLights_return);}
  if (useStepLights == true){int stepLights_return = stepLightLogic(); Serial.print(",SL:"); Serial.print(stepLights_return);}
  if (useDitchLights == true){int ditchLight_return = ditchLightLogic(); Serial.print(",DL:"); Serial.print(ditchLight_return);}
  if (useFrontHeadlight == true){int frontHeadlight_return = frontHeadlightState(); Serial.print(",FL:"); Serial.print(frontHeadlight_return);}
  if (useRearHeadlight == true){int rearHeadLight_return = rearHeadlightState(); Serial.print(",RL:"); Serial.print(rearHeadLight_return);}
  if (useIndicatorPanel == true){indicatorLights();}
  Serial.println();


  // also if you want a debug line for calibration use these two lines by deleting the '//', changing to the pin you want to check
  // and then ensure to re add the '//' when done (things will break otherwise)

  // Serial.println(digitalRead(0));
  // Serial.println(analogRead(A0));

}

// these are the individual functions that run the logic for the controls
// these do not need to be modified, all configurations can be found at the top of the code file
// if something goes terribly wrong is is straight up missing, find a way to tell me (the author) please :(

void testControlAnalog(int pin){
  int controlPosition = analogRead(pin);
  Serial.println(controlPosition);
}

int currentThrottle(){
  // Determine which notch the throttle is in
  throttlePosition = analogRead(throttle); 
  // Serial.println(analogRead(dyno));

    int minDifference = abs(throttlePosition - notchCenters[0]);
    throttleNotch = 0;

    for (int i = 1; i < 9; i++) {
        int difference = abs(throttlePosition - notchCenters[i]);
        if (difference < minDifference) {
            minDifference = difference;
            throttleNotch = i;
        }
    }

    // Set throttlePositionUDP based on notch
    // wait this might be entirely unnecesary
    // aint gonna remove it till I test it tho
    switch (throttleNotch) {
        case 0:
            throttlePositionUDP = 0;
            break;
        case 1:
            throttlePositionUDP = 1;
            break;
        case 2:
            throttlePositionUDP = 2;
            break;
        case 3:
            throttlePositionUDP = 3;
            break;
        case 4:
            throttlePositionUDP = 4;
            break;
        case 5:
            throttlePositionUDP = 5;
            break;
        case 6:
            throttlePositionUDP = 6;
            break;
        case 7:
            throttlePositionUDP = 7;
            break;
        case 8:
            throttlePositionUDP = 8;
            break;
    }

    return throttlePositionUDP;
}

int currentDynamic(){
  int dynoValue = analogRead(dyno);
  dynoMappedValue = map(dynoValue, dynoMin, dynoMax, 0, 255);
  dynamicBrakePositionUDP = constrain(dynoMappedValue, 0, 255);
  return dynamicBrakePositionUDP;
}

int currentReverser(){
  if (digitalRead(reverserForward) == HIGH){
  reverserStateUDP = 255;
  } else if (digitalRead(reverserReverse) == HIGH){
    reverserStateUDP = 0;
  } else {
    reverserStateUDP = 128;
  }

  return reverserStateUDP;
}

int currentAutomatic(){
  autoValue = analogRead(autoBrake);  // Read the potentiometer value (0 to 1023)
  int autoMappedValue;
  if (autoValue >= handleOffEmergencyThreshold) {
    if (!autoHighTimerRunning) {
      autoHighStartTime = millis();
      autoHighTimerRunning = true;
    }

    if (millis() - autoHighStartTime >= holdDuration) {
      autoMappedValue = 255;
    } else {
      autoMappedValue = 254; 
    }
  } else {
    autoHighTimerRunning = false; // reset timer if condition breaks

    if (autoValue >= fullServiceHandleOffThreshold && autoValue < handleOffEmergencyThreshold) {
      autoMappedValue = 254;
    } else if (autoValue > fullService && autoValue < fullServiceHandleOffThreshold) {
      autoMappedValue = 253;
    } else if (autoValue >= releaseMinimumThreshold && autoValue <= fullService) {
      autoMappedValue = map(autoValue, releaseMinimumThreshold, fullService, 0, 253);
    } else {
      autoMappedValue = 0;
    }
  }

  automaticBrakePositionUDP = constrain(autoMappedValue, 0, 255);

  return automaticBrakePositionUDP;
}

int currentIndependent(){
  indyValue = analogRead(indyBrake);
  indyMappedValue = map(indyValue, indyMin, indyMax, 0, 255);
  //Serial.println(indyValue);
  independentBrakePositionUDP = constrain(indyMappedValue, 0, 255);
  return independentBrakePositionUDP;
}

int currentBailOff(){
  if (digitalRead(bail_off) == LOW){
    independentBrakeBailUDP = 1;
  } else {
    independentBrakeBailUDP = 0;
  }

  return independentBrakeBailUDP;
}

int hornLogic(){
  int hornState = digitalRead(horn);
  if (hornState == LOW) {
    hornStateUDP = 1;
  } else {
    hornStateUDP = 0;
  }

  return hornStateUDP;
}

int bellLogic(){
  int bellState = digitalRead(bell);
  if (bellState == LOW) {
    bellStateUDP = 1;
  } else {
    bellStateUDP = 0;
  }

  return bellStateUDP;
}

int alerterLogic(){
  if (digitalRead(alerter) == LOW){
    alerterPressedUDP = 1;
  } else {
    alerterPressedUDP = 0;
  }

  return alerterPressedUDP;
}

int sandLogic(){
  if (digitalRead(sand) == LOW){
    sandStateUDP = 1;
  } else {
    sandStateUDP = 0;
  } 

  return sandStateUDP;
}

int genFieldLogic(){
  genFieldUDP = digitalRead(genField);
  return !genFieldUDP;
}

int engineRunLogic(){
  engRunUDP = digitalRead(engineRun);
  return !engRunUDP;
}

int controlFuelLogic(){
  conFuelPumpUDP = digitalRead(fuelPump);
  return !conFuelPumpUDP;
}

int gaugeLightsLogic(){
  gaugeLightUDP = digitalRead(gaugeLight);
  return !gaugeLightUDP;
}

int stepLightLogic(){
  stepLightUDP = digitalRead(stepLight);
  return !stepLightUDP;
}

int ditchLightLogic(){
  ditchLightUDP = digitalRead(ditchLight);
  return !ditchLightUDP;
}

int frontHeadlightState(){
  int frontHeadlightValue = analogRead(hdlt_front);

    int minDifference2 = abs(frontHeadlightValue - frontLightPositions[0]);
    frontLightPosition = 0;

    for (int i = 1; i < 4; i++) {
        int difference2 = abs(frontHeadlightValue - frontLightPositions[i]);
        if (difference2 < minDifference2) {
            minDifference2 = difference2;
            frontLightPosition = i;
        }
    }
    return frontLightPosition;
}

int rearHeadlightState(){
  int rearHeadlightValue = analogRead(hdlt_rear);

    int minDifference3 = abs(rearHeadlightValue - rearLightPositions[0]);
    rearLightPosition = 0;

    for (int i = 1; i < 4; i++) {
        int difference3 = abs(rearHeadlightValue - rearLightPositions[i]);
        if (difference3 < minDifference3) {
            minDifference3 = difference3;
            rearLightPosition = i;
        }
    }

    return rearLightPosition;
}

void indicatorLights() {
  digitalWrite(SAND, LOW);
  digitalWrite(WHEEL_SLIP, LOW);
  digitalWrite(PCS_OPEN, LOW);
  digitalWrite(BRAKE_WARN, LOW);
  /*
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("PB:")) {
      SSt = line.substring(18).toInt();
      PC = line.substring(13).toInt();
      WS = line.substring(8).toInt();
      PB = line.substring(3).toInt();
    }
  }

  if(WS == 1){
        digitalWrite(2, HIGH);
      } else {
        digitalWrite(2, LOW);
      }

  if(PC == 1){
        digitalWrite(3, HIGH);
      } else {
        digitalWrite(3, LOW);
      }
    if(PB == 1){
        digitalWrite(4, HIGH);
      } else {
        digitalWrite(4, LOW);
      }

    if(SSt == 1){
        digitalWrite(5, HIGH);
      } else {
        digitalWrite(5, LOW);
      }
      */
}

void indicatorLightsI2C(){
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    Serial.println(line);

    if (line.startsWith("PB:")) {
      SSt = line.substring(18).toInt();
      PC = line.substring(13).toInt();
      WS = line.substring(8).toInt();
      PB = line.substring(3).toInt();
    }


    indicators indicatorLights = {};
    indicatorLights.sand_indicator = SSt;
    indicatorLights.slip_indicator = WS;
    indicatorLights.pcs_indicator = PC;
    indicatorLights.brake_indicator = PB;

    Wire.write((byte *)&indicatorLights, sizeof(indicators));
  }
}

void requestData(){
  Wire.requestFrom(8, 16);    // request 6 bytes from peripheral device #8
  while(Wire.available()){   
    inputs my_stuct_2 = {};
    Wire.readBytes((byte*)&my_stuct_2, sizeof(inputs));
    headlight_recieved = my_stuct_2.F_HeadlightState;
    rearlight_recieved = my_stuct_2.R_HeadlightState;
    genField_recieved = my_stuct_2.genFieldState;
    engineRun_recieved = my_stuct_2.engineRunState;
    controlPump_recieved = my_stuct_2.controlPumpState;
    stepLight_recieved = my_stuct_2.stepLightState;
    gaugeLight_recieved = my_stuct_2.gaugeLightState;
    ditchLight_recieved = my_stuct_2.ditchLightState;
  }
  Serial.print(",HL:");
  Serial.print(headlight_recieved);
  Serial.print(",RL:");
  Serial.print(rearlight_recieved);
  Serial.print(",GF:");
  Serial.print(genField_recieved);
  Serial.print(",ER:");
  Serial.print(engineRun_recieved);
  Serial.print(",FP:");
  Serial.print(controlPump_recieved);
  Serial.print(",SL:");
  Serial.print(stepLight_recieved);
  Serial.print(",GL:");
  Serial.print(gaugeLight_recieved);
  Serial.print(",DL:");
  Serial.print(ditchLight_recieved);
}

