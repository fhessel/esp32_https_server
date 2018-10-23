#include "util.hpp"

namespace httpsserver {

int parseInt(std::string s) {
	int i = 0; // value
	int m = 1; // multiplier

	// Check sign
	size_t x = 0;
	if (s[0]=='-') {

		x = 1;
	} else if (s[0]=='+') {
		x = 1;
	}

	// Convert by base 10
	for(; x < s.size(); x++) {
		char c = s[x];
		if (c >= '0' && c<='9') {
			i = i*10 + (c-'0');
		} else {
			break;
		}
	}

	// Combine both.
	return m*i;
}

std::string intToString(int i) {
	if (i==0) {
		return "0";
	}
	// We need this much digits
	int digits = ceil(log10(i));
	char c[digits+1];
	c[digits] = '\0';

	for(int x = digits-1; x >= 0; x--) {
		char v = (i%10);
		c[x] = '0' + v;
		i = (i-v)/10;
	}

	return std::string(c);
}

static const char kBase64Alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

int base64EncodedLength(size_t length) {
	return (length + 2 - ((length + 2) % 3)) / 3 * 4;
} // base64EncodedLength

int base64EncodedLength(const std::string &in) {
	return base64EncodedLength(in.length());
} // base64EncodedLength

void a3_to_a4(unsigned char * a4, unsigned char * a3) {
	a4[0] = (a3[0] & 0xfc) >> 2;
	a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
	a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
	a4[3] = (a3[2] & 0x3f);
} // a3_to_a4

/**
 * Encode a string into base 64.
 */
bool base64Encode(const std::string &in, std::string *out) {
	int i = 0, j = 0;
	size_t enc_len = 0;
	unsigned char a3[3];
	unsigned char a4[4];

	out->resize(base64EncodedLength(in));

	int input_len = in.size();
	std::string::const_iterator input = in.begin();

	while (input_len--) {
		a3[i++] = *(input++);
		if (i == 3) {
			a3_to_a4(a4, a3);

			for (i = 0; i < 4; i++) {
				(*out)[enc_len++] = kBase64Alphabet[a4[i]];
			}

			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 3; j++) {
			a3[j] = '\0';
		}

		a3_to_a4(a4, a3);

		for (j = 0; j < i + 1; j++) {
			(*out)[enc_len++] = kBase64Alphabet[a4[j]];
		}

		while ((i++ < 3)) {
			(*out)[enc_len++] = '=';
		}
	}

	return (enc_len == out->size());
} // base64Encode

}
