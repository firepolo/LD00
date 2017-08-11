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

#define TOP_VIEW_MODE 0

struct Pointer
{
	static inline void Delete(void *p)
	{
		if (!p) return;
		delete p;
		p = NULL;
	}
};

struct Array
{
	static inline void Delete(void **p, GLuint count)
	{
		if (!p) return;
		while (count) Pointer::Delete(p[--count]);
		delete[] p;
		p = NULL;
	}
};

struct File
{
	static char *ReadAll(const char *filename, GLuint *size = 0)
	{
		std::ifstream is(filename, std::ifstream::binary);
		if (!is.is_open()) return NULL;
		is.seekg(0, is.end);
		GLuint s = is.tellg();
		is.seekg(0, is.beg);
		char *p = new char[s + 1];
		is.read(p, s);
		is.close();
		p[s] = '\0';
		if (size) *size = s;
		return p;
	}
};

struct Point
{
	short x, y;
	Point &Set(short x, short y) { this->x = x; this->y = y; return *this; }
	bool operator==(const Point &o) const { return *((int*)this) == *((int*)&o); }
};

struct Camera
{
	glm::vec3 position;
	glm::vec3 look;
	float angle;
	
	Camera(glm::vec3 _position, float _angle) : position(_position), angle(_angle), look(glm::vec3(glm::cos(angle), 0, glm::sin(angle))) {}
};

class Shader
{
public:
	GLuint id;

	static Shader *Load(GLuint mask, const std::string &filename)
	{
		GLuint id = glCreateProgram();
		GLuint doCompile = 0;
		GLuint compiled = 0;
		if (mask & 1) { ++doCompile; compiled += Compile(id, GL_VERTEX_SHADER, filename + ".vs"); }
		if (mask & 2) { ++doCompile; compiled += Compile(id, GL_GEOMETRY_SHADER, filename + ".gs"); }
		if (mask & 4) { ++doCompile; compiled += Compile(id, GL_FRAGMENT_SHADER, filename + ".fs"); }
		if (compiled != doCompile) return NULL;

		glLinkProgram(id);
		GLint linked;
		glGetProgramiv(id, GL_LINK_STATUS, &linked);
		return linked == GL_TRUE ? new Shader(id) : NULL;
	}
	
	~Shader() { glDeleteProgram(id); }

	inline void Bind() { glUseProgram(id); }
	inline void Unbind() { glUseProgram(0); }

private:
	static GLuint Compile(GLuint id, GLenum type, const std::string &filename)
	{
		GLuint shader = glCreateShader(type);
		if (shader == 0) return 0;
		
		char *src = File::ReadAll(filename.c_str());
		if (!src) return 0;
		
		glShaderSource(shader, 1, (char **)&src, NULL);
		delete[] src;

		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == GL_FALSE) return 0;
		glAttachShader(id, shader);
		return 1;
	}
	
	Shader(GLuint _id) : id(_id) {}
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

	GLuint vbo;
	GLuint vao;
	GLuint count;
	
	static Model *Load(const std::string &filename)
	{
		GLuint size = 0;
		char *data = File::ReadAll(filename.c_str(), &size);
		if (!data || size < 18 || (size % 18)) return NULL;
		
		float *vertices = new float[size];
		char s = -1;
		for (GLuint i = 0; i < size; ++i)
		{
			s = i % 3 ? s : !s;
			float f = data[i] - '0';
			vertices[i] = s ? f * 0.25f : f * 0.5f;
		}
		delete[] data;
		
		GLuint vbo;
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, size * sizeof(float), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		delete[] vertices;
		
		GLuint vao;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)&((Vertex *)0)->s);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
		return new Model(vbo, vao, size / 6);
	}
	
	~Model()
	{
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(0);
		glDeleteVertexArrays(1, &vao);
		glDeleteBuffers(1, &vbo);
	}

	inline void Bind() { glBindVertexArray(vao); }
	inline void Unbind() { glBindVertexArray(0); }
	
private:
	Model(GLuint _vbo, GLuint _vao, GLuint _count) : vbo(_vbo), vao(_vao), count(_count) {}
};

class Texture
{
public:
	GLuint id;

