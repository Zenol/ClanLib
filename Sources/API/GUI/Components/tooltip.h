/*
**  ClanLib SDK
**  Copyright (c) 1997-2013 The ClanLib Team
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
**    Harry Storbacka
*/


#pragma once

#include "../api_gui.h"
#include "../gui_component.h"

namespace clan
{
/// \addtogroup clanGUI_Components clanGUI Components
/// \{

class GUIManager;
class ToolTip_Impl;

/// \brief Tool tip component.
class CL_API_GUI ToolTip : public GUIComponent
{
/// \name Construction
/// \{
public:

	/// \brief Constructs a ToolTip
	///
	/// \param gui_manager = GUIManager
	ToolTip(GUIManager gui_manager);

	virtual ~ToolTip();

/// \}
/// \name Attributes
/// \{
public:

	using GUIComponent::get_named_item;

	/// \brief Find the child ToolTip with the specified component ID name.
	///
	/// If it was not found, an exception is thrown.
	static ToolTip *get_named_item(GUIComponent *reference_component, const std::string &id);

	/// \brief Get Text
	///
	/// \return text
	std::string get_text() const;

/// \}
/// \name Operations
/// \{
public:

	/// \brief Set text
	///
	/// \param text = String Ref
	void set_text(const std::string &text);

	/// \brief Show
	///
	/// \param position = Point
	void show(const Point &position);

	/// \brief Show delayed
	///
	/// \param position = Point
	/// \param delay_time = value
	void show_delayed(const Point &position, int delay_time = 750);

	/// \brief Hide
	void hide();

/// \}
/// \name Implementation
/// \{
private:
	std::shared_ptr<ToolTip_Impl> impl;
/// \}
};

}

/// \}
