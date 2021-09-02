#include "c_ragebot.h"
#include "c_aimhelper.h"
#include "c_trace_system.h"
#include "../utils/math.h"
#include "../sdk/c_weapon_system.h"
#include "../sdk/c_debug_overlay.h"
#include "c_prediction_system.h"
#include "c_antiaim.h"
#include "c_resolver.h"
#include <algorithm>
void c_ragebot::aim(c_cs_player* local, c_user_cmd* cmd, bool& send_packet)
{
	last_pitch = std::nullopt;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
	if (!weapon)
		return;

	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());
	if (!wpn_info)
		return;

	if (!local->can_shoot(cmd, global_vars_base->curtime))
		return;

	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!weapon_cfg.has_value())
		return;

	std::vector<aim_info> hitpoints = {};

	client_entity_list()->for_each_player([&](c_cs_player* player) -> void
	{
		if (!player->is_enemy() || !player->is_alive() || player->get_gun_game_immunity())
			return;

		const auto latest = animation_system->get_latest_animation(player);

		if (!latest.has_value())
			return;

		const auto oldest = animation_system->get_oldest_animation(player);

		const auto rtt = 2.f * net_channel->get_latency(flow_outgoing);
		const auto breaking_lagcomp = latest.value()->lag && latest.value()->lag <= 17 && is_breaking_lagcomp(latest.value());
		const auto can_delay_shot = (latest.value()->lag > time_to_ticks(rtt) + global_vars_base->interval_per_tick);
		const auto delay_shot = (time_to_ticks(rtt) + time_to_ticks(global_vars_base->curtime - latest.value()->sim_time)
			+ global_vars_base->interval_per_tick >= latest.value()->lag);

		if (breaking_lagcomp && delay_shot && can_delay_shot)
			return;

		if (breaking_lagcomp && !can_delay_shot)
			return;

		std::optional<aim_info> aimbot_info;
		if (!aimbot_info.has_value())
		{
			if (oldest.has_value())
			{
				const auto alternative = scan_record(local, oldest.value());
				if (alternative.has_value())
					aimbot_info = alternative;
			}
			else
			{
				const auto alternative = scan_record(local, latest.value());
				if (alternative.has_value())
					aimbot_info = alternative;
			}
		}
		else
		{
			if (oldest.has_value())
			{
				const auto alternative = scan_record(local, oldest.value());
				if (alternative.has_value() && aimbot_info.value().damage < alternative.value().damage)
					aimbot_info = alternative;
			}
			else
			{
				const auto alternative = scan_record(local, latest.value());
				if (alternative.has_value() && aimbot_info.value().damage < alternative.value().damage)
					aimbot_info = alternative;
			}
		}

		if (aimbot_info.has_value())
			hitpoints.push_back(aimbot_info.value());
	});

	aim_info best_match = { c_vector3d(), -FLT_MAX, nullptr, false, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };

	// find best target spot of all valid spots.
	for (auto& hitpoint : hitpoints)
		if (hitpoint.damage > best_match.damage)
			best_match = hitpoint;

	// stop if no target found.
	if (best_match.damage < 0.f)
		return;

	// run autostop.
	autostop(local, cmd);

	// scope the weapon.
	if ((wpn_info->get_weapon_id() == weapon_g3sg1 || wpn_info->get_weapon_id() == weapon_scar20
		|| wpn_info->get_weapon_id() == weapon_ssg08 || wpn_info->get_weapon_id() == weapon_awp
		|| wpn_info->get_weapon_id() == weapon_sg556 || wpn_info->get_weapon_id() == weapon_aug) && weapon->get_zoom_level() == 0)
		cmd->buttons |= c_user_cmd::flags::attack2;

	// calculate angle.
	auto angle = math::calc_angle(local->get_shoot_position(), best_match.position);
	math::normalize(angle);
	// store pitch for eye correction.
	last_pitch = angle.x;

	// select final aimpoint.
	if (c_aimhelper::can_hit(local, best_match.animation, best_match.position, weapon_cfg.value().hitchance / 100.f, best_match.hitbox))
	{
		// store shot info for resolver.
		if (!best_match.alt_attack)
		{
			resolver::shot shot{};
			shot.damage = best_match.damage;
			shot.start = local->get_shoot_position();
			shot.end = best_match.position;
			shot.hitgroup = best_match.hitgroup;
			shot.hitbox = best_match.hitbox;
			shot.time = global_vars_base->curtime;
			shot.record = *best_match.animation;
			shot.tickcount = time_to_ticks(best_match.animation->sim_time + calculate_lerp());
			shot.manual = false;
			c_resolver::register_shot(std::move(shot));
		}

		// set correct information to user_cmd.
		cmd->viewangles = angle;
		cmd->tick_count = time_to_ticks(best_match.animation->sim_time + calculate_lerp());
		cmd->buttons |= best_match.alt_attack ? c_user_cmd::attack2 : c_user_cmd::attack;
	}
	else
	{
		// scope the weapon.
		if ((wpn_info->get_weapon_id() == weapon_g3sg1 || wpn_info->get_weapon_id() == weapon_scar20
			|| wpn_info->get_weapon_id() == weapon_ssg08 || wpn_info->get_weapon_id() == weapon_awp
			|| wpn_info->get_weapon_id() == weapon_sg556 || wpn_info->get_weapon_id() == weapon_aug) && weapon->get_zoom_level() == 0)
			cmd->buttons |= c_user_cmd::flags::attack2;
		return;
	}
}

