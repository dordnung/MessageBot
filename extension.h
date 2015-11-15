/**
 * -----------------------------------------------------
 * File			extension.h
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


#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_

// Includes
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <queue>
#include <string>

#include "smsdk_ext.h"


// Sleeping for Linux and Windows
#if defined _WIN32
#define Sleeping(x) Sleep(x);
#elif defined _LINUX
#define Sleeping(x) usleep(x * 1000);
#endif


// Struct for message queue
typedef struct {
public:
	IPluginFunction *callback;
	bool showLogin;

	std::string username;
	std::string password;
	std::string message;

} Message;


// Struct for forwards
typedef struct {
public:
	IPluginFunction *function;
	int result;

} PawnFuncThreadReturn;


// Main extension class
class MessageBot : public SDKExtension {
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();

public:
	// Prepare Forward
	void prepareForward(IPluginFunction *function, int result);
};


// Thread wachting for new messages
class WatchThread : public IThread {
public:
	void RunThread(IThreadHandle *pThread);
	void OnTerminate(IThreadHandle *pThread, bool cancel) {};
};


// Natives
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params);

// Deprecated, does nothing
cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params);


void OnGameFrameHit(bool simulating);

// Converts a steamid to a CSteamID
uint64_t steamId2toSteamId64(std::string steamId2);

#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_