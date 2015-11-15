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

#include <stdio.h>
#include <vector>
#include "webapi.h"

int main(int argc, const char* argv[]) {
	// ensure the correct number of parameters are used.
	if (argc == 5) {
		const char *username = argv[1];
		const char *password = argv[2];
		const char *message = argv[3];
		uint64_t steamId64 = strtoull(argv[4], NULL, 10);

		std::vector<uint64_t> receiver;
		receiver.push_back(steamId64);

		// Send the message
		WebAPIClass webApi;
		webApi.sendMessageWebAPI(username, password, message, true, receiver);
	} else {
		printf("Usage: messagebot-tester <username> <password> <message> <receiverSteamId64>");
	}

}