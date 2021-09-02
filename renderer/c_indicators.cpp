#include "../BASS/bass.h"
#include "c_indicators.h"
#include "../sdk/c_engine_client.h"
#include "../hacks/c_fake_ping.h"
#include "../hacks/c_trace_system.h"
#include "../hacks/c_antiaim.h"
#include "../sdk/c_client_state.h"
#include "../sdk/c_weapon_system.h"
#include "../utils/math.h"
#include "../sdk/c_input.h"

void c_indicators::draw()
{
	const auto local = c_cs_player::get_local_player();

	if (config.misc.arrows && engine_client()->is_ingame()
		&& local && local->is_alive())
		draw_antiaim();

	if (!config.misc.indicators)
		return;

	auto position = c_vector2d(20, renderer->get_center().y - 80);

	draw_radio_info(position);	

	if (!engine_client()->is_ingame() || !local || !local->is_alive())
		return;

	draw_choke(position);
	draw_ping_acceptance(position);
	draw_velocity(position);
}

void c_indicators::draw_radio_info(c_vector2d& position)
{
	if (strlen(BASS::bass_metadata) > 0 && config.misc.radio_channel)
	{
		static const auto radio_info = __("Now playing:");
		static const auto muted = __("MUTED");

		_rt(radio, radio_info);
		renderer->text(position, radio, c_color::primary(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);
		position.y += 16;

		renderer->text(position, BASS::bass_metadata, c_color::foreground(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);
		position.y += 16;

		if (radio_muted)
		{
			_rt(rad_muted, muted);
			renderer->text(position, rad_muted, c_color::gradient3(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);
		}
	}
	position.y += 32;
}

void c_indicators::draw_ping_acceptance(c_vector2d& position)
{
	static const auto ping_acceptance = __("Fake Latency - %d ms");
	const auto current_ping = net_channel->get_avg_latency(flow_incoming) * 1000.0f;
	_rt(pa_format, ping_acceptance);
	char pa[40];
	sprintf_s(pa, pa_format, static_cast<int>(current_ping));
	draw_progressbar(position, pa, c_color::gradient1(), c_color::accent(), current_ping, 200.f);
}

void c_indicators::draw_velocity(c_vector2d& position)
{
	if (!config.rage.enabled)
		return;
	static const auto velocity = __("Velocity - %d u/s");
	_rt(ch, velocity);
	char pa[40];
	sprintf_s(pa, ch, static_cast<int>(c_cs_player::get_local_player()->get_velocity().length2d()));
	draw_progressbar(position, pa, c_color::primary(), c_color::accent(), c_cs_player::get_local_player()->get_velocity().length2d(), 260.f);
}

void c_indicators::draw_penetration_crosshair()
{
	const auto local = c_cs_player::get_local_player();
	const auto weapon = reinterpret_cast<c_base_combat_weapon*>(client_entity_list()->get_client_entity_from_handle(local->get_current_weapon_handle()));
	if (!weapon)
		return;
	const auto info = weapon_system->get_weapon_data(weapon->get_item_definition());
	if (!info)
		return;
	float length = info->flRange;
	game_trace tr;
	ray r;
	c_trace_filter filter;
	filter.skip_entity = local;
	c_vector3d start = local->get_shoot_position();
	c_vector3d viewangles = engine_client()->get_view_angles();
	c_vector3d fw;
	math::angle_vectors(viewangles, fw);
	fw.normalize();
	c_vector3d end = start + fw * length;
	r.init(start, end);
	engine_trace()->trace_ray(r, mask_shot, &filter, &tr);
	if (!tr.did_hit() || tr.entity)
	{
		int width = 0;
		int height = 0;
		engine_client()->get_screen_size(width, height);
		bool damage = false;
		auto const wall = trace_system->wall_penetration(local->get_shoot_position(), tr.endpos, nullptr, local);
		if (wall.has_value())
			damage = true;
		renderer->circle_filled(c_vector2d(width / 2, height / 2), 2.f, damage ? c_color::primary() : c_color::accent());
	}
}

void c_indicators::draw_choke(c_vector2d& position)
{
	if (!config.rage.enabled)
		return;
	static const auto choke = __("Choke - %d ticks");
	_rt(ch, choke);
	char pa[40];
	sprintf_s(pa, ch, static_cast<int>(antiaim->visual_choke));
	draw_progressbar(position, pa, c_color::gradient3(), c_color::gradient4(), antiaim->visual_choke, 16.f);
}

void c_indicators::draw_antiaim()
{
	if (!config.rage.enabled)
		return;

	const auto local = c_cs_player::get_local_player();

	if (input->camera_in_third_person)
	{
		c_vector2d src, dst;
		c_vector3d src3D, dst3D, forward;
		src3D = local->get_abs_origin();
		math::angle_vectors(c_qangle(0, antiaim->last_real.y, 0), forward);
		forward *= 35.f;
		dst3D = src3D + forward;

		if (!renderer->screen_transform(src3D, src, renderer->world_to_screen_matrix()) || !renderer->screen_transform(dst3D, dst, renderer->world_to_screen_matrix()))
			return;

		renderer->line(src, dst, c_color::primary());
		renderer->text(dst, "REAL", c_color::primary(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);

		math::angle_vectors(c_qangle(0, antiaim->last_fake.y, 0), forward);
		forward *= 35.f;
		dst3D = src3D + forward;

		if (!renderer->screen_transform(src3D, src, renderer->world_to_screen_matrix()) || !renderer->screen_transform(dst3D, dst, renderer->world_to_screen_matrix()))
			return;

		renderer->line(src, dst, c_color::accent());
		renderer->text(dst, "FAKE", c_color::accent(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);
	}
	else
	{
		const auto yaw = engine_client()->get_view_angles().y;
		const auto real = deg2rad(yaw - antiaim->last_real.y);
		const auto fake = deg2rad(yaw - antiaim->last_fake.y);

		draw_arrow(real, c_color::primary());
		draw_arrow(fake, c_color::accent());
	}
}

void c_indicators::draw_progressbar(c_vector2d& position, const char* name, const c_color color1, const c_color color2, const float progress, const float maxvalue)
{
	renderer->text(position, name, c_color::foreground(), fnv1a("tahoma13"), c_font::font_flags::drop_shadow);
	position.y += 16;

	const auto size = c_vector2d(120, 12);
	renderer->rect_filled_linear_gradient(position + c_vector2d(2, 2), c_vector2d(size.x - 4, size.y - 4), color1, color2, true);
	renderer->rect_filled(position + c_vector2d(2 + size.x * std::clamp(progress / maxvalue, 0.f, 1.f), 2),
		c_vector2d(std::clamp(size.x * std::clamp(1.f - progress / maxvalue, 0.f, 1.f) - 4, 0.f, size.x - 4), size.y - 4), c_color::background());
	renderer->rect(position + c_vector2d(1, 1), size - c_vector2d(3, 3), c_color::shadow(100));
	renderer->rect(position, size - c_vector2d(1, 1), c_color::border());

	position.y += 20;
}

void c_indicators::draw_arrow(float angle, c_color color)
{
	c_vector2d pos[8] =
	{
		{ -7.f, -50.f },
		{ -7.f, -140.f },

		{ 7.f, -50.f },
		{ 7.f, -140.f },

		{ -20.f, -130.f },
		{ 0.f, -160.f },

		{ 20.f, -130.f },
		{ 0.f, -160.f }
	};
	
	for (auto& p : pos)
	{
		const auto s = sin(angle);
		const auto c = cos(angle);

		p = c_vector2d(p.x * c - p.y * s, p.x * s + p.y * c) + renderer->get_center();
	}

	renderer->line(pos[0], pos[1], color);
	renderer->line(pos[2], pos[3], color);
	renderer->line(pos[4], pos[5], color);
	renderer->line(pos[6], pos[7], color);
}
