#include "PlayMode.hpp"

#include "LitColorTextureProgram.hpp"

#include "DrawLines.hpp"
#include "Mesh.hpp"
#include "Load.hpp"
#include "gl_errors.hpp"
#include "data_path.hpp"

#include <string>
#include <glm/gtc/type_ptr.hpp>

#include <random>

GLuint hexapod_meshes_for_lit_color_texture_program = 0;
Load< MeshBuffer > hexapod_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer const *ret = new MeshBuffer(data_path("pool.pnct"));
	hexapod_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);
	return ret;
});

Load< Scene > hexapod_scene(LoadTagDefault, []() -> Scene const * {
	return new Scene(data_path("pool.scene"), [&](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = hexapod_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable &drawable = scene.drawables.back();

		drawable.pipeline = lit_color_texture_program_pipeline;

		drawable.pipeline.vao = hexapod_meshes_for_lit_color_texture_program;
		drawable.pipeline.type = mesh.type;
		drawable.pipeline.start = mesh.start;
		drawable.pipeline.count = mesh.count;

	});
});

PlayMode::PlayMode() : scene(*hexapod_scene) {
	//get pointers to leg for convenience:

	std::string duckname = "mrducky";
	uint8_t counter = 0;
	for (auto &transform : scene.transforms) {
		if (transform.name == "wammerhammer") hammer = &transform;
		else if (transform.name == "waterselector") hover = &transform;
		else if (!(transform.name.find(duckname))) 
		{
			float randwobble = ((rand()) / static_cast <float> (RAND_MAX)) * 3.0f + 1.0f;
			//first set all the ducks z to - 2 -> under the water to start with and in the holes vector
			Scene::Transform *newduck =nullptr;
			newduck = &transform;
			duckies.emplace_back(newduck);

			randomWobbles.emplace_back(randwobble);

			uint8_t holenum = counter;
			duckholes.emplace_back(holenum);

			glm::vec3 duckpos = glm::vec3(newduck->position.x, newduck->position.y, newduck->position.z);
			hoverpos.emplace_back(duckpos);
			counter ++;
		}
	}

	if (hammer == nullptr) throw std::runtime_error("Hammer not found.");
	if (hover == nullptr) throw std::runtime_error("Hover not found.");

	hammer_base_rotation = hammer->rotation;
	hammer_base_position = hammer->position;
	hover_position = hover->position;
	
	for(int i = 0; i <duckies.size(); i ++)
	{
		glm::quat duck_rotate = duckies[i] -> rotation;
		duckie_rotations.emplace_back(duck_rotate);
	}


	for(int x = 0; x < sizeof(sink)/sizeof(sink[0]); x ++)
	{
		sink[x] = 2.0f;
		squash[x] = 0.0f;
		rise[x] = 0.0f;
		sinktimes[x] = float(rand()%4 + 1);
	}

	//get pointer to camera for convenience:
	if (scene.cameras.size() != 1) throw std::runtime_error("Expecting scene to have exactly one camera, but it has " + std::to_string(scene.cameras.size()));
	camera = &scene.cameras.front();
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
		}
		else if (evt.key.keysym.sym == SDLK_LEFT) {
			hleft.downs += 1;
			hleft.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			hright.downs += 1;
			hright.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			hup.downs += 1;
			hup.pressed = true;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			hdown.downs += 1;
			hdown.pressed = true;
			return true;
		}
		else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.downs += 1;
			space.pressed = true;
			return true;
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
		}
		  else if (evt.key.keysym.sym == SDLK_LEFT) {
			hleft.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_RIGHT) {
			hright.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_UP) {
			hup.pressed = false;
			return true;
		} else if (evt.key.keysym.sym == SDLK_DOWN) {
			hdown.pressed = false;
			return true;
		}
		 else if (evt.key.keysym.sym == SDLK_SPACE) {
			space.pressed = false;
			return true;
		}
	}
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

	return false;
}


