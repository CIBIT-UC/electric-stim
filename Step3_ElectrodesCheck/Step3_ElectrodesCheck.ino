/* Electrodes Initial Check v1.0

   This sketch allows to check the voltages for each electrode for the main stimulation task.

*/

#include <SPI.h>  // Include the SPI library
#include <math.h> // Include the math library

// ----- Stimulation properties ------------------------------------------------------------ //
// ----------------------------------------------------------------------------------------- //
int nElectrodes = 20;     // Number of stimulation electrodes
float pulseWidth = 0.200; // Pulse width in milliseconds (PWM Variables)
int stimFrequency = 30;  // Stimulation frequency in Hertz (PWM Variables)
int stimDuration = 2;     // Stimulation duration in seconds (PWM Variables)
float ILimStartValue = 1; // Initial value of Ilim/current (in mA)

// ----- INPUTS ---------------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //
// Perception thresholds - From ThresholdUpdate_MRI.ino
float Currents[] = {2.10, 1.95, 3.3, 1.20, 1.35, 2.20, 2.50, 1.80, 1.60, 1.80, 1.35, 1.20, 1.35, 1.45, 1.35, 1.35, 1.10, 1.60, 1.70, 2.10};

// ----- Stimulation eletrodes ------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //
int StimulationSequence_Active[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
int StimulationSequence_Ground[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 19};

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

// Button pins
const int buttonPreviousElectrodePin = 9;               // the number of the pushbutton pin - Previous electrode
const int buttonNextElectrodePin = 10;                  // the number of the pushbutton pin - Next electrode
//const int buttonResponseNoPin = 11;                     // the number of the pushbutton pin - Answer: NO
const int buttonResponseYesPin = 12;                    // the number of the pushbutton pin - Answer: YES

// ----- Initialization -------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //
int CurrentElectrode = 0;                               // index of electrode in range StimulationSequence_Active (will be updated in real-time)
int buttonPreviousElectrodeState = 0;                   // state of Electrode button - Previous (will be updated in real-time)
int buttonNextElectrodeState = 0;                       // state of Electrode button - Next (will be updated in real-time)
int buttonResponseYesState = 0;                         // state of stepRange button - Yes (will be updated in real-time)
// int buttonResponseNoState = 0;                          // state of stepRange button - No (will be updated in real-time)
float output[2] = {0, 0};                               // array for outputs from stimulation
float voltagesArray[20];                               // voltages threhsolds array - an array capable of holding 20 entries numbered 0 to 19; stores thresholds for each electrode

//Vmon--> A1
//Imon-->A2

// ILim initial values
float maxILimActualValue = 4.30;                        // max Ilim value (mA)
float ILimActualValue = ILimStartValue;                 // current value for Ilim in mA (will be updated in real-time)

// Vset initial values
int maxVSet = 80;                                      // maximum value for Vset in Volts
int VSet = maxVSet;                                     // current value for Vset in Volts (will be updated in real-time)

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

  // ----- Buttons ---------------------------------------------------------------------- //
  pinMode(buttonPreviousElectrodePin, INPUT);
  pinMode(buttonNextElectrodePin, INPUT);
  //pinMode(buttonResponseNoPin, INPUT);  
  pinMode(buttonResponseYesPin, INPUT);

}

