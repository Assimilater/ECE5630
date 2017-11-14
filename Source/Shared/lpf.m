fid = fopen('lpf_scaled.bin', 'wb');
fwrite(fid, size(lpf, 2), 'int');
fwrite(fid, lpf, 'float');
fclose(fid);

fid = fopen('digInterp.bin');
y = fread(fid, 'float');
fclose(fid);
