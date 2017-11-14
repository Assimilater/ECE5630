#include <iostream>
#include "conv.hpp"
#include "signal.hpp"
#include "PolyFilter.hpp"

void Resample(int U, int D, Signal<float>* h) {

}

void FilterPoly(Signal<float>* h) {
	PolyFilter interprolater(3, 2, h);

}

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

	Resample(3, 2, h);
	FilterPoly(h);

	delete h;
	return 0;
}
