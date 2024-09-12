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
	
	void SpawnCoins();
	void CheckCoinCollision();

	//----- game state -----

	//input tracking:
	struct Button {
		uint8_t downs = 0;
		uint8_t pressed = 0;
	} left, right, down, up, space;

	//local copy of the game scene (so code can change it during gameplay):
	Scene scene;

	Scene::Transform *cup = nullptr;
	Scene::Transform *fan = nullptr;
	Scene::Transform *plane_1 = nullptr;
	Scene::Transform *plane_2 = nullptr;
	Scene::Transform *coin_1 = nullptr;
	Scene::Transform *coin_2 = nullptr;
	Scene::Transform *coin_3 = nullptr;

	glm::quat fan_base_rotation;
	glm::quat coin_base_rotation;
	glm::quat plane_1_base_rotation;
	glm::vec3 plane_starting_position;
	std::vector<Scene::Transform*> coins;
	std::unordered_map<Scene::Transform*, bool> coin_collected;
	uint32_t score = 0;

	//camera:
	Scene::Camera *camera = nullptr;

};
