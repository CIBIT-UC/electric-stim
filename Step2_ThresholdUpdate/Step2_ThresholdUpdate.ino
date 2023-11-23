/* Threshold Update v1.0

   This sketch allows the definition of a threshold for each electrode pair.

*/

#include <SPI.h>  // Include the SPI library
#include <math.h> // Include the math library

// ----- Stimulation properties ------------------------------------------------------------ //
// ----------------------------------------------------------------------------------------- //
int nElectrodes = 20;     // Number of stimulation electrodes
float pulseWidth = 0.200; // Pulse width in milliseconds (PWM Variables)
int stimFrequency = 30;   // Stimulation frequency in Hertz (PWM Variables)
int stimDuration = 2;     // Stimulation duration in seconds (PWM Variables)
float ILimStartValue = 1; // Initial value of Ilim/current (in mA)
int VSetStartValue = 100; // Initial value of VSet (in V)

// ----- INPUTS ---------------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //

// Subject
float Currents[] = {1.35, 1.2, 2.55, 0.45, 1.1, 1.95, 2.25, 1.55, 1.35, 1.55, 1.1, 0.95, 0.6, 1.2, 1.1, 1.1, 0.85, 1.35, 1.45, 1.35};
float Voltages[] = {28, 27, 26, 15, 23, 32, 36, 31, 32, 36, 28, 23, 19, 28, 28, 29, 22, 27, 29, 27};

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
const int buttonResponseNoPin = 11;                     // the number of the pushbutton pin - Answer: NO
const int buttonResponseYesPin = 12;                    // the number of the pushbutton pin - Answer: YES

// ----- Initialization -------------------------------------------------------------------- //
// ----------------------------------------------------------------------------------------- //
int currentElectrode = 0;                               // index of electrode in range StimulationSequence_Active (will be updated in real-time)
int buttonPreviousElectrodeState = 0;                   // state of Electrode button - Previous (will be updated in real-time)
int buttonNextElectrodeState = 0;                       // state of Electrode button - Next (will be updated in real-time)
int buttonResponseNoState = 0;                          // state of stepRange button - No (will be updated in real-time)
int buttonResponseYesState = 0;                         // state of stepRange button - Yes (will be updated in real-time)
int answersArray[20];                                   // answers array (1=yes/sentiu, 2=no/não sentiu) - an array capable of holding 20 entries numbered 0 to 19
int answersArrayIndex = 0;
float currentsArray[20];                                // currents threhsolds array - an array capable of holding 20 entries numbered 0 to 19; stores thresholds for each electrode
int maxCurrentCount = 0;
boolean continueflag = false;

//Vmon--> A1
//Imon--> A2

// ILim initial values
float maxILimActualValue = 4.30;                        // max Ilim value (mA)
float ILimActualValue = ILimStartValue;                 // current value for Ilim in mA (will be updated in real-time)

// Vset initial values
int maxVSet = 100;                                      // maximum value for Vset in Volts
int maxVSetActualValue = 80;                            // max safe value for VSet
int VSetActualValue = VSetStartValue;                   // current value for VSet in Volts (will be updated in real-time)

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
  pinMode(buttonResponseNoPin, INPUT);
  pinMode(buttonResponseYesPin, INPUT);

}

