#include "PPU466.hpp"
#include "Mode.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <deque>

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//functions called by main loop:
	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up;

	//some weird background animation:
	float background_fade = 0.0f;

	bool facing_left = true;
	bool can_jump = false;
	
	//player position:
	glm::vec2 player_at = glm::vec2(0.0f);
	glm::vec2 player_velocity = glm::vec2(0.0f, 0.0f);
	float gravity = -800.0f;
	float long_jump_gravity = -400.0f;
	float jump_speed = 200.0f;
	float maxXVel = 150.0f;
	float player_acceleration = 1200.0f;
	float drag = 700.0f;

	

	//----- drawing handled by PPU466 -----

	PPU466 ppu;
};
