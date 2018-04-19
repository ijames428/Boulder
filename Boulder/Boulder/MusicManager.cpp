#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "MusicManager.h"  
#include "GameLibrary\Settings.h"
#include "GameLibrary\Singleton.h"

MusicManager::MusicManager() {
	fadeTime = defaultFadeTime = 60;
	fadeStartTime = 0;
	fadingInSong = "";
}

void MusicManager::Update() {
	internalFrame++;

	if (fading) {
		fadeInPercent = (internalFrame - fadeStartTime) / (float)fadeTime;

		if (fadeInPercent > 1.0f) {
			fadeInPercent = 1.0f;
		} else if (fadeInPercent < 0.0f) {
			fadeInPercent = 0.0f;
		}

		fadeOutPercent = 1.0f - fadeInPercent;

		for (std::map<string, sf::Music*>::iterator song = songs.begin(); song != songs.end(); ++song) {
			if (song->first == fadingInSong) {
				if (song->second->getVolume() < Singleton<Settings>::Get()->music_volume * fadeInPercent) {
					song->second->setVolume(Singleton<Settings>::Get()->music_volume * fadeInPercent);
				}

				if (fadeInPercent >= 1.0f) {
					fading = false;
					song->second->setVolume(Singleton<Settings>::Get()->music_volume);
				}
			} else {
				if (song->second->getVolume() > Singleton<Settings>::Get()->music_volume * fadeOutPercent) {
					song->second->setVolume(Singleton<Settings>::Get()->music_volume * fadeOutPercent);
				}

				if (fadeOutPercent <= 0.0f) {
					song->second->setVolume(0.0f);
				}
			}
		}
	}
}

sf::Music* MusicManager::AddSong(string song_file_path_and_name) {
	sf::Music* new_music = new sf::Music();
	if (!new_music->openFromFile(song_file_path_and_name))
		return nullptr;
	new_music->setVolume(0.0f);
	new_music->play();
	new_music->setLoop(true);

	songs[song_file_path_and_name] = new_music;

	return new_music;
}

void MusicManager::FadeToSong(string song_file_path_and_name, int fade_time) {
	if (fade_time == -1) {
		fadeTime = defaultFadeTime;
	} else {
		fadeTime = fade_time;
	}

	fading = true;
	fadingInSong = song_file_path_and_name;
	fadeStartTime = internalFrame;
}

void MusicManager::RestartSong(string song_file_path_and_name) {
	for (std::map<string, sf::Music*>::iterator song = songs.begin(); song != songs.end(); ++song) {
		if (song->first == song_file_path_and_name) {
			song->second->stop();
			song->second->play();
		}
	}
}