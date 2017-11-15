#include <iostream>
#include <fstream>
#include "signal.hpp"

class DigiResampler {
private:
	int U, D, N;
	Signal<float>* h;
	float* buff;

	std::ofstream* fout;

	int buff_i, y_i;

public:
	DigiResampler(int up, int down, Signal<float>* filter, std::ofstream* file) {
		U = up;
		D = down;
		h = filter;
		N = h->N();
		buff = new float[N]();
		buff_i = y_i = 0;
		fout = file;
	}
	~DigiResampler() {
		delete[] buff;
	}

	DigiResampler(const DigiResampler& rhs) = delete;
	DigiResampler& operator=(DigiResampler const& rhs) = delete;

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

class PolyResampler {
private:
	struct PolyFilter {
		float* Filter;
		float* Buffer;
	};
	PolyFilter* R;
	int U, D, Rn;
	int Ri;

	std::ofstream* fout;

	inline PolyFilter* GetFilter(int u, int d) {
		if (u < 0 || d < 0 || u < U || d < D) {
			return nullptr;
		}
		return &R[u*D + d];
	}

	PolyResampler(const PolyResampler& rhs) = delete;
	PolyResampler& operator=(PolyResampler const& rhs) = delete;

public:
	~PolyResampler() {
		for (int u = 0; u < U; ++u) {
			for (int d = 0; d < D; ++d) {
				PolyFilter* Rud = &R[u*D + d];
				delete[] Rud->Buffer;
				delete[] Rud->Filter;
			}
		}
		delete[] R;
	}

	// In this case, I believe (naive approach) it will be more cache efficient to have each Rud filter be contiguous
	PolyResampler(int up, int down, Signal<float>* filter, std::ofstream* file) {
		fout = file;
		U = up;
		D = down;
		Rn = filter->N() / 6;
		R = new PolyFilter[U * D];
		for (int u = 0; u < U; ++u) {
			for (int d = 0; d < D; ++d) {
				PolyFilter* Rud = &R[u*D + d];
				Rud->Buffer = new float[Rn]();
				Rud->Filter = new float[Rn];

				// Generate the smaller filters
				for (int n = 0; n < Rn; ++n) {
					Rud->Filter[n] = filter->Get((n * U * D) + (u * D) + d);
				}
			}
		}
		Ri = 0;
	}

	void feed(float xn) {
		int d = Ri % D;

		// Feed xn to all R-filters at the d-offset
		for (int u = 0; u < U; ++u) {
			PolyFilter* Rud = &R[u*D + d];

			// Convolve xn
			for (int n = 0; n < Rn; ++n) {
				Rud->Buffer[(Ri + n) % Rn] += xn * Rud->Filter[n];
			}
		}

		int nextRi = (Ri + 1) % Rn;
		d = nextRi % D;

		// After x[0], x[1], ..., x[D] has been feed through
		if (d == 0) {
			// Output y[0], y[1], ..., y[U]
			for (int u = 0; u < U; ++u) {
				float Run = 0;
				for (int d = 0; d < D; ++d) {
					PolyFilter* Rud = &R[u*D + d];
					Run += Rud->Buffer[Ri];
					Rud->Buffer[Ri] = 0;
				}
				fout->write((char*)(&Run), sizeof(float));
			}
		}

		Ri = nextRi;
	}

};

#define INTERP_UP       3
#define INTERP_DOWN     2

int main() {
	int err;
	Signal<float>* h = nullptr;

	err = OpenBin("lpf_scaled.bin", &h);
	if (err != 0) {
		return err;
	}
	if (h == nullptr) {
		std::cout << "lpf not parsed correctly" << std::endl;
		return -1;
	}

	std::ifstream fin("ghostbustersray.bin", std::ios::binary | std::ios::in);
	std::ofstream fDigOut("digInterp.bin", std::ios::binary | std::ios::out);
	std::ofstream fPolOut("polInterp.bin", std::ios::binary | std::ios::out);

	DigiResampler DigInterp(INTERP_UP, INTERP_DOWN, h, &fDigOut);
	PolyResampler PolInterp(INTERP_UP, INTERP_DOWN, h, &fPolOut);

	int N = 0;
	fin.read((char*)&N, sizeof(int));
	float* x = new float[N];
	fin.read((char*)x, sizeof(float) * N);

	int L = (N * INTERP_UP) / INTERP_DOWN;
	//fDigOut.write((char*)&L, sizeof(int));
	//fPolOut.write((char*)&L, sizeof(int));

	for (int i = 0; i < N; ++i) {
		DigInterp.feed(x[i]);
		PolInterp.feed(x[i]);
	}

	fin.close();
	fDigOut.close();
	//fPolOut.close();

	delete h;
	return 0;
}
