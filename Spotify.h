#pragma once
#ifndef __SPOTIFY_INTEGRATION_HEADER__
#define __SPOTIFY_INTEGRATION_HEADER__
#include "framework.h"
#include "curl/curl.h"
#include "nlohmann/json.hpp"
#include <iostream>

#include "Config.h"
#include "Utils.h"
#include "base64.h"

struct SpotifySong {
	std::string id;
	std::string title;
	std::string artists;
	std::string first_artist;
	bool is_playing;
};

class Spotify
{
public:

	bool getCurrentListeningSong(ConfigHelper* config, SpotifySong* buffer);

	bool requestSpotify(std::string url, ConfigHelper* config, nlohmann::json* response);

	bool generateTokenFromRefresh(ConfigHelper* config);
};

#endif

