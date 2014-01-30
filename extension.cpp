/**
 * -----------------------------------------------------
 * File			extension.cpp
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



// Include
#include "extension.h"
#include "osw.h"
#include "webapi.h"

// Are we loaded?
bool extensionLoaded;
bool volatile m_locked; 


// For Forward
CVector<PawnFuncThreadReturn *> vecPawnReturn;
IThreadHandle *threading;


// User Data
char username[128];
char password[128];



// Send Method
SendMethod sendMethod = SEND_METHOD_ONLINEAPI;


// Queue start
Queue *queueStart = NULL;




// Register extension
MessageBot messageBot;
SMEXT_LINK(&messageBot);




// Natives
sp_nativeinfo_t messagebot_natives[] = 
{
	{"MessageBot_SetSendMethod", MessageBot_SetSendMethod},
	{"MessageBot_SendMessage", MessageBot_SendMessage},
	{"MessageBot_SetLoginData", MessageBot_SetLoginData},
	{"MessageBot_AddRecipient", MessageBot_AddRecipient},
	{"MessageBot_RemoveRecipient", MessageBot_RemoveRecipient},
	{"MessageBot_IsRecipient", MessageBot_IsRecipient},
	{"MessageBot_ClearRecipients", MessageBot_ClearRecipients},
	{NULL, NULL}
};




// Extension loaded
bool MessageBot::SDK_OnLoad(char *error, size_t maxlength, bool late)
{
	// Empty username and password
	strcpy(username, "");
	strcpy(password, "");

	
	// Add natives and register library 
	sharesys->AddNatives(myself, messagebot_natives);
	sharesys->RegisterLibrary(myself, "messagebot");


	oswClass = new OSWClass();
	webClass = new WebAPIClass();


	// GameFrame
	smutils->AddGameFrameHook(&OnGameFrameHit);
	m_locked = false;


	// Loaded
	extensionLoaded = true;

	// Start the watch Thread
	threading = threader->MakeThread(new watchThread(), Thread_Default);


	// Loaded
	return true;
}






// Unloaded
void MessageBot::SDK_OnUnload()
{
	// unLoaded
	extensionLoaded = false;


	rootconsole->ConsolePrint("[MessageBot] Please wait until Thread is finished...");


	// Stop Thread
	if (threading != NULL && threading->GetState() != Thread_Done)
	{
		threading->WaitForThread();
		threading->DestroyThis();
	}

	
	// Disconnect here
	oswClass->LogoutOSW(true);


	// Delete all messages
	while (queueStart != NULL)
	{
		queueStart->remove();
	}


	// Delete all recipients
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		if (recipients[i] != NULL)
		{
			// delete
			delete recipients[i];

			recipients[i] = NULL;
		}
	}


	// Remove Frame hook
	smutils->RemoveGameFrameHook(&OnGameFrameHit);


	// Delete oswclass
	delete oswClass;
	delete webClass;
}





// Prepare the Forward
void MessageBot::prepareForward(IPluginFunction *func, CallBackResult result, int errorS)
{
	// Create return
	PawnFuncThreadReturn *pReturn = new PawnFuncThreadReturn;



	// Fill in
	pReturn->pFunc = func;
	pReturn->result = result;
	pReturn->errorState = errorS;



	// Push to Vector
	m_locked = true;

	vecPawnReturn.push_back(pReturn);

	m_locked = false;
}





// Frame hit
void OnGameFrameHit(bool simulating)
{
	// Could lock?
	if (m_locked)
	{
		return;
	}
	

	// Lock
	m_locked = true;


	// Item in vec?
	if (!vecPawnReturn.empty())
	{
		// Get Last item
		PawnFuncThreadReturn *pReturn = vecPawnReturn.back();
		vecPawnReturn.pop_back();
		
		// Call Forward
		IPluginFunction *pFunc = pReturn->pFunc;

		if (pFunc != NULL && pFunc->IsRunnable())
		{
			pFunc->PushCell(pReturn->result);
			pFunc->PushCell(pReturn->errorState);
			pFunc->Execute(NULL);
		}
		
		// Delete item
		delete pReturn;
	}
	

	// Unlock
	m_locked = false;
}






// Thread executed
void watchThread::RunThread(IThreadHandle *pHandle)
{
	// Infinite Loop while loaded
	while (extensionLoaded)
	{		
		// Extension loaded?
		if (extensionLoaded)
		{
			// Sleep here, sleeping is sooo good :)
			Sleeping(1000);
		}


		// Item in list?
		if (extensionLoaded && queueStart != NULL)
		{
			// steam connection setup
			if (oswClass->getSetup() == 0 && sendMethod == SEND_METHOD_STEAMWORKS)
			{
				if (oswClass->SetupOSW() == 2)
				{
					smutils->LogError(myself, "Failed to connect to steam libraries... Switching to Online Method");

					sendMethod = SEND_METHOD_ONLINEAPI;
				}
			}



			int errState = 0;

			if (oswClass->getSetup() == 1 && sendMethod == SEND_METHOD_STEAMWORKS)
			{
				// Send Message
				CallBackResult result = oswClass->SendMessageOSW(queueStart->getUsername(), queueStart->getPassword(), queueStart->getMessage(), queueStart->getOnline(), errState);


				// got a valid result?
				messageBot.prepareForward(queueStart->getCallback(), result, errState);

				queueStart->remove();

				continue;
			}


			// Send Message via Webapi
			CallBackResult result = webClass->SendMessageWebAPI(queueStart->getUsername(), queueStart->getPassword(), queueStart->getMessage(), queueStart->getOnline(), errState);


			messageBot.prepareForward(queueStart->getCallback(), result, errState);

			queueStart->remove();
		}
	}
}








// Natives
// Sends a Message
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params)
{
	// callback
	IPluginFunction *callback;


	// Message
	char message[MAX_MESSAGE_LENGTH];
	char *message_cpy;


	// Get Data
	callback = pContext->GetFunctionById(params[1]);

	pContext->LocalToString(params[2], &message_cpy);
	strcpy(message, message_cpy);


	if (callback != NULL)
	{
		// Create new Queue
		Queue *newQueue = new Queue(callback, username, password, message, params[3]);

		// Add Head
		if (queueStart == NULL)
		{
			queueStart = newQueue;
		}
		else
		{
			// Add at end
			queueStart->add(newQueue);
		}
	}
	
	return 1;
}




// Set the send method
cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params)
{
	sendMethod = (SendMethod)params[1];

	return 1;
}




// Set the login data
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params)
{
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
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params)
{
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64 commID = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);


	// Convert to uint64
	if (strstr(steamid, ":") == NULL)
	{
		std::stringstream str;

		str << steamid;
		str >> commID;
	}


	// Check duplicate
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		// Not Empty?
		if (recipients[i] != NULL)
		{
			if (strcmp(recipients[i]->Render(), steamid) == 0 || (commID != 0 && commID == recipients[i]->ConvertToUint64()))
			{
				return 0;
			}
		}
	}


	// Go through all recipients
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		// Empty?
		if (recipients[i] == NULL)
		{
			// Maybe it's a community ID?
			if (commID != 0)
			{
				recipients[i] = new CSteamID(commID);
			}
			else
			{
				// Convert from Steamid
				recipients[i] = oswClass->steamIDtoCSteamID(steamid);
			}


			// Valid Steamid?
			if (recipients[i] != NULL && recipients[i]->IsValid())
			{
				return 1;
			}
			else if (recipients[i] != NULL)
			{
				// Delete invalid
				delete recipients[i];

				recipients[i] = NULL;

				return 0;
			}
			else
			{
				return 0;
			}
		}
	}

	return 0;
}




// Remove a recipient
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params)
{
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64 commID = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);


	// Convert to uint64
	if (strstr(steamid, ":") == NULL)
	{
		std::stringstream str;

		str << steamid;
		str >> commID;
	}


	// Search for steamid
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		// Not Empty?
		if (recipients[i] != NULL)
		{
			if (strcmp(recipients[i]->Render(), steamid) == 0 || (commID != 0 && commID == recipients[i]->ConvertToUint64()))
			{
				// Delete
				delete recipients[i];

				recipients[i] = NULL;

				return 1;
			}
		}
	}

	return 0;
}




// Is steamid a recipient?
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params)
{
	// User Data
	char steamid[128];
	char *steamid_cpy;

	uint64 commID = 0;

	// Get String
	pContext->LocalToString(params[1], &steamid_cpy);

	strcpy(steamid, steamid_cpy);


	// Convert to uint64
	if (strstr(steamid, ":") == NULL)
	{
		std::stringstream str;

		str << steamid;
		str >> commID;
	}


	// Search for steamid
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		// Not Empty?
		if (recipients[i] != NULL)
		{
			if (strcmp(recipients[i]->Render(), steamid) == 0 || (commID != 0 && commID == recipients[i]->ConvertToUint64()))
			{
				return 1;
			}
		}
	}

	return 0;
}



// Clear hole recipient list
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params)
{
	for (int i=0; i < MAX_RECIPIENTS; i++)
	{
		// Not empty?
		if (recipients[i] != NULL)
		{
			// Delete
			delete recipients[i];

			recipients[i] = NULL;
		}
	}

	return 0;
}





// Queue Class
Queue::Queue(IPluginFunction *func, char *name, char *pw, char *message, int online)
{
	strcpy(user, name);
	strcpy(pass, pw);
	strcpy(txt, message);

	// Show online?
	if (online == 0)
	{
		showLogin = false;
	}
	else
	{
		showLogin = true;
	}

	callback = func; 
	next = NULL;
}



// Get Methods for queue
Queue *Queue::getNext() const
{
	return next;
}

char* Queue::getUsername() const
{
	return (char*)user;
}

char *Queue::getPassword() const
{
	return (char*)pass;
}

char *Queue::getMessage() const
{
	return (char*)txt;
}



bool Queue::getOnline() const
{
	return showLogin;
}

IPluginFunction *Queue::getCallback() const
{
	return callback;
}





// Add new item at the end
void Queue::add(Queue *newQueue)
{
	// if next -> recursive
	if (next != NULL)
	{
		next->add(newQueue);
	}
	else
	{
		// if end -> at here
		next = newQueue;
	}
}



// Remove first item on the queue
void Queue::remove()
{
	// Do we have a start?
	if (queueStart != NULL)
	{
		// Get next item on the queue
		Queue *buffer = queueStart->getNext();


		// Delete first item
		delete queueStart;

		// Set new queue start
		queueStart = buffer;
	}
}