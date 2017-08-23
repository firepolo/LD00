#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <time.h>
#include <sdl2/sdl.h>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <al/al.h>
#include <al/alc.h>

#define TOP_VIEW_MODE 1

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

#define FPS 16

#define MAX_KEYS 128

#define CAMERA_SPEED 0.05f
#define CAMERA_ANGLE_SPEED 0.05f
#define CAMERA_VISIBLE_DISTANCE 3

#define HITBOX_SIZE 0.05f

#define ENEMY_SPEED 0.02f
#define ENEMY_DECISIONS_TICKS 60
#define ENEMY_ANIMATION_WALK_FRAMES 2
#define ENEMY_ANIMATION_WALK_TICKS 16

struct Random
{
	template<typename T>
	static inline T GetNumber(T min, T max);
};

struct Pointer
{
	static inline void Delete(void *p);
};

struct Array
{
	static inline void Delete(void **p, GLuint count);
};

struct File
{
	static char *ReadAll(const char *filename, GLuint *size = 0);
};

class Shader
{
public:
	static Shader *WORLD;
	static Shader *Load(GLuint mask, const std::string &filename);
	
	GLuint id;
	~Shader();

	inline void Bind();
	inline void Unbind();

private:
	static GLuint Compile(GLuint id, GLenum type, const std::string &filename);
	
	Shader(GLuint _id);
};

class Model
{
public:
	struct Vertex
	{
		float x, y, z;
		float s, t;
		float _unused;
	};

	static Model *E, *I, *H, *L, *U, *ENEMY;
	static Model *Load(const std::string &filename);

	GLuint vbo, vao, count;
	~Model();

	inline void Bind();
	inline void Unbind();
	
private:
	Model(GLuint _vbo, GLuint _vao, GLuint _count);
};

class Texture
{
public:
	static Texture *GLOBAL;
	static Texture *Load(const std::string &filename);
	
	GLuint id;
	~Texture();

	inline void Bind();
	inline void Unbind();
	
private:
	Texture(GLuint _id);
};

class Sound
{
public:
	static Sound *TEST;
	static Sound *Load(const std::string &filename);
	
	GLuint buf, src;
	~Sound();
	
	inline int GetState();
	inline void SetLooping(bool looping);
	inline void Play();
	inline void Pause();
	inline void Stop();

private:
	Sound(GLuint _buf, GLuint _src);
};

struct Input
{
	static bool *KEYBOARD;
};

struct Point
{
	short x, y;
	Point &Set(short x, short y);
	bool operator==(const Point &o) const;
};

struct Camera
{
	static Camera *INSTANCE;
	
	glm::vec3 position;
	glm::vec3 look;
	float angle;
	
	Camera(glm::vec3 _position, float _angle);
};

struct Enemy
{
	glm::vec3 position;
	glm::vec3 direction;
	GLuint decisionTick, maxDecisionTick;
	GLuint animation, frame, animationTick;
	
	void SetDirection();
	Enemy(glm::vec3 _position);
	void Update();
};

struct Block
{
	Model *model;
	glm::mat4 transform;
	std::vector<Enemy> enemies;
	
	Block(Model *_model, glm::mat4 _transform);
	~Block();
	
	void Draw();
};

struct Map
{
	static Map *INSTANCE;

	Block **blocks;
	Point size;
	Point origin;
	
	Map(Block **blocks, const Point &size, const Point &origin);
	~Map();
	
	inline float GetX(float x);
	inline float GetY(float y);
	inline Block *GetBlock(const glm::vec3 &position);
	
	bool CanMove(glm::vec3 &position, const glm::vec3 &direction);
	void Move(glm::vec3 &position, const glm::vec3 &direction);
	void AddEnemies(GLuint number);
	static Map *Generate(GLuint size);
};

class App
{
public:
	static int Start();
	static int Initialize();

private:
	static SDL_Window *window;
	static SDL_GLContext videoContext;
	static ALCcontext *audioContext;

	static int Shutdown(int exit);
};