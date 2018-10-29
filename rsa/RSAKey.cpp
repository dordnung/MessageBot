/**
 * -----------------------------------------------------
 * File         RSAKey.cpp
 * Authors      David Ordnung, Impact
 * License      GPLv3
 * Web          http://dordnung.de, http://gugyclan.eu
 * -----------------------------------------------------
 *
 * Originally provided for CallAdmin by David Ordnung and Impact
 *
 * Copyright (C) 2014-2018 David Ordnung, Impact
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

#include "RSAKey.h"
#include "SecureRandom.h"
#include "3rdparty/base64/base64.h"

#include <algorithm>


RSAKey::RSAKey(std::string N, std::string E) {
    n = BigUnsignedInABase(N, 16);
    e = BigUnsignedInABase(E, 16);
}

BigUnsigned *RSAKey::DoPublic(BigUnsigned *x) {
    return new BigUnsigned(modexp(*x, e, n));
}

std::string RSAKey::Encrypt(std::string text) {
    BigUnsigned *m = pkcs1pad2(text, (n.bitLength() + 7) >> 3);
    if (!m) {
        return nullptr;
    }

    BigUnsigned *c = DoPublic(m);
    if (!c) {
        delete m;
        return nullptr;
    }

    std::string hex = HexDecode(BigUnsignedInABase(*c, 16));

    delete c;
    delete m;

    return base64_encode(reinterpret_cast<const unsigned char*>(hex.c_str()), hex.length());
}

BigUnsigned *RSAKey::pkcs1pad2(std::string s, int num) {
    int temp = num;

    if ((size_t)num < s.length() + 11) {
        return nullptr;
    }

    unsigned short *ba = new unsigned short[num];
    int i = s.length() - 1;

    while (i >= 0 && num > 0) {
        ba[--num] = s[i--];
    }

    ba[--num] = 0;

    SecureRandom rng;

    int x[1];
    while (num > 2) {
        x[0] = 0;

        while (x[0] == 0) {
            rng.NextBytes(x, 1);
        }

        ba[--num] = static_cast<unsigned short>(x[0]);
    }


    ba[--num] = 2;
    ba[--num] = 0;

    ReverseArray(ba, temp);

    return new BigUnsigned(BigUnsignedInABase(ba, temp - 1, 256));
}

std::string RSAKey::HexDecode(std::string input) {
    static const char *const lut = "0123456789ABCDEF";
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

        output.push_back(static_cast<char>(((p - lut) << 4) | (q - lut)));
    }

    return output;
}

void RSAKey::ReverseArray(unsigned short *a, int len) {
    int temp, i;

    for (i = 0; i < len / 2; ++i) {
        temp = a[len - i - 1];
        a[len - i - 1] = a[i];
        a[i] = static_cast<unsigned short>(temp);
    }
}