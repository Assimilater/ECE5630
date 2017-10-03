#pragma once
#include <functional>
#include <string>
#include "types.h"

template <typename T>
class Image {
private:
	inline void clean() {
		delete[] image;
		image = nullptr;
		width = height = length = 0;
	}
	inline void copy(const Image<T>& rhs) {
		width = rhs.width;
		height = rhs.height;
		length = width * height;
		image = new T[length];
		//memcpy_s(image, sizeof(T) * length, rhs.image, sizeof(T) * length);
		memcpy(image, rhs.image, sizeof(T) * length);
	}

protected:
	int width, height, length;
	T* image;

	Image() {
		width = 0;
		height = 0;
		length = 0;
		image = nullptr;
	}

public:
	friend int OpenPGM(std::string, Image<byte>**);
	friend int SavePGM(std::string, Image<byte>*);

	typedef std::function<void(int, int, T)> accessor;
	typedef std::function<T(int, int, T)> mutator;
	typedef std::function<T(int, int)> setter;

	inline void each(accessor f) const {
		int i = 0;
		for (int n = 0; n < height; ++n) {
			for (int m = 0; m < width; ++m) {
				f(m, n, image[i++]);
			}
		}
	}
	inline void each(mutator f) {
		int i = 0;
		for (int n = 0; n < height; ++n) {
			for (int m = 0; m < width; ++m) {
				image[i] = f(m, n, image[i]);
				++i;
			}
		}
	}
	Image(int w, int h, setter f) {
		width = w;
		height = h;
		length = width * height;
		image = new T[length];
		int i = 0;
		for (int n = 0; n < height; ++n) {
			for (int m = 0; m < width; ++m) {
				image[i++] = f(m, n);
			}
		}
	}

	Image(int w, int h) {
		width = w;
		height = h;
		length = width * height;
		image = new T[length]();
	}
	Image(int w, int h, T v) {
		width = w;
		height = h;
		length = width * height;
		image = new T[length];
		int i = 0;
		for (int n = 0; n < height; ++n) {
			for (int m = 0; m < width; ++m) {
				image[i++] = v;
			}
		}
	}
	Image(int w, int h, T* img) {
		width = w;
		height = h;
		length = width * height;
		image = new T[length];
		//memcpy_s(image, sizeof(T) * length, img, sizeof(T) * length);
		memcpy(image, img, sizeof(T) * length);
	}
	Image<T>& operator=(const Image<T>& rhs) { clean(); copy(rhs); return *this; }
	Image(const Image<T>& rhs) { copy(rhs); }
	~Image() { clean(); }

	inline const int M() const { return width; }
	inline const int N() const { return height; }

	inline T Get(int m, int n) const {
		if (m < 0 || n < 0 || m >= width || n >= height || image == nullptr) { return 0; }
		return image[width*n + m];
	}
	inline void Set(int m, int n, T val) {
		if (m < 0 || n < 0 || m >= width || n >= height || image == nullptr) { return; }
		image[width*n + m] = val;
	}
};

#define ERROR_PGM_FILE          (1 << 0)
#define ERROR_PGM_HEADER        (1 << 1)
#define ERROR_PGM_MAXSIZE       (1 << 2)

int OpenPGM(std::string, Image<byte>**);
int SavePGM(std::string, Image<byte>*);
