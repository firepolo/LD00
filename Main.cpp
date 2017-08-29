#include "Main.h"

template<typename T>
inline T Random::GetNumber(T min, T max)
{
	return T(min + rand() / (float)RAND_MAX * (max - min));
}

inline void Pointer::Delete(void *p)
{
	if (!p) return;
	delete p;
	p = NULL;
}

inline void Array::Delete(void **p, GLuint count)
{
	if (!p) return;
	while (count) Pointer::Delete(p[--count]);
	delete[] p;
	p = NULL;
}

char *File::ReadAll(const char *filename, GLuint *size)
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

const glm::mat4 Mat4::PROJECTION = glm::perspective<float>(M_PI / 180.0f * 70.0f, WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.01f, 100.0f);
const glm::mat4 Mat4::IDENTITY = glm::mat4(1);
const glm::mat4 Mat4::HAND = glm::scale(Mat4::IDENTITY, glm::vec3(2, 2, 0));

Shader *Shader::WORLD = NULL;

Shader *Shader::Load(GLuint mask, const std::string &filename)
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

Shader::Shader(GLuint _id) : id(_id) {}
Shader::~Shader() { glDeleteProgram(id); }

inline void Shader::Bind() { glUseProgram(id); }
inline void Shader::Unbind() { glUseProgram(0); }

GLuint Shader::Compile(GLuint id, GLenum type, const std::string &filename)
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

Model *Model::E = NULL, *Model::I = NULL, *Model::H = NULL, *Model::L = NULL, *Model::U = NULL, *Model::ENEMY = NULL;

Model *Model::Load(const std::string &filename)
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

Model::Model(GLuint _vbo, GLuint _vao, GLuint _count) : vbo(_vbo), vao(_vao), count(_count) {}
	
Model::~Model()
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
}

inline void Model::Bind() { glBindVertexArray(vao); }
inline void Model::Unbind() { glBindVertexArray(0); }

Texture *Texture::GLOBAL = NULL;

Texture *Texture::Load(const std::string &filename)
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

Texture::Texture(GLuint _id) : id(_id) {}
Texture::~Texture() { glDeleteTextures(1, &id); }

inline void Texture::Bind() { glBindTexture(GL_TEXTURE_2D, id); }
inline void Texture::Unbind() { glBindTexture(GL_TEXTURE_2D, 0); }

Sound *Sound::TEST = NULL;

Sound *Sound::Load(const std::string &filename)
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

Sound::Sound(GLuint _buf, GLuint _src) : buf(_buf), src(_src) {}
Sound::~Sound() { alDeleteSources(1, &src); alDeleteBuffers(1, &buf); }

inline int Sound::GetState() { int s; alGetSourcei(src, AL_SOURCE_STATE, &s); return s; }
inline void Sound::SetLooping(bool looping) { alSourcei(src, AL_LOOPING, looping); }
inline void Sound::Play() { if (GetState() != AL_PLAYING) alSourcePlay(src); }
inline void Sound::Pause() { if (GetState() == AL_PLAYING) alSourcePause(src); }
inline void Sound::Stop() { if (GetState() != AL_STOPPED) alSourceStop(src); }

bool *Input::KEYBOARD = 0;

Point &Point::Set(short x, short y)
{
	this->x = x;
	this->y = y;
	return *this;
}

bool Point::operator==(const Point &o) const
{
	return *((int*)this) == *((int*)&o);
}

Player *Player::INSTANCE = NULL;
Player::Player(glm::vec3 _position, float _angle) : position(_position), angle(_angle), look(glm::vec3(glm::cos(angle), 0, glm::sin(angle))), frame(1), attackTicks(0), moving(0) {}

