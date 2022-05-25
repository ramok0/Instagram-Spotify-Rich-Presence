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
		char ch = _getch(); //get key
		if (ch == 's') { //if the key is s (stop)
			insta->disconnect(context); //disconnect to instagram
			curl_global_cleanup(); //cleanup curl
			printf("You can close the program; it will close in ~50 seconds max.\n");
			Sleep(3000); //so the user can read the text
			break;
		}
	}
}

int main()
{
	//Init curl
	curl_global_init(CURL_GLOBAL_ALL);

	//create ours classes
	ConfigHelper config(CONFIGFILENAME); //to use config file
	Instagram insta(&config); //use instagram's api
	InstagramContext context; //sentive informations like sessionId, accountId, deviceId, csrfToken
	InstagramAccount account; //account informations (firstname, email, and others stuff)
	Spotify spotify; //use spotify's api 
	Utils utils; //some utils functions
	if (insta.login(&context)) { //login to instagram
		printf("Connected to instagram !\n");
#ifdef DEBUG
		printf("Session Id Token : %s\n", context.sessionId.c_str());
#endif
		std::thread cancelThread(cancelThreadFct, &insta, &context); //create a thread to disconnect to instagram
		cancelThread.detach();
		if (insta.pullInformations(&context, &account)) { //get account informations
			SpotifySong currentlyPlayed; //to stock currentlyPlayed spotify song
			if (spotify.getCurrentListeningSong(&config, &currentlyPlayed)) { //get the current listening song on spotify
				bool firstChange = false; //bool to see if the bio changed 1 time since the program has started
				while (1) {
					if (context.sessionId.size() == 0) break; //if theres no sessionId, break the loop
					bool changedBio = false; //bool to see if the bio changed in the current iteration
					config.reload(); //reload the config in case smth has changed in it
					std::string biography = config.insta_bio; //get the biography of the user
					SpotifySong songInfo; //stock the current played song and compare with currentlyPlayed to see if the song has changed
					if (spotify.getCurrentListeningSong(&config, &songInfo)) { 
						if (songInfo.id != currentlyPlayed.id || firstChange == false) { //does the song changed ?
							/*
							biography style

							%title% = get the title of the song
							%artist% = get every artists in the song (feat.)
							%first_artist% = get first artist in the list of the artists
							%play% = add an emoji to see if the song is on pause or not
							*/
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
								std::string emoji = songInfo.is_playing ? "▶️" : "⏸️";
								std::string voidChar = "%E3%85%A4";
								std::stringstream toReplace;
								toReplace << "⬅️" << utils.repeat(2, voidChar) << emoji << utils.repeat(2, voidChar) << "➡️";
								biography = biography.replace(biography.find("%play%"), 6, toReplace.str());
							}


							//printing the new song name if it has changed
							printf("Now playing : %s\n", songInfo.title.c_str());
							bool editProfile = insta.editProfile(&context, account, biography); //edit the profile with new biography
							changedBio = true; 
							printf("EditProfile : %s\n", editProfile ? "true" : "false"); //printing if the profile has been changed
							currentlyPlayed = songInfo; //changing the currentlyPlayed variable with the new song
							if (firstChange == false) {
								firstChange = true;
							}
						}
					}

					/*
					* timeout = 1000ms for spotify api
					* 50000ms if the profile has been edited (because of instagram's rate limit)
					*/
					int timeout = 1000; 
					if (changedBio == true) {
						timeout += 49000;
					}
					Sleep(timeout);
				}

				return 0;
			}
		}
		else {
			printf("Failed to pull account details\n");
		}
	}
	else {
		printf("Not connected to instagram\n");
	}

	Sleep(15000);
	printf("\n\nThe program has encountered an unexcepted error.\n");
	return 1;
}