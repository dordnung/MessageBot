/**
 * -----------------------------------------------------
 * File         MessageThread.h
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

#ifndef _MESSAGE_THREAD_H_
#define _MESSAGE_THREAD_H_

#include "sdk/smsdk_ext.h"
#include "CallbackFunction.h"
#include "Message.h"

#include <memory>

class MessageThread : public IThread {
private:
    Message message;
    std::shared_ptr<CallbackFunction_t> callbackFunction;

public:
    MessageThread(Message message, std::shared_ptr<CallbackFunction_t> callbackFunction);

    void RunThread(IThreadHandle *pThread);
    void OnTerminate(IThreadHandle *pThread, bool cancel);
};

#endif
