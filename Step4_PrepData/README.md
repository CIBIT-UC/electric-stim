# Step4_PrepData
This step is to prepare the data for the Step 5.

## ElectrodeSequenceGenerator.m

MATLAB code to generate a pseudo-random sequence of electrodes for the stimulation. This is performed per subject and per run. The sequence is then provided to PreparaDataForArduino.m.

## Jitter Generator

MATLAB code to estimate time jitters between the stimulation of each electrode. The output is then copied to the Arduino code of Step 5.

## PrepareDataForArduino.m

Format the electrode sequence, currents and voltages for each run and each subject. The output is then copied to the Arduino code of Step 5.
