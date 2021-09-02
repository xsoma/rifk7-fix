#include "c_client.h"
#include "../utils/c_hook.h"
#include "../hacks/c_animation_system.h"
#include "../sdk/c_client_entity_list.h"
#include "../sdk/c_game_rules.h"
#include "../sdk/c_weapon_system.h"
#include "../hacks/c_esp.h"
#include "c_net_channel_.h"
#include "../hacks/c_movement.h"
#include "../hacks/c_resolver.h"
#include "../hacks/debug.h"
#include "../hacks/c_miscellaneous.h"
#include "../hacks/c_chams.h"
#include "../hacks/c_ragebot.h"
#include "c_client.h"
#include "../hacks/c_prediction_system.h"
#include "../sdk/c_input.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_prediction.h"
#include "../sdk/c_base_view_model.h"
//#include "../hacks/c_legitbot.h"
#include "c_client_state_.h"
#include "c_events.h"
#include "c_cl_camera_height_restriction_debug.h"
#include "../sdk/c_material_system.h"

void c_client::hook()
{
	static c_hook<c_base_client> hook(base_client());
	_level_init_pre_entity = hook.apply<level_init_pre_entity_t>(5, level_init_pre_entity);
	_create_move = hook.apply<create_move_t>(22, ::create_move);
	_frame_stage_notify = hook.apply<frame_stage_notify_t>(37, frame_stage_notify);
}

void c_client::level_init_pre_entity(c_base_client* client, uint32_t, const char* map_name)
{
	static const auto welcome_message = __("Now playing on: %s.");

	if (map_name)
	{
		char welcome[80];
		_rt(welcome_msg, welcome_message);
		sprintf_s(welcome, welcome_msg, map_name);
		logging->info(welcome);
	}
	c_client_state_::cmds.clear();
	_level_init_pre_entity(client, map_name);
}

template<typename FuncType>
__forceinline static FuncType CallVFuckingFunc(void* ppClass, int index)
{
	int* pVTable = *(int**)ppClass;
	int dwAddress = pVTable[index];
	return (FuncType)(dwAddress);
}

