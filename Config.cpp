/**
 * -----------------------------------------------------
 * File         Config.cpp
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

#include "Config.h"

#define DEFAULT_WAIT_TIME_BETWEEN_MESSAGES 2000
#define DEFAULT_WAIT_TIME_AFTER_LOGOUT 5000
#define DEFAULT_REQUEST_TIMEOUT 30

// Global variable for accessing config
Config messageBotConfig;

Config::Config() :
    waitBetweenMessages(DEFAULT_WAIT_TIME_BETWEEN_MESSAGES), waitAfterLogout(DEFAULT_WAIT_TIME_AFTER_LOGOUT),
    requestTimeout(DEFAULT_REQUEST_TIMEOUT), debugEnabled(false), shuffleRecipients(false) {}

void Config::ResetConfig() {
    this->username = std::string();
    this->password = std::string();
    this->waitBetweenMessages = DEFAULT_WAIT_TIME_BETWEEN_MESSAGES;
    this->waitAfterLogout = DEFAULT_WAIT_TIME_AFTER_LOGOUT;
    this->requestTimeout = DEFAULT_REQUEST_TIMEOUT;
    this->recipients.clear();
    this->debugEnabled = false;
    this->shuffleRecipients = false;
}