void c_ragebot::autostop(c_cs_player* local, c_user_cmd* cmd)
{
	if (cmd->buttons & c_user_cmd::jump)
		return;

	static const auto nospread = cvar()->find_var(_("weapon_accuracy_nospread"));

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (nospread->get_int() || !local->is_on_ground() ||
		(weapon && weapon->get_item_definition() == weapon_taser) && local->is_on_ground())
		return;

	const auto wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!wpn_info)
		return;

	auto& info = get_autostop_info();

	if (info.call_time == global_vars_base->curtime)
	{
		info.did_stop = true;
		return;
	}

	info.did_stop = false;
	info.call_time = global_vars_base->curtime;

	if (local->get_velocity().length2d() <= wpn_info->get_standing_accuracy(weapon))
		return;
	else
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
		prediction_system->repredict(local, cmd);
		if (config.rage.slow_walk && GetAsyncKeyState(config.rage.slow_walk))
		{
			info.did_stop = true;
			return;
		}

		if (local->get_velocity().length2d() <= wpn_info->get_standing_accuracy(weapon))
			return;
	}

	c_qangle dir;
	math::vector_angles(prediction_system->unpredicted_velocity, dir);
	const auto angles = engine_client()->get_view_angles();
	dir.y = angles.y - dir.y;

	c_vector3d move;
	math::angle_vectors(dir, move);

	if (prediction_system->unpredicted_velocity.length2d() > .1f)
		move *= -math::forward_bounds / std::max(std::abs(move.x), std::abs(move.y));

	cmd->forwardmove = move.x;
	cmd->sidemove = move.y;

	const auto backup = cmd->viewangles;
	cmd->viewangles = angles;
	prediction_system->repredict(local, cmd);
	cmd->viewangles = backup;
	if (local->get_velocity().length2d() > prediction_system->unpredicted_velocity.length2d())
	{
		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;
	}
	prediction_system->repredict(local, cmd);
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record(c_cs_player* local, c_animation_system::animation* animation)
{
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return std::nullopt;

	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!info)
		return std::nullopt;

	const auto is_zeus = weapon->get_item_definition() == weapon_taser;
	const auto is_knife = !is_zeus && info->WeaponType == weapontype_knife;

	if (is_knife)
		return scan_record_knife(local, animation);

	return scan_record_gun(local, animation);
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record_knife(c_cs_player* local, c_animation_system::animation* animation)
{
	static const auto is_behind = [](c_cs_player* local, c_animation_system::animation* animation) -> bool
	{
		auto vec_los = animation->origin - local->get_origin();
		vec_los.z = 0.0f;

		c_vector3d forward;
		math::angle_vectors(animation->eye_angles, forward);
		forward.z = 0.0f;

		return vec_los.normalize().dot(forward) > 0.475f;
	};

	static const auto should_stab = [](c_cs_player* local, c_animation_system::animation* animation) -> bool
	{
		struct table_t
		{
			unsigned char swing[2][2][2];
			unsigned char stab[2][2];
		};

		static const table_t table = {
			{
				{
					{ 25, 90 },
					{ 21, 76 }
				},
				{
					{ 40, 90 },
					{ 34, 76 }
				}
			},
			{
				{ 65, 180 },
				{ 55, 153 }
			}
		};

		const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
			client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

		if (!weapon)
			return false;

		const auto has_armor = animation->player->get_armor() > 0;
		const auto first_swing = weapon->get_next_primary_attack() + 0.4f < global_vars_base->curtime;
		const auto behind = is_behind(local, animation);

		const int stab_dmg = table.stab[has_armor][behind];
		const int slash_dmg = table.swing[false][has_armor][behind];
		const int swing_dmg = table.swing[first_swing][has_armor][behind];

		if (animation->player->get_health() <= swing_dmg)
			return false;

		if (animation->player->get_health() <= stab_dmg)
			return true;

		if (animation->player->get_health() > swing_dmg + slash_dmg + stab_dmg)
			return true;

		return false;
	};

	const auto studio_model = model_info_client()->get_studio_model(animation->player->get_model());

	if (!studio_model)
		return std::nullopt;

	const auto stab = should_stab(local, animation);
	const auto range = stab ? 32.0f : 48.0f;
	game_trace tr;
	auto spot = animation->player->get_hitbox_position(c_cs_player::hitbox::upper_chest, animation->bones);
	const auto hitbox = studio_model->get_hitbox(static_cast<uint32_t>(c_cs_player::hitbox::upper_chest), 0);

	if (!spot.has_value() || !hitbox)
		return std::nullopt;

	c_vector3d forward;
	const auto calc = math::calc_angle(local->get_shoot_position(), spot.value());
	math::angle_vectors(calc, forward);

	spot.value() += forward * hitbox->radius;

	c_trace_system::run_emulated(animation, [&]() -> void
	{
		uint32_t filter[4] = { c_engine_trace::get_filter_simple_vtable(), reinterpret_cast<uint32_t>(local), 0, 0 };

		ray r;
		c_vector3d aim;
		const auto angle = math::calc_angle(local->get_shoot_position(), spot.value());
		math::angle_vectors(angle, aim);
		const auto end = local->get_shoot_position() + aim * range;
		r.init(local->get_shoot_position(), end);

		engine_trace()->trace_ray(r, mask_solid, reinterpret_cast<trace_filter*>(filter), &tr);

		if (tr.fraction >= 1.0f)
		{
			const c_vector3d min(-16.f, -16.f, -18.f);
			const c_vector3d max(16.f, 16.f, 18.f);
			r.init(local->get_shoot_position(), end, min, max);
			engine_trace()->trace_ray(r, mask_solid, reinterpret_cast<trace_filter*>(filter), &tr);
		}
	});

	if (tr.entity != animation->player)
		return std::nullopt;

	return aim_info{ tr.endpos, 100.f, animation, stab, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };
}

