#include "Mode.hpp"

#include "Scene.hpp"

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
	} left, right, down, up, hleft, hright, hdown, hup, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;
	int points = 0; //POINT TRACKER
	
	float sun = -1.0f;//shader changing floats
	float red = 0.0f;
	float green = 0.0f;
	float blue = 0.0f;
	int dir = 1;


	std::vector<uint8_t> duckholes; // vector for which holes have ducks in them
	std::vector<uint8_t> holes; //vector for which holes are empty

	float floattimes[9]; //array for the amount of time until the ducks float up to the surface
	float sinktimes[9]; //array for the amount of time until the ducks sink under the water

	//animations
	float squash[9];
	float sink[9];
	float rise[9];
	
	std::vector<float> randomWobbles; //duck mesh vectors
	std::vector<Scene::Transform *> duckies;
	std::vector<glm::quat> duckie_rotations;

	float hammerwobble = 0.0f; //hammer mesh vectors and hammer properties
	Scene::Transform *hammer = nullptr;
	glm::quat hammer_base_rotation;
	glm::quat hammer_base_position;
	float wack = 0.0f;
	float hammercooldown = 0.0f;

	Scene::Transform *hover = nullptr; //hover mesh vectors and hover properties
	glm::quat hover_position;
	std::vector<glm::vec3> hoverpos;
	int hoverhole = 0;
	float hovercooldown = 0.0f;

	//hexapod leg to wobble:
	Scene::Transform *hip = nullptr;
	Scene::Transform *upper_leg = nullptr;
	Scene::Transform *lower_leg = nullptr;
	glm::quat hip_base_rotation;
	glm::quat upper_leg_base_rotation;
	glm::quat lower_leg_base_rotation;
	float wobble = 0.0f;
	
	//camera:
	Scene::Camera *camera = nullptr;

};
