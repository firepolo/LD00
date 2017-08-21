#ifndef ENEMY_H
#define ENEMY_H

#include <Util.h>
#include <Map.h>
#include <glm/glm.hpp>

#define ENEMY_SPEED 0.02f

class Enemy
{
public:
	glm::vec3 position;
	glm::vec3 direction;
	GLuint tick, decisionTick;
	
	void SetDirection()
	{
		direction.x = Random::GetNumber<float>(-ENEMY_SPEED, ENEMY_SPEED);
		direction.y = Random::GetNumber<float>(-ENEMY_SPEED, ENEMY_SPEED);
		tick = decisionTick;
	}
	
	Enemy(glm::vec3 _position) : position(_position)
	{
		decisionTick = Random::GetNumber<GLuint>(20, 100);
		SetDirection();
	}
	
	void Update()
	{
		if (--tick == 0) SetDirection();
		
		Map::INSTANCE->Move(position, direction);
	}
};

#endif