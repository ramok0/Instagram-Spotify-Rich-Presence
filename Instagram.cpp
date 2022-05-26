#include "Instagram.h"



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
			nlohmann::json json = nlohmann::json::parse(readBuffer); //parsing the response
			if (utils.jsonExists(json, "config")) { //if the config object exists
				if (utils.jsonExists(json["config"], "csrf_token")) { //if the csrf_token property exists in the config object
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
	if (!getSharedData(&data)) { //getting csrftoken & deviceId
#ifdef DEBUG
		printf("Error while getting shared data\n");
#endif
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
			headers = curl_slist_append(headers, "Connection: close");
			headers = curl_slist_append(headers, csrfTokenHeader.c_str());
			curl_easy_setopt(curl, CURLOPT_URL, "https://www.instagram.com/accounts/login/ajax/");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
			curl_easy_setopt(curl, CURLOPT_POST, 1L); 
			curl_easy_setopt(curl, CURLOPT_COOKIEFILE, ""); //enabling cookies
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str()); 
	

			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
				return false;
			}

			long http_code = 0;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code); //getting the response code
			struct curl_slist* cookies = NULL;
			curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookies); //getting the cookies
			if (http_code == 200) {
					nlohmann::json json = nlohmann::json::parse(responseBuffer); //parsing response
						if (utils.jsonExists(json, "status")) {
							if (std::string(json["status"]) == "fail") { //checking if the status is "fail"
								printf("Status fail :/\n");
								return false;
					     	}

							struct curl_slist* each = cookies; //parsing cookies 
#ifdef DEBUG
							printf("Parsing cookies...\n");
#else
							std::cout << termcolor::bright_green << "Parsing cookies" << termcolor::reset << std::endl;
#endif
							while (each) {
									
								std::string stringed = std::string(each->data);
								std::vector<std::string> occurences = utils.split(stringed, "\t");
								if (occurences[5] == "sessionid") { //getting the sessionid value
									context->sessionId = occurences[6];
								}
								else if (occurences[5] == "ds_user_id") { //getting the ds_user_id value
									context->accountId = occurences[6];
								}
								
								each = each->next;
							}

		
						curl_slist_free_all(headers); //cleanup headers
						curl_slist_free_all(cookies); //cleanup cookies
						curl_easy_cleanup(curl); //cleanup curl
						return true;
				}
				else {
#ifdef DEBUG
							printf("An error occured while logging in.\n");
#else 
							std::cout << termcolor::bright_red << "An error occured while logging in." << termcolor::reset << std::endl;
#endif
					curl_slist_free_all(headers); //cleanup headers
					curl_slist_free_all(cookies); //cleanup cookies
					curl_easy_cleanup(curl); //cleanup curl
					return false;
				}
			}
			else {
				//print the status code in case of errors
#ifdef DEBUG
				printf("Status code : %d\n", http_code); 
#else 
				std::cout << termcolor::bright_red << "The request ended with status code : " << http_code << std::endl << "Learn more here : " << "https://developer.mozilla.org/fr/docs/Web/HTTP/Status" << termcolor::reset << std::endl;
#endif
				return false;
			}
		}
		else {
#ifdef DEBUG
			printf("Error while initializating curl !\n");
#else 
			std::cout << termcolor::bright_red << "Error while initializating curl !" << termcolor::reset << std::endl;
#endif
			return false;
		}

		
	}
}

bool Instagram::disconnect(InstagramContext* context)
{
	Utils utils;
	nlohmann::json body; 
	//body {user_id: "<userid>", one_tap_app_login: "0"}
	body["user_id"] = context->accountId;
	body["one_tap_app_login"] = "0";
	//post request with body as json without catching the response
	if (utils.Request("https://www.instagram.com/accounts/logout/ajax/", nullptr, body.dump(), context, false, "application/json")) {
		context->accountId = ""; //remove every properties to the context because they do not exists anymore
		context->deviceId = "";
		context->sessionId = "";
		context->csrfToken = "";
		context->shouldDisconnect = true;
		return true;
	}
	else {
		printf("Failed to disconnect properly\n");
		exit(1);
		return false; //returning false even if the program is closed 
	}
	return false;
}

bool Instagram::pullInformations(InstagramContext* context, InstagramAccount* account)
{
	Utils utils;
	nlohmann::json response; //to catch the response
	//get request to the instagram's api
	if (utils.Request("https://www.instagram.com/accounts/edit/?__a=1&__d=dis", &response, "", context, true)) {
		if (utils.jsonExists(response, "form_data")) { //checking if form data exists in the response
			account->insta_first_name = std::string(response["form_data"]["first_name"]); 
			account->insta_email = std::string(response["form_data"]["email"]);
			account->insta_username = std::string(response["form_data"]["username"]);
			account->external_url = std::string(response["form_data"]["external_url"]);
			account->chaining_enabled = "";
			account->phone_number = std::string(response["form_data"]["phone_number"]);
			return true;
		}
		else {
#ifdef DEBUG
			printf("No form data\n");
#endif
			return false;
		}
	}
	else {
#ifdef DEBUG
		printf("Request failed\n");
#endif
		return false;
	}
}

std::string phoneNumberToCorrectPhone(std::string phoneNumber) {
	if (phoneNumber.size() == 0) return phoneNumber; //if theres no phoneNumber, we're not parsing it #logic
	std::string phone = phoneNumber.replace(phoneNumber.find("+"), 1, "%2B"); //replace + with %2B
	std::replace(phone.begin(), phone.end(), ' ', '+'); //replace " " with +
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
