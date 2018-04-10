#pragma once

#include "stdafx.h"
#include <iostream>
#include "Drawable.h"

Drawable::Drawable(sf::RenderWindow *window, Camera *cam) {
	render_window = window;
	camera = cam;
}