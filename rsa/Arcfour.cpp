/**
 * -----------------------------------------------------
 * File         Arcfour.cpp
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

#include "Arcfour.h"


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

int Arcfour::Next() {
    i = (i + 1) & 255;
    j = (j + S[i]) & 255;
    int t = S[i];

    S[i] = S[j];
    S[j] = t;

    return S[(t + S[i]) & 255];
}