function [x, Fs] = audio2bin(fin, fout)
    % Construct output file name from input
    if (nargin < 2)
        fout = [fin(1:max(strfind(fin, '.'))), 'bin'];
    end
    
    % Path correction
    fin = [pwd,'\',fin];
    fout = [pwd,'\',fout];
    fprintf('Input file = %s\n', fin);
    fprintf('Output file = %s\n', fout);
    
    % Read the audio file
    [x, Fs] = audioread(fin);
    fid = fopen(fout, 'wb');
    
    % Determine data dimensions
    channels = size(x, 2); % columns in x (size of second dimension)
    samples = size(x, 1); % rows in x
    
    % Write the header
    % nchan = channels
    % nsamp = samples
    % sfreq = Fs
    fwrite(fid, samples, 'int');
    
    % Arrange channels by row so the (:) will coalesce correctly
    
    % Output the data
    %fwrite(fid, x.'(:), 'float');
    
    % Alternatively, use a for loop (it may not be efficient, but it matches bin2audio)
    for i = 1:samples
        for j = 1:channels
            fwrite(fid, x(i, j), 'float');
        end
    end
    
    % Release the file handler
    fclose(fid);
end
