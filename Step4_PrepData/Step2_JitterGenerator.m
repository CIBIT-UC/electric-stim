%Jitter Generator

% Set parameters
sequenceLength = 39;
possibleValues = [6, 8, 10];
targetMean = 8;

% Generate random sequence
randomSequence = randi(length(possibleValues), 1, sequenceLength);
sequenceValues = possibleValues(randomSequence);

% Adjust the sequence to have mean=8
currentMean = mean(sequenceValues);

while abs(currentMean - targetMean) > 0.01
    % Randomly choose an index to change
    indexToChange = randi(sequenceLength);

    % Randomly choose a new value from possible values
    newValue = possibleValues(randi(length(possibleValues)));

    % Update the sequence
    sequenceValues(indexToChange) = newValue;

    % Update the mean
    currentMean = mean(sequenceValues);
end

% Count 
countOf10 = sum(sequenceValues == 10);
countOf8 = sum(sequenceValues == 8);
countOf6 = sum(sequenceValues == 6);

% Display the generated sequence and its mean
disp('Generated Sequence:');
disp(sequenceValues);
disp(['Mean of the Sequence: ' num2str(currentMean)]);