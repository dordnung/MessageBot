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


#ifndef _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_
#define _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_



// Max recipients
#define MAX_RECIPIENTS 300

// Max message length
#define MAX_MESSAGE_LENGTH 4096



// Includes
#include <stdlib.h>
#include <sstream>
#include <Steamworks.h>

#include "smsdk_ext.h"
#include "sh_vector.h"



// Sleeping for Linux and Windows
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
	NO_RECEIVER,
};



// enum for SendMethod
enum SendMethod
{
	SEND_METHOD_STEAMWORKS = 0,
	SEND_METHOD_ONLINEAPI,
};






// Extension class
class MessageBot : public SDKExtension
{
public:
	virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
	virtual void SDK_OnUnload();

public:
	// Prepare Forward
	void prepareForward(IPluginFunction *func, CallBackResult result, int errorS);
};





// message Queue 
class Queue
{
private:
	IPluginFunction *callback;
	bool showLogin;

	char user[128];
	char pass[128];
	char txt[MAX_MESSAGE_LENGTH];

	Queue *next;

public:
	Queue(IPluginFunction *func, char *name, char *pw, char *message, int online);

	
	// get methods
	char *getUsername() const;
	char *getPassword() const;
	char *getMessage() const;

	Queue *getNext() const;
	IPluginFunction *getCallback() const;
	bool getOnline() const;

	
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
	int errorState;

} PawnFuncThreadReturn;




// OSW Class
extern CSteamID *recipients[MAX_RECIPIENTS];
extern SendMethod sendMethod;


// Are we loaded?
extern bool extensionLoaded;



// Natives
cell_t MessageBot_SetSendMethod(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SendMessage(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params);



void OnGameFrameHit(bool simulating);


#endif // _INCLUDE_SOURCEMOD_EXTENSION_PROPER_H_