template<class T>
static T* FindHudElement(const char* name)
{
	static auto pThis = *reinterpret_cast<DWORD**> (sig("client.dll", "B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08") + 1);

	static auto find_hud_element = reinterpret_cast<DWORD(__thiscall*)(void*, const char*)>(sig("client.dll", "55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));
	return (T*)find_hud_element(pThis, name);
}

c_user_cmd *c_input::GetUserCmd(int nSlot, int sequence_number)
{
	typedef c_user_cmd*(__thiscall *GetUserCmd_t)(void*, int, int);
	return CallVFuckingFunc<GetUserCmd_t>(this, 8)(this, nSlot, sequence_number);
}

void patch(PVOID address, const int type, const int bytes)
{
	DWORD d, ds; //declared for future use.
	VirtualProtect(address, bytes, PAGE_EXECUTE_READWRITE, &d); //remove write protection!
	memset(address, type, bytes); //patch the data
	VirtualProtect(address, bytes, d, &ds); //set the write protection back to its normal state
}

void __stdcall c_client::create_move(int sequence_number, float input_sample_frametime, bool active, bool& sendpacket)
{
	static const auto get_check_sum = reinterpret_cast<uint32_t(__thiscall*)(c_user_cmd*)>(sig("client.dll", "53 8B D9 83 C8"));

	if (!get_check_sum) {
		cvar()->console_color_printf(false, c_color(255, 100, 100), "FAILED TO FIND `get_check_sum`\n");
	}
	_create_move(base_client(), sequence_number, input_sample_frametime, active);

	auto cmd = input->GetUserCmd(0, sequence_number);
	auto verified_cmd = &input->verified_commands[sequence_number % 150];
	if (!cmd || cmd == 0) return _create_move(base_client(), sequence_number, input_sample_frametime, active);

	const auto local = reinterpret_cast<c_cs_player*>(client_entity_list()->get_client_entity(engine_client()->get_local_player()));

	c_cl_camera_height_restriction_debug::in_cm = true;

	if (!cmd->command_number || !engine_client()->is_ingame() || !engine_client()->is_connected() || !local)
	{
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	if (c_net_channel_::hk == nullptr && client_state && client_state->net_channel)
		c_net_channel_::apply_to_net_chan(client_state->net_channel);

	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(
		client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));

	c_base_combat_weapon::weapon_data* wpn_info = nullptr;

	if (!weapon || !(wpn_info = weapon_system->get_weapon_data(weapon->get_item_definition())))
	{
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	animation_system->in_jump = cmd->buttons & c_user_cmd::jump;

	g_nadepred.trace(cmd, local);
	// run clantag
	c_miscellaneous::clantag();

	if (!config.rage.enabled /*|| engine_client()->get_engine_build_number() != 13754*/ || !local->is_alive())
	{
		prediction_system->initial(local, cmd, sequence_number);

		//c_legitbot::backtrack(local, cmd);

		//c_legitbot::aim(cmd);

		if (local->is_shooting(cmd, global_vars_base->curtime))
		{
			// store shot info.
			resolver::shot shot{};
			shot.start = local->get_shoot_position();
			shot.time = global_vars_base->curtime;
			shot.manual = true;
			c_resolver::register_shot(std::move(shot));
		}
		local->get_pose_parameter() = animation_system->local_animation.poses;
		*local->get_animation_layers() = animation_system->server_layers;
		animation_system->local_animation.eye_angles = cmd->viewangles;
		math::normalize(animation_system->local_animation.eye_angles);
		local->get_anim_state()->feet_yaw_rate = 0.f;
		animation_system->update_player(local);
		animation_system->local_animation.abs_ang.y = local->get_anim_state()->goal_feet_yaw;
		animation_system->local_animation.layers = *local->get_animation_layers();
		animation_system->local_animation.poses = local->get_pose_parameter();
		prediction_system->restore();

		if (sendpacket)
		{
			local->setup_bones(c_chams::instance()->fakelagmatrix, 128, bone_used_by_hitbox, global_vars_base->curtime);
		}

		// verify new command.
		math::ensure_bounds(cmd->viewangles, *reinterpret_cast<c_vector3d*>(&cmd->forwardmove));
		verified_cmd->cmd = *cmd;
		verified_cmd->crc = get_check_sum(&verified_cmd->cmd);
		c_cl_camera_height_restriction_debug::in_cm = false;
		return;
	}

	static std::once_flag fl;
	std::call_once(fl, []()
	{
		static auto fakelagboost = reinterpret_cast<int*> (sig("engine.dll", "0F 4F F0 89 5D FC") - 0x6);
		patch(static_cast<PVOID> (fakelagboost), 17, 1);
	});

	cmd->buttons &= ~c_user_cmd::speed;

	c_movement::run(local, cmd);

	prediction_system->initial(local, cmd, sequence_number);
	auto target_angle = cmd->viewangles;
	
	// initialize revolver.
	local->can_shoot(cmd, global_vars_base->curtime);
	antiaim->fakelag(local, cmd, sendpacket);
	const auto stopped_last_interval = c_ragebot::get_autostop_info().did_stop;
	c_ragebot::autostop(local, cmd);

	if (local->is_shooting(cmd, global_vars_base->curtime))
	{
		// store shot info.
		resolver::shot shot{};
		shot.start = local->get_shoot_position();
		shot.time = global_vars_base->curtime;
		shot.manual = true;
		c_resolver::register_shot(std::move(shot));
	}

	c_ragebot::aim(local, cmd, sendpacket);
	if (local->is_shooting(cmd, global_vars_base->curtime))
	{
		c_aimhelper::fix_movement(cmd, target_angle);
		antiaim->shot_cmd = cmd->command_number;
	}

	if ((!client_state->choked_commands && stopped_last_interval)
		|| (stopped_last_interval && c_cs_player::can_not_shoot_due_to_cock && weapon->get_item_definition() == weapon_revolver))
		c_ragebot::autostop(local, cmd);

	if (!c_ragebot::get_autostop_info().did_stop)
	{
		cmd->forwardmove = prediction_system->original_cmd.forwardmove;
		cmd->sidemove = prediction_system->original_cmd.sidemove;
		prediction_system->repredict(local, cmd);
	}
	antiaim->run(local, cmd, sendpacket, sequence_number);
	if (client_state->choked_commands >= (game_rules->is_valve_ds() ? 6 : 16))
	{
		sendpacket = true;
	}
	c_client_state_::cmds.push_back(cmd->command_number);
	prediction_system->repredict(local, cmd);
	prediction_system->restore();

	if (sendpacket)
	{
		antiaim->last_real = cmd->viewangles;
		local->setup_bones(c_chams::instance()->fakelagmatrix, 128, bone_used_by_hitbox, global_vars_base->curtime);
	}
	else
	{
		antiaim->last_fake = cmd->viewangles;
	}

	// remove duck stamina.
	c_miscellaneous::remove_duck_stamina(cmd);
	// movement fix
	c_aimhelper::fix_movement(cmd, target_angle);
	// verify new command.
	math::ensure_bounds(cmd->viewangles, *reinterpret_cast<c_vector3d*>(&cmd->forwardmove));
	c_miscellaneous::set_buttons_for_direction(cmd);
	verified_cmd->cmd = *cmd;
	verified_cmd->crc = get_check_sum(&verified_cmd->cmd);
	c_cl_camera_height_restriction_debug::in_cm = false;
	return;
}
bool ReallocatedDeathNoticeHUD = false;
void c_client::frame_stage_notify(c_base_client* client, uint32_t, const clientframestage stage)
{
	static bool old_night_mode = false;
	static float old_props_transparency = 0.f;
	if (!engine_client()->is_ingame() || !engine_client()->is_connected() || !c_cs_player::get_local_player())
	{
		if (c_net_channel_::hk) {
			c_net_channel_::hk->unapply(39);
			c_net_channel_::_process_packet = nullptr;
			c_net_channel_::hk->unapply(40);
			c_net_channel_::_send_netmsg = nullptr;
			c_net_channel_::hk->unapply(46);
			c_net_channel_::_send_datagram = nullptr;
			c_net_channel_::hk = nullptr;
		}
		old_night_mode = false;
		old_props_transparency = 0.f;
		return _frame_stage_notify(client, stage);
	}

	const auto local = c_cs_player::get_local_player();

	static auto cycle = 0.f;
	static auto anim_time = 0.f;

	const auto view_model = local ? reinterpret_cast<c_base_view_model*>(
		client_entity_list()->get_client_entity_from_handle(local->get_view_model())) : nullptr;

		
		
			if (old_night_mode != config.misc.night_mode || old_props_transparency != config.misc.props_transparency)
			{
				if (cvar()) {
					static const auto fog_enable = cvar()->find_var(_("fog_enable"));
					fog_enable->set_value((config.misc.night_mode ? 1 : 0));
					static const auto fog_override = cvar()->find_var(_("fog_override"));
					fog_override->set_value((config.misc.night_mode ? 1 : 0));
					static const auto fog_color = cvar()->find_var(_("fog_color"));
					fog_color->set_value("200 100 200");
					static const auto fog_start = cvar()->find_var(_("fog_start"));
					fog_start->set_value(0);
					static const auto fog_end = cvar()->find_var(_("fog_end"));
					fog_end->set_value(4500);
					static const auto fog_destiny = cvar()->find_var(_("fog_maxdensity"));
					fog_destiny->set_value(.25f);
					static const auto r_DrawSpecificStaticProp = cvar()->find_var(_("r_DrawSpecificStaticProp"));
					r_DrawSpecificStaticProp->set_value(0);
				}
				for (MaterialHandle_t i = material_system()->FirstMaterial(); i != material_system()->InvalidMaterial(); i = material_system()->NextMaterial(i))
				{
					IMaterial* pMaterial = material_system()->GetMaterial(i);
					if (!pMaterial)
						continue;
					if (strstr(pMaterial->GetTextureGroupName(), "World") || strstr(pMaterial->GetTextureGroupName(), "StaticProp") || strstr(pMaterial->GetTextureGroupName(), "SkyBox"))
					{
						if (config.misc.night_mode) {
							if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
							{
								pMaterial->ColorModulate(0.20, 0.10, 0.20);
								pMaterial->AlphaModulate(1.f - (config.misc.props_transparency / 100.f));
							}
							else if(strstr(pMaterial->GetTextureGroupName(), "World"))
							{
								pMaterial->ColorModulate(0.10, 0.10, 0.10);
							}
							else if (strstr(pMaterial->GetTextureGroupName(), "SkyBox"))
							{
								pMaterial->ColorModulate(0.10, 0.10, 0.10);
							}
						}
						else {
							if (strstr(pMaterial->GetTextureGroupName(), "StaticProp"))
							{
								pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
								pMaterial->AlphaModulate(1.f - (config.misc.props_transparency / 100.f));
							}
							else if (strstr(pMaterial->GetTextureGroupName(), "World"))
							{
								pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
							}
							else if (strstr(pMaterial->GetTextureGroupName(), "SkyBox"))
							{
								pMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
							}
						}
					}
				}
				old_night_mode = config.misc.night_mode;
				old_props_transparency = config.misc.props_transparency;
			}
		
	

	if (stage == frame_render_start)
	{
		c_resolver::on_render_start();

		if (local && local->is_alive())
		{
			local->set_abs_angles(animation_system->local_animation.abs_ang);
			if (config.misc.preserve_killfeed)
			{
				static void(__thiscall * ClearDeathNotices)(DWORD);
				static DWORD* deathNotice;

				if (!ReallocatedDeathNoticeHUD)
				{
					// find HUD cuz we haven't found it yet.
					ReallocatedDeathNoticeHUD = true;
					deathNotice = FindHudElement<DWORD>("CCSGO_HudDeathNotice");
					ClearDeathNotices = (void(__thiscall*)(DWORD))(sig("client.dll", "55 8B EC 83 EC 0C 53 56 8B 71 58"));
				}
				else
				{
					if (deathNotice)
					{
						// preserve killfeed.
						if (c_events::round_flags == ROUND_IN_PROGRESS)
						{
							float* localDeathNotice = (float*)((DWORD)deathNotice + 0x50);
							if (localDeathNotice)
							{
								*localDeathNotice = FLT_MAX;
							}
						}

						// clear killfeed cuz round is done.
						if (c_events::round_flags == ROUND_STARTING && deathNotice - 20)
						{
							if (ClearDeathNotices)
							{
								ClearDeathNotices(((DWORD)deathNotice - 20));
							}
						}
					}
				}
			}
		}
		else
		{
			ReallocatedDeathNoticeHUD = false;
		}
	}

	_frame_stage_notify(client, stage);

	if (stage == frame_render_start)
	{
		antiaim->increment_visual_progress();
		c_esp::store_data();
		c_miscellaneous::remove_flash();
		c_miscellaneous::remove_smoke();
		c_miscellaneous::engine_radar();
	}

	if (stage == frame_net_update_postdataupdate_start && local && local->is_alive())
	{

		if (view_model && !input->camera_in_third_person)
		{
			view_model->get_anim_time() = anim_time;
			view_model->get_cycle() = cycle;
		}

		animation_system->server_layers = *local->get_animation_layers();
	}

	if (view_model)
	{
		cycle = view_model->get_cycle();
		anim_time = view_model->get_anim_time();
	}

	if (stage == frame_net_update_end)
	{
		animation_system->post_player_update();
	}
}

__declspec(naked) void create_move(int sequence_number, float input_sample_frametime, bool active)
{
	__asm
	{
		push ebp
		mov  ebp, esp
		push ebx;
		push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call c_client::create_move
			pop  ebx
			pop  ebp
			retn 0Ch
	}
}
