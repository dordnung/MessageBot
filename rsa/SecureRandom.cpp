/**
 * -----------------------------------------------------
 * File         SecureRandom.cpp
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

#include "SecureRandom.h"

#include <algorithm>
#include <ctime>
#include <math.h>


SecureRandom::SecureRandom() : psize(256) {
    pptr = 0;
    state = nullptr;

    while (pptr < psize) {
        srand(rand() + (unsigned int)std::time(0));
        int rand1 = rand();

        srand(rand() + (unsigned int)std::time(0));
        int rand2 = rand();

        uint64_t t = (uint64_t)floor((rand1 * rand2) % 65537);

        pool[pptr++] = Urs(t, 8);
        pool[pptr++] = t & 255;
    }

    pptr = 0;
    SeedTime();
}

SecureRandom::~SecureRandom() {
    if (this->state) {
        delete state;
    }
}

void SecureRandom::SeedInt(uint64_t x) {
    pool[pptr++] ^= x & 255;
    pool[pptr++] ^= (x >> 8) & 255;
    pool[pptr++] ^= (x >> 16) & 255;
    pool[pptr++] ^= (x >> 24) & 255;

    if (pptr >= psize) {
        pptr -= psize;
    }
}

void SecureRandom::SeedTime() {
    SeedInt(1122926989487);
}

uint64_t SecureRandom::Urs(uint64_t a, uint64_t b) {
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

int SecureRandom::GetByte() {
    if (!state) {
        SeedTime();

        state = new Arcfour(pool, psize);

        for (pptr = 0; pptr < psize; ++pptr) {
            pool[pptr] = 0;
        }

        pptr = 0;
    }

    return state->Next();
}

void SecureRandom::NextBytes(int *ba, int len) {
    for (int i = 0; i < len; ++i) {
        ba[i] = GetByte();
    }
}