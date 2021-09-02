#include "c_resolver.h"
#include "../utils/math.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"
#include "../sdk/c_client_entity_list.h"
#include "../sdk/c_weapon_system.h"
#include <random>
#include "c_esp.h"

static std::random_device rd;
static std::mt19937 rng(rd());
float normalize_yaw_180(float yaw)
{
	if (yaw > 180)
		yaw -= (round(yaw / 360) * 360.f);
	else if (yaw < -180)
		yaw += (round(yaw / 360) * -360.f);

	return yaw;
}
float approach(float cur, float target, float inc) {
	inc = abs(inc);

	if (cur < target)
		return std::min(cur + inc, target);
	if (cur > target)
		return std::max(cur - inc, target);

	return target;
}
float angle_difference(float a, float b) {
	auto diff = normalize_yaw_180(a - b);

	if (diff < 180)
		return diff;
	return diff - 360;
}
float approach_angle(float cur, float target, float inc) {
	auto diff = angle_difference(target, cur);
	return approach(cur, cur + diff, inc);
}
void c_resolver::resolve(c_animation_system::animation* anim)
{
	const auto info = animation_system->get_animation_info(anim->player);
	if (!info)
		return;
	auto resolver_yaw = 0.f;
	float base_yaw = anim->eye_angles.y;
	info->stage = resolver_1;
	if (anim->lag <= 1 || anim->shot || !anim->has_anim_state || anim->player->get_info().fakeplayer)
		return;
	resolver_yaw = base_yaw;
	if (config.rage.override_resolver_key && GetAsyncKeyState(config.rage.override_resolver_key))
	{
		c_vector2d abs_origin2d;
		if (renderer->screen_transform(anim->abs_origin, abs_origin2d, renderer->world_to_screen_matrix())) {
			float delta = renderer->get_center().x - abs_origin2d.x;
			if (delta > 0)
			{
				resolver_yaw = base_yaw - *reinterpret_cast<float*>(&anim->anim_state.pad10[516]);
				info->stage = resolver_5;
			}
			else if (delta < 0)
			{
				resolver_yaw = base_yaw + *reinterpret_cast<float*>(&anim->anim_state.pad10[516]);
				info->stage = resolver_6;
			}
		}
	}
	else
	{
		auto freestanding_yaw = 0.f;
		const auto local = c_cs_player::get_local_player();
		const auto plus_desync = base_yaw + *reinterpret_cast<float*>(&anim->anim_state.pad10[516]);
		const auto minus_desync = base_yaw - *reinterpret_cast<float*>(&anim->anim_state.pad10[516]);

		get_target_freestand_yaw(local, &freestanding_yaw);

		const auto diff_from_plus_desync = fabs(math::angle_diff(freestanding_yaw, plus_desync));
		const auto diff_from_minus_desync = fabs(math::angle_diff(freestanding_yaw, minus_desync));

		const auto first_yaw = diff_from_plus_desync < diff_from_minus_desync ? plus_desync : minus_desync;
		const auto second_yaw = diff_from_plus_desync < diff_from_minus_desync ? minus_desync : plus_desync;
		const auto third_yaw = math::calc_angle(local->get_shoot_position(),
			anim->player->get_shoot_position()).y;

		switch (info->missed_due_to_resolver % 3)
		{
		case 0:
			resolver_yaw = first_yaw;
			info->stage = resolver_2;
			break;
		case 1:
			resolver_yaw = second_yaw;
			info->stage = resolver_3;
			break;
		case 2:
			resolver_yaw = third_yaw;
			info->stage = resolver_4;
			break;
		default:
			break;
		}
	}
	anim->anim_state.goal_feet_yaw = math::normalize_yaw(resolver_yaw);
}

bool c_resolver::get_target_freestand_yaw(c_cs_player* target, float* yaw)
{
	float dmg_left = 0.f;
	float dmg_right = 0.f;

	static auto get_rotated_pos = [](c_vector3d start, float rotation, float distance)
	{
		float rad = deg2rad(rotation);
		start.x += cos(rad) * distance;
		start.y += sin(rad) * distance;

		return start;
	};

	const auto local = c_cs_player::get_local_player();

	if (!local || !target || !local->is_alive())
		return false;

	c_vector3d local_eye_pos = local->get_shoot_position();
	c_vector3d eye_pos = target->get_shoot_position();
	c_qangle angle = math::calc_angle(local_eye_pos, eye_pos);

	auto backwards = angle.y;

	c_vector3d pos_left = get_rotated_pos(eye_pos, angle.y + 90.f, 40.f);
	c_vector3d pos_right = get_rotated_pos(eye_pos, angle.y + 90.f, -40.f);

	const auto wall_left = trace_system->wall_penetration(local_eye_pos, pos_left,
		nullptr, local);

	const auto wall_right = trace_system->wall_penetration(local_eye_pos, pos_right,
		nullptr, local);

	if (wall_left.has_value())
		dmg_left = wall_left.value().damage;

	if (wall_right.has_value())
		dmg_right = wall_right.value().damage;

	// we can hit both sides, lets force backwards
	if (fabsf(dmg_left - dmg_right) < 10.f)
	{
		*yaw = backwards;
		return false;
	}

	bool direction = dmg_left > dmg_right;
	*yaw = direction ? angle.y - 90.f : angle.y + 90.f;

	return true;
}

