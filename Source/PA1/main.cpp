#include <iostream>
#include <thread>
#include "image.hpp"

float H1[] = {
	1 / 81.0f, 2 / 81.0f, 3 / 81.0f, 2 / 81.0f, 1 / 81.0f,
	2 / 81.0f, 4 / 81.0f, 6 / 81.0f, 4 / 81.0f, 2 / 81.0f,
	3 / 81.0f, 6 / 81.0f, 9 / 81.0f, 6 / 81.0f, 3 / 81.0f,
	2 / 81.0f, 4 / 81.0f, 6 / 81.0f, 4 / 81.0f, 2 / 81.0f,
	1 / 81.0f, 2 / 81.0f, 3 / 81.0f, 2 / 81.0f, 1 / 81.0f,
};

float S1[] = {
	1, 0, -1,
	2, 0, -2,
	1, 0, -1,
};

float S2[] = {
	-1, -2, -1,
	 0,  0,  0,
	 1,  2,  1,
};

template<typename T1, typename T2>
Image<float>* Convolve2D(Image<T1>* image, Image<T2>* filter) {
	int M = image->M() + filter->M() - 1;
	int N = image->N() + filter->N() - 1;
	return new Image<float>(M, N, [image, filter](int m, int n) -> float {
		float sum = 0;
		filter->each([image, m, n, &sum](int l, int k, T2 v) -> void {
			sum += (float)v * image->Get(m - l, n - k);
		});
		return sum;
	});
}

template<typename T1, typename T2>
Image<float>* O1Convolve2D(Image<T1>* image, Image<T2>* filter) {
	int MI = image->M();
	int NI = image->N();

	int MF = filter->M();
	int NF = filter->N();

	int M = MI + MF - 1;
	int N = NI + NF - 1;

	Image<float>* output = new Image<float>(M, N);
	for (int n = 0; n < N; ++n) {
		for (int m = 0; m < M; ++m) {
			float sum = 0;
			for (int k = 0; k <= n && k < NF; ++k) {
				for (int l = 0; l <= m && l < MF; ++l) {
					sum += (float)filter->Get(l, k) * image->Get(m - l, n - k);
				}
			}
			output->Set(m, n, sum);
		}
	}
	return output;
}

template<typename T1, typename T2>
class Plan {
public:
	Image<T1>* I;
	Image<T2>* F;
	int MI, NI;
	int MF, NF;
	int M, N;
	Image<float>* out;

	Plan(Image<T1>* image, Image<T2>* filter) {
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

template<typename T1, typename T2>
void FastConvolve2DThread(Plan<T1, T2>* plan, int n0, int n1) {
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

#define POOL_SIZE 10

template<typename T1, typename T2>
Image<float>* FastConvolve2D(Image<T1>* image, Image<T2>* filter) {
	Plan<T1, T2> plan(image, filter);

	std::thread* pool[POOL_SIZE];
	int dn = plan.N / 10 + 1;

	for (int i = 0; i < POOL_SIZE; ++i) {
		int n0 = i * dn;
		int n1 = n0 + dn;
		pool[i] = new std::thread(FastConvolve2DThread<T1, T2>, &plan, n0, n1);
	}

	for (int i = 0; i < POOL_SIZE; ++i) {
		pool[i]->join();
		delete pool[i];
		pool[i] = nullptr;
	}

	return plan.out;
}

int main() {
	int err;

	Image<float>* H1Filter = new Image<float>(5, 5, H1);
	Image<float>* S1Filter = new Image<float>(3, 3, S1);
	Image<float>* S2Filter = new Image<float>(3, 3, S2);

	Image<byte>* image;
	Image<byte>* filter;

	err = OpenPGM("image.pgm", &image);
	if (err != ERROR_NONE) {
		std::cout << "Unable to parse image.pgm! Error Code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}

	err = OpenPGM("filter_final.pgm", &filter);
	if (err != ERROR_NONE) {
		std::cout << "Unable to parse filter_final.pgm! Error Code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}

	//err = SavePGM("test_image.pgm", image);
	//if (err != ERROR_NONE) {
	//	std::cout << "Unable to save test_image.pgm! Error Code: " << err << std::endl;
	//	exit(EXIT_FAILURE);
	//}

	//err = SavePGM("test_filter.pgm", filter);
	//if (err != ERROR_NONE) {
	//	std::cout << "Unable to save test_filter.pgm! Error Code: " << err << std::endl;
	//	exit(EXIT_FAILURE);
	//}

	// Problem 2
	Image<float>* H1F = FastConvolve2D(image, H1Filter);
	Image<byte>* P2 = new Image<byte>(H1F->M(), H1F->N(), [H1F](int m, int n) -> byte {
		float h1 = H1F->Get(m, n);
		if (h1 > 255) { h1 = 255; }
		if (h1 < 0) { h1 = 0; }
		return (byte)h1;
	});
	err = SavePGM("P2.pgm", P2);
	if (err != ERROR_NONE) {
		std::cout << "Unable to save P2.pgm! Error Code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}

	// Problem 3
	Image<float>* G1 = FastConvolve2D(image, S1Filter);
	Image<float>* G2 = FastConvolve2D(image, S2Filter);
	Image<byte>* P3 = new Image<byte>(G1->M(), G1->N(), [G1, G2](int m, int n) -> byte {
		float g1 = G1->Get(m, n);
		float g2 = G2->Get(m, n);
		g1 = g1 < 0 ? g1 * -1 : g1;
		g2 = g2 < 0 ? g2 * -1 : g2;
		float sum = g1 + g2;
		if (sum > 255) { sum = 255; }
		return (byte)sum;
	});
	err = SavePGM("P3.pgm", P3);
	if (err != ERROR_NONE) {
		std::cout << "Unable to save P3.pgm! Error Code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}

	// Problem 4

	// Before you filter, subtract a scalar value from each value in the filter
	// The scalar is the minimim value in the filter
	byte minScalar = 255;
	filter->each([&minScalar](int m, int n, byte v) -> void {
		if (v < minScalar) { minScalar = v; }
	});
	filter->each([minScalar](int m, int n, byte v) -> byte {
		return v - minScalar;
	});

	Image<float>* F1 = FastConvolve2D(image, filter);

	// Scale the filtered image so the maximum value in the image is 255
	// and all negative values after scaling are set to zero (done in ctor() of P4)
	float minF1, maxF1;
	minF1 = maxF1 = F1->Get(0, 0);
	F1->each([&minF1, &maxF1](int m, int n, float v) -> void {
		if (v < minF1) { minF1 = v; }
		if (v > maxF1) { maxF1 = v; }
	});
	F1->each([minF1, maxF1](int m, int n, float v) -> float {
		float scaled = v * 255 / maxF1;
		return scaled;
	});

	Image<byte>* P4 = new Image<byte>(F1->M(), F1->N(), [F1](int m, int n) -> byte {
		float f1 = F1->Get(m, n);
		if (f1 > 255) { f1 = 255; }
		if (f1 < 0) { f1 = 0; }
		return (byte)f1;
	});
	err = SavePGM("P4.pgm", P4);
	if (err != ERROR_NONE) {
		std::cout << "Unable to save P4.pgm! Error Code: " << err << std::endl;
		exit(EXIT_FAILURE);
	}

	delete image;
	delete filter;
	return 0;
}
