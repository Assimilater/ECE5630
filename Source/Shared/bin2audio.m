function [x, Fs] = bin2audio(fin, fout)
    % Construct output file name from input
    if (nargin < 2)
        fout = [fin(1:max(strfind(fin, '.'))), 'wav'];
    end
    
    % Path correction
    fin = [pwd,'\output\',fin];
    fout = [pwd,'\output\',fout];
    fprintf('Input file = %s\n', fin);
    fprintf('Output file = %s\n', fout);
    
    % Read the header
    % nchan = channels
    % nsamp = samples
    % sfreq = Fs
    fid = fopen(fin, 'rb');
    channels = fread(fid, 1, 'int');
    samples = fread(fid, 1, 'int');
    Fs = fread(fid, 1, 'int');
    
    % Read the data
    [a, ~] = fread(fid, inf, 'float');
    size(a, 1)
    
    % Release the file handler
    fclose(fid);
    
    % Just use nested loops this time
    x = zeros(samples, channels);
    for i = 1:samples
        for j = 1:channels
            % i-1 because matlab is stupid with 1-based indexing
            x(i, j) = a((i - 1)*channels + j);
        end
    end
    
    % Write out the data
    audiowrite(fout, x, Fs);
end