void PlayMode::update(float elapsed) {

	//wobble variables
	
	sun += dir * (elapsed  / 25.0f);
	red += dir * (elapsed/ 100.0f);
	green += dir  * (elapsed/ 100.0f);

	hovercooldown += elapsed;
	wobble += elapsed / 10.0f;
	hammercooldown += elapsed;
	
	

	if((red > 1.0f) || (red< 0.0f)) //game is over!
	{
		if(red > 1.0f)
		red = 1.0f;
		if(red < 0.0f)
		red = 0.0f;
		
		dir = dir * -1;
	}
	else if(red > 0.75)
	{
		blue += dir * ( elapsed/ 75.0f );
	}



	//rotate the hammer menancingly 

	if(wack != 0.0f)
	{
		if(wack > 0.0f)
		{
			hammer -> rotation =  hammer_base_rotation * glm::angleAxis(
			glm::radians(-1 * ((0.5f - wack)* 45.0f * float(M_PI))),
			glm::vec3(1.0f, 0.0f, 0.0f)); 

			wack -= elapsed;
			if(wack < 0.0f)
			wack = -0.5f;

		}
		else if(wack < 0.0f)
		{
			hammer -> rotation =  hammer_base_rotation * glm::angleAxis(
			glm::radians(-1* ((-1 * wack)* 45.0f * float(M_PI))),
			glm::vec3(1.0f, 0.0f, 0.0f)); 

			wack += elapsed;
			if(wack > 0.0f)
			wack = 0.0f;

		}

	}

	//wobble the duckies
	for(int i = 0; i < duckies.size(); i++)
	{
		duckies[i] -> rotation = duckie_rotations[i] * glm::angleAxis(
		glm::radians(10.0f * std::sin(wobble * randomWobbles[i] * 2.0f  * float(M_PI))),
		glm::vec3(1.0f, 0.0f, 1.0f ));

		duckies[i] -> position.z += 0.005f * std::sin(wobble * randomWobbles[i] * 10.0f  * float(M_PI));
	}

	//squash the duckies, sink the duckies

	for(int i = 0; i < holes.size(); i++)
	{
		if(squash[holes[i]] > 0.0f) //if the duck hasn't been fully squashed
		{
			duckies[holes[i]] ->scale.y -= elapsed;
			squash[holes[i]] -= elapsed;

		}
		else if(sink[holes[i]] > 0.0f) //if the duck hasn't been fully sunk
		{
			if(squash[holes[i]] == -2.0f) //if the duck hasn't been squashed
			{
				duckies[holes[i]] -> position.z -= 4.0f * elapsed;
				sink[holes[i]] -= 4.0f * elapsed;
			}
			else
			{
				duckies[holes[i]] -> position.z -= elapsed;
				sink[holes[i]] -= elapsed;
			}

		}
	}

	//rise the duckies, sink the duckies if enough time has passed
	for(int i = 0; i < duckholes.size(); i++)
	{
		sinktimes[duckholes[i]] -= elapsed;

		if(rise[duckholes[i]] > 0.0f) //if the duck hasn't fully risen yet
		{
			duckies[duckholes[i]] ->position.z += elapsed;
			rise[duckholes[i]] -= elapsed;

			if(rise[duckholes[i]] < 0.0f) //if the duck has finished rising
			{
				sinktimes[duckholes[i]] = float(rand()%4 + 1);
			}
			
		}
		else if(sinktimes[duckholes[i]] <= 0.0f) //if the duck has been above water for too long
		{
			uint8_t duckless = uint8_t(duckholes[i]);
			holes.push_back(duckless);
			sink[duckholes[i]] = 2.0f - rise[duckholes[i]]; //reset the sink time
			squash[duckholes[i]] = -2.0f; //set the squash time to -2.0f
			
			//put a random float in & start counting down
			floattimes[duckholes[i]] = float(rand()%4 + 1);
			auto it = duckholes.begin() + i; //create an iterator to erase it from holes
			duckholes.erase(it);

		}
	}
	
	//move hover to the position of the duck to be wacked
	hover -> position.x = hoverpos[hoverhole].x;
	hover -> position.y = hoverpos[hoverhole].y;
	hover -> position.z = hoverpos[hoverhole].z + abs(0.25f * std::sin(wobble * 10.0f * float(M_PI)));
	hover -> scale.x += ( 0.001f *  std::sin(wobble * 10.0f * float(M_PI)));
	hover -> scale.z += (0.001f * std::sin(wobble * 10.0f * float(M_PI)));




	//iterate through the empty hole list, if the float timer hits 0 put it in the duckholes list and 
	//bring back up the duck
	
	for(int i=0; i < holes.size(); i++)
	{
		floattimes[holes[i]] -= elapsed;

		if(floattimes[holes[i]] < 0.0f) //if the duck has finished sinking
		{

			floattimes[holes[i]] = 0.0f;
			rise[holes[i]] = 2.0f - sink[holes[i]];
			sink[holes[i]] = 2.0f;
			squash[holes[i]] = 0.0f;
			duckies[holes[i]] ->scale.y = 0.3f;

			uint8_t duckhole = uint8_t(holes[i]);
			duckholes.emplace_back(duckhole);
			auto it = holes.begin() + i; //create an iterator to erase it from holes
			holes.erase(it);
			
		}
	}

	//if spacebar is hit, and that hole has a duck in it, that duck sinks 
	if(hammercooldown > 1.0f)
	{
			if(space.pressed) 
		{
			auto first = duckholes.begin(); //find from geeksforgeeks
			auto last = duckholes.end();

			// Searching for key in the range [first, last)
			auto it = find(first, last, hoverhole);

			// Checking if element is found or not
			if (it != duckholes.end())
			{
				points += 1;
				if(sink[hoverhole] == 2.0f) //if the duck is not sinking
				{
					duckholes.erase(it);
					uint8_t duckless = uint8_t(hoverhole);
					holes.push_back(duckless);
					sink[hoverhole] = 2.0f - rise[hoverhole];
					squash[hoverhole] = 0.15f;
					//put a random float in & start counting down
					floattimes[hoverhole] = float(rand()%4 + 1);
					
				}
			}
			wack = 0.25f;
			hammercooldown = 0.0f;
		}
		
	}
	
	

	//I don't think I will particularly need camera movement for my wacka bunny game 
	//move camera:
	{

		//combine inputs into a move:
		constexpr float PlayerSpeed = 30.0f;
		glm::vec2 move = glm::vec2(0.0f);
		if (left.pressed && !right.pressed) move.x =-1.0f;
		if (!left.pressed && right.pressed) move.x = 1.0f;
		if (down.pressed && !up.pressed) move.y =-1.0f;
		if (!down.pressed && up.pressed) move.y = 1.0f;

		//make it so that moving diagonally doesn't go faster:
		if (move != glm::vec2(0.0f)) move = glm::normalize(move) * PlayerSpeed * elapsed;

		glm::mat4x3 frame = camera->transform->make_local_to_parent();
		glm::vec3 frame_right = frame[0];
		//glm::vec3 up = frame[1];
		glm::vec3 frame_forward = -frame[2];

		camera->transform->position += move.x * frame_right + move.y * frame_forward;
		hammer->position += move.x * frame_right + move.y * frame_forward;

		if(hovercooldown > 0.5f)
		{
			if (hleft.pressed && !hright.pressed)
			{
				hoverhole -= 1;
				if(hoverhole < 0 ) hoverhole = 0;
				hovercooldown = 0.0f;
			} 
			if (!hleft.pressed && hright.pressed) 
			{
				hoverhole += 1;
				if(hoverhole >= int(duckies.size())) hoverhole = int(duckies.size()) - 1;
				hovercooldown = 0.0f;
			}
			if (!hdown.pressed && hup.pressed)
			{
				hoverhole -= 3;
				if(hoverhole < 0 ) hoverhole += 3;
				hovercooldown = 0.0f;
			} 
			if (hdown.pressed && !hup.pressed)
			{
				hoverhole +=3;
				if(hoverhole >= int(duckies.size())) hoverhole -= 3;
				hovercooldown = 0.0f;
			}

			
		}

	}

	//reset button press counters:
	left.downs = 0;
	right.downs = 0;
	up.downs = 0;
	down.downs = 0;

	hleft.downs = 0;
	hright.downs = 0;
	hup.downs = 0;
	hdown.downs = 0;

	space.downs = 0;



	//pseudocode for wackabunny



	//Vector corresponding to empty holes for the rabbits to pop out of 
	//Vector corresponding to regular point rabbits
	//Vector corresponding to golden point rabbits


	//the holes get moved around between the vectors
		//for example - 1 corresponds to hole one, if a regular rabbit enters that hole the hole 
		//is removed from the empty holes vector and is placed in the regular point rabbit vector

	//if a rabbit chooses a hole (random function), it will pop out of that hole 
		// i.e. it will take that hole out of the vector list
		//once it is finished popping out of the hole, the hole will go back into the vector list


	//what about there being no collission code? Don't need it!
		//have a "cursor" that highlights which hole the player will hit
		//if the player hits the hole while a rabbit is on it (i.e. the hole is NOT in the vector list)
			//play the rabbit hit animation
			//what about for multiple colored rabbits with different points?
				//check both vector lists to see which rabbit is in the hole
		//else the player goes on cooldown for hitting a hole with no rabbit 

	//STRETCH GOAL: day-night cycle? The game ends when the moon is in the sky 


	//what animations do I want in my game? Can I even have animations that are not rotations?
	//at the very least scale the rabbit y value down to indicate a hit and rotate the ears to the side
	//how many rabbits should be in the scene - would it be easier for each hold to have its own regular rabbit 
	//and golden rabbit mesh?


}

