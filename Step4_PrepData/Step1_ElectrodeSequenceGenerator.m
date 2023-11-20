done = false;

while ~done
    
    seq_active = randperm(20); %generate a seq of 20 integers without repetition
    seq_active = [seq_active seq_active]; %duplicate
    timeout = 0; %number of try
    
    while timeout < 10
        
        dd = diff(seq_active); %find diff between i and i+1don
        
        if any(dd == 1 | dd == -1) %if consecutive numbers
            
            idx = find(dd == 1 | dd == -1); %find those indexs
            
            for jj = 1:length(idx)
                
                try
                    seq_active([idx(jj) idx(jj)+3]) = seq_active([idx(jj)+3 idx(jj)]); % replace/shuffle
                catch
                    seq_active([idx(jj) idx(jj)-3]) = seq_active([idx(jj)-3 idx(jj)]); %or replace/shuffle 
                end
                
            end
            
        else
            done = true;
        end
        
        timeout = timeout+1;
        
    end
    
end

%seq_active=seq_active'


for i=1:length(seq_active)

    seq_ground(i) = seq_active(i) + 1;

    if seq_active(i) == 20
        seq_ground(i) = seq_active(i) - 1;
    end
end

%seq_ground = seq_ground'

%%
m = histcounts(seq_active);

dd1 = diff(seq_active);
idx1 = find(dd == 1 | dd == -1);

if isempty(idx1)
    disp('Valid sequence of electrodes - seq_active and seq_ground.')
else
    error('Run the code again.')
end