void loop() {

  int timer = 0;
  
  Serial.println(" ");
  Serial.println("1. SELECT ELECTRODE (use the 1st and 2nd buttons)."); 
  Serial.println("2. To start the stimulation press the 4th (YES) button.");
  
  while (timer < 10000) {

    // Read button state
    buttonPreviousElectrodeState = digitalRead(buttonPreviousElectrodePin);
    buttonNextElectrodeState = digitalRead(buttonNextElectrodePin);
    buttonResponseYesState = digitalRead(buttonResponseYesPin);
    //buttonResponseNoState = digitalRead(buttonResponseNoPin);

    if (buttonPreviousElectrodeState) {
      CurrentElectrode = CurrentElectrode - 1;
      Serial.print("Electrode: "); Serial.println(CurrentElectrode);

    } else if (buttonNextElectrodeState) {
      CurrentElectrode = CurrentElectrode + 1;
      Serial.print("Electrode: "); Serial.println(CurrentElectrode);

    } else if (buttonResponseYesState) {  // reset Vset and ILim values
      ILimActualValue = ILimStartValue;
      VSet = maxVSet;

      break;
    }

    delay(150);

    // Validate currentElectrode value
    if (CurrentElectrode > nElectrodes) {
      CurrentElectrode = 0;
      Serial.print("Electrode: "); Serial.println(CurrentElectrode);
    } else if (CurrentElectrode < 0) {
      CurrentElectrode = 20;
      Serial.print("Electrode: "); Serial.println(CurrentElectrode);
    }
    
    timer++;

  }
  
  // Wait 1 second to avoid button press repetition
  delay(1000); 
  
  if (CurrentElectrode != 0) {

      //buttonResponseNoState = 0;
      buttonResponseYesState = 0;
      buttonPreviousElectrodeState = 0;
      buttonNextElectrodeState = 0;

      // ----- STIM --------------------------------------- //
      Serial.println(" "); Serial.println("START STIMULATION");
      
      // ----- Set VSet and ILim ----------------------------------------------------------------- //
      // The true ILim value is the sum of ILimRange and ILimCompensation (this is necessary due to a small deviation in the stimulation board)      
      float ILimCompensation = 0.15 * Currents[CurrentElectrode-1];
      float ILimActualValue_Compensated = Currents[CurrentElectrode-1] + ILimCompensation;

      // convert Ilim & Vset (stimulation outputs in V (max 100V))
      int ILimValue = convertmiliAtoV(ILimActualValue_Compensated);
      int VSetValue = convertVtoV(VSet);

      // -----  Set Vset and ILim values in the DAC -------------------------------------------//
      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(VsetPin, VSetValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(IlimPin, ILimValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      // ----- Switch board ------------------------- //
      switchElectrodes(StimulationSequence_Active[CurrentElectrode - 1], StimulationSequence_Ground[CurrentElectrode - 1], SBPins, nSBPins);

      // ----- PWM to stimulate ------------------------ //
      Serial.print("Stimulating for "); Serial.print(stimDuration); Serial.println(" seconds ...");
      Serial.print("ActiveElectrode = ");             Serial.println(StimulationSequence_Active[CurrentElectrode-1]);
      Serial.print(" [Theoretical] Current = ");      Serial.println(Currents[CurrentElectrode-1]);
      Serial.print(" [Theoretical] Voltage = ");      Serial.println(round(VSetValue * 2.5 * 40 / 4095));
      
      VSet = stimulate(stimFrequency, stimDuration, PWMPin, pulseONDelayMicroSeconds, pulseOFFDelay1, pulseOFFDelay2, VSetValue);
      voltagesArray[CurrentElectrode - 1] = VSet;

      Serial.print("Voltages array: ");
          for (int i = 0; i < (sizeof(voltagesArray) / sizeof(voltagesArray[0])); i++) {
            Serial.print(voltagesArray[i]); Serial.print(", ");
          }
      Serial.println(" ");
      
      delay(1000);

  } else {
    turnOffAll(SBPins, nSBPins);
  }
  
  delay(1000); // Wait 1 second to avoid button press repetition

}

// ----- FUNCTIONS ------------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //

/*
  p
*/
float stimulate(int stimFrequency, int stimDuration, int PWMPin, float pulseONDelayMicroSeconds, float pulseOFFDelay1, float pulseOFFDelay2, float VSet) {

  float VmonArray[stimFrequency * stimDuration];
  float ImonArray[stimFrequency * stimDuration];

  // ----- PWM ---------------------------------- //
  for (int ii = 0; ii < stimFrequency * stimDuration; ii++) {

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
  Serial.print("Vmon max (0-2.5 V): "); Serial.println(VmonMax);

  VSet = ceil(VmonMax * 40.0 + 10);                                             // conversion to 0-100 V range plus the margin of 5 volts
  Serial.print("Vset max (0-100V): ");  Serial.println(VSet);

  float ImonMax = maxValue(ImonArray) * (5 / 1023.0);                           // conversion to volts
  Serial.print("Imon max (0-2.5 V): "); Serial.println(ImonMax);
  Serial.print("Imon max (0-5 mA): ");  Serial.println(ImonMax * 2);  // conversion to mA

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
  Set the active and ground electrodes based on the switch board operation logic.
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
