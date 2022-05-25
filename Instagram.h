#pragma once
#ifndef __INSTAGRAM_CLASS_MAIN__
#define __INSTAGRAM_CLASS_MAIN__

#include "framework.h"
#include <iostream>
#include <string>

#include "Config.h"





class Instagram
{
private : 
	bool getSharedData(InstagramSharedData* data);
public:
	ConfigHelper* config;


	bool login(InstagramContext* context);

	bool disconnect(InstagramContext* context);

	bool pullInformations(InstagramContext* context, InstagramAccount* account);

	bool editProfile(InstagramContext* context, InstagramAccount account, std::string biography);


	Instagram(ConfigHelper* helper);

	Instagram();
};

#endif

