%% MAIN.m v1.0

clear,clc 
KbName('UnifyKeyNames');

%% Settings
TRIGGER = false;

%% Create an Arduino Connection
% Create an Arduino connection using the specified device and connection parameters.
arduinoObj = arduino('com8','uno'); % MRI STIM PC - if not working, check device manager for the COM port of arduino

%% Configure Arduino Pins as digital outs

TriggerPin = 2;
StopPin = 3;

configurePin(arduinoObj, sprintf('D%i',TriggerPin), 'DigitalOutput');
configurePin(arduinoObj, sprintf('D%i',StopPin), 'DigitalOutput');

%% Open trigger COM port

if TRIGGER
    syncbox_handle = IOPort('OpenSerialPort', 'COM5', 'BaudRate=57600 DataBits=8 Parity=None StopBits=1 FlowControl=None');
    IOPort('Purge',syncbox_handle); % Force clean data
end

%% Wait for trigger

% -- Wait for Key Press or Trigger
if TRIGGER
    disp('Waiting for trigger...')

    [gotTrigger, timeStamp] = waitForTrigger(syncbox_handle,1,300); % timeOut = 5 min (300s)
    if gotTrigger
        
        % write 5V to TriggerPin
        fastwriteDigitalPin(arduinoObj, sprintf('D%i', TriggerPin ), 1)
    
        disp('[runSampleTask] Trigger Received. Start stimulation! To interrupt press S!')
        IOPort('Purge', syncbox_handle);
        pause(1);
        
        % write 0V to TriggerPin
        fastwriteDigitalPin(arduinoObj, sprintf('D%i', TriggerPin ), 0)
        
    else
        error('[runSampleTask] Trigger not received before timeout.');
        IOPort('Close',syncbox_handle);
    end
else
    disp('Waiting for keyboard input...')
    KbPressWait;
    
    % write 5V to TriggerPin
    fastwriteDigitalPin(arduinoObj, sprintf('D%i', TriggerPin ), 1)
    
     disp('[runSampleTask] Trigger Received. Start stimulation! To interrupt press S!')
        pause(1);
        
        % write 0V to TriggerPin
        fastwriteDigitalPin(arduinoObj, sprintf('D%i', TriggerPin ), 0)

end


%% Start recording keyboard input

REC = true;

while REC

    % -------- STOP KEY --------
    [~,~,keyCode] = KbCheck();
    if keyCode(KbName('s')) == 1 % Quit if "S" is pressed
       fastwriteDigitalPin(arduinoObj, sprintf('D%i', StopPin ), 1)
       pause(15);
       fastwriteDigitalPin(arduinoObj, sprintf('D%i', StopPin ), 0)
       REC = false; 
    end
    
    % -------- END KEY --------
    if keyCode(KbName('e')) == 1 % Quit if "E    " is pressed
        break;
    end

end

%% Close COM port
if TRIGGER
    IOPort('Close',syncbox_handle);
end

