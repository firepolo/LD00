#ifndef MAP_H
#define MAP_H

#include <Resources.h>
#include <Enemy.h>
#include <Camera.h>

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#define HITBOX_SIZE 0.05f

class Block
{
public:
	Model *model;
	glm::mat4 transform;
	std::vector<Enemy> enemies;
	
	Block(Model *_model, glm::mat4 _transform) : model(_model), transform(_transform) {}
	~Block() {}
	
	void Draw()
	{
		model->Bind();
		glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&transform);
		glDrawArrays(GL_TRIANGLES, 0, model->count);
		model->Unbind();
		
		for (std::vector<Enemy>::iterator it = enemies.begin(), end = enemies.end(); it != end; ++it)
		{
			it->Update();
			
			Model::ENEMY->Bind();
			glm::mat4 uModel = glm::rotate(glm::translate(glm::mat4(1), it->position), (float)atan2(Camera::INSTANCE->position.x - it->position.x, Camera::INSTANCE->position.z - it->position.z), glm::vec3(0, 1, 0));
			glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&uModel);
			glDrawArrays(GL_TRIANGLES, 0, Model::ENEMY->count);
			Model::ENEMY->Unbind();
		}
	}
};

class Map
{
public:
	static Map *INSTANCE;

	Block **blocks;
	Point size;
	Point origin;
	
	Map(Block **blocks, const Point &size, const Point &origin)
	{
		this->blocks = blocks;
		this->size = size;
		this->origin = origin;
	}
	
	~Map()
	{
		Array::Delete((void **)blocks, size.x * size.y);
	}
	
	bool CanMove(glm::vec3 &position, const glm::vec3 &direction)
	{
		float x = position.x + direction.x;
		float z = position.z + direction.z;
		float hx = x - origin.x + 0.5f + (direction.x < 0 ? -HITBOX_SIZE : HITBOX_SIZE);
		float hz = z - origin.y + 0.5f + (direction.z < 0 ? -HITBOX_SIZE : HITBOX_SIZE);
		
		if (hx < 0 || hx >= size.x || hz < 0 || hz >= size.y) return false;
		if (blocks[(int)hz * size.x + (int)hx] == NULL) return false;
		
		position.x = x;
		position.z = z;
		return true;
	}
	
	void Move(glm::vec3 &position, const glm::vec3 &direction)
	{
		if (CanMove(position, direction)) return;
		if (CanMove(position, glm::vec3(0, 0, direction.z))) return;
		if (CanMove(position, glm::vec3(direction.x, 0, 0))) return;
	}
	
	void AddEnemies(GLuint number)
	{
		while (number)
		{
			/*float x = float(rand() / (float)RAND_MAX * size.x);
			float z = float(rand() / (float)RAND_MAX * size.y);*/
			float x = Random::GetNumber<float>(0, size.x);
			float z = Random::GetNumber<float>(0, size.y);
			GLuint i = (int)z * size.x + (int)x;
			if (blocks[i])
			{
				blocks[i]->enemies.push_back(Enemy(glm::vec3(x + origin.x - 0.5, 0, z + origin.y - 0.5)));
				--number;
			}
		}
	}
	
	static Map *Generate(GLuint size)
	{
		std::vector<Point> points;
		const Point dirs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
		
		Point p = { 0 };
		points.push_back({0});
		
		short l = INT_MAX, r = INT_MIN, t = INT_MAX, b = INT_MIN;
		
		srand(time(0));
		
		while (points.size() < size)
		{
			Point d = dirs[Random::GetNumber<GLuint>(0, 4)];
			Point n = { p.x + d.x, p.y + d.y };
			if (std::find(points.begin(), points.end(), n) == points.end())
			{
				if (n.x < l) l = n.x;
				else if (n.x > r) r = n.x;
				if (n.y < t) t = n.y;
				else if (n.y > b) b = n.y;
				points.push_back(n);
			}
			*((int *)&p) = *((int *)&n);
		}
		
		short w = glm::abs(l) + glm::abs(r) + 1;
		short h = glm::abs(t) + glm::abs(b) + 1;
		size = w * h;
		Block **blocks = new Block*[size];
		memset(blocks, 0, size * sizeof(void *));
		
		for (std::vector<Point>::iterator beg = points.begin(), end = points.end(), it = beg; it != end; ++it)
		{
			// Find neighbors left, top, right, bottom
			GLuint c = std::find(beg, end, p.Set(it->x - 1, it->y)) != end;
			c |= (std::find(beg, end, p.Set(it->x, it->y - 1)) != end) << 1;
			c |= (std::find(beg, end, p.Set(it->x + 1, it->y)) != end) << 2;
			c |= (std::find(beg, end, p.Set(it->x, it->y + 1)) != end) << 3;
			
			GLuint i = (it->y - t) * w + (it->x - l);
			
			// E
			if (c == 0b1111) blocks[i] = new Block(Model::E, glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			// I
			else if (c == 0b1110) blocks[i] = new Block(Model::I, glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b1101) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b1011) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0111) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// H
			else if (c == 0b1010) blocks[i] = new Block(Model::H, glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b0101) blocks[i] = new Block(Model::H, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// L
			else if (c == 0b1100) blocks[i] = new Block(Model::L, glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b1001) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b0011) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0110) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// U
			else if (c == 0b1000) blocks[i] = new Block(Model::U, glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b0001) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b0010) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0100) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
		}
		
		return new Map(blocks, { w, h }, { l, t });
	}
};

Map *Map::INSTANCE = NULL;

#endif