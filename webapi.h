/**
* -----------------------------------------------------
* File			webapi.h
* Authors		David O., Impact
* License		GPLv3
* Web			http://popoklopsi.de, http://gugyclan.eu
* -----------------------------------------------------
*
* Originally provided for CallAdmin by Popoklopsi and Impact
*
* Copyright (C) 2014-2016 David O., Impact
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

#ifndef _INCLUDE_WEBAPI_H_
#define _INCLUDE_WEBAPI_H_

#include <list>
#include <stdint.h>

#include "rsa.h"
#include "json/json.h"
#include "curl/curl.h"

#define DEBUG_WEBAPI 0
#define CLIENT_ID "DE45CD61"
#define CLIENT_SCOPE "read_profile write_profile read_client write_client"
#define USER_AGENT_APP "Steam App / Android / 1.0.6 / 1328177"
#define USER_AGENT_ANDROID "Mozilla/5.0 (Linux; U; Android; en-gb;) AppleWebKit/534.30 (KHTML, like Gecko) Version/4.0 Mobile Safari/534.30"
#define WEB_COOKIE "forceMobile=1;mobileClient=android;mobileClientVersion=1328177;Steam_Language=english;"


// enum for Result
enum CallBackResult {
	CallBackResult_SUCCESS = 0,
	CallBackResult_LOGIN_ERROR,
	CallBackResult_TIMEOUT_ERROR,
	CallBackResult_ARRAY_EMPTY,
	CallBackResult_NO_RECEIVER,
};


// Struct for result
typedef struct {
	// Chars
	std::string resultString;
	std::string resultHeader;
	char curlError[CURL_ERROR_SIZE + 1];

} CurlReturn;


// Working with WebAPI
class WebAPIClass {
private:
	bool loggedIn;
	int lastMessage;

	CURL *curl;

	std::string username;
	std::string password;
	std::string oauth;
	std::string steamid;
	std::string umqid;

	Json::Value friendList;
	Json::Value onlineStates;

public:
	WebAPIClass() : loggedIn(false), lastMessage(0), curl(NULL), username(""), password(""), oauth(""), steamid(""), umqid("") {
		curl_global_init(CURL_GLOBAL_ALL);
	}

	~WebAPIClass() {
		// Clean
		curl_global_cleanup();
	}


	// Send the message throw the web API
	CallBackResult SendMessageWebAPI(std::string username, std::string password, std::string message, bool showOnline, std::list<uint64_t> &recipients);

private:
	// Setup web api stuff
	bool LoginWebAPI();

	// Get UmqID
	bool LoginUMQID();

	// Get FriendList
	void GetFriendList();

	// Get Userstats
	void GetUserStats(std::list<uint64_t> &recipients);

	// Show online
	void ShowOnline();

	// Show online
	void AcceptFriend(std::string friendSteam, std::string &sessionID);

	// Logout 
	void LogoutWebAPI();

	// Get a Page
	CurlReturn GetPage(const char* url, const char* userAgent, const char* cookies, char* post, ...);

	// Curl received
	static size_t PageGet(void *buffer, size_t size, size_t nmemb, void *stream);

	// header received
	static size_t HeaderGet(void *buffer, size_t size, size_t nmemb, void *stream);

	// Urlencode String
	std::string urlencode(std::string code);
};


#endif
