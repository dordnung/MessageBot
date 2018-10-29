/**
 * -----------------------------------------------------
 * File         Config.h
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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <vector>

/**
 * Config class for different stuff.
 * Class with public members, as simple setters are not meaningful.
 */
class Config {
public:
    std::string username;
    std::string password;

    int waitBetweenMessages;
    int waitAfterLogout;
    int requestTimeout;

    std::vector<uint64_t> recipients;
    bool debugEnabled;

public:
    Config();

    void ResetConfig();
};

extern Config messageBotConfig;

#endif
