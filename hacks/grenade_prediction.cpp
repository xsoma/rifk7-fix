#pragma once

#include "c_esp.h"
#include "../utils/math.h"
#include "../sdk/c_model_info_client.h"
#include "../utils/c_config.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_view_beams.h"
#include "../sdk/c_debug_overlay.h"
#include "../utils/c_singleton.h"
#include "../sdk/c_cs_player.h"
#include "../sdk/c_engine_trace.h"
#include "c_animation_system.h"

c_nade_prediction g_nadepred;

void c_nade_prediction::predict(c_user_cmd *ucmd, c_cs_player* local) {
	//	readability
	constexpr float restitution = 0.45f;
	constexpr float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	constexpr float velocity = 403.0f * 0.9f;

	float step, gravity, new_velocity, unk01;
	int index{}, grenade_act{ 1 };
	c_vector3d pos, thrown_direction, start, eye_origin;
	c_vector3d angles, thrown;

	//	first time setup
	static auto sv_gravity = cvar()->find_var("sv_gravity");

	//	calculate step and actual gravity value
	gravity = sv_gravity->get_float() / 8.0f;
	step = global_vars_base->interval_per_tick;

	//	get local view and eye origin
	eye_origin = local->get_shoot_position();
	angles = ucmd->viewangles;

	//	copy current angles and normalise pitch
	thrown = angles;

	if (thrown.x < 0) {
		thrown.x = -10 + thrown.x * ((90 - 10) / 90.0f);
	}
	else {
		thrown.x = -10 + thrown.x * ((90 + 10) / 90.0f);
	}

	//	find out how we're throwing the grenade
	auto primary_attack = ucmd->buttons & c_user_cmd::attack;
	auto secondary_attack = ucmd->buttons & c_user_cmd::attack2;

	if (primary_attack && secondary_attack) {
		grenade_act = ACT_LOB;
	}
	else if (secondary_attack) {
		grenade_act = ACT_DROP;
	}

	//	apply 'magic' and modulate by velocity
	unk01 = power[grenade_act];

	unk01 = unk01 * 0.7f;
	unk01 = unk01 + 0.3f;

	new_velocity = velocity * unk01;

	//	here's where the fun begins
	math::angle_vectors(thrown, thrown_direction);

	start = eye_origin + thrown_direction * 16.0f;
	thrown_direction = (thrown_direction * new_velocity) + local->get_velocity();

	//	let's go ahead and predict
	for (auto time = 0.0f; index < 500; time += step) {
		pos = start + thrown_direction * step;

		//	setup trace
		game_trace t{};
		c_trace_filter filter;
		filter.skip_entity = local;

		ray r;
		r.init(start, pos);

		engine_trace()->trace_ray(r, mask_solid, &filter, &t);

		//	modify path if we have hit something
		if (t.fraction != 1.0f) {
			thrown_direction = t.plane.normal * -2.0f * thrown_direction.dot(t.plane.normal) + thrown_direction;

			thrown_direction *= restitution;

			pos = start + thrown_direction * t.fraction * step;

			time += (step * (1.0f - t.fraction));
		}

		const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
			client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

		//	check for detonation
		auto detonate = detonated(weapon, time, t);

		//	emplace nade point
		_points.at(index++) = c_nadepoint(start, pos, t.fraction != 1.0f, true, t.plane.normal, detonate);
		start = pos;

		//	apply gravity modifier
		thrown_direction.z -= gravity * t.fraction * step;

		if (detonate) {
			break;
		}
	}

	//	invalidate all empty points and finish prediction
	for (auto n = index; n < 500; ++n) {
		_points.at(n).m_valid = false;
	}

	_predicted = true;
}

bool c_nade_prediction::detonated(c_base_combat_weapon *weapon, float time, game_trace &trace) {
	if (!weapon) {
		return true;
	}

	//	get weapon item index
	const auto index = weapon->get_item_definition();

	switch (index) {
		//	flash and HE grenades only live up to 2.5s after thrown
	case 43:
	case 44:
		if (time > 2.5f) {
			return true;
		}
		break;

		//	fire grenades detonate on ground hit, or 3.5s after thrown
	case weapon_molotov:
	case 48:
		if (trace.fraction != 1.0f && trace.plane.normal.z > 0.7f || time > 3.5f) {
			return true;
		}
		break;

		//	decoy and smoke grenades were buggy in prediction, so i used this ghetto work-around
	case weapon_decoy:
	case 45:
		if (time > 2.5f) {
			return true;
		}
		break;
	}

	return false;
}

void c_nade_prediction::trace(c_user_cmd *ucmd, c_cs_player* local) {
	
	if (!(ucmd->buttons & c_user_cmd::attack) && !(ucmd->buttons & c_user_cmd::attack2)) {
		_predicted = false;
		return;
	}

	const static std::vector< int > nades{
		weapon_flashbang,
		weapon_smokegrenade,
		weapon_hegrenade,
		weapon_molotov,
		weapon_decoy,
		weapon_incgrenade
	};

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon) {
		return;
	}

	if (std::find(nades.begin(), nades.end(), weapon->get_item_definition()) != nades.end()) {
		return predict(ucmd, local);
	}

	_predicted = false;
}

void c_nade_prediction::draw() 
{
	if (!config.esp.nade_pred)
		return;

	if (!engine_client()->is_ingame())
		return;

	c_vector2d start, end;

	//	draw nade path
	if (_predicted) {
		for (auto &p : _points) {
			if (!p.m_valid) {
				break;
			}

			if (!renderer->screen_transform(p.m_start, start, c_esp::info.world_to_screen_matrix))
				return;

			if (!renderer->screen_transform(p.m_end, end, c_esp::info.world_to_screen_matrix))
				return;

			c_color c = config.esp.nade_color;

			switch (config.esp.nade_type) {
			case 0:
				renderer->line(c_vector2d(start.x, start.y), c_vector2d(end.x, end.y), c);
				break;
			case 1:
				renderer->line(c_vector2d(start.x, start.y), c_vector2d(end.x, end.y), c);
				break;
			case 2:
				renderer->line(c_vector2d(start.x, start.y), c_vector2d(end.x, end.y), c);
				if (p.m_plane)
					renderer->rect(c_vector2d(start.x - 2, start.y - 2), c_vector2d(5, 5), c);
				if (p.m_detonate) {
					c_vector3d forward;
					math::angle_vectors(engine_client()->get_view_angles(), forward);
					const auto transform = math::rotation_matrix(forward);
					renderer->ball(p.m_end, 10.f, transform, c, c_esp::info.world_to_screen_matrix);
				}
				break;
			}
		}
	}
}