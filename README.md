# Development and assessment of a new multichannel electrocutaneous device for non-invasive somatosensory stimulation for magnetic resonance applications
Repository related to the development of the somatosensory electrocutaneous stimulation device to be used in the MR environment.

**Pre-print:** [here](https://www.biorxiv.org/content/10.1101/2024.05.27.595320v1)

### Authors:
Carolina Travassos, Alexandre Sayal, Paulo Fonte, Nuno Carolino, Bruno Direito, Luis Lopes, Sónia Afonso, Tania Lopes, Teresa Sousa, Miguel Castelo-Branco

### Abstract
Electrocutaneous stimulation (ES) relies on the application of an electrical current flowing through the surface of the skin, eliciting a tactile percept. It can be applied for somatosensory mapping approaches at functional magnetic resonance imaging (fMRI) to obtain somatotopic maps illustrating the spatial patterns reflecting the functional organization of the primary somatosensory cortex (S1). However, its accessibility remains constrained, particularly in applications requiring multiple stimulation channels. Furthermore, the magnetic resonance (MR) environment poses several limitations in this regard. This study presents a prototype of a multichannel electrocutaneous stimulation device designed for somatosensory stimulation of the upper limbs of human participants in an MR environment in an inexpensive, safe, customizable, controlled, reproducible, and automated way. Our current-controlled, voltage-limited, stimulation device comprises 20 stimulation channels that can be individually configured to deliver various non-simultaneous combinations of personalized electrical pulses, depending on the subject, stimulation site, and stimulation paradigm. It can deliver a predefined electrical stimulus during fMRI acquisition, synchronized with the stimulation task design and triggered upon initiation of the acquisition sequence. Regarding device assessment, we conducted tests using an electrical circuit equivalent to the impedance of the human body and the electrode-skin interface to validate its feasibility. Then, we evaluated user acceptability by testing the device in human participants. Considering the stringent conditions of the MR environment, we performed a comprehensive set of safety and compatibility evaluations using a phantom. Lastly, we acquired structural and functional MR data from a participant during a somatosensory stimulation experiment to validate brain activity elicited by electric stimulation with our device. These assessments confirmed the device’s safety in fMRI studies and its ability to elicit brain activity in the expected brain areas. The scope of application of our device includes fMRI studies focused on somatosensory mapping and brain-computer interfaces related to somatosensory feedback.

## **This repository includes:**

- Software to control the electrocutaneous stimulation device for S1 Mapping and Threshold Definition;

- Software for data processing and analysis.

# Folders:
- `Step1_ThresholdDefinition`: Code to define the threshold of the stimulation device.
- `Step2_ThresholdUpdate`: Code to update the current thresholds inside the MRI scanner.
- `Step3_ElectrodesCheck`: Code to verify the voltage thresholds inside the MRI scanner.
- `Step4_PrepData`: Code to prepare the data for the main stimulation task (step 5).
- `Step5_StimulationTask`: Code to run the main stimulation task.
- `Other`: miscellaneous code.

# Requirements:
- MATLAB 2019b or later
- Psychtoolbox 3.0.17 or later
- MATLAB Support Package for Arduino Hardware
- Arduino IDE 1.8.19 or later
- Picoscope 7

**2024**
