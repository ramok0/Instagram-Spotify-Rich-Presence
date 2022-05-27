#pragma execution_character_set("utf-8")

#include <thread> //thread -> disconnect thread

#include <conio.h> //getch

#include "framework.h" 
#include "Config.h" //config class
#include "Instagram.h" //instagram class
#include "Spotify.h" //spotify class



void cancelThreadFct(Instagram* insta, InstagramContext* context) {
	while (1) {
		char ch = _getch(); //get key
		if (ch == 's') { //if the key is s (stop)
			if (insta->disconnect(context)) //disconnect to instagram
			{

				curl_global_cleanup(); //cleanup curl
#ifdef DEBUG
				printf("The program will exit by itself.\n");
#else
				std::cout << termcolor::bright_green << "Disconnect from instagram." << std::endl << "You can close the program; it will close in ~50 seconds max." << termcolor::reset << std::endl;
#endif
#ifdef WINDOWS_SYS
				Sleep(3000);
#elif POSIX_SYS
				sleep(3);
#endif //so the user can read the text
			}
			else {
#ifdef DEBUG
				printf("Failed to disconnect, closing\n");
				exit(1);
#else
				std::cout << termcolor::red << "The program failed to exit, please kill this program with taskmanager." << termcolor::reset << std::endl;
#endif
			}
			break;
		}
	}
}

int mai2n();


int main()
{
	//Init curl
	curl_global_init(CURL_GLOBAL_ALL);

	//create ours classes
	ConfigHelper config(CONFIGFILENAME); //to use config file
	Instagram insta(&config); //use instagram's api
	InstagramContext context; //sentive informations like sessionId, accountId, deviceId, csrfToken
	context.shouldDisconnect = false;
	InstagramAccount account; //account informations (firstname, email, and others stuff)
	Spotify spotify; //use spotify's api 
	Utils utils; //some utils functions
#ifndef DEBUG
	std::cout << termcolor::bright_green << "Welcome to Instagram Spotify Rich Presence" << std::endl << "Made by Ramok (https://github.com/Ramokprout)" << std::endl << termcolor::reset;
#endif
	if (insta.login(&context)) { //login to instagram
#ifdef DEBUG
		printf("Connected to instagram !\n");
		printf("Session Id Token : %s\n", context.sessionId.c_str());
#else 
		std::cout << termcolor::bright_blue << "Successfully connected to Instagram !" << termcolor::reset << std::endl;
#endif
		std::thread cancelThread(cancelThreadFct, &insta, &context); //create a thread to disconnect to instagram
		cancelThread.detach();
		if (insta.pullInformations(&context, &account)) { //get account informations
#ifndef DEBUG
			std::cout << termcolor::bright_green << "Successfully got your account informations" << termcolor::reset << std::endl;
#endif
			SpotifySong currentlyPlayed; //to stock currentlyPlayed spotify song
				while (1) {
					try {
						if (context.shouldDisconnect == true) break; //if theres no sessionId, break the loop
						bool changedBio = false; //bool to see if the bio changed in the current iteration
						config.reload(); //reload the config in case smth has changed in it
						std::string biography = config.insta_bio; //get the biography of the user
						SpotifySong songInfo; //stock the current played song and compare with currentlyPlayed to see if the song has changed
						if (spotify.getCurrentListeningSong(&config, &songInfo)) {
							if (songInfo.id != currentlyPlayed.id) { //the song changed ?
								/*
								biography style

								%title% = get the title of the song
								%title-removefeat% = get the title of the song and remove anything after (feat.
								%artist% = get every artists in the song (feat.)
								%first_artist% = get first artist in the list of the artists
								%play% = add an emoji to see if the song is on pause or not
								*/

								if (config.noColors && utils.includes(songInfo.title, "A COLORS SHOW")) {
									utils.fastReplace(&songInfo.title, " - A COLORS SHOW", "");
								}

								utils.fastReplace(&biography, "%title%", songInfo.title);


								if (utils.includes(biography, "%title-removefeat%")) {
									std::string title = songInfo.title;
									size_t featIndex = title.find("(feat."); //get pos of (feat.
									if (featIndex) {
										title = title.substr(0, featIndex);
									}
									utils.fastReplace(&biography, "%title-removefeat%", title); //replace %title-removefeat% with the title without feat
								}



								utils.fastReplace(&biography, "%artist%", songInfo.artists); //replace %artist% by the actual artists
								utils.fastReplace(&biography, "%first_artist%", songInfo.first_artist); //replace %first_artist% with the artist



								if (utils.includes(biography, "%play%")) { //here i use includes because we are defining some vars 
									std::string emoji = songInfo.is_playing ? "▶️" : "⏸️";
									std::string voidChar = "%E3%85%A4";
									std::stringstream toReplace;
									toReplace << "⬅️" << utils.repeat(2, voidChar) << emoji << utils.repeat(2, voidChar) << "➡️";
									utils.fastReplace(&biography, "%play%", toReplace.str()); //replace %play% with our playbar
								}


								//printing the new song name if it has changed
#ifdef DEBUG
								printf("Now playing : %s\n", songInfo.title.c_str());
#else
								std::cout << termcolor::bright_blue << "Now playing : " << songInfo.title << termcolor::reset << std::endl;
#endif
								bool editProfile = insta.editProfile(&context, account, biography); //edit the profile with new biography
								changedBio = true;
#ifdef DEBUG
								printf("EditProfile : %s\n", editProfile ? "true" : "false"); //printing if the profile has been changed
#else
								if (editProfile) {
									std::cout << termcolor::bright_green << "Edited profile successfully." << termcolor::reset << std::endl;
								}
								else {
									std::cout << termcolor::bright_red << "Failed to edit profile !" << termcolor::reset << std::endl;
								}
#endif
								currentlyPlayed = songInfo; //changing the currentlyPlayed variable with the new song
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
#ifdef WINDOWS_SYS
						Sleep(timeout);
#elif POSIX_SYS
						sleep(timeout / 1000);
#endif
					}
					catch (...) {
#ifdef WINDOWS_SYS
						Sleep(15000);
#elif POSIX_SYS
						sleep(15);
#endif
					}
				}
				printf("loop has been broken\n");
				return 1;
			
		}
		else {
#ifdef DEBUG
			printf("Failed to pull account details\n");
#else
			std::cout << termcolor::bright_red << "Failed to get instagram account details !" << termcolor::reset << std::endl;
#endif
		}
	}
	else {
#ifdef DEBUG
	printf("Not connected to instagram\n");
#else
	std::cout << termcolor::bright_red << "Failed to connect to instagram" << termcolor::reset << std::endl;
#endif
	}

#ifdef WINDOWS_SYS
	Sleep(15000);
#elif POSIX_SYS
	sleep(15);
#endif
	printf("\n\nThe program has encountered an unexcepted error.\n");
	return 1;
}