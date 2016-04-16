/**
* -----------------------------------------------------
* File			webapi.cpp
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

#include <stdarg.h>
#include "webapi.h"


// Use server console if sourcemod build, otherwise use window console
#if defined SOURCEMOD_BUILD
#include "extension.h"

#define LogError(fmt, ...) smutils->LogError(myself, fmt, ##__VA_ARGS__)
#if defined DEBUG_WEBAPI && DEBUG_WEBAPI == 1
#define Debug(message) smutils->LogMessage(myself, message)
#else
#define Debug(message)
#endif
#else
#define LogError(fmt, ...) printf(fmt, ##__VA_ARGS__)
#if defined DEBUG_WEBAPI && DEBUG_WEBAPI == 1
#define Debug(message) printf("%s\n", message)
#else
#define Debug(message)
#endif
#endif


bool WebAPIClass::LoginWebAPI() {
	// Username and password must have at least a length of 3
	if (this->username.length() < 3 || this->password.length() < 3) {
		return false;
	}

	// Notify steam that we need oauth
	std::string sessionPage = std::string("https://steamcommunity.com/mobilelogin?oauth_client_id=") + urlencode(CLIENT_ID) + std::string("&oauth_scope=") + urlencode(CLIENT_SCOPE);
	Debug(sessionPage.c_str());
	GetPage(sessionPage.c_str(), USER_AGENT_ANDROID, NULL, NULL);

	Json::Value root;
	Json::Reader reader;

	// Get the RSA key to login
	CurlReturn curlReturn = GetPage("https://steamcommunity.com/mobilelogin/getrsakey", USER_AGENT_ANDROID, NULL, "username=%s", urlencode(this->username).c_str());
	Debug(curlReturn.resultString.c_str());

	// Check for errors
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive WebAPI RSAKey. Error: %s", curlReturn.curlError);

		return false;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse WebAPI RSAKey. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	if (!root.get("success", false).asBool()) {
		LogError("Failed to get WebAPI RSAKey");

		return false;
	}

	// Read values
	std::string mod = root.get("publickey_mod", "").asString();
	std::string exp = root.get("publickey_exp", "").asString();
	std::string timestamp = root.get("timestamp", "").asString();

	if (mod.empty() || exp.empty() || timestamp.empty()) {
		LogError("Failed to get WebAPI RSAKey Information");

		return false;
	}

	// Now encrypt it with RSA
	std::string encrypted = encrypt(mod.c_str(), exp.c_str(), this->password.c_str());

	// And login with it
	curlReturn = GetPage("https://steamcommunity.com/mobilelogin/dologin/", USER_AGENT_ANDROID, NULL, "password=%s&username=%s&twofactorcode=&emailauth=&loginfriendlyname=CallAdmin&captchagid=-1&captcha_text=&emailsteamid=&rsatimestamp=%s&remember_login=true&oauth_client_id=%s&oauth_scope=%s", urlencode(encrypted).c_str(), urlencode(this->username).c_str(), urlencode(timestamp).c_str(), urlencode(CLIENT_ID).c_str(), urlencode(CLIENT_SCOPE).c_str());
	Debug(curlReturn.resultString.c_str());

	// Check for errors
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to Login. Error: %s", curlReturn.curlError);

		return false;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse Login Result. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	if (!root.get("success", false).asBool() || !root.get("login_complete", false).asBool()) {
		std::string error = root.get("message", "").asString();

		if (error.empty()) {
			error = root.toStyledString();
		}

		LogError("Failed to succesfully Login. Error: %s", error.c_str());

		return false;
	}

	// Read oauth from result
	std::string auth = root.get("oauth", "").asString();

	if (auth.empty()) {
		LogError("Failed to get Login Auth!");

		return false;
	}

	if (!reader.parse(auth, root)) {
		LogError("Failed to parse Auth token. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	// And from there read steamid and oauth token
	this->steamid = root["steamid"].asString();
	this->oauth = root["oauth_token"].asString();

	if (this->steamid.empty() || this->oauth.empty()) {
		LogError("Failed to get Login steamid and auth. Error: %s!", root.toStyledString().c_str());

		return false;
	}


	return true;
}


bool WebAPIClass::LoginUMQID() {
	Json::Value root;
	Json::Reader reader;
	std::string error;

	// Login to get UMQID
	CurlReturn curlReturn = GetPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logon/v0001", USER_AGENT_APP, NULL, "access_token=%s", urlencode(this->oauth).c_str());
	Debug(curlReturn.resultString.c_str());

	// Valid result?
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive UMQID. Error: %s", curlReturn.curlError);

		return false;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse UMQID. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	Debug(root.toStyledString().c_str());
	error = root.get("error", "").asString();

	if (error != "OK") {
		LogError("Failed to get UMQID. Error: %s", error.c_str());

		return false;
	}

	// Parse UMQID
	this->umqid = root.get("umqid", "").asString();

	if (this->umqid.empty()) {
		LogError("Failed to get UMQID.");

		return false;
	}

	this->lastMessage = root.get("message", "").asInt();

	return true;
}


void WebAPIClass::LogoutWebAPI() {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	// Logout
	CurlReturn curlReturn = GetPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logoff/v0001", USER_AGENT_APP, NULL, "access_token=%s&umqid=%s", urlencode(this->oauth).c_str(), urlencode(this->umqid).c_str());
	Debug(curlReturn.resultString.c_str());

	this->umqid = "";

	// Successfull?
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to Logout. Error: %s", curlReturn.curlError);

		return;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse Logout. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	Debug(root.toStyledString().c_str());
	error = root.get("error", "").asString();

	if (error != "OK") {
		LogError("Failed to Logout. Error: %s", error.c_str());
	}
}


void WebAPIClass::GetFriendList() {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetFriendList/v0001";
	url = url + "?access_token=" + urlencode(this->oauth) + "&relationship=friend,requestrecipient";

	// Read the friend list of the bot
	CurlReturn curlReturn = GetPage(url.c_str(), USER_AGENT_APP, NULL, "");
	Debug(curlReturn.resultString.c_str());

	// Valid result?
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive FriendList. Error: %s", curlReturn.curlError);

		return;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse FriendList. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	// Save friend list
	Debug(root.toStyledString().c_str());
	this->friendList = root;
}


void WebAPIClass::GetUserStats(std::list<uint64_t> &recipients) {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";
	bool isFirst = true;

	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetUserSummaries/v0001";
	url = url + "?access_token=" + urlencode(this->oauth) + "&steamids=";

	// Append all recipients to the request
	for (std::list<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		if (isFirst) {
			isFirst = false;

			url = url + std::to_string(*recipient);
		} else {
			url = url + "," + std::to_string(*recipient);
		}
	}

	// Get user stats of all recipients
	CurlReturn curlReturn = GetPage(url.c_str(), USER_AGENT_APP, NULL, "");
	Debug(curlReturn.resultString.c_str());

	// Valid result?
	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive UserStats. Error: %s", curlReturn.curlError);

		return;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse UserStats. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	// Save online states
	Debug(root.toStyledString().c_str());
	this->onlineStates = root;
}


void WebAPIClass::ShowOnline() {
	// Show the bot as online
	CurlReturn curlReturn = GetPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Poll/v0001", USER_AGENT_APP, NULL, "access_token=%s&umqid=%s&message=%i", urlencode(this->oauth).c_str(), urlencode(this->umqid).c_str(), this->lastMessage);

	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to show as Online. Error: %s", curlReturn.curlError);

		return;
	}
}


void WebAPIClass::AcceptFriend(std::string friendSteam, std::string &sessionID) {
	// First of all get a session key
	std::string cookie = std::string(WEB_COOKIE) + std::string("steamLogin=");
	cookie = cookie + this->steamid + std::string("||oauth:") + this->oauth;

	if (sessionID.empty()) {
		CurlReturn curlReturn = GetPage("https://steamcommunity.com/mobilesettings/GetManifest/v0001", USER_AGENT_APP, cookie.c_str(), "");

		// Find session cookie
		std::size_t found = curlReturn.resultHeader.find("sessionid=");

		if (found != std::string::npos) {
			std::size_t found2 = curlReturn.resultHeader.find(";", found);

			if (found2 != std::string::npos) {
				// Found it
				sessionID = curlReturn.resultHeader.substr(found + 10, found2 - found - 10);
			}
		}
	}

	// Found a valid session Id?
	if (!sessionID.empty()) {
		cookie = cookie + std::string(";sessionid=") + sessionID;

		// Now add the friend
		GetPage((std::string("https://steamcommunity.com/profiles/") + this->steamid + std::string("/home_process")).c_str(), USER_AGENT_ANDROID, cookie.c_str(), "json=1&xml=1&action=approvePending&itype=friend&perform=accept&sessionID=%s&id=%s", sessionID.c_str(), urlencode(friendSteam).c_str());
	}
}


CallBackResult WebAPIClass::SendMessageWebAPI(std::string username, std::string password, std::string message, bool showOnline, std::list<uint64_t> &recipients) {
	// No recipient?
	if (recipients.size() == 0) {
		return CallBackResult_ARRAY_EMPTY;
	}

	// Need complete new login?
	if (this->username != username || !this->loggedIn || this->oauth.empty()) {
		this->username = username;
		this->password = password;

		if (!(this->loggedIn = this->LoginWebAPI())) {
			return CallBackResult_LOGIN_ERROR;
		}
	}

	// Need a UMQID?
	if (this->umqid.empty()) {
		if (!LoginUMQID()) {
			this->loggedIn = false;

			return CallBackResult_LOGIN_ERROR;
		}
	}

	// Get user stats and the friend list
	GetUserStats(recipients);
	GetFriendList();

	if (showOnline && this->lastMessage != 0) {
		// Show the bot as online
		ShowOnline();
	}

	Json::Value root;
	Json::Reader reader;
	std::string errors = "";
	std::string sessionID = "";

	// Check if there is valid recipient which is online
	for (std::list<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		bool isUserValid = false;

		Json::Value friendValue = this->friendList.get("friends", "");
		Json::Value userValue = this->onlineStates.get("players", "");

		for (int j = 0; friendValue.isValidIndex(j); j++) {
			std::string steam = friendValue[j].get("steamid", "").asString();
			std::string relation = friendValue[j].get("relationship", "").asString();

			if (steam == std::to_string(*recipient)) {
				if (relation == "requestrecipient") {
					// Accept the friend if there is a request
					AcceptFriend(steam, sessionID);

					break;
				}

				for (int k = 0; userValue.isValidIndex(k); k++) {
					steam = userValue[k].get("steamid", "").asString();
					int online = userValue[k].get("personastate", "").asInt();

					if (steam == std::to_string(*recipient)) {
						if (online > 0) {
							isUserValid = true;

							break;
						}
					}
				}
			}

			if (isUserValid) {
				break;
			}
		}

		if (!isUserValid) {
			continue;
		}

		// Send the message to the recipient
		CurlReturn curlReturn = GetPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Message/v0001", USER_AGENT_APP, NULL, "access_token=%s&text=%s&steamid_dst=%lld&umqid=%s&type=saytext", urlencode(this->oauth).c_str(), urlencode(message).c_str(), *recipient, urlencode(this->umqid).c_str());
		Debug(curlReturn.resultString.c_str());

		// Valid?
		if (strcmp(curlReturn.curlError, "") != 0) {
			LogError("Failed to receive message site. Error: %s", curlReturn.curlError);

			LogoutWebAPI();

			return CallBackResult_TIMEOUT_ERROR;
		}

		if (!reader.parse(curlReturn.resultString, root)) {
			LogError("Failed to parse message. Error: %s", reader.getFormattedErrorMessages().c_str());

			LogoutWebAPI();

			return CallBackResult_LOGIN_ERROR;
		}

		Debug(root.toStyledString().c_str());
		errors = root.get("error", "").asString();

		if (errors != "OK") {
			LogError("Failed to send Message. Error: %s", errors.c_str());

			LogoutWebAPI();

			return CallBackResult_NO_RECEIVER;
		}
	}

	// Logout on finish
	LogoutWebAPI();

	return CallBackResult_SUCCESS;
}


CurlReturn WebAPIClass::GetPage(const char* url, const char* useragent, const char* cookies, char* post, ...) {
	char fmtString[1024];

	CurlReturn *curlReturn = new CurlReturn;

	strcpy(curlReturn->curlError, "");
	curlReturn->resultString = "";
	curlReturn->resultHeader = "";

	// Process post list
	if (post != NULL) {
		va_list argptr;
		va_start(argptr, post);

		vsprintf(fmtString, post, argptr);

		va_end(argptr);

	} else {
		fmtString[0] = '\0';
	}

	curl = curl_easy_init();

	if (curl != NULL) {
		struct curl_slist *chunk = NULL;

		// Set up Curl
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, curlReturn->curlError);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WebAPIClass::HeaderGet);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebAPIClass::PageGet);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, curlReturn);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, curlReturn);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

		// Append post data
		if (strlen(fmtString) > 0) {
			chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
			chunk = curl_slist_append(chunk, (std::string("Content-length: ") + std::to_string(strlen(fmtString))).c_str());

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fmtString);
		}

		// Cookies need to be set
		if (cookies == NULL) {
			cookies = WEB_COOKIE;
		}

		chunk = curl_slist_append(chunk, (std::string("Cookie: ") + std::string(cookies)).c_str());

		if (chunk != NULL) {
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}

		// Perform
		if (curl_easy_perform(curl) != CURLE_OK) {
			curlReturn->resultString = curlReturn->curlError;
		}

		// Clean
		curl_easy_cleanup(curl);

		if (chunk != NULL) {
			curl_slist_free_all(chunk);
		}
	}

	// Save result
	CurlReturn curlReturnTemp;

	curlReturnTemp.resultHeader = curlReturn->resultHeader;
	curlReturnTemp.resultString = curlReturn->resultString;
	strcpy(curlReturnTemp.curlError, curlReturn->curlError);

	delete curlReturn;

	// Return result
	return curlReturnTemp;
}


// Got something of the page
size_t WebAPIClass::PageGet(void *buffer, size_t size, size_t nmemb, void *stream) {
	// Buffer
	CurlReturn *curlReturn = static_cast<CurlReturn *>(stream);

	// Add buffer
	curlReturn->resultString.append(static_cast<char *>(buffer));

	return size * nmemb;
}


// Got something of the header
size_t WebAPIClass::HeaderGet(void *buffer, size_t size, size_t nmemb, void *stream) {
	// Buffer
	CurlReturn *curlReturn = static_cast<CurlReturn *>(stream);

	// Add buffer
	curlReturn->resultHeader.append(static_cast<char *>(buffer));

	return size * nmemb;
}


std::string WebAPIClass::urlencode(std::string code) {
	curl = curl_easy_init();
	std::string ret = "";

	if (curl != NULL) {
		char *escaped = curl_easy_escape(curl, code.c_str(), code.length());

		if (escaped != NULL) {
			ret = escaped;
			curl_free(escaped);
		}

		curl_easy_cleanup(curl);
	}

	return ret;
}
