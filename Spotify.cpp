#include "Spotify.h"

bool Spotify::getCurrentListeningSong(ConfigHelper* config, SpotifySong* songbuffer)
{
	nlohmann::json response;
	if (requestSpotify("https://api.spotify.com/v1/me/player/currently-playing", config, &response)) {
		std::string title = std::string(response["item"]["name"]);
		std::string artists;
		for (auto& artist : response["item"]["artists"]) {
			artists += std::string(artist["name"]) + " & ";
		}

		artists = artists.substr(0, artists.size() - 3);
		songbuffer->title = title;
		songbuffer->artists = artists;
		songbuffer->id = std::string(response["item"]["id"]);
		songbuffer->first_artist = std::string(response["item"]["artists"][0]["name"]);
		//songbuffer->progress_ms = response["progress_ms"];
		//songbuffer->total_ms = response["item"]["duration_ms"];

		songbuffer->is_playing = response["is_playing"].get<bool>();
		return true;
	}
	else {
		return false;
	}

}

bool Spotify::requestSpotify(std::string url, ConfigHelper* config, nlohmann::json* response)
{
	Utils utils;
	CURL* curl;
	CURLcode res;
	std::string responseBuffer;
	curl = curl_easy_init();
	if (curl) {
		struct curl_slist* headers = NULL;
		std::string authorizationHeader = "Authorization: Bearer " + config->spotify_access_token;
		headers = curl_slist_append(headers, authorizationHeader.c_str());
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
			return false;
		}

		long http_status = 0;

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);

		if (http_status == 200) {
			*response = nlohmann::json::parse(responseBuffer);
			return true;
		}
		else if (http_status == 401) {
			if (generateTokenFromRefresh(config)) {
				return requestSpotify(url, config, response);
			}
			else {
				return false;
			}
		}
		else if (http_status == 204) {
			return false;
		}
		else {
			printf("Error, request to spotify api ended with status %d\n", http_status);
			return false;
		}
	}
	else {
		printf("An error occured while initializating curl!\n");
		curl_easy_cleanup(curl);
		return false;
	}

}

bool Spotify::generateTokenFromRefresh(ConfigHelper* config)
{
	CURL* curl;
	CURLcode res;
	Utils utils;
	std::string responseBuffer;
	curl = curl_easy_init();
	if (curl) {
		struct curl_slist* headers = NULL;
		std::string toB64 = config->spotify_client_id + ":" + config->spotify_client_secret;
		std::string authorizationHeader = "Authorization: Basic " + base64_encode(toB64);
	//	printf("Authorization Header : %s\n", authorizationHeader.c_str());
#ifdef DEBUG
		printf("Refreshed Spotify's authorization header.\n");
#endif
		std::string body = "grant_type=refresh_token&refresh_token=" + utils.urlencode(config->spotify_refresh_token);
		headers = curl_slist_append(headers, authorizationHeader.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
		curl_easy_setopt(curl, CURLOPT_URL, "https://accounts.spotify.com/api/token");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
			return false;
		}
		long http_status = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
		curl_easy_cleanup(curl);
		if (http_status == 200) {

			nlohmann::json response = nlohmann::json::parse(responseBuffer);
			if (utils.jsonExists(response, "access_token")) {
				config->spotify_access_token = std::string(response["access_token"]);
				config->save();
				return true;
			}
			else return false;
		}
		else {
			return false;
		}
	}
	else {
		printf("An error occured while initializating curl!\n");
		curl_easy_cleanup(curl);
		return false;
	}
}
