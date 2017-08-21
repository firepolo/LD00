#ifndef APP_H
#define APP_H

#include <Input.h>
#include <Map.h>

#define TOP_VIEW_MODE 1

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 240

#define BUFFER_WIDTH 320
#define BUFFER_HEIGHT 240

#define FPS 16

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
					if (event.key.keysym.scancode < MAX_KEYS) Input::KEYBOARD[event.key.keysym.scancode] = event.type == SDL_KEYDOWN;
					continue;
				}
			}
			
			// INPUT CHECKING
			if (Input::KEYBOARD[SDL_SCANCODE_W] || Input::KEYBOARD[SDL_SCANCODE_UP]) Map::INSTANCE->Move(Camera::INSTANCE->position, Camera::INSTANCE->look * CAMERA_SPEED);
			else if (Input::KEYBOARD[SDL_SCANCODE_S] || Input::KEYBOARD[SDL_SCANCODE_DOWN]) Map::INSTANCE->Move(Camera::INSTANCE->position, Camera::INSTANCE->look * -CAMERA_SPEED);
			
			if (Input::KEYBOARD[SDL_SCANCODE_A]) Map::INSTANCE->Move(Camera::INSTANCE->position, glm::vec3(Camera::INSTANCE->look.z * CAMERA_SPEED, 0, Camera::INSTANCE->look.x * -CAMERA_SPEED));
			else if (Input::KEYBOARD[SDL_SCANCODE_D]) Map::INSTANCE->Move(Camera::INSTANCE->position, glm::vec3(Camera::INSTANCE->look.z * -CAMERA_SPEED, 0, Camera::INSTANCE->look.x * CAMERA_SPEED));
			
			if (Input::KEYBOARD[SDL_SCANCODE_LEFT])
			{
				Camera::INSTANCE->angle -= CAMERA_ANGLE_SPEED;
				Camera::INSTANCE->look.x = glm::cos(Camera::INSTANCE->angle);
				Camera::INSTANCE->look.z = glm::sin(Camera::INSTANCE->angle);
			}
			else if (Input::KEYBOARD[SDL_SCANCODE_RIGHT])
			{
				Camera::INSTANCE->angle += CAMERA_ANGLE_SPEED;
				Camera::INSTANCE->look.x = glm::cos(Camera::INSTANCE->angle);
				Camera::INSTANCE->look.z = glm::sin(Camera::INSTANCE->angle);
			}
			
			if (Input::KEYBOARD[SDL_SCANCODE_SPACE]) Sound::TEST->Play();
			
			// RENDER
			Shader::WORLD->Bind();
			Texture::GLOBAL->Bind();
			
			glm::mat4 uProjection = glm::perspective<float>(M_PI / 180.0f * 70.0f, WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
			
		#if TOP_VIEW_MODE==1
			glm::mat4 uView = glm::lookAt(Camera::INSTANCE->position, glm::vec3(Camera::INSTANCE->position.x + Camera::INSTANCE->look.x, 2, Camera::INSTANCE->position.z + Camera::INSTANCE->look.z), glm::vec3(0, 1, 0));
		#else
			glm::mat4 uView = glm::lookAt(Camera::INSTANCE->position, Camera::INSTANCE->position + Camera::INSTANCE->look, glm::vec3(0, 1, 0));
		#endif
			
			glUniformMatrix4fv(1, 1, GL_FALSE, (float *)&uView);
			glUniformMatrix4fv(2, 1, GL_FALSE, (float *)&uProjection);
			glUniform1i(3, 0);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			for (int z = Camera::INSTANCE->position.z - Map::INSTANCE->origin.y + 0.5f - CAMERA_VISIBLE_DISTANCE, ez = z + (CAMERA_VISIBLE_DISTANCE << 1); z <= ez; ++z)
			{
				if (z < 0 || z >= Map::INSTANCE->size.y) continue;
				
				for (int x = Camera::INSTANCE->position.x - Map::INSTANCE->origin.x + 0.5f - CAMERA_VISIBLE_DISTANCE, ex = x + (CAMERA_VISIBLE_DISTANCE << 1); x <= ex; ++x)
				{
					if (x < 0 || x >= Map::INSTANCE->size.x) continue;
					
					Block *b = Map::INSTANCE->blocks[z * Map::INSTANCE->size.x + x];
					if (b != NULL) b->Draw();
				}
			}
			
			Texture::GLOBAL->Unbind();
			Shader::WORLD->Unbind();

			SDL_GL_SwapWindow(window);
			SDL_Delay(FPS);
		}

		return Shutdown(0);
	}

	static int Initialize()
	{
		if (SDL_Init(SDL_INIT_VIDEO)) return Shutdown(1);

		window = SDL_CreateWindow("3D game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
		if (!window) return Shutdown(2);

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

		videoContext = SDL_GL_CreateContext(window);
		if (!videoContext || glewInit() != GLEW_OK) return Shutdown(3);
		
		audioContext = alcCreateContext(alcOpenDevice(NULL), NULL);
		if (!audioContext) return Shutdown(5);
		alcMakeContextCurrent(audioContext);

		Shader::WORLD = Shader::Load(0b101, "resources\\shaders\\world");
		if (!Shader::WORLD) return Shutdown(10);

		Texture::GLOBAL = Texture::Load("resources\\textures\\mega.bmp");
		if (!Texture::GLOBAL) return Shutdown(11);
		
		Sound::TEST = Sound::Load("resources\\sounds\\shoot.wav");
		if (!Sound::TEST) return Shutdown(17);
		
		Model::E = Model::Load("resources\\models\\E.mol");
		if (!Model::E) return Shutdown(12);
		Model::I = Model::Load("resources\\models\\I.mol");
		if (!Model::I) return Shutdown(13);
		Model::H = Model::Load("resources\\models\\H.mol");
		if (!Model::H) return Shutdown(14);
		Model::L = Model::Load("resources\\models\\L.mol");
		if (!Model::L) return Shutdown(15);
		Model::U = Model::Load("resources\\models\\U.mol");
		if (!Model::U) return Shutdown(16);
		Model::ENEMY = Model::Load("resources\\models\\enemy.mol");
		if (!Model::ENEMY) return Shutdown(17);
		
		Map::INSTANCE = Map::Generate(64);
		Map::INSTANCE->AddEnemies(32);
		
		Input::KEYBOARD = new bool[MAX_KEYS];
		memset(Input::KEYBOARD, 0, MAX_KEYS);
		
		#if TOP_VIEW_MODE==1
			Camera::INSTANCE = new Camera(glm::vec3(0, 5, 0), 0);
		#else
			Camera::INSTANCE = new Camera(glm::vec3(0, 0, 0), 0);
		#endif
		
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glClearColor(0.1, 0.5, 0.8, 1);
		
		return 0;
	}

private:
	static SDL_Window *window;
	static SDL_GLContext videoContext;
	static ALCcontext *audioContext;

	static int Shutdown(int exit)
	{
		Pointer::Delete(Camera::INSTANCE);
		Pointer::Delete(Input::KEYBOARD);
		Pointer::Delete(Map::INSTANCE);
		
		Pointer::Delete(Model::ENEMY);
		Pointer::Delete(Model::U);
		Pointer::Delete(Model::L);
		Pointer::Delete(Model::H);
		Pointer::Delete(Model::I);
		Pointer::Delete(Model::E);
		
		Pointer::Delete(Sound::TEST);
		Pointer::Delete(Texture::GLOBAL);
		Pointer::Delete(Shader::WORLD);
		
		if (audioContext)
		{
			ALCdevice *device = alcGetContextsDevice(audioContext);
			alcMakeContextCurrent(NULL);
			alcDestroyContext(audioContext);
			alcCloseDevice(device);
		}
		if (videoContext) SDL_GL_DeleteContext(videoContext);
		if (window) SDL_DestroyWindow(window);
		SDL_Quit();

		return exit;
	}
};

SDL_Window *App::window = NULL;
SDL_GLContext App::videoContext = NULL;
ALCcontext *App::audioContext = NULL;

#endif