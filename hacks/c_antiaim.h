#pragma once

#include "../includes.h"

class c_antiaim : public c_singleton<c_antiaim>
{
public:
	c_antiaim();
	void fakelag(c_cs_player* local, c_user_cmd* cmd, bool& send_packet);
	void run(c_cs_player* local, c_user_cmd* cmd, bool& send_packet, int sequence);
	void prepare_animation(c_cs_player* local);
	void predict(c_cs_player* local, c_user_cmd* cmd);
	float visual_choke = 0.f;
	c_qangle last_real{}, last_fake{};
	void increment_visual_progress();
	int side = -1;
	uint32_t shot_cmd{};
private:
	bool on_peek(c_cs_player* local, bool& target);
	float next_lby_update = 0.f, lby_update = 0.f,
		min_delta = 0.f, max_delta = 0.f,
		stop_to_full_running_fraction = 0.f,
		feet_speed_stand = 0.f, feet_speed_ducked = 0.f, body_lean = 0.f;
	uint32_t estimated_choke = 0;
	bool is_standing = false;
};

#define antiaim c_antiaim::instance()