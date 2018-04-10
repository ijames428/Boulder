#ifndef DIALOGUE_LINE_H
#define DIALOGUE_LINE_H

using namespace std;
#include <SFML/Graphics.hpp>
#include "GameLibrary\Json\json.h"

class DialogueLine {
private:
	DialogueLine* NextDialogueLine;
	DialogueLine* PreviousDialogueLine;
	bool DoesLineAlreadyExist(string pid);
public:
	DialogueLine(string pid, DialogueLine* previous_dialogue_line, Json::Value jsonData);
	string PID;
	string Line;
	string Name;
	std::vector<string> Tags;
	DialogueLine* GetNextDialogueLine(string stage_of_the_game);
	DialogueLine* GetFirstRelevantDialogueLine(string stage_of_the_game);

	//static DialogueLine* DialogueLineTreeBuilder(Json::Value jsonData);
};

#endif