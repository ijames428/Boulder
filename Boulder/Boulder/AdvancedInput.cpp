#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "AdvancedInput.h"  
#define pi 3.14159265359f

AdvancedInput::AdvancedInput(int number_of_frames_in_queue) {
	numberOfFramesInQueue = number_of_frames_in_queue;
	pastStickInputAngles = std::vector<float>();
}

void AdvancedInput::AddStickInput(float x, float y) {
	float new_angle = atan2(-y, x) * 180 / pi;

	//if (new_angle < 0) {
	//	new_angle = 360.0f + new_angle;
	//}

	pastStickInputAngles.push_back(new_angle);

	if ((int)pastStickInputAngles.size() > numberOfFramesInQueue) {
		pastStickInputAngles.erase(pastStickInputAngles.begin());
	}
}

bool AdvancedInput::DidPlayerDoQuarterCircleForward(bool facing_right) {
	if (pastStickInputAngles.size() == 0)
		return false;

	bool did_qcf = false;
	bool ended_in_eligible_spot = false;
	int number_of_eligible_inputs = 1;

	if ((facing_right && abs(pastStickInputAngles[pastStickInputAngles.size() - 1]) < 30.0f) ||
		(!facing_right && abs(pastStickInputAngles[pastStickInputAngles.size() - 1]) > 150.0f)) {
		ended_in_eligible_spot = true;
	}

	if (ended_in_eligible_spot) {
		float last_angle_looked_at = pastStickInputAngles[pastStickInputAngles.size() - 1];

		if (!facing_right && last_angle_looked_at < 0.0f) {
			last_angle_looked_at = 360.0f + last_angle_looked_at;
		}

		for (int i = (int)pastStickInputAngles.size() - 2; i >= 0; i--) {
			if ((facing_right && pastStickInputAngles[i] < last_angle_looked_at) ||
				(!facing_right && (pastStickInputAngles[i] > 0.0f ? pastStickInputAngles[i] : 360.0f + pastStickInputAngles[i]) > last_angle_looked_at)) {
				number_of_eligible_inputs++;
				if (facing_right) {
					last_angle_looked_at = pastStickInputAngles[i];
				} else {
					last_angle_looked_at = pastStickInputAngles[i] > 0.0f ? pastStickInputAngles[i] : 360.0f + pastStickInputAngles[i];
				}
			} else {
				break;
			}
		}

		if (pastStickInputAngles[(int)pastStickInputAngles.size() - number_of_eligible_inputs] < -60.0f &&
			pastStickInputAngles[(int)pastStickInputAngles.size() - number_of_eligible_inputs] > -120.0f)
		{
			did_qcf = true;
		}
	}

	return did_qcf;
}