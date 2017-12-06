#pragma once

#ifndef DRAWABLE_H
#define DRAWABLE_H

using namespace std;

#include <SFML/Graphics.hpp>
#include "Box2D\Common\b2Draw.h"
#include "..\GameLibrary\Camera.h"
#define PIXEL_SCALE /*15.f*/40.0f

class Drawable : public virtual b2Draw {
protected:
private:
	sf::RenderWindow* render_window;
	Camera* camera;
public:
	Drawable(sf::RenderWindow *window, Camera *cam);

	/// Draw a closed polygon provided in CCW order.
	virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	}

	/// Draw a solid closed polygon provided in CCW order.
	virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
		std::vector<sf::Vertex> drawable_vertices;
		sf::Color vertex_color = sf::Color((sf::Uint8)color.r * (sf::Uint8)255.0f, (sf::Uint8)color.g * (sf::Uint8)255.0f, (sf::Uint8)color.b * (sf::Uint8)255.0f, (sf::Uint8)color.a * (sf::Uint8)255.0f);

		for (int i = 0; i < vertexCount; i++) {
			drawable_vertices.push_back(sf::Vertex(sf::Vector2f((vertices[i].x - camera->viewport_position.x) * PIXEL_SCALE, (vertices[i].y - camera->viewport_position.y) * PIXEL_SCALE), vertex_color, sf::Vector2f(vertices[i].x, vertices[i].y)));
		}
		drawable_vertices.push_back(sf::Vertex(sf::Vector2f((vertices[0].x - camera->viewport_position.x) * PIXEL_SCALE, (vertices[0].y - camera->viewport_position.y) * PIXEL_SCALE), vertex_color, sf::Vector2f(vertices[0].x, vertices[0].y)));

		render_window->draw(&drawable_vertices[0], drawable_vertices.size(), sf::LinesStrip);
	}

	/// Draw a circle.
	virtual void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color) {
		sf::CircleShape circle = sf::CircleShape(radius * 2.0f);
		circle.setPosition(center.x, center.y);
		sf::Color vertex_color = sf::Color((sf::Uint8)color.r * (sf::Uint8)255.0f, (sf::Uint8)color.g * (sf::Uint8)255.0f, (sf::Uint8)color.b * (sf::Uint8)255.0f, (sf::Uint8)color.a * (sf::Uint8)255.0f);

		std::vector<sf::Vertex> drawable_vertices;
		for (int i = 0; i < (int)circle.getPointCount(); i++) {
			drawable_vertices.push_back(sf::Vertex(sf::Vector2f((circle.getPoint(i).x - camera->viewport_position.x) * PIXEL_SCALE, (circle.getPoint(i).y - camera->viewport_position.y) * PIXEL_SCALE), vertex_color, sf::Vector2f(circle.getPoint(i).x, circle.getPoint(i).y)));
		}

		render_window->draw(&drawable_vertices[0], drawable_vertices.size(), sf::LinesStrip);
	}

	/// Draw a solid circle.
	virtual void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color) {
		sf::CircleShape circle = sf::CircleShape(radius);
		circle.setPosition((center.x - camera->viewport_position.x) * PIXEL_SCALE, (center.y - camera->viewport_position.y) * PIXEL_SCALE);
		circle.setOrigin(radius, radius);
		circle.setScale(PIXEL_SCALE, PIXEL_SCALE);
		render_window->draw(circle);
	}

	/// Draw a line segment.
	virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
		//cout << "DrawSegment\n";
	}

	/// Draw a transform. Choose your own length scale.
	/// @param xf a transform.
	virtual void DrawTransform(const b2Transform& xf) {
		//cout << "DrawTransform\n";
	}

	/// Draw a point.
	virtual void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color) {
		//cout << "DrawPoint\n";
	}
};

#endif