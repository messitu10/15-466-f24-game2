#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint fan_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > fan_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("fan_output.pnct"));
	fan_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > fan_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("fan_output.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = fan_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = fan_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;
	});
});

PlayMode::PlayMode() : scene(*fan_scene) {
	for (auto &transform : scene.transforms) {
		if (transform.name == "Cup") cup = &transform;
		else if (transform.name == "Fan") fan = &transform;
		else if (transform.name == "Plane") plane_1 = &transform;
		else if (transform.name == "Coin_1") coin_1 = &transform;
		else if (transform.name == "Coin_2") coin_2 = &transform;
		else if (transform.name == "Coin_3") coin_3 = &transform;
	}
	if (cup == nullptr) throw std::runtime_error("cup not found.");
	if (fan == nullptr) throw std::runtime_error("fan not found.");
	if (plane_1 == nullptr) throw std::runtime_error("plane not found.");
	if (coin_1 == nullptr) throw std::runtime_error("coin_1 not found.");
	if (coin_2 == nullptr) throw std::runtime_error("coin_2 not found.");
	if (coin_3 == nullptr) throw std::runtime_error("coin_3 not found.");
	
	
	coin_base_rotation = coin_1->rotation;
	coins.emplace_back(coin_1);
	coins.emplace_back(coin_2);
	coins.emplace_back(coin_3);
	SpawnCoins();

	plane_starting_position = glm::vec3(-150.0f, 0.0f, 50.0f);
	plane_1->position = plane_starting_position;

	cup->position.z += 50.0f;
	fan_base_rotation = fan->rotation;
	plane_1_base_rotation = plane_1->rotation;

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();

	static glm::vec3 camera_offset = glm::vec3(0.0f, -300.0f, 200.0f); // Adjust these values for desired position
	camera->transform->position = camera_offset;
	camera->transform->rotation = glm::quatLookAt(glm::normalize(cup->position - camera->transform->position), glm::vec3(0.0f, 0.0f, 1.0f));
}

PlayMode::~PlayMode() {
}

bool PlayMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {

	if (evt.type == SDL_KEYDOWN) {
		if (evt.key.keysym.sym == SDLK_ESCAPE) {
			SDL_SetRelativeMouseMode(SDL_FALSE);
			return true;
		} else if (evt.key.keysym.sym == SDLK_a) {
			left.downs += 1;
			left.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.downs += 1;
			right.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.downs += 1;
			up.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.downs += 1;
			down.pressed = true;
			return true;
		} else if(evt.key.keysym.sym == SDLK_SPACE){
			space.downs += 1;
			space.pressed = true;
		} 
		
	} else if (evt.type == SDL_KEYUP) {
		if (evt.key.keysym.sym == SDLK_a) {
			left.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_d) {
			right.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_w) {
			up.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_s) {
			down.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}

	} 

	/*
	else if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
			SDL_SetRelativeMouseMode(SDL_TRUE);
			return true;
		}
	} else if (evt.type == SDL_MOUSEMOTION) {
		if (SDL_GetRelativeMouseMode() == SDL_TRUE) {
			glm::vec2 motion = glm::vec2(
				evt.motion.xrel / float(window_size.y),
				-evt.motion.yrel / float(window_size.y)
			);
			camera->transform->rotation = glm::normalize(
				camera->transform->rotation
				* glm::angleAxis(-motion.x * camera->fovy, glm::vec3(0.0f, 1.0f, 0.0f))
				* glm::angleAxis(motion.y * camera->fovy, glm::vec3(1.0f, 0.0f, 0.0f))
			);
			return true;
		}
	}
	*/

	return false;
}

void PlayMode::SpawnCoins() {
    // random position generation
    std::random_device rd;                 
    std::mt19937 gen(rd());              
	float prev_x = -80.0f;   
	std::uniform_real_distribution<float> dis_z(20.0f, 110.0f);
	
    // iterate over each coin and set a random position
    for (Scene::Transform* coin : coins) {
		coin_collected[coin] = false;
		std::uniform_real_distribution<float> dis_x(prev_x, prev_x + 60.0f); 
   		
		coin->scale = glm::vec3(1.0f, 1.0f, 1.0f);
        coin->position.x = dis_x(gen);
        coin->position.z = dis_z(gen);

		prev_x = prev_x + 90.0f;
    }
}

