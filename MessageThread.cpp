/**
 * -----------------------------------------------------
 * File         MessageThread.cpp
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

#include "MessageThread.h"
#include "MessageBot.h"
#include "Callback.h"

MessageThread::MessageThread(Message message, std::shared_ptr<CallbackFunction_t> callbackFunction) :
    message(message), callbackFunction(callbackFunction) {}

void MessageThread::RunThread(IThreadHandle *pHandle) {
    // Send message via Webapi
    WebAPIResult_t result = messageBot.SendSteamMessage(message);

    // Add callback to queue
    messageBot.AppendCallback(std::make_shared<Callback>(this->callbackFunction, result.type, result.error));
}

void MessageThread::OnTerminate(IThreadHandle *pThread, bool cancel) {
    messageBot.UnregisterAndDeleteThreadHandle(pThread);
    delete this;
}