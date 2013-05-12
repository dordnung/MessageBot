/**
 * -----------------------------------------------------
 * File			extension.h
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


#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_



// Max recipients
#define MAX_RECIPIENTS 300


// Max recipients
#define MAX_MESSAGE_LENGTH 1024


// Includes
#include <Steamworks.h>
#include <stdlib.h>

#include "smsdk_ext.h"




// Sleeping
#if defined _WIN32
	#define Sleeping(x) Sleep(x);
#elif defined _LINUX
	#define Sleeping(x) usleep(x*1000);
#endif






// enum for Result
enum CallBackResult
{
	SUCCESS = 0,
	LOGIN_ERROR,
	TIMEOUT_ERROR,
	ARRAY_EMPTY,
	NO_RECEIVER
};





// Extension class
class MessageBot : public SDKExtension
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();
};






// message Queue 
class Queue
{
private:
	IPluginFunction *callback;
	EPersonaState state;

	char user[128];
	char pass[128];
	char txt[MAX_MESSAGE_LENGTH];

	Queue *next;

public:
	Queue(IPluginFunction *func, char *name, char *pw, char *message, int online);

	
	// get methods
	char *getUsername() const;
	char *getPassword() const;
	char *getMessage()const ;

	Queue *getNext() const ;
	IPluginFunction *getCallback() const;
	EPersonaState getOnline() const;

	
	// Remove last item
	void remove();

	// Add new item at the end
	void add(Queue *newQueue);
};





// Thread wachting for new messages
class watchThread : public IThread
{
public:
	watchThread() : IThread() {};

	void RunThread(IThreadHandle *pThread);
	void OnTerminate(IThreadHandle *pThread, bool cancel) {};
};





// Struct for forwards
typedef struct
{
public:
	IPluginFunction *pFunc;
	CallBackResult result;
	cell_t error;

} PawnFuncThreadReturn;







// Steam Methods
typedef bool (*GetCallbackFn)(HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg);
typedef void (*FreeLastCallbackFn)(HSteamPipe hSteamPipe);



// OnGameFrame
void OnGameFrameHit(bool simulating);


// Prepare Forward
void prepareForward(IPluginFunction *func, CallBackResult result, cell_t error = 0);


// Natives
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params);


// Steamid to CSteamID
CSteamID *steamIDtoCSteamID (char *steamid);



#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_