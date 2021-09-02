#pragma once

#include <sstream>
#include "../security/string_obfuscation.h"

static const auto welcome_message = []() -> std::pair<std::string, char>
{
	std::stringstream str;
	str << _("-------------------------------------------------------------------------------") << std::endl;
	str << _("					rifk7 [tweex-csgo]") << std::endl;
	str << _("-------------------------------------------------------------------------------") << std::endl;

	constexpr auto key = random::_char<__COUNTER__>::value;
	auto txt = str.str();

	for (auto& c : txt)
		c ^= key;

	return std::make_pair(txt, key);
}();
