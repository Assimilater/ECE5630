#include "signal.hpp"
#include <fstream>


int OpenBin(std::string file, Signal<float>** iin) {
	std::fstream fin(file, std::ios::binary | std::ios::in);
	if (!fin) {
		return ERROR_BIN_FILE;
	}

	int N;
	fin.read((char*)&N, sizeof(int));
	Signal<float>* in = new Signal<float>(N);
	fin.read((char*)(in->signal), sizeof(float) * N);
	fin.close();
	*iin = in;
	return 0;
}

int SaveBin(std::string file, Signal<float>* out) {
	std::fstream fout(file, std::ios::binary | std::ios::out);
	fout.write((char*)out->length, sizeof(int));
	fout.write((char*)out->signal, out->length * sizeof(float));
	fout.close();
	return 0;
}
