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
#include "MessageBot.h"

cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params) {
    return messageBot.SetLoginData(pContext, params);
}

cell_t MessageBot_SendBotMessage(IPluginContext *pContext, const cell_t *params) {
    return messageBot.SendBotMessage(pContext, params);
}

cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params) {
    return messageBot.AddRecipient(pContext, params);
}

cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params) {
    return messageBot.RemoveRecipient(pContext, params);
}

cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params) {
    return messageBot.IsRecipient(pContext, params);
}

cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params) {
    return messageBot.ClearRecipients(pContext, params);
}

cell_t MessageBot_SetDebugStatus(IPluginContext *pContext, const cell_t *params) {
    return messageBot.SetDebugStatus(pContext, params);
}