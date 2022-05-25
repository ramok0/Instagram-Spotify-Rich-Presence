#pragma once

#ifndef __INSTAGRAMCONTEXTFDP__
#define __INSTAGRAMCONTEXTFDP__

#include <iostream>
struct InstagramContext {
	std::string accountId;
	std::string sessionId;
	std::string csrfToken;
	std::string deviceId;
};

#endif