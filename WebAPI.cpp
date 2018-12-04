/**
 * -----------------------------------------------------
 * File         WebAPI.cpp
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

#include "WebAPI.h"
#include "Config.h"
#include "rsa/RSAKey.h"

#include <chrono>
#include <stdarg.h>
#include <string.h>
#include <algorithm>
#include <random>
#include <fstream>

#define CLIENT_ID "DE45CD61"
#define CLIENT_SCOPE "read_profile write_profile read_client write_client"

#define USER_AGENT_APP "Steam App / Android / 2.3.1 / 3922515"
#define USER_AGENT_ANDROID "Mozilla/5.0 (Linux; U; Android; en-gb;) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30"

#define MOBILE_CLIENT_COOKIE "mobileClient=android; path=/; domain=steamcommunity.com; secure"
#define MOBILE_CLIENT_VERSION_COOKIE "mobileClientVersion=3922515+%282.3.1%29; path=/; domain=steamcommunity.com; secure"
#define LANGUAGE_COOKIE "Steam_Language=english; path=/; domain=steamcommunity.com; secure"
#define STEAM_LOGIN_SECURE_COOKIE "steamLoginSecure=null%7C%7Cnull; path=/; domain=steamcommunity.com; secure"
#define STEAM_LOGIN_COOKIE "steamLogin=null%7C%7Cnull; path=/; domain=steamcommunity.com; secure"

#if defined _WIN32 || defined _WIN64
#define sleep_ms(x) Sleep(x);
#else
#define sleep_ms(x) usleep(x * 1000);
#endif

// Use server console if sourcemod build, otherwise use printf
#if defined SOURCEMOD_BUILD
#include "MessageBot.h"
#define LogError(fmt, ...) smutils->LogError(myself, fmt, ##__VA_ARGS__)
#define Debug(fmt, ...) if (this->debugEnabled) smutils->LogMessage(myself, fmt, ##__VA_ARGS__)
#else
#define LogError(fmt, ...) printf(fmt, ##__VA_ARGS__)
#define Debug(fmt, ...) if (this->debugEnabled) printf(fmt ## "\n", ##__VA_ARGS__)
#endif


WebAPI::WebAPI() : debugEnabled(false), requestTimeout(0), webAPIClient(nullptr), steamCommunityClient(nullptr) {
    this->steamCommunityClient = curl_easy_init();
    this->webAPIClient = curl_easy_init();
}

WebAPI::~WebAPI() {
    if (this->steamCommunityClient) {
        curl_easy_cleanup(this->steamCommunityClient);
    }

    if (this->webAPIClient) {
        curl_easy_cleanup(this->webAPIClient);
    }
}

Json::Value WebAPI::LoginSteamCommunity(std::string username, std::string password) {
    Debug("[DEBUG] Trying to login to the steam community");

    Json::Value result;
    Json::Reader reader;

    // First add needed cookies
    this->AddCookie(this->steamCommunityClient, MOBILE_CLIENT_COOKIE);
    this->AddCookie(this->steamCommunityClient, MOBILE_CLIENT_VERSION_COOKIE);
    this->AddCookie(this->steamCommunityClient, LANGUAGE_COOKIE);

    // Notify steam that we need oauth
    std::string sessionPage = "https://steamcommunity.com/mobilelogin?oauth_client_id=" + this->urlencode(CLIENT_ID) + "&oauth_scope=" + this->urlencode(CLIENT_SCOPE);

    WriteDataInfo pageInfo = this->GetPage(this->steamCommunityClient, sessionPage, USER_AGENT_ANDROID, nullptr);
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to receive SteamCommunity RSA key. Error: '" + pageInfo.error + "'";
        return result;
    }

    // Get the RSA key to login
    long long time = std::chrono::system_clock::now().time_since_epoch().count();
    pageInfo = this->GetPage(this->steamCommunityClient, "https://steamcommunity.com/mobilelogin/getrsakey", USER_AGENT_ANDROID,
                             "username=%s&donotcache=%lld", const_cast<char *>(this->urlencode(username).c_str()), time);

    // Check for errors
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to receive SteamCommunity RSA key. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse SteamCommunity RSA key. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    if (!result.get("success", false).asBool()) {
        std::string json = result.toStyledString();

        result["error"] = "Failed to get SteamCommunity RSA key. JSON: '" + json + "'";
        return result;
    }

    // Read RSA information
    std::string mod = result.get("publickey_mod", "").asString();
    std::string exp = result.get("publickey_exp", "").asString();
    std::string timestamp = result.get("timestamp", "").asString();

    if (mod.empty() || exp.empty() || timestamp.empty()) {
        result["success"] = false;
        result["error"] = "Failed to get SteamCommunity RSA Key information. Got: (mod, exp, timestamp) -> (" + mod + ", " + exp + ", " + timestamp + ")";
        return result;
    }

    // Now encrypt it with RSA
    RSAKey rsaKey(mod.c_str(), exp.c_str());
    std::string encrypted = rsaKey.Encrypt(password.c_str());

    // And login with it
    pageInfo = this->GetPage(this->steamCommunityClient, "https://steamcommunity.com/mobilelogin/dologin/", USER_AGENT_ANDROID,
                             "donotcache=%lld&password=%s&username=%s&twofactorcode=&emailauth=&loginfriendlyname=CallAdmin&captchagid=-1&captcha_text=&emailsteamid=&rsatimestamp=%s&remember_login=true&oauth_client_id=%s",
                             time, this->urlencode(encrypted).c_str(), this->urlencode(username).c_str(), timestamp.c_str(), CLIENT_ID);

    // Check for errors
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to login. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse login result. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    if (!result.get("success", false).asBool() || !result.get("login_complete", false).asBool()) {
        std::string json = result.toStyledString();

        result["success"] = false;
        result["error"] = "Failed to successfully login. JSON: '" + json + "'";
        return result;
    }

    // Read oauth from result
    std::string auth = result.get("oauth", "").asString();
    if (auth.empty()) {
        result["success"] = false;
        result["error"] = "Got empty oauth!";
        return result;
    }

    result.clear();
    if (!reader.parse(auth, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse oauth token. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    // And from there read steamid and oauth token
    if (result.get("steamid", "").asString().empty() || result.get("oauth_token", "").asString().empty()) {
        result["success"] = false;
        result["error"] = "Failed to get login steamid or oauth. Got: '" + auth + "'";
        return result;
    }

    Debug("[DEBUG] Logged in succesfully");

    result["success"] = true;
    return result;
}

Json::Value WebAPI::LoginWebAPI(std::string accessToken) {
    Debug("[DEBUG] Trying to login to the web API");

    Json::Value result;
    Json::Reader reader;

    // Login to get UMQID
    WriteDataInfo pageInfo = this->GetPage(this->webAPIClient, "https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logon/v0001",
                                           USER_AGENT_APP, "access_token=%s", accessToken.c_str());

    // Valid result?
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to receive UMQID. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse UMQID. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    std::string error = result.get("error", "").asString();
    if (error != "OK") {
        result["success"] = false;
        result["error"] = "Failed to get UMQID. Error: '" + error + "'";
        return result;
    }

    // Parse UMQID
    std::string umqid = result.get("umqid", "").asString();
    if (umqid.empty()) {
        std::string json = result.toStyledString();

        result["success"] = false;
        result["error"] = "Got empty UMQID. JSON: '" + json + "'";
        return result;
    }

    Debug("[DEBUG] Got UMQID");
    result["success"] = true;
    return result;
}

void WebAPI::LogoutWebAPI() {
    Debug("[DEBUG] Trying to logout");

    // Invalidate community cookies
    this->AddCookie(this->steamCommunityClient, STEAM_LOGIN_SECURE_COOKIE);
    this->AddCookie(this->steamCommunityClient, STEAM_LOGIN_COOKIE);

    // Just go back to session page with cookies notifying logout
    std::string sessionPage = "https://steamcommunity.com/mobilelogin?oauth_client_id=" + this->urlencode(CLIENT_ID) + "&oauth_scope=" + this->urlencode(CLIENT_SCOPE);
    this->GetPage(this->steamCommunityClient, sessionPage, USER_AGENT_ANDROID, nullptr);

    Debug("[DEBUG] Logged out");
}

Json::Value WebAPI::GetFriendList(std::string accessToken) {
    Debug("[DEBUG] Trying to get friend list");

    Json::Value result;
    Json::Reader reader;

    std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetFriendList/v0001";
    url = url + "?access_token=" + accessToken + "&relationship=friend,requestrecipient";

    // Read the friend list of the bot
    WriteDataInfo pageInfo = this->GetPage(this->webAPIClient, url, USER_AGENT_APP, nullptr);

    // Valid result?
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to receive friend list. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse friend list. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    Debug("[DEBUG] Got friend list");
    result["success"] = true;
    return result;
}


Json::Value WebAPI::GetUserStats(std::string accessToken, std::vector<uint64_t> &users) {
    Debug("[DEBUG] Trying to get user stats");

    Json::Value result;
    Json::Reader reader;

    std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetUserSummaries/v0001";
    url = url + "?access_token=" + accessToken + "&steamids=";

    // Append all users to the request
    bool isFirst = true;
    for (auto user = users.begin(); user != users.end(); user++) {
        if (isFirst) {
            isFirst = false;
            url = url + std::to_string(*user);
        } else {
            url = url + "," + std::to_string(*user);
        }
    }

    // Get user stats of all users
    WriteDataInfo pageInfo = this->GetPage(this->webAPIClient, url, USER_AGENT_APP, nullptr);

    // Valid result?
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to receive user stats. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse user stats. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    Debug("[DEBUG] Got user stats");
    result["success"] = true;
    return result;
}


Json::Value WebAPI::AcceptFriend(std::string sessionId, std::string ownSteamId, std::string friendSteamId) {
    Debug("[DEBUG] Trying to accept friend with steamid '%s'", friendSteamId.c_str());

    Json::Value result;
    Json::Reader reader;

    // Accept the friend with a AJAX request
    std::string url = "https://steamcommunity.com/profiles/" + ownSteamId + "/friends/action";
    WriteDataInfo pageInfo = this->GetPage(this->steamCommunityClient, url, USER_AGENT_ANDROID,
                                           "sessionid=%s&steamid=%s&ajax=1&action=accept&steamids[]=%s", sessionId.c_str(), ownSteamId.c_str(), friendSteamId.c_str());

    // Valid result?
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to accept friend. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse friend accept. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    if (result.get("success", false).asInt() != 1) {
        std::string json = result.toStyledString();

        result["success"] = false;
        result["error"] = "Failed to accept friend. JSON: '" + json + "'";
        return result;
    }

    Debug("[DEBUG] Friend accepted");
    result["success"] = true;
    return result;
}

Json::Value WebAPI::SendSteamMessage(std::string accessToken, std::string umqid, uint64_t steamid, std::string text) {
    Debug("[DEBUG] Trying to send a message to '%lld'", steamid);

    Json::Value result;
    Json::Reader reader;

    // Send the message
    WebAPI::WriteDataInfo pageInfo = this->GetPage(this->webAPIClient, "https://api.steampowered.com/ISteamWebUserPresenceOAuth/Message/v0001",
                                                   USER_AGENT_APP, "access_token=%s&umqid=%s&type=saytext&steamid_dst=%lld&text=%s",
                                                   accessToken.c_str(), umqid.c_str(), steamid, urlencode(text).c_str());

    // Valid result?
    if (!pageInfo.error.empty()) {
        result["success"] = false;
        result["error"] = "Failed to send message. Error: '" + pageInfo.error + "'";
        return result;
    }

    result.clear();
    if (!reader.parse(pageInfo.content, result)) {
        result["success"] = false;
        result["error"] = "Failed to parse sent message result. Error: '" + reader.getFormattedErrorMessages() + "'";
        return result;
    }

    std::string error = result.get("error", "").asString();
    if (error != "OK") {
        result["success"] = false;
        result["error"] = "Failed to send message. Error: '" + error + "'";
        return result;
    }

    Debug("[DEBUG] Sent message");
    result["success"] = true;
    return result;
}

WebAPIResult_t WebAPI::SendSteamMessage(Message message) {
    this->debugEnabled = message.config.debugEnabled;
    this->requestTimeout = message.config.requestTimeout;


    std::vector<uint64_t> recipientsCopy(message.config.recipients);
    if (message.config.shuffleRecipients) {
        std::random_device randomDevice;
        std::mt19937 randomEngine(randomDevice());

        std::shuffle(recipientsCopy.begin(), recipientsCopy.end(), randomEngine);
        Debug("[DEBUG] Shuffled recipient list");
    }

    Debug("[DEBUG] Trying to send a message to user '%s' with password '%s' and message '%s'", message.config.username.c_str(), message.config.password.c_str(), message.text.c_str());

    WebAPIResult_t result;

    // No recipient?
    if (recipientsCopy.size() == 0) {
        Debug("[DEBUG] Couldn't send message, as no recipients are defined");

        result.type = WebAPIResult_NO_RECEIVER;
        result.error = "No receiver was configurated";
        return result;
    }

    Json::Value loginSteamCommunityResult = this->LoginSteamCommunity(message.config.username, message.config.password);
    if (!loginSteamCommunityResult["success"].asBool()) {
        LogError(loginSteamCommunityResult["error"].asString().c_str());

        result.type = WebAPIResult_LOGIN_ERROR;
        result.error = loginSteamCommunityResult["error"].asString();
        return result;
    }

    std::string accessToken = loginSteamCommunityResult["oauth_token"].asString();
    std::string steamid = loginSteamCommunityResult["steamid"].asString();
    std::string sessionid = this->GetCookie(this->steamCommunityClient, "sessionid");

    Json::Value loginWebAPIResult = this->LoginWebAPI(accessToken);
    if (!loginWebAPIResult["success"].asBool()) {
        LogError(loginWebAPIResult["error"].asString().c_str());

        result.type = WebAPIResult_LOGIN_ERROR;
        result.error = loginSteamCommunityResult["error"].asString();
        return result;
    }

    std::string umqid = loginWebAPIResult["umqid"].asString();

    // Get friend list
    Json::Value friendListResult = this->GetFriendList(accessToken);
    if (!friendListResult["success"].asBool()) {
        LogError(friendListResult["error"].asString().c_str());

        result.type = WebAPIResult_API_ERROR;
        result.error = loginSteamCommunityResult["error"].asString();
        return result;
    }

    // Accept all friends
    Json::Value friendValue = friendListResult.get("friends", "");

    for (int i = 0; friendValue.isValidIndex(i); i++) {
        std::string steam = friendValue[i].get("steamid", "").asString();
        std::string relation = friendValue[i].get("relationship", "").asString();

        if (relation == "requestrecipient") {
            // Accept the friend if there is a request
            this->AcceptFriend(sessionid, steamid, steam);

            // Add a second timeout, as otherwise two consecutive requests can fail!
            sleep_ms(1000);
        }
    }

    // Get user stats
    Json::Value userStatsResult = this->GetUserStats(accessToken, recipientsCopy);
    if (!userStatsResult["success"].asBool()) {
        LogError(userStatsResult["error"].asString().c_str());

        result.type = WebAPIResult_API_ERROR;
        result.error = loginSteamCommunityResult["error"].asString();
        return result;
    }

    // Check if there is valid recipient which is online
    Json::Value userValues = userStatsResult.get("players", "");
    for (auto recipient = recipientsCopy.begin(); recipient != recipientsCopy.end(); recipient++) {
        for (int i = 0; userValues.isValidIndex(i); i++) {
            std::string steam = userValues[i].get("steamid", "").asString();
            int online = userValues[i].get("personastate", 0).asInt();

            if (steam == std::to_string(*recipient) && online) {
                // Send the message to the recipient
                Json::Value sendMessageResult = this->SendSteamMessage(accessToken, umqid, *recipient, message.text);
                if (!sendMessageResult["success"].asBool()) {
                    LogError(sendMessageResult["error"].asString().c_str());
                    this->LogoutWebAPI();

                    // Wait after logout, as steam needs a few seconds until logout is complete
                    sleep_ms(message.config.waitAfterLogout);

                    result.type = WebAPIResult_API_ERROR;
                    result.error = loginSteamCommunityResult["error"].asString();
                    return result;
                }

                // Wait between messages, as the user may occur some limitations on how much messages he can send
                sleep_ms(message.config.waitBetweenMessages);
                break;
            }
        }
    }

    // Logout on finish
    this->LogoutWebAPI();

    // Wait after logout, as steam needs a few seconds until logout is complete
    sleep_ms(message.config.waitAfterLogout);

    Debug("[DEBUG] Sent message");

    result.type = WebAPIResult_SUCCESS;
    result.error = std::string();
    return result;
}

WebAPI::WriteDataInfo WebAPI::GetPage(CURL *client, std::string url, std::string useragent, char *post, ...) {
    // First reset the curl handle
    curl_easy_reset(client);

    // Process post list
    char postStr[1024];
    if (post) {
        va_list argptr;

        va_start(argptr, post);
        vsprintf(postStr, post, argptr);
        va_end(argptr);
    } else {
        postStr[0] = '\0';
    }

    // Set URL
    curl_easy_setopt(client, CURLOPT_URL, url.c_str());

    // Disable SSL verifying for peer
    curl_easy_setopt(client, CURLOPT_SSL_VERIFYPEER, 0L);

#if defined unix || defined __unix__ || defined __linux__ || defined __unix || defined __APPLE__ || defined __darwin__
    // Use our own ca-bundle on unix like systems
    char caPath[PLATFORM_MAX_PATH + 1];
    smutils->BuildPath(Path_SM, caPath, sizeof(caPath), "data/messagebot/ca-bundle.crt");

    static bool caErrorReported = false;
    if (std::ifstream(caPath).good())
    {
        curl_easy_setopt(client, CURLOPT_CAINFO, caPath);
    }
    else if (!caErrorReported)
    {
        LogError("File 'ca-bundle.crt' is missing from 'sourcemod/data/messagebot/' folder, please install it")
        caErrorReported = true;
    }
#endif

    // Set the write function and data
    WriteDataInfo writeData;
    curl_easy_setopt(client, CURLOPT_WRITEFUNCTION, WebAPI::WriteData);
    curl_easy_setopt(client, CURLOPT_WRITEDATA, &writeData);

    // Set timeout
    curl_easy_setopt(client, CURLOPT_TIMEOUT, this->requestTimeout);

    // Prevent signals to interrupt our thread
    curl_easy_setopt(client, CURLOPT_NOSIGNAL, 1L);

    // Collect error information
    char errorBuffer[CURL_ERROR_SIZE + 1];
    curl_easy_setopt(client, CURLOPT_ERRORBUFFER, errorBuffer);

    // Set the http user agent
    curl_easy_setopt(client, CURLOPT_USERAGENT, useragent.c_str());

    // Disable following redirects
    curl_easy_setopt(client, CURLOPT_FOLLOWLOCATION, 0L);

    // Append post data
    struct curl_slist *chunk = nullptr;
    if (strlen(postStr) > 0) {
        chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
        chunk = curl_slist_append(chunk, (std::string("Content-length: ") + std::to_string(strlen(postStr))).c_str());

        curl_easy_setopt(client, CURLOPT_POSTFIELDS, postStr);
    }

    // Enable cookie tracking
    curl_easy_setopt(client, CURLOPT_COOKIEFILE, "");

    // Add all HTTP headers
    if (chunk) {
        curl_easy_setopt(client, CURLOPT_HTTPHEADER, chunk);
    }

    Debug("[DEBUG] Request to '%s' with data '%s'", url.c_str(), postStr);

    if (this->debugEnabled) {
#if !defined SOURCEMOD_BUILD
        // Enable verbose for non sourcemod build
        curl_easy_setopt(client, CURLOPT_VERBOSE, 1L);
#endif
        // Debug all cookies
        struct curl_slist *cookies = nullptr;
        if (curl_easy_getinfo(client, CURLINFO_COOKIELIST, &cookies) == CURLE_OK && cookies) {
            struct curl_slist *cookie = cookies;
            while (cookie) {
                Debug("Cookie: '%s'", cookie->data);
                cookie = cookie->next;
            }

            curl_slist_free_all(cookies);
        }
    }

    // Perform curl request
    if (curl_easy_perform(client) != CURLE_OK) {
        writeData.content = errorBuffer;
        writeData.error = errorBuffer;
    }

    // Clean up curl
    if (chunk) {
        curl_slist_free_all(chunk);
    }

    Debug("[DEBUG] Response from '%s' with content '%s'", url.c_str(), writeData.content.c_str());

    // Return result
    return writeData;
}

void WebAPI::AddCookie(CURL *client, std::string cookie) {
    curl_easy_setopt(client, CURLOPT_COOKIELIST, ("Set-Cookie: " + cookie).c_str());
}

std::string WebAPI::GetCookie(CURL *client, std::string cookieName) {
    struct curl_slist *cookies = nullptr;

    if (curl_easy_getinfo(client, CURLINFO_COOKIELIST, &cookies) == CURLE_OK && cookies) {
        struct curl_slist *cookie = cookies;
        while (cookie) {
            std::string data = cookie->data;
            if (data.find(cookieName + "\t") != std::string::npos) {
                return data.substr(data.find(cookieName + "\t") + cookieName.length() + 1);
            }

            cookie = cookie->next;
        }

        curl_slist_free_all(cookies);
    }

    return std::string();
}

std::string WebAPI::urlencode(std::string str) {
    CURL *curl = curl_easy_init();
    std::string ret = "";

    if (curl) {
        char *escaped = curl_easy_escape(curl, str.c_str(), str.length());
        if (escaped) {
            ret = escaped;
            curl_free(escaped);
        }

        curl_easy_cleanup(curl);
    }

    return ret;
}

size_t WebAPI::WriteData(char *ptr, size_t size, size_t nmemb, void *userdata) {
    // Get the data info
    WebAPI::WriteDataInfo *dataInfo = static_cast<WebAPI::WriteDataInfo *>(userdata);

    // Add to content
    size_t realsize = size * nmemb;
    dataInfo->content.append(ptr, realsize);

    return realsize;
}