float c_resolver::get_max_desync_delta(c_cs_player* player)
{
	if (!player || !player->is_alive() || player->is_dormant_virt())
		return 0.f;

	auto animstate = uintptr_t(player->get_anim_state());
	if (!animstate)
		return 0.f;

	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = std::max<float>(0, std::min(*reinterpret_cast<float*>(animstate + 0xF8), 1.f));
	float speedfactor = speedfraction >= 0.f ? std::max<float>(0, std::min(1.f, *reinterpret_cast<float*> (animstate + 0xFC))) : speedfactor = 0.f;

	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;

	if (duckammount > 0)
		unk2 += +((duckammount * speedfactor) * (0.5f - unk2));

	auto retvalue = *(float*)(animstate + 0x334) * unk2;

	if (retvalue > 1.f)
		retvalue -= 1.f;

	retvalue = std::clamp(retvalue, 28.f, 60.f);
	return retvalue;
}
void c_resolver::resolve_shot(resolver::shot& shot)
{
	if (!config.rage.enabled || shot.manual)
		return;

	const auto info = animation_system->get_animation_info(shot.record.player);

	if (!info)
		return;

	const auto hdr = model_info_client()->get_studio_model(shot.record.player->get_model());
	if (!hdr)
		return;

	if (shot.server_info.damage > 0)
	{
		static const auto hit_msg = __("Hit %s in %s for %d damage [history ticks: %i] | stage %i.");
		_rt(hit, hit_msg);
		char msg[255];
		switch (shot.server_info.hitgroup)
		{
		case hitgroup_head:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("head"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_chest:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("chest"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_stomach:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("stomach"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_leftarm:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("left arm"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_rightarm:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("right arm"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_leftleg:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("left leg"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		case hitgroup_rightleg:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("right leg"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		default:
			sprintf_s(msg, hit, shot.record.player->get_info().name, _("unknown"), shot.server_info.damage, std::max(0, prediction_system->original_cmd.tick_count - shot.tickcount), info->stage);
			break;
		}
		logging->info(msg);
	}
	else
	{
		const auto spread_miss = !c_aimhelper::can_hit_hitbox(shot.start, shot.server_info.impacts.back(), &shot.record, hdr, shot.hitbox);
		if (spread_miss)
		{
			logging->info(_("Missed shot due to spread."));
			++info->missed_due_to_spread;
		}
		else
		{
			char msg[255];
			static const auto miss_msg = __("Missed shot due to resolver | stage %i.");
			_rt(miss, miss_msg);
			sprintf_s(msg, miss, info->stage);
			logging->info(msg);
			++info->missed_due_to_resolver;
		}
	}
	if (!shot.record.player->is_alive() || shot.record.player->get_info().fakeplayer
		|| !shot.record.has_anim_state || !info)
		return;

	// check deviation from server.
	shot.record.apply(shot.record.player);

	if (info->missed_due_to_resolver > 5)
		info->missed_due_to_resolver = 0;

	// apply changes.
	if (!info->frames.empty())
	{
		c_animation_system::animation* previous = nullptr;
		// jump back to the beginning.
		*shot.record.player->get_anim_state() = info->frames.back().anim_state;

		for (auto it = info->frames.rbegin(); it != info->frames.rend(); ++it)
		{
			auto& frame = *it;
			const auto frame_player = reinterpret_cast<c_cs_player*>(
				client_entity_list()->get_client_entity(frame.index));

			if (frame_player == frame.player
				&& frame.player == shot.record.player)
			{
				// re-run complete animation code and repredict all animations in between!
				frame.anim_state = *shot.record.player->get_anim_state();
				frame.apply(shot.record.player);
				shot.record.player->get_flags() = frame.flags;
				*shot.record.player->get_animation_layers() = frame.layers;
				shot.record.player->get_simtime() = frame.sim_time;

				info->update_animations(&frame, previous);
				frame.abs_ang.y = shot.record.player->get_anim_state()->goal_feet_yaw;
				frame.flags = shot.record.player->get_flags();
				*shot.record.player->get_animation_layers() = frame.layers;
				frame.build_server_bones(shot.record.player);
				previous = &frame;
			}
		}
	}
}

void c_resolver::register_shot(resolver::shot&& s)
{
	shots.emplace_front(std::move(s));
}

void c_resolver::on_player_hurt(c_game_event* event)
{
	const auto attacker = event->get_int(_("attacker"));
	const auto attacker_index = engine_client()->get_player_for_user_id(attacker);

	if (attacker_index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		last_confirmed = &*it;
		break;
	}

	if (!last_confirmed)
		return;

	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != last_confirmed->record.index)
		return;

	last_confirmed->server_info.index = index;
	last_confirmed->server_info.damage = event->get_int(_("dmg_health"));
	last_confirmed->server_info.hitgroup = event->get_int(_("hitgroup"));
}

void c_resolver::on_bullet_impact(c_game_event* event)
{
	const auto userid = event->get_int(_("userid"));
	const auto index = engine_client()->get_player_for_user_id(userid);

	if (index != engine_client()->get_local_player())
		return;

	if (shots.empty())
		return;

	resolver::shot* last_confirmed = nullptr;

	for (auto it = shots.rbegin(); it != shots.rend(); it = next(it))
	{
		last_confirmed = &*it;
		break;
	}

	if (!last_confirmed)
		return;

	last_confirmed->impacted = true;
	last_confirmed->server_info.impacts.emplace_back(event->get_float(_("x")),
		event->get_float(_("y")),
		event->get_float(_("z")));
}

void c_resolver::on_render_start()
{
	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->time + 1.f < global_vars_base->curtime)
			it = shots.erase(it);
		else
			it = next(it);
	}

	for (auto it = shots.begin(); it != shots.end();)
	{
		if (it->impacted)
		{
			resolve_shot(*it);
			c_esp::draw_local_impact(it->start, it->server_info.impacts.back());
			it = shots.erase(it);
		}
		else
			it = next(it);
	}
}