#include "c_legitbot.h"
#include "../utils/math.h"
#include "../sdk/c_global_vars_base.h"
#include "../sdk/c_cvar.h"
#include "c_animation_system.h"
#include "c_aimhelper.h"
#include "../hooks/idirect3ddevice9.h"

void c_legitbot::aim(c_user_cmd* cmd)
{
    if (!config.legit.assist)
        return;

    auto local = c_cs_player::get_local_player();

    if (!local)
        return;

    const auto weapon = client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle());

    if (!weapon)
        return;

    static std::optional<std::tuple<c_vector3d, float, c_animation_system::animation*>> target = std::nullopt;

    if (!target.has_value())
    {
        const auto hitbox = c_cs_player::hitbox::head;

        target = c_aimhelper::get_legit_target(config.legit.fov, config.legit.backtrack / 100.f, hitbox, true);
    }
	else
	{
		const auto shoot_position = local->get_shoot_position();

		auto aim_angle = math::calc_angle(shoot_position, std::get<0>(target.value()));

		math::normalize(aim_angle);
		cmd->viewangles = aim_angle;
	}
}

void c_legitbot::backtrack(c_cs_player* local, c_user_cmd* cmd)
{
	if (config.legit.backtrack <= 0.f
		|| !(cmd->buttons & c_user_cmd::flags::attack)
		|| !local->can_shoot(cmd, global_vars_base->curtime, true))
		return;

	const auto target = c_aimhelper::get_legit_target(config.legit.fov, config.legit.backtrack / 100.f);

	if (!target.has_value())
		return;

	cmd->tick_count = time_to_ticks(std::get<1>(target.value())) + lerp_ticks;
}
