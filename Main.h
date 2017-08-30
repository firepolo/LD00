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

#define TOP_VIEW_MODE 0

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

#define FPS 16

#define MAX_KEYS 128

#define PLAYER_SPEED 0.05f
#define PLAYER_ANGLE_SPEED 0.05f
#define PLAYER_VISIBLE_DISTANCE 3
#define PLAYER_ATTACK_TICKS 16
#define PLAYER_LIFES 100

#define HITBOX_SIZE 0.05f
#define HIT_DISTANCE 0.3f

#define ENEMY_SPEED 0.02f
#define ENEMY_DECISIONS_TICKS 60
#define ENEMY_SPEAK_MIN_TICKS 240
#define ENEMY_SPEAK_MAX_TICKS 360
#define ENEMY_ANIMATION_TICKS 16
#define ENEMY_ANIMATION_WALK_FRAMES 2
#define ENEMY_ANIMATION_FALL_FRAMES 3


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

struct Mat4
{
	static const glm::mat4 PROJECTION;
	static const glm::mat4 IDENTITY;
	static const glm::mat4 HAND;
};

class Shader
{
public:
	static Shader *WORLD;
	static Shader *POST;
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

	static Model *E;
	static Model *I;
	static Model *H;
	static Model *L;
	static Model *U;
	static Model *ENEMY;
	static Model *POST;
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

class SoundBuffer
{
public:
	static SoundBuffer *MUSIC;
	static SoundBuffer *HIT;
	static SoundBuffer *CROWBAR;
	static SoundBuffer *ENEMY;
	
	static SoundBuffer *Load(const std::string &filename);
	
	GLuint id;
	~SoundBuffer();
	
private:
	SoundBuffer(GLuint _id);
};

struct Sound
{
	static Sound *MUSIC;
	static Sound *HIT;
	static Sound *CROWBAR;
	
	GLuint id;
	
	Sound(SoundBuffer *buf);
	~Sound();
	
	inline void Attach(SoundBuffer *buf);
	inline int GetState();
	inline void SetLooping(bool looping);
	inline void Play();
	inline void Pause();
	inline void Stop();
	inline void SetVolume(float volume);
};

class FrameBuffer
{
public:
	static FrameBuffer *POST;
	static FrameBuffer *Create(GLuint width, GLuint height);
	
	GLuint fbo;
	GLuint color;
	GLuint depth;
	
	~FrameBuffer();
	
	inline void Bind();
	inline void Unbind();
	inline void BindColor();
	inline void BindDepth();
	
private:
	FrameBuffer(GLuint _fbo, GLuint _tex, GLuint _rbo);
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

struct Player
{
	static Player *INSTANCE;
	
	glm::vec3 position;
	glm::vec3 look;
	float angle;
	float moving;
	
	GLuint frame;
	GLuint attackTicks;
	GLuint life;
	
	Player(glm::vec3 _position, float _angle);
	
	void CheckInput();
	void Draw();
};

struct Enemy
{
	glm::vec3 position;
	glm::vec3 direction;
	GLushort decisionTick, speakTick;
	GLuint animation, frame, animationTick;
	Sound *source;
	
	Enemy(glm::vec3 _position);
	~Enemy();
	
	void SetDirection();
	void PlayFallAnimation();
	void Update();
};

template<GLuint chunk, GLuint ptrSize = sizeof(void *)>
struct EnemyList
{
	Enemy **data;
	GLuint size;
	GLuint capacity;
	
	EnemyList();
	~EnemyList();
	
	void Add(Enemy *enemy);
	void Remove(GLuint index);
};

struct Block
{
	Model *model;
	glm::mat4 transform;
	EnemyList<4> enemies;
	
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
	std::vector<Enemy *> enemies;
	
	Map(Block **blocks, const Point &size, const Point &origin);
	~Map();
	
	inline float GetX(float x);
	inline float GetY(float y);
	inline Block *GetBlock(const glm::vec3 &position);
	
	bool CanMove(glm::vec3 &position, const glm::vec3 &direction);
	void Move(glm::vec3 &position, const glm::vec3 &direction);
	void AddEnemies(GLuint number);
	void Draw();
	
	static Map *Generate(GLuint size);
};

class App
{
public:
	static Point WindowSize;

	static int Start();
	static int Initialize();

private:
	static SDL_Window *window;
	static SDL_GLContext videoContext;
	static ALCcontext *audioContext;

	static int Shutdown(int exit, const char *msg);
};