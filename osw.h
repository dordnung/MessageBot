/**
* -----------------------------------------------------
* File			osw.h
* Authors		David <popoklopsi> Ordnung, Impact
* Idea			Zephyrus
* License		GPLv3
* Web			http://popoklopsi.de, http://gugyclan.eu
* -----------------------------------------------------
*
* Originally provided for CallAdmin by Popoklopsi and Impact
*
* Copyright (C) 2014 David <popoklopsi> Ordnung, Impact
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


#ifndef _INCLUDE_OSW_H_
#define _INCLUDE_OSW_H_

#include <Steamworks.h>
#include "extension.h"



#define SET_SETUP(x) do {setup=x; return x;} while(0)

typedef bool(*GetCallbackFn)(HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg);
typedef void(*FreeLastCallbackFn)(HSteamPipe hSteamPipe);




// Working with steamworks
class OSWClass
{
private:
	int setup;

	HSteamPipe steamPipe;
	HSteamUser steamUser;
	IClientFriends *clientFriends;
	IClientEngine *clientEngine;
	IClientUser *clientUser;

	GetCallbackFn GetCallback;
	FreeLastCallbackFn FreeLastCallback;

public:
	OSWClass()
	{
		setup = 0;
		steamPipe = 0;
		steamUser = 0;
		
		clientFriends = NULL;
		clientEngine = NULL;
		clientUser = NULL;
		GetCallback = NULL;
		FreeLastCallback = NULL;
	}

	int getSetup() { return setup; }

	// Setup steam stuff
	int SetupOSW();

	// Send process for OSW
	CallBackResult SendMessageOSW(char *user, char *pass, char *msg, bool showLogin, int &error);

	// Logout 
	void LogoutOSW(bool clean);

	// Steamid to CSteamID
	CSteamID *steamIDtoCSteamID(char *steamid);
};




extern OSWClass* oswClass;


#endif