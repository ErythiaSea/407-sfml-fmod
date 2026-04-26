#pragma once
#include <SFML/Graphics.hpp>

class Point
{
public:
	Point(sf::Vector2f p_position);
	Point();
	virtual ~Point();


	sf::Vector2f getPosition();
	void setPosition(sf::Vector2f new_position);


private:


protected:
	sf::Vector2f position;
};