	static Texture *Load(const std::string &filename)
	{
		SDL_Surface *bmp = SDL_LoadBMP_RW(SDL_RWFromFile(filename.c_str(), "rb"), 1);
		if (!bmp) return NULL;

		glActiveTexture(0);

		GLuint id;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bmp->w, bmp->h, 0, GL_BGR, GL_UNSIGNED_BYTE, bmp->pixels);
		glBindTexture(GL_TEXTURE_2D, 0);
		SDL_FreeSurface(bmp);

		return new Texture(id);
	}
	
	~Texture() { glDeleteTextures(1, &id); }

	inline void Bind() { glBindTexture(GL_TEXTURE_2D, id); }
	inline void Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }
	
private:
	Texture(GLuint _id) : id(_id) {}
};

class Block
{
public:
	Model *model;
	glm::mat4 transform;
	
	Block(Model *_model, glm::mat4 _transform) : model(_model), transform(_transform) {}
};

class Map
{
public:
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
	
	static Map *Generate(GLuint size, Model **models)
	{
		std::vector<Point> points;
		const Point dirs[] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };
		
		Point p = { 0 };
		points.push_back({0});
		
		short l = INT_MAX, r = INT_MIN, t = INT_MAX, b = INT_MIN;
		
		srand(time(0));
		
		while (points.size() < size)
		{
			Point d = dirs[int(rand() / (float)RAND_MAX * 4)];
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
			if (c == 0b1111) blocks[i] = new Block(models[0], glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			// I
			else if (c == 0b1110) blocks[i] = new Block(models[1], glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b1101) blocks[i] = new Block(models[1], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b1011) blocks[i] = new Block(models[1], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0111) blocks[i] = new Block(models[1], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// H
			else if (c == 0b1010) blocks[i] = new Block(models[2], glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b0101) blocks[i] = new Block(models[2], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// L
			else if (c == 0b1100) blocks[i] = new Block(models[3], glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b1001) blocks[i] = new Block(models[3], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b0011) blocks[i] = new Block(models[3], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0110) blocks[i] = new Block(models[3], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
			// U
			else if (c == 0b1000) blocks[i] = new Block(models[4], glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)));
			else if (c == 0b0001) blocks[i] = new Block(models[4], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
			else if (c == 0b0010) blocks[i] = new Block(models[4], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
			else if (c == 0b0100) blocks[i] = new Block(models[4], glm::rotate<float>(glm::translate(glm::mat4(1), glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
		}
		
		return new Map(blocks, { w, h }, { l, t });
	}
};

class App
{
public:
	static int Start()
	{
		SDL_Event event;

		for (;;)
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT) return Shutdown(0);
				
				if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				{
					if (event.key.keysym.scancode < MAX_KEYS) keys[event.key.keysym.scancode] = event.type == SDL_KEYDOWN;
					continue;
				}
			}
			
			// INPUT CHECKING
			if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP]) map->Move(camera->position, camera->look * CAMERA_SPEED);
			else if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN]) map->Move(camera->position, camera->look * -CAMERA_SPEED);
			
			if (keys[SDL_SCANCODE_A]) map->Move(camera->position, glm::vec3(camera->look.z * CAMERA_SPEED, 0, camera->look.x * -CAMERA_SPEED));
			else if (keys[SDL_SCANCODE_D]) map->Move(camera->position, glm::vec3(camera->look.z * -CAMERA_SPEED, 0, camera->look.x * CAMERA_SPEED));
			
			if (keys[SDL_SCANCODE_LEFT])
			{
				camera->angle -= CAMERA_ANGLE_SPEED;
				camera->look.x = glm::cos(camera->angle);
				camera->look.z = glm::sin(camera->angle);
			}
			else if (keys[SDL_SCANCODE_RIGHT])
			{
				camera->angle += CAMERA_ANGLE_SPEED;
				camera->look.x = glm::cos(camera->angle);
				camera->look.z = glm::sin(camera->angle);
			}
			
			// RENDER
			shader->Bind();
			texture->Bind();
			
			glm::mat4 uProjection = glm::perspective<float>(M_PI / 180.0f * 70.0f, WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
			
		#if TOP_VIEW_MODE==1
			glm::mat4 uView = glm::lookAt(camera->position, glm::vec3(camera->position.x + camera->look.x, 2, camera->position.z + camera->look.z), glm::vec3(0, 1, 0));
		#else
			glm::mat4 uView = glm::lookAt(camera->position, camera->position + camera->look, glm::vec3(0, 1, 0));
		#endif
			
			glUniformMatrix4fv(1, 1, GL_FALSE, (float *)&uView);
			glUniformMatrix4fv(2, 1, GL_FALSE, (float *)&uProjection);
			glUniform1i(3, 0);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			for (int z = camera->position.z - map->origin.y + 0.5f - CAMERA_VISIBLE_DISTANCE, ez = z + (CAMERA_VISIBLE_DISTANCE << 1); z <= ez; ++z)
			{
				if (z < 0 || z >= map->size.y) continue;
				
				for (int x = camera->position.x - map->origin.x + 0.5f - CAMERA_VISIBLE_DISTANCE, ex = x + (CAMERA_VISIBLE_DISTANCE << 1); x <= ex; ++x)
				{
					if (x < 0 || x >= map->size.x) continue;
					
					Block *b = map->blocks[z * map->size.x + x];
					if (b == NULL) continue;
					
					b->model->Bind();
					glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&b->transform);
					glDrawArrays(GL_TRIANGLES, 0, b->model->count);
					b->model->Unbind();
				}
			}
			
			texture->Unbind();
			shader->Unbind();

			SDL_GL_SwapWindow(window);
			SDL_Delay(FPS);
		}

