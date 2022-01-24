// internal
#include "ai_system.hpp"
// Returns the local bounding coordinates scaled by the current size of the entity
extern vec2 get_bounding_box(const Motion& motion);


// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
extern bool collides(const Motion& motion1, const Motion& motion2);
extern bool expert_part;
void AISystem::step(float elapsed_ms)
{
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE FISH AI HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will likely want to write new functions and need to create
	// new data structures to implement a more sophisticated Fish AI.
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	ComponentContainer<Motion> &motion_container = registry.motions;
	Entity salmon = motion_container.entities[0];
	//const RenderRequest &render_request = registry.renderRequests.get(salmon);
	Motion salmon_motion = registry.motions.get(salmon);
	for (uint i = 1; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		bool isFish = registry.fishAI.has(entity_i);
		if (!isFish) {
			continue;
		}
		FishAI& fish_i = registry.fishAI.get(entity_i);
		fish_i.distance = sqrtf(powf((motion_i.position.x - salmon_motion.position.x), 2) + powf((motion_i.position.y - salmon_motion.position.y), 2));

		fish_i.check_time += elapsed_ms;
		if (fish_i.check_time >= check_frequency) {
			fish_i.check_time = 0.f;
			if (fish_i.distance < mini_distance) {
				//motion_i.velocity.x = 0.f;
				if ((motion_i.position.y - salmon_motion.position.y) > 0) {
					motion_i.velocity.y = 110.f;
					fish_i.move_up = false;
				}
				else {
					motion_i.velocity.y = -110.f;
					fish_i.move_up = true;
				}
				fish_i.avoiding = true;
			}
			else {
				if (fish_i.avoiding) {
					//motion_i.velocity.y = 0.f;
					motion_i.velocity.x = -100.f;
					fish_i.avoiding = false;
				}
			}
		}
	}
	if (!expert_part) { return; }
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		for (uint j = 0; j < motion_container.components.size(); j++) // i+1
		{
			if (i == j)
				continue;

			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				const RenderRequest &render_requesti = registry.renderRequests.get(entity_i);
				const RenderRequest &render_requestj = registry.renderRequests.get(entity_j);
				if ((render_requesti.used_texture == TEXTURE_ASSET_ID::TURTLE &&
					render_requestj.used_texture == TEXTURE_ASSET_ID::FISH)|| 
					(render_requestj.used_texture == TEXTURE_ASSET_ID::TURTLE &&
						render_requesti.used_texture == TEXTURE_ASSET_ID::FISH) ){
					if (render_requesti.used_texture == TEXTURE_ASSET_ID::TURTLE) {
						motion_i.position.y -= 80;
					}
					if (render_requestj.used_texture == TEXTURE_ASSET_ID::TURTLE) {
						motion_j.position.y -= 80;
					}

				}
			}
		}
	}
}