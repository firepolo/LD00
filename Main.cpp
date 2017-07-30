#include <stdio.h>
#include <sdl2/sdl.h>
#include <gl/glew.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

class Shader
{
public:
	static bool Compile(GLuint id, const char *vertex, const char *geometry, const char *fragment)
	{
		GLuint doLoad = 0;
		GLuint loaded = 0;
		if (vertex) { ++doLoad; loaded += Load(id, GL_VERTEX_SHADER, vertex); }
		if (geometry) { ++doLoad; loaded += Load(id, GL_GEOMETRY_SHADER, geometry); }
		if (fragment) { ++doLoad; loaded += Load(id, GL_FRAGMENT_SHADER, fragment); }
		if (loaded != doLoad) return false;
		
		glLinkProgram(id);
		GLint linked;
		glGetProgramiv(id, GL_LINK_STATUS, &linked);
		return linked == GL_TRUE;
	}
	
private:
	static GLuint Load(GLuint id, GLenum type, const char *src)
	{
		GLuint shader = glCreateShader(type);
		if (shader == 0) return 0;
		
		glShaderSource(shader, 1, (char **)&src, NULL);
		glCompileShader(shader);
		GLint compiled;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == GL_FALSE) return 0;
		glAttachShader(id, shader);
		return 1;
	}
};

struct Buffer
{
	static void Create(GLuint *id, GLenum target, GLsizeiptr size, GLvoid *data, GLenum usage)
	{
		glGenBuffers(1, id);
		glBindBuffer(target, *id);
		glBufferData(target, size, data, usage);
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
			}
			
			glClear(GL_COLOR_BUFFER_BIT);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			
			SDL_GL_SwapWindow(window);
			SDL_Delay(33);
		}
		
		return Shutdown(100);
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
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
		
		context = SDL_GL_CreateContext(window);
		if (context == NULL || glewInit() != GLEW_OK) return Shutdown(3);
		
		firstShader = glCreateProgram();
		if (!Shader::Compile(firstShader,
						"#version 450 core\nconst vec4 v[4] = vec4[](vec4(-0.5, -0.5, 0, 1), vec4(0.5, -0.5, 0, 1), vec4(-0.5, 0.5, 0, 1), vec4(0.5, 0.5, 0, 1)); void main() { gl_Position = v[gl_VertexID]; }",
						NULL,
						"#version 450 core\nlayout(location=0) out vec4 oColor; void main() { oColor = vec4(1); }")) return Shutdown(4);
		
		glGenTextures(1, &renderTexture);
		glBindTexture(GL_TEXTURE_2D, renderTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, BUFFER_WIDTH, BUFFER_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
		
		glGenRenderbuffers(1, &renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, BUFFER_WIDTH, BUFFER_HEIGHT);
		
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		Buffer::Create(&firstVertexBuffer, GL_ARRAY_BUFFER, 4, NULL, GL_DYNAMIC_COPY);
		
		glGenVertexArrays(1, &firstVertexArray);
		glBindVertexArray(firstVertexArray);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
			glEnableVertexAttribArray(0);
		glBindVertexArray(0);
		
		glUseProgram(firstShader);
		glBindVertexArray(firstVertexArray);
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClearColor(0.1, 0.5, 0.8, 1);
		
		return 0;
	}

private:
	static SDL_Window *window;
	static SDL_GLContext context;
	
	static GLuint firstShader;
	static GLuint renderTexture;
	static GLuint renderBuffer;
	static GLuint frameBuffer;
	static GLuint firstVertexBuffer;
	static GLuint firstVertexArray;
	
	static int Shutdown(int exit)
	{
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &firstVertexBuffer);
		
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &firstVertexArray);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &frameBuffer);
		
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
		glDeleteRenderbuffers(1, &renderBuffer);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glDeleteTextures(1, &renderTexture);
		
		glUseProgram(0);
		glDeleteProgram(firstShader);
		
		if (context) SDL_GL_DeleteContext(context);
		if (window) SDL_DestroyWindow(window);
		SDL_Quit();
		
		return exit;
	}
};

GLuint App::firstShader = 0;
GLuint App::renderTexture = 0;
GLuint App::renderBuffer = 0;
GLuint App::frameBuffer = 0;
GLuint App::firstVertexBuffer = 0;
GLuint App::firstVertexArray = 0;

SDL_Window *App::window = NULL;
SDL_GLContext App::context = NULL;

int main(int argc, char *argv[])
{
	int err = App::Initialize();
	if (err) return err;
	return App::Start();
}