bool c_ragebot::is_breaking_lagcomp(c_animation_system::animation* animation)
{
	static constexpr auto teleport_dist = 64 * 64;

	const auto info = animation_system->get_animation_info(animation->player);

	if (!info || info->frames.size() < 2)
		return false;

	if (info->frames[0].dormant)
		return false;

	auto prev_org = info->frames[0].origin;
	auto skip_first = true;

	// walk context looking for any invalidating event
	for (auto& record : info->frames)
	{
		if (skip_first)
		{
			skip_first = false;
			continue;
		}

		if (record.dormant)
			break;

		auto delta = record.origin - prev_org;
		if (delta.length2dsqr() > teleport_dist)
		{
			// lost track, too much difference
			return true;
		}

		// did we find a context smaller than target time?
		if (record.sim_time <= animation->sim_time)
			break; // hurra, stop

		prev_org = record.origin;
	}

	return false;
}

std::optional<c_ragebot::aim_info> c_ragebot::scan_record_gun(c_cs_player* local, c_animation_system::animation* animation, std::optional<c_vector3d> pos)
{
	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (!animation || !animation->player || !weapon_cfg.has_value())
		return std::nullopt;

	const auto info = animation_system->get_animation_info(animation->player);

	if (!info)
		return std::nullopt;

	const auto cfg = weapon_cfg.value();

	auto should_baim = false;

	if (config.rage.override_min_damage_key && GetAsyncKeyState(config.rage.override_min_damage_key))
	{
		last_min_dmg = cfg.override_min_damage;
	}
	else
	{
		last_min_dmg = cfg.min_dmg;
	}

	for (const auto& hitbox : c_cs_player::hitboxes_baim)
	{
		const auto center = animation->player->get_hitbox_position(hitbox, const_cast<matrix3x4*>(animation->bones));
		if (center.has_value() && !should_baim)
		{
			const auto center_wall = trace_system->wall_penetration(local->get_shoot_position(),
				center.value(), animation);

			if (center_wall.has_value() && center_wall.value().hitbox == hitbox)
			{
				if (center_wall.value().damage > static_cast<float>(animation->player->get_health()))
				{
					should_baim = true;
				}
			}
		}
	}
	if (config.rage.prevent_baim_key && GetAsyncKeyState(config.rage.prevent_baim_key))
	{
		should_baim = true;
	}
	if (animation->player->get_info().fakeplayer || animation->shot)
	{
		should_baim = false;
	}

	aim_info best_match = { c_vector3d(), -FLT_MAX, nullptr, false, c_vector3d(), 0.f, 0.f, c_cs_player::hitbox::head, 0 };

	const auto scan_box = [&](c_cs_player::hitbox hitbox)
	{
		auto box = animation->player->get_hitbox_position(hitbox, const_cast<matrix3x4*>(animation->bones));
		if (!box.has_value())
			return;

		auto scale_hitbox = .2f;

		switch (hitbox)
		{
		case c_cs_player::hitbox::head:
			scale_hitbox = cfg.head_scale / 100.f;
			break;
		case c_cs_player::hitbox::pelvis:
			scale_hitbox = cfg.body_scale / 100.f;
			break;
		case c_cs_player::hitbox::body:
			scale_hitbox = cfg.body_scale / 100.f;
			break;
		case c_cs_player::hitbox::thorax:
			scale_hitbox = cfg.body_scale / 100.f;
			break;
		case c_cs_player::hitbox::upper_chest:
			scale_hitbox = cfg.body_scale / 100.f;
			break;
		}
		auto points = c_aimhelper::select_multipoint(animation, hitbox, hitgroup_head, scale_hitbox);
		points.emplace_back(box.value(), 0.f, animation, false, box.value(), 0.f, 0.f, hitbox, hitgroup_head);
		
		for (auto& point : points)
		{
			const auto wall = trace_system->wall_penetration(pos.value_or(local->get_shoot_position()),
				point.position, animation);

			if (!wall.has_value())
				continue;

			if (hitbox != wall.value().hitbox)
				continue;

			point.hitgroup = wall.value().hitgroup;
			point.damage = wall.value().damage;

			if (point.damage > best_match.damage)
				best_match = point;
		}
	};
	if (config.rage.force_onshot_key && GetAsyncKeyState(config.rage.force_onshot_key))
	{
		if (animation->shot)
		{
			for (const auto& hitbox : c_cs_player::hitboxes_aiming)
			{
				scan_box(hitbox);
			}
		}
	}
	else
	{
		if (should_baim)
			for (const auto& hitbox : c_cs_player::hitboxes_baim)
				scan_box(hitbox);
		else
			for (const auto& hitbox : c_cs_player::hitboxes_aiming)
				scan_box(hitbox);
	}

	if (best_match.damage > std::min(last_min_dmg, (float)animation->player->get_health()))
		return best_match;
	else
		return std::nullopt;
}

c_ragebot::autostop_info& c_ragebot::get_autostop_info()
{
	static autostop_info info{ -FLT_MAX, false };
	return info;
}