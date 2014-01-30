/**
* -----------------------------------------------------
* File			osw.cpp
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


#include "osw.h"
#include "extension.h"


// Create class
OSWClass* oswClass = NULL;


// Recipients
CSteamID *recipients[MAX_RECIPIENTS];






// Setup steam connection
int OSWClass::SetupOSW()
{
	// Load DLL or SO
	#if defined _WIN32
		DynamicLibrary lib("steamclient.dll");
	#elif defined _LINUX
		DynamicLibrary lib("steamclient.so");
	#endif


	// is Loaded?
	if (!lib.IsLoaded())
	{
		g_pSM->LogError(myself, "Unable to load steam client engine.");

		SET_SETUP(2);
	}


	// Factory
	CreateInterfaceFn factory = reinterpret_cast<CreateInterfaceFn>(lib.GetSymbol("CreateInterface"));

	// Get Factory
	if (!factory)
	{
		g_pSM->LogError(myself, "Unable to load steamclient factory.");

		SET_SETUP(2);
	}


	// Callback
	GetCallback = reinterpret_cast<GetCallbackFn>(lib.GetSymbol("Steam_BGetCallback"));

	if (!GetCallback)
	{
		g_pSM->LogError(myself, "Unable to load Steam callback.");

		SET_SETUP(2);
	}


	FreeLastCallback = reinterpret_cast<FreeLastCallbackFn>(lib.GetSymbol("Steam_FreeLastCallback"));

	if (!FreeLastCallback)
	{
		g_pSM->LogError(myself, "Unable to load Steam free callback.");

		SET_SETUP(2);
	}


	// Get Steam Engine
	clientEngine = reinterpret_cast<IClientEngine*>(factory(CLIENTENGINE_INTERFACE_VERSION, NULL));

	if (!clientEngine)
	{
		clientEngine = reinterpret_cast<IClientEngine*>(factory("CLIENTENGINE_INTERFACE_VERSION003", NULL));

		if (!clientEngine)
		{
			g_pSM->LogError(myself, "Unable to get the client engine.");

			SET_SETUP(2);
		}
	}



	// Get User
	steamUser = clientEngine->CreateLocalUser(&steamPipe, k_EAccountTypeIndividual);

	if (!steamUser || !steamPipe)
	{
		g_pSM->LogError(myself, "Unable to create the local user.");

		LogoutOSW(true);

		SET_SETUP(2);
	}



	// Get Client User
	clientUser = reinterpret_cast<IClientUser*>(clientEngine->GetIClientUser(steamUser, steamPipe, CLIENTUSER_INTERFACE_VERSION));

	if (!clientUser)
	{
		g_pSM->LogError(myself, "Unable to get the client user interface.");

		LogoutOSW(true);

		SET_SETUP(2);
	}



	// Get Friends
	clientFriends = reinterpret_cast<IClientFriends*>((IClientFriends *)clientEngine->GetIClientFriends(steamUser, steamPipe, CLIENTFRIENDS_INTERFACE_VERSION));

	if (!clientFriends)
	{
		g_pSM->LogError(myself, "Unable to get the client friends interface.");

		LogoutOSW(true);

		SET_SETUP(2);
	}


	SET_SETUP(1);
}




CallBackResult OSWClass::SendMessageOSW(char *user, char *pass, char *msg, bool showLogin, int &error)
{
	// Login
	clientUser->LogOnWithPassword(false, user, pass);

	// Last Callback
	CallbackMsg_t callBack;


	// Timeout
	time_t timeout = time(0) + 10;


	// Not longer as 10 seconds
	while (time(0) < timeout && extensionLoaded)
	{
		// Get last callbacks
		while (GetCallback(steamPipe, &callBack) && extensionLoaded)
		{
			if (time(0) > timeout)
			{
				break;
			}

			// Logged In?
			if (callBack.m_iCallback == SteamServersConnected_t::k_iCallback)
			{
				bool foundData = false;
				bool foundUser = false;
				EPersonaState state = showLogin ? k_EPersonaStateOnline : k_EPersonaStateOffline;


				// Logged in!
				clientUser->SetSelfAsPrimaryChatDestination();
				clientFriends->SetPersonaState(state);


				// Wait until online
				EPersonaState personaState = clientFriends->GetPersonaState();

				while (personaState != state)
				{
					Sleeping(10);

					personaState = clientFriends->GetPersonaState();
				}

				Sleeping(50);


				// Add all Recipients and send message
				for (int i = 0; i < MAX_RECIPIENTS && extensionLoaded; i++)
				{
					// Valid Steamid?
					if (recipients[i] != NULL && clientFriends != NULL)
					{
						// We found one
						foundUser = true;


						// Add Recipients
						EFriendRelationship state = clientFriends->GetFriendRelationship(*recipients[i]);

						if (clientFriends->GetFriendRelationship(*recipients[i]) != k_EFriendRelationshipFriend)
						{
							if (state != k_EFriendRelationshipRequestRecipient)
							{
								if (state == k_EFriendRelationshipRequestInitiator)
								{
									smutils->LogError(myself, "Recipient %s is neither a friend nor he send an invite to the Bot nor he accepted the request!", *recipients[i]->Render());
								}
								else
								{
									clientFriends->AddFriend(*recipients[i]);
									
									smutils->LogError(myself, "Recipient %s is neither a friend nor he send an invite to the Bot. We sent him a request by the Bot!", *recipients[i]->Render());
								}

								continue;
							}

							// Add the friend
							clientFriends->AddFriend(*recipients[i]);


							// Wait until added
							state = clientFriends->GetFriendRelationship(*recipients[i]);

							while (state == k_EFriendRelationshipRequestRecipient)
							{
								Sleeping(10);

								state = clientFriends->GetFriendRelationship(*recipients[i]);
							}
						}


						int messageCountOld = clientFriends->GetChatMessagesCount(*recipients[i]);


						// Send him the message
						if (clientFriends->SendMsgToFriend(*recipients[i], k_EChatEntryTypeChatMsg, msg, strlen(msg + 1)))
						{
							foundData = true;
						}
						else if (clientFriends->ReplyToFriendMessage(*recipients[i], msg))
						{
							// We found one
							foundData = true;
						}

						if (foundData)
						{
							int messageCountNew = clientFriends->GetChatMessagesCount(*recipients[i]);

							while (messageCountOld >= messageCountNew)
							{
								Sleeping(10);

								messageCountNew = clientFriends->GetChatMessagesCount(*recipients[i]);
							}
						}
					}

					Sleeping(10);
				}

				// Array is empty?
				if (!foundUser)
				{
					// Array is Empty !
					error = 1;

					FreeLastCallback(steamPipe);
					LogoutOSW(false);


					return ARRAY_EMPTY;
				}

				// no receiver?
				else if (!foundData)
				{
					error = 1;

					FreeLastCallback(steamPipe);
					LogoutOSW(false);


					return NO_RECEIVER;
				}
				else
				{
					error = 1;

					FreeLastCallback(steamPipe);
					LogoutOSW(false);


					return SUCCESS;
				}
			}
			

			// Error on connect
			else if (callBack.m_iCallback == SteamServerConnectFailure_t::k_iCallback)
			{
				// Get Error Code
				SteamServerConnectFailure_t *errorState = (SteamServerConnectFailure_t *)callBack.m_pubParam;

				// We have a Login Error
				error = (int)errorState->m_eResult;

				
				FreeLastCallback(steamPipe);
				LogoutOSW(false);

				return LOGIN_ERROR;
			}

			FreeLastCallback(steamPipe);
		}

		Sleeping(100);
	}


	// Timeout
	error = 1;

	LogoutOSW(false);
	FreeLastCallback(steamPipe);

	return TIMEOUT_ERROR;
}




// Logout and clean up if necessary
void OSWClass::LogoutOSW(bool clean)
{
	// Logout
	if (clientUser)
	{
		clientUser->LogOff();

		ELogonState state = clientUser->GetLogonState();

		while (state != k_ELogonStateNotLoggedOn)
		{
			Sleeping(50);

			state = clientUser->GetLogonState();
		}
	}

	if (clean && steamPipe)
	{
		if (steamUser)
		{
			clientEngine->ReleaseUser(steamPipe, steamUser);
		}

		clientEngine->BReleaseSteamPipe(steamPipe);
	}

	// Not setup anymore
	setup = 0;
}




// Converts a steamid to a CSteamID
CSteamID* OSWClass::steamIDtoCSteamID(char* steamid)
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
	#if defined _WIN32
		uintID += 76561197960265728 + server;
	#elif defined _LINUX
		uintID += 76561197960265728LLU + server;
	#endif


	// Return it
	return new CSteamID(uintID);
}