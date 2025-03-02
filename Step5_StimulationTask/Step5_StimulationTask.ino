/* Main Stimulation Task v1.0

   This sketch runs the main stimulation task.

*/

#include <SPI.h>  // Include the SPI library
#include <math.h> // Include the math library

// ----------------------------------------------------------------------------------------- //
// ----- Stimulation properties ------------------------------------------------------------ //
// ----------------------------------------------------------------------------------------- //
int nElectrodes = 20;     // Number of stimulation electrodes
float pulseWidth = 0.200; // Pulse width in milliseconds (PWM Variables)
int stimFrequency = 100;  // Stimulation frequency in Hertz (PWM Variables)
int stimDuration = 4;     // Stimulation duration in seconds (PWM Variables)
int Repetitions = 16;     // intermittent paradigm: 200ms ON + 50ms OFF; TR=2s, 16 repetitions that totals 4 seconds
int intermittentON = 200; // in milliseconds
int intermittentOFF = 50; // in mseconds (for delay function)

// ----------------------------------------------------------------------------------------- //
// ----- INPUTS ---------------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //

// ----- Stimulation eletrodes (Stim site) ------------------------------------------------- //
// PrepData/Step3_PreparaDataForArduino.mat
int StimulationSequence_Active[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int StimulationSequence_Ground[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

// ----- Jitters (s) ------------------------------------------------------------------ //
// FROM: PrepData/Step2_JitterGenerator.m
unsigned long Jitters[] = {8000, 10000, 8000, 6000, 6000, 10000, 6000, 10000, 8000, 6000, 8000, 10000, 6000, 10000, 8000, 10000, 6000, 10000, 8000, 6000, 6000, 10000, 8000, 8000, 6000, 8000, 10000, 10000, 8000, 6000, 10000, 10000, 6000, 6000, 8000, 10000, 8000, 6000, 8000, 8000};

// ----- Perception thresholds (Current (mA)) ---------------------------------------------- //
// FROM: PrepData/Step3_PreparaDataForArduino.mat

float Currents[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// ----- Perception thresholds (Voltage (V)) ---------------------------------------------- //
// FROM: PrepData/Step3_PreparaDataForArduino.mat

float Voltages[] = {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};

// ----------------------------------------------------------------------------------------- //
// ----- PWM timing parameters ------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //

// Calculate delays after pulseON
// -> 0.109 ms is the time that the analogRead() inside the loop takes to run
// -> 0.007 is the time that digitalWrite() takes to run
float pulseONDelay = pulseWidth - 0.007 - 0.109;
int pulseONDelayMicroSeconds = round(pulseONDelay * 1000);

// Calculate delays after pulseOFF
float pulseOFFDelay = (1000.0 / stimFrequency) - pulseWidth - 0.007 - 0.109; // in milliseconds
float pulseOFFDelay1 = floor(pulseOFFDelay);
int pulseOFFDelay2 = round((pulseOFFDelay - pulseOFFDelay1) * 1000);

// ----------------------------------------------------------------------------------------- //
// ----- PINS ------------------------------------------------------------------------------ //
// ----------------------------------------------------------------------------------------- //

// DAC pins
#define SS 53     // affectation of CS pin
#define VsetPin 0 // Vset is output pin A in DAC
#define IlimPin 1 // Ilim is output pin B in DAC

// PWM pins
int PWMPin = 8; // Arduino mega pin 8

// Switch board pins
int SBPins[] = {14, 15, 16, 17, 18, 19, 6, 7, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 2, 3, 4, 5};
int nSBPins = 40;

// Trigger and stop pins
int TriggerPin = A8;
int StopPin = A9;
float output[2] = {0, 0};
// ----------------------------------------------------------------------------------------- //
// ----- Initialization -------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- /
bool triggerFlag = true;                  // flag MRI trigger

// ----- VOID SETUP ------------------------------------------------------------------------ //
// ----------------------------------------------------------------------------------------- //
void setup() {
  // Serial communication
  Serial.begin(19200);

  // ----- Switch board Setup ------------------------------------------------------------- //
  for (int pp = 0; pp < nSBPins; pp++) {
    pinMode(SBPins[pp], OUTPUT);
  }

  // turn off all relays
  turnOffAll(SBPins, nSBPins);

  // ----- DAC Setup --------------------------------------------------------------------- //

  // initialize SPI
  SPI.begin();                          // corresponds to SPI2 - available on connector J8
  SPI.setDataMode(SPI_MODE0);           // Clock starts at a LOW value and transmits data on the falling edge of the clock signal
  SPI.setBitOrder(MSBFIRST);            // during the transfer of data, send the most significant bit first
  SPI.setClockDivider(SPI_CLOCK_DIV16); // configuration of clock at 1 MHz

  // set SS pin as output
  pinMode(SS, OUTPUT);

  // set up DAC
  digitalWrite(SS, LOW);               // selecting the DAC
  setUpDAC();                          // defined function call to set up the DA4 to use its internal voltage reference
  digitalWrite(SS, HIGH);              // deselecting the DAC

  // ----- PWM Setup -------------------------------------------------------------------- //
  pinMode(PWMPin, OUTPUT);

  // Initialize as off
  digitalWrite(PWMPin, HIGH);

  // ----- Trigger and stop ---------------------------------------------------------------------- //
  pinMode(TriggerPin, INPUT);
  pinMode(StopPin, INPUT);


}

void loop() {

  // wait for trigger
  //Serial.println(analogRead(TriggerPin));
  
  if (analogRead(TriggerPin) > 1000) { //trigger corresponds to ~5V, therefore > 1000 (0-5V --> 0-1023)
    triggerFlag = true;
  }

  if (triggerFlag) {

    Serial.println("Trigger received from Uno.");

    unsigned long StartTime = millis(); //Serial.print("StartTime (ms) = "); Serial.println(StartTime);

    // ----- Wait 10s ----------------------------------------------------------------------------------------------- //
    Serial.println("Waiting 10 seconds...");
    //delay(10*1000);
    unsigned long CurrentTime = millis();
    while (CurrentTime - StartTime < 10000) {
      CurrentTime = millis();
    }
    //unsigned long ElapsedTimeWait1 = CurrentTime - StartTime; Serial.print("ElapsedTime Wait1 (ms) = "); Serial.println(ElapsedTimeWait1);

    // ----- Start run ----------------------------------------------------------------------------------------------- //
    for (int electrodeIndex = 0; electrodeIndex < nElectrodes * 2; electrodeIndex++) {
      Serial.println(" ");

      //unsigned long StartTimeForLoop = millis(); Serial.print("StartTime ForLoop (ms) = "); Serial.println(StartTimeForLoop);

      // ----- Define ACTIVE and GROUND electrodes ----------------------------------------------------------------- //

      //unsigned long StartTimeSwitchElectrodes = millis();

      switchElectrodes(StimulationSequence_Active[electrodeIndex], StimulationSequence_Ground[electrodeIndex], SBPins, nSBPins);
      //delay(4); //compensate for switchElectrodes (ms)

      //unsigned long CurrentTimeSwitchElectrodes = millis();
      //unsigned long ElapsedTimeSwitchElectrodes = CurrentTimeSwitchElectrodes - StartTimeSwitchElectrodes; Serial.print("ElapsedTime SwitchElectrodes (ms) = "); Serial.println(ElapsedTimeSwitchElectrodes);

      // ----- Set VSet and ILim ----------------------------------------------------------------- //
      // The true ILim value is the sum of ILimRange and ILimCompensation (this is necessary due to a small deviation in the stimulation board

      float ILimCompensation = 0.15 * Currents[electrodeIndex];
      float ILimActualValue_Compensated = Currents[electrodeIndex] + ILimCompensation;

      // convert Ilim & Vset (stimulation outputs in V (max 100V))
      int ILimValue = convertmiliAtoV(ILimActualValue_Compensated);
      int VSetValue = convertVtoV(Voltages[electrodeIndex]);

      // -----  Set Vset and ILim values in the DAC -------------------------------------------//
      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(VsetPin, VSetValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(IlimPin, ILimValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      //unsigned long CurrentTimeSetValues = millis();
      //unsigned long ElapsedTimeSetValues = CurrentTimeSetValues - StartTimeSetValues; Serial.print("ElapsedTime SetValues (ms) = "); Serial.println(ElapsedTimeSetValues);

      // ----- Stimulate ---------------------------------------------------------------------- //
      Serial.print("START stimulation: electrodeIndex = ");      Serial.print(electrodeIndex);
      Serial.print(" | activeElectrode = ");      Serial.println(StimulationSequence_Active[electrodeIndex]);
      //Serial.print(" | groundElectrode = ");      Serial.println(StimulationSequence_Ground[electrodeIndex]);

      // ----- 1. Jitter ---------------------------------------------------------------------- //
      unsigned long StartTimeJitter = millis();
      unsigned long CurrentTimeJitter = millis();
      Serial.print("Waiting ... Jitter (s): ");  Serial.println(Jitters[electrodeIndex]);
      //delay(Jitters[electrodeIndex] * 1000);
      while (CurrentTimeJitter - StartTimeJitter < Jitters[electrodeIndex]) {
        CurrentTimeJitter = millis();
      }
      //unsigned long ElapsedTimeJitter = CurrentTimeJitter - StartTimeJitter; Serial.print("ElapsedTime Jitter (ms) = "); Serial.println(ElapsedTimeJitter);

      // ----- 2. Stimulation ---------------------------------------------------------------------- //
      //unsigned long StartTimeStimulation = millis();

      for (int nrRepetitions = 0; nrRepetitions < Repetitions; nrRepetitions++) {

        stimulate(stimFrequency, intermittentON, intermittentOFF, PWMPin, pulseONDelayMicroSeconds, pulseOFFDelay1, pulseOFFDelay2, 100, output); // assumindo Vset = 100 (só para não alterar a função stimualte abaixo)

      }
      //unsigned long CurrentTimeStimulation = millis();
      //unsigned long ElapsedTimeStimulation = CurrentTimeStimulation - StartTimeStimulation; Serial.print("ElapsedTime Stimulation (ms) = "); Serial.println(ElapsedTimeStimulation);

      Serial.println("STOP stimulation. Parameters:");
      //Serial.print("Vmon max (0-2.5 V): "); Serial.println(output[0]);
      Serial.print("Vmon max (0-100 V): "); Serial.println(output[0] * 40);           // conversion to V
      //Serial.print("Imon max (0-2.5 V): "); Serial.println(output[1]);
      Serial.print("Imon max (0-5 mA):  "); Serial.println(output[1] * 2);            // conversion to mA

      // break if STOP signal is on
      if (analogRead(StopPin) > 1000) {
        Serial.println("STOP received from Uno.");
        turnOffAll(SBPins, nSBPins);
        break;
      }

      //unsigned long CurrentTimeForLoop = millis();
      //unsigned long ElapsedTimeForLoop = CurrentTimeForLoop - StartTimeForLoop; Serial.print("ElapsedTime ForLoop (ms) = "); Serial.println(ElapsedTimeForLoop);

    }

    //unsigned long StartTimeTurnOff = millis();
    turnOffAll(SBPins, nSBPins);
    //unsigned long CurrentTimeTurnOff = millis();
    //unsigned long ElapsedTimeTurnOff = CurrentTimeTurnOff - StartTimeTurnOff; Serial.print("ElapsedTime TurnOff (ms) = "); Serial.println(ElapsedTimeTurnOff);

    // ----- Wait 10s ----------------------------------------------------------------------------------------------- //
    unsigned long StartTimeDelay2 = millis();
    unsigned long CurrentTimeDelay2 = millis();
    Serial.println(" "); Serial.println("Waiting another 10 seconds...");
    //delay(10*1000);
    while (CurrentTimeDelay2 - StartTimeDelay2 < 10000) {
      CurrentTimeDelay2 = millis();
    }
    //unsigned long ElapsedTimeDelay2 = CurrentTimeDelay2 - StartTimeDelay2; Serial.print("ElapsedTime Delay2 (ms) = "); Serial.println(ElapsedTimeDelay2);

    unsigned long CurrentTimeTotal = millis();
    unsigned long ElapsedTimeTotal = CurrentTimeTotal - StartTime; Serial.print("ElapsedTime Total (ms) = "); Serial.println(ElapsedTimeTotal);

    Serial.println("---------------------- END OF RUN ----------------------");

    // Reset trigger flag
    triggerFlag = false;

  }

}

// ----- FUNCTIONS ------------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //

/*
  p
*/
float stimulate(int stimFrequency, int stimDuration, int intermittentOFF, int PWMPin, float pulseONDelayMicroSeconds, float pulseOFFDelay1, int pulseOFFDelay2, float VSet, float (& output) [2]) {

  float VmonArray[stimFrequency * stimDuration / 1000];
  float ImonArray[stimFrequency * stimDuration / 1000];

  // ----- PWM ---------------------------------- //
  for (int ii = 0; ii < (stimFrequency * stimDuration / 1000); ii++) {

    // Turn on
    digitalWrite(PWMPin, LOW);
    delayMicroseconds(pulseONDelayMicroSeconds);

    // Read Imon - this takes approximately 0.109 ms
    ImonArray[ii] = analogRead(A2);

    // Turn off
    digitalWrite(PWMPin, HIGH);

    // Read Vmon - this takes approximately 0.109 ms
    VmonArray[ii] = analogRead(A1);

    delay(pulseOFFDelay1);
    delayMicroseconds(pulseOFFDelay2);

  } // end PWM

  float VmonMax = maxValue(VmonArray) * (5 / 1023.0);                           // conversion to volts
  //Serial.print("Vmon max (0-2.5 V): "); Serial.println(VmonMax);

  VSet = ceil(VmonMax * 40.0 + 10);                                             // conversion to 0-100 V range plus the margin of 5 volts
  //Serial.print("Vset max (0-100V): ");  Serial.println(VSet);

  float ImonMax = maxValue(ImonArray) * (5 / 1023.0);                           // conversion to volts
  //Serial.print("Imon max (0-2.5 V): "); Serial.println(ImonMax);
  //Serial.print("Imon max (0-5 mA): ");  Serial.println(ImonMax * 2);          // conversion to mA

  unsigned long StartTimeOFF = millis();
  unsigned long CurrentTimeOFF = millis();
  while (CurrentTimeOFF - StartTimeOFF < 50) {
    CurrentTimeOFF = millis();
  }
  //delay(intermittentOFF); // wait 50ms - intermittent paradigm

  output[0] = VmonMax;
  output[1] = ImonMax;
  return VSet;

}

/*
  Returns the maximum value of an array.
*/
float maxValue(float myArray[]) {
  float maxVal = myArray[0];
  float minVal = myArray[0];

  for (int i = 0; i < (sizeof(myArray) / sizeof(myArray[0])); i++) {
    if (myArray[i] > maxVal) {
      maxVal = myArray[i];
    }
  }

  return maxVal;
}

/*
  Converts from milliamperes to volts (2 mA <--> 1 V) and then to the DAC range of 0-4095.
*/
float convertmiliAtoV(float mA) {
  // 2.5V corresponds to 5mA and the value 4095

  float volt = mA / 2; // 2mA/V
  return volt * 4095 / 2.5; // conversion to the range 0-4095

}

/*
  Converts from stimulation volts to Vset volts (40 V <--> 1 V) and then to the DAC range of 0-4095.
*/
float convertVtoV(float outV) {
  // 2.5V corresponds to 100V and the value 4095

  float volt = outV / 40;
  return volt * 4095 / 2.5; // conversion to the range 0-4095

}

/*
  Communicate with the DAC. Writes the value "value" to the pin set by "address".
  Valid address values are between 0 and 7; 0 for pin A, 1 for pin B, and so on.
  An address value of 15 will provide power to all of the outputs simultaneously.
  Value should be in the range 0-4095.
*/
void sendAddressAndValue(int address, int value) {

  if (((address >= 0 && address <= 7) || address == 15) && (value > 0 && value <= 4095)) {

    SPI.transfer(0b00000011); // 4 bits of junk and a command to update DAC register
    delay(1); // small time delay to ensure the DAC processes the data

    int value1 = highByte(value); // collecting the upper half of the value
    int value2 = lowByte(value); // collecting the lower half of the value
    int addressAndValue1 = (address << 4) + value1; // combining the address and value into one piece
    SPI.transfer(addressAndValue1); // the 4 bits of address and the upper half of the value

    delay(1); // small time delay to ensure the DAC processes the data

    SPI.transfer(value2); // the lower half of the value
    delay(1); // small time delay to ensure the DAC processes the data

    SPI.transfer(0);//junk data
    delay(1); // small time delay to ensure the DAC processes the data

  } else {

    SPI.transfer(0b00000011); // 4 bits of junk and a command to update DAC register
    delay(1); // small time delay to ensure the DAC processes the data
    SPI.transfer(0b11110000); // all addresses and a value of zero
    delay(1); // small time delay to ensure the DAC processes the data
    SPI.transfer(0); // a value of zero
    delay(1); // small time delay to ensure the DAC processes the data
    SPI.transfer(0);//junk data
    delay(1); // small time delay to ensure the DAC processes the data

  } // end of else statement

} //end of sendAddressAndValue

/*
  Set up the communication with the DAC.
*/
void setUpDAC() {

  SPI.transfer(0b00001000);//4 bits of junk and a command to change reference voltage
  delay(1); // small time delay to ensure DAC processes the data
  SPI.transfer(0);//sending more values since it doesn't care about these (thanks to the Command) and we need 32 bits total
  delay(1);
  SPI.transfer(0);//same reason
  delay(1);
  SPI.transfer(0b00000001);//7 bits of ignored data and a final "1" to indicate to use the internal reference
  delay(1);

  digitalWrite(SS, HIGH);
  delay(1);
  digitalWrite(SS, LOW);

  //turning all the outputs off to begin with
  SPI.transfer(0b00000011); // 4 bits of junk and a command to update DAC register
  delay(1); // small time delay to ensure the DAC processes the data
  SPI.transfer(0b11110000); // all addresses and a value of zero
  delay(1); // small time delay to ensure the DAC processes the data
  SPI.transfer(0); // a value of zero
  delay(1); // small time delay to ensure the DAC processes the data
  SPI.transfer(0);//junk data
  delay(1); // small time delay to ensure the DAC processes the data

}

/*
  Turn off all the relays in the switch board.
*/
void turnOffAll(int SBPins[], int nSBPins) {

  for (int pp = 0; pp < nSBPins; pp++) {
    digitalWrite(SBPins[pp], LOW);
  }

}

/*
  Set the ACTIVE and GROUND electrodes based on the switch board operation logic.
*/
void switchElectrodes(int activeElectrode, int groundElectrode, int SBPins[], int nSBPins) {

  // turn off all pins
  turnOffAll(SBPins, nSBPins);

  // fetch corresponding pins
  int activePin = activeElectrode * 2 - 1;
  int groundPin = groundElectrode * 2 - 1;

  // Execute
  digitalWrite(SBPins[activePin - 1], HIGH);
  digitalWrite(SBPins[activePin], HIGH);
  digitalWrite(SBPins[groundPin - 1], HIGH);

}
