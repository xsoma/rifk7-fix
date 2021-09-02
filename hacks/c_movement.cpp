#include "c_movement.h"

void c_movement::run(c_cs_player* local, c_user_cmd* cmd)
{
	bhop(local, cmd);
	autostrafe(local, cmd);
}

void c_movement::autostrafe(c_cs_player* local, c_user_cmd* cmd)
{
	const auto vel = local->get_velocity().length2d();

	if (vel < 1.f)
		return;

	if (!(local->get_flags() & c_cs_player::on_ground))
	{
		if (cmd->mousedx > 1 || cmd->mousedx < -1)
			cmd->sidemove = cmd->mousedx < 0.f ? -450.f : 450.f;
		else
		{
			cmd->forwardmove = std::clamp(10000.f / vel, -450.0f, 450.0f);
			cmd->sidemove = cmd->command_number % 2 == 0 ? -450.f : 450.f;
		}
	}
};

void c_movement::bhop(c_cs_player* local, c_user_cmd* cmd)
{
	const auto move_type = local->get_move_type();
	const auto flags = local->get_flags();

	if (move_type != c_cs_player::movetype_ladder && move_type != c_cs_player::movetype_noclip &&
		!(flags & c_cs_player::in_water))
	{
		if (cmd->buttons & c_user_cmd::jump)
		{
			if (!(local->get_flags() & c_cs_player::on_ground))
			{
				cmd->buttons &= ~c_user_cmd::jump;
			}
		}
	}
}