void Player::CheckInput()
{
	char moveMask = Input::KEYBOARD[SDL_SCANCODE_W] || Input::KEYBOARD[SDL_SCANCODE_UP];
	moveMask |= (Input::KEYBOARD[SDL_SCANCODE_S] || Input::KEYBOARD[SDL_SCANCODE_DOWN]) << 1;
	moveMask |= Input::KEYBOARD[SDL_SCANCODE_A] << 2;
	moveMask |= Input::KEYBOARD[SDL_SCANCODE_D] << 3;
	
	if (moveMask & 1) Map::INSTANCE->Move(position, look * PLAYER_SPEED);
	else if (moveMask & 2) Map::INSTANCE->Move(position, look * -PLAYER_SPEED);
	
	if (moveMask & 4) Map::INSTANCE->Move(position, glm::vec3(look.z * PLAYER_SPEED, 0, look.x * -PLAYER_SPEED));
	else if (moveMask & 8) Map::INSTANCE->Move(position, glm::vec3(look.z * -PLAYER_SPEED, 0, look.x * PLAYER_SPEED));
	
	if (Input::KEYBOARD[SDL_SCANCODE_LEFT])
	{
		angle -= PLAYER_ANGLE_SPEED;
		look.x = glm::cos(angle);
		look.z = glm::sin(angle);
	}
	else if (Input::KEYBOARD[SDL_SCANCODE_RIGHT])
	{
		angle += PLAYER_ANGLE_SPEED;
		look.x = glm::cos(angle);
		look.z = glm::sin(angle);
	}
	
	if (moveMask)
	{
		moving += 0.25f;
		position.y = glm::cos(moving) * 0.05f;
	}
	
	if (attackTicks > 1)
	{
		--attackTicks;
		return;
	}
	
	frame = 1;
	
	if (attackTicks == 1 && Input::KEYBOARD[SDL_SCANCODE_SPACE]) return;
	
	attackTicks = 0;
	
	if (Input::KEYBOARD[SDL_SCANCODE_SPACE])
	{
		attackTicks = PLAYER_ATTACK_TICKS;
		frame = 2;
		
		bool hit = false;
		
		for (int z = position.z - Map::INSTANCE->origin.y + 0.5f - 1, ez = z + 2; z <= ez; ++z)
		{
			if (z < 0 || z >= Map::INSTANCE->size.y) continue;
			
			for (int x = position.x - Map::INSTANCE->origin.x + 0.5f - 1, ex = x + 2; x <= ex; ++x)
			{
				if (x < 0 || x >= Map::INSTANCE->size.x) continue;
				
				Block *b = Map::INSTANCE->blocks[z * Map::INSTANCE->size.x + x];
				if (b == NULL) continue;
				
				for (int i = 0; i < b->enemies.size; ++i)
				{
					Enemy *enemy = b->enemies.data[i];
					if (enemy->animation > 0) continue;
					
					glm::vec3 relative = enemy->position - position;
					float dp = glm::dot(look, relative);
					
					if (dp > 0 && glm::length(relative) < HIT_DISTANCE)
					{
						enemy->PlayFallAnimation();
						hit = true;
					}
				}
			}
		}
		
		if (hit) Sound::TEST->Play();
	}
	else if (!Input::KEYBOARD[SDL_SCANCODE_SPACE]) frame = 1;
}

void Player::Draw()
{
	Model::ENEMY->Bind();
	
	glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&Mat4::IDENTITY);
	glUniformMatrix4fv(1, 1, GL_FALSE, (float *)&Mat4::HAND);
	glUniformMatrix4fv(2, 1, GL_FALSE, (float *)&Mat4::IDENTITY);
	glUniform1i(3, 2);
	glUniform1i(4, frame);
	glDrawArrays(GL_TRIANGLES, 0, Model::ENEMY->count);
	Model::ENEMY->Unbind();
}

Enemy::Enemy(glm::vec3 _position) : position(_position)
{
	decisionTick = ENEMY_DECISIONS_TICKS;
	animationTick = ENEMY_ANIMATION_TICKS;
	animation = 0;
	frame = 0;
	SetDirection();
}

void Enemy::SetDirection()
{
	direction.x = Random::GetNumber<float>(-ENEMY_SPEED, ENEMY_SPEED);
	direction.z = Random::GetNumber<float>(-ENEMY_SPEED, ENEMY_SPEED);
	decisionTick = Random::GetNumber<GLuint>(0, ENEMY_DECISIONS_TICKS);
}

void Enemy::PlayFallAnimation()
{
	frame = 0;
	animation = 1;
	animationTick = ENEMY_ANIMATION_FALL_FRAMES;
}

void Enemy::Update()
{
	if (animation == 2) return;
	
 	if (animation == 1)
	{
		if (--animationTick == 0)
		{
			if (frame++ == ENEMY_ANIMATION_FALL_FRAMES)
			{
				animation = 2;
				frame = 0;
			}
			animationTick = ENEMY_ANIMATION_TICKS;
		}
		return;
	}
	
	if (--decisionTick == 0) SetDirection();
	if (--animationTick == 0)
	{
		frame = (frame + 1) % ENEMY_ANIMATION_WALK_FRAMES;
		animationTick = ENEMY_ANIMATION_TICKS;
	}
	
	Map::INSTANCE->Move(position, direction);
}

