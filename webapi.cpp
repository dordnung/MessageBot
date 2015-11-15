/**
* -----------------------------------------------------
* File			webapi.cpp
* Authors		David <popoklopsi> Ordnung, Impact
* License		GPLv3
* Web			http://popoklopsi.de, http://gugyclan.eu
* -----------------------------------------------------
*
* Originally provided for CallAdmin by Popoklopsi and Impact
*
* Copyright (C) 2014-2015 David <popoklopsi> Ordnung, Impact
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


#include "webapi.h"

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


bool WebAPIClass::loginWebAPI() {
	// Username and password must have at least a length of 3
	if (username.length() < 3 || password.length() < 3) {
		return false;
	}

	// Notify steam that we need oauth
	std::string sessionPage = std::string("https://steamcommunity.com/mobilelogin?oauth_client_id=") + urlencode(CLIENT_ID) + std::string("&oauth_scope=") + urlencode(CLIENT_SCOPE);
	Debug(sessionPage.c_str());
	getPage(sessionPage.c_str(), USER_AGENT_ANDROID, NULL, NULL);

	Json::Value root;
	Json::Reader reader;

	CurlReturn curlReturn = getPage("https://steamcommunity.com/mobilelogin/getrsakey", USER_AGENT_ANDROID, NULL, "username=%s", urlencode(username).c_str());
	Debug(curlReturn.resultString.c_str());

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


	std::string mod = root.get("publickey_mod", "").asString();
	std::string exp = root.get("publickey_exp", "").asString();
	std::string timestamp = root.get("timestamp", "").asString();

	if (mod.empty() || exp.empty() || timestamp.empty()) {
		LogError("Failed to get WebAPI RSAKey Information");

		return false;
	}


	std::string encrypted = encrypt(mod.c_str(), exp.c_str(), password.c_str());


	curlReturn = getPage("https://steamcommunity.com/mobilelogin/dologin/", USER_AGENT_ANDROID, NULL, "password=%s&username=%s&twofactorcode=&emailauth=&loginfriendlyname=CallAdmin&captchagid=-1&captcha_text=&emailsteamid=&rsatimestamp=%s&remember_login=true&oauth_client_id=%s&oauth_scope=%s", urlencode(encrypted).c_str(), urlencode(username).c_str(), urlencode(timestamp).c_str(), urlencode(CLIENT_ID).c_str(), urlencode(CLIENT_SCOPE).c_str());
	Debug(curlReturn.resultString.c_str());


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

	std::string auth = root.get("oauth", "").asString();

	if (auth.empty()) {
		LogError("Failed to get Login Auth!");

		return false;
	}


	if (!reader.parse(auth, root)) {
		LogError("Failed to parse Auth token. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	steamid = root["steamid"].asString();
	oauth = root["oauth_token"].asString();

	if (steamid.empty() || oauth.empty()) {
		LogError("Failed to get Login steamid and auth. Error: %s!", root.toStyledString().c_str());

		return false;
	}


	return true;
}


bool WebAPIClass::loginUMQID() {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	CurlReturn curlReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logon/v0001", USER_AGENT_APP, NULL, "access_token=%s", urlencode(oauth).c_str());
	Debug(curlReturn.resultString.c_str());

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


	umqid = root.get("umqid", "").asString();

	if (umqid.empty()) {
		LogError("Failed to get UMQID.");

		return false;
	}

	lastMessage = root.get("message", "").asInt();

	return true;
}


void WebAPIClass::logoutWebAPI() {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	CurlReturn curlReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logoff/v0001", USER_AGENT_APP, NULL, "access_token=%s&umqid=%s", urlencode(oauth).c_str(), urlencode(umqid).c_str());
	Debug(curlReturn.resultString.c_str());

	umqid = "";


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


void WebAPIClass::getFriendList() {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";


	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetFriendList/v0001";
	url = url + "?access_token=" + urlencode(oauth) + "&relationship=friend,requestrecipient";


	CurlReturn curlReturn = getPage(url.c_str(), USER_AGENT_APP, NULL, "");
	Debug(curlReturn.resultString.c_str());

	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive FriendList. Error: %s", curlReturn.curlError);

		return;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse FriendList. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	Debug(root.toStyledString().c_str());
	friendList = root;
}


void WebAPIClass::getUserStats(std::vector<uint64_t> &recipients) {
	Json::Value root;
	Json::Reader reader;
	std::string error = "";
	bool isFirst = true;

	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetUserSummaries/v0001";
	url = url + "?access_token=" + urlencode(oauth) + "&steamids=";


	for (std::vector<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		if (isFirst) {
			isFirst = false;

			url = url + std::to_string(*recipient);
		} else {
			url = url + "," + std::to_string(*recipient);
		}
	}


	CurlReturn curlReturn = getPage(url.c_str(), USER_AGENT_APP, NULL, "");
	Debug(curlReturn.resultString.c_str());

	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to receive UserStats. Error: %s", curlReturn.curlError);

		return;
	}

	if (!reader.parse(curlReturn.resultString, root)) {
		LogError("Failed to parse UserStats. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	Debug(root.toStyledString().c_str());
	onlineStates = root;
}


void WebAPIClass::showOnline() {
	CurlReturn curlReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Poll/v0001", USER_AGENT_APP, NULL, "access_token=%s&umqid=%s&message=%i", urlencode(oauth).c_str(), urlencode(umqid).c_str(), lastMessage);


	if (strcmp(curlReturn.curlError, "") != 0) {
		LogError("Failed to show as Online. Error: %s", curlReturn.curlError);

		return;
	}
}


void WebAPIClass::acceptFriend(std::string friendSteam, std::string &sessionID) {
	std::string cookie = std::string(WEB_COOKIE) + std::string("steamLogin=");
	cookie = cookie + steamid + std::string("||oauth:") + oauth;

	if (sessionID.empty()) {
		CurlReturn curlReturn = getPage("https://steamcommunity.com/mobilesettings/GetManifest/v0001", USER_AGENT_APP, cookie.c_str(), "");

		std::size_t found = curlReturn.resultHeader.find("sessionid=");

		if (found != std::string::npos) {
			std::size_t found2 = curlReturn.resultHeader.find(";", found);

			if (found2 != std::string::npos) {
				sessionID = curlReturn.resultHeader.substr(found + 10, found2 - found - 10);
			}
		}
	}


	if (!sessionID.empty()) {
		cookie = cookie + std::string(";sessionid=") + sessionID;

		getPage((std::string("https://steamcommunity.com/profiles/") + std::string(steamid) + std::string("/home_process")).c_str(), USER_AGENT_ANDROID, cookie.c_str(), "json=1&xml=1&action=approvePending&itype=friend&perform=accept&sessionID=%s&id=%s", sessionID.c_str(), urlencode(friendSteam).c_str());
	}
}


CallBackResult WebAPIClass::sendMessageWebAPI(std::string user, std::string pass, std::string msg, bool sOnline, std::vector<uint64_t> &recipients) {
	if (username != user || !loggedIn || oauth.empty()) {
		username = user;
		password = pass;

		if (!(loggedIn = this->loginWebAPI())) {
			return LOGIN_ERROR;
		}
	}

	if (umqid.empty()) {
		if (!loginUMQID()) {
			loggedIn = false;

			return LOGIN_ERROR;
		}
	}

	getUserStats(recipients);
	getFriendList();

	if (sOnline && lastMessage != 0) {
		showOnline();
	}


	Json::Value root;
	Json::Reader reader;
	std::string errors = "";
	std::string sessionID = "";

	bool foundUser = false;

	for (std::vector<uint64_t>::iterator recipient = recipients.begin(); recipient != recipients.end(); recipient++) {
		bool isUserValid = false;

		foundUser = true;


		Json::Value friendValue = friendList.get("friends", "");
		Json::Value userValue = onlineStates.get("players", "");


		for (int j = 0; friendValue.isValidIndex(j); j++) {
			std::string steam = friendValue[j].get("steamid", "").asString();
			std::string relation = friendValue[j].get("relationship", "").asString();

			if (steam == std::to_string(*recipient)) {
				if (relation == "requestrecipient") {
					acceptFriend(steam, sessionID);

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


		CurlReturn curlReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Message/v0001", USER_AGENT_APP, NULL, "access_token=%s&text=%s&steamid_dst=%lld&umqid=%s&type=saytext", urlencode(oauth).c_str(), urlencode(msg).c_str(), *recipient, urlencode(umqid).c_str());
		Debug(curlReturn.resultString.c_str());

		if (strcmp(curlReturn.curlError, "") != 0) {
			LogError("Failed to receive Message Site. Error: %s", curlReturn.curlError);

			logoutWebAPI();

			return TIMEOUT_ERROR;
		}

		if (!reader.parse(curlReturn.resultString, root)) {
			LogError("Failed to parse Message. Error: %s", reader.getFormattedErrorMessages().c_str());

			logoutWebAPI();

			return LOGIN_ERROR;
		}

		Debug(root.toStyledString().c_str());
		errors = root.get("error", "").asString();

		if (errors != "OK") {
			LogError("Failed to send Message. Error: %s", errors.c_str());

			logoutWebAPI();

			return NO_RECEIVER;
		}
	}


	logoutWebAPI();

	if (!foundUser) {
		return ARRAY_EMPTY;
	}


	return SUCCESS;
}


CurlReturn WebAPIClass::getPage(const char* url, const char* useragent, const char* cookies, char* post, ...) {
	char fmtString[1024];

	CurlReturn *curlReturn = new CurlReturn;

	strcpy(curlReturn->curlError, "");
	curlReturn->resultString = "";
	curlReturn->resultHeader = "";


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
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WebAPIClass::headerGet);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebAPIClass::pageGet);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, curlReturn);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, curlReturn);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

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


	CurlReturn curlReturnTemp;

	curlReturnTemp.resultHeader = curlReturn->resultHeader;
	curlReturnTemp.resultString = curlReturn->resultString;
	strcpy(curlReturnTemp.curlError, curlReturn->curlError);

	delete curlReturn;


	return curlReturnTemp;
}


// Get something of the page
size_t WebAPIClass::pageGet(void *buffer, size_t size, size_t nmemb, void *stream) {
	// Buffer
	CurlReturn *curlReturn = (CurlReturn *)stream;

	// Add buffer
	curlReturn->resultString.append((char *)buffer);

	return size * nmemb;
}


// Get something of the page
size_t WebAPIClass::headerGet(void *buffer, size_t size, size_t nmemb, void *stream) {
	// Buffer
	CurlReturn *curlReturn = (CurlReturn *)stream;

	// Add buffer
	curlReturn->resultHeader.append((char *)buffer);

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