/**
* -----------------------------------------------------
* File			webapi.h
* Authors		David <popoklopsi> Ordnung, Impact
* Idea			Zephyrus
* License		GPLv3
* Web			http://popoklopsi.de, http://gugyclan.eu
* -----------------------------------------------------
*
* Originally provided for CallAdmin by Popoklopsi and Impact
*
* Copyright (C) 2014 David <popoklopsi> Ordnung, Impact
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


#define CLIENT_ID "DE45CD61"
#define CLIENT_SCOPE "read_profile write_profile read_client write_client"
#define USER_AGENT "Steam App / Android / 1.0.6 / 1328177"
#define APP_ID "1328177"


#include <string.h>
#include "rsa.h"
#include "extension.h"
#include "json/json.h"
#include "curl/curl.h"




// Struct for result
typedef struct
{
public:
	// Chars
	std::string pResultString;
	std::string pResultHeader;
	char curlError[CURL_ERROR_SIZE + 1];

} CurlReturn;




// Working with webpai
class WebAPIClass
{
private:
	bool loggedin;
	int lastmessage;

	CURL *curl;

	std::string username;
	std::string password;
	std::string oauth;
	std::string steamid;
	std::string umqid;

	Json::Value friendlist;
	Json::Value onlinestates;

public:
	WebAPIClass()
	{
		username = "";
		password = "";
		oauth = "";
		steamid = "";
		umqid = "";

		lastmessage = 0;
		loggedin = false;
		curl = NULL;

		curl_global_init(CURL_GLOBAL_ALL);
	}

	~WebAPIClass()
	{
		// Clean
		curl_global_cleanup();
	}


	// Send process for OSW
	CallBackResult SendMessageWebAPI(char *user, char *pass, char *msg, bool sOnline, int &error);

private:
	// Setup web api stuff
	bool LoginWebAPI();

	// Get UmqID
	bool LoginUMQID();


	// Get FriendList
	void getFriendList();

	// Get Userstats
	void getUserStats();

	// Show online
	void showOnline();

	// Show online
	void acceptFriend(std::string friendSteam, std::string &sessionID);


	// Logout 
	void LogoutWebAPI();


	// Get a Page
	CurlReturn getPage(const char* url, const char* cookies, char* post, ...);

	// Curl received
	static size_t page_get(void *buffer, size_t size, size_t nmemb, void *stream);

	// header received
	static size_t header_get(void *buffer, size_t size, size_t nmemb, void *stream);


	// Urlencode String
	std::string urlencode(std::string code);
};



extern WebAPIClass* webClass;


#endif