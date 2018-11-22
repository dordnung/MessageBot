/**
 * -----------------------------------------------------
 * File         natives.cpp
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

#include "natives.h"
#include "Config.h"
#include "Message.h"
#include "MessageBot.h"
#include "MessageThread.h"

#include <sstream>
#include <vector>

enum MessageBot_Option {
    OPTION_DEBUG,
    OPTION_WAIT_BETWEEN_MESSAGES,
    OPTION_WAIT_AFTER_LOGOUT,
    OPTION_REQUEST_TIMEOUT,
    OPTION_SHUFFLE_RECIPIENTS,
    OPTION_MAX
};


cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params) {
    char *username;
    char *password;

    // Get username and password
    pContext->LocalToString(params[1], &username);
    pContext->LocalToString(params[2], &password);

    // Copy Strings to config variables
    messageBotConfig.username = username;
    messageBotConfig.password = password;

    return 1;
}

cell_t MessageBot_SendBotMessage(IPluginContext *pContext, const cell_t *params) {
    // Create a callback function from the given callback
    auto callback = messageBot.CreateCallbackFunction(pContext->GetFunctionById(params[1]));
    if (!callback) {
        pContext->ThrowNativeError("Callback ID %x is invalid", params[1]);
        return 0;
    }

    // Get the message text to send
    char *messageText;
    pContext->LocalToString(params[2], &messageText);

    // Create new message
    Message message;
    message.config = messageBotConfig;
    message.text = messageText;

    // Start a new thread
    MessageThread *messageThread = new MessageThread(message, callback);
    if (!messageBot.RegisterAndStartThread(messageThread)) {
        delete messageThread;

        pContext->ThrowNativeError("Couldn't create a new thread");
        return 0;
    }

    return 1;
}

cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params) {
    // Read steamId from params
    char *steamid;
    pContext->LocalToString(params[1], &steamid);

    // Valid community Id?
    uint64_t commId = MessageBot_SteamId2toSteamId64(steamid);
    if (!commId) {
        return 0;
    }

    // Check for duplicates
    for (auto recipient = messageBotConfig.recipients.begin(); recipient != messageBotConfig.recipients.end(); recipient++) {
        if (commId == *recipient) {
            return 0;
        }
    }

    // Append recipient if not found yet
    messageBotConfig.recipients.push_back(commId);

    return 1;
}

cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params) {
    // Read steamId from params
    char *steamid;
    pContext->LocalToString(params[1], &steamid);

    // Valid community Id?
    uint64_t commId = MessageBot_SteamId2toSteamId64(steamid);
    if (!commId) {
        return 0;
    }

    // Search for steamid
    for (auto recipient = messageBotConfig.recipients.begin(); recipient != messageBotConfig.recipients.end(); recipient++) {
        if (commId == *recipient) {
            // Delete if found
            messageBotConfig.recipients.erase(recipient);

            return 1;
        }
    }

    return 0;
}

cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params) {
    // Read steamId from params
    char *steamid;
    pContext->LocalToString(params[1], &steamid);

    // Valid community Id?
    uint64_t commId = MessageBot_SteamId2toSteamId64(steamid);
    if (!commId) {
        return 0;
    }

    // Search for steamid
    for (auto recipient = messageBotConfig.recipients.begin(); recipient != messageBotConfig.recipients.end(); recipient++) {
        if (commId == *recipient) {
            return 1;
        }
    }

    return 0;
}

cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params) {
    messageBotConfig.recipients.clear();
    return 1;
}

cell_t MessageBot_SetOption(IPluginContext *pContext, const cell_t *params) {
    int optionType = params[1];
    if (optionType >= OPTION_MAX) {
        pContext->ThrowNativeError("Option with value '%d' is invalid!", optionType);
        return 0;
    }

    MessageBot_Option option = static_cast<MessageBot_Option>(optionType);
    switch (option) {
        case OPTION_DEBUG:
            messageBotConfig.debugEnabled = params[2];
            break;
        case OPTION_WAIT_BETWEEN_MESSAGES:
            messageBotConfig.waitBetweenMessages = params[2];
            break;
        case OPTION_WAIT_AFTER_LOGOUT:
            messageBotConfig.waitAfterLogout = params[2];
            break;
        case OPTION_REQUEST_TIMEOUT:
            messageBotConfig.requestTimeout = params[2];
            break;
        case OPTION_SHUFFLE_RECIPIENTS:
            messageBotConfig.shuffleRecipients = params[2];
            break;
    }

    return 1;
}

cell_t MessageBot_GetOption(IPluginContext *pContext, const cell_t *params) {
    int optionType = params[1];
    if (optionType >= OPTION_MAX) {
        pContext->ThrowNativeError("Option with value '%d' is invalid!", optionType);
        return 0;
    }

    MessageBot_Option option = static_cast<MessageBot_Option>(optionType);
    switch (option) {
        case OPTION_DEBUG:
            return messageBotConfig.debugEnabled;
        case OPTION_WAIT_BETWEEN_MESSAGES:
            return messageBotConfig.waitBetweenMessages;
        case OPTION_WAIT_AFTER_LOGOUT:
            return messageBotConfig.waitAfterLogout;
        case OPTION_REQUEST_TIMEOUT:
            return messageBotConfig.requestTimeout;
        case OPTION_SHUFFLE_RECIPIENTS:
            return messageBotConfig.shuffleRecipients;
    }

    return 1;
}

uint64_t MessageBot_SteamId2toSteamId64(std::string steamId2) {
    // Maybe it's already a community Id
    if (steamId2.find(":") == std::string::npos) {
        return strtoull(steamId2.c_str(), nullptr, 10);
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

    uint64_t server = strtoull(strippedSteamId.at(1).c_str(), nullptr, 10);
    uint64_t authId = strtoull(strippedSteamId.at(2).c_str(), nullptr, 10);

    // Wrong format
    if (!authId) {
        return 0;
    }

    // Calculate community Id
#if defined _WIN32
    return  authId * 2 + 76561197960265728 + server;
#elif defined _LINUX
    return  authId * 2 + 76561197960265728LLU + server;
#endif
}