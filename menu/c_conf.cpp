#include "c_conf.h"
#include "../c_rifk.h"
#include "../security/string_obfuscation.h"
#include "framework/c_group_box.h"
#include "framework/c_button.h"
#include "../utils/c_config.h"

c_conf::c_conf(const c_vector2d size) : c_flow_layout(size, c_vector2d(12, 12))
{
	static const auto cfg1 = __("C:\\rifk7\\cfg\\1.cfg");
	static const auto cfg2 = __("C:\\rifk7\\cfg\\2.cfg");
	static const auto cfg3 = __("C:\\rifk7\\cfg\\3.cfg");
	static const auto cfg4 = __("C:\\rifk7\\cfg\\4.cfg");
	static const auto cfg5 = __("C:\\rifk7\\cfg\\5.cfg");
	static const auto cfg6 = __("C:\\rifk7\\cfg\\6.cfg");
	static const auto cfg7 = __("C:\\rifk7\\cfg\\7.cfg");
	static const auto cfg8 = __("C:\\rifk7\\cfg\\8.cfg");
	static const auto cfg9 = __("C:\\rifk7\\cfg\\9.cfg");
	static const auto cfg10 = __("C:\\rifk7\\cfg\\10.cfg");
	static const auto cfg11 = __("C:\\rifk7\\cfg\\11.cfg");
	static const auto cfg12 = __("C:\\rifk7\\cfg\\12.cfg");

	add_child(std::make_shared<c_group_box>(__("1"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 1"), c_vector2d(8, 4), []() -> void
		{
			_rt(ra, cfg1);
			char path[MAX_PATH];
			sprintf_s(path, ra, loader_info.loader_path);
			c_config::load_from_file(path);
		}),
			make_child(c_button, __("Save 1"), c_vector2d(8, 4), []() -> void
		{
			_rt(ra, cfg1);
			char path[MAX_PATH];
			sprintf_s(path, ra, loader_info.loader_path);
			c_config::save_to_file(path);
		})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("2"), std::initializer_list<std::shared_ptr<c_drawable>> {
			make_child(c_button, __("Load 2"), c_vector2d(8, 4), []() -> void
		{
			_rt(le, cfg2);
			char path[MAX_PATH];
			sprintf_s(path, le, loader_info.loader_path);
			c_config::load_from_file(path);
		}),
			make_child(c_button, __("Save 2"), c_vector2d(8, 4), []() -> void
		{
			_rt(le, cfg2);
			char path[MAX_PATH];
			sprintf_s(path, le, loader_info.loader_path);
			c_config::save_to_file(path);
		})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("3"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 3"), c_vector2d(8, 4), []() -> void
			{
				_rt(ra, cfg3);
				char path[MAX_PATH];
				sprintf_s(path, ra, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 3"), c_vector2d(8, 4), []() -> void
				{
					_rt(ra, cfg3);
					char path[MAX_PATH];
					sprintf_s(path, ra, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("4"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 4"), c_vector2d(8, 4), []() -> void
			{
				_rt(le, cfg4);
				char path[MAX_PATH];
				sprintf_s(path, le, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 4"), c_vector2d(8, 4), []() -> void
				{
					_rt(le, cfg4);
					char path[MAX_PATH];
					sprintf_s(path, le, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("5"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 5"), c_vector2d(8, 4), []() -> void
			{
				_rt(ra, cfg5);
				char path[MAX_PATH];
				sprintf_s(path, ra, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 5"), c_vector2d(8, 4), []() -> void
				{
					_rt(ra, cfg5);
					char path[MAX_PATH];
					sprintf_s(path, ra, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("6"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 6"), c_vector2d(8, 4), []() -> void
			{
				_rt(le, cfg6);
				char path[MAX_PATH];
				sprintf_s(path, le, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 6"), c_vector2d(8, 4), []() -> void
				{
					_rt(le, cfg6);
					char path[MAX_PATH];
					sprintf_s(path, le, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("7"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 7"), c_vector2d(8, 4), []() -> void
			{
				_rt(ra, cfg7);
				char path[MAX_PATH];
				sprintf_s(path, ra, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 7"), c_vector2d(8, 4), []() -> void
				{
					_rt(ra, cfg7);
					char path[MAX_PATH];
					sprintf_s(path, ra, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("8"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 8"), c_vector2d(8, 4), []() -> void
			{
				_rt(le, cfg8);
				char path[MAX_PATH];
				sprintf_s(path, le, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 8"), c_vector2d(8, 4), []() -> void
				{
					_rt(le, cfg8);
					char path[MAX_PATH];
					sprintf_s(path, le, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("9"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 9"), c_vector2d(8, 4), []() -> void
			{
				_rt(ra, cfg9);
				char path[MAX_PATH];
				sprintf_s(path, ra, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 9"), c_vector2d(8, 4), []() -> void
				{
					_rt(ra, cfg9);
					char path[MAX_PATH];
					sprintf_s(path, ra, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("10"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 10"), c_vector2d(8, 4), []() -> void
			{
				_rt(le, cfg10);
				char path[MAX_PATH];
				sprintf_s(path, le, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 10"), c_vector2d(8, 4), []() -> void
				{
					_rt(le, cfg10);
					char path[MAX_PATH];
					sprintf_s(path, le, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("11"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 11"), c_vector2d(8, 4), []() -> void
			{
				_rt(ra, cfg11);
				char path[MAX_PATH];
				sprintf_s(path, ra, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 11"), c_vector2d(8, 4), []() -> void
				{
					_rt(ra, cfg11);
					char path[MAX_PATH];
					sprintf_s(path, ra, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));

	add_child(std::make_shared<c_group_box>(__("12"), std::initializer_list<std::shared_ptr<c_drawable>> {
		make_child(c_button, __("Load 12"), c_vector2d(8, 4), []() -> void
			{
				_rt(le, cfg12);
				char path[MAX_PATH];
				sprintf_s(path, le, loader_info.loader_path);
				c_config::load_from_file(path);
			}),
			make_child(c_button, __("Save 12"), c_vector2d(8, 4), []() -> void
				{
					_rt(le, cfg12);
					char path[MAX_PATH];
					sprintf_s(path, le, loader_info.loader_path);
					c_config::save_to_file(path);
				})
	}, c_vector2d(275, 40)));
}
