#pragma once
#ifndef __SPOTIFY_INTEGRATION_HEADER__
#define __SPOTIFY_INTEGRATION_HEADER__
#include "framework.h"
#include "curl/curl.h" //curl functions
#include "nlohmann/json.hpp" //json
#include "Config.h" //config helper class
#include "Utils.h" //utils functions
#include "base64.h" //base 64 -> spotify basic auth

struct SpotifySong {
	std::string id;
	std::string title;
	std::string artists;
	std::string first_artist;
	bool is_playing;
};

class Spotify
{

private: 	
	   bool requestSpotify(std::string url, ConfigHelper* config, nlohmann::json* response);

	   bool generateTokenFromRefresh(ConfigHelper* config);
public:

	bool getCurrentListeningSong(ConfigHelper* config, SpotifySong* buffer);


};

#endif