void PlayMode::draw(glm::uvec2 const &drawable_size) {
	//update camera aspect ratio for drawable:
	camera->aspect = float(drawable_size.x) / float(drawable_size.y);

	//set up light type and position for lit_color_texture_program:
	// TODO: consider using the Light(s) in the scene to do this
	glUseProgram(lit_color_texture_program->program);
	glUniform1i(lit_color_texture_program->LIGHT_TYPE_int, 1);
	glUniform1f(lit_color_texture_program->SUN_float, sun);
	
	glUniform3fv(lit_color_texture_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(-1.0f * dir, -1.0f,1.0f)));
	glUniform3fv(lit_color_texture_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 0.95f)));
	glUseProgram(0);


	glClearColor(0.76f - red, 0.973f - green, 1.0f - blue, 1.0f);
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

		constexpr float H = 0.09f;
		float ofs = 2.0f / drawable_size.y;
		lines.draw_text(std::to_string(points),
			glm::vec3(0.0f, aspect/2.0f -ofs * 2.0f, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0x00, 0x00, 0x00, 0x00));

		lines.draw_text(std::to_string(points),
			glm::vec3(0.0f, aspect/2.0f, 0.0),
			glm::vec3(H, 0.0f, 0.0f), glm::vec3(0.0f, H, 0.0f),
			glm::u8vec4(0xff, 0xff, 0xff, 0x00));
	}
}