template<GLuint chunk, GLuint ptrSize = sizeof(void *)>
EnemyList<chunk, ptrSize>::EnemyList()
{
	data = new Enemy*[chunk];
	capacity = chunk;
	size = 0;
}

template<GLuint chunk, GLuint ptrSize = sizeof(void *)>
EnemyList<chunk, ptrSize>::~EnemyList() { Array::Delete((void **)data, 0); }

template<GLuint chunk, GLuint ptrSize = sizeof(void *)>
void EnemyList<chunk, ptrSize>::Add(Enemy *enemy)
{
	GLuint last = size;
	
	if (++size >= capacity)
	{
		capacity += chunk;
		Enemy **newData = new Enemy*[capacity];
		memcpy(newData, data, last * ptrSize);
		Array::Delete((void **)data, 0);
		data = newData;
	}
	
	data[last] = enemy;
}

template<GLuint chunk, GLuint ptrSize = sizeof(void *)>
void EnemyList<chunk, ptrSize>::Remove(GLuint index)
{
	if (size == 0 || index < 0 || index >= size) return;
	
	Enemy **dst = data + index;
	memmove(dst, dst + 1, (--size - index) * ptrSize);
}

Block::Block(Model *_model, glm::mat4 _transform) : model(_model), transform(_transform) {}
Block::~Block() {}

void Block::Draw()
{
	glUniform1i(3, 0);
	glUniform1i(4, 0);
	
	model->Bind();
	glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&transform);
	glDrawArrays(GL_TRIANGLES, 0, model->count);
	model->Unbind();
	
	for (GLuint i = 0; i < enemies.size;)
	{
		Enemy *it = enemies.data[i];
		it->Update();
		
		Model::ENEMY->Bind();
		glm::mat4 uModel = glm::rotate(glm::translate(Mat4::IDENTITY, it->position), (float)atan2(Player::INSTANCE->position.x - it->position.x, Player::INSTANCE->position.z - it->position.z), glm::vec3(0, 1, 0));
		glUniformMatrix4fv(0, 1, GL_FALSE, (float *)&uModel);
		glUniform1i(3, it->animation);
		glUniform1i(4, it->frame);
		glDrawArrays(GL_TRIANGLES, 0, Model::ENEMY->count);
		Model::ENEMY->Unbind();
		
		Block *block = Map::INSTANCE->GetBlock(it->position);
		
		if (block == this || block == NULL) ++i;
		else
		{
			block->enemies.Add(it);
			enemies.Remove(i);
		}
	}
}

Map *Map::INSTANCE = NULL;

Map::Map(Block **blocks, const Point &size, const Point &origin)
{
	this->blocks = blocks;
	this->size = size;
	this->origin = origin;
}

Map::~Map()
{
	Array::Delete((void **)blocks, size.x * size.y);
	Array::Delete((void **)&enemies[0], enemies.size());
}

inline float Map::GetX(float x) { return x - origin.x + 0.5f; }
inline float Map::GetY(float y) { return y - origin.y + 0.5f; }
inline Block *Map::GetBlock(const glm::vec3 &position)
{
	int x = (int)GetX(position.x);
	int y = (int)GetY(position.z);
	return x < 0 || x >= size.x || y < 0 || y >= size.y ? NULL : blocks[y * size.x + x];
}

bool Map::CanMove(glm::vec3 &position, const glm::vec3 &direction)
{
	float x = position.x + direction.x;
	float z = position.z + direction.z;
	float hx = GetX(x) + (direction.x < 0 ? -HITBOX_SIZE : HITBOX_SIZE);
	float hz = GetY(z) + (direction.z < 0 ? -HITBOX_SIZE : HITBOX_SIZE);
	
	if (hx < 0 || hx >= size.x || hz < 0 || hz >= size.y) return false;
	if (blocks[(int)hz * size.x + (int)hx] == NULL) return false;
	
	position.x = x;
	position.z = z;
	return true;
}

void Map::Move(glm::vec3 &position, const glm::vec3 &direction)
{
	if (CanMove(position, direction)) return;
	if (CanMove(position, glm::vec3(0, 0, direction.z))) return;
	if (CanMove(position, glm::vec3(direction.x, 0, 0))) return;
}

void Map::AddEnemies(GLuint number)
{
	while (number)
	{
		float x = Random::GetNumber<float>(0, size.x);
		float z = Random::GetNumber<float>(0, size.y);
		GLuint i = (int)z * size.x + (int)x;
		if (blocks[i])
		{
			Enemy *enemy = new Enemy(glm::vec3(x + origin.x - 0.5, 0, z + origin.y - 0.5));
			enemies.push_back(enemy);
			blocks[i]->enemies.Add(enemy);
			--number;
		}
	}
}

