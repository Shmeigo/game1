#include "PlayMode.hpp"
#include "load_save_png.hpp"

//for the GL_ERRORS() macro:
#include "gl_errors.hpp"

//for glm::value_ptr() :
#include <glm/gtc/type_ptr.hpp>

#include <random>

PlayMode::PlayMode() {
	//TODO:
	// you *must* use an asset pipeline of some sort to generate tiles.
	// don't hardcode them like this!
	// or, at least, if you do hardcode them like this,
	//  make yourself a script that spits out the code that you paste in here
	//   and check that script into your repository.

	auto initialize_sprite = [this](int sprite_ind, int pal_ind, std::string const filename) {
		glm::uvec2 size;
		std::vector< glm::u8vec4 >data;
		load_png(filename, &size, &data, LowerLeftOrigin);
			ppu.palette_table[pal_ind] = {
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		};
		ppu.tile_table[sprite_ind].bit0 = {
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
		};
		ppu.tile_table[sprite_ind].bit1 = {
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
			0b00000000,
		};
		int ind = 0;
		for (uint32_t i = 0; i < size.x * size.y; i++) {
			if (std::find(std::begin(ppu.palette_table[pal_ind]), std::end(ppu.palette_table[pal_ind]), data[i]) == std::end(ppu.palette_table[pal_ind])) {
				ppu.palette_table[pal_ind][ind] = data[i];
				ind++;
				if (ind >= 4) {
					break;
				}
			}
		}
		for (uint32_t i = 0; i < size.y; i++) {
			for (uint32_t j = 0; j < size.x; j++) {
				if (data[i * size.x + j] == ppu.palette_table[pal_ind][1]) {
						ppu.tile_table[sprite_ind].bit0[i] |= (1 << j);
				} else if (data[i * size.x + j] == ppu.palette_table[pal_ind][2]) {
						ppu.tile_table[sprite_ind].bit1[i] |= (1 << j);
				}
			}
		}
	};

	initialize_sprite(32, 7, "sprites/game1idle.png");
	initialize_sprite(33, 7, "sprites/game1idleflip.png");
	initialize_sprite(34, 7, "sprites/game1up.png");
	initialize_sprite(35, 7, "sprites/game1down.png");
	initialize_sprite(36, 4, "sprites/game1dirt.png");
	initialize_sprite(37, 5, "sprites/game1grass.png");


	for (int i = 0; i < ppu.BackgroundWidth * ppu.BackgroundHeight; i++) {
		ppu.background[i] = 0;
	}

	//makes the outside of tiles 0-16 solid:
	ppu.palette_table[0] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};

	//makes the center of tiles 0-16 solid:
	ppu.palette_table[1] = {
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
		glm::u8vec4(0x00, 0x00, 0x00, 0x00),
	};
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_LEFT || evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			facing_left = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT || evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			facing_left = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_w || evt.key.keysym.sym == SDLK_SPACE) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN || evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		}
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_LEFT || evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT || evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP || evt.key.keysym.sym == SDLK_w || evt.key.keysym.sym == SDLK_SPACE) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN || evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		}
	}

	return false;
}

void PlayMode::update(float elapsed) {


	if (left.pressed) player_velocity.x -= player_acceleration * elapsed;
	else if (right.pressed) player_velocity.x += player_acceleration * elapsed;
	else {
		player_velocity.x += player_velocity.x > 0 ? -drag * elapsed : drag * elapsed;
		if (player_velocity.x < 25.0f && player_velocity.x > -25.0f) {
			player_velocity.x = 0.0f;
		}
	}
	if (player_velocity.x > maxXVel) player_velocity.x = maxXVel;
	if (player_velocity.x < -maxXVel) player_velocity.x = -maxXVel;

	if (up.pressed && can_jump) {
		player_velocity.y = jump_speed;
		can_jump = false;
	}
	player_velocity.y += up.pressed ? elapsed * long_jump_gravity : elapsed * gravity;
	player_at += elapsed * player_velocity;

	if (player_at.y <= 48.0f) {
		player_at.y = 48.0f;
		player_velocity.y = 0.0f;
		can_jump = true;
	}	
	if (player_at.x <= 0.0f) {
		player_at.x = 0.0f;
		player_velocity.x = 0.0f;
	}
	if (player_at.x >= 248.0f) {
		player_at.x = 248.0f;
		player_velocity.x = 0.0f;
	}
	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//--- set ppu state based on game state ---
	ppu.background_color = glm::u8vec4(
		0xa0, 0xa0, 0xa0, 0xff
	);

	//tilemap gets recomputed every frame as some weird plasma thing:
	//NOTE: don't do this in your game! actually make a map or something :-)
	for (uint32_t y = 0; y < PPU466::BackgroundHeight; ++y) {
		for (uint32_t x = 0; x < PPU466::BackgroundWidth; ++x) {
			ppu.background[x+PPU466::BackgroundWidth*y] = ((x+y)%16);
			if (y < 8) {
				ppu.background[x + ppu.BackgroundWidth * y] = 0 | (4 << 8) | 36;
			} else if (y == 8) {
				ppu.background[x + ppu.BackgroundWidth * y] = 0 | (5 << 8) | 37;
			}
		}
	}
	//background scroll:
	ppu.background_position.x = int32_t(-0.5f * player_at.x);
	ppu.background_position.y = int32_t(-0.5f * player_at.y);


	//player sprite:
	ppu.sprites[0].x = int32_t(player_at.x);
	ppu.sprites[0].y = int32_t(player_at.y);
	if (player_velocity.y > 0.2f) {
		ppu.sprites[0].index = 34;
	} else if (player_velocity.y < -0.2f) {
		ppu.sprites[0].index = 35;
	} else if (facing_left) {
		ppu.sprites[0].index = 32;
	} else {
		ppu.sprites[0].index = 33;
	}
	ppu.sprites[0].attributes = 7;

	//--- actually draw ---
	ppu.draw(drawable_size);
}
