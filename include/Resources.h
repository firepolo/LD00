#ifndef RESOURCES_H
#define RESOURCES_H

#include <fstream>
#include <sdl2/sdl.h>
#include <gl/glew.h>
#include <al/al.h>
#include <al/alc.h>

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
	static Shader *WORLD;

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

Shader *Shader::WORLD = NULL;

class Model
{
public:
	static Model *E, *I, *H, *L, *U, *ENEMY;

	struct Vertex
	{
		float x, y, z;
		float s, t;
		float _unused;
	};

	GLuint vbo, vao, count;
	
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

Model *Model::E = NULL, *Model::I = NULL, *Model::H = NULL, *Model::L = NULL, *Model::U = NULL, *Model::ENEMY = NULL;

class Texture
{
public:
	static Texture *GLOBAL;

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

Texture *Texture::GLOBAL = NULL;

class Sound
{
public:
	static Sound *TEST;

	GLuint buf, src;
	
	static Sound *Load(const std::string &filename)
	{
		char *data = File::ReadAll(filename.c_str());
		if (*(short *)(data + 20) != 1 ||	// Audio format PCM
			*(short *)(data + 22) != 1 ||	// Number channels 1
			*(GLuint *)(data + 24) != 44100 ||	// Sample rate 44100
			*(short *)(data + 34) != 16)	// Bits per sample 16
			return NULL;
		
		GLuint buf;
		alGenBuffers(1, &buf);
		alBufferData(buf, AL_FORMAT_MONO16, data + 44, *(GLuint *)(data + 40), 44100);
		delete[] data;
		
		GLuint src;
		alGenSources(1, &src);
		alSourcei(src, AL_BUFFER, buf);
		
		return new Sound(buf, src);
	}
	
	~Sound() { alDeleteSources(1, &src); alDeleteBuffers(1, &buf); }
	
	inline int GetState() { int s; alGetSourcei(src, AL_SOURCE_STATE, &s); return s; }
	inline void SetLooping(bool looping) { alSourcei(src, AL_LOOPING, looping); }
	inline void Play() { if (GetState() != AL_PLAYING) alSourcePlay(src); }
	inline void Pause() { if (GetState() == AL_PLAYING) alSourcePause(src); }
	inline void Stop() { if (GetState() != AL_STOPPED) alSourceStop(src); }

private:
	Sound(GLuint _buf, GLuint _src) : buf(_buf), src(_src) {}
};

Sound *Sound::TEST = NULL;

#endif