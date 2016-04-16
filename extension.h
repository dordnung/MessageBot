/**
 * -----------------------------------------------------
 * File			extension.h
 * Authors		David O., Impact
 * License		GPLv3
 * Web			http://popoklopsi.de, http://gugyclan.eu
 * -----------------------------------------------------
 *
 * Originally provided for CallAdmin by Popoklopsi and Impact
 *
 * Copyright (C) 2014-2016 David O., Impact
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
#include <string>
#include <list>
#include <queue>
#include <mutex>

#include "smsdk_ext.h"
#include "webapi.h"

// Sleeping for Linux and Windows
#ifdef _WIN32
#define Sleeping(x) Sleep(x);
#elif defined _LINUX
#define Sleeping(x) usleep(x * 1000);
#endif


// Struct for a message
typedef struct {
	IPluginFunction *callback;
	bool showLogin;

	std::string username;
	std::string password;
	std::string message;

} Message;


// Struct for a forward
typedef struct {
	IPluginFunction *function;
	int result;

} PawnForward;


// Main extension class
class MessageBot : public SDKExtension {
public:
	MessageBot();

	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();

	// Prepare Forward
	void PrepareForward(IPluginFunction *function, int result);

	// Converts a steamid to a SteamId64
	uint64_t SteamId2toSteamId64(IPluginContext *pContext, const cell_t *params, int32_t steamIdParam);

	// Frame hit
	void OnGameFrameHit(bool simulating);

	// Natives
	cell_t SetLoginData(IPluginContext *pContext, const cell_t *params);
	cell_t SendBotMessage(IPluginContext *pContext, const cell_t *params);
	cell_t AddRecipient(IPluginContext *pContext, const cell_t *params);
	cell_t RemoveRecipient(IPluginContext *pContext, const cell_t *params);
	cell_t IsRecipient(IPluginContext *pContext, const cell_t *params);
	cell_t ClearRecipients(IPluginContext *pContext, const cell_t *params);

	// Deprecated, does nothing
	cell_t SetSendMethod(IPluginContext *pContext, const cell_t *params);

private:
	// Thread can access vars
	friend class WatchThread;

	WebAPIClass *webClass;
	IThreadHandle *watchThread;

	// Are we loaded?
	bool extensionLoaded;

	// Username and password
	std::string username;
	std::string password;

	// Mutex for watchThread
	std::mutex mutex;

	// Recipients
	std::list<uint64_t> recipients;

	// Queue for forwards
	std::queue<PawnForward> pawnForwards;

	// Queue for messages
	std::queue<Message> messageQueue;
};


// Thread wachting for new messages
class WatchThread : public IThread {
public:
	void RunThread(IThreadHandle *pThread);
	void OnTerminate(IThreadHandle *pThread, bool cancel) {
	};
};


// Game frame and native wrappers
void MessageBot_OnGameFrameHit(bool simulating);

cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SendBotMessage(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params);


#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
