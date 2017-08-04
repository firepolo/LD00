#include <iostream>
#include <fstream>
#include <string>
#include <sdl2/sdl.h>
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

struct Util
{
	static inline GLuint randUint(GLuint min, GLuint max)
	{
		return min + GLuint(rand() / (float)RAND_MAX * (max - min));
	}
};

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
			vertices[i] = s ? f * 0.25f : f;
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
			}

			shader->Bind();
			texture->Bind();
			
			glm::mat4 uProjection = glm::perspective<float>(M_PI / 180.0f * 70.0f, WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 1000.0f);
			glm::mat4 uView = glm::translate(glm::mat4(1), glm::vec3(0, -0.5f, -5));
			glm::mat4 uModel = glm::rotate(glm::mat4(1), 0.5f, glm::vec3(0.0f, -1, 0));
			
			glUniformMatrix4fv(1, 1, GL_FALSE, (float *)&uView);
			glUniformMatrix4fv(2, 1, GL_FALSE, (float *)&uProjection);
			glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&uModel);
			glUniform1i(3, 0);

			shapes[0]->Bind();

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLES, 0, shapes[0]->count);

			shapes[0]->Unbind();
			texture->Unbind();
			shader->Unbind();

			SDL_GL_SwapWindow(window);
			SDL_Delay(33);
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
		
		shapes = new Model*[1];
		shapes[0] = Model::Load("resources\\models\\I.mol");
		if (!shapes[0]) return Shutdown(6);
		
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
	static Model **shapes;

	static int Shutdown(int exit)
	{
		Array::Delete((void **)shapes, 1);
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
Model **App::shapes = NULL;

SDL_Window *App::window = NULL;
SDL_GLContext App::context = NULL;

int main(int argc, char *argv[])
{
	int err = App::Initialize();
	if (err) return err;
	return App::Start();
}