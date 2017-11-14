#pragma once
#include "signal.hpp"

class PolyFilter {
private:
	struct Rud {
		float* Filter;
		float* Buff;
	};
	Rud** F;
	int U, D, N;

public:
	// In this case, I believe (naive approach) it will be more cache efficient to have each Rdu filter be contiguous
	PolyFilter(int u, int d, Signal<float>* h) {
		U = u;
		D = d;
		N = h->N() / 6;
		F = new Rud*[U];
		for (int i = 0; i < U; ++i) {
			F[i] = new Rud[D];
			Rud* Fi = F[i];
			for (int j = 0; j < D; ++j) {
				Fi[j].Filter = new float[N];
				Fi[j].Buff = new float[N]();
				float* Hij = Fi[j].Filter;
				for (int k = 0; k < N; ++k) {
					Hij[k] = h->Get((k * U * D) + (i * D) + j);
				}
			}
		}
	}
	~PolyFilter() {
		for (int i = 0; i < U; ++i) {
			Rud* Hi = F[i];
			for (int j = 0; j < D; ++j) {
				delete[] Hi[j].Filter;
				delete[] Hi[j].Buff;
			}
			delete[] F[i];
		}
		delete[] F;
	}

	void feed(float xn);

	PolyFilter(const PolyFilter& rhs) = delete;
	PolyFilter& operator=(PolyFilter const& rhs) = delete;

	inline float GetH(int u, int d, int n) {
		if (u < 0 || d < 0 || n < 0 || u < U || d < D || n < N) {
			return 0;
		}
		return F[u][d].Filter[n];
	}
	inline void SetH(int u, int d, int n, float v) {
		if (u < 0 || d < 0 || n < 0 || u < U || d < D || n < N) {
			return;
		}
		F[u][d].Filter[n] = v;
	}

	inline float GetF(int u, int d, int n) {
		if (u < 0 || d < 0 || n < 0 || u < U || d < D || n < N) {
			return 0;
		}
		return F[u][d].Buff[n];
	}
	inline void SetF(int u, int d, int n, float v) {
		if (u < 0 || d < 0 || n < 0 || u < U || d < D || n < N) {
			return;
		}
		F[u][d].Buff[n] = v;
	}
};
