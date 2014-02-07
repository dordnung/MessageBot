/**
* -----------------------------------------------------
* File			webapi.cpp
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


#include "webapi.h"



WebAPIClass* webClass = NULL;



bool WebAPIClass::LoginWebAPI()
{
	// Username and password must have at least a length of 3
	if (username.length() < 3 || password.length() < 3)
	{
		return false;
	}



	Json::Value root;
	Json::Reader reader;

	CurlReturn pReturn = getPage("https://steamcommunity.com/mobilelogin/getrsakey/", NULL, "username=%s", urlencode(username).c_str());


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to receive WebAPI RSAKey. Error: %s", pReturn.curlError);

		return false;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse WebAPI RSAKey. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	if (!root.get("success", false).asBool())
	{
		smutils->LogError(myself, "Failed to get WebAPI RSAKey");

		return false;
	}


	std::string mod = root.get("publickey_mod", "").asString();
	std::string exp = root.get("publickey_exp", "").asString();
	std::string timestamp = root.get("timestamp", "").asString();


	if (mod.empty() || exp.empty() || timestamp.empty())
	{
		smutils->LogError(myself, "Failed to get WebAPI RSAKey Information");

		return false;
	}




	std::string encrypted = encrypt(mod.c_str(), exp.c_str(), password.c_str());


	pReturn = getPage("https://steamcommunity.com/mobilelogin/dologin/", NULL, "username=%s&password=%s&emailauth=&captchagid=&captcha_text=&oauth_client_id=%s&oauth_scope=%s&emailsteamid=&remember_login=true&rsatimestamp=%s", urlencode(username).c_str(), urlencode(encrypted).c_str(), urlencode(CLIENT_ID).c_str(), urlencode(CLIENT_SCOPE).c_str(), urlencode(timestamp).c_str());



	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to Login. Error: %s", pReturn.curlError);

		return false;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse Login Result. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}

	if (!root.get("success", false).asBool() || !root.get("login_complete", false).asBool())
	{
		std::string error = root.get("message", "").asString();

		if (error.empty())
		{
			error = root.toStyledString();
		}

		smutils->LogError(myself, "Failed to succesfully Login. Error: %s", error.c_str());

		return false;
	}



	std::string auth = root.get("oauth", "").asString();

	if (auth.empty())
	{
		smutils->LogError(myself, "Failed to get Login Auth!");

		return false;
	}

	if (!reader.parse(auth, root))
	{
		smutils->LogError(myself, "Failed to parse Auth token. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}



	steamid = root["steamid"].asString();
	oauth = root["oauth_token"].asString();

	if (steamid.empty() || oauth.empty())
	{
		smutils->LogError(myself, "Failed to get Login steamid and auth. Error: %s!", root.toStyledString().c_str());

		return false;
	}


	return true;
}





bool WebAPIClass::LoginUMQID()
{
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	CurlReturn pReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logon/v0001", NULL, "access_token=%s", urlencode(oauth).c_str());


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to receive UMQID. Error: %s", pReturn.curlError);

		return false;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse UMQID. Error: %s", reader.getFormattedErrorMessages().c_str());

		return false;
	}


	error = root.get("error", "").asString();

	if (error != "OK")
	{
		smutils->LogError(myself, "Failed to get UMQID. Error: %s", error.c_str());

		return false;
	}


	umqid = root.get("umqid", "").asString();

	if (umqid.empty())
	{
		smutils->LogError(myself, "Failed to get UMQID.");

		return false;
	}

	lastmessage = root.get("message", "").asInt();

	return true;
}





void WebAPIClass::LogoutWebAPI()
{
	Json::Value root;
	Json::Reader reader;
	std::string error = "";

	CurlReturn pReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Logoff/v0001", NULL, "access_token=%s&umqid=%s", urlencode(oauth).c_str(), urlencode(umqid).c_str());

	umqid = "";


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to Logout. Error: %s", pReturn.curlError);

		return;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse Logout. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}


	error = root.get("error", "").asString();

	if (error != "OK")
	{
		smutils->LogError(myself, "Failed to Logout. Error: %s", error.c_str());
	}
}





void WebAPIClass::getFriendList()
{
	Json::Value root;
	Json::Reader reader;
	std::string error = "";


	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetFriendList/v0001";
	url = url + "?access_token=" + urlencode(oauth) + "&relationship=friend,requestrecipient";


	CurlReturn pReturn = getPage(url.c_str(), NULL, "");


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to receive FriendList. Error: %s", pReturn.curlError);

		return;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse FriendList. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	friendlist = root;
}





void WebAPIClass::getUserStats()
{
	Json::Value root;
	Json::Reader reader;
	std::string error = "";
	bool isFirst = true;

	std::string url = "https://api.steampowered.com/ISteamUserOAuth/GetUserSummaries/v0001";
	url = url + "?access_token=" + urlencode(oauth) + "&steamids=";


	for (int i = 0; i < MAX_RECIPIENTS; i++)
	{
		if (recipients[i] != NULL)
		{
			if (isFirst)
			{
				isFirst = false;

				url = url + std::to_string(recipients[i]->ConvertToUint64());
			}
			else
			{
				url = url + "," + std::to_string(recipients[i]->ConvertToUint64());
			}
		}
	}


	CurlReturn pReturn = getPage(url.c_str(), NULL, "");


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to receive UserStats. Error: %s", pReturn.curlError);

		return;
	}

	if (!reader.parse(pReturn.pResultString, root))
	{
		smutils->LogError(myself, "Failed to parse UserStats. Error: %s", reader.getFormattedErrorMessages().c_str());

		return;
	}

	onlinestates = root;
}




void WebAPIClass::showOnline()
{
	CurlReturn pReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Poll/v0001", NULL, "access_token=%s&umqid=%s&message=%i", urlencode(oauth).c_str(), urlencode(umqid).c_str(), lastmessage);


	if (strcmp(pReturn.curlError, "") != 0)
	{
		smutils->LogError(myself, "Failed to show as Online. Error: %s", pReturn.curlError);

		return;
	}
}




void WebAPIClass::acceptFriend(std::string friendSteam, std::string &sessionID)
{
	std::string cookie("forceMobile=1;mobileClient=android;mobileClientVersion=");

	cookie = cookie + std::string(APP_ID) + std::string(";Steam_Language=english;steamLogin=");
	cookie = cookie + steamid + std::string("||oauth:") + oauth;

	if (sessionID.empty())
	{
		CurlReturn pReturn = getPage("https://steamcommunity.com/mobilesettings/GetManifest/v0001", cookie.c_str(), "");

		std::size_t found = pReturn.pResultHeader.find("sessionid=");

		if (found != std::string::npos)
		{
			std::size_t found2 = pReturn.pResultHeader.find(";", found);

			if (found2 != std::string::npos)
			{
				sessionID = pReturn.pResultHeader.substr(found + 10, found2 - found - 10);
			}
		}
	}


	if (!sessionID.empty())
	{
		cookie = cookie + std::string(";sessionid=") + sessionID;

		getPage((std::string("https://steamcommunity.com/profiles/") + std::string(steamid) + std::string("/home_process")).c_str(), cookie.c_str(), "json=1&xml=1&action=approvePending&itype=friend&perform=accept&sessionID=%s&id=%s", sessionID.c_str(), urlencode(friendSteam).c_str());
	}
}




CallBackResult WebAPIClass::SendMessageWebAPI(char *user, char *pass, char *msg, bool sOnline, int &error)
{
	if (username != user || !loggedin || oauth.empty())
	{
		username = user;
		password = pass;

		if (!(loggedin = webClass->LoginWebAPI()))
		{
			return LOGIN_ERROR;
		}
	}

	if (umqid.empty())
	{
		if (!LoginUMQID())
		{
			loggedin = false;

			return LOGIN_ERROR;
		}
	}


	getUserStats();
	getFriendList();

	if (sOnline && lastmessage != 0)
	{
		showOnline();
	}


	Json::Value root;
	Json::Reader reader;
	std::string errors = "";
	std::string sessionID = "";

	bool foundUser = false;


	for (int i = 0; i < MAX_RECIPIENTS && extensionLoaded; i++)
	{
		// Valid Steamid?
		if (recipients[i] != NULL)
		{
			bool isUserValid = false;

			foundUser = true;


			Json::Value friendValue = friendlist.get("friends", "");
			Json::Value userValue = onlinestates.get("players", "");


			for (int j=0; friendValue.isValidIndex(j); j++)
			{
				std::string steam = friendValue[j].get("steamid", "").asString();
				std::string relation = friendValue[j].get("relationship", "").asString();

				if (steam == std::to_string(recipients[i]->ConvertToUint64()))
				{
					if (relation == "requestrecipient")
					{
						acceptFriend(steam, sessionID);

						break;
					}
					
					for (int k = 0; userValue.isValidIndex(k); k++)
					{
						steam = userValue[k].get("steamid", "").asString();
						int online = userValue[k].get("personastate", "").asInt();

						if (steam == std::to_string(recipients[i]->ConvertToUint64()))
						{
							if (online > 0)
							{
								isUserValid = true;

								break;
							}
						}
					}
				}
			
				if (isUserValid)
				{
					break;
				}
			}



			if (!isUserValid)
			{
				continue;
			}


			CurlReturn pReturn = getPage("https://api.steampowered.com/ISteamWebUserPresenceOAuth/Message/v0001", NULL, "access_token=%s&text=%s&steamid_dst=%lld&umqid=%s&type=saytext", urlencode(oauth).c_str(), urlencode(msg).c_str(), recipients[i]->ConvertToUint64(), urlencode(umqid).c_str());


			if (strcmp(pReturn.curlError, "") != 0)
			{
				smutils->LogError(myself, "Failed to receive Message Site. Error: %s", pReturn.curlError);

				LogoutWebAPI();

				return TIMEOUT_ERROR;
			}

			if (!reader.parse(pReturn.pResultString, root))
			{
				smutils->LogError(myself, "Failed to parse Message. Error: %s", reader.getFormattedErrorMessages().c_str());

				LogoutWebAPI();

				return LOGIN_ERROR;
			}


			errors = root.get("error", "").asString();

			if (errors != "OK")
			{
				smutils->LogError(myself, "Failed to send Message. Error: %s", errors.c_str());

				LogoutWebAPI();

				return NO_RECEIVER;
			}
		}
	}


	LogoutWebAPI();

	if (!foundUser)
	{
		return ARRAY_EMPTY;
	}


	return SUCCESS;
}





CurlReturn WebAPIClass::getPage(const char* url, const char* cookies, char* post, ...)
{
	char fmtString[1024];

	CurlReturn *pReturn = new CurlReturn;

	strcpy(pReturn->curlError, "");
	pReturn->pResultString = "";
	pReturn->pResultHeader = "";


	if (post != NULL)
	{
		va_list argptr;
		va_start(argptr, post);

		vsprintf(fmtString, post, argptr);

		va_end(argptr);

	}
	else
	{
		fmtString[0] = '\0';
	}

	curl = curl_easy_init();

	if (curl != NULL)
	{
		struct curl_slist *chunk = NULL;

		// Set up Curl
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, pReturn->curlError);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WebAPIClass::header_get);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WebAPIClass::page_get);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, pReturn);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, pReturn);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);

		if (strlen(fmtString) > 0)
		{
			chunk = curl_slist_append(chunk, "Content-Type: application/x-www-form-urlencoded");
			chunk = curl_slist_append(chunk, (std::string("Content-length: ") + std::to_string(strlen(fmtString))).c_str());

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fmtString);
		}


		if (cookies != NULL)
		{
			chunk = curl_slist_append(chunk, (std::string("Cookie: ") + std::string(cookies)).c_str());
		}


		if (chunk != NULL)
		{
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		}


		// Perform
		if (curl_easy_perform(curl) != CURLE_OK)
		{
			pReturn->pResultString = pReturn->curlError;
		}


		// Clean
		curl_easy_cleanup(curl);


		if (chunk != NULL)
		{
			curl_slist_free_all(chunk);
		}
	}


	CurlReturn pReturnTemp;

	pReturnTemp.pResultHeader = pReturn->pResultHeader;
	pReturnTemp.pResultString = pReturn->pResultString;
	strcpy(pReturnTemp.curlError, pReturn->curlError);

	delete pReturn;


	return pReturnTemp;
}




// Get something of the page
size_t WebAPIClass::page_get(void *buffer, size_t size, size_t nmemb, void *stream)
{
	// Buffer
	CurlReturn *pReturn = (CurlReturn *)stream;


	// Add buffer
	pReturn->pResultString.append((char *)buffer);


	return size * nmemb;
}




// Get something of the page
size_t WebAPIClass::header_get(void *buffer, size_t size, size_t nmemb, void *stream)
{
	// Buffer
	CurlReturn *pReturn = (CurlReturn *)stream;


	// Add buffer
	pReturn->pResultHeader.append((char *)buffer);


	return size * nmemb;
}





std::string WebAPIClass::urlencode(std::string code)
{
	curl = curl_easy_init();
	std::string ret = "";

	if (curl != NULL)
	{
		char *escaped = curl_easy_escape(curl, code.c_str(), code.length());


		if (escaped != NULL)
		{
			ret = escaped;
			curl_free(escaped);
		}

		curl_easy_cleanup(curl);
	}


	return ret;
}