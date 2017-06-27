#include <stdio.h>
#include <sdl2/sdl.h>
#include <gl/glew.h>

#define WIDTH 800
#define HEIGHT 600

struct Shader
{
	static GLint Attach(GLuint program, GLuint shader)
	{
		if (shader > 0) glAttachShader(program, shader);
	}
	
	static GLint Load(GLenum type, const char *file)
	{
		GLuint shader = glCreateShader(type);
		if (shader == 0) return -1;
		
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
		if (compiled == GL_FALSE) return -2;
		return shader;
	}
	
	static GLint Create(GLuint vertex, GLuint geometry, GLuint fragment)
	{
		GLint program = glCreateProgram();
		if (program == 0) return -1;
		Attach(program, vertex);
		Attach(program, geometry);
		Attach(program, fragment);
		glLinkProgram(program);
		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked);
		return linked == GL_FALSE ? -2 : program;
	}
	
	static void Delete(GLuint program)
	{
		glUseProgram(0);
		glDeleteProgram(program);
	}
};

struct FrameBuffer
{
	static GLint Create()
	{
	}
	
	static void Delete()
	{
	}
};

struct VertexBuffer
{
	static GLint Create(GLsizeiptr size, GLvoid *data, GLenum usage)
	{
		GLuint buffer;
		glGenBuffers(1, &buffer);
		glBindBuffer(GL_ARRAY_BUFFER, buffer);
		glBufferData(GL_ARRAY_BUFFER, size, data, usage);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	static void Delete(GLuint buffer)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &buffer);
	}
};

struct App
{
	static SDL_Window *window;
	static SDL_GLContext gl;
	
	static int Shutdown(int exit)
	{
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
		if (gl == NULL) return Shutdown(3);
		
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