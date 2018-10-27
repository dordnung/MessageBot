/**
 * -----------------------------------------------------
 * File         MessageBot.h
 * Authors      David Ordnung, Impact
 * License      GPLv3
 * Web          http://dordnung.de, http://gugyclan.eu
 * -----------------------------------------------------
 *
 * Originally provided for CallAdmin by David Ordnung and Impact
 *
 * Copyright (C) 2014-2018 David Ordnung, Impact
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

#ifndef _MESSAGE_BOT_H_
#define _MESSAGE_BOT_H_

#include "sdk/smsdk_ext.h"
#include "Callback.h"
#include "CallbackFunction.h"
#include "Message.h"
#include "MessageThread.h"
#include "WebAPIResult.h"

#include <string>
#include <deque>
#include <vector>

class MessageBot : public SDKExtension, public IPluginsListener {
private:
    IMutex *mutex;

    std::deque<std::shared_ptr<Callback>> callbackQueue;
    std::vector<std::shared_ptr<CallbackFunction_t>> callbackFunctions;
    std::deque<IThreadHandle *> waitingThreads;

    IThreadHandle *runningThread;
    IThreadHandle *messageThread;

    bool isRunning;

    std::string username;
    std::string password;

    std::vector<uint64_t> recipients;
    bool debugEnabled;

public:
    MessageBot();

    virtual bool SDK_OnLoad(char *error, size_t maxlength, bool late);
    virtual void SDK_OnUnload();
    virtual void OnPluginUnloaded(IPlugin *plugin);

    void AppendCallback(std::shared_ptr<Callback> callback);
    std::shared_ptr<CallbackFunction_t> CreateCallbackFunction(IPluginFunction *function);

    bool RegisterAndStartThread(IThread *thread);
    void UnregisterAndDeleteThreadHandle(IThreadHandle *threadHandle);

    void OnGameFrameHit(bool simulating);

    cell_t SetLoginData(IPluginContext *pContext, const cell_t *params);
    cell_t SendBotMessage(IPluginContext *pContext, const cell_t *params);
    cell_t AddRecipient(IPluginContext *pContext, const cell_t *params);
    cell_t RemoveRecipient(IPluginContext *pContext, const cell_t *params);
    cell_t IsRecipient(IPluginContext *pContext, const cell_t *params);
    cell_t ClearRecipients(IPluginContext *pContext, const cell_t *params);
    cell_t SetDebugStatus(IPluginContext *pContext, const cell_t *params);

    WebAPIResult_t SendSteamMessage(Message message);
    uint64_t SteamId2toSteamId64(std::string steamId2);
};

void MessageBot_OnGameFrameHit(bool simulating);

extern MessageBot messageBot;

#endif