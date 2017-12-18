/*
 * util.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: frank
 */


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


}
