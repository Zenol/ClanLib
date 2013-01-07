/*
**  ClanLib SDK
**  Copyright (c) 1997-2012 The ClanLib Team
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
**  Note: Some of the libraries ClanLib may link to may have additional
**  requirements or restrictions.
**
**  File Author(s):
**
**    Magnus Norddahl
*/

#pragma once

#include "../css_property_parser.h"
#include "API/CSSLayout/css_box_properties.h"

namespace clan
{

class CSSParserBorderImage : public CSSPropertyParser
{
public:
	std::vector<std::string> get_names();
	void parse(CSSBoxProperties &properties, const std::string &name, const std::vector<CSSToken> &tokens);
	
private:
	bool parse_source(CSSValueBorderImageSource &border_image_source, size_t &parse_pos, const std::vector<CSSToken> &tokens);
	bool parse_slice(CSSValueBorderImageSlice &border_image_slice, size_t &parse_pos, const std::vector<CSSToken> &tokens);
	bool parse_width(CSSValueBorderImageWidth &border_image_width, size_t &parse_pos, const std::vector<CSSToken> &tokens);
	bool parse_outset(CSSValueBorderImageOutset &border_image_outset, size_t &parse_pos, const std::vector<CSSToken> &tokens);
	bool parse_repeat(CSSValueBorderImageRepeat &border_image_repeat, size_t &parse_pos, const std::vector<CSSToken> &tokens);
};

}