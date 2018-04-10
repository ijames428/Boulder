#pragma once

#include "stdafx.h"
#include <iostream>
#include "GridHandler.h"

GridHandler::GridHandler() {
	Grid = std::vector<std::vector<std::vector<RigidBody*>>>(grid_width, std::vector<std::vector<RigidBody*>>(grid_height, std::vector<RigidBody*>()));
}

//void GridHandler::Init() {
//	Grid = std::vector<std::vector<std::vector<RigidBody*>>>(grid_width, std::vector<std::vector<RigidBody*>>(grid_height, std::vector<RigidBody*>()));
//}

void GridHandler::AddRigidBodyToGrid(RigidBody* rb) {
	int topLeftX = (int)rb->GetCurrentPosition().x / cell_width;
	int topLeftY = (int)rb->GetCurrentPosition().y / cell_height;
	int botRightX = (int)(rb->GetCurrentPosition().x + rb->GetCurrentDimensions().x) / cell_width;
	int botRightY = (int)(rb->GetCurrentPosition().y + rb->GetCurrentDimensions().y) / cell_height;

	if (topLeftX > 0) {
		topLeftX--;
	}
	if (topLeftY > 0) {
		topLeftY--;
	}
	if (botRightX < grid_width) {
		botRightX++;
	}
	if (botRightY < grid_height) {
		botRightY++;
	}

	rb->SetGridTopLeftX(topLeftX);
	rb->SetGridTopLeftY(topLeftY);
	rb->SetGridBotRightX(botRightX);
	rb->SetGridBotRightY(botRightY);

	for (int w = topLeftX; w <= botRightX; w++) {
		for (int h = topLeftY; h <= botRightY; h++) {
			if (std::find(Grid[w][h].begin(), Grid[w][h].end(), rb) == Grid[w][h].end()) {
				Grid[w][h].push_back(rb);
			}
		}
	}
}

void GridHandler::MoveRigidBodyInGrid(RigidBody* rb) {
	int topLeftX = (int)rb->GetCurrentPosition().x / cell_width;
	int topLeftY = (int)rb->GetCurrentPosition().y / cell_height;
	int botRightX = (int)(rb->GetCurrentPosition().x + rb->GetCurrentDimensions().x) / cell_width;
	int botRightY = (int)(rb->GetCurrentPosition().y + rb->GetCurrentDimensions().y) / cell_height;

	if (topLeftX < 0 || topLeftY < 0 || botRightX < 0 || botRightY < 0 ||
		topLeftX >= (int)Grid.size() || topLeftY >= (int)Grid.size() || botRightX >= (int)Grid.size() || botRightY >= (int)Grid.size()) {
		return;
	}

	if (topLeftX != rb->GetGridTopLeftX() || topLeftY != rb->GetGridTopLeftY() || botRightX != rb->GetGridBotRightX() || botRightY != rb->GetGridBotRightY()) {
		for (int w = topLeftX; w <= botRightX; w++) {
			for (int h = topLeftY; h <= botRightY; h++) {
				for (int r = 0; r < (int)Grid[w][h].size(); r++) {
					if (Grid[w][h][r]->GetID() == rb->GetID()) {
						Grid[w][h].erase(Grid[w][h].begin() + r);
						break;
					}
				}
			}
		}

		AddRigidBodyToGrid(rb);
	}
}

std::vector<RigidBody*> GridHandler::GetObjectsInGridLocation(int grid_x, int grid_y) {
	return Grid[grid_x][grid_y];
}