		return Shutdown(0);
	}

	static int Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO)) return Shutdown(1);

		window = SDL_CreateWindow("3D game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
		if (window == NULL) return Shutdown(2);

		SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 6);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		context = SDL_GL_CreateContext(window);
		if (context == NULL || glewInit() != GLEW_OK) return Shutdown(3);

		shader = Shader::Load(0b101, "resources\\shaders\\basic");
		if (!shader) return Shutdown(4);

		texture = Texture::Load("resources\\textures\\mega.bmp");
		if (!texture) return Shutdown(5);
		
		models = new Model*[5];
		models[0] = Model::Load("resources\\models\\E.mol");
		if (!models[0]) return Shutdown(6);
		models[1] = Model::Load("resources\\models\\I.mol");
		if (!models[1]) return Shutdown(7);
		models[2] = Model::Load("resources\\models\\H.mol");
		if (!models[2]) return Shutdown(8);
		models[3] = Model::Load("resources\\models\\L.mol");
		if (!models[3]) return Shutdown(9);
		models[4] = Model::Load("resources\\models\\U.mol");
		if (!models[4]) return Shutdown(10);
		
		map = Map::Generate(64, models);
		
		keys = new bool[MAX_KEYS];
		memset(keys, 0, MAX_KEYS);
		
		#if TOP_VIEW_MODE==1
			camera = new Camera(glm::vec3(0, 5, 0), 0);
		#else
			camera = new Camera(glm::vec3(0, 0, 0), 0);
		#endif
		
		
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glClearColor(0.1, 0.5, 0.8, 1);
		
		return 0;
	}

private:
	static SDL_Window *window;
	static SDL_GLContext context;
	
	static Shader *shader;
	static Texture *texture;
	static Model **models;
	static Map *map;
	static bool *keys;
	static Camera *camera;

	static int Shutdown(int exit)
	{
		Array::Delete((void **)models, 5);
		Pointer::Delete(texture);
		Pointer::Delete(shader);

		if (context) SDL_GL_DeleteContext(context);
		if (window) SDL_DestroyWindow(window);
		SDL_Quit();

		return exit;
	}
};

Shader *App::shader = NULL;
Texture *App::texture = NULL;
Model **App::models = NULL;
Map *App::map = NULL;
bool *App::keys = NULL;
Camera *App::camera = NULL;

SDL_Window *App::window = NULL;
SDL_GLContext App::context = NULL;

int main(int argc, char *argv[])
{
	int err = App::Initialize();
	if (err) return err;
	return App::Start();
}