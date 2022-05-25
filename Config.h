#pragma once

#ifndef __CONFIG_FILE_STRUCT_HEADER__
#define __CONFIG_FILE_STRUCT_HEADER__


#include <fstream> // I/O
#include <filesystem> // I/O

#include "framework.h"
#include "nlohmann/json.hpp" //JSON
#include "Utils.h" //UTILS FUNCTIONS



class ConfigHelper {
private:
	std::string fileName;
public:

	std::string spotify_client_id;
	std::string spotify_client_secret;
	std::string spotify_refresh_token;
	std::string spotify_access_token;
	std::string insta_username;
	std::string insta_password;
	std::string insta_bio;



	ConfigHelper(std::string fileName) {
		
		if (!std::filesystem::exists(fileName)) {
			std::ofstream file(fileName);
			if (file.is_open()) {
				file << "{}";
				file.flush();
			}
			file.close();
			printf("Please, complete the config.json file with a correct config\nif you need help, read the README.md file.\n");
			Sleep(15000);
			exit(0);
		}
		else {
			this->fileName = fileName;
			reload();
		}
	}

	void reload() {
		Utils utils;
		std::ifstream file(fileName);
		nlohmann::json j;
		if (file.is_open()) {
			file >> j; //read file
		}
		file.close(); //close the stream as the file has been parsed

		std::vector<std::string> keys = { "spotify_client_id", "spotify_client_secret", "spotify_refresh_token", "spotify_access_token", "insta_username", "insta_password","insta_username", "insta_bio" };
		for (auto& key : keys) {
			if (!utils.jsonExists(j, key)) { //check that every keys exists
				printf("Key %s does not exists!\n", key.c_str());
				exit(0);
			}
		}
		//SPOTIFY REQUIRED
		this->spotify_client_id = std::string(j["spotify_client_id"]);
		this->spotify_client_secret = std::string(j["spotify_client_secret"]);
		this->spotify_refresh_token = std::string(j["spotify_refresh_token"]);
		this->spotify_access_token = std::string(j["spotify_access_token"]);

		//INSTAGRAM REQUIRED
		this->insta_username = std::string(j["insta_username"]);
		this->insta_password = std::string(j["insta_password"]);
		this->insta_bio = std::string(j["insta_bio"]);
	}

	void save() {
		std::ofstream file(this->fileName);
		nlohmann::json j;
		//file => nlohmann::json => json
		j["insta_username"] = this->insta_username;
		j["insta_password"] = this->insta_password;
		j["insta_bio"] = this->insta_bio;
		j["spotify_access_token"] = this->spotify_access_token;
		j["spotify_refresh_token"] = this->spotify_refresh_token;
		j["spotify_client_id"] = this->spotify_client_id;
		j["spotify_client_secret"] = this->spotify_client_secret;

		if (file.is_open()) {
			file << std::setw(4) << j << std::endl;;
		}
		file.flush(); //write
		file.close(); //close the stream
	}

	ConfigHelper() {
	}
};

#endif