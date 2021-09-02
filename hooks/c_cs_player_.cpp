#include "c_cs_player_.h"
#include "../hacks/c_trace_system.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_global_vars_base.h"
#include "../sdk/c_surface_props.h"
#include "../sdk/c_engine_trace.h"
#include "../sdk/c_prediction.h"
#include "../sdk/recv_prop_hook.h"
#include <future>
#include "../utils/math.h"
#include "../utils/c_config.h"
#include "c_events.h"
#include "../hacks/c_ragebot.h"

void c_cs_player_::hook()
{
	static uint32_t dummy[1] = { reinterpret_cast<uint32_t>(c_cs_player::get_vtable()) };
	hk = std::make_unique<c_hook<uint32_t>>(dummy);
	
	hk->apply<void(__thiscall*)()>(157, is_player);  // maby fps drain

	_eye_angles = hk->apply<eye_angles_t>(169, eye_angles);

	_build_transformations = hk->apply<build_transformations_t>(189, build_transformations);
	_do_extra_bone_processing = hk->apply<do_extra_bone_processing_t>(197, do_extra_bone_processing);
	_standard_blending_rules = hk->apply<standard_blending_rules_t>(205, standard_blending_rules);
	_update_client_side_animation = hk->apply<update_client_side_animation_t>(223, update_client_side_animation);
}

void c_cs_player_::apply_to_player(c_cs_player* player)
{
	hk->patch_pointer(reinterpret_cast<uintptr_t*>(player));
}

c_qangle* c_cs_player_::eye_angles(c_cs_player* player, uint32_t)
{
	static auto return_to_fire_bullet = sig("client.dll", "8B 0D ? ? ? ? F3 0F 7E 00 8B 40 08 89 45 E4");
	static auto return_to_set_first_person_viewangles = sig("client.dll", "8B 5D 0C 8B 08 89 0B 8B 48 04 89 4B 04 B9");

	if (player->is_local_player()
		&& _ReturnAddress() != return_to_fire_bullet
		&& _ReturnAddress() != return_to_set_first_person_viewangles
		&& config.rage.enabled
		&& engine_client()->is_ingame())
	{
		auto& angle = animation_system->local_animation.eye_angles;

		if (player->get_anim_state()
			&& player->get_anim_state()->in_hit_ground_animation
			&& !animation_system->in_jump)
			angle.x = c_ragebot::last_pitch.value_or(0.f);

		return &angle;
	}

	return _eye_angles(player);
}

/*bool c_cs_player_::rebuild_setup_bones(c_cs_player* player, matrix3x4* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	if (!player || !player->is_alive())
		return false;

	if (player->get_sequence_activity(player->get_animation_layers[3]->cycle) == -1)
	{
		return false;
	}

	// We should get rid of this someday when we have solutions for the odd cases where a bone doesn't
	// get setup and its transform is asked for later.
	if (cvar()->find_var("cl_setupallbones")->get_int())
	{
		boneMask |= bone_used_by_anything;
	}

	int nLOD = 0;
	int nMask = bone_used_by_vertex_lod0;
	for (; nLOD < 128; ++nLOD, nMask « = 1)
	{
		if (boneMask & nMask)
			break;
	}
	for (; nLOD < 128; ++nLOD, nMask « = 1)
	{
		boneMask |= nMask;
	}

	float flLastBoneSetupTime = player->get_last_bone_setup_time();
	if (currentTime < flLastBoneSetupTime)
	{
		player->get_readable_bones() = 0;
		player->get_writable_bones() = 0;
		player->get_last_bone_setup_time() = currentTime;
	}

	// Have we cached off all bones meeting the flag set?
	DWORD EntityPlus4 = (DWORD)((DWORD)player + 4);

	// Setup our transform based on render angles and origin.
	c_vector3d LocalOrigin = player->get_abs_origin();
	c_qangle LocalAngles = player->get_abs_angles();

	__declspec (align(16)) matrix3x4 parentTransform;
	math::angle_matrix(LocalAngles);
	math::angle_matrix(LocalOrigin);

	// Load the boneMask with the total of what was asked for last frame.

	// Allow access to the bones we're setting up so we don't get asserts in here.
	int oldReadableBones = player->get_readable_bones();
	int oldWritableBones = player->get_writable_bones();
	int newWritableBones = oldReadableBones | boneMask;
	player->get_writable_bones() = newWritableBones;
	player->get_readable_bones() = newWritableBones;

	c_vector3d pos[128];
	quaternion q[128];
	memset(pos, 0xFF, sizeof(pos));
	memset(q, 0xFF, sizeof(q));

	int backup_occlusion_flags = player->get_occlusion_flags();
	int backup_occlusion_framecount = player->get_occlusion_framecount();

	int newBoneMask = boneMask;
	if (backup_occlusion_framecount != -1)
		newBoneMask = boneMask & backup_occlusion_flags;

	newBoneMask |= 0x80000;

	c_studio_hdr* hdr = player->get_model_hdr();
	player->standard_blending_rules(hdr, pos, q, currentTime, newBoneMask);

	if (newBoneMask != boneMask)
	{
		studiohdr* hdrt = hdr->m_pStudioHdr;
		if (hdrt->numbones > 0)
		{
			intptr_t* pFlags = *(intptr_t**)((DWORD)hdr + 48);
			c_vector3d* pEntPos = (c_vector3d*)((DWORD)EntityPlus4 + (2660 - sizeof(float) * 2));
			quaternion* pEntQ = (quaternion*)((DWORD)EntityPlus4 + 5724);

			for (DWORD i = 0; i < hdrt->numbones; i++)
			{
				if (!(pFlags[i] & 0x80300))
				{
					pos[i] = pEntPos[i];
					q[i] = pEntQ[i];
				}
			}
		}
	}

	player->get_occlusion_framecount() = backup_occlusion_framecount;

	byte computed[32] = { 0 };

	// don't calculate IK on ragdolls
	player->build_transformations(hdr, pos, q, parentTransform, boneMask, &computed[0]);

	*(int*)(EntityPlus4 + 224) &= ~8;
	return true;
}*/

