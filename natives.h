/**
 * -----------------------------------------------------
 * File         natives.h
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

#ifndef _NATIVES_H_
#define _NATIVES_H_

#include "sdk/smsdk_ext.h"
#include <string>

cell_t MessageBot_SetLoginData(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SendBotMessage(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_AddRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_RemoveRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_IsRecipient(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_ClearRecipients(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_SetOption(IPluginContext *pContext, const cell_t *params);
cell_t MessageBot_GetOption(IPluginContext *pContext, const cell_t *params);

uint64_t MessageBot_SteamId2toSteamId64(std::string steamId2);

static sp_nativeinfo_t messagebot_natives[] =
{
    { "MessageBot_SendMessage", MessageBot_SendBotMessage },
    { "MessageBot_SetLoginData", MessageBot_SetLoginData },
    { "MessageBot_AddRecipient", MessageBot_AddRecipient },
    { "MessageBot_RemoveRecipient", MessageBot_RemoveRecipient },
    { "MessageBot_IsRecipient", MessageBot_IsRecipient },
    { "MessageBot_ClearRecipients", MessageBot_ClearRecipients },
    { "MessageBot_SetOption", MessageBot_SetOption },
    { "MessageBot_GetOption", MessageBot_GetOption },
    { NULL, NULL }
};

#endif
