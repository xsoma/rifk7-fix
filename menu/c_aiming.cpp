#include "c_aiming.h"
#include "../security/string_obfuscation.h"
#include "framework/c_group_box.h"
#include "framework/c_checkbox.h"
#include "../utils/c_config.h"
#include "framework/c_float_select.h"
#include "framework/c_hotkey_select.h"

c_aiming::c_aiming(const c_vector2d size) : c_flow_layout(size, c_vector2d(12, 12))
{
	add_child(std::make_shared<c_group_box>(__("General"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_checkbox, __("Enabled"), config.rage.enabled),
		make_child(c_checkbox, __("LBY Breaker"), config.rage.experimental_aa),
		make_child(c_hotkey_select, __("Flip Desync Side"), config.rage.experimental_aa_flip),
		make_child(c_hotkey_select, __("Slowwalk"), config.rage.slow_walk)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Auto Snipers"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.auto_snipe.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.auto_snipe.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.auto_snipe.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.auto_snipe.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Scout"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.scout.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.scout.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.scout.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.scout.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("AWP"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.awp.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.awp.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.awp.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.awp.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Light Pistols"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.pistol.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.pistol.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.pistol.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.pistol.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Heavy Pistols"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.heavy.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.heavy.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.heavy.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.heavy.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Rifles"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.rifle.hitchance),
			make_child(c_float_select, __("Minimum Damage"), config.rage.rifle.min_dmg),
			make_child(c_float_select, __("Head Scale"), config.rage.rifle.head_scale, 100.f),
			make_child(c_float_select, __("Body Scale"), config.rage.rifle.body_scale, 100.f)
	}, c_vector2d(275, 75)));
	add_child(std::make_shared<c_group_box>(__("Other Guns"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_float_select, __("Hitchance"), config.rage.misc.hitchance),
		make_child(c_float_select, __("Minimum Damage"), config.rage.misc.min_dmg),
		make_child(c_float_select, __("Head Scale"), config.rage.misc.head_scale, 100.f),
		make_child(c_float_select, __("Body Scale"), config.rage.misc.body_scale, 100.f)
	}, c_vector2d(275, 75)));
}
