#pragma once
#include "Point.h"

class ColliderShape : public Point
{
public:
	ColliderShape(sf::Vector2f p_centre, sf::Vector2f p_size);

	ColliderShape(sf::Vector2f p_centre, std::array<sf::Vector2f, 4> p_corners);

	ColliderShape();

	~ColliderShape();


	void update(sf::Transform p_transform_to_apply);

	virtual void setCentre(sf::Vector2f p_new_centre);
	virtual void setSize(sf::Vector2f p_new_size);


	std::array<sf::Vector2f, 4> getTransformedCorners();
	std::array<sf::Vector2f, 4> getLocalCorners();

	std::array<sf::Vector2f, 4> getEdges();

	// only accurate for no rotation! fine for this game (PPP).
	sf::FloatRect getFloatRect();


private:
	void setCorners(); 

	bool custom = false; //stores if the collider uses a default rectangle collsision shape or a fully custom setup where each corner has a defined position

	std::array<sf::Vector2f, 4> corners;
	std::array<sf::Vector2f, 4> transformed_corners;
	sf::Vector2f centre;
	sf::Vector2f size;
};

