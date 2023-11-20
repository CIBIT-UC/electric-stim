% --------------- Prepare data for Arduino Mapping Code -------------------
% Received stimulation thresholds (currents and voltages) sequentially and
% rearrange it according to the order of the stimulation sequence of each
% run

%% INPUTS
% 1. Stimulation sequences - ACTIVE - from ElectrodeSequenceGenerator.m

% Subject
StimulationSequence_Run1_Active = [17, 12, 14, 8, 3, 13, 6, 2, 7, 15, 10, 3, 20, 8, 17, 1, 19, 7, 13, 6, 18, 5, 9, 16, 4, 1, 20, 9, 4, 11, 2, 15, 10, 19, 5, 14, 11, 16, 12, 18];
StimulationSequence_Run2_Active = [16, 13, 2, 14, 7, 19, 4, 17, 12, 20, 1, 18, 5, 9, 6, 10, 3, 11, 18, 7, 17, 10, 3, 19, 13, 4, 8, 6, 14, 5, 1, 16, 8, 2, 9, 20, 12, 15, 11, 15]; 
StimulationSequence_Run3_Active = [6, 17, 2, 9, 20, 3, 10, 16, 13, 4, 15, 5, 11, 14, 7, 1, 12, 19, 7, 2, 17, 9, 14, 11, 1, 12, 6, 20, 5, 18, 8, 18, 8, 3, 15, 10, 4, 16, 13, 19];
StimulationSequence_Run4_Active = [9, 2, 4, 20, 13, 6, 1, 15, 12, 18, 3, 7, 19, 10, 17, 5, 13, 6, 9, 4, 14, 11, 17, 12, 3, 16, 2, 19, 10, 1, 14, 11, 5, 16, 7, 20, 8, 15, 8, 18]; 

% 2. Thresholds (per electode order) - from "ElectrodesCheck" code
% Subject
Currents = [2.10, 1.95, 3.3, 1.20, 1.35, 2.20, 2.50, 1.80, 1.60, 1.80, 1.35, 1.20, 1.35, 1.45, 1.35, 1.35, 1.10, 1.60, 1.70, 2.10];
Voltages = [55.00, 33.00, 36.00, 25.00, 22.00, 32.00, 35.00, 25.00, 28.00, 25.00, 24.00, 23.00, 20.00, 24.00, 28.00, 26.00, 24.00, 25.00, 26.00, 22.00];


%% SORT ARRAYS
% Currents
Currents_Run1_sorted = Currents(StimulationSequence_Run1_Active);
Currents_Run2_sorted = Currents(StimulationSequence_Run2_Active);
Currents_Run3_sorted = Currents(StimulationSequence_Run3_Active);
Currents_Run4_sorted = Currents(StimulationSequence_Run4_Active);

% Voltages
Voltages_Run1_sorted = Voltages(StimulationSequence_Run1_Active);
Voltages_Run2_sorted = Voltages(StimulationSequence_Run2_Active);
Voltages_Run3_sorted = Voltages(StimulationSequence_Run3_Active);
Voltages_Run4_sorted = Voltages(StimulationSequence_Run4_Active);


%% PREPARE ARRAYS FOR ARDUINO MAPPING CODE
% Currents
Currents_Run1  = sprintf('%.2f, ', Currents_Run1_sorted);
Currents_Run1  = Currents_Run1(1:end-2)

Currents_Run2  = sprintf('%.2f, ', Currents_Run2_sorted);
Currents_Run2  = Currents_Run2(1:end-2)

Currents_Run3  = sprintf('%.2f, ', Currents_Run3_sorted);
Currents_Run3  = Currents_Run3(1:end-2)

Currents_Run4  = sprintf('%.2f, ', Currents_Run4_sorted);
Currents_Run4  = Currents_Run4(1:end-2)

% Voltages
Voltages_Run1  = sprintf('%.0f, ', Voltages_Run1_sorted);
Voltages_Run1  = Voltages_Run1(1:end-2)

Voltages_Run2  = sprintf('%.0f, ', Voltages_Run2_sorted);
Voltages_Run2  = Voltages_Run2(1:end-2)

Voltages_Run3  = sprintf('%.0f, ', Voltages_Run3_sorted);
Voltages_Run3  = Voltages_Run3(1:end-2)

Voltages_Run4  = sprintf('%.0f, ', Voltages_Run4_sorted);
Voltages_Run4  = Voltages_Run4(1:end-2)