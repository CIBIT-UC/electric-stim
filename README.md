# electric-stim
Code that supports the Electric stimulation device developed at CIBIT.

Device development and assessment are described in detail in https://doi.org/10.1101/2024.05.27.595320

## Folders:
- `Step1_ThresholdDefinition`: Code to define the threshold of the stimulation device.
- `Step2_ThresholdUpdate`: Code to update the current thresholds inside the MRI scanner.
- `Step3_ElectrodesCheck`: Code to verify the voltage thresholds inside the MRI scanner.
- `Step4_PrepData`: Code to prepare the data for the main stimulation task (step 5).
- `Step5_StimulationTask`: Code to run the main stimulation task.
- `Other`: miscellaneous code.


## Requirements:
- MATLAB 2019b or later
- Psychtoolbox 3.0.17 or later
- MATLAB Support Package for Arduino Hardware
- Arduino IDE 1.8.19 or later
- Picoscope 7
