#pragma once

#include "stdafx.h"
#include <iostream>
#include "Drawable.h"

Drawable::Drawable(sf::RenderWindow *window) {
	render_window = window;
}

void Drawable::SetShape(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
};