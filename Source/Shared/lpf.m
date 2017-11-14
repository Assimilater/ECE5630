fid = fopen('lpf.bin', 'wb');
fwrite(fid, size(lpf, 2), 'int');
fwrite(fid, lpf, 'float');
fclose(fid);
