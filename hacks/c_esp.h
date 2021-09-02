#pragma once

#include "../includes.h"
#include "../hooks/idirect3ddevice9.h"
#include <mutex>

class c_esp
{
	struct player_info
	{
		c_base_handle handle{};
		c_cs_player* player{};
		int32_t index{};
		char name[16]{};
		c_vector3d origin;
		c_qangle angles;
		std::optional<c_vector3d> head_position;
		float duck_amount{};
		int health{}, last_health{}, health_fade{}, ammo{}, ammo_max{};
		bool is_enemy{}, is_on_ground{}, dormant{}, reload{};
		c_base_animating::animation_layers layers{};
		matrix3x4 bones[128]{};
		bool good_bones{};
		c_base_combat_weapon* pWeapon;
	};

	struct nade_info
	{
		int32_t type;
		c_vector3d origin;
	};

	struct esp_info
	{
		esp_info() { }  // NOLINT(hicpp-use-equals-default, modernize-use-equals-default)

		std::vector<player_info> players;
		std::vector<nade_info> nades;
		c_vector3d shoot_position{};
		bool is_scoped{};
		viewmatrix world_to_screen_matrix{};
		std::mutex mutex;
	};

public:
	static void draw();
	static void draw_local_impact(c_vector3d start, c_vector3d end);
	static void draw_enemy_impact(c_game_event* event);

	static void store_data();

	inline static esp_info info;
private:
	static constexpr auto esp_flags = c_font::font_flags::centered_x | c_font::font_flags::centered_y | c_font::font_flags::drop_shadow;
	static constexpr auto collision_box_top = 72.f;
	static constexpr auto collision_box_mod = 18.f;
	static constexpr auto head_radius = 6.5f;
	static constexpr auto fade_frequency =  255 / 1.f;

	static void esp_name(player_info& player, const c_vector2d& from, float width, const c_color& color);
	static void esp_box(const c_vector2d& from, float width, float height, const c_color& color);
	static void esp_health(player_info& player, const c_vector2d& from, float height, const c_color& color);
	static void esp_ammo(player_info& player, const c_vector2d& from, float width, float height, const c_color& color);
	static void esp_radar(std::optional<c_vector3d> position, const c_color& color);
    static void esp_skeleton(player_info& player, const c_color& color, bool retro = false);
	static void esp_weapon(player_info& player, const c_vector2d& from, const float width, const float height, const c_color& color);
	static void draw_player(player_info& player);
	static void draw_players();

	static void draw_nade(nade_info& nade);
	static void draw_nades();

	static void draw_scope();
	static void draw_impact(c_vector3d start, c_vector3d end, c_color color);
};


class c_nadepoint {
public:
	c_nadepoint() {
		m_valid = false;
	}

	c_nadepoint(c_vector3d start, c_vector3d end, bool plane, bool valid, c_vector3d normal, bool detonate) {
		m_start = start;
		m_end = end;
		m_plane = plane;
		m_valid = valid;
		m_normal = normal;
		m_detonate = detonate;
	}

	c_vector3d m_start, m_end, m_normal;
	bool m_valid, m_plane, m_detonate;
};

enum nade_throw_act {
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP
};

class c_nade_prediction {
	std::array< c_nadepoint, 500 >	_points{ };
	bool		 _predicted = false;

	void predict(c_user_cmd* ucmd, c_cs_player* local);
	bool detonated(c_base_combat_weapon* weapon, float time, game_trace& trace);
public:
	void trace(c_user_cmd* ucmd, c_cs_player* local);
	void draw();
};

extern c_nade_prediction g_nadepred;

