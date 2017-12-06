#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "StatusTimer.h"  
#include "SmashWorld.h"  
#include "GameLibrary\Singleton.h"  

StatusTimer::StatusTimer(sf::Int32 dur_in_frames) {
	starting_frame = 0;
	duration_in_frames = dur_in_frames;
}

bool StatusTimer::IsActive() {
	return starting_frame + duration_in_frames > Singleton<SmashWorld>::Get()->GetCurrentFrame();
}

void StatusTimer::Start() {
	starting_frame = Singleton<SmashWorld>::Get()->GetCurrentFrame();
}

void StatusTimer::Stop() {
	starting_frame = 0;
}