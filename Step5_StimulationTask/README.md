# Stimulation Task

## Final_StimulationTask_MRI.ino

Arduino MEGA code for the main stimulation task. This code is uploaded to the Arduino board per run and subject.

## UnoController.m

MATLAB code to control the Arduino Uno board. This code wait for the trigger from the MRI scanner and then send a start signal to the Arduino MEGA board.

### waitforTrigger.m

Function that reads COM port that received the start trigger from the MRI scanner.

### Requirements

This code requires the edit of a function (`writeDigitalPin.m`) from the MATLAB Support Package for Arduino Hardware to add a new function inside it. The new function is called `fastwriteDigitalPin` and it is used to write the digital pin faster than the original function.

```matlab
% Create new function called fastwriteDigitalPin:
function fastwriteDigitalPin(obj, pin, data)
    %   Set or Reset a digital pin.
    %
    %   Syntax:
    %   writeDigitalPin(obj,pin,value)
    %
    %   Description:
    %   Writes specified value to the specified pin on the Arduino hardware.
    %
    %   Input Arguments:
    %   obj   - Low cost hardware object
    %   pin   - Digital pin number on the hardware (character vector or string)
    %   value - Digital value (0, 1) or (true, false) to write to the specified pin (double).
    %
    %   See also readDigitalPin, writePWMVoltage, writePWMDutyCycle

    try
        if (nargin < 3)
            obj.localizedError('MATLAB:minrhs');
        end
        obj.writeDigitalPinHook(pin, data);
    catch e
        throwAsCaller(e);
    end
end
```