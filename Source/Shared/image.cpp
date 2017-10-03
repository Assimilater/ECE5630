#include "image.hpp"
#include <fstream>

enum ParsePGM {
	Width,
	Height,
	MaxVal,
	Data,
};

int OpenPGM(std::string file, Image<byte>** out) {
	std::fstream fin(file, std::ios::binary | std::ios::in);
	if (!fin) {
		return ERROR_PGM_FILE;
	}

	char in;
	bool error = false;

	// Validate the magic numbers
	fin.read(&in, 1);
	if (in == 'P') {
		fin.read(&in, 1);
		if (in == '5') {
			fin.read(&in, 1);
			if (in != '\n') {
				error = true;
			}
		}
		else {
			error = true;
		}
	} else {
		error = true;
	}

	if (error) {
		fin.close();
		return ERROR_PGM_HEADER;
	}

	// Do the actual parsing
	Image<byte>* iin = new Image<byte>();

	ParsePGM state = ParsePGM::Width;
	bool comment = false;
	bool newline = true;
	int maxsize = 0;

	while (true) {
		fin.read(&in, 1);

		if (newline) {
			newline = false;
			if (in == '#') {
				comment = true;
				continue;
			}
		}

		if (comment) {
			if (in == '\n') {
				comment = false;
				newline = true;
			}
			continue;
		}

		switch (state) {
			case ParsePGM::Width:
				if (in == ' ' || in == '\n' || in == '\t' || in == 0) {
					state = ParsePGM::Height;
					break;
				}
				if (in < '0' || in > '9') {
					delete iin;
					fin.close();
					return ERROR_PGM_HEADER;
				}
				iin->width *= 10;
				iin->width += in - '0';
				break;

			case ParsePGM::Height:
				if (in == ' ' || in == '\n' || in == '\t' || in == 0) {
					state = ParsePGM::MaxVal;
					break;
				}
				if (in < '0' || in > '9') {
					delete iin;
					fin.close();
					return ERROR_PGM_HEADER;
				}
				iin->height *= 10;
				iin->height += in - '0';
				break;

			case ParsePGM::MaxVal:
				if (in == ' ' || in == '\n' || in == '\t' || in == 0) {
					state = ParsePGM::Data;
					if (maxsize != 255) {
						delete iin;
						fin.close();
						return ERROR_PGM_HEADER | ERROR_PGM_MAXSIZE;
					}

					iin->length = iin->width * iin->height;
					iin->image = new byte[iin->length];
					break;
				}
				if (in < '0' || in > '9') {
					delete iin;
					fin.close();
					return ERROR_PGM_HEADER;
				}
				maxsize *= 10;
				maxsize += in - '0';
				break;
		}

		if (state == ParsePGM::Data) { break; }
	}

	fin.read((char*)iin->image, iin->length);
	fin.close();

	*out = iin;
	return 0;
}

int SavePGM(std::string file, Image<byte>* in) {
	std::fstream fout(file, std::ios::binary | std::ios::out);
	fout << "P5\n" << in->width << ' ' << in->height << " 255\n";
	fout.write((char*)in->image, in->length);
	fout.close();
	return 0;
}
