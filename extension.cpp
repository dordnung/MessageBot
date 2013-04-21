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
 * Copyright (C) 2013 David <popoklopsi> Ordnung, Impact
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



// Includes
#include <sstream>

#include "extension.h"
#include "sh_vector.h"


// Are we loaded?
bool extensionLoaded;


// For Forward
CVector<PawnFuncThreadReturn *> vecPawnReturn;
IMutex *g_pPawnMutex;
IThreadHandle *threading;

// Recipients
CSteamID *recipients[MAX_RECIPIENTS];


// User Data
char *username;
char *password;


// Steam stuff
HSteamPipe pipeSteam;
HSteamUser clientUser;
IClientFriends *steamFriends;
IClientEngine *steamClient;
IClientUser *steamUser;



// Queue start
Queue *queueStart = NULL;



// Register extension
MessageBot messageBot;
SMEXT_LINK(&messageBot);





// Natives
sp_nativeinfo_t messagebot_natives[] = 
{
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
	// Add natives and register library 
	sharesys->AddNatives(myself, messagebot_natives);
	sharesys->RegisterLibrary(myself, "messagebot");


	// GameFrame
	smutils->AddGameFrameHook(&OnGameFrameHit);
	g_pPawnMutex = threader->MakeMutex();


	// Empty username and password
	username = "";
	password = "";



	// Factory laod
	CSteamAPILoader loader;
	CreateInterfaceFn factory = loader.GetSteam3Factory();



	// Get Factory
	if (!factory)
	{
		g_pSM->LogError(myself, "Unable to load steamclient factory.");

		return false;
	}



	// Get Steam Engine
	steamClient = (IClientEngine *)factory(CLIENTENGINE_INTERFACE_VERSION, NULL);

	if (!steamClient)
	{
		g_pSM->LogError(myself, "Unable to get the client engine.");

		return false;
	}



	// Get User
	clientUser = steamClient->CreateLocalUser(&pipeSteam, k_EAccountTypeIndividual);

	if (!clientUser || !pipeSteam)
	{
		g_pSM->LogError(myself, "Unable to create the local user.");

		return false;
	}



	// Get Client User
	steamUser = (IClientUser *)steamClient->GetIClientUser(clientUser, pipeSteam, CLIENTUSER_INTERFACE_VERSION);

	if (!steamUser)
	{
		g_pSM->LogError(myself, "Unable to get the client user interface.");

		steamClient->ReleaseUser(pipeSteam, clientUser);
		steamClient->BReleaseSteamPipe(pipeSteam);

		return false;
	}



	// Get Friends
	steamFriends = (IClientFriends *)steamClient->GetIClientFriends(clientUser, pipeSteam, CLIENTFRIENDS_INTERFACE_VERSION);

	if (!steamFriends)
	{
		g_pSM->LogError(myself, "Unable to get the client friends interface.");

		steamClient->ReleaseUser(pipeSteam, clientUser);
		steamClient->BReleaseSteamPipe(pipeSteam);

		return false;
	}


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
	// Stop Thread
	if (threading != NULL)
	{
		threading->DestroyThis();
	}

	// Loaded
	extensionLoaded = false;


	// Disconnect here
	if (steamClient && pipeSteam)
	{
		// Release
		steamClient->ReleaseUser(pipeSteam, clientUser);
		steamClient->BReleaseSteamPipe(pipeSteam);
	}


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


	// Remove Frame hook and mutex
	smutils->RemoveGameFrameHook(&OnGameFrameHit);

	g_pPawnMutex->DestroyThis();
}





// Frame hit
void OnGameFrameHit(bool simulating)
{
	// Could lock?
	if (!g_pPawnMutex->TryLock())
	{
		return;
	}
	

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
			pFunc->PushCell(pReturn->error);
			pFunc->Execute(NULL);
		}
		
		// Delete item
		delete pReturn;
	}
	

	// Unlock
	g_pPawnMutex->Unlock();
}






