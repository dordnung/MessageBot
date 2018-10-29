/**
 * -----------------------------------------------------
 * File         RSAKey.h
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

#ifndef _INCLUDE_RSA_RSA_KEY_H_
#define _INCLUDE_RSA_RSA_KEY_H_

#include "3rdparty/bigint/BigIntegerLibrary.hh"
#include <string>

class RSAKey {
private:
    BigUnsigned n;
    BigUnsigned e;

public:
    RSAKey(std::string N, std::string E);

    // Encrypt a text
    std::string Encrypt(std::string text);

private:
    BigUnsigned *DoPublic(BigUnsigned *x);
    BigUnsigned *pkcs1pad2(std::string s, int num);

    std::string HexDecode(std::string input);

    void ReverseArray(unsigned short *a, int len);
};

#endif