#pragma once
#include <thread>
#include "image.hpp"

// Struct helper for Multithreading in Optimization 2
template<typename T1, typename T2>
class Conv2DPlan {
public:
	Image<T1>* I;
	Image<T2>* F;
	int MI, NI;
	int MF, NF;
	int M, N;
	Image<float>* out;

	Conv2DPlan(Image<T1>* image, Image<T2>* filter) {
		I = image; F = filter;

		MI = I->M();
		NI = I->N();

		MF = F->M();
		NF = F->N();

		M = MI + MF - 1;
		N = NI + NF - 1;

		out = new Image<float>(M, N);
	}
};

// Thread routine for Multithreading in Opimization 2 (compare loop with O1Convolve2D)
template<typename T1, typename T2>
void Conv2DThread(Conv2DPlan<T1, T2>* plan, int n0, int n1) {
	if (n1 > plan->N) { n1 = plan->N; }
	for (int n = n0; n < n1; ++n) {
		for (int m = 0; m < plan->M; ++m) {
			float sum = 0;
			for (int k = 0; k <= n && k < plan->NF; ++k) {
				for (int l = 0; l <= m && l < plan->MF; ++l) {
					sum += (float)plan->F->Get(l, k) * plan->I->Get(m - l, n - k);
				}
			}
			plan->out->Set(m, n, sum);
		}
	}
}

// The number of threads to use for convolution (divided roughly equally, the last one may be slightly less workload)
#define CONV_POOL_SIZE 10

template<typename T1, typename T2>
Image<float>* Conv2D(Image<T1>* image, Image<T2>* filter) {
	Conv2DPlan<T1, T2> plan(image, filter);

	std::thread* pool[CONV_POOL_SIZE];
	int dn = plan.N / 10 + 1;

	for (int i = 0; i < CONV_POOL_SIZE; ++i) {
		int n0 = i * dn;
		int n1 = n0 + dn;
		pool[i] = new std::thread(Conv2DThread<T1, T2>, &plan, n0, n1);
	}

	for (int i = 0; i < CONV_POOL_SIZE; ++i) {
		pool[i]->join();
		delete pool[i];
		pool[i] = nullptr;
	}

	return plan.out;
}

// Don't clutter up the pre-processor defintions, we're done with it
#undef CONV_POOL_SIZE
