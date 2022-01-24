// internal
#include "physics_system.hpp"
#include "world_init.hpp"


// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion& motion)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(motion.scale.x), abs(motion.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides0(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(motion1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(motion2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}
bool collides(const Motion& motion1, const Motion& motion2)
{
	vec2 dp = motion1.position - motion2.position;
	float dist = sqrt(dot(dp, dp));

	const vec2 other_bonding_box = get_bounding_box(motion1);
	const float other_r_radius = min(other_bonding_box.x, other_bonding_box.y) / 2.f;

	const vec2 my_bonding_box = get_bounding_box(motion2);
	const float my_r_radius = min(my_bonding_box.x, my_bonding_box.y) / 2.f;
	if (dist < (other_r_radius + my_r_radius))
		return true;
	return false;
}
// create square box for our entities
void createBox(vec2 position, vec2 size) {
	if (size.x >= size.y) {
		createLine(vec2(position.x, position.y - (size.y / 2)), vec2(size.y, 5.f));
		createLine(vec2(position.x, position.y + (size.y / 2)), vec2(size.y, 5.f));
		createLine(vec2(position.x - (size.y / 2), position.y), vec2(5.f, size.y));
		createLine(vec2(position.x + (size.y / 2), position.y), vec2(5.f, size.y));
	} else {
		createLine(vec2(position.x, position.y - (size.x / 2)), vec2(size.x, 5.f));
		createLine(vec2(position.x, position.y + (size.x / 2)), vec2(size.x, 5.f));
		createLine(vec2(position.x - (size.x / 2), position.y), vec2(5.f, size.x));
		createLine(vec2(position.x + (size.x / 2), position.y), vec2(5.f, size.x));
	}
}

