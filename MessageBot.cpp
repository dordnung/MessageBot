/**
 * -----------------------------------------------------
 * File         MessageBot.cpp
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

#include "MessageBot.h"
#include "natives.h"
#include "Config.h"

#include <curl/curl.h>

#if defined _WIN32 || defined _WIN64
#define sleep_ms(x) Sleep(x);
#else
#define sleep_ms(x) usleep(x * 1000);
#endif


MessageBot::MessageBot() {
    this->mutex = nullptr;
    this->isRunning = false;
    this->runningThread = nullptr;
    this->messageThread = nullptr;
}

bool MessageBot::SDK_OnLoad(char *error, size_t maxlength, bool late) {
    this->isRunning = true;

    // Creates needed mutex
    this->mutex = threader->MakeMutex();

    // Add natives and register library
    sharesys->AddNatives(myself, messagebot_natives);
    sharesys->RegisterLibrary(myself, "messagebot");

    // Register game frame hook
    smutils->AddGameFrameHook(&MessageBot_OnGameFrameHit);

    // Add this plugin listener
    plsys->AddPluginsListener(this);

    // Init CURL
    curl_global_init(CURL_GLOBAL_ALL);

    // Loaded
    return true;
}

void MessageBot::SDK_OnUnload() {
    this->mutex->Lock();

    // Mark that we are not running anymore
    this->isRunning = false;

    // Remove game frame hook so no callback will run anymore
    smutils->RemoveGameFrameHook(&MessageBot_OnGameFrameHit);

    this->mutex->Unlock();

    // Wait until running thread is finished
    if (runningThread) {
        rootconsole->ConsolePrint("[MessageBot] Please wait until running thread is finished...");
        this->runningThread->WaitForThread();
        this->runningThread = nullptr;
        rootconsole->ConsolePrint("[MessageBot] Thread finished executing");
    }

    // Remove all waiting threads
    for (auto it = this->waitingThreads.begin(); it != waitingThreads.end(); ++it) {
        (*it)->DestroyThis();
    }

    // Remove plugin listener
    plsys->RemovePluginsListener(this);

    // Clear STL stuff
    this->callbackQueue.clear();
    this->callbackFunctions.clear();
    this->waitingThreads.clear();

    // Remove created mutex
    this->mutex->DestroyThis();

    // Reset the message thread value
    this->messageThread = nullptr;

    // Reset config values at end
    messageBotConfig.ResetConfig();

    // Finally clean up CURL
    curl_global_cleanup();
}

void MessageBot::OnPluginUnloaded(IPlugin *plugin) {
    // Search if the plugin has any pending callback functions and invalidate them
    for (auto it = this->callbackFunctions.begin(); it != callbackFunctions.end();) {
        if ((*it)->plugin == plugin) {
            // Mark it as invalid and remove it from the list
            (*it)->isValid = false;
            it = this->callbackFunctions.erase(it);
        } else {
            ++it;
        }
    }
}

void MessageBot::AppendCallback(std::shared_ptr<Callback> callback) {
    // Lock mutex to gain thread safety
    while (!this->mutex->TryLock()) {
        sleep_ms(1);
    }

    if (this->isRunning) {
        // Add the callback to the queue and unlock mutex again
        this->callbackQueue.push_back(callback);
    }

    this->mutex->Unlock();
}

std::shared_ptr<CallbackFunction_t> MessageBot::CreateCallbackFunction(IPluginFunction *function) {
    if (!function || !function->IsRunnable()) {
        // Function is not valid
        return nullptr;
    }

    auto plugin = plsys->FindPluginByContext(function->GetParentRuntime()->GetDefaultContext()->GetContext());
    if (!plugin) {
        // Plugin is not valid
        return nullptr;
    }

    // Check if we already have the callback function
    for (auto it = this->callbackFunctions.begin(); it != callbackFunctions.end(); ++it) {
        if ((*it)->function == function) {
            auto callbackFunction = (*it);
            callbackFunction->plugin = plugin;
            callbackFunction->isValid = true;

            // Reuse the callback function
            return callbackFunction;
        }
    }

    auto callbackFunction = std::make_shared<CallbackFunction_t>();
    callbackFunction->plugin = plugin;
    callbackFunction->function = function;
    callbackFunction->isValid = true;

    // Add to the internal list of callback functions
    this->callbackFunctions.push_back(callbackFunction);
    return callbackFunction;
}


bool MessageBot::RegisterAndStartThread(IThread *thread) {
    // Create the thread suspended, add it to the list of waiting threads
    IThreadHandle *handle = threader->MakeThread(thread, Thread_CreateSuspended);
    if (!handle) {
        return false;
    }

    this->mutex->Lock();
    this->waitingThreads.push_back(handle);
    this->mutex->Unlock();

    return true;
}

void MessageBot::UnregisterAndDeleteThreadHandle(IThreadHandle *threadHandle) {
    while (!this->mutex->TryLock()) {
        sleep_ms(1);
    }

    // Destroy the thread handle to free resources
    this->runningThread = nullptr;
    threadHandle->DestroyThis();

    this->mutex->Unlock();
}

void MessageBot::OnGameFrameHit(bool simulating) {
    // Lock the mutex to gain thread safety
    if (!this->mutex->TryLock()) {
        // Couldn't lock -> do not wait
        return;
    }

    // Are there outstandig callbacks?
    std::shared_ptr<Callback> callback = nullptr;
    if (this->isRunning && !this->callbackQueue.empty()) {
        callback = this->callbackQueue.front();

        // Remove the callback from the queue
        // No deleting needed, as callbacks are shared pointers
        callbackQueue.pop_front();
    }

    // Are there waiting threads?
    IThreadHandle *waitingThread = nullptr;
    if (this->isRunning && !this->runningThread && !this->waitingThreads.empty()) {
        waitingThread = this->waitingThreads.front();

        // Remove the waiting thread from the queue
        waitingThreads.pop_front();
    }

    // Unlock mutex
    this->mutex->Unlock();

    // Proccess callback and thread outside mutex lock to avoid infinite loop
    if (callback) {
        if (callback->callbackFunction->isValid && callback->callbackFunction->function->IsRunnable()) {
            // Fire the callback if the callback function is valid
            callback->Fire();
        }
    }

    if (waitingThread) {
        this->runningThread = waitingThread;
        waitingThread->Unpause();
    }
}

void MessageBot_OnGameFrameHit(bool simulating) {
    messageBot.OnGameFrameHit(simulating);
}


// Create and link the extension
MessageBot messageBot;
SMEXT_LINK(&messageBot);