void Map::Draw()
{
#if TOP_VIEW_MODE==1
	glm::mat4 uView = glm::lookAt(Player::INSTANCE->position, glm::vec3(Player::INSTANCE->position.x + Player::INSTANCE->look.x, 2, Player::INSTANCE->position.z + Player::INSTANCE->look.z), glm::vec3(0, 1, 0));
#else
	glm::mat4 uView = glm::lookAt(Player::INSTANCE->position, Player::INSTANCE->position + Player::INSTANCE->look, glm::vec3(0, 1, 0));
#endif
	
	glUniformMatrix4fv(1, 1, GL_FALSE, (float *)&uView);
	glUniformMatrix4fv(2, 1, GL_FALSE, (float *)&Mat4::PROJECTION);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	for (int z = Player::INSTANCE->position.z - origin.y + 0.5f - PLAYER_VISIBLE_DISTANCE, ez = z + (PLAYER_VISIBLE_DISTANCE << 1); z <= ez; ++z)
	{
		if (z < 0 || z >= size.y) continue;
		
		for (int x = Player::INSTANCE->position.x - origin.x + 0.5f - PLAYER_VISIBLE_DISTANCE, ex = x + (PLAYER_VISIBLE_DISTANCE << 1); x <= ex; ++x)
		{
			if (x < 0 || x >= size.x) continue;
			
			Block *b = blocks[z * size.x + x];
			if (b != NULL) b->Draw();
		}
	}
}

Map *Map::Generate(GLuint size)
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
		if (c == 0b1111) blocks[i] = new Block(Model::E, glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)));
		// I
		else if (c == 0b1110) blocks[i] = new Block(Model::I, glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)));
		else if (c == 0b1101) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
		else if (c == 0b1011) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
		else if (c == 0b0111) blocks[i] = new Block(Model::I, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
		// H
		else if (c == 0b1010) blocks[i] = new Block(Model::H, glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)));
		else if (c == 0b0101) blocks[i] = new Block(Model::H, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
		// L
		else if (c == 0b1100) blocks[i] = new Block(Model::L, glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)));
		else if (c == 0b1001) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
		else if (c == 0b0011) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
		else if (c == 0b0110) blocks[i] = new Block(Model::L, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
		// U
		else if (c == 0b1000) blocks[i] = new Block(Model::U, glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)));
		else if (c == 0b0001) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / -2, glm::vec3(0, 1, 0)));
		else if (c == 0b0010) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), -M_PI, glm::vec3(0, 1, 0)));
		else if (c == 0b0100) blocks[i] = new Block(Model::U, glm::rotate<float>(glm::translate(Mat4::IDENTITY, glm::vec3(it->x, 0, it->y)), M_PI / 2, glm::vec3(0, 1, 0)));
	}
	
	return new Map(blocks, { w, h }, { l, t });
}

SDL_Window *App::window = NULL;
SDL_GLContext App::videoContext = NULL;
ALCcontext *App::audioContext = NULL;

int App::Start()
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
		Player::INSTANCE->CheckInput();
		
		// RENDER
		Shader::WORLD->Bind();
		Texture::GLOBAL->Bind();
		
		Map::INSTANCE->Draw();
		Player::INSTANCE->Draw();
		
		Texture::GLOBAL->Unbind();
		Shader::WORLD->Unbind();

		SDL_GL_SwapWindow(window);
		SDL_Delay(FPS);
	}

	return Shutdown(0);
}

int App::Initialize()
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

	Texture::GLOBAL = Texture::Load("resources\\textures\\global.bmp");
	if (!Texture::GLOBAL) return Shutdown(11);
	
	Sound::TEST = Sound::Load("resources\\sounds\\shoot.wav");
	if (!Sound::TEST) return Shutdown(18);
	
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
		Player::INSTANCE = new Player(glm::vec3(0, 5, 0), 0);
	#else
		Player::INSTANCE = new Player(glm::vec3(0, 0, 0), 0);
	#endif
	
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.1, 0.5, 0.8, 1);
	
	Shader::WORLD->Bind();
	glUniform1i(5, 0);
	Shader::WORLD->Unbind();
	
	return 0;
}

int App::Shutdown(int exit)
{
	Pointer::Delete(Player::INSTANCE);
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

int main(int argc, char *argv[])
{
	int err = App::Initialize();
	if (err) return err;
	return App::Start();
}