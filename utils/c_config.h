#pragma once

#include "c_singleton.h"
#include "../security/fnv1a.h"
#include "../security/string_obfuscation.h"
#include "../sdk/c_color.h"
#include "base64.h"
#include <fstream>

using json = nlohmann::json;

#define make_key(var) std::to_string(fnv1a(var))

class c_config : public c_singleton<c_config>
{
public:
	struct config_conf
	{
		struct esp_conf
		{
			struct player
			{
				c_color color, impacts_color;
				bool box, info, impacts;
				int radar, bar, skeleton;
			};

			player team, enemy;

			int nade_type;
			c_color nade_color;
			bool nade_pred, hitmarker;
			int hitsound;
			float hitsound_volume;
		};

		struct chams_conf
		{
			struct chams
			{
				int type;
				c_color color;
				bool xqz;
			};

			chams team, enemy, backtrack, local, arms, weapon;
			bool remove_sleeves;
		};

		struct legit_conf
		{
			bool assist;
			float fov, smooth, backtrack;
			bool only_backtrack;
		};

		struct rage_conf
		{
			struct weapon_conf
			{
				float hitchance, min_dmg,
					head_scale, body_scale, override_min_damage;
			};

			bool enabled, experimental_aa, fakelag_disable_knife, fakelag_disable_taser, fakelag_disable_revolver;
			int experimental_aa_flip, slow_walk, override_min_damage_key, prevent_baim_key, override_resolver_key, force_onshot_key;
			weapon_conf auto_snipe, scout, awp,
				pistol, heavy, rifle, misc;
			float body_lean, inv_body_lean, fakelag_standing, fakelag_moving, fakelag_in_air, fakelag_fakeland;
		};

		struct misc_conf
		{
			int radio_channel;
			float radio_volume;
			int radio_mute;
			int ping_switch;

			float thirdperson_dist;
			int thirdperson_switch;
			int buy_bot;

			bool engine_radar, no_smoke, no_flash, no_scope, preserve_killfeed, hit_effect, kill_effect, night_mode, clantag, indicators, arrows;

			float fov, fov_view, props_transparency;
		};

		esp_conf esp;
		chams_conf chams;
		legit_conf legit;
		rage_conf rage;
		misc_conf misc;
	};

	c_config() = default;

	config_conf& get()
	{
		static config_conf config{
			// esp
			{
				// esp.team
				{
					c_color(120, 120, 255, 255),
					c_color(120, 120, 255, 255),
					false,
					false,
					false,
					0,
					0,
					0
				},
			// esp.enemy
			{
				c_color(230, 70, 140, 255),
				c_color(230, 70, 140, 255),
				false,
				false,
				false,
				0,
				0,
				0
			},
			0,
			c_color::primary(),
			false,
			false,
			0,
			100.f
		},
			// chams
			{
				// chams.team
				{
					0,
					c_color(120, 120, 255, 255),
					false
				},
			// chams.enemy
			{
				0,
				c_color(230, 70, 140, 255),
				false
			},
			// chams.backtrack
			{
				0,
				c_color(70, 225, 230, 255),
				false
			},
			// chams.local
			{
				0,
				c_color::gradient2()
			},
			// chams.arms
			{
				0,
				c_color::primary()
			},
			// chams.weapons
			{
				0,
				c_color::accent()
			},
			false
		},
			// legit
			{
				false,
				0.f,
				0.f,
				0.f
			},
			// rage
			{
				false,
				false,
				false,
				false,
				false,
				0,
				0,
				0,
				0,
				0,
				0,
				// rage.auto_snipe
				{
					0.f,
					0.f,
					0.f,
					0.f
				},
			// rage.scout
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			// rage.awp
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			// rage.pistol
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			// rage.heavy
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			// rage.rifles
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			// rage.misc
			{
				0.f,
				0.f,
				0.f,
				0.f
			},
			100.f,
			100.f,
			1.f,
			13.f,
			8.f,
			16.f
		},

			// misc
			{
				0,
				100.f,
				0,
				0,

				0.f,
				0,

				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,
				false,

				0,
				68.f,
				90.f,
				0.f
			}
		};

		return config;
	}

