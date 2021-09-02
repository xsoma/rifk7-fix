#include "c_hvh.h"
#include "../security/string_obfuscation.h"
#include "framework/c_group_box.h"
#include "framework/c_checkbox.h"
#include "../utils/c_config.h"
#include "framework/c_float_select.h"
#include "framework/c_hotkey_select.h"

c_hvh::c_hvh(const c_vector2d size) : c_flow_layout(size, c_vector2d(12, 12))
{
	add_child(std::make_shared<c_group_box>(__("Override Minimum Damage"), std::initializer_list<std::shared_ptr<c_drawable>>
	{
		make_child(c_hotkey_select, __("Override Key"), config.rage.override_min_damage_key),
		make_child(c_float_select, __("Auto Snipers"), config.rage.auto_snipe.override_min_damage),
		make_child(c_float_select, __("Scout"), config.rage.scout.override_min_damage),
		make_child(c_float_select, __("AWP"), config.rage.awp.override_min_damage),
		make_child(c_float_select, __("Light Pistols"), config.rage.pistol.override_min_damage),
		make_child(c_float_select, __("Heavy Pistols"), config.rage.heavy.override_min_damage),
		make_child(c_float_select, __("Other Guns"), config.rage.misc.override_min_damage)
	}, c_vector2d(275, 140)));

	add_child(std::make_shared<c_group_box>(__("Fakelag Settings"), std::initializer_list<std::shared_ptr<c_drawable>>
	{
		make_child(c_float_select, __("Factor When Stand"), config.rage.fakelag_standing, 16.f, 1.f),
		make_child(c_float_select, __("Factor When Moving"), config.rage.fakelag_moving, 16.f, 1.f),
		make_child(c_float_select, __("Factor When In Air"), config.rage.fakelag_in_air, 16.f, 1.f),
		make_child(c_float_select, __("Factor For Fakeland"), config.rage.fakelag_fakeland, 16.f, 1.f),
		make_child(c_checkbox, __("Disable With Knife"), config.rage.fakelag_disable_knife),
		make_child(c_checkbox, __("Disable With Taser"), config.rage.fakelag_disable_taser),
		make_child(c_checkbox, __("Disable With Revolver"), config.rage.fakelag_disable_revolver)
	}, c_vector2d(275, 140)));

	add_child(std::make_shared<c_group_box>(__("Accuracy boost"), std::initializer_list<std::shared_ptr<c_drawable>>
	{
		make_child(c_hotkey_select, __("Body Aim Priority"), config.rage.prevent_baim_key),
			make_child(c_hotkey_select, __("Override Resolver"), config.rage.override_resolver_key),
			make_child(c_hotkey_select, __("Force On-shot Aim "), config.rage.force_onshot_key),
	}, c_vector2d(275, 60)));

	add_child(std::make_shared<c_group_box>(__("Desync Settings"), std::initializer_list<std::shared_ptr<c_drawable>>
	{
		make_child(c_float_select, __("Body Lean"), config.rage.body_lean, 100.f),
		make_child(c_float_select, __("Inverted Body Lean"), config.rage.inv_body_lean, 100.f)
	}, c_vector2d(275, 140)));
}