void PlayMode::CheckCoinCollision() {
	for (Scene::Transform* coin : coins) {
		if(coin_collected[coin] == false){
			if(plane_1->position.x > coin->position.x - 3.0f && plane_1->position.x < coin->position.x + 3.0f){
				if(plane_1->position.z > coin->position.z - 5.0f && plane_1->position.z < coin->position.z + 5.0f){
					coin->scale = glm::vec3(0.0f, 0.0f, 0.0f);
					score += 1;
					coin_collected[coin] = true;
				}
			}
		}
	}
}


void PlayMode::update(float elapsed) {

    // increment a continuous rotation angle based on elapsed time:
    static float rotation_speed = 360.0f;
	static float rotation_acceleration = 60.0f;
    static float rotation_angle = 0.0f;

    // update the rotation angle
    rotation_angle += rotation_speed * elapsed;
    rotation_angle = fmod(rotation_angle, 360.0f);

    fan->rotation = fan_base_rotation * glm::angleAxis(
        glm::radians(rotation_angle),
        glm::vec3(0.0f, 0.0f, 1.0f) 
    );

	//cup movement
	{
		constexpr float PlayerSpeed = 60.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x = -1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;

		if (space.pressed && rotation_speed <= 1080.0f) rotation_speed += rotation_acceleration;
		if (!space.pressed && rotation_speed >= 180.0f) rotation_speed -= rotation_acceleration;

		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;
		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];

		frame_right.z = 0.0f;

		frame_right = glm::normalize(frame_right);

		cup->position += move.x * frame_right;
		cup->position.z = 0.0f;
	}

	//plane movement
	plane_1->position.x += 0.5f;
	{	
		static float max_tilt_angle = 30.0f; 
    	static float current_tilt_angle = 0.0f; 
		glm::quat tilt_rotation = glm::angleAxis(glm::radians(current_tilt_angle), glm::vec3(0.0f, 1.0f, 0.0f)); 
		if(space.pressed && plane_1->position.z < 120.0f){
			if(cup->position.x >= plane_1->position.x - 10.0f && 
				cup->position.x <= plane_1->position.x + 10.0f){
				plane_1->position.z += 0.003f * (rotation_speed - 360.0f);
				current_tilt_angle = std::max(current_tilt_angle - 3.0f, -max_tilt_angle);
				plane_1->rotation = tilt_rotation * plane_1_base_rotation;
			}
		}
		else if(!space.pressed && plane_1->position.z > 20.0f){
			plane_1->position.z -= 0.2f;
			current_tilt_angle = std::min(current_tilt_angle + 5.0f, max_tilt_angle);
			plane_1->rotation = tilt_rotation * plane_1_base_rotation;
		}

		else{
			current_tilt_angle = std::max(current_tilt_angle - 3.0f, 0.0f);
			plane_1->rotation = tilt_rotation * plane_1_base_rotation;
		}
	}

	//coin rotations
	{
		static float rotation_speed = 90.0f;
    	static float rotation_angle = 0.0f;

		rotation_angle += rotation_speed * elapsed;
		rotation_angle = fmod(rotation_angle, 360.0f);

		for(Scene::Transform *coin : coins){
			
			coin->rotation = coin_base_rotation * glm::angleAxis(
       		glm::radians(rotation_angle),
        	glm::vec3(0.0f, 1.0f, 0.0f) 
			);
		}
	}

	//coin collision and update score
	{
		CheckCoinCollision();
	}

	//plane reset and coin respawn
	if(plane_1->position.x > 150.0f){
			plane_1->rotation = plane_1_base_rotation;
			plane_1->position = plane_starting_position;
			cup->position.x = 0.0f;	
			SpawnCoins();
	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;
}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(0.0f, 0.0f,-1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);

	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClearDepth(1.0f); //1.0 is actually the default value to clear the depth buffer to, but FYI you can change it.
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS); //this is the default depth comparison function, but FYI you can change it.

	GL_ERRORS(); //print any errors produced by this setup code

	scene.draw(*camera);

	{ //use DrawLines to overlay some text:
		glDisable(GL_DEPTH_TEST);
		float aspect = float(drawable_size.x) / float(drawable_size.y);
		DrawLines lines(glm::mat4(
			1.0f / aspect, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		));

		std::string score_text = "Score: " + std::to_string(score);
		constexpr float H = 0.09f;
		lines.draw_text(score_text,
			glm::vec3(-aspect + 0.1f * H, -1.0 + 0.1f * H, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(score_text,
			glm::vec3(-aspect + 0.1f * H + ofs, -1.0 + 0.1f * H + ofs, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
