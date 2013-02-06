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
**    Harry Storbacka
**    Mark Page
*/

#pragma once

#include "API/GL/opengl_window_description.h"
#include "GL/opengl_target_provider.h"

namespace clan
{

class GL1TargetProvider : public OpenGLTargetProvider
{
/// \name Construction
/// \{

public:
	GL1TargetProvider();

	~GL1TargetProvider();


/// \}
/// \name Attributes
/// \{

public:

/// \}
/// \name Operations
/// \{

public:
	DisplayWindowProvider *alloc_display_window();

/// \}
/// \name Implementation
/// \{

private:
#ifdef WIN32
	friend class GL1WindowProvider_WGL;
#else
	friend class GL1WindowProvider_GLX;
#endif

/// \}
};

}