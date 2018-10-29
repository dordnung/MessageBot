/**
 * -----------------------------------------------------
 * File         WebAPI.h
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

#ifndef _WEB_API_H_
#define _WEB_API_H_

#include "3rdparty/json/json/json.h"
#include "Message.h"
#include "WebAPIResult.h"

#include <curl/curl.h>
#include <vector>
#include <map>
#include <string>

class WebAPI {
private:
    bool debugEnabled;
    int requestTimeout;

    CURL *webAPIClient;
    CURL *steamCommunityClient;

public:
    WebAPI();
    ~WebAPI();

    WebAPIResult_t SendSteamMessage(Message message);

private:
    typedef struct {
        std::string content;
        std::string error;
    } WriteDataInfo;

    Json::Value LoginSteamCommunity(std::string username, std::string password);
    Json::Value LoginWebAPI(std::string accessToken);
    void LogoutWebAPI();

    Json::Value GetFriendList(std::string accessToken);
    Json::Value GetUserStats(std::string accessToken, std::vector<uint64_t> &users);
    Json::Value AcceptFriend(std::string sessionId, std::string ownSteamId, std::string friendSteamId);
    Json::Value SendSteamMessage(std::string accessToken, std::string umqid, uint64_t steamid, std::string text);

    WebAPI::WriteDataInfo GetPage(CURL *client, std::string url, std::string userAgent, char *post, ...);
    std::string urlencode(std::string str);

    void AddCookie(CURL *client, std::string cookie);
    std::string GetCookie(CURL *client, std::string cookieName);

    static size_t WriteData(char *ptr, size_t size, size_t nmemb, void *userdata);
};

#endif