	static void load_from_file(const std::string& config_path);
	static void save_to_file(const std::string& config_path);
};

inline void to_json(nlohmann::json& j, const c_config::config_conf::esp_conf::player& value)
{
	j = json{
		{ _("color"), value.color },
		{ _("impacts_color"), value.impacts_color },
		{ _("box"), value.box },
		{ _("info"), value.info },
		{ _("impacts"), value.impacts },
		{ _("radar"), value.radar },
		{ _("bar"), value.bar },
		{ _("skeleton"), value.skeleton },
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::esp_conf::player& value)
{
	value.color = j.value(_("color"), c_color(120, 120, 255, 255));
	value.impacts_color = j.value(_("impacts_color"), c_color(120, 120, 255, 255));
	value.box = j.value(_("box"), false);
	value.info = j.value(_("info"), false);
	value.impacts = j.value(_("impacts"), false);
	value.radar = j.value(_("radar"), 0);
	value.bar = j.value(_("bar"), 0);
	value.skeleton = j.value(_("skeleton"), 0);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::esp_conf& value)
{
	j = json{
		{ _("team"), value.team },
		{ _("enemy"), value.enemy },
		{ _("nade_type"), value.nade_type },
		{ _("nade_color"), value.nade_color },
		{ _("nade_pred"), value.nade_pred },
		{ _("hitmarker"), value.hitmarker },
		{ _("hitsound"), value.hitsound },
		{ _("hitsound_volume"), value.hitsound_volume }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::esp_conf& value)
{
	value.team = j.value(_("team"), c_config::config_conf::esp_conf::player{});
	value.enemy = j.value(_("enemy"), c_config::config_conf::esp_conf::player{});
	value.nade_type = j.value(_("nade_type"), 0);
	value.nade_color = j.value(_("nade_color"), c_color::primary());
	value.nade_pred = j.value(_("nade_pred"), false);
	value.hitmarker = j.value(_("hitmarker"), false);
	value.hitsound = j.value(_("hitsound"), 0);
	value.hitsound_volume = j.value(_("hitsound_volume"), 100.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::chams_conf::chams& value)
{
	j = json{
		{ _("type"), value.type },
		{ _("color"), value.color },
		{ _("xqz"), value.xqz }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::chams_conf::chams& value)
{
	value.type = j.value(_("type"), 0);
	value.color = j.value(_("color"), c_color(120, 120, 255, 255));
	value.xqz = j.value(_("xqz"), false);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::chams_conf& value)
{
	j = json{
		{ _("team"), value.team },
		{ _("enemy"), value.enemy },
		{ _("backtrack"), value.backtrack },
		{ _("local"), value.local },
		{ _("arms"), value.arms },
		{ _("weapon"), value.weapon },
		{ _("remove_sleeves"), value.remove_sleeves }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::chams_conf& value)
{
	value.team = j.value(_("team"), c_config::config_conf::chams_conf::chams{});
	value.enemy = j.value(_("enemy"), c_config::config_conf::chams_conf::chams{});
	value.backtrack = j.value(_("backtrack"), c_config::config_conf::chams_conf::chams{});
	value.local = j.value(_("local"), c_config::config_conf::chams_conf::chams{});
	value.arms = j.value(_("arms"), c_config::config_conf::chams_conf::chams{});
	value.weapon = j.value(_("weapon"), c_config::config_conf::chams_conf::chams{});
	value.remove_sleeves = j.value(_("remove_sleeves"), false);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::legit_conf& value)
{
	j = json{
		{ _("assist"), value.assist },
		{ _("fov"), value.fov },
		{ _("smooth"), value.smooth },
		{ _("backtrack"), value.backtrack }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::legit_conf& value)
{
	value.assist = j.value(_("assist"), false);
	value.fov = j.value(_("fov"), 0.f);
	value.smooth = j.value(_("smooth"), 0.f);
	value.backtrack = j.value(_("backtrack"), 0.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf::weapon_conf& value)
{
	j = json{
		{ _("hitchance"), value.hitchance },
		{ _("min_dmg"), value.min_dmg },
		{ _("head_scale"), value.head_scale },
		{ _("body_scale"), value.body_scale },
		{ _("override_min_damage"), value.override_min_damage }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf::weapon_conf& value)
{
	value.hitchance = j.value(_("hitchance"), 0.f);
	value.min_dmg = j.value(_("min_dmg"), 0.f);
	value.head_scale = j.value(_("head_scale"), 0.f);
	value.body_scale = j.value(_("body_scale"), 0.f);
	value.override_min_damage = j.value(_("override_min_damage"), 0.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::rage_conf& value)
{
	j = json{
		{ _("enabled"), value.enabled },
		{ _("experimental_aa"), value.experimental_aa },
		{ _("fakelag_disable_knife"), value.fakelag_disable_knife },
		{ _("fakelag_disable_taser"), value.fakelag_disable_taser },
		{ _("fakelag_disable_revolver"), value.fakelag_disable_revolver },
		{ _("experimental_aa_flip"), value.experimental_aa_flip },
		{ _("slow_walk"), value.slow_walk },
		{ _("override_min_damage_key"), value.override_min_damage_key },
		{ _("prevent_baim_key"), value.prevent_baim_key },
		{ _("override_resolver_key"), value.override_resolver_key },
		{ _("force_onshot_key"), value.force_onshot_key },
		{ _("auto_snipe"), value.auto_snipe },
		{ _("scout"), value.scout },
		{ _("awp"), value.awp },
		{ _("pistol"), value.pistol },
		{ _("heavy"), value.heavy },
		{ _("rifle"), value.rifle },
		{ _("misc"), value.misc },
		{ _("body_lean"), value.body_lean },
		{ _("inv_body_lean"), value.inv_body_lean },
		{ _("fakelag_standing"), value.fakelag_standing },
		{ _("fakelag_moving"), value.fakelag_moving },
		{ _("fakelag_in_air"), value.fakelag_in_air },
		{ _("fakelag_fakeland"), value.fakelag_fakeland }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::rage_conf& value)
{
	value.enabled = j.value(_("enabled"), false);
	value.experimental_aa = j.value(_("experimental_aa"), false);
	value.fakelag_disable_knife = j.value(_("fakelag_disable_knife"), false);
	value.fakelag_disable_taser = j.value(_("fakelag_disable_taser"), false);
	value.fakelag_disable_revolver = j.value(_("fakelag_disable_revolver"), false);
	value.experimental_aa_flip = j.value(_("experimental_aa_flip"), 0);
	value.slow_walk = j.value(_("slow_walk"), 0);
	value.override_min_damage_key = j.value(_("override_min_damage_key"), 0);
	value.prevent_baim_key = j.value(_("prevent_baim_key"), 0);
	value.override_resolver_key = j.value(_("override_resolver_key"), 0);
	value.force_onshot_key = j.value(_("force_onshot_key"), 0);
	value.auto_snipe = j.value(_("auto_snipe"), c_config::config_conf::rage_conf::weapon_conf{});
	value.scout = j.value(_("scout"), c_config::config_conf::rage_conf::weapon_conf{});
	value.awp = j.value(_("awp"), c_config::config_conf::rage_conf::weapon_conf{});
	value.pistol = j.value(_("pistol"), c_config::config_conf::rage_conf::weapon_conf{});
	value.heavy = j.value(_("heavy"), c_config::config_conf::rage_conf::weapon_conf{});
	value.rifle = j.value(_("rifle"), c_config::config_conf::rage_conf::weapon_conf{});
	value.misc = j.value(_("misc"), c_config::config_conf::rage_conf::weapon_conf{});
	value.body_lean = j.value(_("body_lean"), 100.f);
	value.inv_body_lean = j.value(_("inv_body_lean"), 100.f);
	value.fakelag_standing = j.value(_("fakelag_standing"), 1.f);
	value.fakelag_moving = j.value(_("fakelag_moving"), 13.f);
	value.fakelag_in_air = j.value(_("fakelag_in_air"), 8.f);
	value.fakelag_fakeland = j.value(_("fakelag_fakeland"), 16.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf::misc_conf& value)
{
	j = json{
		{ _("radio_channel"), value.radio_channel },
		{ _("radio_volume"), value.radio_volume },
		{ _("radio_mute"), value.radio_mute },
		{ _("ping_switch"), value.ping_switch },
		{ _("thirdperson_dist"), value.thirdperson_dist },
		{ _("thirdperson_switch"), value.thirdperson_switch },
		{ _("buy_bot"), value.buy_bot },
		{ _("engine_radar"), value.engine_radar },
		{ _("no_smoke"), value.no_smoke },
		{ _("no_flash"), value.no_flash },
		{ _("no_scope"), value.no_scope },
		{ _("preserve_killfeed"), value.preserve_killfeed },
		{ _("hit_effect"), value.hit_effect },
		{ _("kill_effect"), value.kill_effect },
		{ _("night_mode"), value.night_mode },
		{ _("clantag"), value.clantag },
		{ _("indicators"), value.indicators },
		{ _("arrows"), value.arrows },
		{ _("fov"), value.fov },
		{ _("fov_view"), value.fov_view },
		{ _("props_transparency"), value.props_transparency }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf::misc_conf& value)
{
	value.radio_channel = j.value(_("radio_channel"), 0);
	value.radio_volume = j.value(_("radio_volume"), 100.f);
	value.radio_mute = j.value(_("radio_mute"), 0);
	value.ping_switch = j.value(_("ping_switch"), 0);
	value.thirdperson_dist = j.value(_("thirdperson_dist"), 0.f);
	value.thirdperson_switch = j.value(_("thirdperson_switch"), 0);
	value.buy_bot = j.value(_("buy_bot"), 0);
	value.engine_radar = j.value(_("engine_radar"), false);
	value.no_smoke = j.value(_("no_smoke"), false);
	value.no_flash = j.value(_("no_flash"), false);
	value.no_scope = j.value(_("no_scope"), false);
	value.preserve_killfeed = j.value(_("preserve_killfeed"), false);
	value.hit_effect = j.value(_("hit_effect"), false);
	value.kill_effect = j.value(_("kill_effect"), false);
	value.night_mode = j.value(_("night_mode"), false);
	value.clantag = j.value(_("clantag"), false);
	value.indicators = j.value(_("indicators"), false);
	value.arrows = j.value(_("arrows"), false);
	value.fov = j.value(_("fov"), 68.f);
	value.fov_view = j.value(_("fov_view"), 90.f);
	value.props_transparency = j.value(_("props_transparency"), 0.f);
}

inline void to_json(nlohmann::json& j, const c_config::config_conf& value)
{
	j = json{
		{ _("esp"), value.esp },
		{ _("chams"), value.chams },
		{ _("legit"), value.legit },
		{ _("rage"), value.rage },
		{ _("misc"), value.misc }
	};
}

inline void from_json(const nlohmann::json& j, c_config::config_conf& value)
{
	value.esp = j.value(_("esp"), c_config::config_conf::esp_conf{});
	value.chams = j.value(_("chams"), c_config::config_conf::chams_conf{});
	value.legit = j.value(_("legit"), c_config::config_conf::legit_conf{});
	value.rage = j.value(_("rage"), c_config::config_conf::rage_conf{});
	value.misc = j.value(_("misc"), c_config::config_conf::misc_conf{});
}

inline void c_config::load_from_file(const std::string& config_path)
{
	std::ifstream file(config_path);
	std::stringstream rage;
	rage << file.rdbuf();

	if (rage.str().empty())
		return;

	auto cfg = base64::decode(rage.str());
	from_json(json::parse(cfg), instance()->get());
}

inline void c_config::save_to_file(const std::string& config_path)
{
	json exp;
	to_json(exp, instance()->get());
	const auto str = base64::encode(exp.dump());

	std::ofstream file(config_path);
	file << str;
}

#define config c_config::instance()->get()
