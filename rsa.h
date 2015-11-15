/**
* -----------------------------------------------------
* File			rsa.h
* Authors		David <popoklopsi> Ordnung, Impact
* License		GPLv3
* Web			http://popoklopsi.de, http://gugyclan.eu
* -----------------------------------------------------
*
* Originally provided for CallAdmin by Popoklopsi and Impact
*
* Copyright (C) 2014-2015 David <popoklopsi> Ordnung, Impact
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>
*/

#ifndef _INCLUDE_RSA_H_
#define _INCLUDE_RSA_H_


#include <algorithm>
#include <ctime>
#include <string.h>
#include <math.h>
#include <stdint.h>

#include "bigint/BigIntegerLibrary.hh"
#include "base64.h"


class Arcfour {
private:
	int i;
	int j;
	int S[256];

public:
	Arcfour(uint64_t *key, int len);

	int next();
};


class SecureRandom {
private:
	Arcfour *state;
	int psize;
	uint64_t pool[256];
	int pptr;

public:
	SecureRandom();

	void seed_int(uint64_t x);
	void seed_time();

	uint64_t urs(uint64_t a, uint64_t b);

	int get_byte();
	void nextBytes(int *ba, int len);
};


class RSAKey {
private:
	BigUnsigned n;
	BigUnsigned e;

public:
	RSAKey(const char* N, const char* E);

	BigUnsigned* doPublic(BigUnsigned* x);

	// Encrypt a text
	std::string encrypt(const char* text);

	// Encrypt using pkcs1 padding 2
	BigUnsigned* pkcs1pad2(std::string s, int n);

	// Decodes a hex. String
	std::string hexDecode(const std::string& input);

	// Reverses an int array
	void reverseArray(unsigned short *a, int len);
};


// Encrypt RSA with mod, exp and a password
std::string encrypt(const char* mod, const char* exp, const char* pw);


#endif