void loop() {

  // Cleaning previous arrays/variables
  answersArrayIndex = 0;
  maxCurrentCount = 0;
  continueflag = false;
  memset(answersArray, 0, sizeof(answersArray));
  //memset(currentsArray, 0, sizeof(currentsArray));

  Serial.println(" "); Serial.println("--------------------------------------------------");

  // ----- CHOOSE ELECTRODE --------------------------------------------------------------------- //
  int timer = 0;

  Serial.println("1. SELECT ELECTRODE (use the 1st and 2nd buttons)."); 
  Serial.println("2. To start the stimulation press the 4th (YES) button.");
  
  while (timer < 10000) {

    // Read button state
    buttonPreviousElectrodeState = digitalRead(buttonPreviousElectrodePin);
    buttonNextElectrodeState = digitalRead(buttonNextElectrodePin);
    buttonResponseYesState = digitalRead(buttonResponseYesPin);

    if (buttonPreviousElectrodeState) {
      currentElectrode = currentElectrode - 1;
      continueflag = true;
      Serial.print("Electrode: "); Serial.println(currentElectrode);

    } else if (buttonNextElectrodeState) {
      currentElectrode = currentElectrode + 1;
      continueflag = true;
      Serial.print("Electrode: "); Serial.println(currentElectrode);

    } else if (buttonResponseYesState) {  // reset Vset and ILim values
      // reset Vset and ILim values
      ILimActualValue = ILimStartValue;
      VSetActualValue = VSetStartValue;

      break;
    }

    delay(150);

    // Validate currentElectrode value
    if (currentElectrode > nElectrodes) {
      currentElectrode = 0;
      Serial.print("Electrode: "); Serial.println(currentElectrode);
    } else if (currentElectrode < 0) {
      currentElectrode = 20;
      Serial.print("Electrode: "); Serial.println(currentElectrode);
    }

    timer++;

  }

  // Wait 1 second to avoid button press repetition
  delay(1000);

  // Update ILimActualValue and VSetValue
  ILimActualValue = Currents[currentElectrode - 1];
  Serial.print("ILim threshold: ");    Serial.println(ILimActualValue);
  VSetActualValue = maxVSetActualValue;
  Serial.print("VSet threshold: ");         Serial.println(VSetActualValue);
  
  while (continueflag == true) {

    // --- THRESHOLD DEFINITION --- //
    if (currentElectrode != 0) {

      buttonResponseNoState = 0;
      buttonResponseYesState = 0;
      buttonPreviousElectrodeState = 0;
      buttonNextElectrodeState = 0;

      // ----- STIM --------------------------------------- //
      Serial.println(" "); Serial.println("START STIMULATION");

      // ----- Set VSet and ILim ----------------------------------------------------------------- //
      // The true ILim value is the sum of ILimRange and ILimCompensation (this is necessary due to a small deviation in the stimulation board)
      float ILimCompensation = 0.15 * ILimActualValue;
      float ILimActualValue_Compensated = ILimActualValue + ILimCompensation;

      // convert Ilim & Vset (stimulation outputs in V (max 100V))
      int ILimValue = convertmiliAtoV(ILimActualValue_Compensated);
      int VSetValue = convertVtoV(VSetActualValue);

      // -----  Set Vset and ILim values in the DAC -------------------------------------------//
      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(VsetPin, VSetValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      digitalWrite(SS, LOW);                    // selects the DAC
      sendAddressAndValue(IlimPin, ILimValue);  // full analog signal
      digitalWrite(SS, HIGH);                   // deselects the DAC

      // ----- Print values ------
      Serial.print("ILim actual value (before compensation) (0-5 mA): ");       Serial.println(ILimActualValue);
      Serial.print("ILim actual value (after compensation) (0-5 mA): ");        Serial.println(ILimActualValue_Compensated);
      Serial.print("ILim value in V (0-2.5 V): ");                              Serial.println(ILimActualValue_Compensated / 2);
      Serial.print("VSet actual value (DAC range of 0-4095): ");                Serial.println(VSetValue);
      Serial.println(" ");

      // ----- Switch board ------------------------- //
      switchElectrodes(StimulationSequence_Active[currentElectrode - 1], StimulationSequence_Ground[currentElectrode - 1], SBPins, nSBPins);

      // ----- PWM to stimulate ------------------------ //
      if (ILimActualValue == 0) {
        Serial.println(" "); Serial.println("!!! Skipping stimulation because ILim = 0 mA. ANSWER NO IN THE NEXT QUESTION. !!!"); Serial.println(" ");
      } else {
        
        Serial.println(" "); Serial.print("Stimulating for "); Serial.print(stimDuration); Serial.println(" seconds ...");
        
        VSetActualValue = stimulate(stimFrequency, stimDuration, PWMPin, pulseONDelayMicroSeconds, pulseOFFDelay1, pulseOFFDelay2, VSetValue);
        
      }

      // ----- Decision ------------------------ //
      Serial.println(" "); Serial.println("Did participant feel the stimulation?");
      Serial.println("Press button 3 if the answer is NO, or button 4 if the answer is YES.");

      timer = 0;
      while (timer < 10000) {

        buttonResponseNoState = digitalRead(buttonResponseNoPin);
        buttonResponseYesState = digitalRead(buttonResponseYesPin);
        buttonPreviousElectrodeState = digitalRead(buttonPreviousElectrodePin);
        buttonNextElectrodeState = digitalRead(buttonNextElectrodePin);

        if (buttonResponseNoState) {
          Serial.println("Answer: No.");
          answersArray[answersArrayIndex] = 2;                // Answer: no = 2
          break;

        } else if (buttonResponseYesState) {
          Serial.println("Answer: Yes.");
          answersArray[answersArrayIndex] = 1;                // Answer: yes = 1
          break;

        } else if (buttonNextElectrodeState || buttonPreviousElectrodeState) {
          ILimActualValue = -99;
          break;
        }

        delay(150);
        timer++;

      } //end while(timmer<1000) cycle


      // ---- Update Ilim Actual Value and Current array ----
      // Ilim = Ilim + 0.25 (if the answer is yes) or Ilim = Ilim + 0.50 (if the answer is no)

      if ( answersArrayIndex == 0 && (answersArray[answersArrayIndex] == 1) ) {
        Serial.println("Answer=YES (first response)"); Serial.println(" ");
        ILimActualValue = Currents[currentElectrode - 1] + 0.25;
        continueflag = false;
        currentsArray[currentElectrode - 1] = ILimActualValue;

      } else if ( answersArrayIndex == 0 && (answersArray[answersArrayIndex] == 2) ) {
        Serial.println("Answer=NO (first response)"); Serial.println(" ");
        ILimActualValue = Currents[currentElectrode - 1] + 0.50;
        continueflag = true;

      } else if ( ( (answersArray[answersArrayIndex] == answersArray[answersArrayIndex - 1]) ) && (answersArray[answersArrayIndex] == 2) ) {
        Serial.println("Answer=NO (same response as previous)"); Serial.println(" ");
        ILimActualValue = ILimActualValue + 0.50;
        continueflag = true;

      } else if (answersArray[answersArrayIndex] != answersArray[answersArrayIndex - 1] && (answersArray[answersArrayIndex] == 1)) {
        Serial.println("Answer=YES (different response)"); Serial.println(" ");
        ILimActualValue = ILimActualValue + 0.25;
        continueflag = false;
        currentsArray[currentElectrode - 1] = ILimActualValue;
      }

      // Validate new ILim values
      if (ILimActualValue < 0) {                                                         // Ilim(min) = 0mA
        Serial.println(" ");           Serial.println("!!! WARNING - Ilim < 0 mA !!!");   Serial.println(" ");
        LimActualValue = 0;           // set the minimum value for ILim. the participant should report nothing at this value. Current will increase in the next iteration.

      } else if (ILimActualValue > maxILimActualValue) {                                 // Ilim(max) = 4.4mA
        Serial.println(" ");           Serial.println("!!! WARNING - Ilim > 4.3 mA !!!");   Serial.println(" ");
        ILimActualValue = maxILimActualValue;     // set the maximum value for ILim. if it works with this value, ok, if not, it means that the participant has to be excluded.
        maxCurrentCount = maxCurrentCount + 1;

        if (maxCurrentCount == 2) {
          Serial.println(" "); Serial.println("---- !!!! Participant not eligible. Aborting !!!! ---");
        }
      }

      // Validate new VSet values
      if (VSetActualValue > maxVSetActualValue) {                                 // maxVSetActualValue = 80V --> escolhi 80 para nos dar este sinal de alerta!!
        Serial.println(" ");           Serial.println("!!! WARNING - VSet > 80V !!!");   Serial.println(" ");
        VSetActualValue = maxVSetActualValue;  // set the maximum value for VSet. if it works with this value, ok, if not, it means that the participant has to be excluded.
        maxCurrentCount = maxCurrentCount + 1;

        if (maxCurrentCount == 2) {
          Serial.println(" "); Serial.println("---- !!!! Participant not eligible. Aborting !!!! ---");
        }
      }

      delay(1000);

      // ---- Threshold definition ----
      if (answersArray[answersArrayIndex] == 1 && maxCurrentCount != 2 && continueflag == false) {  // If the participant felt the last adjustment, use the atual current value (IlimActual).
        Serial.println("------ Threshold defined. ------");
        
        Serial.print("---- ILim value = "); Serial.print(ILimActualValue); Serial.println(" mA. ----------"); 
        
        Serial.print("Currents array: ");
          for (int i = 0; i < (sizeof(currentsArray) / sizeof(currentsArray[0])); i++) {
            Serial.print(currentsArray[i]); Serial.print(", ");
          }
      Serial.println(" "); 
      }

      // Update indexes
      answersArrayIndex++;
      //currentsArrayIndex++;

    } else {
      turnOffAll(SBPins, nSBPins);
    }

    // Wait 1 second to avoid button press repetition
    delay(1000);
  }
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
