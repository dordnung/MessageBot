/**
* -----------------------------------------------------
* File			rsa.cpp
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

#include "rsa.h"


Arcfour::Arcfour(uint64_t *key, int len) {
	for (int k = 0; k < 256; ++k) {
		S[k] = k;
	}

	int l = 0;

	for (int k = 0; k < 256; ++k) {
		l = (l + S[k] + key[k % len]) & 255;
		int t = S[k];

		S[k] = S[l];
		S[l] = t;
	}

	i = 0;
	j = 0;
}


int Arcfour::next() {
	i = (i + 1) & 255;
	j = (j + S[i]) & 255;
	int t = S[i];

	S[i] = S[j];
	S[j] = t;

	return S[(t + S[i]) & 255];
}


SecureRandom::SecureRandom() : psize(256) {
	pptr = 0;
	state = NULL;

	while (pptr < psize) {
		srand(rand() + (unsigned int)std::time(0));
		int rand1 = rand();

		srand(rand() + (unsigned int)std::time(0));
		int rand2 = rand();

		uint64_t t = (uint64_t)floor((rand1 * rand2) % 65537);

		pool[pptr++] = urs(t, 8);
		pool[pptr++] = t & 255;

	}

	pptr = 0;
	seed_time();
}


void SecureRandom::seed_int(uint64_t x) {
	pool[pptr++] ^= x & 255;
	pool[pptr++] ^= (x >> 8) & 255;
	pool[pptr++] ^= (x >> 16) & 255;
	pool[pptr++] ^= (x >> 24) & 255;

	if (pptr >= psize) {
		pptr -= psize;
	}
}


void SecureRandom::seed_time() {
	seed_int(1122926989487);
}


uint64_t SecureRandom::urs(uint64_t a, uint64_t b) {
	a &= 0xffffffff;
	b &= 0x1f;
	if (a & 0x80000000 && b > 0) {
		a = (a >> 1) & 0x7fffffff;
		a = a >> (b - 1);
	} else {
		a = (a >> b);
	}

	return a;
}


int SecureRandom::get_byte() {
	if (state == NULL) {
		seed_time();

		state = new Arcfour(pool, psize);

		for (pptr = 0; pptr < psize; ++pptr) {
			pool[pptr] = 0;
		}

		pptr = 0;
	}

	return state->next();
}


void SecureRandom::nextBytes(int *ba, int len) {
	for (int i = 0; i < len; ++i) {
		ba[i] = get_byte();
	}
}


RSAKey::RSAKey(const char* N, const char* E) {
	n = BigUnsignedInABase(N, 16);
	e = BigUnsignedInABase(E, 16);
}


BigUnsigned* RSAKey::doPublic(BigUnsigned* x) {
	return new BigUnsigned(modexp(*x, e, n));
}


// Encrypt a text
std::string RSAKey::encrypt(const char* text) {
	BigUnsigned *m = pkcs1pad2(text, (n.bitLength() + 7) >> 3);

	if (m == NULL) {
		return NULL;
	}

	BigUnsigned *c = doPublic(m);

	if (c == NULL) {
		delete m;

		return NULL;
	}


	std::string h = hexDecode(BigUnsignedInABase(*c, 16));

	delete m;
	delete c;

	std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(h.c_str()), h.length());

	return encoded;
}


// Encrypt using pkcs1 padding 2
BigUnsigned* RSAKey::pkcs1pad2(std::string s, int n) {
	int temp = n;

	if ((unsigned int)n < s.length() + 11) {
		return NULL;
	}

	unsigned short *ba = new unsigned short[n];
	int i = s.length() - 1;

	while (i >= 0 && n > 0) {
		ba[--n] = s[i--];
	}

	ba[--n] = 0;


	SecureRandom rng;

	int x[1];

	while (n > 2) {
		x[0] = 0;

		while (x[0] == 0) {
			rng.nextBytes(x, 1);
		}

		ba[--n] = x[0];
	}


	ba[--n] = 2;
	ba[--n] = 0;

	reverseArray(ba, temp);

	return new BigUnsigned(BigUnsignedInABase(ba, temp - 1, 256));
}


// Decodes a hex. String
std::string RSAKey::hexDecode(const std::string& input) {
	static const char* const lut = "0123456789ABCDEF";
	size_t len = input.length();

	if (len & 1) {
		return "";
	}

	std::string output;
	output.reserve(len / 2);

	for (size_t i = 0; i < len; i += 2) {
		char a = input[i];
		const char* p = std::lower_bound(lut, lut + 16, a);

		if (*p != a) {
			return "";
		}

		char b = input[i + 1];
		const char* q = std::lower_bound(lut, lut + 16, b);

		if (*q != b) {
			return "";
		}

		output.push_back(((p - lut) << 4) | (q - lut));
	}

	return output;
}


// Reverses an int array
void RSAKey::reverseArray(unsigned short *a, int len) {
	int temp, i;

	for (i = 0; i < len / 2; ++i) {
		temp = a[len - i - 1];
		a[len - i - 1] = a[i];
		a[i] = temp;
	}
}


// Encrypt RSA with mod, exp and a password
std::string encrypt(const char* mod, const char* exp, const char* pw) {
	RSAKey rsa(mod, exp);

	return rsa.encrypt(pw);
}