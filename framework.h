#pragma once

#ifndef __FRAMEWORK_HEADER_HS__
#define __FRAMEWORK_HEADER_HS__



#include <string> // structs
#define CONFIGFILENAME "config.json"



#ifndef DEBUG
#include "termcolor.h"
#endif

#if defined(_WIN32) || defined(_WIN64)
#   define WINDOWS_SYS
#include <Windows.h>
#elif defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
#   define POSIX_SYS
#include <unistd.h>
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