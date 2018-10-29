/**
 * -----------------------------------------------------
 * File			tester.cpp
 * Authors		David Ordnung, Impact
 * License		GPLv3
 * Web			http://dordnung.de, http://gugyclan.eu
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

#include <stdio.h>
#include <list>

#include "WebAPI.h"

int main(int argc, const char* argv[]) {
    // ensure the correct number of parameters are used.
    if (argc == 5) {
        Message message;
        message.config.debugEnabled = true;
        message.config.waitBetweenMessages = 2000;
        message.config.waitAfterLogout = 5000;

        message.config.username = argv[1];
        message.config.password = argv[2];

        uint64_t steamId64 = strtoull(argv[4], NULL, 10);
        message.config.recipients.push_back(steamId64);

        message.text = argv[3];

        // Send the message
        WebAPI webApi;
        webApi.SendSteamMessage(message);
    } else {
        printf("Usage: messagebot-tester <username> <password> <message> <receiverSteamId64>");
    }
}