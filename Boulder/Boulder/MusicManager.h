#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

using namespace std; 
#include <map>
#include <SFML/Audio.hpp>

class MusicManager {
private:
	sf::Int64 internalFrame;
	sf::Int64 fadeTime;
	sf::Int64 defaultFadeTime;
	sf::Int64 fadeStartTime;
	std::map<string, sf::Music*> songs;
	string fadingInSong;
	float fadeInPercent;
	float fadeOutPercent;
	bool fading;
public:
	MusicManager();
	void Update();
	sf::Music* AddSong(string song_file_path_and_name);
	void RestartSong(string song_file_path_and_name);
	void FadeToSong(string song_file_path_and_name, int fade_time = -1);
};

#endif