extern bool expert_part;
void PhysicsSystem::step(float elapsed_ms, float window_width_px, float window_height_px)
{
	// Move fish based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	static float t = 0;
	float cpu_second = 1.0f * (elapsed_ms / 1000.f);
	last_freeze_time += cpu_second;
	if (last_freeze_time > 1) {
		last_freeze_time = 0;
		freeze = false;
	}
	ComponentContainer<Motion> &motion_container = registry.motions;
	Entity &salmon = motion_container.entities[0];
	//const RenderRequest &render_request = registry.renderRequests.get(salmon);
	Motion &salmon_motion = registry.motions.get(salmon);
	auto& motion_registry = registry.motions;
	for(uint i = 0; i< motion_registry.size(); i++)
	{
		// !!! TODO A1: update motion.position based on step_seconds and motion.velocity
		Motion& motion = motion_registry.components[i];
		Entity entity = motion_registry.entities[i];
		float step_seconds = 1.0f * (elapsed_ms / 1000.f);
		(void)elapsed_ms; // placeholder to silence unused warning until implemented
		GLint n = 100;
		GLfloat R = 100.05;
		GLfloat Pi = 3.14159;
		if (!freeze) {
			if (entity.tid >= 1) {

				motion.position.x = 350 * i + R * cos(2 * Pi / n * motion.t);
				motion.position.y = 320 * i + R * sin(2 * Pi / n * motion.t);
				motion.t += 0.1;
			}
			else {
				int samples = 10;
				const RenderRequest &render_request = registry.renderRequests.get(entity);
				if (render_request.used_texture == TEXTURE_ASSET_ID::TURTLE && expert_part) {
					float dx = (salmon_motion.position.x - motion.position.x) / (float)samples;
					float dy = (salmon_motion.position.y - motion.position.y) / (float)samples;
					if (motion.position.x > (window_width_px / 3)) {
						motion.position.x += dx * step_seconds;
						motion.position.y += dy * step_seconds;
					}
					else {
						motion.position = motion.position + motion.velocity * step_seconds;
					}
					

					//
				}
				else {

					motion.position = motion.position + motion.velocity * step_seconds;
				}
				
			}
		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE UPDATES HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	for (Entity& entity : registry.physics.entities) {
		Motion& motion = registry.motions.get(entity);
		float dela = 1.0f * (elapsed_ms / 1000.f);
		float gravity = 50.1f;
		float water_force = 50.1f;
		float water_drag = 1.1f;
		

		if (expert_part) {
			float radius = sqrt(registry.bounces.get(entity).mass);
			if (motion.position.y >= window_height_px - radius) {
				registry.physics.get(entity).idle = true;
				motion.velocity.y = 0;
				motion.velocity.x = 0;
			}
			else {
				if (!registry.physics.get(entity).idle) {
					motion.velocity.y += gravity * dela;
				}
				motion.velocity -= motion.velocity * water_drag * dela;
				motion.velocity.x -= water_force * dela;
			}
			
		}
		else {
			motion.velocity.y += gravity * dela;
		}
	}
	// Check for collisions between all moving entities
    //ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		for(uint j = 0; j<motion_container.components.size(); j++) // i+1
		{
			if (i == j)
				continue;

			Motion& motion_j = motion_container.components[j];
			if (collides(motion_i, motion_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				//registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
	

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: HANDLE SALMON - WALL collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//ComponentContainer<Motion> &motion_container = registry.motions;
	for (uint i = 0; i < motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		bool isFish = registry.fishAI.has(entity_i);
		if (!isFish) {
			continue;
		}
		if ((motion_i.position.y - (abs(motion_i.scale.y) / 2) < 0.f) || (motion_i.position.y + (abs(motion_i.scale.y) / 2) > window_height_px)) {
			motion_i.velocity.y = -motion_i.velocity.y;
		}
	}
	// you may need the following quantities to compute wall positions
	(float)window_width_px; (float)window_height_px;

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A2: DRAW DEBUG INFO HERE on Salmon mesh collision
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 2
	// You will want to use the createLine from world_init.hpp
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Motion& motion_i = motion_container.components[0];
	Entity entity_i = motion_container.entities[0];
 
	float top = motion_i.scale.y / 2;
	float bottom = window_height_px - motion_i.scale.y / 2;
	if (motion_i.position.y <= top) {
		motion_i.position.y = top;
	}
	else if (motion_i.position.y >= bottom) {
		motion_i.position.y = bottom;
	}
	
	// debugging of bounding boxes
	if (debugging.in_debug_mode)
	{
		uint size_before_adding_new = (uint)motion_container.components.size();
		
		for (uint i = 0; i < size_before_adding_new; i++)
		{
			Motion& motion_i = motion_container.components[i];
			createBox(motion_i.position, motion_i.scale);
			Entity entity_i = motion_container.entities[i];

			bool isFish = registry.fishAI.has(entity_i);
			if (isFish) {
				FishAI& fish_i = registry.fishAI.get(entity_i);

				if (fish_i.move_up) {
					createLine(vec2(motion_i.position.x, motion_i.position.y - (mini_distance / 2)), vec2(3.5f, mini_distance));
				}
				else {
					createLine(vec2(motion_i.position.x, motion_i.position.y + (mini_distance / 2)), vec2(3.5f, mini_distance));
				}
				
				if (fish_i.avoiding && last_freeze_time > 0) {
					freeze = true;
					createBox(salmon_motion.position, vec2(2 * mini_distance, 2 * mini_distance));
				}
				
			}
			// visualize the radius with two axis-aligned lines
			/*const vec2 bonding_box = get_bounding_box(motion_i);
			float radius = sqrt(dot(bonding_box/2.f, bonding_box/2.f));
			vec2 line_scale1 = { motion_i.scale.x / 10, 2*radius };
			Entity line1 = createLine(motion_i.position, line_scale1);
			vec2 line_scale2 = { 2*radius, motion_i.scale.x / 10};
			Entity line2 = createLine(motion_i.position, line_scale2);*/

			// !!! TODO A2: implement debugging of bounding boxes and mesh


		}
	}

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// TODO A3: HANDLE PEBBLE collisions HERE
	// DON'T WORRY ABOUT THIS UNTIL ASSIGNMENT 3
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		Entity entity1 = collisionsRegistry.entities[i];
		Entity entity2 = collisionsRegistry.components[i].other;

		if (registry.bounces.has(entity1) && registry.bounces.has(entity2)) {
			Motion& motion1 = registry.motions.get(entity1);
			Motion& motion2 = registry.motions.get(entity2);

		

			registry.bounces.get(entity1).update(motion1, motion2, registry.bounces.get(entity2));
			

			vec2 dp = motion1.position - motion2.position;
			float dist = sqrt(dot(dp, dp));

			const vec2 other_bonding_box = get_bounding_box(motion1);
			const float other_r_radius = min(other_bonding_box.x, other_bonding_box.y) / 2.f;

			const vec2 my_bonding_box = get_bounding_box(motion2);
			const float my_r_radius = min(my_bonding_box.x, my_bonding_box.y) / 2.f;
	 
			float other_mass = registry.bounces.get(entity1).mass;
			float my_mass = registry.bounces.get(entity2).mass;

			vec2 gap = dp * ((other_r_radius + my_r_radius) / dist - 1);

			
			motion2.position += -gap * other_mass / (other_mass + my_mass);
			motion1.position += gap * my_mass / (other_mass + my_mass);
		}
	}
}