// Thread executed
void watchThread::RunThread(IThreadHandle *pHandle)
{
	// Infinite Loop while loaded
	while (extensionLoaded)
	{
		// Item in list?
		if (queueStart != NULL && steamUser != NULL)
		{
			// Login
			steamUser->LogOnWithPassword(true, queueStart->getUsername(), queueStart->getPassword());


			// Finished or Error?
			bool finished = false;
			bool foundError = false;

			// Last Callback
			CallbackMsg_t callBack;

			// Timeout
			time_t timeout = time(0) + 10;


			// Not longer as 10 seconds
			while(time(0) < timeout && extensionLoaded)
			{
				// Get last callbacks
				while (Steam_BGetCallback(pipeSteam, &callBack))
				{
					// Logged In?
					if (callBack.m_iCallback == SteamServersConnected_t::k_iCallback && steamUser != NULL)
					{
						bool foundData = false;
						char *message = queueStart->getMessage();

						// Logged in!
						steamUser->SetSelfAsPrimaryChatDestination();
						steamFriends->SetPersonaState(k_EPersonaStateOnline);


						// Add all Recipients and send message
						for (int i = 0; i < MAX_RECIPIENTS; i++)
						{
							// Valid Steamid?
							if (recipients[i] != NULL && steamFriends != NULL)
							{
								// Add Recipients
								if (steamFriends->GetFriendRelationship(*recipients[i]) != k_EFriendRelationshipFriend)
								{
									steamFriends->AddFriend(*recipients[i]);
								}

								// Send him the message
								if (steamFriends->SendMsgToFriend(*recipients[i], k_EChatEntryTypeChatMsg, message, strlen(message) + 1))
								{
									// We found one
									foundData = true;
								}
							}
						}


						// Array is empty?
						if (!foundData)
						{
							// Array is Empty !
							prepareForward(queueStart->getCallback(), ARRAY_EMPTY);

							// Found Error
							foundError = true;
						}
						else
						{
							// Success :)
							prepareForward(queueStart->getCallback(), SUCCESS);

							finished = true;
						}
					}

					// Error on connect
					else if (callBack.m_iCallback == SteamServerConnectFailure_t::k_iCallback && steamFriends != NULL)
					{
						// Login Error
						g_pSM->LogError(myself, "Couldn't login in %s's account.", queueStart->getUsername());


						// Get Error Code
						SteamServerConnectFailure_t *error = (SteamServerConnectFailure_t *)callBack.m_pubParam;

						// We have a Login Error
						prepareForward(queueStart->getCallback(), LOGIN_ERROR, error->m_eResult);


						// Found Error
						foundError = true;
					}



					// Free callback
					Steam_FreeLastCallback(pipeSteam);



					// We found a error or finished -> stop here
					if (foundError || finished)
					{
						break;
					}
				}

				// We found a error -> stop here
				if (foundError || finished)
				{
					break;
				}

				// Sleep here
				#if defined _WIN32
					Sleep(10);
				#elif defined _LINUX
					usleep(10);
				#endif
			}

			// Timeout
			if (!foundError && !finished)
			{
				prepareForward(queueStart->getCallback(), TIMEOUT_ERROR);
			}


			// Remove last
			queueStart->remove();


			// Sleep here
			#if defined _WIN32
				Sleep(100);
			#elif defined _LINUX
				usleep(100);
			#endif


			// Logout
			steamUser->LogOff();
		}
		
		// Sleep here
		#if defined _WIN32
			Sleep(100);
		#elif defined _LINUX
			usleep(100);
		#endif
	}
}





// Natives
// Sends a Message
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params)
{
	// callback
	IPluginFunction *callback;

	// Message
	char *message;



	// Get Data
	callback = pContext->GetFunctionById(params[1]);

	pContext->LocalToString(params[2], &message);


	if (callback != NULL)
	{
		// Create new Queue
		Queue *newQueue = new Queue(callback, username, password, message);

		// Add Head
		if (queueStart == NULL)
		{
			queueStart = newQueue;
		}
		else
		{
			// Add at end
			Queue *start = queueStart;

			while (start->getNext() != NULL)
			{
				start = start->getNext();
			}

			// Add
			start->setNext(newQueue);
		}
	}

	return 0;
}




// Set the login data
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params)
{
	// Get global username and password
	pContext->LocalToString(params[1], &username);
	pContext->LocalToString(params[2], &password);

	return 0;
}




// Native to add recipient
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params)
{
	// User Data
	char *steamid;
	uint64 commID = 0;


	// Get Steamid
	pContext->LocalToString(params[1], &steamid);


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
		// Empty?
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
				recipients[i] = steamIDtoCSteamID(steamid);
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
	char *steamid;
	uint64 commID = 0;

	pContext->LocalToString(params[1], &steamid);


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
	char *steamid;
	uint64 commID = 0;

	pContext->LocalToString(params[1], &steamid);


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
Queue::Queue(IPluginFunction *func, char *name, char *pw, char *message)
{
	user = name;
	pass = pw;
	txt = message;

	callback = func; 
	next = NULL;
}



// Get Methods for queue
Queue *Queue::getNext() 
{
	return next;
}

char* Queue::getUsername() 
{
	return user;
}

char *Queue::getPassword() 
{
	return pass;
}

char *Queue::getMessage() 
{
	return txt;
}


IPluginFunction *Queue::getCallback() 
{
	return callback;
}





// Set the next item on the list
void Queue::setNext(Queue *queueNext) 
{
	next = queueNext;
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






// Prepare the Forward
void prepareForward(IPluginFunction *func, CallBackResult result, cell_t error)
{
	// Create return
	PawnFuncThreadReturn *pReturn = new PawnFuncThreadReturn;



	// Fill in
	pReturn->pFunc = func;
	pReturn->result = result;
	pReturn->error = error;



	// Push to Vector
	g_pPawnMutex->Lock();

	vecPawnReturn.push_back(pReturn);

	g_pPawnMutex->Unlock();
}





// Converts a steamid to a CSteamID
CSteamID *steamIDtoCSteamID (char* steamid)
{
	// To small
	if (strlen(steamid) < 11)
	{
		return NULL;
	}


	int server = 0;
	int authID = 0;

	// Strip the steamid
	char *strip = strtok(steamid, ":");



	while (strip)
	{
		strip = strtok(NULL, ":");

		char *strip2 = strtok(NULL, ":");

		// Read server and authID
		if (strip2)
		{
			server = atoi(strip);
			authID = atoi(strip2);
		}
	}



	// Wrong Format
	if (authID == 0)
	{
		return NULL;
	}


	uint64 uintID = (uint64)authID * 2;

	// Convert to a uint64
	uintID += 76561197960265728 + server;


	// Return it
	return new CSteamID(uintID);
}