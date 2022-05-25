#include "Instagram.h"
#include "Utils.h"
#include "Spotify.h"



bool Instagram::getSharedData(InstagramSharedData* data)
{
	Utils utils;
	CURL* curl;
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://www.instagram.com/data/shared_data/");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

		res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
			return false;
		}

		curl_easy_cleanup(curl);
		try {
			nlohmann::json json = nlohmann::json::parse(readBuffer);
			if (utils.jsonExists(json, "config")) {
				if (utils.jsonExists(json["config"], "csrf_token")) {
					data->csrf_token = std::string(json["config"]["csrf_token"]);
				}
			}

			if (utils.jsonExists(json, "device_id")) {
				data->device_id = std::string(json["device_id"]);
			}

			return true;
		}
		catch (std::exception ex) {
			return false;
		}
	}
	else {
		return false;
	}
}

bool Instagram::login(InstagramContext* context)
{
	Utils utils;
	InstagramSharedData data;
	if (!getSharedData(&data)) {
		printf("Error while getting shared data\n");
		return false;
	}
	else {
		context->csrfToken = data.csrf_token;
		context->deviceId = data.device_id;

		CURL* curl;
		CURLcode res;
		std::string responseBuffer;

		curl = curl_easy_init();

		if (curl) {
			std::string body = "enc_password=" + utils.urlencode(this->config->insta_password) + "&username=" + utils.urlencode(this->config->insta_username) + "&queryParams=" + utils.urlencode("{}") + "&optIntoOneTap=false&stopDeletionNonce=&trustedDeviceRecords=" + utils.urlencode("{}");
			struct curl_slist* headers = NULL;
			std::string csrfTokenHeader = "X-CSRFToken: " + context->csrfToken;
			headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.67 Safari/537.36");
			headers = curl_slist_append(headers, "Accept-Encoding: gzip,deflate");
			headers = curl_slist_append(headers, "Connection: close");
			headers = curl_slist_append(headers, csrfTokenHeader.c_str());
			curl_easy_setopt(curl, CURLOPT_URL, "https://www.instagram.com/accounts/login/ajax/");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
	

			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
				return false;
			}

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
			struct curl_slist* cookies = NULL;
			curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies);
			if (http_code == 200) {
					nlohmann::json json = nlohmann::json::parse(responseBuffer);
						if (utils.jsonExists(json, "status")) {
							if (std::string(json["status"]) == "fail") {
								printf("Status fail :/\n");
								return false;
						}

							struct curl_slist* each = cookies;
							while (each) {
									
								std::string stringed = std::string(each->data);
								printf("Raw Cookie : %s\n", stringed.c_str());
								std::vector<std::string> occurences = utils.split(stringed, "\t");
								printf("Name : %s\n", occurences[5].c_str());
								if (occurences[5] == "sessionid") {
									context->sessionId = occurences[6];
								}
								else if (occurences[5] == "ds_user_id") {
									context->accountId = occurences[6];
								}
								
								each = each->next;
							}

		
						curl_slist_free_all(headers);
						curl_easy_cleanup(curl);
						return true;
				}
				else {
					printf("An error occured while logging in.\n");
					curl_slist_free_all(headers);
					curl_slist_free_all(cookies);
					curl_easy_cleanup(curl);
					return false;
				}
			}
			else {
				printf("Status code : %d\n", http_code);
				return false;
			}
		}
		else {
			printf("Error while initializating curl !\n");
			return false;
		}

		
	}
}

bool Instagram::disconnect(InstagramContext* context)
{
	Utils utils;
	nlohmann::json body;
	body["user_id"] = context->accountId;
	body["one_tap_app_login"] = "0";
	if (utils.Request("https://www.instagram.com/accounts/logout/ajax/", nullptr, body.dump(), context, false, "application/json")) {
		context->accountId = "";
		context->deviceId = "";
		context->sessionId = "";
		context->csrfToken = "";
		printf("Disconnected from Instagram\n");
	}
	else {
		printf("Failed to disconnect properly\n");
		exit(1);
	}
	return false;
}

bool Instagram::pullInformations(InstagramContext* context, InstagramAccount* account)
{
	Utils utils;
	nlohmann::json response;
	if (utils.Request("https://www.instagram.com/accounts/edit/?__a=1&__d=dis", &response, "", context, true)) {
		if (utils.jsonExists(response, "form_data")) {
			account->insta_first_name = std::string(response["form_data"]["first_name"]);
			account->insta_email = std::string(response["form_data"]["email"]);
			account->insta_username = std::string(response["form_data"]["username"]);
			account->external_url = std::string(response["form_data"]["external_url"]);
			account->chaining_enabled = "";
			account->phone_number = std::string(response["form_data"]["phone_number"]);
			printf("Pulled informations\n");
			return true;
		}
		else {
			printf("No form data\n");
			return false;
		}
	}
	else {
		printf("Request failed\n");
		return false;
	}
}

std::string phoneNumberToCorrectPhone(std::string phoneNumber) {
	if (phoneNumber.size() == 0) return phoneNumber;
	std::string phone = phoneNumber.replace(phoneNumber.find("+"), 1, "%2B");
	std::replace(phone.begin(), phone.end(), ' ', '+');
	return phone;
}

bool Instagram::editProfile(InstagramContext* context,InstagramAccount account, std::string biography)
{
	Utils utils;
	std::stringstream body;
	body << "first_name=" << utils.urlencode(account.insta_first_name) << "&username=" << account.insta_username << "&email=" << utils.urlencode(account.insta_email) << "&phone_number=" << phoneNumberToCorrectPhone(account.phone_number) << "&external_url=" << utils.urlencode(account.external_url) << "&chaining_enabled=" << utils.urlencode(account.chaining_enabled) << "&biography=" << biography;
	nlohmann::json response;
	return utils.Request("https://www.instagram.com/accounts/edit/", &response, body.str(), context);
}

Instagram::Instagram(ConfigHelper* helper)
{
	this->config = helper;
}

Instagram::Instagram()
{
}
