#pragma once

#include "stdafx.h"
#include "Light.h"  

Light::Light(sf::Vector2f pos, float r, float g, float b) {
	position = pos;
	red = r;
	green = g;
	blue = b;
}