#pragma execution_character_set("utf-8")

#include <curl/curl.h>
#include <thread>

#include <stdio.h>
#include <conio.h>

#include "framework.h"
#include "Config.h"
#include "Instagram.h"
#include "Spotify.h"
#include "InstagramAccount.h";

void cancelThreadFct(Instagram* insta, InstagramContext* context) {
	while (1) {
		char ch = _getch();
		if (ch == 's') {
			//disconnect
			printf("Should disconnect\n");
			insta->disconnect(context);
		}
	}
}

int main()
{
	curl_global_init(CURL_GLOBAL_ALL);
	ConfigHelper config(CONFIGFILENAME);
	Instagram insta(&config);
	InstagramContext context;
	InstagramAccount account;
	Spotify spotify;
	if (insta.login(&context)) {
		printf("Connected to instagram !\n");
		printf("Session Id Token : %s\n", context.sessionId.c_str());
		std::thread cancelThread(cancelThreadFct, &insta, &context);
		if (insta.pullInformations(&context, &account)) {
			SpotifySong currentlyPlayed;
			if (spotify.getCurrentListeningSong(&config, &currentlyPlayed)) {
				bool firstChange = false;
				while (1) {
					bool changedBio = false;
					config.reload();
					std::string biography = config.insta_bio;
					SpotifySong songInfo;
					if (spotify.getCurrentListeningSong(&config, &songInfo)) {

						if (songInfo.id != currentlyPlayed.id || firstChange == false) {
							if (biography.find("%title%") != std::string::npos) {
								biography = biography.replace(biography.find("%title%"), 7, songInfo.title);
							}

							if (biography.find("%artist%") != std::string::npos) {
								biography = biography.replace(biography.find("%artist%"), 8, songInfo.artists);
							}

							if (biography.find("%first_artist%") != std::string::npos) {
								biography = biography.replace(biography.find("%first_artist%"), 14, songInfo.first_artist);
							}

							if (biography.find("%play%") != std::string::npos) {
								std::string emoji = "";
								if (songInfo.is_playing == true) {
									emoji = "▶️";
								}
								else {
									emoji = "⏸️";
								}
								std::string toWrite = "⬅️%E3%85%A4%E3%85%A4" + emoji + "%E3%85%A4%E3%85%A4➡️";
								biography = biography.replace(biography.find("%play%"), 6, toWrite);
							}


							///	printf("Biography : %s\n", biography.c_str());
							printf("Now playing : %s\n", songInfo.title.c_str());
							bool editProfile = insta.editProfile(&context, account, biography);
							changedBio = true;
							printf("EditProfile : %s\n", editProfile ? "true" : "false");
							currentlyPlayed = songInfo;
							if (firstChange == false) {
								firstChange = true;
							}
						}
					}

					int timeout = 1000;
					if (changedBio == true) {
						timeout += 49000;
					}
					Sleep(timeout);
				}
			}
		}
		else {
			printf("Failed to pull account details\n");
			Sleep(15000);
			exit(1);
		}
	}
	else {
		printf("Not connected to instagram\n");
	}

	Sleep(15000);
	printf("\n\nEnd of program\n");

}