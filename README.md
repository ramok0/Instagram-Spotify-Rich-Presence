# Instagram Spotify Rich Presence
A little program to put what you are listening on spotify in your instagram biography.
This program might get your account rate limited.

## 📸 Screenshots

![It looks like this](https://media.discordapp.net/attachments/971422905328095255/979073568023408690/Untitled.png)

## ⚡ Performances

This program is well optimized so you can leave it in the background; it will not eat more than 5mo of ram and 1% of CPU

## 💻 Run Locally

- Download the [latest release](https://github.com/Ramokprout/Instagram-Spotify-Rich-Presence/releases/latest) or build it yourself

- Create a spotify application

- Create a spotify token with your account (with the current-listening scope)

- Put every infos in your config.json

- Start the .exe

- Enjoy

## 🔧 Config file

| Key                    | Description                                          | Key          |
| :--------              | :-------                                             | :----------  |
| `insta_username`       | your instagram insta_username                        | **String**   |
| `insta_password`       | your instagram enc password                          | **String**   |
| `insta_bio`            | your instagram biography with format                 | **String**   |
| `spotify_access_token` | your spotify access_token                            | **String**   |
| `spotify_refresh_token`| your spotify refresh_token                           | **String**   |
| `spotify_client_id`    | your spotify's app client id                         | **String**   |
| `spotify_client_secret`| your spotify's app client secret                     | **String**   |
| `noColors`             | Remove the "A COLORS SHOW" in the title of the song  | **Boolean**  |
