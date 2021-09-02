#include "c_misc.h"
#include "framework/c_group_box.h"
#include "framework/c_float_select.h"
#include "../security/string_obfuscation.h"
#include "../utils/c_config.h"
#include "framework/c_hotkey_select.h"
#include "framework/c_checkbox.h"

c_misc::c_misc(const c_vector2d size) : c_flow_layout(size, c_vector2d(12, 12))
{
	add_child(std::make_shared<c_group_box>(__("Gameplay"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_hotkey_select, __("Fake Latency Switch"), config.misc.ping_switch),
		make_child(c_float_select, __("Thirdperson Distance"), config.misc.thirdperson_dist, 200.f, 0.f, 1.f),
		make_child(c_hotkey_select, __("Thirdperson Switch"), config.misc.thirdperson_switch),
		make_child(c_select, __("Buy Bot"), std::vector<std::pair<std::string, char>> {
			__("Off"),
			__("Auto Snipers"),
			__("Scout"),
			__("AWP")
		}, config.misc.buy_bot),
		make_child(c_checkbox, __("Engine Radar"), config.misc.engine_radar),
		make_child(c_checkbox, __("No Scope"), config.misc.no_scope),
		make_child(c_checkbox, __("No Smoke"), config.misc.no_smoke),
		make_child(c_checkbox, __("No Flash"), config.misc.no_flash)
	}));
	add_child(std::make_shared<c_group_box>(__("Visuals"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_checkbox, __("Show indicators"), config.misc.indicators),
		make_child(c_checkbox, __("Anti-aim arrows"), config.misc.arrows),
		make_child(c_float_select, __("World FOV"), config.misc.fov),
		make_child(c_float_select, __("Viewmodel FOV"), config.misc.fov_view),
		make_child(c_checkbox, __("Preserve Killfeed"), config.misc.preserve_killfeed),
		make_child(c_checkbox, __("Hit Effect"), config.misc.hit_effect),
		make_child(c_checkbox, __("Kill Effect"), config.misc.kill_effect),
		make_child(c_checkbox, __("Night Mode"), config.misc.night_mode),
		make_child(c_float_select, __("Props Transparency"), config.misc.props_transparency)
	}));
	add_child(std::make_shared<c_group_box>(__("Other"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_checkbox, __("Clantag"), config.misc.clantag),
		make_child(c_select, __("Radio channel"), std::vector<std::pair<std::string, char>> {
			__("Off"),
			__("Chanson"),
			__("1990s"),
			__("DE Rap"),
			__("Minimal"),
			__("Chill Out"),
			__("Podrubasik"),
			__("Hand of Doom"),
			__("Metal Nation"),
			__("News")
		}, config.misc.radio_channel),
		make_child(c_float_select, __("Radio volume"), config.misc.radio_volume, 100.f, 0.f, 1.f),
		make_child(c_hotkey_select, __("Mute radio"), config.misc.radio_mute)
	}));
}
