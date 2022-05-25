#pragma once
#ifndef __INSTAGRAM_CLASS_MAIN__
#define __INSTAGRAM_CLASS_MAIN__

#include "framework.h"
#include <iostream>
#include <string>

#include "Config.h"

#include "InstagramContext.h"
#include "InstagramAccount.h"

struct InstagramSharedData {
	std::string device_id;
	std::string csrf_token;
};



class Instagram
{

public:
	ConfigHelper* config;

	bool getSharedData(InstagramSharedData* data);

	bool login(InstagramContext* context);

	bool disconnect(InstagramContext* context);

	bool pullInformations(InstagramContext* context, InstagramAccount* account);

	bool editProfile(InstagramContext* context, InstagramAccount account, std::string biography);


	Instagram(ConfigHelper* helper);

	Instagram();
};

#endif