void __fastcall c_cs_player_::build_transformations(c_cs_player* player, uint32_t, c_studio_hdr* hdr, c_vector3d* pos, quaternion* q,
	const matrix3x4& transform, const int32_t mask, byte* computed)
{
	// backup bone flags.
	const auto backup_bone_flags = hdr->bone_flags;

	// stop procedural animations.
	for (auto i = 0; i < hdr->bone_flags.Count(); i++)
		hdr->bone_flags.Element(i) &= ~bone_always_procedural;

	_build_transformations(player, hdr, pos, q, transform, mask, computed);

	// restore bone flags.
	hdr->bone_flags = backup_bone_flags;
}

int c_cs_player_::proxy_is_player(c_cs_player* player, void* return_address, void* eax)
{
	static const auto return_to_should_skip_animframe = sig("client.dll", "84 C0 75 02 5F C3 8B 0D");

	if (return_address != return_to_should_skip_animframe)
		return 2;

	const auto local = c_cs_player::get_local_player();

	if (player->is_local_player() && player->is_alive())
		antiaim->prepare_animation(player);

	if (!local || !local->is_alive() || !player->is_enemy())
		return 2;

	return !(player->get_effects() & c_base_entity::ef_nointerp);
}

void c_cs_player_::standard_blending_rules(c_cs_player* player, uint32_t, c_studio_hdr* hdr, c_vector3d* pos, quaternion* q, const float time, int mask)
{
	if (player->is_enemy() || player->is_local_player())
		mask = bone_used_by_server;

	if (player->is_local_player())
		mask |= bone_used_by_bone_merge;
	
	_standard_blending_rules(player, hdr, pos, q, time, mask);
}

void c_cs_player_::do_extra_bone_processing(c_cs_player* player, uint32_t, c_studio_hdr* hdr, c_vector3d* pos, quaternion* q,
	const matrix3x4& matrix, uint8_t* bone_computed, void* context)
{	
	
	if (player->get_effects() & c_base_entity::ef_nointerp)
		return;

	const auto state = uint32_t(player->get_anim_state());

	if (!state)
		return _do_extra_bone_processing(player, hdr, pos, q, matrix, bone_computed, context);

	const auto backup_tickcount = *reinterpret_cast<int32_t*>(state + 8);
	*reinterpret_cast<int32_t*>(state + 8) = 0;
	_do_extra_bone_processing(player, hdr, pos, q, matrix, bone_computed, context);
	*reinterpret_cast<int32_t*>(state + 8) = backup_tickcount;
}

void c_cs_player_::update_client_side_animation(c_cs_player* player, uint32_t)
{
	const auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive() || !player->is_local_player())
		return _update_client_side_animation(player);

	if (animation_system->enable_bones)
		_update_client_side_animation(player);
}

// ReSharper disable once CppDeclaratorNeverUsed
static uint32_t is_player_retn_address = 0;

bool __declspec(naked) is_player(void* eax, void* edx)
{
	_asm
	{
		push eax

		mov eax, [esp + 4]
		mov is_player_retn_address, eax

		push is_player_retn_address
		push ecx
		call c_cs_player_::proxy_is_player

		cmp eax, 1
		je _retn1

		cmp eax, 0
		je _retn0

		mov al, 1
		retn

		_retn0:
			mov al, 0
			retn

		_retn1:
			pop eax
			mov eax, is_player_retn_address
			add eax, 0x6B
			push eax
			retn
	}
}
