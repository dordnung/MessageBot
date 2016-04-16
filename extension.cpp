/**
 * -----------------------------------------------------
 * File			extension.cpp
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

 // Include
#include <sstream>
#include <cstdlib>
#include "extension.h"


// Create and register extension
MessageBot messageBot;
SMEXT_LINK(&messageBot);


// Natives, only wrapper functions can be used
static sp_nativeinfo_t messagebot_natives[] =
{
	{ "MessageBot_SetSendMethod", MessageBot_SetSendMethod },
	{ "MessageBot_SendMessage", MessageBot_SendBotMessage },
	{ "MessageBot_SetLoginData", MessageBot_SetLoginData },
	{ "MessageBot_AddRecipient", MessageBot_AddRecipient },
	{ "MessageBot_RemoveRecipient",  MessageBot_RemoveRecipient },
	{ "MessageBot_IsRecipient",  MessageBot_IsRecipient },
	{ "MessageBot_ClearRecipients",  MessageBot_ClearRecipients },
	{ NULL, NULL }
};


// Constructor to set default values
MessageBot::MessageBot() {
	// Default values
	this->webClass = NULL;
	this->watchThread = NULL;
	this->extensionLoaded = false;
	this->username.clear();
	this->password.clear();
}


// Extension loaded
bool MessageBot::SDK_OnLoad(char *error, size_t maxlength, bool late) {
	// Add natives and register library 
	sharesys->AddNatives(myself, messagebot_natives);
	sharesys->RegisterLibrary(myself, "messagebot");

	// Create web api class
	this->webClass = new WebAPIClass();

	// Register game frame hook
	smutils->AddGameFrameHook(&MessageBot_OnGameFrameHit);

	// It's now loaded
	extensionLoaded = true;

	// Start the watch thread
	this->watchThread = threader->MakeThread(new WatchThread(), Thread_Default);

	// Loaded
	return true;
}


// Unloaded
void MessageBot::SDK_OnUnload() {
	// It's now unloaded
	this->extensionLoaded = false;

	rootconsole->ConsolePrint("[MessageBot] Please wait until Messagebot thread is finished...");

	// Remove frame hook
	smutils->RemoveGameFrameHook(&MessageBot_OnGameFrameHit);

	// Stop thread
	if (this->watchThread != NULL) {
		if (this->watchThread->GetState() != Thread_Done) {
			// But first of all wait until it's finished
			this->watchThread->WaitForThread();
		}

		// Destroys the thread object
		this->watchThread->DestroyThis();
	}

	// Delete all messages
	while (!this->messageQueue.empty()) {
		this->messageQueue.pop();
	}

	// Delete all recipients
	this->recipients.clear();

	// Delete webClass
	delete this->webClass;

	// Reset values
	this->webClass = NULL;
	this->watchThread = NULL;
	this->username.clear();
	this->password.clear();
}


// Prepare the Forward
void MessageBot::PrepareForward(IPluginFunction *function, int result) {
	this->mutex.lock();
	this->pawnForwards.push({ function, result });
	this->mutex.unlock();
}


// Converts a steamId2 to a steamId64
uint64_t MessageBot::SteamId2toSteamId64(IPluginContext *pContext, const cell_t *params, int32_t steamIdParam) {
	// Read steamId from params
	char *steamid;

	pContext->LocalToString(params[steamIdParam], &steamid);
	std::string steamId2(steamid);

	// Maybe it's already a community Id
	if (steamId2.find(":") == std::string::npos) {
		return strtoull(steamid, NULL, 10);
	}
	
	// To small for a valid steam Id
	if (steamId2.length() < 11) {
		return 0;
	}

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

	uint64_t server = strtoull(strippedSteamId.at(1).c_str(), NULL, 10);
	uint64_t authId = strtoull(strippedSteamId.at(2).c_str(), NULL, 10);

	// Wrong format
	if (authId == 0) {
		return 0;
	}

	// Calculate community Id
#if defined _WIN32
	return  authId * 2 + 76561197960265728 + server;
#elif defined _LINUX
	return  authId * 2 + 76561197960265728LLU + server;
#endif
}


// Frame hit
void MessageBot::OnGameFrameHit(bool simulating) {
	// Could lock?
	if (!this->mutex.try_lock()) {
		return;
	}

	// Lock
	this->mutex.lock();

	// Item in queue?
	if (!this->pawnForwards.empty()) {
		// Get first item and remove it from queue
		PawnForward pawnForward = this->pawnForwards.front();
		this->pawnForwards.pop();

		// Call forward if valid
		IPluginFunction *pawnFunc = pawnForward.function;

		if (pawnFunc != NULL && pawnFunc->IsRunnable()) {
			pawnFunc->PushCell(pawnForward.result);
			pawnFunc->PushCell(0);
			pawnFunc->Execute(NULL);
		}
	}

	// Unlock
	this->mutex.unlock();
}


// Natives
// Sends a Message
cell_t MessageBot::SendBotMessage(IPluginContext *pContext, const cell_t *params) {
	// callback
	IPluginFunction *callback;

	// Message
	char *message;

	// Get Data
	callback = pContext->GetFunctionById(params[1]);
	pContext->LocalToString(params[2], &message);

	// Valid callback?
	if (callback != NULL) {
		// Create new message
		Message messsage;

		messsage.username = this->username;
		messsage.password = this->password;
		messsage.callback = callback;
		messsage.message = message;
		messsage.showLogin = !!params[3];

		// Append message
		this->messageQueue.push(messsage);
	}

	return 1;
}


// Set the login data
cell_t MessageBot::SetLoginData(IPluginContext *pContext, const cell_t *params) {
	// User Data
	char *username;
	char *password;

	// Get global username and password
	pContext->LocalToString(params[1], &username);
	pContext->LocalToString(params[2], &password);

	// Copy Strings
	this->username = username;
	this->password = password;

	return 1;
}


// Native to add recipient
cell_t MessageBot::AddRecipient(IPluginContext *pContext, const cell_t *params) {
	uint64_t commId = this->SteamId2toSteamId64(pContext, params, 1);

	// Valid community Id?
	if (commId == 0) {
		return 0;
	}

	// Check for duplicates
	for (std::list<uint64_t>::iterator recipient = this->recipients.begin(); recipient != this->recipients.end(); recipient++) {
		if (commId == *recipient) {
			return 0;
		}
	}

	// Append recipient
	this->recipients.push_back(commId);

	return 1;
}


// Remove a recipient
cell_t MessageBot::RemoveRecipient(IPluginContext *pContext, const cell_t *params) {
	uint64_t commId = this->SteamId2toSteamId64(pContext, params, 1);

	// Valid community Id?
	if (commId == 0) {
		return 0;
	}

	// Search for steamid
	for (std::list<uint64_t>::iterator recipient = this->recipients.begin(); recipient != this->recipients.end(); recipient++) {
		if (commId == *recipient) {
			// Delete
			this->recipients.erase(recipient);

			return 1;
		}
	}

	return 0;
}


// Is steamid a recipient?
cell_t MessageBot::IsRecipient(IPluginContext *pContext, const cell_t *params) {
	uint64_t commId = this->SteamId2toSteamId64(pContext, params, 1);

	// Valid community Id?
	if (commId == 0) {
		return 0;
	}

	// Search for steamid
	for (std::list<uint64_t>::iterator recipient = this->recipients.begin(); recipient != this->recipients.end(); recipient++) {
		if (commId == *recipient) {
			return 1;
		}
	}

	return 0;
}

// Clear whole recipient list
cell_t MessageBot::ClearRecipients(IPluginContext *pContext, const cell_t *params) {
	this->recipients.clear();

	return 0;
}


// Set the send method
// Deprecated
cell_t MessageBot::SetSendMethod(IPluginContext *pContext, const cell_t *params) {
	return 1;
}



// Thread executed
void WatchThread::RunThread(IThreadHandle *pHandle) {
	// Infinite Loop while loaded
	while (messageBot.extensionLoaded) {
		// Sleep only in little steps to handle unload correctly
		for (int i = 0; i < 20; i++) {
			Sleeping(50);

			if (!messageBot.extensionLoaded) {
				return;
			}
		}

		// Is a item in list?
		if (messageBot.extensionLoaded && !messageBot.messageQueue.empty()) {
			Message message = messageBot.messageQueue.front();

			// Send Message via Webapi
			CallBackResult result = messageBot.webClass->SendMessageWebAPI(message.username, message.password, message.message, message.showLogin, messageBot.recipients);

			// Prepare the forward
			messageBot.PrepareForward(message.callback, result);

			// Remove the message from the queue
			messageBot.messageQueue.pop();
		}
	}
}


// Game frame and native wrappers
void MessageBot_OnGameFrameHit(bool simulating) {
	messageBot.OnGameFrameHit(simulating);
}

cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params) {
	return messageBot.SetLoginData(pContext, params);
}

cell_t MessageBot_SendBotMessage(IPluginContext *pContext, const cell_t *params) {
	return messageBot.SendBotMessage(pContext, params);
}

cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params) {
	return messageBot.AddRecipient(pContext, params);
}

cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params) {
	return messageBot.RemoveRecipient(pContext, params);
}

cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params) {
	return messageBot.IsRecipient(pContext, params);
}

cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params) {
	return messageBot.ClearRecipients(pContext, params);
}

cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params) {
	return messageBot.SetSendMethod(pContext, params);
}
