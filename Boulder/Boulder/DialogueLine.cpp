#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "DialogueLine.h"
#include "Utilities.h"

DialogueLine::DialogueLine(string pid, DialogueLine* previous_dialogue_line, Json::Value jsonData) {
	PID = pid;
	PreviousDialogueLine = previous_dialogue_line;

	for (int i = 0; i < (int)jsonData["passages"].size(); i++) {
		string peekPid = jsonData["passages"][i]["pid"].asString();

		if (PID == peekPid) {
			Line = jsonData["passages"][i]["text"].asString();

			std::vector<string> vstrings = Utilities::Split(Line, '[');
			Line = vstrings[0];
			
			if (Utilities::Contains(Line, "|ExecuteActions|")) {
				vstrings = Utilities::Split(Line, '|');
				Line = vstrings[0];
				ExecuteActionsWhenLineIsHit = true;
			}

			Name = jsonData["passages"][i]["name"].asString();

			Tags = std::vector<string>();
			if ((int)jsonData["passages"][i]["tags"].size() > 0) {
				for (int tag = 0; tag < (int)jsonData["passages"][i]["tags"].size(); tag++) {
					Tags.push_back(jsonData["passages"][i]["tags"][tag].asString());
				}
			}

			if ((int)jsonData["passages"][i]["links"].size() > 0) {
				for (int link = 0; link < (int)jsonData["passages"][i]["links"].size(); link++) {
					string linkPid = jsonData["passages"][i]["links"][link]["pid"].asString();
					NextDialogueLine = new DialogueLine(linkPid, this, jsonData);
				}
			}

			break;
		}
	}
}

bool DialogueLine::DoesLineAlreadyExist(string pid) {
	if (PID == pid) {
		return true;
	}

	if (NextDialogueLine != nullptr) {
		return NextDialogueLine->DoesLineAlreadyExist(pid);
	}

	return false;
}

DialogueLine* DialogueLine::GetNextDialogueLine(string stage_of_the_game) {
	if (NextDialogueLine != nullptr) {
		for (int i = 0; i < (int)NextDialogueLine->Tags.size(); i++) {
			if (NextDialogueLine->Tags[i] == stage_of_the_game) {
				return NextDialogueLine;
			}
		}

		return NextDialogueLine->GetNextDialogueLine(stage_of_the_game);
	} else {
		return nullptr;
	}

	return nullptr;
}

DialogueLine* DialogueLine::GetFirstRelevantDialogueLine(string stage_of_the_game) {
	for (int i = 0; i < (int)Tags.size(); i++) {
		if (Tags[i] == stage_of_the_game) {
			return this;
		}
	}

	if (NextDialogueLine != nullptr) {
		return NextDialogueLine->GetFirstRelevantDialogueLine(stage_of_the_game);
	}

	return nullptr;
}