#pragma once

#ifndef __FRAMEWORK_HEADER_HS__
#define __FRAMEWORK_HEADER_HS__



#include <string> // structs
#define CONFIGFILENAME "config.json"

#ifndef DEBUG
#include "termcolor.h"
#endif

struct InstagramAccount {
	std::string chaining_enabled;
	std::string external_url;
	std::string insta_email;
	std::string insta_first_name;
	std::string insta_username;
	std::string phone_number;
};

struct InstagramContext {
	std::string accountId;
	std::string sessionId;
	std::string csrfToken;
	std::string deviceId;
	bool shouldDisconnect;
};

struct InstagramSharedData {
	std::string device_id;
	std::string csrf_token;
};

#endif