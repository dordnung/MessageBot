/**
 * -----------------------------------------------------
 * File			extension.cpp
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

// Include
#include "extension.h"
#include "webapi.h"

WebAPIClass *webClass;

// Are we loaded?
bool extensionLoaded;
bool volatile m_locked;

// For Forward
std::vector <PawnFuncThreadReturn *> vecPawnReturn;
IThreadHandle *threading;

// User Data
char username[128];
char password[128];

// Recipients
std::vector<uint64_t> recipients;

// Queue start
std::queue<Message> messageQueue;

// Register extension
MessageBot messageBot;
SMEXT_LINK(&messageBot);


// Natives
sp_nativeinfo_t messagebot_natives[] =
{
	{ "MessageBot_SetSendMethod", MessageBot_SetSendMethod },
	{ "MessageBot_SendMessage", MessageBot_SendMessage },
	{ "MessageBot_SetLoginData", MessageBot_SetLoginData },
	{ "MessageBot_AddRecipient", MessageBot_AddRecipient },
	{ "MessageBot_RemoveRecipient", MessageBot_RemoveRecipient },
	{ "MessageBot_IsRecipient", MessageBot_IsRecipient },
	{ "MessageBot_ClearRecipients", MessageBot_ClearRecipients },
	{ NULL, NULL }
};


// Extension loaded
bool MessageBot::SDK_OnLoad(char *error, size_t maxlength, bool late) {
	// Empty username and password
	strcpy(username, "");
	strcpy(password, "");


	// Add natives and register library 
	sharesys->AddNatives(myself, messagebot_natives);
	sharesys->RegisterLibrary(myself, "messagebot");

	webClass = new WebAPIClass();

	// GameFrame
	smutils->AddGameFrameHook(&OnGameFrameHit);
	m_locked = false;

	// Loaded
	extensionLoaded = true;

	// Start the watch Thread
	threading = threader->MakeThread(new WatchThread(), Thread_Default);

	// Loaded
	return true;
}


// Unloaded
void MessageBot::SDK_OnUnload() {
	// unloaded
	extensionLoaded = false;

	rootconsole->ConsolePrint("[MessageBot] Please wait until Thread is finished...");

	// Stop Thread
	if (threading != NULL && threading->GetState() != Thread_Done) {
		threading->WaitForThread();
		threading->DestroyThis();
	}

	// Delete all messages
	while (!messageQueue.empty()) {
		messageQueue.pop();
	}

	// Delete all recipients
	recipients.clear();

	// Remove Frame hook
	smutils->RemoveGameFrameHook(&OnGameFrameHit);

	// Delete webClass
	delete webClass;
}


// Prepare the Forward
void MessageBot::prepareForward(IPluginFunction *function, int result) {
	// Create return
	PawnFuncThreadReturn *pawnReturn = new PawnFuncThreadReturn;

	// Fill in
	pawnReturn->function = function;
	pawnReturn->result = result;

	// Push to Vector
	m_locked = true;

	vecPawnReturn.push_back(pawnReturn);

	m_locked = false;
}

// Frame hit
void OnGameFrameHit(bool simulating) {
	// Could lock?
	if (m_locked) {
		return;
	}

	// Lock
	m_locked = true;

	// Item in vec?
	if (!vecPawnReturn.empty()) {
		// Get Last item
		PawnFuncThreadReturn *pawnReturn = vecPawnReturn.back();
		vecPawnReturn.pop_back();

		// Call Forward
		IPluginFunction *pawnFunc = pawnReturn->function;

		if (pawnFunc != NULL && pawnFunc->IsRunnable()) {
			pawnFunc->PushCell(pawnReturn->result);
			pawnFunc->PushCell(0);
			pawnFunc->Execute(NULL);
		}

		// Delete item
		delete pawnReturn;
	}

	// Unlock
	m_locked = false;
}


// Thread executed
void WatchThread::RunThread(IThreadHandle *pHandle) {
	// Infinite Loop while loaded
	while (extensionLoaded) {
		// Extension loaded?
		if (extensionLoaded) {
			// Sleep here, sleeping is sooo good :)
			Sleeping(1000);
		}

		// Item in list?
		if (extensionLoaded && !messageQueue.empty()) {
			Message start = messageQueue.front();

			// Send Message via Webapi
			CallBackResult result = webClass->sendMessageWebAPI(start.username, start.password, start.message, start.showLogin, recipients);

			messageBot.prepareForward(start.callback, result);

			// Remove message
			messageQueue.pop();
		}
	}
}


// Natives
// Sends a Message
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params) {
	// callback
	IPluginFunction *callback;

	// Message
	char *message;

	// Get Data
	callback = pContext->GetFunctionById(params[1]);
	pContext->LocalToString(params[2], &message);

	if (callback != NULL) {
		// Create new Message
		Message messsage;

		messsage.callback = callback;
		messsage.username = username;
		messsage.password = password;
		messsage.message = message;
		messsage.showLogin = !!params[3];

		messageQueue.push(messsage);
	}

	return 1;
}


// Set the send method
// Deprecated
cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params) {
	return 1;
}


// Set the login data
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params) {
	// User Data
	char *username_cpy;
	char *password_cpy;

	// Get global username and password
	pContext->LocalToString(params[1], &username_cpy);
	pContext->LocalToString(params[2], &password_cpy);

	// Copy Strings
	strcpy(username, username_cpy);
	strcpy(password, password_cpy);

	return 1;
}


// Native to add recipient
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params) {
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64_t commId = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);

	// Convert to uint64_t
	if (strstr(steamid, ":") == NULL) {
		std::stringstream str;

		str << steamid;
		str >> commId;
	} else {
		commId = steamId2toSteamId64(steamid);
	}

	// Check duplicate
	for (std::vector<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		if (commId != 0 && commId == *recipient) {
			return 0;
		}
	}

	uint64_t recipient;

	// Maybe it's a community ID?
	if (commId != 0) {
		recipient = commId;
	} else {
		// Convert from Steamid
		recipient = steamId2toSteamId64(steamid);
	}

	// Valid Steamid?
	if (recipient > 0) {
		recipients.push_back(recipient);

		return 1;
	}

	return 0;
}


// Remove a recipient
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params) {
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64_t commId = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);

	// Convert to uint64_t
	if (strstr(steamid, ":") == NULL) {
		std::stringstream str;

		str << steamid;
		str >> commId;
	} else {
		commId = steamId2toSteamId64(steamid);
	}


	// Search for steamid
	for (std::vector<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		if (commId != 0 && commId == *recipient) {
			// Delete
			recipients.erase(recipient);

			return 1;
		}
	}

	return 0;
}


// Is steamid a recipient?
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params) {
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64_t commId = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);

	// Convert to uint64_t
	if (strstr(steamid, ":") == NULL) {
		std::stringstream str;

		str << steamid;
		str >> commId;
	} else {
		commId = steamId2toSteamId64(steamid);
	}

	// Search for steamid
	for (std::vector<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		if (commId != 0 && commId == *recipient) {
			return 1;
		}
	}

	return 0;
}

// Clear whole recipient list
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params) {
	recipients.clear();

	return 0;
}


// Converts a steamId2 to a steamId64
uint64_t steamId2toSteamId64(std::string steamId2) {
	// To small
	if (steamId2.length() < 11) {
		return 0;
	}

	int server = 0;
	int authID = 0;

	// Strip the steamid
	std::vector<std::string> strippedSteamId;

	std::stringstream ss(steamId2);
	std::string item;

	while (std::getline(ss, item, ':')) {
		strippedSteamId.push_back(item);
	}

	// There should be 3 parts
	if (strippedSteamId.size() != 3) {
		return 0;
	}

	server = atoi(strippedSteamId.at(1).c_str());
	authID = atoi(strippedSteamId.at(2).c_str());

	// Wrong Format
	if (authID == 0) {
		return 0;
	}

	uint64_t uintId = (uint64_t)authID * 2;

	// Convert to a uint64_t
#if defined _WIN32
	uintId += 76561197960265728 + server;
#elif defined _LINUX
	uintId += 76561197960265728LLU + server;
#endif

	// Return it
	return uintId;
}