#pragma once
#include <functional>
#include <string>
#include "types.h"

template <typename T>
class Signal {
private:
	inline void clean() {
		delete[] signal;
		signal = nullptr;
		length = 0;
	}
	inline void copy(const Signal<T>& rhs) {
		length = rhs.length;
		signal = new T[length];
		memcpy(signal, rhs.signal, sizeof(T) * length);
	}

protected:
	int length;
	T* signal;

	Signal() {
		length = 0;
		signal = nullptr;
	}

public:
	friend int OpenWAV(std::string, Signal<byte>**);
	friend int SaveWAV(std::string, Signal<byte>*);

	typedef std::function<void(int, T)> accessor;
	typedef std::function<T(int, T)> mutator;
	typedef std::function<T(int)> setter;

	inline void each(accessor f) const {
		for (int n = 0; n < length; ++n) {
			f(n, signal[n]);
		}
	}
	inline void each(mutator f) {
		for (int n = 0; n < length; ++n) {
			signal[n] = f(n, signal[n]);
		}
	}
	Signal(int l, setter f) {
		length = l;
		signal = new T[length];
		for (int n = 0; n < length; ++n) {
			signal[n] = f(n);
		}
	}

	Signal(int l) {
		length = l;
		signal = new T[length]();
	}
	Signal(int l, T v) {
		length = l;
		signal = new T[length];
		for (int n = 0; n < length; ++n) {
			signal[n++] = v;
		}
	}
	Signal(int l, T* sig) {
		length = l;
		signal = new T[length];
		memcpy(signal, sig, sizeof(T) * length);
	}
	Signal<T>& operator=(const Signal<T>& rhs) { clean(); copy(rhs); return *this; }
	Signal(const Signal<T>& rhs) { copy(rhs); }
	~Signal() { clean(); }

	inline const int N() const { return length; }
	inline const int ConvTailN() const { return length / 2; }

	inline T Get(int n) const {
		if (n < 0 || n >= length || signal == nullptr) { return 0; }
		return signal[n];
	}
	inline void Set(int n, T val) {
		if (n < 0 || n >= length || signal == nullptr) { return; }
		signal[n] = val;
	}
};

#define ERROR_PGM_FILE          (1 << 0)
#define ERROR_PGM_HEADER        (1 << 1)
#define ERROR_PGM_MAXSIZE       (1 << 2)

int OpenWAV(std::string, Signal<byte>**);
int SaveWAV(std::string, Signal<byte>*);
