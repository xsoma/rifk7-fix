#include "c_antiaim.h"
#include "../utils/math.h"
#include "../sdk/c_game_rules.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_debug_overlay.h"
#include "../sdk/c_input.h"
#include "../sdk/c_prediction.h"
#include "../sdk/c_weapon_system.h"
#include "../menu/c_menu.h"
#include "c_aimhelper.h"
#include "c_miscellaneous.h"
#include "c_ragebot.h"
#include "c_trace_system.h"
#include "c_prediction_system.h"
#include "../hooks/c_events.h"
#include <random>

static std::random_device rd;
static std::mt19937 rng(rd());

c_antiaim::c_antiaim()
{
	hotkeys->register_callback([&](const uint32_t code) -> void
		{
			if (code != 0 && code == config.rage.experimental_aa_flip && config.rage.experimental_aa_flip && !menu->is_open())
				side = !side;
		});
}

void c_antiaim::fakelag(c_cs_player* local, c_user_cmd* cmd, bool& send_packet)
{
	auto target_standing = game_rules->is_valve_ds() ? std::min(6, (int)config.rage.fakelag_standing) : (int)config.rage.fakelag_standing;
	auto target_moving = game_rules->is_valve_ds() ? std::min(6, (int)config.rage.fakelag_moving) : (int)config.rage.fakelag_moving;
	auto target_air = game_rules->is_valve_ds() ? std::min(6, (int)config.rage.fakelag_in_air) : (int)config.rage.fakelag_in_air;
	auto target_fakeland = game_rules->is_valve_ds() ? std::min(6, (int)config.rage.fakelag_fakeland) : (int)config.rage.fakelag_fakeland;
	static auto fakeland = false;

	if (!local || !local->is_alive() || !cmd || game_rules->is_freeze_period())
		return;

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	if (!weapon)
		return;

	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!info)
		return;

	static auto choke_amount = 1.f;
	if (!local->is_on_ground())
		choke_amount = target_air;
	else
	{
		if (local->get_velocity().length2d() > .1f)
			choke_amount = target_moving;
		else
			choke_amount = target_standing;
	}

	// fake land to desync animations for pasted cheats.
	auto origin = local->get_origin();
	auto velocity = local->get_velocity();
	auto flags = local->get_flags();

	c_trace_system::extrapolate(local, origin, velocity, flags, prediction_system->unpredicted_flags & c_base_player::on_ground);

	if (flags & c_base_player::flags::on_ground && !(local->get_flags() & c_base_player::flags::on_ground))
	{
		fakeland = fabsf(math::normalize_yaw(local->get_lby() - cmd->viewangles.y)) > 35.f;
		if (fakeland)
		{
			choke_amount = target_fakeland;
		}
	}
	else if (local->get_flags() & c_base_player::flags::on_ground && fakeland)
	{
		fakeland = false;
	}

	if (config.rage.fakelag_disable_knife && info->WeaponType == weapontype_knife)
	{
		choke_amount = 1;
	}
	if (config.rage.fakelag_disable_taser && weapon->get_item_definition() == weapon_taser)
	{
		choke_amount = 1;
	}
	if (config.rage.fakelag_disable_revolver && weapon->get_item_definition() == weapon_revolver)
	{
		choke_amount = 1;
	}
	// set send packet and stats.
	send_packet = client_state->choked_commands >= choke_amount;
}

void c_antiaim::run(c_cs_player* local, c_user_cmd* cmd, bool& send_packet, int sequence)
{
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

	if (!weapon || !info || cmd->buttons & c_user_cmd::flags::use ||
		local->get_move_type() == c_base_player::movetype_observer ||
		local->get_move_type() == c_base_player::movetype_noclip ||
		local->get_move_type() == c_base_player::movetype_ladder ||
		local->get_flags() & c_base_player::frozen ||
		game_rules->is_freeze_period())
	{
		local->get_pose_parameter() = animation_system->local_animation.poses;
		*local->get_animation_layers() = animation_system->server_layers;
		animation_system->local_animation.eye_angles = cmd->viewangles;
		math::normalize(animation_system->local_animation.eye_angles);
		local->get_anim_state()->feet_yaw_rate = 0.f;
		animation_system->update_player(local);
		animation_system->local_animation.abs_ang.y = local->get_anim_state()->goal_feet_yaw;
		animation_system->local_animation.layers = *local->get_animation_layers();
		animation_system->local_animation.poses = local->get_pose_parameter();
		return;
	}
	*local->get_animation_layers() = animation_system->server_layers;
	if (cmd->command_number == shot_cmd)
	{
		cmd->viewangles = input->commands[shot_cmd % 150].viewangles - local->get_punch_angle() * cvar()->find_var(_("weapon_recoil_scale"))->get_float();
	}
	else
	{
		cmd->viewangles.x = 89.f;
		cmd->viewangles.y += 180.f;
		cmd->viewangles.y += side == 1 ? -config.rage.body_lean : config.rage.inv_body_lean;
		if (config.rage.experimental_aa)
		{
			if (!send_packet)
				if (next_lby_update <= global_vars_base->curtime) {
					cmd->viewangles.y -= side * 120.f;
				}
				else
					cmd->viewangles.y += side * 120.f;
		}
		else
		{
			if (!(cmd->buttons & c_user_cmd::flags::forward || cmd->buttons & c_user_cmd::flags::back || cmd->buttons & c_user_cmd::flags::move_left || cmd->buttons & c_user_cmd::flags::move_right) && local->get_flags() & c_base_player::on_ground)
			{
				cmd->forwardmove = cmd->command_number % 2 ? local->get_flags() & c_base_player::ducking ? 2.94117647f : .505f : local->get_flags() & c_base_player::ducking ? -2.94117647f : -.505f;
			}
			if (!send_packet)
				cmd->viewangles.y += side * 120.f;
		}
	}
	math::normalize(cmd->viewangles);
	local->get_anim_state()->feet_yaw_rate = 0.f;
	animation_system->local_animation.eye_angles = antiaim->last_real;
	animation_system->update_player(local);
	animation_system->local_animation.abs_ang.y = local->get_anim_state()->goal_feet_yaw;
	animation_system->local_animation.layers = animation_system->server_layers;
	animation_system->local_animation.poses = local->get_pose_parameter();
}

