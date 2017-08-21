#ifndef CAMERA_H
#define CAMERA_H

#define CAMERA_SPEED 0.05f
#define CAMERA_ANGLE_SPEED 0.05f
#define CAMERA_VISIBLE_DISTANCE 3

struct Point
{
	short x, y;
	Point &Set(short x, short y) { this->x = x; this->y = y; return *this; }
	bool operator==(const Point &o) const { return *((int*)this) == *((int*)&o); }
};

struct Camera
{
	static Camera *INSTANCE;
	
	glm::vec3 position;
	glm::vec3 look;
	float angle;
	
	Camera(glm::vec3 _position, float _angle) : position(_position), angle(_angle), look(glm::vec3(glm::cos(angle), 0, glm::sin(angle))) {}
};

Camera *Camera::INSTANCE = NULL;

#endif