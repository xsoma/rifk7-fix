#include "c_engine_client.h"
#include "../utils/c_hook.h"
#include <future>
#include "../hacks/c_animation_system.h"

void c_engine_client_::hook()
{
	static c_hook<c_engine_client> hook(engine_client());
	_is_box_visible = hook.apply<is_box_visible_t>(32, is_box_visible);
	_fire_event = hook.apply<fire_event_t>(59, fire_event);
	_is_hltv = hook.apply<is_hltv_t>(93, is_hltv);
	_is_paused = hook.apply<is_paused_t>(90, is_paused);
}

int32_t __fastcall c_engine_client_::is_box_visible(c_engine_client* engine_client, uint32_t, c_vector3d& min, c_vector3d& max)
{
	static const auto ret = __("\x85\xC0\x74\x2D\x83\x7D\x10\x00\x75\x1C");

	_rt(addr, ret);
	if (!memcmp(_ReturnAddress(), addr, 10))
		return 1;

	return _is_box_visible(engine_client, min, max);
}

void __fastcall c_engine_client_::fire_event(c_engine_client* engine_client, uint32_t)
{
	const auto local = c_cs_player::get_local_player();

	if (!local || !local->is_alive() || !engine_client->is_ingame())
		return _fire_event(engine_client);

	auto cur_event = *reinterpret_cast<c_event_info**>(reinterpret_cast<uintptr_t>(client_state) + 0x4E64);

	if (!cur_event)
		return _fire_event(engine_client);

	return _fire_event(engine_client);
}

bool c_engine_client_::is_hltv(c_engine_client* engine_client, uint32_t)
{
	static const auto return_to_setup_velocity = sig("client.dll", "84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80");
	static const auto return_to_accumulate_layers = sig("client.dll", "84 C0 75 0D F6 87");

	if (_ReturnAddress() == return_to_setup_velocity && animation_system->enable_bones && engine_client->is_ingame())
		return true;

	if (_ReturnAddress() == return_to_accumulate_layers && engine_client->is_ingame())
		return true;

	return _is_hltv(engine_client);
}

bool c_engine_client_::is_paused(c_engine_client* engine_client, uint32_t)
{
	static const auto extrapolation = sig("client.dll", "FF D0 A1 ?? ?? ?? ?? B9 ?? ?? ?? ?? D9 1D ?? ?? ?? ?? FF 50 34 85 C0 74 22 8B 0D ?? ?? ?? ??") + 0x29;

	if (_ReturnAddress() == (void*)extrapolation)
		return true;

	return _is_paused(engine_client);
}
