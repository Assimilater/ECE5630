#include <iostream>
#include <fstream>
#include "conv.hpp"
#include "signal.hpp"
#include "PolyFilter.hpp"

class Resampler {
private:
	int U, D, N;
	Signal<float>* h;
	float* buff;

	std::ofstream* fout;

	int buff_i, y_i;

public:
	Resampler(int up, int down, Signal<float>* filter, std::ofstream* file) {
		U = up;
		D = down;
		h = filter;
		N = h->N();
		buff = new float[N]();
		buff_i = y_i = 0;
		fout = file;
	}
	~Resampler() {
		delete[] buff;
	}

	Resampler(const Resampler& rhs) = delete;
	Resampler& operator=(Resampler const& rhs) = delete;

	void feed(float xn) {
		// Convolve xn
		for (int i = 0; i < N; ++i) {
			buff[(buff_i + i) % N] += xn * h->Get(i);
		}

		// Upsample (buff_i increments to simulate inserting 0's)
		int y_f = buff_i + U;
		buff_i = y_f % N;

		// Downsample
		while (y_i < y_f) {
			// Output buff[y_i % N]
			float y_o = buff[y_i % N];
			fout->write((char*)(&y_o), sizeof(float));

			for (int d = 0; d < D; ++d) {
				buff[(y_i + d) % N] = 0;
			}

			// Discard D-1 values
			y_i += D;
		}
		// Done here so overflow doesn't prevent us from knowing when to stop
		y_i %= N;
	}
};

#define INTERP_UP       3
#define INTERP_DOWN     2
#define BUFFER_SIZE     1024

int main() {
	int err;
	Signal<float>* h = nullptr;

	err = OpenBin("lpf_wide.bin", &h);
	if (err != 0) {
		return err;
	}
	if (h == nullptr) {
		std::cout << "lpf not parsed correctly" << std::endl;
		return -1;
	}

	std::ifstream fin("ghostbustersray.bin", std::ios::binary | std::ios::in);
	std::ofstream fDigOut("digInterp.bin", std::ios::binary | std::ios::out);
	//std::ofstream fPolOut("polInterp.bin", std::ios::binary | std::ios::out);

	Resampler  DigInterp(INTERP_UP, INTERP_DOWN, h, &fDigOut);
	//PolyFilter PolInterp(INTERP_UP, INTERP_DOWN, h);

	int N = 0;
	fin.read((char*)&N, sizeof(int));
	float* x = new float[N];
	fin.read((char*)x, sizeof(float) * N);

	int L = (N * INTERP_UP) / INTERP_DOWN;
	//fDigOut.write((char*)&L, sizeof(int));
	//fPolOut.write((char*)&L, sizeof(int));

	for (int i = 0; i < N; ++i) {
		DigInterp.feed(x[i]);
	}

	fin.close();
	fDigOut.close();
	//fPolOut.close();

	delete h;
	return 0;
}
