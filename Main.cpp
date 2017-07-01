#include <stdio.h>
#include <sdl2/sdl.h>
#include <gl/glew.h>

#define WIDTH 800
#define HEIGHT 600

class Shader
{
public:
	static GLuint id;
	
	static bool Set(const char *vertex, const char *geometry, const char *fragment)
	{
		GLuint loaded = 0;
		if (vertex) loaded += Load(GL_VERTEX_SHADER, vertex);
		if (geometry) loaded += Load(GL_GEOMETRY_SHADER, geometry);
		if (fragment) loaded += Load(GL_FRAGMENT_SHADER, fragment);
		if (loaded == 0) return false;
		
		if (id > 0) Delete();
		id = glCreateProgram();
		if (id == 0) return false;
		glLinkProgram(id);
		GLint linked;
		glGetProgramiv(id, GL_LINK_STATUS, &linked);
		return linked == GL_TRUE;
	}
	
	static void Delete()
	{
		if (id > 0) glDeleteProgram(id);
	}
	
private:
	static bool Load(GLenum type, const char *file)
	{
		GLuint shader = glCreateShader(type);
		if (shader == 0) return false;
		
		FILE *in = fopen(file, "rb");
		fseek(in, 0, SEEK_END);
		long int size = ftell(in);
		fseek(in, 0, SEEK_SET);
		
		char *src = new char[size + 1];
		fread(src, 1, size, in);
		src[size] = 0;
		fclose(in);
		
		glShaderSource(shader, 1, (char **)src, NULL);
		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == GL_FALSE) return false;
		glAttachShader(id, shader);
		return true;
	}
};

GLuint Shader::id = 0;

class PostProcess
{
public:
	static GLuint texture;
	static GLuint frameBuffer;
	static GLuint renderBuffer;
	
	static bool Set()
	{
		printf("-1\n");
		if (texture > 0) Delete();
		
		printf("-2\n");
		glGenTextures(1, &texture);
		if (texture == 0) return false;
		
		printf("-3\n");
		glGenRenderbuffers(1, &renderBuffer);
		if (renderBuffer == 0) return false;
		
		glGenFramebuffers(1, &frameBuffer);
		if (frameBuffer == 0) return false;
		
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, HEIGHT);
		
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
		return true;
	}
	
	static void Delete()
	{
		if (renderBuffer > 0) glDeleteRenderbuffers(1, &renderBuffer);
		if (texture > 0) glDeleteTextures(1, &texture);
		if (frameBuffer > 0) glDeleteFramebuffers(1, &frameBuffer);
	}
};

GLuint PostProcess::texture = 0;
GLuint PostProcess::frameBuffer = 0;
GLuint PostProcess::renderBuffer = 0;

class VertexBuffer
{
public:
	static GLuint id;

	static bool Set(GLsizeiptr size, GLvoid *data, GLenum usage)
	{
		if (data == NULL) return false;
		if (id > 0) Delete();
		
		glGenBuffers(1, &id);
		if (id == 0) return false;
		
		glBindBuffer(GL_ARRAY_BUFFER, id);
		glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return true;
	}
	
	static void Delete()
	{
		if (id > 0) glDeleteBuffers(1, &id);
	}
};

GLuint VertexBuffer::id = 0;

struct App
{
	static SDL_Window *window;
	static SDL_GLContext gl;
	
	static int Shutdown(int exit)
	{
		VertexBuffer::Delete();
		PostProcess::Delete();
		Shader::Delete();
		
		if (gl) SDL_GL_DeleteContext(gl);
		if (window) SDL_DestroyWindow(window);
		SDL_Quit();
		
		return exit;
	}
	
	static int Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO)) return Shutdown(1);
		
		window = SDL_CreateWindow("3D game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
		if (window == NULL) return Shutdown(2);
		
		gl = SDL_GL_CreateContext(window);
		if (gl == NULL || glewInit() != GLEW_OK) return Shutdown(3);
		
		if (!Shader::Set(NULL, NULL, NULL)) return Shutdown(4);
		if (!PostProcess::Set()) return Shutdown(5);
		if (!VertexBuffer::Set(0, NULL, 0)) return Shutdown(6);
		
		printf("Initialized\n");
		
		return 0;
	}
	
	static int Start()
	{
		SDL_Event event;
		
		for (;;)
		{
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT) return Shutdown(0);
			}
			
			glClearColor(1, 1, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			SDL_GL_SwapWindow(window);
			SDL_Delay(33);
		}
		
		return Shutdown(100);
	}
};

SDL_Window *App::window = NULL;
SDL_GLContext App::gl = NULL;

int main(int argc, char *argv[])
{
	int err = App::Initialize();
	if (err) return err;
	return App::Start();
}