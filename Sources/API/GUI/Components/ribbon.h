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

#include "../gui_component.h"
#include "../../Core/Signals/callback_v0.h"
#include "../../Core/Signals/callback_1.h"
#include "../gui_theme_part.h"
namespace clan
{

class RibbonMenu;
class RibbonPage;
class RibbonSection;
class PushButton;
class Ribbon_Impl;

/// \brief Ribbon component.
class CL_API_GUI Ribbon : public GUIComponent
{
/// \name Construction
/// \{
public:
	Ribbon(GUIComponent *container);
	~Ribbon();
/// \}

/// \name Attributes
/// \{
public:
	RibbonMenu *get_menu();
/// \}

/// \name Operations
/// \{
public:
/// \}

/// \name Implementation
/// \{
private:
	std::shared_ptr<Ribbon_Impl> impl;

	friend class RibbonPage;
/// \}
};

}
