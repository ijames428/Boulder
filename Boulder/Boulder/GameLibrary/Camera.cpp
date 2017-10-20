#pragma once

using namespace std;
#include "stdafx.h"
#include <iostream>
#include "Camera.h"
//#include "..\PlatformerLibrary\Test.h"
#include "..\GameLibrary\GameLibrary.h"

Camera::Camera(sf::Vector2f position, sf::Vector2f dimensions) {
	viewport_position = position;
	viewport_dimensions = dimensions;
}