void c_antiaim::prepare_animation(c_cs_player* local)
{
	const auto state = local->get_anim_state();

	*local->get_animation_layers() = animation_system->local_animation.layers;
	local->get_pose_parameter() = animation_system->local_animation.poses;

	if (local->get_move_type() == c_base_player::movetype_observer ||
		local->get_move_type() == c_base_player::movetype_noclip ||
		local->get_move_type() == c_base_player::movetype_ladder ||
		local->get_flags() & c_base_player::frozen ||
		game_rules->is_freeze_period() ||
		!c_events::round_flags == ROUND_IN_PROGRESS ||
		!state ||
		!config.rage.enabled)
		return;

	local->get_animation_layers()->at(7).weight = 0.f;
}

void c_antiaim::predict(c_cs_player* local, c_user_cmd* cmd)
{
	const auto state = local->get_anim_state();

	if (!local->is_local_player() || !state)
		return;

	if (state->speed > .1)
	{
		next_lby_update = global_vars_base->curtime + .22f;
	}

	if (next_lby_update <= global_vars_base->curtime)
	{
		lby_update = global_vars_base->curtime;
		next_lby_update = global_vars_base->curtime + 1.1f;
	}

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	min_delta = *reinterpret_cast<float*>(&state->pad10[512]);
	max_delta = *reinterpret_cast<float*>(&state->pad10[516]);

	float max_speed = 260.f;

	if (weapon)
	{
		const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());

		if (info)
			max_speed = std::max(.001f, info->flMaxPlayerSpeed);
	}

	auto velocity = local->get_velocity();
	const auto abs_velocity_length = powf(velocity.length(), 2.f);
	const auto fraction = 1.f / (abs_velocity_length + .00000011920929f);

	if (abs_velocity_length > 97344.008f)
		velocity *= velocity * 312.f;

	auto speed = velocity.length();

	if (speed >= 260.f)
		speed = 260.f;

	feet_speed_stand = (1.923077f / max_speed) * speed;
	feet_speed_ducked = (2.9411764f / max_speed) * speed;

	auto feet_speed = (((stop_to_full_running_fraction * -.3f) - .2f) * std::clamp(feet_speed_stand, 0.f, 1.f)) + 1.f;

	if (state->duck_amount > 0.f)
		feet_speed = feet_speed + ((std::clamp(feet_speed_ducked, 0.f, 1.f) * state->duck_amount) * (.5f - feet_speed));

	min_delta *= feet_speed;
	max_delta *= feet_speed;

	if (stop_to_full_running_fraction > 0.0 && stop_to_full_running_fraction < 1.0)
	{
		const auto interval = global_vars_base->interval_per_tick * 2.f;

		if (is_standing)
			stop_to_full_running_fraction = stop_to_full_running_fraction - interval;
		else
			stop_to_full_running_fraction = interval + stop_to_full_running_fraction;

		stop_to_full_running_fraction = std::clamp(stop_to_full_running_fraction, 0.f, 1.f);
	}

	if (speed > 135.2f && is_standing)
	{
		stop_to_full_running_fraction = fmaxf(stop_to_full_running_fraction, .0099999998f);
		is_standing = false;
	}

	if (speed < 135.2f && !is_standing)
	{
		stop_to_full_running_fraction = fminf(stop_to_full_running_fraction, .99000001f);
		is_standing = true;
	}
}

void c_antiaim::increment_visual_progress()
{
	if (!config.rage.enabled || !engine_client()->is_ingame())
		return;

	visual_choke = client_state->choked_commands;
}

bool c_antiaim::on_peek(c_cs_player* local, bool& target)
{
	target = true;

	const auto weapon_cfg = c_aimhelper::get_weapon_conf();

	if (local->get_abs_velocity().length2d() < 2.f || !weapon_cfg.has_value())
		return false;

	target = false;

	const auto velocity = local->get_velocity() * (2.f / 3.f);
	const auto ticks = client_state->choked_commands > 1 ? game_rules->is_valve_ds() ? std::min(6, (int)config.rage.fakelag_moving) : config.rage.fakelag_moving : 10;
	const auto pos = local->get_shoot_position() + velocity * ticks_to_time(ticks);

	auto found = false;

	client_entity_list()->for_each_player([&] (c_cs_player* player) -> void
	{
		if (!player->is_enemy() || player->is_dormant() || player->is_local_player())
			return;

		const auto record = animation_system->get_latest_animation(player);

		if (!record.has_value())
			return;

		const auto scan = c_ragebot::scan_record_gun(local, record.value(), pos);

		if (scan.has_value() && (scan.value().damage > weapon_cfg.value().min_dmg
				|| player->get_health() < scan.value().damage))
			found = true;
	});

	if (found)
		return true;

	target = true;
	return false;
}