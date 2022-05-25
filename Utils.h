#pragma once

#ifndef __HEADER_GUARD_UTILS_CLASS__
#define __HEADER_GUARD_UTILS_CLASS__
#include <curl/curl.h> //request function
#include "nlohmann/json.hpp" //json 
#include "framework.h" 

class Utils {
private :    static void hexchar(unsigned char c, unsigned char& hex1, unsigned char& hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}
public :
	static size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp)
	{
		((std::string*)userp)->append((char*)contents, size * nmemb);
		return size * nmemb;
	}

	static bool jsonExists(nlohmann::json json, std::string key) {
		return json.find(key) != json.end();
	}


	static std::string repeat(int n, std::string toRepeat) {
		std::ostringstream os;
		for (int i = 0; i < n; i++)
			os << toRepeat;
		return os.str();
	}


    static std::string urlencode(std::string s)
    {
        const char* str = s.c_str();
        std::vector<char> v(s.size());
        v.clear();
        for (size_t i = 0, l = s.size(); i < l; i++)
        {
            char c = str[i];
            if ((c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                c == '-' || c == '_' || c == '.' || c == '!' || c == '~' ||
                c == '*' || c == '\'' || c == '(' || c == ')')
            {
                v.push_back(c);
            }
            else if (c == ' ')
            {
                v.push_back('+');
            }
            else
            {
                v.push_back('%');
                unsigned char d1, d2;
                hexchar(c, d1, d2);
                v.push_back(d1);
                v.push_back(d2);
            }
        }

        return std::string(v.cbegin(), v.cend());
    }


	bool Request(std::string url, nlohmann::json* response, std::string body, InstagramContext* context, bool get = false, std::string contentType = "application/x-www-form-urlencoded") {
		CURL* curl = nullptr;
		curl = curl_easy_init();
		if (curl) {
			CURLcode res;
			Utils utils;
			std::string responseBuffer;
			struct curl_slist* headers = NULL;
			std::string csrfTokenHeader = "X-CSRFToken: " + context->csrfToken;
			headers = curl_slist_append(headers, ("Content-Type: " + contentType).c_str());
			headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/101.0.4951.67 Safari/537.36");
			headers = curl_slist_append(headers, "Connection: close");
			headers = curl_slist_append(headers, csrfTokenHeader.c_str());
			std::string cookie = "sessionid=" + context->sessionId + ";";
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip");
			curl_easy_setopt(curl, CURLOPT_COOKIE, cookie.c_str());
			if (get == false) {
				curl_easy_setopt(curl, CURLOPT_POST, 1L);
				curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
			}

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, utils.WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				printf("curl_easy_perform return %s [%d]\n", curl_easy_strerror(res), res);
				return false;
			}
			long http_status = 0;

			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
		

			try {
				if (response != nullptr) {
					*response = nlohmann::json::parse(responseBuffer);
				}
	
			}
			catch (std::exception ex) {
			}



			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
			return true;
		}
		else {
			printf("An error occured while initializing curl !\n");
			return false;
		}


		return false;

	}

	static std::vector<std::string> split(std::string str, std::string token) {
		std::vector<std::string>result;
		while (str.size()) {
			size_t index = str.find(token);
			if (index != std::string::npos) {
				result.push_back(str.substr(0, index));
				str = str.substr(index + token.size());
				if (str.size() == 0)result.push_back(str);
			}
			else {
				result.push_back(str);
				str = "";
			}
		}
		return result;
	}

};

#endif